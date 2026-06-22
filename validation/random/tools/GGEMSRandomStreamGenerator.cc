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
#include "GGEMS/core/units/GGEMSUnits.hh"
#include "GGEMS/frameworks/GGEMSOpenCL.hh"
#include "GGEMS/frameworks/GGEMSOpenCLKernel.hh"
#include "GGEMS/frameworks/GGEMSOpenCLSVMBuffer.hh"

namespace {
using ggems::core::random::GGEMSJKissState;
using ggems::core::random::GGEMSPCG32State;
using ggems::core::random::GGEMSPhiloxState;
using ggems::core::random::GGEMSRandom;
using ggems::core::random::GGEMSRandomEngine;

struct Options {
  std::string engine{"philox"};
  std::uint64_t seed{77777ULL};
  std::uint32_t particle_count{1024U};
  std::uint32_t words_per_particle{1024U};
  std::size_t local_size{64U};
  std::string device_selector{"cpu"};

  std::filesystem::path output_path{
      std::filesystem::path{GGEMS_VALIDATION_RANDOM_STREAM_ROOT} /
      "philox_uint32_smoke.bin"};
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

void WriteUInt32Binary(std::filesystem::path const &path,
                       std::uint32_t const *values, std::size_t value_count) {
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

  std::uint64_t value_bytes =
      total_words * static_cast<std::uint64_t>(sizeof(std::uint32_t));

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
      context.CreateSVMBuffer(ggems::units::Bytes{value_bytes});

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
  WriteUInt32Binary(options.output_path,
                    static_cast<std::uint32_t const *>(values_buffer.GetData()),
                    static_cast<std::size_t>(total_words));
  values_buffer.Unmap();

  std::cout << "Stream generated   : " << options.output_path.string() << '\n';
  std::cout << "Bytes written      : " << value_bytes << '\n';

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
