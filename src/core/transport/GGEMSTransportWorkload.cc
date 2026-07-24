#include "GGEMS/core/transport/GGEMSTransportWorkload.hh"

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

#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/core/random/GGEMSRandom.hh"
#include "GGEMS/core/sources/GGEMSSourceRecord.hh"
#include "GGEMS/core/sources/GGEMSSourceRunRange.hh"
#include "GGEMS/core/transport/GGEMSDiagnosticProjection.hh"
#include "GGEMS/core/transport/GGEMSTransportCounters.hh"
#include "GGEMS/frameworks/GGEMSOpenCL.hh"
#include "GGEMS/frameworks/GGEMSOpenCLKernel.hh"
#include "GGEMS/frameworks/GGEMSOpenCLProfiler.hh"
#include "GGEMS/frameworks/GGEMSOpenCLSVMHostAccess.hh"
#include "GGEMS/core/observer/GGEMSObserverRecord.hh"
#include "GGEMS/core/units/GGEMSBytesUnits.hh"

namespace {

using ObserverConfigRecord = ggems::core::observer::GGEMSObserverConfigRecord;
using ObserverCounters = ggems::core::observer::GGEMSObserverCounters;
using ObserverRecord = ggems::core::observer::GGEMSObserverRecord;
using SourceRecord = ggems::core::sources::GGEMSSourceRecord;
using SourceRunRange = ggems::core::sources::GGEMSSourceRunRange;
using TransportCounters = ggems::core::transport::GGEMSTransportCounters;
using TransportRunConfig = ggems::core::transport::GGEMSTransportRunConfig;

// =============================================================================
// =============================================================================

[[nodiscard]] auto
ComputeRandomStatesSize(ggems::core::random::GGEMSRandom const &random,
                        std::uint32_t worker_count,
                        std::uint64_t first_stream_id) -> ggems::units::Bytes {
  GGEMS_CHECK_RECOVERABLE(worker_count > 0U,
                          "Transport worker count must be non-zero.");

  random.ValidateStateRange(first_stream_id, worker_count);

  return ggems::units::Bytes{static_cast<std::uint64_t>(worker_count) *
                             static_cast<std::uint64_t>(random.GetStateSize())};
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto
ComputeObserverRecordsSize(std::uint32_t observer_record_capacity)
    -> ggems::units::Bytes {
  GGEMS_CHECK_RECOVERABLE(
      observer_record_capacity > 0U,
      "Transport observer record capacity must be non-zero.");

  return ggems::units::Bytes{
      static_cast<std::uint64_t>(observer_record_capacity) *
      sizeof(ObserverRecord)};
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto CheckedSourceCount(std::uint32_t source_count)
    -> std::uint32_t {
  GGEMS_CHECK_RECOVERABLE(source_count > 0U,
                          "Transport source count must be non-zero.");
  return source_count;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto RoundUp(std::size_t value, std::size_t multiple) noexcept
    -> std::size_t {
  return ((value + multiple - 1U) / multiple) * multiple;
}

// =============================================================================
// =============================================================================

auto ValidateRunConfig(TransportRunConfig const &config,
                       std::uint32_t stable_source_count,
                       std::uint32_t worker_count) -> void {
  GGEMS_CHECK_RECOVERABLE(config.total_primary_count > 0U,
                          "Transport primary count must be non-zero.");

  GGEMS_CHECK_RECOVERABLE(
      config.source_records.size() == config.source_ranges.size(),
      "Transport source record and range counts must match.");

  GGEMS_CHECK_RECOVERABLE(
      config.source_records.size() ==
          static_cast<std::size_t>(stable_source_count),
      "Transport source arrays do not match the stable source count.");

  std::uint32_t const safe_atomic_primary_count =
      std::numeric_limits<std::uint32_t>::max() - worker_count;

  GGEMS_CHECK_RECOVERABLE(
      config.total_primary_count <= safe_atomic_primary_count,
      std::format("Transport workload primary count {} exceeds the safe uint32 "
                  "atomic stream limit {} for {} workers.",
                  config.total_primary_count, safe_atomic_primary_count,
                  worker_count));

  ggems::core::transport::ValidateDiagnosticTransportSources(
      config.source_records, config.source_ranges);

  std::uint64_t projection_primary_count{0ULL};

  for (std::size_t source_index = 0U;
       source_index < config.source_ranges.size(); ++source_index) {
    SourceRunRange const &range = config.source_ranges[source_index];

    GGEMS_CHECK_RECOVERABLE(
        range.projection_primary_begin == projection_primary_count,
        std::format("Transport source range {} begins at {}, expected {}.",
                    source_index, range.projection_primary_begin,
                    projection_primary_count));

    GGEMS_CHECK_RECOVERABLE(
        range.primary_count <= std::numeric_limits<std::uint64_t>::max() -
                                   projection_primary_count,
        std::format("Transport source range {} overflows uint64.",
                    source_index));

    projection_primary_count += range.primary_count;
  }

  std::uint64_t const workload_primary_count = config.total_primary_count;

  GGEMS_CHECK_RECOVERABLE(
      config.device_primary_offset <=
          std::numeric_limits<std::uint64_t>::max() -
              (workload_primary_count - 1ULL),
      "Transport workload projection-primary interval overflows uint64.");

  std::uint64_t const last_projection_primary_id =
      config.device_primary_offset + workload_primary_count - 1ULL;

  GGEMS_CHECK_RECOVERABLE(
      last_projection_primary_id < projection_primary_count,
      "Transport workload primary interval is outside the source ranges.");

  GGEMS_CHECK_RECOVERABLE(
      config.projection_history_offset <=
          std::numeric_limits<std::uint64_t>::max() -
              last_projection_primary_id,
      "Transport workload global-primary interval overflows uint64.");
}
} // namespace

namespace ggems::core::transport {

GGEMSTransportWorkload::GGEMSTransportWorkload(
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
  InitialiseRandomStatesInSVM();
  ResetCountersInSVM();
  ResetObserverInSVM();
}

// -----------------------------------------------------------------------------

auto GGEMSTransportWorkload::InitialiseRandomStatesInSVM() -> void {
  std::uint64_t const state_bytes = random_states_buffer_.GetSize().value;

  GGEMS_CHECK_INTERNAL(
      state_bytes <= std::numeric_limits<std::size_t>::max(),
      "Random state buffer size exceeds host addressable storage.");

  auto *state_storage =
      static_cast<std::byte *>(random_states_buffer_.GetData());

  random_states_buffer_.Map(CL_MAP_WRITE);

  random_->InitialiseStates(
      random_stream_offset_,
      std::span<std::byte>{state_storage,
                           static_cast<std::size_t>(state_bytes)});

  random_states_buffer_.Unmap();
}

// -----------------------------------------------------------------------------

auto GGEMSTransportWorkload::ResetCountersInSVM() -> void {
  ggems::ocl::WriteSVMFromHost(counters_buffer_, TransportCounters{});
}

// -----------------------------------------------------------------------------

auto GGEMSTransportWorkload::ReadCountersFromSVM() -> GGEMSTransportCounters {
  return ggems::ocl::ReadSVMToHost<TransportCounters>(counters_buffer_);
}

// -----------------------------------------------------------------------------

auto GGEMSTransportWorkload::ReadObserverCountersFromSVM()
    -> observer::GGEMSObserverCounters {
  return ggems::ocl::ReadSVMToHost<ObserverCounters>(observer_counters_buffer_);
}

// -----------------------------------------------------------------------------

auto GGEMSTransportWorkload::ResetObserverInSVM() -> void {
  ggems::ocl::WriteSVMFromHost(observer_counters_buffer_, ObserverCounters{});
  ggems::ocl::FillSVMFromHost(observer_records_buffer_,
                              observer_record_capacity_, ObserverRecord{});
}

// -----------------------------------------------------------------------------

auto GGEMSTransportWorkload::WriteObserverConfigToSVM(
    observer::GGEMSObserverConfigRecord const &observer_config) -> void {
  ggems::ocl::WriteSVMFromHost(observer_config_buffer_, observer_config);
}

// -----------------------------------------------------------------------------

auto GGEMSTransportWorkload::ReadObserverRecordsFromSVM(
    std::uint32_t record_count) -> std::vector<observer::GGEMSObserverRecord> {
  std::uint32_t const bounded_record_count =
      std::min(record_count, observer_record_capacity_);

  std::vector<ObserverRecord> records(bounded_record_count);

  if (records.empty()) {
    return records;
  }

  ggems::ocl::ReadSVMToHost(observer_records_buffer_,
                            std::span<ObserverRecord>{records});

  return records;
}

// -----------------------------------------------------------------------------

auto GGEMSTransportWorkload::Run(GGEMSTransportRunConfig const &config)
    -> GGEMSTransportRunReport {
  ValidateRunConfig(config, source_count_, worker_count_);

  ResetCountersInSVM();
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
      kernel_root_ / "core" / "transport";

  std::string const observer_build_definition =
      config.observer_config.enabled != 0U
          ? "-DGGEMS_ENABLE_TRANSPORT_OBSERVER=1"
          : "-DGGEMS_ENABLE_TRANSPORT_OBSERVER=0";

  std::string const build_options = std::format(
      "-cl-std=CL2.0 -I{} {} {}", kernel_root_.generic_string(),
      random_->GetKernelBuildDefinition(), observer_build_definition);

  auto &program =
      opencl.GetOrCreateProgram(*context_, kernel_transport_root,
                                "particle_stream_transport", build_options);

  cl::Kernel raw_kernel = program.CreateKernel("particle_stream_transport");

  ggems::ocl::GGEMSOpenCLKernel kernel{*context_, std::move(raw_kernel),
                                       "particle_stream_transport"};

  constexpr cl_uint k_expected_argument_count{14U};
  cl_uint argument_index{0U};

  kernel.SetArgSVMPointer(argument_index++, random_states_buffer_.GetData());
  kernel.SetArgSVMPointer(argument_index++, counters_buffer_.GetData());
  kernel.SetArgSVMPointer(argument_index++, source_records_buffer_.GetData());
  kernel.SetArgSVMPointer(argument_index++, source_ranges_buffer_.GetData());
  kernel.SetArg(argument_index++, static_cast<cl_uint>(source_count_));
  kernel.SetArg(argument_index++,
                static_cast<cl_uint>(config.total_primary_count));
  kernel.SetArg(argument_index++,
                static_cast<cl_ulong>(config.projection_history_offset));
  kernel.SetArg(argument_index++,
                static_cast<cl_ulong>(config.device_primary_offset));
  kernel.SetArgSVMPointer(argument_index++, observer_config_buffer_.GetData());
  kernel.SetArgSVMPointer(argument_index++,
                          observer_counters_buffer_.GetData());
  kernel.SetArgSVMPointer(argument_index++, observer_records_buffer_.GetData());
  kernel.SetArg(argument_index++,
                static_cast<cl_uint>(observer_record_capacity_));
  kernel.SetArg(argument_index++, static_cast<cl_ulong>(config.run_id));
  kernel.SetArg(argument_index++, static_cast<cl_uint>(worker_count_));

  GGEMS_CHECK_INTERNAL(argument_index == k_expected_argument_count,
                       "Transport kernel argument count is inconsistent.");

  constexpr std::size_t k_local_size{64U};
  std::size_t const global_size = RoundUp(worker_count_, k_local_size);

  ggems::ocl::GGEMSOpenCLProfiler profiler{};
  profiler.Start();

  cl::Event event = kernel.RunAndGetEvent({global_size}, {k_local_size});

  profiler.Stop();
  profiler.RecordKernelEvent(event);

  TransportCounters const transport_counters = ReadCountersFromSVM();
  ObserverCounters const observer_counters = ReadObserverCountersFromSVM();
  std::vector<ObserverRecord> observer_records =
      ReadObserverRecordsFromSVM(observer_counters.record_count);

  GGEMSTransportRunReport report{};
  report.context_index = context_index_;
  report.device_name = device_name_;
  report.counters = transport_counters;
  report.observer_counters = observer_counters;
  report.observer_records = std::move(observer_records);
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
