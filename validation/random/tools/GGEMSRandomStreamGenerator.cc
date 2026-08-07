#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>
#include <cstddef>
#include <cstdlib>
#include <exception>
#include <span>
#include <utility>

#include "GGEMS/frameworks/GGEMSOpenCLLaunchGeometry.hh"
#include "GGEMS/core/random/GGEMSRandom.hh"
#include "GGEMS/frameworks/GGEMSOpenCL.hh"
#include "GGEMS/frameworks/GGEMSOpenCLKernel.hh"
#include "GGEMS/frameworks/GGEMSOpenCLSVMBuffer.hh"
#include "GGEMS/core/units/GGEMSBytesUnits.hh"

namespace {
using ggems::core::random::GGEMSRandom;

// =============================================================================
// =============================================================================

enum class StreamType : std::int8_t { RawUInt32, FloatHigh24Bytes };

// =============================================================================
// =============================================================================

auto ToString(StreamType stream_type) -> std::string {
  switch (stream_type) {
  case StreamType::RawUInt32:
    return "raw_uint32";
  case StreamType::FloatHigh24Bytes:
    return "float_high24_bytes";
  }

  throw std::runtime_error("Unsupported stream type.");
}

// =============================================================================
// =============================================================================

auto ToPractRandInputMode(StreamType stream_type) -> std::string {
  switch (stream_type) {
  case StreamType::RawUInt32:
    return "stdin32";
  case StreamType::FloatHigh24Bytes:
    return "stdin8";
  }

  throw std::runtime_error("Unsupported stream type.");
}

// =============================================================================
// =============================================================================

auto ParseStreamType(std::string_view value) -> StreamType {
  if (value == "raw_uint32") {
    return StreamType::RawUInt32;
  }

  if (value == "float_high24_bytes") {
    return StreamType::FloatHigh24Bytes;
  }

  throw std::runtime_error(std::format("Unsupported stream type '{}'.", value));
}

// =============================================================================
// =============================================================================

auto GetOutputByteCount(StreamType stream_type, std::uint64_t total_words)
    -> std::uint64_t {
  switch (stream_type) {
  case StreamType::RawUInt32:
    return total_words * 4ULL;
  case StreamType::FloatHigh24Bytes:
    return total_words * 3ULL;
  }

  throw std::runtime_error("Unsupported stream type.");
}

// =============================================================================
// =============================================================================

struct Options {
  std::string engine{"philox"};
  std::uint64_t seed{77777ULL};
  std::uint32_t particle_count{1024U};
  std::uint32_t words_per_particle{1024U};
  std::size_t local_size{64U};
  std::string device_selector{"cpu"};
  bool force{false};
  StreamType stream_type{StreamType::RawUInt32};

  std::filesystem::path output_path{
      std::filesystem::path{GGEMS_VALIDATION_RANDOM_STREAM_ROOT} /
      "philox_uint32_smoke.bin"};

  std::filesystem::path manifest_path{
      std::filesystem::path{GGEMS_VALIDATION_RANDOM_RESULT_ROOT} / "summary" /
      "random_uint32_stream_manifest.json"};
};

// =============================================================================
// =============================================================================

auto PrintUsage(char const *executable_name) -> void {
  std::cout << "Usage:\n"
            << " " << executable_name << " [options]\n\n"
            << "Options:\n"
            << "  --engine <jkiss|pcg32|philox>\n"
            << "  --seed <uint64>\n"
            << "  --particles <uint32>\n"
            << "  --words-per-particle <uint32>\n"
            << "  --local-size <size_t>\n"
            << "  --device <gpu|cpu|all|vendor token>\n"
            << "  --output <path>\n"
            << "  --manifest <path>\n"
            << "  --stream-type <raw_uint32|float_high24_bytes>\n"
            << "  --force\n"
            << "  --help\n";
}

// =============================================================================
// =============================================================================

auto ReadArgumentValue(int &index, int argc, char **argv,
                       std::string_view option_name) -> std::string {
  if (index + 1 >= argc) {
    throw std::runtime_error(
        std::format("Missing value after '{}'.", option_name));
  }

  ++index;
  return std::string{argv[index]};
}

// =============================================================================
// =============================================================================

auto ParseArguments(int argc, char **argv) -> Options {
  Options options;

  for (int i = 1; i < argc; ++i) {
    std::string_view arg{argv[i]};

    if (arg == "--help" || arg == "-h") {
      PrintUsage(argv[0]);
      std::exit(EXIT_SUCCESS);
    }

    if (arg == "--engine") {
      options.engine = ReadArgumentValue(i, argc, argv, arg);
    } else if (arg == "--seed") {
      options.seed = std::stoull(ReadArgumentValue(i, argc, argv, arg));
    } else if (arg == "--particles") {
      options.particle_count = static_cast<std::uint32_t>(
          std::stoul(ReadArgumentValue(i, argc, argv, arg)));
    } else if (arg == "--words-per-particle") {
      options.words_per_particle = static_cast<std::uint32_t>(
          std::stoul(ReadArgumentValue(i, argc, argv, arg)));
    } else if (arg == "--local-size") {
      options.local_size = static_cast<std::size_t>(
          std::stoull(ReadArgumentValue(i, argc, argv, arg)));
    } else if (arg == "--device") {
      options.device_selector = ReadArgumentValue(i, argc, argv, arg);
    } else if (arg == "--output") {
      options.output_path = ReadArgumentValue(i, argc, argv, arg);
    } else if (arg == "--manifest") {
      options.manifest_path = ReadArgumentValue(i, argc, argv, arg);
    } else if (arg == "--force") {
      options.force = true;
    } else if (arg == "--stream-type") {
      options.stream_type =
          ParseStreamType(ReadArgumentValue(i, argc, argv, arg));
    } else {
      throw std::runtime_error(std::format("Unknown argument '{}'.", arg));
    }
  }

  if (options.particle_count == 0U) {
    throw std::runtime_error("Particle count must be greater than zero.");
  }

  if (options.words_per_particle == 0U) {
    throw std::runtime_error("Words per particle must be greater than zero.");
  }

  if (options.local_size == 0U) {
    throw std::runtime_error("Local size must be greater than zero.");
  }

  return options;
}

// =============================================================================
// =============================================================================

auto EnsureOutputCanBeWritten(std::filesystem::path const &path, bool force,
                              std::string_view label) -> void {
  if (std::filesystem::exists(path) && !force) {
    throw std::runtime_error(std::format(
        "{} file already exists: '{}'. Use --force to overwrite it.", label,
        path.string()));
  }

  if (!path.parent_path().empty()) {
    std::filesystem::create_directories(path.parent_path());
  }
}

// =============================================================================
// =============================================================================

auto WriteUInt32Binary(std::filesystem::path const &path,
                       std::uint32_t const *values, std::size_t value_count,
                       bool force) -> void {
  EnsureOutputCanBeWritten(path, force, "Stream");

  if (!path.parent_path().empty()) {
    std::filesystem::create_directories(path.parent_path());
  }

  std::ofstream stream{path, std::ios::binary};

  if (!stream) {
    throw std::runtime_error(
        std::format("Cannot open output stream '{}'.", path.string()));
  }

  stream.write(
      reinterpret_cast<char const *>(values),
      static_cast<std::streamsize>(value_count * sizeof(std::uint32_t)));

  if (!stream) {
    throw std::runtime_error(
        std::format("Failed to write output stream '{}'.", path.string()));
  }
}

// =============================================================================
// =============================================================================

auto WriteFloatHigh24Bytes(std::filesystem::path const &path,
                           std::uint32_t const *values, std::size_t value_count,
                           bool force) -> void {
  EnsureOutputCanBeWritten(path, force, "Stream");

  std::ofstream stream{path, std::ios::binary};

  if (!stream) {
    throw std::runtime_error(
        std::format("Cannot open output stream '{}'.", path.string()));
  }

  constexpr std::size_t k_chunk_value_count = 1U << 20U;

  std::vector<char> buffer;
  buffer.resize(k_chunk_value_count * 3U);

  for (std::size_t offset = 0U; offset < value_count;
       offset += k_chunk_value_count) {
    std::size_t current_count =
        std::min(k_chunk_value_count, value_count - offset);

    for (std::size_t i = 0U; i < current_count; ++i) {
      std::uint32_t useful_bits = values[offset + i] >> 8U;

      buffer[(3U * i) + 0U] = static_cast<char>(useful_bits & 0xFFU);
      buffer[(3U * i) + 1U] = static_cast<char>((useful_bits >> 8U) & 0xFFU);
      buffer[(3U * i) + 2U] = static_cast<char>((useful_bits >> 16U) & 0xFFU);
    }

    stream.write(buffer.data(),
                 static_cast<std::streamsize>(current_count * 3U));

    if (!stream) {
      throw std::runtime_error(
          std::format("Failed to write output stream '{}'.", path.string()));
    }
  }
}

// =============================================================================
// =============================================================================

void WriteRandomStream(std::filesystem::path const &path,
                       StreamType stream_type, std::uint32_t const *values,
                       std::size_t value_count, bool force) {
  switch (stream_type) {
  case StreamType::RawUInt32:
    WriteUInt32Binary(path, values, value_count, force);
    return;

  case StreamType::FloatHigh24Bytes:
    WriteFloatHigh24Bytes(path, values, value_count, force);
    return;
  }

  throw std::runtime_error("Unsupported stream type");
}

// =============================================================================
// =============================================================================

auto JsonEscape(std::string_view text) -> std::string {
  std::string escaped;
  escaped.reserve(text.size());

  for (char letter : text) {
    switch (letter) {
    case '\\':
      escaped += "\\\\";
      break;
    case '"':
      escaped += "\\\"";
      break;
    case '\n':
      escaped += "\\n";
      break;
    case '\r':
      escaped += "\\r";
      break;
    case '\t':
      escaped += "\\t";
      break;
    default:
      escaped += letter;
      break;
    }
  }

  return escaped;
}

// =============================================================================
// =============================================================================

auto WriteMinimalManifest(std::filesystem::path const &path,
                          Options const &options, GGEMSRandom const &random,
                          ggems::ocl::GGEMSOpenCLDevice const &device,
                          std::uint64_t total_words, std::uint64_t output_bytes,
                          std::string const &sha256) -> void {
  if (!path.parent_path().empty()) {
    std::filesystem::create_directories(path.parent_path());
  }

  std::ofstream stream{path};

  if (!stream) {
    throw std::runtime_error(
        std::format("Cannot open manifest file '{}'.", path.string()));
  }

  stream << "{\n";
  stream << "  \"schema_version\": 1,\n";
  stream << "  \"random\": {\n";
  stream << R"(    "engine": ")" << random.GetEngineName() << "\",\n";
  stream << "    \"seed\": " << random.GetSeed() << ",\n";
  stream << "    \"particle_count\": " << options.particle_count << ",\n";
  stream << "    \"words_per_particle\": " << options.words_per_particle
         << ",\n";
  stream << "    \"total_words\": " << total_words << ",\n";
  stream << "    \"byte_count\": " << output_bytes << ",\n";
  stream << R"(    "stream_type": ")" << ToString(options.stream_type)
         << "\",\n";
  stream << R"(    "practrand_input_mode": ")"
         << ToPractRandInputMode(options.stream_type) << "\"\n";
  stream << "  },\n";
  stream << "  \"opencl\": {\n";
  stream << R"(    "device_selector": ")" << JsonEscape(options.device_selector)
         << "\",\n";
  stream << R"(    "device_name": ")" << JsonEscape(device.GetName())
         << "\",\n";
  stream << R"(    "device_vendor": ")" << JsonEscape(device.GetVendor())
         << "\",\n";
  stream << R"(    "device_version": ")" << JsonEscape(device.GetVersion())
         << "\",\n";
  stream << R"(    "driver_version": ")"
         << JsonEscape(device.GetDriverVersion()) << "\",\n";
  stream << "    \"local_size\": " << options.local_size << "\n";
  stream << "  },\n";
  stream << "  \"output\": {\n";
  stream << R"(    "stream_path": ")" << options.output_path.generic_string()
         << "\"\n";
  stream << "  },\n";
  stream << "  \"integrity\": {\n";
  stream << "    \"algorithm\": \"SHA-256\",\n";
  stream << R"(    "value": ")" << sha256 << "\"\n";
  stream << "  }\n";
  stream << "}\n";

  if (!stream) {
    throw std::runtime_error(
        std::format("Failed to write manifest file '{}'.", path.string()));
  }
}

// =============================================================================
// =============================================================================

auto GenerateRandomStream(Options const &options) -> void {
  GGEMSRandom random;
  random.SetEngine(options.engine);
  random.SetSeed(options.seed);

  std::uint64_t total_words =
      static_cast<std::uint64_t>(options.particle_count) *
      static_cast<std::uint64_t>(options.words_per_particle);

  if (total_words >
      static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max())) {
    throw std::runtime_error("Requested stream is too large for this host.");
  }

  std::uint64_t state_bytes =
      static_cast<std::uint64_t>(options.particle_count) *
      static_cast<std::uint64_t>(random.GetStateSize());

  std::uint64_t value_buffer_bytes =
      total_words * static_cast<std::uint64_t>(sizeof(std::uint32_t));

  std::uint64_t output_bytes =
      GetOutputByteCount(options.stream_type, total_words);

  std::cout << "GGEMS random stream generator\n";
  std::cout << "Engine             : " << random.GetEngineName() << '\n';
  std::cout << "Seed               : " << random.GetSeed() << '\n';
  std::cout << "Particle count     : " << options.particle_count << '\n';
  std::cout << "Words per particle : " << options.words_per_particle << '\n';
  std::cout << "Total words        : " << total_words << '\n';

  auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();

  opencl.SelectDevices({options.device_selector});
  opencl.Initialize();

  if (opencl.GetContext().empty()) {
    throw std::runtime_error("No GGEMS OpenCL context available.");
  }

  auto &context = opencl.GetContext().front();
  auto const &device = context.GetDevice();

  std::filesystem::path kernel_root{GGEMS_KERNEL_ROOT};
  std::filesystem::path validation_kernel_root{
      GGEMS_VALIDATION_RANDOM_KERNEL_ROOT};

  std::string build_options =
      std::format("-cl-std=CL2.0 -I\"{}\" {}", kernel_root.generic_string(),
                  random.GetKernelBuildDefinition());

  auto &program = opencl.GetOrCreateProgram(
      context, validation_kernel_root, "random_uint32_stream", build_options);

  cl::Kernel raw_kernel = program.CreateKernel("random_uint32_stream");

  ggems::ocl::GGEMSOpenCLKernel kernel{context, std::move(raw_kernel),
                                       "random_uint32_stream"};

  auto states_buffer =
      context.CreateSVMBuffer(ggems::units::Bytes{state_bytes});
  auto values_buffer =
      context.CreateSVMBuffer(ggems::units::Bytes{value_buffer_bytes});

  states_buffer.Map(CL_MAP_WRITE);

  auto state_storage =
      std::span<std::byte>{static_cast<std::byte *>(states_buffer.GetData()),
                           static_cast<std::size_t>(state_bytes)};
  random.InitializeStates(0ULL, state_storage);

  states_buffer.Unmap();

  values_buffer.Map(CL_MAP_WRITE);
  auto *values = static_cast<std::uint32_t *>(values_buffer.GetData());
  std::fill(values, values + static_cast<std::size_t>(total_words), 0U);
  values_buffer.Unmap();

  kernel.SetArgSVMPointer(0U, states_buffer.GetData());
  kernel.SetArgSVMPointer(1U, values_buffer.GetData());
  kernel.SetArg(2U, static_cast<cl_uint>(options.particle_count));
  kernel.SetArg(3U, static_cast<cl_uint>(options.words_per_particle));

  auto const padded_global_work_size =
      ggems::ocl::detail::TryComputePaddedGlobalWorkSize(
          static_cast<std::size_t>(options.particle_count), options.local_size);
  if (!padded_global_work_size.has_value()) {
    throw std::runtime_error(
        "Unable to compute the padded OpenCL global work size.");
  }
  std::size_t const global_size = *padded_global_work_size;

  kernel.Run({global_size}, {options.local_size});

  values_buffer.Map(CL_MAP_READ);

  WriteRandomStream(options.output_path, options.stream_type,
                    static_cast<std::uint32_t const *>(values_buffer.GetData()),
                    static_cast<std::size_t>(total_words), options.force);

  values_buffer.Unmap();

  std::string sha256{"manual"};

  WriteMinimalManifest(options.manifest_path, options, random, device,
                       total_words, output_bytes, sha256);

  std::cout << "Manifest generated : " << options.manifest_path.string()
            << '\n';
  std::cout << "Stream generated   : " << options.output_path.string() << '\n';
  std::cout << "Bytes written      : " << output_bytes << '\n';

  opencl.Clean();
}

} // namespace

// =============================================================================
// =============================================================================

auto main(int argc, char **argv) -> int {
  try {
    Options options = ParseArguments(argc, argv);
    GenerateRandomStream(options);
  } catch (std::exception const &e) {
    std::cerr << "GGEMS random stream generation failed:\n" << e.what() << '\n';
  }
  return EXIT_SUCCESS;
}
