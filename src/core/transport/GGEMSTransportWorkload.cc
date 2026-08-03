#include "GGEMS/core/transport/GGEMSTransportWorkload.hh"

#include <algorithm>

#if defined(GGEMS_ENABLE_INTERNAL_TEST_HOOKS)
#include <atomic>
#endif

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <limits>
#include <span>
#include <string>
#include <utility>
#include <vector>
#include <memory>

#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/core/random/GGEMSRandom.hh"
#include "GGEMS/core/sources/GGEMSSourceRecord.hh"
#include "GGEMS/core/sources/GGEMSEnergyDistributionRecord.hh"
#include "GGEMS/core/sources/GGEMSSourceRunSnapshot.hh"
#include "GGEMS/core/sources/GGEMSSourceRunRange.hh"
#include "GGEMS/core/sources/GGEMSSourcePopulationRecord.hh"
#include "GGEMS/core/sources/GGEMSSourcePopulation.hh"
#include "GGEMS/core/transport/GGEMSDiagnosticProjection.hh"
#include "GGEMS/core/transport/GGEMSTransportCounters.hh"
#include "GGEMS/core/transport/GGEMSTransportWorkloadPlan.hh"
#include "GGEMS/frameworks/GGEMSOpenCL.hh"
#include "GGEMS/frameworks/GGEMSOpenCLKernel.hh"
#include "GGEMS/frameworks/GGEMSOpenCLProfiler.hh"
#include "GGEMS/frameworks/GGEMSOpenCLSVMHostAccess.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideEmissionRecord.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideGroupRange.hh"
#include "GGEMS/core/observer/GGEMSObserverRecord.hh"
#include "GGEMS/core/units/GGEMSBytesUnits.hh"
#include "GGEMS/core/units/GGEMSTimeUnits.hh"
#include "GGEMS/core/units/GGEMSQuantity.hh"

namespace {

// =============================================================================
// =============================================================================

using ObserverConfigRecord = ggems::core::observer::GGEMSObserverConfigRecord;
using ObserverCounters = ggems::core::observer::GGEMSObserverCounters;
using ObserverRecord = ggems::core::observer::GGEMSObserverRecord;
using RadionuclideEmissionRecord =
    ggems::core::radioactivity::GGEMSRadionuclideEmissionRecord;
using RadionuclideGroupRange =
    ggems::core::radioactivity::GGEMSRadionuclideGroupRange;
using SourcePopulationRecord =
    ggems::core::sources::GGEMSSourcePopulationRecord;
using SourceRecord = ggems::core::sources::GGEMSSourceRecord;
using SourceRunRange = ggems::core::sources::GGEMSSourceRunRange;
using TransportCounters = ggems::core::transport::GGEMSTransportCounters;
using TransportRunConfig = ggems::core::transport::GGEMSTransportRunConfig;
using EnergyDistributionRecord =
    ggems::core::sources::GGEMSEnergyDistributionRecord;

#if defined(GGEMS_ENABLE_INTERNAL_TEST_HOOKS)
std::atomic<std::uint32_t> test_launch_primary_count_limit{0U};
std::atomic<std::uint32_t> test_fail_after_completed_chunk_count{0U};
#endif

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

[[nodiscard]] auto ComputeArrayBufferSize(std::uint64_t logical_entry_count,
                                          std::uint64_t element_size)
    -> ggems::units::Bytes {
  std::uint64_t const physical_entry_count =
      std::max(std::uint64_t{1U}, logical_entry_count);

  GGEMS_CHECK_RECOVERABLE(
      physical_entry_count <=
          std::numeric_limits<std::uint64_t>::max() / element_size,
      "Transport array buffer size overflows uint64 storage.");

  return ggems::units::Bytes{physical_entry_count * element_size};
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto CheckedSourceCount(std::size_t source_count)
    -> std::uint32_t {
  GGEMS_CHECK_RECOVERABLE(source_count > 0U,
                          "Transport source count must be non-zero.");

  GGEMS_CHECK_RECOVERABLE(std::in_range<std::uint32_t>(source_count),
                          "Transport source count exceeds uint32 storage.");
  return static_cast<std::uint32_t>(source_count);
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto CheckedEmissionCount(std::size_t emission_count)
    -> std::uint32_t {
  GGEMS_CHECK_RECOVERABLE(std::in_range<std::uint32_t>(emission_count),
                          "Transport emission count exceeds uint32 storage.");
  return static_cast<std::uint32_t>(emission_count);
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto CheckedEnergyDistributionRecordCount(
    ggems::core::sources::GGEMSSourceConfigurationSnapshot const
        &source_configuration) -> std::uint64_t {
  GGEMS_CHECK_RECOVERABLE(
      std::in_range<std::uint64_t>(
          source_configuration.GetEnergyDistributionRecords().size()),
      "Transport energy-distribution record count exceeds uint64 storage.");
  return static_cast<std::uint64_t>(
      source_configuration.GetEnergyDistributionRecords().size());
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto CheckedEnergyTableEntryCount(
    ggems::core::sources::GGEMSSourceConfigurationSnapshot const
        &source_configuration) -> std::uint64_t {
  GGEMS_CHECK_INTERNAL(
      source_configuration.GetEnergyValuesMilliElectronVolt().size() ==
          source_configuration.GetCumulativeTicketUpperBounds().size(),
      "Transport source configuration energy and ticket counts do not match.");
  GGEMS_CHECK_RECOVERABLE(
      std::in_range<std::uint64_t>(
          source_configuration.GetEnergyValuesMilliElectronVolt().size()),
      "Transport energy table entry count exceeds uint64 storage.");

  return static_cast<std::uint64_t>(
      source_configuration.GetEnergyValuesMilliElectronVolt().size());
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto RoundUp(std::size_t value, std::size_t multiple) noexcept
    -> std::size_t {
  return ((value + multiple - 1U) / multiple) * multiple;
}

// =============================================================================
// =============================================================================

inline constexpr std::uint32_t k_observer_records_per_primary{2U};

[[nodiscard]] auto
ComputeObserverSafeLaunchPrimaryCount(std::uint32_t transport_limit,
                                      bool observer_enabled) noexcept
    -> std::uint32_t {
  if (!observer_enabled) {
    return transport_limit;
  }

  std::uint32_t const observer_limit =
      std::numeric_limits<std::uint32_t>::max() /
      k_observer_records_per_primary;
  return std::min(transport_limit, observer_limit);
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto ResolveLaunchPrimaryCountLimit(std::uint32_t worker_count,
                                                  std::uint32_t requested_limit)
    -> std::uint32_t {
  std::uint32_t const safe_limit =
      ggems::core::transport::ComputeSafeTransportLaunchPrimaryCount(
          worker_count);

#if defined(GGEMS_ENABLE_INTERNAL_TEST_HOOKS)
  if (requested_limit == 0U) {
    requested_limit = test_launch_primary_count_limit.load();
  }
#endif

  if (requested_limit == 0U) {
    return safe_limit;
  }

  GGEMS_CHECK_RECOVERABLE(
      requested_limit <= safe_limit,
      "Requested transport launch-primary limit exceeds the safe uint32 "
      "atomic stream capacity.");
  return requested_limit;
}

// =============================================================================
// =============================================================================

auto ValidateRunConfigImpl(TransportRunConfig const &config,
                           std::uint32_t stable_source_count,
                           std::uint32_t stable_emission_count,
                           std::uint32_t worker_count,
                           std::uint32_t launch_primary_count_limit) -> void {
  GGEMS_CHECK_RECOVERABLE(config.total_primary_count > 0U,
                          "Transport primary count must be non-zero.");

  GGEMS_CHECK_RECOVERABLE(
      config.source_records.size() == config.source_ranges.size() &&
          config.source_records.size() ==
              config.source_population_records.size(),
      "Transport source record, population, and range counts must match.");

  GGEMS_CHECK_RECOVERABLE(
      config.source_records.size() ==
          static_cast<std::size_t>(stable_source_count),
      "Transport source arrays do not match the stable source count.");

  std::uint32_t const safe_atomic_primary_count =
      ggems::core::transport::ComputeSafeTransportLaunchPrimaryCount(
          worker_count);

  GGEMS_CHECK_RECOVERABLE(
      launch_primary_count_limit > 0U &&
          launch_primary_count_limit <= safe_atomic_primary_count,
      "Transport launch-primary limit exceeds the safe uint32 atomic stream "
      "capacity.");
  GGEMS_CHECK_RECOVERABLE(
      config.radionuclide_group_ranges.size() ==
          static_cast<std::size_t>(stable_emission_count),
      "Transport radionuclide group ranges do not match the stable emission "
      "count.");

  ggems::core::transport::ValidateDiagnosticTransportSources(
      config.source_records, config.source_ranges);

  std::uint64_t projection_primary_count{0ULL};

  std::uint64_t next_emission_index{0ULL};

  for (std::size_t source_index = 0U;
       source_index < config.source_ranges.size(); ++source_index) {
    SourceRunRange const &range = config.source_ranges[source_index];
    SourcePopulationRecord const &population =
        config.source_population_records[source_index];

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

    std::uint32_t const count_driven_mode =
        ggems::core::sources::ToKernelSourcePopulationMode(
            ggems::core::sources::GGEMSSourcePopulationMode::CountDriven);
    std::uint32_t const activity_driven_mode =
        ggems::core::sources::ToKernelSourcePopulationMode(
            ggems::core::sources::GGEMSSourcePopulationMode::ActivityDriven);

    if (population.population_mode == count_driven_mode) {
      GGEMS_CHECK_RECOVERABLE(
          population.first_emission_index == 0U &&
              population.emission_count == 0U &&
              population.scaled_decay == 0.0F,
          std::format("CountDriven source population record {} is not "
                      "canonical.",
                      source_index));
      continue;
    }

    GGEMS_CHECK_RECOVERABLE(
        population.population_mode == activity_driven_mode,
        std::format("Source population record {} has an unknown mode.",
                    source_index));
    GGEMS_CHECK_RECOVERABLE(
        std::isfinite(population.scaled_decay) &&
            population.scaled_decay >= 0.0F,
        std::format("ActivityDriven source population record {} has invalid "
                    "scaled decay.",
                    source_index));
    GGEMS_CHECK_RECOVERABLE(
        population.first_emission_index == next_emission_index,
        std::format("ActivityDriven source population record {} begins at "
                    "emission {}, expected {}.",
                    source_index, population.first_emission_index,
                    next_emission_index));

    std::uint64_t const emission_end =
        next_emission_index + population.emission_count;
    GGEMS_CHECK_RECOVERABLE(
        emission_end <= stable_emission_count,
        std::format("ActivityDriven source population record {} exceeds the "
                    "stable emission range.",
                    source_index));

    std::uint64_t source_local_primary_count{0ULL};
    for (std::uint64_t emission_index = next_emission_index;
         emission_index < emission_end; ++emission_index) {
      RadionuclideGroupRange const &group =
          config.radionuclide_group_ranges[static_cast<std::size_t>(
              emission_index)];
      GGEMS_CHECK_RECOVERABLE(
          group.source_local_primary_begin == source_local_primary_count,
          std::format("Radionuclide group range {} begins at {}, expected {}.",
                      emission_index, group.source_local_primary_begin,
                      source_local_primary_count));
      GGEMS_CHECK_RECOVERABLE(
          group.primary_count <= std::numeric_limits<std::uint64_t>::max() -
                                     source_local_primary_count,
          std::format("Radionuclide group range {} overflows uint64.",
                      emission_index));
      source_local_primary_count += group.primary_count;
    }
    GGEMS_CHECK_RECOVERABLE(
        source_local_primary_count == range.primary_count,
        std::format("ActivityDriven source population record {} group total "
                    "does not match its source range.",
                    source_index));
    next_emission_index = emission_end;
  }

  GGEMS_CHECK_RECOVERABLE(
      next_emission_index == stable_emission_count,
      "Transport source populations do not cover every stable emission.");

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

// =============================================================================
// =============================================================================

#if defined(GGEMS_ENABLE_INTERNAL_TEST_HOOKS)
auto SetTransportWorkloadTestControl(
    std::uint32_t launch_primary_count_limit,
    std::uint32_t fail_after_completed_chunk_count) noexcept -> void {
  test_launch_primary_count_limit.store(launch_primary_count_limit);
  test_fail_after_completed_chunk_count.store(fail_after_completed_chunk_count);
}
#endif

// =============================================================================
// =============================================================================

auto ValidateTransportRunConfig(GGEMSTransportRunConfig const &config,
                                std::uint32_t stable_source_count,
                                std::uint32_t stable_emission_count,
                                std::uint32_t worker_count,
                                std::uint32_t launch_primary_count_limit)
    -> void {
  ValidateRunConfigImpl(config, stable_source_count, stable_emission_count,
                        worker_count, launch_primary_count_limit);
}

// =============================================================================
// =============================================================================

GGEMSTransportWorkload::GGEMSTransportWorkload(
    ggems::ocl::GGEMSOpenCLContext &context, std::filesystem::path kernel_root,
    random::GGEMSRandom const &random, std::uint32_t worker_count,
    sources::GGEMSSourceConfigurationSnapshot const &source_configuration,
    std::uint64_t random_stream_offset, std::uint32_t context_index,
    std::uint32_t observer_record_capacity,
    std::uint32_t launch_primary_count_limit)
    : context_{&context}, kernel_root_{std::move(kernel_root)},
      random_{&random},
      random_kernel_build_definition_{random.GetKernelBuildDefinition()},
      worker_count_{worker_count},
      source_count_{CheckedSourceCount(source_configuration.GetSourceCount())},
      emission_count_{
          CheckedEmissionCount(source_configuration.GetEmissionCount())},
      random_stream_offset_{random_stream_offset},
      context_index_{context_index},
      device_name_{context.GetDevice().GetName()},
      observer_record_capacity_{observer_record_capacity},
      launch_primary_count_limit_{ResolveLaunchPrimaryCountLimit(
          worker_count, launch_primary_count_limit)},
      random_states_buffer_{context.CreateSVMBuffer(
          ComputeRandomStatesSize(random, worker_count, random_stream_offset))},
      counters_buffer_{context.CreateSVMBuffer(
          ggems::units::Bytes{sizeof(TransportCounters)})},
      source_records_buffer_{context.CreateSVMBuffer(ggems::units::Bytes{
          static_cast<std::uint64_t>(source_count_) * sizeof(SourceRecord)})},
      source_population_records_buffer_{context.CreateSVMBuffer(
          ggems::units::Bytes{static_cast<std::uint64_t>(source_count_) *
                              sizeof(SourcePopulationRecord)})},
      source_ranges_buffer_{context.CreateSVMBuffer(ggems::units::Bytes{
          static_cast<std::uint64_t>(source_count_) * sizeof(SourceRunRange)})},
      radionuclide_emission_records_buffer_{
          context.CreateSVMBuffer(ComputeArrayBufferSize(
              emission_count_, sizeof(RadionuclideEmissionRecord)))},
      radionuclide_group_ranges_buffer_{
          context.CreateSVMBuffer(ComputeArrayBufferSize(
              emission_count_, sizeof(RadionuclideGroupRange)))},
      energy_distribution_records_buffer_{
          context.CreateSVMBuffer(ComputeArrayBufferSize(
              CheckedEnergyDistributionRecordCount(source_configuration),
              sizeof(EnergyDistributionRecord)))},
      energy_values_buffer_{context.CreateSVMBuffer(ComputeArrayBufferSize(
          CheckedEnergyTableEntryCount(source_configuration),
          sizeof(std::uint64_t)))},
      cumulative_ticket_upper_buffer_{
          context.CreateSVMBuffer(ComputeArrayBufferSize(
              CheckedEnergyTableEntryCount(source_configuration),
              sizeof(std::uint64_t)))},
      observer_config_buffer_{context.CreateSVMBuffer(
          ggems::units::Bytes{sizeof(ObserverConfigRecord)})},
      observer_counters_buffer_{context.CreateSVMBuffer(
          ggems::units::Bytes{sizeof(ObserverCounters)})},
      observer_records_buffer_{context.CreateSVMBuffer(
          ComputeObserverRecordsSize(observer_record_capacity))} {
  auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();
  std::filesystem::path const kernel_transport_root =
      kernel_root_ / "core" / "transport";
  std::string const build_options = std::format(
      "-cl-std=CL2.0 -I{} {} "
      "-DGGEMS_ENABLE_TRANSPORT_OBSERVER=1",
      kernel_root_.generic_string(), random_kernel_build_definition_);
  auto &program =
      opencl.GetOrCreateProgram(*context_, kernel_transport_root,
                                "particle_stream_transport", build_options);
  cl::Kernel raw_kernel = program.CreateKernel("particle_stream_transport");
  kernel_ = std::make_unique<ggems::ocl::GGEMSOpenCLKernel>(
      *context_, std::move(raw_kernel), "particle_stream_transport");

  constexpr cl_uint k_expected_argument_count{20U};
  cl_uint argument_index{0U};
  kernel_->SetArgSVMPointer(argument_index++, random_states_buffer_.GetData());
  kernel_->SetArgSVMPointer(argument_index++, counters_buffer_.GetData());
  kernel_->SetArgSVMPointer(argument_index++, source_records_buffer_.GetData());
  kernel_->SetArgSVMPointer(argument_index++, source_ranges_buffer_.GetData());
  kernel_->SetArg(argument_index++, static_cast<cl_uint>(source_count_));
  kernel_->SetArg(argument_index++, cl_uint{0U});
  kernel_->SetArg(argument_index++, cl_ulong{0ULL});
  kernel_->SetArg(argument_index++, cl_ulong{0ULL});
  kernel_->SetArgSVMPointer(argument_index++,
                            observer_config_buffer_.GetData());
  kernel_->SetArgSVMPointer(argument_index++,
                            observer_counters_buffer_.GetData());
  kernel_->SetArgSVMPointer(argument_index++,
                            observer_records_buffer_.GetData());
  kernel_->SetArg(argument_index++, cl_uint{0U});
  kernel_->SetArg(argument_index++, cl_ulong{0ULL});
  kernel_->SetArg(argument_index++, static_cast<cl_uint>(worker_count_));
  kernel_->SetArgSVMPointer(argument_index++,
                            energy_distribution_records_buffer_.GetData());
  kernel_->SetArgSVMPointer(argument_index++, energy_values_buffer_.GetData());
  kernel_->SetArgSVMPointer(argument_index++,
                            cumulative_ticket_upper_buffer_.GetData());
  kernel_->SetArgSVMPointer(argument_index++,
                            source_population_records_buffer_.GetData());
  kernel_->SetArgSVMPointer(argument_index++,
                            radionuclide_emission_records_buffer_.GetData());
  kernel_->SetArgSVMPointer(argument_index++,
                            radionuclide_group_ranges_buffer_.GetData());
  GGEMS_CHECK_INTERNAL(argument_index == k_expected_argument_count,
                       "Transport kernel argument count is inconsistent.");

  auto const &radionuclide_emission_records =
      source_configuration.GetRadionuclideEmissionRecords();
  auto const &energy_distribution_records =
      source_configuration.GetEnergyDistributionRecords();
  auto const &energy_values_milli_eV =
      source_configuration.GetEnergyValuesMilliElectronVolt();
  auto const &cumulative_ticket_upper =
      source_configuration.GetCumulativeTicketUpperBounds();

  GGEMS_CHECK_INTERNAL(
      radionuclide_emission_records.size() == emission_count_,
      "Transport immutable radionuclide emission count is inconsistent.");
  GGEMS_CHECK_INTERNAL(
      energy_distribution_records.size() >= source_count_,
      "Transport immutable energy configuration lacks source-indexed "
      "records.");

  if (radionuclide_emission_records.empty()) {
    ggems::ocl::WriteSVMFromHost(radionuclide_emission_records_buffer_,
                                 RadionuclideEmissionRecord{});
    ggems::ocl::WriteSVMFromHost(radionuclide_group_ranges_buffer_,
                                 RadionuclideGroupRange{});
  } else {
    ggems::ocl::WriteSVMFromHost(radionuclide_emission_records_buffer_,
                                 std::span<RadionuclideEmissionRecord const>{
                                     radionuclide_emission_records});
  }

  ggems::ocl::WriteSVMFromHost(
      energy_distribution_records_buffer_,
      std::span<EnergyDistributionRecord const>{energy_distribution_records});

  if (energy_values_milli_eV.empty()) {
    ggems::ocl::WriteSVMFromHost(energy_values_buffer_, std::uint64_t{0ULL});
    ggems::ocl::WriteSVMFromHost(cumulative_ticket_upper_buffer_,
                                 std::uint64_t{0ULL});
  } else {
    ggems::ocl::WriteSVMFromHost(
        energy_values_buffer_,
        std::span<std::uint64_t const>{energy_values_milli_eV});
    ggems::ocl::WriteSVMFromHost(
        cumulative_ticket_upper_buffer_,
        std::span<std::uint64_t const>{cumulative_ticket_upper});
  }

  InitialiseRandomStatesInSVM();
  ResetCountersInSVM();
  ResetObserverCountersInSVM();
  ggems::ocl::FillSVMFromHost(observer_records_buffer_,
                              observer_record_capacity_, ObserverRecord{});
}

// -----------------------------------------------------------------------------

auto GGEMSTransportWorkload::ValidateRunConfig(
    GGEMSTransportRunConfig const &config) const -> void {
  ValidateTransportRunConfig(config, source_count_, emission_count_,
                             worker_count_, launch_primary_count_limit_);
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

auto GGEMSTransportWorkload::ResetObserverCountersInSVM() -> void {
  ggems::ocl::WriteSVMFromHost(observer_counters_buffer_, ObserverCounters{});
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
  ValidateRunConfig(config);

  ggems::ocl::WriteSVMFromHost(
      source_records_buffer_,
      std::span<SourceRecord const>{config.source_records});

  ggems::ocl::WriteSVMFromHost(source_population_records_buffer_,
                               std::span<SourcePopulationRecord const>{
                                   config.source_population_records});

  ggems::ocl::WriteSVMFromHost(
      source_ranges_buffer_,
      std::span<SourceRunRange const>{config.source_ranges});

  if (!config.radionuclide_group_ranges.empty()) {
    ggems::ocl::WriteSVMFromHost(radionuclide_group_ranges_buffer_,
                                 std::span<RadionuclideGroupRange const>{
                                     config.radionuclide_group_ranges});
  }

  WriteObserverConfigToSVM(config.observer_config);

  GGEMS_CHECK_INTERNAL(kernel_ != nullptr,
                       "Transport kernel is not initialised.");
  kernel_->SetArg(6U, static_cast<cl_ulong>(config.projection_history_offset));
  kernel_->SetArg(12U, static_cast<cl_ulong>(config.run_id));

  constexpr std::size_t k_local_size{64U};
  std::size_t const global_size = RoundUp(worker_count_, k_local_size);

  GGEMSTransportRunReport report{};
  report.context_index = context_index_;
  report.device_name = device_name_;
  report.observer_records.reserve(observer_record_capacity_);

  auto checked_add = [](std::uint64_t &destination, std::uint64_t value,
                        char const *diagnostic) -> void {
    GGEMS_CHECK_INTERNAL(value <= std::numeric_limits<std::uint64_t>::max() -
                                      destination,
                         diagnostic);
    destination += value;
  };

  std::uint32_t const logical_launch_primary_count_limit =
      ComputeObserverSafeLaunchPrimaryCount(
          launch_primary_count_limit_, config.observer_config.enabled != 0U);
  GGEMSTransportChunkIterator chunks{config.device_primary_offset,
                                     config.total_primary_count,
                                     logical_launch_primary_count_limit};

#if defined(GGEMS_ENABLE_INTERNAL_TEST_HOOKS)
  std::uint32_t completed_chunk_count{0U};
#endif

  while (chunks.HasNext()) {
    GGEMSTransportChunk const chunk = chunks.Next();
    std::size_t const remaining_observer_capacity =
        static_cast<std::size_t>(observer_record_capacity_) -
        report.observer_records.size();
    auto const chunk_observer_capacity =
        static_cast<std::uint32_t>(remaining_observer_capacity);

    ResetCountersInSVM();
    ResetObserverCountersInSVM();
    kernel_->SetArg(5U, static_cast<cl_uint>(chunk.primary_count));
    kernel_->SetArg(7U, static_cast<cl_ulong>(chunk.device_primary_offset));
    kernel_->SetArg(11U, static_cast<cl_uint>(chunk_observer_capacity));

    ggems::ocl::GGEMSOpenCLProfiler profiler{};
    profiler.Start();
    cl::Event event = kernel_->RunAndGetEvent({global_size}, {k_local_size});
    profiler.Stop();
    profiler.RecordKernelEvent(event);

    TransportCounters const counters = ReadCountersFromSVM();
    ObserverCounters const observer_counters = ReadObserverCountersFromSVM();
    std::vector<ObserverRecord> observer_records =
        ReadObserverRecordsFromSVM(observer_counters.record_count);

    GGEMS_CHECK_INTERNAL(
        counters.next_primary_id >= chunk.primary_count,
        "Transport chunk primary cursor stopped before its assigned range.");
    checked_add(report.counters.next_primary_id, chunk.primary_count,
                "Logical next-primary total overflows uint64.");
    checked_add(report.counters.consumed_primary_count,
                counters.consumed_primary_count,
                "Logical consumed-primary total overflows uint64.");
    checked_add(report.counters.completed_history_count,
                counters.completed_history_count,
                "Logical completed-history total overflows uint64.");
    checked_add(report.counters.terminal_particle_count,
                counters.terminal_particle_count,
                "Logical terminal-particle total overflows uint64.");
    checked_add(report.counters.created_secondary_count,
                counters.created_secondary_count,
                "Logical created-secondary total overflows uint64.");
    checked_add(report.counters.aionino_to_gamma_count,
                counters.aionino_to_gamma_count,
                "Logical Aionino-to-gamma total overflows uint64.");
    checked_add(report.counters.gamma_to_electron_count,
                counters.gamma_to_electron_count,
                "Logical gamma-to-electron total overflows uint64.");
    checked_add(report.counters.electron_to_electron_count,
                counters.electron_to_electron_count,
                "Logical electron-to-electron total overflows uint64.");
    checked_add(report.counters.overflow_count, counters.overflow_count,
                "Logical transport-overflow total overflows uint64.");
    checked_add(report.counters.total_fake_step_count,
                counters.total_fake_step_count,
                "Logical fake-step total overflows uint64.");
    report.counters.max_stack_depth =
        std::max(report.counters.max_stack_depth,
                 static_cast<std::uint64_t>(counters.max_stack_depth));

    checked_add(report.logical_observer_counters.captured_primary_count,
                observer_counters.captured_primary_count,
                "Logical captured-primary total overflows uint64.");
    checked_add(report.logical_observer_counters.overflow_count,
                observer_counters.overflow_count,
                "Logical observer-overflow total overflows uint64.");

    GGEMS_CHECK_INTERNAL(
        observer_records.size() <= remaining_observer_capacity,
        "Transport chunk exceeded its remaining logical Observer capacity.");
    report.observer_records.insert(report.observer_records.end(),
                                   observer_records.begin(),
                                   observer_records.end());

    checked_add(report.host_time.value, profiler.GetElapsedTime().value,
                "Logical host time overflows uint64 picoseconds.");
    checked_add(report.kernel_time.value, profiler.GetKernelTime().value,
                "Logical kernel time overflows uint64 picoseconds.");
    checked_add(report.command_time.value, profiler.GetCommandTime().value,
                "Logical command time overflows uint64 picoseconds.");

#if defined(GGEMS_ENABLE_INTERNAL_TEST_HOOKS)
    ++completed_chunk_count;
    std::uint32_t const failure_chunk_count =
        test_fail_after_completed_chunk_count.load();
    GGEMS_CHECK_RECOVERABLE(
        failure_chunk_count == 0U ||
            completed_chunk_count != failure_chunk_count,
        "Injected transport failure after a completed chunk.");
#endif
  }

  report.logical_observer_counters.record_count =
      report.observer_records.size();

  auto saturate_u32 = [](std::uint64_t value) noexcept -> std::uint32_t {
    return static_cast<std::uint32_t>(std::min(
        value,
        static_cast<std::uint64_t>(std::numeric_limits<std::uint32_t>::max())));
  };
  report.observer_counters.record_count =
      static_cast<std::uint32_t>(report.observer_records.size());
  report.observer_counters.overflow_count =
      saturate_u32(report.logical_observer_counters.overflow_count);
  report.observer_counters.captured_primary_count =
      saturate_u32(report.logical_observer_counters.captured_primary_count);

  auto compute_rate = [](std::uint64_t count,
                         ggems::units::Duration duration) noexcept -> double {
    if (duration.value == 0ULL) {
      return 0.0;
    }

    auto const seconds = ggems::units::TryConvertTo(duration, "s");
    return seconds.has_value() && *seconds > 0.0L
               ? static_cast<double>(count) / static_cast<double>(*seconds)
               : 0.0;
  };

  report.host_histories_per_second =
      compute_rate(report.counters.completed_history_count, report.host_time);
  report.kernel_histories_per_second =
      compute_rate(report.counters.completed_history_count, report.kernel_time);
  report.host_terminal_particles_per_second =
      compute_rate(report.counters.terminal_particle_count, report.host_time);
  report.kernel_terminal_particles_per_second =
      compute_rate(report.counters.terminal_particle_count, report.kernel_time);

  return report;
}

} // namespace ggems::core::transport
