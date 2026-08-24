#include <cstdint>
#include <cstddef>
#include <string>
#include <filesystem>
#include <iostream>
#include <string_view>
#include <format>
#include <cstdlib>
#include <stdexcept>
#include <utility>
#include <exception>
#include <limits>
#include <fstream>
#include <span>
#include <algorithm>
#include <vector>

#include "GGEMS/random/GGEMSRandom.hh"
#include "GGEMS/opencl/GGEMSOpenCLLaunchGeometry.hh"
#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/opencl/GGEMSOpenCLKernel.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMBuffer.hh"
#include "GGEMS/units/GGEMSBytesUnits.hh"

namespace {

// =============================================================================
// =============================================================================

using ggems::core::random::GGEMSRandom;

// =============================================================================
// =============================================================================

enum class StreamLayout : std::uint8_t { WorkerMajor = 0U, Interleaved = 1U };

// =============================================================================
// =============================================================================

auto ToString(StreamLayout layout) -> std::string_view {
  switch (layout) {
  case StreamLayout::WorkerMajor:
    return "worker_major";
  case StreamLayout::Interleaved:
    return "interleaved";
  }

  throw std::runtime_error("Unsupported random stream layout.");
}

// =============================================================================
// =============================================================================

auto ParseStreamLayout(std::string_view value) -> StreamLayout {
  if (value == "worker_major") {
    return StreamLayout::WorkerMajor;
  }

  if (value == "interleaved") {
    return StreamLayout::Interleaved;
  }

  throw std::runtime_error(
      std::format("Unsupported random stream layout '{}'.", value));
}

// =============================================================================
// =============================================================================

enum class StreamType : std::uint8_t {
  RawUInt32 = 0U,
  Uniform24Scalar,
  Uniform24Vector4
};

// =============================================================================
// =============================================================================

auto ToString(StreamType stream_type) -> std::string_view {
  switch (stream_type) {
  case StreamType::RawUInt32:
    return "raw_uint32";
  case StreamType::Uniform24Scalar:
    return "uniform24_scalar";
  case StreamType::Uniform24Vector4:
    return "uniform24_vector4";
  }

  throw std::runtime_error("Unsupported random stream type.");
}

// =============================================================================
// =============================================================================

auto ParseStreamType(std::string_view value) -> StreamType {
  if (value == "raw_uint32") {
    return StreamType::RawUInt32;
  }

  if (value == "uniform24_scalar") {
    return StreamType::Uniform24Scalar;
  }

  if (value == "uniform24_vector4") {
    return StreamType::Uniform24Vector4;
  }

  throw std::runtime_error(
      std::format("Unsupported random stream type '{}'.", value));
}

// =============================================================================
// =============================================================================

struct Options {
  std::string engine{"philox"};
  std::uint64_t seed{77777ULL};
  std::uint64_t stream_offset{0ULL};
  std::uint32_t worker_count{1024U};
  std::uint32_t samples_per_worker{1024U};
  std::size_t local_size{64U};
  std::string device_selector{"cpu"};
  bool force{false};
  StreamLayout layout{StreamLayout::WorkerMajor};
  StreamType stream_type{StreamType::RawUInt32};

  std::filesystem::path output_path{"random_uint32_stream.bin"};
  std::filesystem::path manifest_path{"random_uint32_stream_manifest.json"};
};

// =============================================================================
// =============================================================================

auto PrintUsage(char const *executable_name) -> void {
  std::cout
      << "Usage:\n"
      << " " << executable_name << " [options]\n\n"
      << "Options:\n"
      << "  --engine <jkiss|pcg32|philox>\n"
      << "  --seed <uint64>\n"
      << "  --stream-offset <uint64>\n"
      << "  --stream-type <raw_uint32|uniform24_scalar|uniform24_vector4>\n"
      << "  --workers <uint32>\n"
      << "  --samples-per-worker <uint32>\n"
      << "  --local-size <size_t>\n"
      << "  --device <gpu|cpu|all|vendor token>\n"
      << "  --output <path>\n"
      << "  --manifest <path>\n"
      << "  --layout <worker_major|interleaved>\n"
      << "  --force\n"
      << "  --help\n";
}

// =============================================================================
// =============================================================================

auto ReadArgumentValue(int &index, int argc, char const *const *argv,
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

auto ParseArguments(int argc, char const *const *argv) -> Options {
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
    } else if (arg == "--stream-offset") {
      options.stream_offset =
          std::stoull(ReadArgumentValue(i, argc, argv, arg));
    } else if (arg == "--stream-type") {
      options.stream_type =
          ParseStreamType(ReadArgumentValue(i, argc, argv, arg));
    } else if (arg == "--workers") {
      options.worker_count = static_cast<std::uint32_t>(
          std::stoul(ReadArgumentValue(i, argc, argv, arg)));
    } else if (arg == "--samples-per-worker") {
      options.samples_per_worker = static_cast<std::uint32_t>(
          std::stoul(ReadArgumentValue(i, argc, argv, arg)));
    } else if (arg == "--local-size") {
      options.local_size = static_cast<std::size_t>(
          std::stoull(ReadArgumentValue(i, argc, argv, arg)));
    } else if (arg == "--device") {
      options.device_selector = ReadArgumentValue(i, argc, argv, arg);
    } else if (arg == "--output") {
      options.output_path = ReadArgumentValue(i, argc, argv, arg);
    } else if (arg == "--layout") {
      options.layout = ParseStreamLayout(ReadArgumentValue(i, argc, argv, arg));
    } else if (arg == "--manifest") {
      options.manifest_path = ReadArgumentValue(i, argc, argv, arg);
    } else if (arg == "--force") {
      options.force = true;
    } else {
      throw std::runtime_error(std::format("Unknown argument '{}'.", arg));
    }
  }

  if (options.worker_count == 0U) {
    throw std::runtime_error("Worker count must be greater than zero.");
  }

  if (options.samples_per_worker == 0U) {
    throw std::runtime_error("Samples per worker must be greater than zero.");
  }

  if (options.stream_type == StreamType::Uniform24Vector4 &&
      (options.samples_per_worker % 4U) != 0U) {
    throw std::runtime_error(
        "Samples per worker must be a multiple of four for uniform24_vector4.");
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

auto WriteUniform24Binary(std::filesystem::path const &path,
                          float const *values, std::size_t value_count,
                          bool force) -> void {
  EnsureOutputCanBeWritten(path, force, "Stream");

  std::ofstream stream{path, std::ios::binary};

  if (!stream) {
    throw std::runtime_error(
        std::format("Cannot open output stream '{}'.", path.string()));
  }

  constexpr std::size_t kChunkSampleCount = 1U << 20U;
  std::vector<std::uint8_t> buffer(kChunkSampleCount * 3U);

  for (std::size_t offset = 0U; offset < value_count;
       offset += kChunkSampleCount) {
    std::size_t const sample_count =
        std::min(kChunkSampleCount, value_count - offset);

    for (std::size_t i = 0U; i < sample_count; ++i) {
      float const uniform = values[offset + i];

      auto const value = static_cast<std::uint32_t>(uniform * 16777216.0F);

      if (value > 0x00FFFFFFU) {
        throw std::runtime_error(
            "Invalid GGEMS uniform value while packing 24-bit stream.");
      }

      buffer[(3U * i) + 0U] = static_cast<std::uint8_t>(value & 0xFFU);

      buffer[(3U * i) + 1U] = static_cast<std::uint8_t>((value >> 8U) & 0xFFU);

      buffer[(3U * i) + 2U] = static_cast<std::uint8_t>((value >> 16U) & 0xFFU);
    }

    stream.write(reinterpret_cast<char const *>(buffer.data()),
                 static_cast<std::streamsize>(sample_count * 3U));

    if (!stream) {
      throw std::runtime_error(
          std::format("Failed to write output stream '{}'.", path.string()));
    }
  }
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
                          std::uint64_t total_samples,
                          std::uint64_t output_bytes) -> void {
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
  stream << "    \"stream_offset\": " << options.stream_offset << ",\n";
  stream << R"(    "stream_type": ")" << ToString(options.stream_type)
         << "\",\n";
  switch (options.stream_type) {
  case StreamType::RawUInt32:
    stream << "    \"sample_bits\": 32,\n";
    break;

  case StreamType::Uniform24Scalar:
  case StreamType::Uniform24Vector4:
    stream << "    \"sample_bits\": 24,\n";
    break;
  }
  stream << R"(    "layout": ")" << ToString(options.layout) << "\",\n";
  stream << "    \"worker_count\": " << options.worker_count << ",\n";
  stream << "    \"samples_per_worker\": " << options.samples_per_worker
         << ",\n";
  stream << "    \"total_samples\": " << total_samples << ",\n";
  stream << "    \"byte_count\": " << output_bytes << "\n";
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

  std::uint64_t total_samples =
      static_cast<std::uint64_t>(options.worker_count) *
      static_cast<std::uint64_t>(options.samples_per_worker);

  if (total_samples >
      static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max())) {
    throw std::runtime_error("Requested stream is too large for this host.");
  }

  std::uint64_t state_bytes = static_cast<std::uint64_t>(options.worker_count) *
                              static_cast<std::uint64_t>(random.GetStateSize());

  std::uint64_t value_buffer_bytes =
      total_samples * static_cast<std::uint64_t>(sizeof(std::uint32_t));

  std::uint64_t output_bytes = 0ULL;

  switch (options.stream_type) {
  case StreamType::RawUInt32:
    output_bytes =
        total_samples * static_cast<std::uint64_t>(sizeof(std::uint32_t));
    break;

  case StreamType::Uniform24Scalar:
  case StreamType::Uniform24Vector4:
    output_bytes = total_samples * 3ULL;
    break;
  }

  std::cout << "GGEMS random stream generator\n";
  std::cout << "Engine             : " << random.GetEngineName() << '\n';
  std::cout << "Seed               : " << random.GetSeed() << '\n';
  std::cout << "Stream offset      : " << options.stream_offset << '\n';
  std::cout << "Stream type        : " << ToString(options.stream_type) << '\n';
  std::cout << "Worker count       : " << options.worker_count << '\n';
  std::cout << "Samples per worker : " << options.samples_per_worker << '\n';
  std::cout << "Total samples      : " << total_samples << '\n';
  std::cout << "Layout             : " << ToString(options.layout) << '\n';

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

  std::string_view kernel_name;

  switch (options.stream_type) {
  case StreamType::RawUInt32:
    kernel_name = "random_uint32_stream";
    break;

  case StreamType::Uniform24Scalar:
    kernel_name = "random_uniform24_scalar_stream";
    break;

  case StreamType::Uniform24Vector4:
    kernel_name = "random_uniform24_vector4_stream";
    break;
  }

  auto const &program = opencl.GetOrCreateProgram(
      context, validation_kernel_root, std::string{kernel_name}, build_options);

  cl::Kernel raw_kernel = program.CreateKernel(std::string{kernel_name});

  ggems::ocl::GGEMSOpenCLKernel kernel{context, std::move(raw_kernel),
                                       std::string{kernel_name}};

  auto states_buffer =
      context.CreateSVMBuffer(ggems::units::Bytes{state_bytes});
  auto values_buffer =
      context.CreateSVMBuffer(ggems::units::Bytes{value_buffer_bytes});

  states_buffer.Map(CL_MAP_WRITE);

  auto state_storage =
      std::span<std::byte>{static_cast<std::byte *>(states_buffer.GetData()),
                           static_cast<std::size_t>(state_bytes)};
  random.InitializeStates(options.stream_offset, state_storage);

  states_buffer.Unmap();

  values_buffer.Map(CL_MAP_WRITE);
  auto *values = static_cast<std::uint32_t *>(values_buffer.GetData());
  std::fill(values, values + static_cast<std::size_t>(total_samples), 0U);
  values_buffer.Unmap();

  kernel.SetArgSVMPointer(0U, states_buffer.GetData());
  kernel.SetArgSVMPointer(1U, values_buffer.GetData());
  kernel.SetArg(2U, static_cast<cl_uint>(options.worker_count));
  kernel.SetArg(3U, static_cast<cl_uint>(options.samples_per_worker));
  kernel.SetArg(4U, static_cast<cl_uint>(options.layout));

  auto const padded_global_work_size =
      ggems::ocl::detail::TryComputePaddedGlobalWorkSize(
          static_cast<std::size_t>(options.worker_count), options.local_size);
  if (!padded_global_work_size.has_value()) {
    throw std::runtime_error(
        "Unable to compute the padded OpenCL global work size.");
  }
  std::size_t const global_size = *padded_global_work_size;

  kernel.Run({global_size}, {options.local_size});

  values_buffer.Map(CL_MAP_READ);

  switch (options.stream_type) {
  case StreamType::RawUInt32:
    WriteUInt32Binary(
        options.output_path,
        static_cast<std::uint32_t const *>(values_buffer.GetData()),
        static_cast<std::size_t>(total_samples), options.force);
    break;

  case StreamType::Uniform24Scalar:
  case StreamType::Uniform24Vector4:
    WriteUniform24Binary(options.output_path,
                         static_cast<float const *>(values_buffer.GetData()),
                         static_cast<std::size_t>(total_samples),
                         options.force);
    break;
  }

  values_buffer.Unmap();

  WriteMinimalManifest(options.manifest_path, options, random, device,
                       total_samples, output_bytes);

  std::cout << "Manifest generated : " << options.manifest_path.string()
            << '\n';
  std::cout << "Stream generated   : " << options.output_path.string() << '\n';
  std::cout << "Bytes written      : " << output_bytes << '\n';
}

} // namespace

// =============================================================================
// =============================================================================

auto main(int argc, char **argv) -> int {
  try {
    Options options = ParseArguments(argc, argv);
    GenerateRandomStream(options);

    return EXIT_SUCCESS;
  } catch (std::exception const &e) {
    std::cerr << "GGEMS random stream generation failed:\n" << e.what() << '\n';

    return EXIT_FAILURE;
  }
}
