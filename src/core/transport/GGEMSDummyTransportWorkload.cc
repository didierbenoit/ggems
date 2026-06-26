#include "GGEMS/core/transport/GGEMSDummyTransportWorkload.hh"

#include <algorithm>
#include <format>
#include <string>
#include <utility>

#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/core/random/GGEMSRandom.hh"
#include "GGEMS/core/random/GGEMSRandomState.hh"
#include "GGEMS/core/units/GGEMSUnits.hh"
#include "GGEMS/frameworks/GGEMSOpenCL.hh"
#include "GGEMS/frameworks/GGEMSOpenCLKernel.hh"

namespace {

using ParticleState = ggems::core::particles::GGEMSParticleState;
using TransportCounters = ggems::core::transport::GGEMSTransportCounters;
using PhiloxState = ggems::core::random::GGEMSPhiloxState;

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

std::uint64_t SplitMix64(std::uint64_t value) noexcept {
  value += 0x9E3779B97F4A7C15ULL;

  value = (value ^ (value >> 30U)) * 0xBF58476D1CE4E5B9ULL;
  value = (value ^ (value >> 27U)) * 0x94D049BB133111EBULL;

  return value ^ (value >> 31U);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

PhiloxState MakePhiloxState(std::uint64_t const seed,
                            std::uint64_t const index) noexcept {
  std::uint64_t const key = SplitMix64(seed);

  return PhiloxState{.counter_0 = 0U,
                     .counter_1 = 0U,
                     .counter_2 = static_cast<std::uint32_t>(index),
                     .counter_3 = static_cast<std::uint32_t>(index >> 32U),
                     .key_0 = static_cast<std::uint32_t>(key),
                     .key_1 = static_cast<std::uint32_t>(key >> 32U)};
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

ggems::units::Bytes ComputeRandomStatesSize(std::uint32_t const worker_count) {
  GGEMS_CHECK_RECOVERABLE(worker_count > 0U,
                          "Dummy transport worker count must be non-zero.");

  return ggems::units::Bytes{static_cast<std::uint64_t>(worker_count) *
                             sizeof(PhiloxState)};
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

ggems::units::Bytes
ComputeWorkerFinalStatesSize(std::uint32_t const worker_count) {
  GGEMS_CHECK_RECOVERABLE(worker_count > 0U,
                          "Dummy transport worker count must be non-zero.");

  return ggems::units::Bytes{static_cast<std::uint64_t>(worker_count) *
                             sizeof(ParticleState)};
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

std::size_t RoundUp(std::size_t const value,
                    std::size_t const multiple) noexcept {
  return ((value + multiple - 1U) / multiple) * multiple;
}

} // namespace

namespace ggems::core::transport {

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

GGEMSDummyTransportWorkload::GGEMSDummyTransportWorkload(
    ggems::ocl::GGEMSOpenCLContext &context, std::filesystem::path kernel_root,
    random::GGEMSRandom const &random, std::uint32_t const worker_count)
    : context_{&context}, kernel_root_{std::move(kernel_root)},
      random_{&random}, worker_count_{worker_count},
      random_states_buffer_{
          context.CreateSVMBuffer(ComputeRandomStatesSize(worker_count))},
      worker_final_states_buffer_{
          context.CreateSVMBuffer(ComputeWorkerFinalStatesSize(worker_count))},
      counters_buffer_{context.CreateSVMBuffer(
          ggems::units::Bytes{sizeof(TransportCounters)})} {
  GGEMS_CHECK_RECOVERABLE(
      random_->GetKernelEngineId() == 3U,
      "Dummy transport prototype currently expects the Philox random engine.");

  InitialiseRandomStatesOnHost();
  ClearWorkerFinalStatesOnHost();
  ResetCountersOnHost();
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void GGEMSDummyTransportWorkload::InitialiseRandomStatesOnHost() {
  random_states_buffer_.Map(CL_MAP_WRITE);

  auto *states = static_cast<PhiloxState *>(random_states_buffer_.GetData());

  for (std::uint32_t i = 0U; i < worker_count_; ++i) {
    states[i] = MakePhiloxState(random_->GetSeed(), i);
  }

  random_states_buffer_.Unmap();
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void GGEMSDummyTransportWorkload::ClearWorkerFinalStatesOnHost() {
  worker_final_states_buffer_.Map(CL_MAP_WRITE);

  auto *states =
      static_cast<ParticleState *>(worker_final_states_buffer_.GetData());

  std::fill(states, states + worker_count_, ParticleState{});

  worker_final_states_buffer_.Unmap();
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void GGEMSDummyTransportWorkload::ResetCountersOnHost() {
  counters_buffer_.Map(CL_MAP_WRITE);

  auto *counters = static_cast<TransportCounters *>(counters_buffer_.GetData());

  *counters = TransportCounters{};

  counters_buffer_.Unmap();
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

GGEMSTransportCounters GGEMSDummyTransportWorkload::ReadCountersOnHost() {
  counters_buffer_.Map(CL_MAP_READ);

  auto const *counters =
      static_cast<TransportCounters const *>(counters_buffer_.GetData());

  TransportCounters copy = *counters;

  counters_buffer_.Unmap();

  return copy;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void GGEMSDummyTransportWorkload::Run(
    GGEMSDummyTransportRunConfig const &config) {
  GGEMS_CHECK_RECOVERABLE(config.total_primary_count > 0U,
                          "Dummy transport primary count must be non-zero.");

  ResetCountersOnHost();
  ClearWorkerFinalStatesOnHost();

  auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();

  std::filesystem::path const kernel_transport_root =
      kernel_root_ / "core" / "transport";

  std::string const build_options = std::format(
      "-cl-std=CL2.0 -I{} {} "
      "-DGGEMS_DUMMY_LOCAL_STACK_CAPACITY=16",
      kernel_root_.generic_string(), random_->GetKernelBuildDefinition());

  auto &program = opencl.GetOrCreateProgram(
      *context_, kernel_transport_root,
      "particle_dummy_stream_branching_transport", build_options);

  cl::Kernel raw_kernel =
      program.CreateKernel("particle_dummy_stream_branching_transport");

  ggems::ocl::GGEMSOpenCLKernel kernel{
      *context_, std::move(raw_kernel),
      "particle_dummy_stream_branching_transport"};

  auto *random_states = random_states_buffer_.GetData();
  auto *worker_final_states = worker_final_states_buffer_.GetData();
  auto *counters = counters_buffer_.GetData();

  kernel.SetArgSVMPointer(0U, random_states);
  kernel.SetArgSVMPointer(1U, worker_final_states);
  kernel.SetArgSVMPointer(2U, counters);
  kernel.SetArg(3U, static_cast<cl_uint>(config.total_primary_count));
  kernel.SetArg(4U, static_cast<cl_ulong>(config.initial_energy_milli_eV));
  kernel.SetArg(5U, static_cast<cl_ulong>(config.min_energy_milli_eV));
  kernel.SetArg(6U, static_cast<cl_uint>(config.max_generation));
  kernel.SetArg(7U, static_cast<cl_uint>(config.max_steps_per_track));

  constexpr std::size_t k_local_size{64U};
  std::size_t const global_size = RoundUp(worker_count_, k_local_size);

  kernel.Run({global_size}, {k_local_size});
}

} // namespace ggems::core::transport
