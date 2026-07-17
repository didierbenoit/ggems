#include "GGEMS/core/transport/GGEMSDummyTransportWorkload.hh"

#include <algorithm>
#include <format>
#include <string>
#include <utility>
#include <vector>

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
using SourceRecord = ggems::core::sources::GGEMSSourceRecord;
using SourceRunRange = ggems::core::sources::GGEMSSourceRunRange;

// =============================================================================
// =============================================================================

std::uint64_t SplitMix64(std::uint64_t value) noexcept {
  value += 0x9E3779B97F4A7C15ULL;

  value = (value ^ (value >> 30U)) * 0xBF58476D1CE4E5B9ULL;
  value = (value ^ (value >> 27U)) * 0x94D049BB133111EBULL;

  return value ^ (value >> 31U);
}

// =============================================================================
// =============================================================================

PhiloxState MakePhiloxState(std::uint64_t seed, std::uint64_t index) noexcept {
  std::uint64_t const key = SplitMix64(seed);

  return PhiloxState{.counter_0 = 0U,
                     .counter_1 = 0U,
                     .counter_2 = static_cast<std::uint32_t>(index),
                     .counter_3 = static_cast<std::uint32_t>(index >> 32U),
                     .key_0 = static_cast<std::uint32_t>(key),
                     .key_1 = static_cast<std::uint32_t>(key >> 32U)};
}

// =============================================================================
// =============================================================================

ggems::units::Bytes ComputeRandomStatesSize(std::uint32_t worker_count) {
  GGEMS_CHECK_RECOVERABLE(worker_count > 0U,
                          "Dummy transport worker count must be non-zero.");

  return ggems::units::Bytes{static_cast<std::uint64_t>(worker_count) *
                             sizeof(PhiloxState)};
}

// =============================================================================
// =============================================================================

ggems::units::Bytes ComputeWorkerFinalStatesSize(std::uint32_t worker_count) {
  GGEMS_CHECK_RECOVERABLE(worker_count > 0U,
                          "Dummy transport worker count must be non-zero.");

  return ggems::units::Bytes{static_cast<std::uint64_t>(worker_count) *
                             sizeof(ParticleState)};
}

// =============================================================================
// =============================================================================

std::size_t RoundUp(std::size_t value, std::size_t multiple) noexcept {
  return ((value + multiple - 1U) / multiple) * multiple;
}

// =============================================================================
// =============================================================================

ggems::units::Bytes
ComputeObserverRecordsSize(std::uint32_t observer_record_capacity) {
  GGEMS_CHECK_RECOVERABLE(
      observer_record_capacity > 0U,
      "Dummy transport observer record capacity must be non-zero.");

  return ggems::units::Bytes{
      static_cast<std::uint64_t>(observer_record_capacity) *
      sizeof(ObserverRecord)};
}

// =============================================================================
// =============================================================================

std::uint32_t ValidateSourceCount(std::uint32_t source_count) {
  GGEMS_CHECK_RECOVERABLE(source_count > 0U,
                          "Dummy transport source count must be non-zero.");

  return source_count;
}

// =============================================================================
// =============================================================================

template <typename Value>
void WriteSVMArrayOnHost(ggems::ocl::GGEMSOpenCLSVMBuffer &buffer,
                         std::vector<Value> const &values) {
  buffer.Map(CL_MAP_WRITE);
  auto *destination = static_cast<Value *>(buffer.GetData());
  std::copy(values.begin(), values.end(), destination);
  buffer.Unmap();
}

} // namespace

namespace ggems::core::transport {

// =============================================================================
// =============================================================================

GGEMSDummyTransportWorkload::GGEMSDummyTransportWorkload(
    ggems::ocl::GGEMSOpenCLContext &context, std::filesystem::path kernel_root,
    random::GGEMSRandom const &random, std::uint32_t worker_count,
    std::uint32_t source_count, std::uint64_t random_stream_offset,
    std::uint32_t context_index, std::uint32_t observer_record_capacity)
    : context_{&context}, kernel_root_{std::move(kernel_root)},
      random_{&random}, worker_count_{worker_count},
      source_count_{source_count}, random_stream_offset_{random_stream_offset},
      context_index_{context_index},
      device_name_{context.GetDevice().GetName()},
      observer_record_capacity_{observer_record_capacity},
      random_states_buffer_{
          context.CreateSVMBuffer(ComputeRandomStatesSize(worker_count))},
      worker_final_states_buffer_{
          context.CreateSVMBuffer(ComputeWorkerFinalStatesSize(worker_count))},
      counters_buffer_{context.CreateSVMBuffer(
          ggems::units::Bytes{sizeof(TransportCounters)})},
      source_records_buffer_{context.CreateSVMBuffer(ggems::units::Bytes{
          static_cast<std::uint64_t>(source_count_) * sizeof(SourceRecord)})},
      source_ranges_buffer_{context.CreateSVMBuffer(ggems::units::Bytes{
          static_cast<std::uint64_t>(source_count_) * sizeof(SourceRunRange)})},
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

// -----------------------------------------------------------------------------

void GGEMSDummyTransportWorkload::InitialiseRandomStatesOnHost() {
  random_states_buffer_.Map(CL_MAP_WRITE);

  auto *states = static_cast<PhiloxState *>(random_states_buffer_.GetData());

  for (std::uint32_t i = 0U; i < worker_count_; ++i) {
    states[i] = MakePhiloxState(random_->GetSeed(), random_stream_offset_ + i);
  }

  random_states_buffer_.Unmap();
}

// -----------------------------------------------------------------------------

void GGEMSDummyTransportWorkload::ClearWorkerFinalStatesOnHost() {
  worker_final_states_buffer_.Map(CL_MAP_WRITE);

  auto *states =
      static_cast<ParticleState *>(worker_final_states_buffer_.GetData());

  std::fill(states, states + worker_count_, ParticleState{});

  worker_final_states_buffer_.Unmap();
}

// -----------------------------------------------------------------------------

void GGEMSDummyTransportWorkload::ResetCountersOnHost() {
  counters_buffer_.Map(CL_MAP_WRITE);

  auto *counters = static_cast<TransportCounters *>(counters_buffer_.GetData());

  *counters = TransportCounters{};

  counters_buffer_.Unmap();
}

// -----------------------------------------------------------------------------

GGEMSTransportCounters GGEMSDummyTransportWorkload::ReadCountersOnHost() {
  counters_buffer_.Map(CL_MAP_READ);

  auto const *counters =
      static_cast<TransportCounters const *>(counters_buffer_.GetData());

  TransportCounters copy = *counters;

  counters_buffer_.Unmap();

  return copy;
}

// -----------------------------------------------------------------------------

observer::GGEMSObserverCounters
GGEMSDummyTransportWorkload::ReadObserverCountersOnHost() {
  observer_counters_buffer_.Map(CL_MAP_READ);

  auto const *counters = static_cast<ObserverCounters const *>(
      observer_counters_buffer_.GetData());

  ObserverCounters copy = *counters;

  observer_counters_buffer_.Unmap();
  return copy;
}

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

void GGEMSDummyTransportWorkload::WriteObserverConfigOnHost(
    observer::GGEMSObserverConfigRecord const &observer_config) {
  observer_config_buffer_.Map(CL_MAP_WRITE);

  auto *config =
      static_cast<ObserverConfigRecord *>(observer_config_buffer_.GetData());

  *config = observer_config;

  observer_config_buffer_.Unmap();
}

// -----------------------------------------------------------------------------

std::vector<observer::GGEMSObserverRecord>
GGEMSDummyTransportWorkload::ReadObserverRecordsOnHost(
    std::uint32_t record_count) {
  std::uint32_t bounded_record_count =
      std::min(record_count, observer_record_capacity_);

  std::vector<ObserverRecord> records(bounded_record_count);

  if (bounded_record_count == 0U) {
    return records;
  }

  observer_records_buffer_.Map(CL_MAP_READ);

  auto const *device_records =
      static_cast<ObserverRecord const *>(observer_records_buffer_.GetData());

  std::copy(device_records, device_records + bounded_record_count,
            records.begin());

  observer_records_buffer_.Unmap();

  return records;
}

// -----------------------------------------------------------------------------

GGEMSDummyTransportRunReport
GGEMSDummyTransportWorkload::Run(GGEMSDummyTransportRunConfig const &config) {
  GGEMS_CHECK_RECOVERABLE(config.total_primary_count > 0U,
                          "Dummy transport primary count must be non-zero.");

  GGEMS_CHECK_RECOVERABLE(
      config.source_records.size() == config.source_ranges.size(),
      "Dummy transport source record and range counts must match.");

  GGEMS_CHECK_RECOVERABLE(
      config.source_records.size() == static_cast<std::size_t>(source_count_),
      "Dummy transport source arrays do not match the stable source count.");

  ResetCountersOnHost();
  ClearWorkerFinalStatesOnHost();
  ResetObserverOnHost();
  WriteSVMArrayOnHost(source_records_buffer_, config.source_records);
  WriteSVMArrayOnHost(source_ranges_buffer_, config.source_ranges);
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
  auto *source_records = source_records_buffer_.GetData();
  auto *source_ranges = source_ranges_buffer_.GetData();
  auto *observer_config = observer_config_buffer_.GetData();
  auto *observer_counters = observer_counters_buffer_.GetData();
  auto *observer_records = observer_records_buffer_.GetData();

  kernel.SetArgSVMPointer(0U, random_states);
  kernel.SetArgSVMPointer(1U, worker_final_states);
  kernel.SetArgSVMPointer(2U, counters_ptr);
  kernel.SetArgSVMPointer(3U, source_records);
  kernel.SetArgSVMPointer(4U, source_ranges);
  kernel.SetArg(5U, static_cast<cl_uint>(source_count_));
  kernel.SetArg(6U, static_cast<cl_uint>(config.total_primary_count));
  kernel.SetArg(7U, static_cast<cl_ulong>(config.projection_history_offset));
  kernel.SetArg(8U, static_cast<cl_ulong>(config.device_primary_offset));
  kernel.SetArg(9U, static_cast<cl_ulong>(config.min_energy_milli_eV));
  kernel.SetArg(10U, static_cast<cl_uint>(config.max_generation));
  kernel.SetArg(11U, static_cast<cl_uint>(config.max_steps_per_track));
  kernel.SetArgSVMPointer(12U, observer_config);
  kernel.SetArgSVMPointer(13U, observer_counters);
  kernel.SetArgSVMPointer(14U, observer_records);
  kernel.SetArg(15U, static_cast<cl_uint>(observer_record_capacity_));
  kernel.SetArg(16U, static_cast<cl_ulong>(config.run_id));
  kernel.SetArg(17U, static_cast<cl_uint>(worker_count_));

  constexpr std::size_t k_local_size{64U};
  std::size_t global_size = RoundUp(worker_count_, k_local_size);

  ggems::ocl::GGEMSOpenCLProfiler profiler{};

  profiler.Start();

  cl::Event event = kernel.RunAndGetEvent({global_size}, {k_local_size});

  profiler.Stop();
  profiler.RecordKernelEvent(event);

  GGEMSTransportCounters transport_counters = ReadCountersOnHost();
  ObserverCounters observer_counters_report = ReadObserverCountersOnHost();
  std::vector<ObserverRecord> observer_records_report =
      ReadObserverRecordsOnHost(observer_counters_report.record_count);

  GGEMSDummyTransportRunReport report{};
  report.context_index = context_index_;
  report.device_name = device_name_;

  report.counters = transport_counters;
  report.observer_counters = observer_counters_report;
  report.observer_records = observer_records_report;

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
