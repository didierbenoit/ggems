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
#include "GGEMS/frameworks/GGEMSOpenCLProfiler.hh"

namespace {

using ParticleState = ggems::core::particles::GGEMSParticleState;
using TransportCounters = ggems::core::transport::GGEMSTransportCounters;
using PhiloxState = ggems::core::random::GGEMSPhiloxState;
using ObserverConfigRecord = ggems::core::observer::GGEMSObserverConfigRecord;
using ObserverCounters = ggems::core::observer::GGEMSObserverCounters;
using ObserverRecord = ggems::core::observer::GGEMSObserverRecord;

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

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

ggems::units::Bytes
ComputeObserverRecordsSize(std::uint32_t observer_record_capacity) {
  GGEMS_CHECK_RECOVERABLE(
      observer_record_capacity > 0U,
      "Dummy transport observer record capacity must be non-zero.");

  return ggems::units::Bytes{
      static_cast<std::uint64_t>(observer_record_capacity) *
      sizeof(ObserverRecord)};
}

} // namespace

namespace ggems::core::transport {

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

GGEMSDummyTransportWorkload::GGEMSDummyTransportWorkload(
    ggems::ocl::GGEMSOpenCLContext &context, std::filesystem::path kernel_root,
    random::GGEMSRandom const &random, std::uint32_t worker_count,
    std::uint64_t random_stream_offset, std::uint32_t context_index,
    std::uint32_t observer_record_capacity)
    : context_{&context}, kernel_root_{std::move(kernel_root)},
      random_{&random}, worker_count_{worker_count},
      random_stream_offset_{random_stream_offset},
      context_index_{context_index},
      device_name_{context.GetDevice().GetName()},
      observer_record_capacity_{observer_record_capacity},
      random_states_buffer_{
          context.CreateSVMBuffer(ComputeRandomStatesSize(worker_count))},
      worker_final_states_buffer_{
          context.CreateSVMBuffer(ComputeWorkerFinalStatesSize(worker_count))},
      counters_buffer_{context.CreateSVMBuffer(
          ggems::units::Bytes{sizeof(TransportCounters)})},
      source_record_buffer_{context.CreateSVMBuffer(
          ggems::units::Bytes{sizeof(sources::GGEMSSourceRecord)})},
      observer_config_buffer_{context.CreateSVMBuffer(
          ggems::units::Bytes{sizeof(ObserverConfigRecord)})},
      observer_counters_buffer_{context.CreateSVMBuffer(
          ggems::units::Bytes{sizeof(ObserverCounters)})},
      observer_records_buffer_{context.CreateSVMBuffer(
          ComputeObserverRecordsSize(observer_record_capacity))} {
  GGEMS_CHECK_RECOVERABLE(
      random_->GetKernelEngineId() == 3U,
      "Dummy transport prototype currently expects the Philox random engine.");

  InitialiseRandomStatesOnHost();
  ClearWorkerFinalStatesOnHost();
  ResetCountersOnHost();
  ResetObserverOnHost();
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void GGEMSDummyTransportWorkload::InitialiseRandomStatesOnHost() {
  random_states_buffer_.Map(CL_MAP_WRITE);

  auto *states = static_cast<PhiloxState *>(random_states_buffer_.GetData());

  for (std::uint32_t i = 0U; i < worker_count_; ++i) {
    states[i] = MakePhiloxState(random_->GetSeed(), random_stream_offset_ + i);
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

void GGEMSDummyTransportWorkload::WriteSourceRecordOnHost(
    sources::GGEMSSourceRecord const &source_record) {
  source_record_buffer_.Map(CL_MAP_WRITE);

  auto *source = static_cast<sources::GGEMSSourceRecord *>(
      source_record_buffer_.GetData());

  *source = source_record;

  source_record_buffer_.Unmap();
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

observer::GGEMSObserverCounters
GGEMSDummyTransportWorkload::ReadObserverCountersOnHost() {
  observer_counters_buffer_.Map(CL_MAP_READ);

  auto const *counters = static_cast<ObserverCounters const *>(
      observer_counters_buffer_.GetData());

  ObserverCounters copy = *counters;

  observer_counters_buffer_.Unmap();
  return copy;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void GGEMSDummyTransportWorkload::ResetObserverOnHost() {
  observer_counters_buffer_.Map(CL_MAP_WRITE);

  auto *counters =
      static_cast<ObserverCounters *>(observer_counters_buffer_.GetData());

  *counters = ObserverCounters{};

  observer_counters_buffer_.Unmap();

  observer_records_buffer_.Map(CL_MAP_WRITE);

  auto *records =
      static_cast<ObserverRecord *>(observer_records_buffer_.GetData());

  std::fill(records, records + observer_record_capacity_, ObserverRecord{});

  observer_records_buffer_.Unmap();
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void GGEMSDummyTransportWorkload::WriteObserverConfigOnHost(
    observer::GGEMSObserverConfigRecord const &observer_config) {
  observer_config_buffer_.Map(CL_MAP_WRITE);

  auto *config =
      static_cast<ObserverConfigRecord *>(observer_config_buffer_.GetData());

  *config = observer_config;

  observer_config_buffer_.Unmap();
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

GGEMSDummyTransportRunReport
GGEMSDummyTransportWorkload::Run(GGEMSDummyTransportRunConfig const &config) {
  GGEMS_CHECK_RECOVERABLE(config.total_primary_count > 0U,
                          "Dummy transport primary count must be non-zero.");

  ResetCountersOnHost();
  ClearWorkerFinalStatesOnHost();
  ResetObserverOnHost();
  WriteSourceRecordOnHost(config.source_record);
  WriteObserverConfigOnHost(config.observer_config);

  auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();

  std::filesystem::path const kernel_transport_root =
      kernel_root_ / "core" / "transport";

  std::string observer_build_definition =
      config.observer_config.enabled != 0U
          ? "-DGGEMS_ENABLE_TRANSPORT_OBSERVER=1"
          : "-DGGEMS_ENABLE_TRANSPORT_OBSERVER=0";

  std::string const build_options = std::format(
      "-cl-std=CL2.0 -I{} {} {} "
      "-DGGEMS_DUMMY_LOCAL_STACK_CAPACITY=16",
      kernel_root_.generic_string(), random_->GetKernelBuildDefinition(),
      observer_build_definition);

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
  auto *counters_ptr = counters_buffer_.GetData();
  auto *source_record = source_record_buffer_.GetData();
  auto *observer_config = observer_config_buffer_.GetData();
  auto *observer_counters = observer_counters_buffer_.GetData();
  auto *observer_records = observer_records_buffer_.GetData();

  kernel.SetArgSVMPointer(0U, random_states);
  kernel.SetArgSVMPointer(1U, worker_final_states);
  kernel.SetArgSVMPointer(2U, counters_ptr);
  kernel.SetArgSVMPointer(3U, source_record);
  kernel.SetArg(4U, static_cast<cl_uint>(config.total_primary_count));
  kernel.SetArg(5U, static_cast<cl_ulong>(config.projection_history_offset));
  kernel.SetArg(6U, static_cast<cl_ulong>(config.device_primary_offset));
  kernel.SetArg(7U, static_cast<cl_ulong>(config.min_energy_milli_eV));
  kernel.SetArg(8U, static_cast<cl_uint>(config.max_generation));
  kernel.SetArg(9U, static_cast<cl_uint>(config.max_steps_per_track));
  kernel.SetArgSVMPointer(10U, observer_config);
  kernel.SetArgSVMPointer(11U, observer_counters);
  kernel.SetArgSVMPointer(12U, observer_records);
  kernel.SetArg(13U, static_cast<cl_uint>(observer_record_capacity_));

  constexpr std::size_t k_local_size{64U};
  std::size_t const global_size = RoundUp(worker_count_, k_local_size);

  ggems::ocl::GGEMSOpenCLProfiler profiler{};

  profiler.Start();

  cl::Event event = kernel.RunAndGetEvent({global_size}, {k_local_size});

  profiler.Stop();
  profiler.RecordKernelEvent(event);

  GGEMSTransportCounters transport_counters = ReadCountersOnHost();
  ObserverCounters observer_counters_report = ReadObserverCountersOnHost();

  GGEMSDummyTransportRunReport report{};
  report.context_index = context_index_;
  report.device_name = device_name_;

  report.counters = transport_counters;
  report.observer_counters = observer_counters_report;

  report.host_time = profiler.GetElapsedTime();
  report.kernel_time = profiler.GetKernelTime();
  report.command_time = profiler.GetCommandTime();

  report.host_histories_per_second =
      profiler.ComputeRatePerSecond(transport_counters.completed_history_count);

  report.kernel_histories_per_second = profiler.ComputeKernelRatePerSecond(
      transport_counters.completed_history_count);

  report.host_terminal_particles_per_second =
      profiler.ComputeRatePerSecond(transport_counters.terminal_particle_count);

  report.kernel_terminal_particles_per_second =
      profiler.ComputeKernelRatePerSecond(
          transport_counters.terminal_particle_count);

  return report;
}

} // namespace ggems::core::transport
