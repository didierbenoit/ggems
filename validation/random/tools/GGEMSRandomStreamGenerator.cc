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

#include "GGEMS/core/random/GGEMSRandom.hh"
#include "GGEMS/core/random/GGEMSRandomEngine.hh"
#include "GGEMS/core/random/GGEMSRandomState.hh"
#include "GGEMS/frameworks/GGEMSOpenCL.hh"
#include "GGEMS/frameworks/GGEMSOpenCLKernel.hh"
#include "GGEMS/frameworks/GGEMSOpenCLSVMBuffer.hh"

namespace {
using ggems::core::random::GGEMSJKissState;
using ggems::core::random::GGEMSPCG32State;
using ggems::core::random::GGEMSPhiloxState;
using ggems::core::random::GGEMSRandom;
using ggems::core::random::GGEMSRandomEngine;

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

enum class StreamType { RawUInt32, FloatHigh24Bytes };

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::string ToString(StreamType stream_type) {
  switch (stream_type) {
  case StreamType::RawUInt32:
    return "raw_uint32";
  case StreamType::FloatHigh24Bytes:
    return "float_high24_bytes";
  }

  throw std::runtime_error("Unsupported stream type.");
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::string ToPractRandInputMode(StreamType stream_type) {
  switch (stream_type) {
  case StreamType::RawUInt32:
    return "stdin32";
  case StreamType::FloatHigh24Bytes:
    return "stdin8";
  }

  throw std::runtime_error("Unsupported stream type.");
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

StreamType ParseStreamType(std::string_view value) {
  if (value == "raw_uint32") {
    return StreamType::RawUInt32;
  }

  if (value == "float_high24_bytes") {
    return StreamType::FloatHigh24Bytes;
  }

  throw std::runtime_error(std::format("Unsupported stream type '{}'.", value));
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::uint64_t GetOutputByteCount(StreamType stream_type,
                                 std::uint64_t total_words) {
  switch (stream_type) {
  case StreamType::RawUInt32:
    return total_words * 4ULL;
  case StreamType::FloatHigh24Bytes:
    return total_words * 3ULL;
  }

  throw std::runtime_error("Unsupported stream type.");
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

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

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void PrintUsage(char const *executable_name) {
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

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::string ReadArgumentValue(int &index, int argc, char **argv,
                              std::string_view option_name) {
  if (index + 1 >= argc) {
    throw std::runtime_error(
        std::format("Missing value after '{}'.", option_name));
  }

  ++index;
  return std::string{argv[index]};
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

Options ParseArguments(int argc, char **argv) {
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

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::size_t RoundUp(std::size_t value, std::size_t multiple) noexcept {
  return ((value + multiple - 1U) / multiple) * multiple;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::uint64_t SplitMix64(std::uint64_t value) noexcept {
  value += 0x9E3779B97F4A7C15ULL;

  value = (value ^ (value >> 30U)) * 0xBF58476D1CE4E5B9ULL;
  value = (value ^ (value >> 27U)) * 0x94D049BB133111EBULL;

  return value ^ (value >> 31U);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSJKissState MakeJKissState(std::uint32_t seed,
                               std::uint32_t index) noexcept {
  return GGEMSJKissState{.x = seed + 123456789U + 1013904223U * index,
                         .y = seed ^ (362436069U + 1664525U * index),
                         .z = seed + 521288629U + 69069U * index,
                         .w = seed ^ (88675123U + 22695477U * index),
                         .c = index & 1U};
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSPCG32State MakePCG32State(std::uint64_t seed,
                               std::uint64_t index) noexcept {
  std::uint64_t state =
      SplitMix64(seed + 0xD1B54A32D192ED03ULL * (index + 1ULL));

  std::uint64_t stream = SplitMix64(seed ^ (0xABC98388FB8FAC03ULL + index));

  return GGEMSPCG32State{.state = state, .increment = stream | 1ULL};
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSPhiloxState MakePhiloxState(std::uint64_t seed,
                                 std::uint64_t index) noexcept {
  std::uint64_t const key = SplitMix64(seed);

  return GGEMSPhiloxState{.counter_0 = 0U,
                          .counter_1 = 0U,
                          .counter_2 = static_cast<std::uint32_t>(index),
                          .counter_3 = static_cast<std::uint32_t>(index >> 32U),
                          .key_0 = static_cast<std::uint32_t>(key),
                          .key_1 = static_cast<std::uint32_t>(key >> 32U)};
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void InitialiseRandomStates(void *states_data, GGEMSRandomEngine engine,
                            std::uint64_t seed, std::uint32_t particle_count) {
  switch (engine) {
  case GGEMSRandomEngine::JKISS: {
    auto *states = static_cast<GGEMSJKissState *>(states_data);

    for (std::uint32_t i = 0U; i < particle_count; ++i) {
      states[i] = MakeJKissState(static_cast<std::uint32_t>(seed), i);
    }

    return;
  }

  case GGEMSRandomEngine::PCG32: {
    auto *states = static_cast<GGEMSPCG32State *>(states_data);

    for (std::uint32_t i = 0U; i < particle_count; ++i) {
      states[i] = MakePCG32State(seed, i);
    }

    return;
  }

  case GGEMSRandomEngine::Philox: {
    auto *states = static_cast<GGEMSPhiloxState *>(states_data);

    for (std::uint32_t i = 0U; i < particle_count; ++i) {
      states[i] = MakePhiloxState(seed, i);
    }

    return;
  }
  }

  throw std::runtime_error("Unsupported GGEMS random engine.");
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void EnsureOutputCanBeWritten(std::filesystem::path const &path, bool force,
                              std::string_view label) {
  if (std::filesystem::exists(path) && !force) {
    throw std::runtime_error(std::format(
        "{} file already exists: '{}'. Use --force to overwrite it.", label,
        path.string()));
  }

  if (!path.parent_path().empty()) {
    std::filesystem::create_directories(path.parent_path());
  }
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void WriteUInt32Binary(std::filesystem::path const &path,
                       std::uint32_t const *values, std::size_t value_count,
                       bool force) {
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

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void WriteFloatHigh24Bytes(std::filesystem::path const &path,
                           std::uint32_t const *values, std::size_t value_count,
                           bool force) {
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

      buffer[3U * i + 0U] = static_cast<char>(useful_bits & 0xFFU);
      buffer[3U * i + 1U] = static_cast<char>((useful_bits >> 8U) & 0xFFU);
      buffer[3U * i + 2U] = static_cast<char>((useful_bits >> 16U) & 0xFFU);
    }

    stream.write(buffer.data(),
                 static_cast<std::streamsize>(current_count * 3U));

    if (!stream) {
      throw std::runtime_error(
          std::format("Failed to write output stream '{}'.", path.string()));
    }
  }
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

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

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::string JsonEscape(std::string_view text) {
  std::string escaped;
  escaped.reserve(text.size());

  for (char c : text) {
    switch (c) {
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
      escaped += c;
      break;
    }
  }

  return escaped;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void WriteMinimalManifest(std::filesystem::path const &path,
                          Options const &options, GGEMSRandom const &random,
                          ggems::ocl::GGEMSOpenCLDevice const &device,
                          std::uint64_t total_words, std::uint64_t output_bytes,
                          std::string const &sha256) {
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
  stream << "    \"engine\": \"" << random.GetEngineName() << "\",\n";
  stream << "    \"seed\": " << random.GetSeed() << ",\n";
  stream << "    \"particle_count\": " << options.particle_count << ",\n";
  stream << "    \"words_per_particle\": " << options.words_per_particle
         << ",\n";
  stream << "    \"total_words\": " << total_words << ",\n";
  stream << "    \"byte_count\": " << output_bytes << ",\n";
  stream << "    \"stream_type\": \"" << ToString(options.stream_type)
         << "\",\n";
  stream << "    \"practrand_input_mode\": \""
         << ToPractRandInputMode(options.stream_type) << "\"\n";
  stream << "  },\n";
  stream << "  \"opencl\": {\n";
  stream << "    \"device_selector\": \"" << JsonEscape(options.device_selector)
         << "\",\n";
  stream << "    \"device_name\": \"" << JsonEscape(device.GetName())
         << "\",\n";
  stream << "    \"device_vendor\": \"" << JsonEscape(device.GetVendor())
         << "\",\n";
  stream << "    \"device_version\": \"" << JsonEscape(device.GetVersion())
         << "\",\n";
  stream << "    \"driver_version\": \""
         << JsonEscape(device.GetDriverVersion()) << "\",\n";
  stream << "    \"local_size\": " << options.local_size << "\n";
  stream << "  },\n";
  stream << "  \"output\": {\n";
  stream << "    \"stream_path\": \"" << options.output_path.generic_string()
         << "\"\n";
  stream << "  },\n";
  stream << "  \"integrity\": {\n";
  stream << "    \"algorithm\": \"SHA-256\",\n";
  stream << "    \"value\": \"" << sha256 << "\"\n";
  stream << "  }\n";
  stream << "}\n";

  if (!stream) {
    throw std::runtime_error(
        std::format("Failed to write manifest file '{}'.", path.string()));
  }
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GenerateRandomStream(Options const &options) {
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
  opencl.Initialise();

  if (opencl.GetContext().empty()) {
    throw std::runtime_error("No GGEMS OpenCL context available.");
  }

  auto &context = opencl.GetContext().front();
  auto &device = context.GetDevice();

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
  InitialiseRandomStates(states_buffer.GetData(), random.GetEngine(),
                         random.GetSeed(), options.particle_count);
  states_buffer.Unmap();

  values_buffer.Map(CL_MAP_WRITE);
  auto *values = static_cast<std::uint32_t *>(values_buffer.GetData());
  std::fill(values, values + static_cast<std::size_t>(total_words), 0U);
  values_buffer.Unmap();

  kernel.SetArgSVMPointer(0U, states_buffer.GetData());
  kernel.SetArgSVMPointer(1U, values_buffer.GetData());
  kernel.SetArg(2U, static_cast<cl_uint>(options.particle_count));
  kernel.SetArg(3U, static_cast<cl_uint>(options.words_per_particle));

  std::size_t const global_size = RoundUp(
      static_cast<std::size_t>(options.particle_count), options.local_size);

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

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

int main(int argc, char **argv) {
  try {
    Options options = ParseArguments(argc, argv);
    GenerateRandomStream(options);
  } catch (std::exception const &e) {
    std::cerr << "GGEMS random stream generation failed:\n" << e.what() << '\n';
  }
  return EXIT_SUCCESS;
}
