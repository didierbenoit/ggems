#include "GGEMS/transport/GGEMSDummyTransportWorkload.hh"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <limits>
#include <span>
#include <string>
#include <utility>
#include <vector>

#include "GGEMS/opencl/GGEMSOpenCLLaunchGeometry.hh"
#include "GGEMS/GGEMSException.hh"
#include "GGEMS/observer/GGEMSObserverRecord.hh"
#include "GGEMS/particles/GGEMSParticleState.hh"
#include "GGEMS/sources/GGEMSSourceRecord.hh"
#include "GGEMS/sources/GGEMSSourceRunRange.hh"
#include "GGEMS/transport/GGEMSTransportCounters.hh"
#include "GGEMS/random/GGEMSRandom.hh"
#include "GGEMS/random/GGEMSRandomState.hh"
#include "GGEMS/units/GGEMSBytesUnits.hh"
#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/opencl/GGEMSOpenCLExternal.hh"
#include "GGEMS/opencl/GGEMSOpenCLKernel.hh"
#include "GGEMS/opencl/GGEMSOpenCLProfiler.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMHostAccess.hh"

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

ggems::units::Bytes
ComputeRandomStatesSize(ggems::core::random::GGEMSRandom const &random,
                        std::uint32_t worker_count,
                        std::uint64_t first_stream_id) {
  if (!(worker_count > 0U)) {
    throw ggems::core::GGEMSRecoverable(
        "Dummy transport worker count must be non-zero.");
  }

  random.ValidateStateRange(first_stream_id, worker_count);

  return ggems::units::Bytes{static_cast<std::uint64_t>(worker_count) *
                             static_cast<std::uint64_t>(random.GetStateSize())};
}

// =============================================================================
// =============================================================================

ggems::units::Bytes ComputeWorkerFinalStatesSize(std::uint32_t worker_count) {
  if (!(worker_count > 0U)) {
    throw ggems::core::GGEMSRecoverable(
        "Dummy transport worker count must be non-zero.");
  }

  return ggems::units::Bytes{static_cast<std::uint64_t>(worker_count) *
                             sizeof(ParticleState)};
}

// =============================================================================
// =============================================================================

ggems::units::Bytes
ComputeObserverRecordsSize(std::uint32_t observer_record_capacity) {
  if (!(observer_record_capacity > 0U)) {
    throw ggems::core::GGEMSRecoverable(
        "Dummy transport observer record capacity must be non-zero.");
  }

  return ggems::units::Bytes{
      static_cast<std::uint64_t>(observer_record_capacity) *
      sizeof(ObserverRecord)};
}

// =============================================================================
// =============================================================================

std::uint32_t CheckedSourceCount(std::uint32_t source_count) {
  if (!(source_count > 0U)) {
    throw ggems::core::GGEMSRecoverable(
        "Dummy transport source count must be non-zero.");
  }

  return source_count;
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
      source_count_{CheckedSourceCount(source_count)},
      random_stream_offset_{random_stream_offset},
      context_index_{context_index},
      device_name_{context.GetDevice().GetName()},
      observer_record_capacity_{observer_record_capacity},
      random_states_buffer_{context.CreateSVMBuffer(
          ComputeRandomStatesSize(random, worker_count, random_stream_offset))},
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

  InitializeRandomStatesInSVM();
  ClearWorkerFinalStatesInSVM();
  ResetCountersInSVM();
  ResetObserverInSVM();
}

// -----------------------------------------------------------------------------

void GGEMSDummyTransportWorkload::InitializeRandomStatesInSVM() {
  std::uint64_t state_bytes = random_states_buffer_.GetSize().value;

  if (!(state_bytes <= std::numeric_limits<std::size_t>::max())) {
    throw ggems::core::GGEMSInternal(
        "Random state buffer size exceeds host addressable storage.");
  }

  auto *state_storage =
      static_cast<std::byte *>(random_states_buffer_.GetData());

  random_states_buffer_.Map(CL_MAP_WRITE);

  random_->InitializeStates(
      random_stream_offset_,
      std::span<std::byte>{state_storage,
                           static_cast<std::size_t>(state_bytes)});

  random_states_buffer_.Unmap();
}

// -----------------------------------------------------------------------------

void GGEMSDummyTransportWorkload::ClearWorkerFinalStatesInSVM() {
  ggems::ocl::FillSVMFromHost(worker_final_states_buffer_, worker_count_,
                              ParticleState{});
}

// -----------------------------------------------------------------------------

void GGEMSDummyTransportWorkload::ResetCountersInSVM() {
  ggems::ocl::WriteSVMFromHost(counters_buffer_, TransportCounters{});
}

// -----------------------------------------------------------------------------

GGEMSTransportCounters GGEMSDummyTransportWorkload::ReadCountersFromSVM() {
  return ggems::ocl::ReadSVMToHost<TransportCounters>(counters_buffer_);
}

// -----------------------------------------------------------------------------

observer::GGEMSObserverCounters
GGEMSDummyTransportWorkload::ReadObserverCountersFromSVM() {
  return ggems::ocl::ReadSVMToHost<ObserverCounters>(observer_counters_buffer_);
}

// -----------------------------------------------------------------------------

void GGEMSDummyTransportWorkload::ResetObserverInSVM() {
  ggems::ocl::WriteSVMFromHost(observer_counters_buffer_, ObserverCounters{});
  ggems::ocl::FillSVMFromHost(observer_records_buffer_,
                              observer_record_capacity_, ObserverRecord{});
}

// -----------------------------------------------------------------------------

void GGEMSDummyTransportWorkload::WriteObserverConfigToSVM(
    observer::GGEMSObserverConfigRecord const &observer_config) {
  ggems::ocl::WriteSVMFromHost(observer_config_buffer_, observer_config);
}

// -----------------------------------------------------------------------------

std::vector<observer::GGEMSObserverRecord>
GGEMSDummyTransportWorkload::ReadObserverRecordsFromSVM(
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
  if (!(config.total_primary_count > 0U)) {
    throw ggems::core::GGEMSRecoverable(
        "Dummy transport primary count must be non-zero.");
  }

  if (!(config.source_records.size() == config.source_ranges.size())) {
    throw ggems::core::GGEMSRecoverable(
        "Dummy transport source record and range counts must match.");
  }

  if (!(config.source_records.size() ==
        static_cast<std::size_t>(source_count_))) {
    throw ggems::core::GGEMSRecoverable(
        "Dummy transport source arrays do not match the stable source count.");
  }

  for (std::size_t source_index = 0U;
       source_index < config.source_records.size(); ++source_index) {
    if (!(config.source_records[source_index].energy_micro_eV > 0ULL)) {
      throw ggems::core::GGEMSRecoverable(std::format(
          "Dummy transport source {} must use an exact positive Mono "
          "energy.",
          source_index));
    }
  }

  ResetCountersInSVM();
  ClearWorkerFinalStatesInSVM();
  ResetObserverInSVM();
  ggems::ocl::WriteSVMFromHost(
      source_records_buffer_,
      std::span<SourceRecord const>{config.source_records});
  ggems::ocl::WriteSVMFromHost(
      source_ranges_buffer_,
      std::span<SourceRunRange const>{config.source_ranges});
  WriteObserverConfigToSVM(config.observer_config);

  auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();

  std::filesystem::path const kernel_transport_root =
      kernel_root_ / "transport";

  std::string observer_build_definition =
      config.observer_config.enabled != 0U
          ? "-DGGEMS_ENABLE_TRANSPORT_OBSERVER=1"
          : "-DGGEMS_ENABLE_TRANSPORT_OBSERVER=0";

  std::string const build_options = std::format(
      "-I{} {} {} "
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

  constexpr cl_uint k_expected_argument_count{18U};
  cl_uint argument_index{0U};

  kernel.SetArgSVMPointer(argument_index++, random_states);
  kernel.SetArgSVMPointer(argument_index++, worker_final_states);
  kernel.SetArgSVMPointer(argument_index++, counters_ptr);
  kernel.SetArgSVMPointer(argument_index++, source_records);
  kernel.SetArgSVMPointer(argument_index++, source_ranges);
  kernel.SetArg(argument_index++, static_cast<cl_uint>(source_count_));
  kernel.SetArg(argument_index++,
                static_cast<cl_uint>(config.total_primary_count));
  kernel.SetArg(argument_index++,
                static_cast<cl_ulong>(config.projection_history_offset));
  kernel.SetArg(argument_index++,
                static_cast<cl_ulong>(config.device_primary_offset));
  kernel.SetArg(argument_index++,
                static_cast<cl_ulong>(config.min_energy_micro_eV));
  kernel.SetArg(argument_index++, static_cast<cl_uint>(config.max_generation));
  kernel.SetArg(argument_index++,
                static_cast<cl_uint>(config.max_steps_per_track));
  kernel.SetArgSVMPointer(argument_index++, observer_config);
  kernel.SetArgSVMPointer(argument_index++, observer_counters);
  kernel.SetArgSVMPointer(argument_index++, observer_records);
  kernel.SetArg(argument_index++,
                static_cast<cl_uint>(observer_record_capacity_));
  kernel.SetArg(argument_index++, static_cast<cl_ulong>(config.run_id));
  kernel.SetArg(argument_index++, static_cast<cl_uint>(worker_count_));

  if (!(argument_index == k_expected_argument_count)) {
    throw ggems::core::GGEMSInternal(
        "Dummy transport kernel argument count is inconsistent.");
  }

  constexpr std::size_t k_local_size{64U};
  auto const padded_global_work_size =
      ggems::ocl::detail::TryComputePaddedGlobalWorkSize(worker_count_,
                                                         k_local_size);
  if (!(padded_global_work_size.has_value())) {
    throw ggems::core::GGEMSInternal(
        "Unable to compute the padded OpenCL global work size.");
  }
  std::size_t const global_size = *padded_global_work_size;

  ggems::ocl::GGEMSOpenCLProfiler profiler{};

  profiler.Start();

  cl::Event event = kernel.RunAndGetEvent({global_size}, {k_local_size});

  profiler.Stop();
  profiler.RecordKernelEvent(event);

  GGEMSTransportCounters transport_counters = ReadCountersFromSVM();
  ObserverCounters observer_counters_report = ReadObserverCountersFromSVM();
  std::vector<ObserverRecord> observer_records_report =
      ReadObserverRecordsFromSVM(observer_counters_report.record_count);

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
