#include <algorithm>
#include <cerrno>
#include <charconv>
#include <climits>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <format>
#include <iostream>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>
#include <csignal>
#include <sys/types.h>
#include <unistd.h>
#include <exception>

#include "GGEMS/logging/GGEMSLogger.hh"
#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/random/GGEMSRandomEngine.hh"
#include "GGEMS/units/GGEMSBytesUnits.hh"
#include "GGEMS/units/GGEMSUnitConversion.hh"

#include "GGEMSRandomUInt32ChunkProducer.hh"

namespace {

using ggems::validation::random::GGEMSRandomUInt32ChunkProducer;
using ggems::validation::random::RandomUInt32StreamLayout;
using ggems::validation::random::RandomUInt32StreamSpecification;

constexpr std::size_t kBytesPerWord{sizeof(std::uint32_t)};
constexpr int kRequiredBitsPerByte{8};
constexpr std::uint32_t kByteMask{0xFFU};
constexpr unsigned int kSecondByteShift{8U};
constexpr unsigned int kThirdByteShift{16U};
constexpr unsigned int kFourthByteShift{24U};
constexpr std::size_t kSerializationBufferBytes{std::size_t{1024U} * 1024U};
constexpr std::size_t kSerializationBufferWords{kSerializationBufferBytes /
                                                kBytesPerWord};

static_assert(CHAR_BIT == kRequiredBitsPerByte);
static_assert(sizeof(std::uint32_t) == 4U);
static_assert(kSerializationBufferBytes % kBytesPerWord == 0U);

// =============================================================================
// =============================================================================

struct Options {
  ggems::core::random::GGEMSRandomEngine engine{
      ggems::core::random::GGEMSRandomEngine::Philox};
  std::uint64_t seed{0ULL};
  std::uint64_t stream_offset{0ULL};
  std::uint32_t worker_count{0U};
  std::uint32_t samples_per_worker{0U};
  std::uint64_t max_chunk_mib{0ULL};
  ggems::units::Bytes maximum_value_buffer_size{0ULL};
  std::size_t local_size{0U};
  std::string device_selector;
  RandomUInt32StreamLayout layout{RandomUInt32StreamLayout::Interleaved};
  std::optional<std::uint64_t> requested_output_word_limit;
  std::uint64_t output_word_limit{0ULL};
  std::uint64_t logical_word_capacity{0ULL};
};

struct OptionPresence {
  bool engine{false};
  bool seed{false};
  bool stream_offset{false};
  bool worker_count{false};
  bool samples_per_worker{false};
  bool max_chunk_mib{false};
  bool local_size{false};
  bool device_selector{false};
  bool layout{false};
  bool output_word_limit{false};
};

enum class WriteStatus : std::uint8_t { Complete, DownstreamClosed };

// =============================================================================
// =============================================================================

auto PrintUsage(char const *executable_name) -> void {
  std::cerr << "Usage:\n"
            << " " << executable_name << " [options]\n\n"
            << "Required options:\n"
            << "  --engine <jkiss|pcg32|philox>\n"
            << "  --seed <uint64>\n"
            << "  --stream-offset <uint64>\n"
            << "  --workers <uint32>\n"
            << "  --samples-per-worker <uint32>\n"
            << "  --max-chunk-mib <uint64>\n"
            << "  --local-size <size_t>\n"
            << "  --device <selector>\n"
            << "  --layout <worker_major|interleaved>\n\n"
            << "Optional options:\n"
            << "  --output-word-limit <uint64>\n"
            << "  --help\n\n"
            << "Standard output contains only canonical little-endian uint32 "
               "bytes.\n";
}

[[nodiscard]] auto ReadArgumentValue(int &index, int argc,
                                     char const *const *argv,
                                     std::string_view option_name)
    -> std::string_view {
  if (index + 1 >= argc) {
    throw std::runtime_error(
        std::format("Missing value after '{}'.", option_name));
  }
  ++index;
  return argv[index];
}

auto MarkPresent(bool &present, std::string_view option_name) -> void {
  if (present) {
    throw std::runtime_error(
        std::format("Option '{}' may be specified only once.", option_name));
  }
  present = true;
}

template <std::unsigned_integral Integer>
[[nodiscard]] auto ParseUnsignedInteger(std::string_view value,
                                        std::string_view option_name)
    -> Integer {
  Integer parsed{0};
  auto const result =
      std::from_chars(value.data(), value.data() + value.size(), parsed);
  if (value.empty() || result.ec != std::errc{} ||
      result.ptr != value.data() + value.size()) {
    throw std::runtime_error(std::format("Invalid decimal value '{}' for '{}'.",
                                         value, option_name));
  }
  return parsed;
}

[[nodiscard]] auto CheckedMultiply(std::uint64_t lhs, std::uint64_t rhs,
                                   std::string_view label) -> std::uint64_t {
  if (lhs != 0ULL && rhs > std::numeric_limits<std::uint64_t>::max() / lhs) {
    throw std::runtime_error(std::format("{} overflows uint64.", label));
  }
  return lhs * rhs;
}

auto ValidateRequiredOptions(OptionPresence const &presence) -> void {
  if (!presence.engine || !presence.seed || !presence.stream_offset ||
      !presence.worker_count || !presence.samples_per_worker ||
      !presence.max_chunk_mib || !presence.local_size ||
      !presence.device_selector || !presence.layout) {
    throw std::runtime_error("A required producer option is missing.");
  }
}

auto FinalizeOptions(Options &options) -> void {
  if (options.worker_count == 0U || options.samples_per_worker == 0U) {
    throw std::runtime_error("Stream dimensions must be greater than zero.");
  }
  if (options.max_chunk_mib == 0ULL) {
    throw std::runtime_error("Maximum chunk size must be greater than zero.");
  }
  if (options.local_size == 0U) {
    throw std::runtime_error("Local size must be greater than zero.");
  }
  if (options.device_selector.empty()) {
    throw std::runtime_error("Device selector must not be empty.");
  }

  auto const maximum_value_buffer_size =
      ggems::units::MakeQuantity<ggems::units::Bytes>(options.max_chunk_mib,
                                                      "MiB");
  if (!maximum_value_buffer_size.has_value()) {
    throw std::runtime_error(
        "Maximum chunk size cannot be represented exactly in bytes.");
  }
  options.maximum_value_buffer_size = *maximum_value_buffer_size;
  options.logical_word_capacity =
      CheckedMultiply(options.worker_count, options.samples_per_worker,
                      "Logical raw uint32 word capacity");
  options.output_word_limit = options.requested_output_word_limit.value_or(
      options.logical_word_capacity);
  if (options.output_word_limit > options.logical_word_capacity) {
    throw std::runtime_error(
        "Output word limit exceeds the finite logical stream capacity.");
  }
}

[[nodiscard]] auto ParseArguments(int argc, char const *const *argv)
    -> Options {
  Options options;
  OptionPresence presence;

  for (int index = 1; index < argc; ++index) {
    std::string_view const argument{argv[index]};

    if (argument == "--help" || argument == "-h") {
      PrintUsage(argv[0]);
      std::exit(EXIT_SUCCESS);
    }
    if (argument == "--engine") {
      MarkPresent(presence.engine, argument);
      options.engine = ggems::core::random::ParseRandomEngine(
          ReadArgumentValue(index, argc, argv, argument));
    } else if (argument == "--seed") {
      MarkPresent(presence.seed, argument);
      options.seed = ParseUnsignedInteger<std::uint64_t>(
          ReadArgumentValue(index, argc, argv, argument), argument);
    } else if (argument == "--stream-offset") {
      MarkPresent(presence.stream_offset, argument);
      options.stream_offset = ParseUnsignedInteger<std::uint64_t>(
          ReadArgumentValue(index, argc, argv, argument), argument);
    } else if (argument == "--workers") {
      MarkPresent(presence.worker_count, argument);
      options.worker_count = ParseUnsignedInteger<std::uint32_t>(
          ReadArgumentValue(index, argc, argv, argument), argument);
    } else if (argument == "--samples-per-worker") {
      MarkPresent(presence.samples_per_worker, argument);
      options.samples_per_worker = ParseUnsignedInteger<std::uint32_t>(
          ReadArgumentValue(index, argc, argv, argument), argument);
    } else if (argument == "--max-chunk-mib") {
      MarkPresent(presence.max_chunk_mib, argument);
      options.max_chunk_mib = ParseUnsignedInteger<std::uint64_t>(
          ReadArgumentValue(index, argc, argv, argument), argument);
    } else if (argument == "--local-size") {
      MarkPresent(presence.local_size, argument);
      options.local_size = ParseUnsignedInteger<std::size_t>(
          ReadArgumentValue(index, argc, argv, argument), argument);
    } else if (argument == "--device") {
      MarkPresent(presence.device_selector, argument);
      options.device_selector = ReadArgumentValue(index, argc, argv, argument);
    } else if (argument == "--layout") {
      MarkPresent(presence.layout, argument);
      options.layout = ggems::validation::random::ParseRandomUInt32StreamLayout(
          ReadArgumentValue(index, argc, argv, argument));
    } else if (argument == "--output-word-limit") {
      MarkPresent(presence.output_word_limit, argument);
      options.requested_output_word_limit = ParseUnsignedInteger<std::uint64_t>(
          ReadArgumentValue(index, argc, argv, argument), argument);
    } else {
      throw std::runtime_error(std::format("Unknown argument '{}'.", argument));
    }
  }

  ValidateRequiredOptions(presence);
  FinalizeOptions(options);
  return options;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto WriteAll(std::span<std::uint8_t const> bytes,
                            std::uint64_t &written_byte_count) -> WriteStatus {
  std::size_t offset{0U};
  while (offset < bytes.size()) {
    ssize_t const written =
        ::write(STDOUT_FILENO, bytes.data() + offset, bytes.size() - offset);
    if (written > 0) {
      auto const written_size = static_cast<std::size_t>(written);
      if (written_size >
          std::numeric_limits<std::uint64_t>::max() - written_byte_count) {
        throw std::runtime_error("Written-byte counter overflowed uint64.");
      }
      written_byte_count += static_cast<std::uint64_t>(written_size);
      offset += written_size;
      continue;
    }
    if (written < 0 && errno == EINTR) {
      continue;
    }
    if (written < 0 && errno == EPIPE) {
      return WriteStatus::DownstreamClosed;
    }

    int const error_number = written < 0 ? errno : EIO;
    throw std::system_error{error_number, std::generic_category(),
                            "stdout write failed"};
  }
  return WriteStatus::Complete;
}

[[nodiscard]] auto EncodeAndWrite(std::span<std::uint32_t const> words,
                                  std::vector<std::uint8_t> &buffer,
                                  std::uint64_t &written_byte_count)
    -> WriteStatus {
  std::size_t word_offset{0U};

  while (word_offset < words.size()) {
    std::size_t const word_count =
        std::min(kSerializationBufferWords, words.size() - word_offset);
    for (std::size_t index = 0U; index < word_count; ++index) {
      std::uint32_t const value = words[word_offset + index];
      std::size_t const byte_offset = index * kBytesPerWord;
      buffer[byte_offset] = static_cast<std::uint8_t>(value & kByteMask);
      buffer[byte_offset + 1U] =
          static_cast<std::uint8_t>((value >> kSecondByteShift) & kByteMask);
      buffer[byte_offset + 2U] =
          static_cast<std::uint8_t>((value >> kThirdByteShift) & kByteMask);
      buffer[byte_offset + 3U] =
          static_cast<std::uint8_t>((value >> kFourthByteShift) & kByteMask);
    }

    WriteStatus const status = WriteAll(
        std::span<std::uint8_t const>{buffer}.first(word_count * kBytesPerWord),
        written_byte_count);
    if (status == WriteStatus::DownstreamClosed) {
      return status;
    }
    word_offset += word_count;
  }
  return WriteStatus::Complete;
}

[[nodiscard]] auto RunProducer(Options const &options) -> bool {
  auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();
  opencl.SelectDevices({options.device_selector});
  opencl.Initialize();

  auto &contexts = opencl.GetContext();
  if (contexts.empty()) {
    throw std::runtime_error("No GGEMS OpenCL context available.");
  }

  RandomUInt32StreamSpecification specification{
      .engine = options.engine,
      .seed = options.seed,
      .stream_offset = options.stream_offset,
      .worker_count = options.worker_count,
      .samples_per_worker = options.samples_per_worker,
      .layout = options.layout,
      .local_size = options.local_size,
      .maximum_value_buffer_size = options.maximum_value_buffer_size,
  };
  GGEMSRandomUInt32ChunkProducer producer{specification, contexts.front(),
                                          options.output_word_limit};

  std::vector<std::uint8_t> serialization_buffer(kSerializationBufferBytes);
  std::uint64_t written_byte_count{0ULL};
  while (!producer.IsExhausted()) {
    auto const words = producer.NextChunk();
    if (words.empty()) {
      throw std::runtime_error(
          "Chunk producer returned an empty chunk before exhaustion.");
    }
    if (EncodeAndWrite(words, serialization_buffer, written_byte_count) ==
        WriteStatus::DownstreamClosed) {
      return true;
    }
  }

  std::uint64_t const expected_bytes = CheckedMultiply(
      options.output_word_limit, static_cast<std::uint64_t>(kBytesPerWord),
      "Requested output byte count");
  if (producer.GetReturnedWordCount() != options.output_word_limit ||
      written_byte_count != expected_bytes) {
    throw std::runtime_error(
        "Producer stopped before writing the requested logical prefix.");
  }
  return false;
}

} // namespace

// =============================================================================
// =============================================================================

auto main(int argc, char **argv) -> int {
  ggems::core::GGEMSLogger::GetInstance().ClearSinks();

  try {
    Options const options = ParseArguments(argc, argv);
    if (::signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
      throw std::runtime_error("Failed to ignore SIGPIPE for stdout.");
    }

    static_cast<void>(RunProducer(options));
    return EXIT_SUCCESS;
  } catch (std::exception const &error) {
    std::cerr << "GGEMS random stream pipe producer failed: " << error.what()
              << '\n';
    return EXIT_FAILURE;
  } catch (...) {
    std::cerr
        << "GGEMS random stream pipe producer failed: unknown exception.\n";
    return EXIT_FAILURE;
  }
}
