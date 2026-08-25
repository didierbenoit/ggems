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

constexpr std::uint64_t kBytesPerMiB{1024ULL * 1024ULL};
constexpr std::uint64_t kDefaultMaxChunkMiB{256ULL};
constexpr std::uint64_t kSVMValueBytesPerSample{
    static_cast<std::uint64_t>(sizeof(std::uint32_t))};

static_assert(sizeof(float) == sizeof(std::uint32_t));

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
  std::uint32_t lanes_used{0U};
  std::uint64_t max_chunk_mib{kDefaultMaxChunkMiB};
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
      << "  --lanes-used <1|2|3|4>\n"
      << "  --workers <uint32>\n"
      << "  --samples-per-worker <uint32>\n"
      << "  --max-chunk-mib <uint64>\n"
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
    } else if (arg == "--lanes-used") {
      options.lanes_used = static_cast<std::uint32_t>(
          std::stoul(ReadArgumentValue(i, argc, argv, arg)));
    } else if (arg == "--workers") {
      options.worker_count = static_cast<std::uint32_t>(
          std::stoul(ReadArgumentValue(i, argc, argv, arg)));
    } else if (arg == "--samples-per-worker") {
      options.samples_per_worker = static_cast<std::uint32_t>(
          std::stoul(ReadArgumentValue(i, argc, argv, arg)));
    } else if (arg == "--max-chunk-mib") {
      options.max_chunk_mib =
          std::stoull(ReadArgumentValue(i, argc, argv, arg));
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

  if (options.stream_type == StreamType::Uniform24Vector4) {
    if (options.lanes_used == 0U) {
      options.lanes_used = 4U;
    }

    if (options.lanes_used > 4U) {
      throw std::runtime_error(
          "Lanes used must be between one and four for uniform24_vector4.");
    }

    if ((options.samples_per_worker % options.lanes_used) != 0U) {
      throw std::runtime_error(
          "Samples per worker must be a multiple of lanes used for "
          "uniform24_vector4.");
    }
  } else if (options.lanes_used != 0U) {
    throw std::runtime_error(
        "--lanes-used is only valid with uniform24_vector4.");
  }

  if (options.max_chunk_mib == 0ULL) {
    throw std::runtime_error("Maximum chunk size must be greater than zero.");
  }

  if (options.max_chunk_mib >
      std::numeric_limits<std::uint64_t>::max() / kBytesPerMiB) {
    throw std::runtime_error("Maximum chunk size is too large.");
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

auto OpenOutputStream(std::filesystem::path const &path, bool force)
    -> std::ofstream {
  EnsureOutputCanBeWritten(path, force, "Stream");

  std::ofstream stream{path, std::ios::binary | std::ios::trunc};

  if (!stream) {
    throw std::runtime_error(
        std::format("Cannot open output stream '{}'.", path.string()));
  }

  return stream;
}

// =============================================================================
// =============================================================================

auto WriteUInt32Chunk(std::ofstream &stream,
                      std::filesystem::path const &path,
                      std::uint32_t const *values,
                      std::size_t value_count) -> void {
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

auto WriteUniform24Chunk(std::ofstream &stream,
                         std::filesystem::path const &path,
                         float const *values,
                         std::size_t value_count) -> void {
  constexpr std::size_t kPackingChunkSampleCount = 1U << 20U;
  std::vector<std::uint8_t> buffer(kPackingChunkSampleCount * 3U);

  for (std::size_t offset = 0U; offset < value_count;
       offset += kPackingChunkSampleCount) {
    std::size_t const sample_count =
        std::min(kPackingChunkSampleCount, value_count - offset);

    for (std::size_t i = 0U; i < sample_count; ++i) {
      float const uniform = values[offset + i];

      auto const value = static_cast<std::uint32_t>(uniform * 16777216.0F);

      if (value > 0x00FFFFFFU) {
        throw std::runtime_error(
            "Invalid GGEMS uniform value while packing 24-bit stream.");
      }

      buffer[(3U * i) + 0U] = static_cast<std::uint8_t>(value & 0xFFU);

      buffer[(3U * i) + 1U] =
          static_cast<std::uint8_t>((value >> 8U) & 0xFFU);

      buffer[(3U * i) + 2U] =
          static_cast<std::uint8_t>((value >> 16U) & 0xFFU);
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

enum class ChunkStrategy : std::uint8_t {
  SampleDepth = 0U,
  WorkerGroups
};

// =============================================================================
// =============================================================================

auto ToString(ChunkStrategy strategy) -> std::string_view {
  switch (strategy) {
  case ChunkStrategy::SampleDepth:
    return "sample_depth";
  case ChunkStrategy::WorkerGroups:
    return "worker_groups";
  }

  throw std::runtime_error("Unsupported random stream chunk strategy.");
}

// =============================================================================
// =============================================================================

struct ChunkPlan {
  ChunkStrategy strategy{ChunkStrategy::SampleDepth};
  std::uint64_t requested_max_value_buffer_bytes{0ULL};
  std::uint64_t effective_max_value_buffer_bytes{0ULL};
  std::uint64_t device_max_allocation_bytes{0ULL};
  std::uint64_t state_buffer_bytes{0ULL};
  std::uint64_t value_buffer_bytes{0ULL};
  std::uint32_t workers_per_chunk{0U};
  std::uint32_t samples_per_worker_per_chunk{0U};
  std::uint64_t chunk_count{0ULL};
};

// =============================================================================
// =============================================================================

auto DivideRoundUp(std::uint64_t numerator, std::uint64_t denominator)
    -> std::uint64_t {
  return (numerator / denominator) +
         static_cast<std::uint64_t>((numerator % denominator) != 0ULL);
}

// =============================================================================
// =============================================================================

auto ComputeChunkPlan(Options const &options, GGEMSRandom const &random,
                      ggems::ocl::GGEMSOpenCLDevice const &device)
    -> ChunkPlan {
  ChunkPlan plan;

  plan.requested_max_value_buffer_bytes =
      options.max_chunk_mib * kBytesPerMiB;
  plan.device_max_allocation_bytes =
      static_cast<std::uint64_t>(device.GetMaxMemAllocSize());
  plan.effective_max_value_buffer_bytes =
      std::min(plan.requested_max_value_buffer_bytes,
               plan.device_max_allocation_bytes);

  if (plan.effective_max_value_buffer_bytes == 0ULL) {
    throw std::runtime_error(
        "OpenCL device reports a zero maximum allocation size.");
  }

  std::uint64_t const state_size =
      static_cast<std::uint64_t>(random.GetStateSize());

  if (state_size == 0ULL) {
    throw std::runtime_error("Random engine reports a zero state size.");
  }

  if (options.layout == StreamLayout::Interleaved) {
    plan.strategy = ChunkStrategy::SampleDepth;
    plan.workers_per_chunk = options.worker_count;

    std::uint64_t const state_buffer_bytes =
        static_cast<std::uint64_t>(options.worker_count) * state_size;

    if (state_buffer_bytes > plan.device_max_allocation_bytes) {
      throw std::runtime_error(
          "Random state buffer exceeds CL_DEVICE_MAX_MEM_ALLOC_SIZE.");
    }

    std::uint64_t const bytes_per_sample_round =
        static_cast<std::uint64_t>(options.worker_count) *
        kSVMValueBytesPerSample;

    std::uint64_t max_samples_per_worker =
        plan.effective_max_value_buffer_bytes / bytes_per_sample_round;

    if (options.stream_type == StreamType::Uniform24Vector4) {
      std::uint64_t const lanes_used =
          static_cast<std::uint64_t>(options.lanes_used);
      max_samples_per_worker -= max_samples_per_worker % lanes_used;
    }

    if (max_samples_per_worker == 0ULL) {
      throw std::runtime_error(
          "Maximum chunk size is too small for one interleaved sample round.");
    }

    plan.samples_per_worker_per_chunk = static_cast<std::uint32_t>(
        std::min<std::uint64_t>(options.samples_per_worker,
                                max_samples_per_worker));

    plan.state_buffer_bytes = state_buffer_bytes;
    plan.value_buffer_bytes =
        static_cast<std::uint64_t>(options.worker_count) *
        static_cast<std::uint64_t>(plan.samples_per_worker_per_chunk) *
        kSVMValueBytesPerSample;
    plan.chunk_count =
        DivideRoundUp(options.samples_per_worker,
                      plan.samples_per_worker_per_chunk);

    return plan;
  }

  plan.strategy = ChunkStrategy::WorkerGroups;
  plan.samples_per_worker_per_chunk = options.samples_per_worker;

  std::uint64_t const value_bytes_per_worker =
      static_cast<std::uint64_t>(options.samples_per_worker) *
      kSVMValueBytesPerSample;

  std::uint64_t const max_workers_by_values =
      plan.effective_max_value_buffer_bytes / value_bytes_per_worker;
  std::uint64_t const max_workers_by_states =
      plan.device_max_allocation_bytes / state_size;
  std::uint64_t const max_workers =
      std::min(max_workers_by_values, max_workers_by_states);

  if (max_workers == 0ULL) {
    throw std::runtime_error(
        "Maximum chunk size is too small for one worker-major stream.");
  }

  plan.workers_per_chunk = static_cast<std::uint32_t>(
      std::min<std::uint64_t>(options.worker_count, max_workers));
  plan.state_buffer_bytes =
      static_cast<std::uint64_t>(plan.workers_per_chunk) * state_size;
  plan.value_buffer_bytes =
      static_cast<std::uint64_t>(plan.workers_per_chunk) *
      value_bytes_per_worker;
  plan.chunk_count =
      DivideRoundUp(options.worker_count, plan.workers_per_chunk);

  return plan;
}

// =============================================================================
// =============================================================================

auto InitializeStates(GGEMSRandom const &random,
                      ggems::ocl::GGEMSOpenCLSVMBuffer &states_buffer,
                      std::uint64_t first_stream_id,
                      std::uint32_t worker_count) -> void {
  std::uint64_t const state_bytes =
      static_cast<std::uint64_t>(worker_count) *
      static_cast<std::uint64_t>(random.GetStateSize());

  if (state_bytes >
      static_cast<std::uint64_t>(std::numeric_limits<std::size_t>::max())) {
    throw std::runtime_error("Random state chunk is too large for this host.");
  }

  states_buffer.Map(CL_MAP_WRITE);

  auto state_storage =
      std::span<std::byte>{static_cast<std::byte *>(states_buffer.GetData()),
                           static_cast<std::size_t>(state_bytes)};
  random.InitializeStates(first_stream_id, state_storage);

  states_buffer.Unmap();
}

// =============================================================================
// =============================================================================

auto RunRandomKernel(ggems::ocl::GGEMSOpenCLKernel &kernel,
                     Options const &options, void *states, void *values,
                     std::uint32_t worker_count,
                     std::uint32_t samples_per_worker) -> void {
  kernel.SetArgSVMPointer(0U, states);
  kernel.SetArgSVMPointer(1U, values);
  kernel.SetArg(2U, static_cast<cl_uint>(worker_count));
  kernel.SetArg(3U, static_cast<cl_uint>(samples_per_worker));
  kernel.SetArg(4U, static_cast<cl_uint>(options.layout));

  if (options.stream_type == StreamType::Uniform24Vector4) {
    kernel.SetArg(5U, static_cast<cl_uint>(options.lanes_used));
  }

  auto const padded_global_work_size =
      ggems::ocl::detail::TryComputePaddedGlobalWorkSize(
          static_cast<std::size_t>(worker_count), options.local_size);

  if (!padded_global_work_size.has_value()) {
    throw std::runtime_error(
        "Unable to compute the padded OpenCL global work size.");
  }

  kernel.Run({*padded_global_work_size}, {options.local_size});
}

// =============================================================================
// =============================================================================

auto WriteValueChunk(std::ofstream &stream,
                     std::filesystem::path const &path,
                     StreamType stream_type, void const *values,
                     std::size_t value_count) -> void {
  switch (stream_type) {
  case StreamType::RawUInt32:
    WriteUInt32Chunk(stream, path,
                     static_cast<std::uint32_t const *>(values), value_count);
    return;

  case StreamType::Uniform24Scalar:
  case StreamType::Uniform24Vector4:
    WriteUniform24Chunk(stream, path, static_cast<float const *>(values),
                        value_count);
    return;
  }

  throw std::runtime_error("Unsupported random stream type.");
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
                          ChunkPlan const &chunk_plan,
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

  if (options.stream_type == StreamType::Uniform24Vector4) {
    stream << "    \"lanes_used\": " << options.lanes_used << ",\n";
  }

  stream << R"(    "layout": ")" << ToString(options.layout) << "\",\n";
  stream << "    \"worker_count\": " << options.worker_count << ",\n";
  stream << "    \"samples_per_worker\": " << options.samples_per_worker
         << ",\n";
  stream << "    \"total_samples\": " << total_samples << ",\n";
  stream << "    \"byte_count\": " << output_bytes << "\n";
  stream << "  },\n";
  stream << "  \"generation\": {\n";
  stream << R"(    "chunk_strategy": ")" << ToString(chunk_plan.strategy)
         << "\",\n";
  stream << "    \"requested_max_value_buffer_bytes\": "
         << chunk_plan.requested_max_value_buffer_bytes << ",\n";
  stream << "    \"effective_max_value_buffer_bytes\": "
         << chunk_plan.effective_max_value_buffer_bytes << ",\n";
  stream << "    \"device_max_allocation_bytes\": "
         << chunk_plan.device_max_allocation_bytes << ",\n";
  stream << "    \"state_buffer_bytes\": " << chunk_plan.state_buffer_bytes
         << ",\n";
  stream << "    \"value_buffer_bytes\": " << chunk_plan.value_buffer_bytes
         << ",\n";
  stream << "    \"workers_per_chunk\": " << chunk_plan.workers_per_chunk
         << ",\n";
  stream << "    \"samples_per_worker_per_chunk\": "
         << chunk_plan.samples_per_worker_per_chunk << ",\n";
  stream << "    \"chunk_count\": " << chunk_plan.chunk_count << "\n";
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

  random.ValidateStateRange(options.stream_offset, options.worker_count);

  if (static_cast<std::uint64_t>(options.worker_count) >
      std::numeric_limits<std::uint64_t>::max() /
          static_cast<std::uint64_t>(options.samples_per_worker)) {
    throw std::runtime_error("Requested stream sample count overflows uint64.");
  }

  std::uint64_t const total_samples =
      static_cast<std::uint64_t>(options.worker_count) *
      static_cast<std::uint64_t>(options.samples_per_worker);

  std::uint64_t const output_bytes_per_sample =
      options.stream_type == StreamType::RawUInt32 ? 4ULL : 3ULL;

  if (total_samples > std::numeric_limits<std::uint64_t>::max() /
                          output_bytes_per_sample) {
    throw std::runtime_error("Requested stream byte count overflows uint64.");
  }

  std::uint64_t const output_bytes =
      total_samples * output_bytes_per_sample;

  std::cout << "GGEMS random stream generator\n";
  std::cout << "Engine             : " << random.GetEngineName() << '\n';
  std::cout << "Seed               : " << random.GetSeed() << '\n';
  std::cout << "Stream offset      : " << options.stream_offset << '\n';
  std::cout << "Stream type        : " << ToString(options.stream_type) << '\n';

  if (options.stream_type == StreamType::Uniform24Vector4) {
    std::cout << "Lanes used         : " << options.lanes_used << '\n';
  }

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
  ChunkPlan const chunk_plan = ComputeChunkPlan(options, random, device);

  std::cout << "Chunk strategy     : " << ToString(chunk_plan.strategy) << '\n';
  std::cout << "Chunk count        : " << chunk_plan.chunk_count << '\n';
  std::cout << "Max chunk request  : " << options.max_chunk_mib << " MiB\n";
  std::cout << "Device max alloc   : "
            << chunk_plan.device_max_allocation_bytes / kBytesPerMiB
            << " MiB\n";
  std::cout << "State SVM buffer   : "
            << chunk_plan.state_buffer_bytes / kBytesPerMiB << " MiB\n";
  std::cout << "Value SVM buffer   : "
            << chunk_plan.value_buffer_bytes / kBytesPerMiB << " MiB\n";

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

  auto states_buffer = context.CreateSVMBuffer(
      ggems::units::Bytes{chunk_plan.state_buffer_bytes});
  auto values_buffer = context.CreateSVMBuffer(
      ggems::units::Bytes{chunk_plan.value_buffer_bytes});

  auto output_stream = OpenOutputStream(options.output_path, options.force);

  std::uint64_t written_samples{0ULL};

  if (options.layout == StreamLayout::Interleaved) {
    InitializeStates(random, states_buffer, options.stream_offset,
                     options.worker_count);

    std::uint32_t remaining_samples = options.samples_per_worker;

    while (remaining_samples > 0U) {
      std::uint32_t const current_samples_per_worker =
          std::min(chunk_plan.samples_per_worker_per_chunk,
                   remaining_samples);

      RunRandomKernel(kernel, options, states_buffer.GetData(),
                      values_buffer.GetData(), options.worker_count,
                      current_samples_per_worker);

      std::uint64_t const current_sample_count =
          static_cast<std::uint64_t>(options.worker_count) *
          static_cast<std::uint64_t>(current_samples_per_worker);

      if (current_sample_count >
          static_cast<std::uint64_t>(
              std::numeric_limits<std::size_t>::max())) {
        throw std::runtime_error(
            "Random value chunk is too large for host I/O.");
      }

      values_buffer.Map(CL_MAP_READ);
      WriteValueChunk(output_stream, options.output_path, options.stream_type,
                      values_buffer.GetData(),
                      static_cast<std::size_t>(current_sample_count));
      values_buffer.Unmap();

      written_samples += current_sample_count;
      remaining_samples -= current_samples_per_worker;
    }
  } else {
    std::uint32_t worker_offset{0U};

    while (worker_offset < options.worker_count) {
      std::uint32_t const current_worker_count =
          std::min(chunk_plan.workers_per_chunk,
                   options.worker_count - worker_offset);

      std::uint64_t const first_stream_id =
          options.stream_offset + static_cast<std::uint64_t>(worker_offset);

      InitializeStates(random, states_buffer, first_stream_id,
                       current_worker_count);

      RunRandomKernel(kernel, options, states_buffer.GetData(),
                      values_buffer.GetData(), current_worker_count,
                      options.samples_per_worker);

      std::uint64_t const current_sample_count =
          static_cast<std::uint64_t>(current_worker_count) *
          static_cast<std::uint64_t>(options.samples_per_worker);

      if (current_sample_count >
          static_cast<std::uint64_t>(
              std::numeric_limits<std::size_t>::max())) {
        throw std::runtime_error(
            "Random value chunk is too large for host I/O.");
      }

      values_buffer.Map(CL_MAP_READ);
      WriteValueChunk(output_stream, options.output_path, options.stream_type,
                      values_buffer.GetData(),
                      static_cast<std::size_t>(current_sample_count));
      values_buffer.Unmap();

      written_samples += current_sample_count;
      worker_offset += current_worker_count;
    }
  }

  if (written_samples != total_samples) {
    throw std::runtime_error("Generated sample count does not match request.");
  }

  output_stream.close();

  if (!output_stream) {
    throw std::runtime_error(
        std::format("Failed to close output stream '{}'.",
                    options.output_path.string()));
  }

  std::uint64_t const actual_output_bytes =
      static_cast<std::uint64_t>(std::filesystem::file_size(options.output_path));

  if (actual_output_bytes != output_bytes) {
    throw std::runtime_error(
        std::format("Output stream size mismatch: expected {} bytes, got {}.",
                    output_bytes, actual_output_bytes));
  }

  WriteMinimalManifest(options.manifest_path, options, random, device,
                       chunk_plan, total_samples, output_bytes);

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
