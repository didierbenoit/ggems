#include <filesystem>
#include <algorithm>
#include <memory>
#include <mutex>
#include <optional>
#include <exception>
#include <thread>
#include <atomic>
#include <vector>
#include <limits>
#include <utility>
#include <cstdint>
#include <cstddef>
#include <format>
#include <span>
#include <string>
#include <type_traits>

#include "GGEMS/core/GGEMSRun.hh"
#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/core/units/GGEMSTimeUnits.hh"
#include "GGEMS/core/random/GGEMSRandom.hh"
#include "GGEMS/core/sources/GGEMSSourceDescription.hh"
#include "GGEMS/core/sources/GGEMSSourcePopulation.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideEmissionPlan.hh"
#include "GGEMS/core/sources/GGEMSSourceRunSnapshot.hh"
#include "GGEMS/core/sources/GGEMSSourceRunRange.hh"
#include "GGEMS/frameworks/GGEMSOpenCL.hh"
#include "GGEMS/core/transport/GGEMSTransportWorkloadPlan.hh"
#include "GGEMS/core/transport/GGEMSTransportCounters.hh"
#include "GGEMS/core/transport/GGEMSTransportWorkload.hh"
#include "GGEMS/core/observer/GGEMSTransportObserver.hh"
#include "GGEMS/core/observer/GGEMSObserverRecord.hh"
#include "GGEMS/core/transport/GGEMSDiagnosticProjection.hh"
#include "GGEMS/core/GGEMSTimeWindow.hh"

using namespace ggems::units;

namespace ggems::core {
namespace {

// =============================================================================
// =============================================================================

struct RunningGuard {
  std::atomic<bool> &running;
  ~RunningGuard() { running.store(false); }
};

// =============================================================================
// =============================================================================

static_assert(
    std::is_nothrow_move_assignable_v<sources::GGEMSSourceRunSnapshot>);

// =============================================================================
// =============================================================================

[[nodiscard]] auto SaturateToUint32(std::uint64_t value) noexcept
    -> std::uint32_t {
  return static_cast<std::uint32_t>(std::min(
      value,
      static_cast<std::uint64_t>(std::numeric_limits<std::uint32_t>::max())));
}

// =============================================================================
// =============================================================================

auto CheckedAccumulate(std::uint64_t &destination, std::uint64_t value,
                       char const *diagnostic) -> void {
  GGEMS_CHECK_RECOVERABLE(value <= std::numeric_limits<std::uint64_t>::max() -
                                       destination,
                          diagnostic);
  destination += value;
}

// =============================================================================
// =============================================================================

auto AccumulateTransportCounters(
    transport::GGEMSTransportLogicalCounters &dst,
    transport::GGEMSTransportLogicalCounters const &src) -> void {
  CheckedAccumulate(dst.next_primary_id, src.next_primary_id,
                    "Transport primary cursor aggregation overflows uint64.");
  CheckedAccumulate(dst.consumed_primary_count, src.consumed_primary_count,
                    "Transport consumed-primary aggregation overflows uint64.");
  CheckedAccumulate(dst.completed_history_count, src.completed_history_count,
                    "Transport history aggregation overflows uint64.");
  CheckedAccumulate(dst.terminal_particle_count, src.terminal_particle_count,
                    "Transport terminal aggregation overflows uint64.");
  CheckedAccumulate(dst.created_secondary_count, src.created_secondary_count,
                    "Transport secondary aggregation overflows uint64.");
  CheckedAccumulate(dst.aionino_to_gamma_count, src.aionino_to_gamma_count,
                    "Transport Aionino transition aggregation overflows.");
  CheckedAccumulate(dst.gamma_to_electron_count, src.gamma_to_electron_count,
                    "Transport Gamma transition aggregation overflows.");
  CheckedAccumulate(dst.electron_to_electron_count,
                    src.electron_to_electron_count,
                    "Transport Electron transition aggregation overflows.");
  CheckedAccumulate(dst.overflow_count, src.overflow_count,
                    "Transport overflow aggregation overflows uint64.");
  CheckedAccumulate(dst.total_fake_step_count, src.total_fake_step_count,
                    "Transport fake-step aggregation overflows uint64.");

  dst.max_stack_depth = std::max(dst.max_stack_depth, src.max_stack_depth);
}

// =============================================================================
// =============================================================================

auto ValidateObserverCapture(
    observer::GGEMSObserverConfigRecord const &config,
    std::span<sources::GGEMSSourceRunRange const> source_ranges) -> void {
  if (config.enabled == 0U || config.capture_specific_primary_enabled == 0U) {
    return;
  }

  std::uint32_t const source_index = config.capture_source_index;

  GGEMS_CHECK_RECOVERABLE(
      static_cast<std::size_t>(source_index) < source_ranges.size(),
      std::format(
          "Observer source index {} is outside the current source snapshot.",
          source_index));

  std::uint64_t const source_primary_count =
      source_ranges[static_cast<std::size_t>(source_index)].primary_count;

  GGEMS_CHECK_RECOVERABLE(
      config.capture_source_local_primary_id < source_primary_count,
      std::format("Observer primary index {} is outside source slot {}, which "
                  "contains {} primaries.",
                  config.capture_source_local_primary_id, source_index,
                  source_primary_count));
}

} // namespace

// =============================================================================
// =============================================================================

GGEMSRun::GGEMSRun() : sources_{std::make_shared<sources::GGEMSSource>()} {
  GGEMS_INFOEX("Core", 3, "GGEMSRun instance created.");
}

// -----------------------------------------------------------------------------

GGEMSRun::~GGEMSRun() = default;

// -----------------------------------------------------------------------------

auto GGEMSRun::GetLastSourceRunSnapshot() const
    -> std::optional<sources::GGEMSSourceRunSnapshot> {
  std::scoped_lock lock{source_run_snapshot_mutex_};
  return last_source_run_snapshot_;
}

// -----------------------------------------------------------------------------

auto GGEMSRun::HasObserver() const noexcept -> bool {
  return observer_ != nullptr;
}

// -----------------------------------------------------------------------------

auto GGEMSRun::SetTimePicoSecond(std::uint64_t start_ps, std::uint64_t stop_ps,
                                 std::uint64_t step_ps) -> void {
  GGEMS_CHECK_RECOVERABLE(!initialised_,
                          "Cannot configure GGEMSRun time after Initalise.");
  GGEMS_CHECK_RECOVERABLE(
      start_ps < stop_ps,
      "GGEMSRun time start must be strictly less than time stop.");
  GGEMS_CHECK_RECOVERABLE(step_ps > 0ULL,
                          "GGEMSRun time step must be non-zero");

  time_start_ps_ = start_ps;
  time_stop_ps_ = stop_ps;
  time_step_ps_ = step_ps;
  current_time_ps_.store(start_ps);
  has_time_configuration_ = true;
}

// -----------------------------------------------------------------------------

auto GGEMSRun::ResetTime() -> void {
  GGEMS_CHECK_RECOVERABLE(!running_.exchange(true),
                          "Cannot reset GGEMSRun time while Run is executing.");

  RunningGuard running_guard{running_};
  current_time_ps_.store(has_time_configuration_ ? time_start_ps_ : 0ULL);
}

// -----------------------------------------------------------------------------

auto GGEMSRun::HasTimeConfiguration() const noexcept -> bool {
  return has_time_configuration_;
}

// -----------------------------------------------------------------------------

auto GGEMSRun::HasNextTimeStep() const noexcept -> bool {
  return !has_time_configuration_ || current_time_ps_.load() < time_stop_ps_;
}

// -----------------------------------------------------------------------------

auto GGEMSRun::GetCurrentTimePicoSecond() const noexcept -> std::uint64_t {
  return current_time_ps_.load();
}

// -----------------------------------------------------------------------------

auto GGEMSRun::GetCurrentTimeWindowPicoSecond() const noexcept
    -> GGEMSTimeWindow {
  if (!has_time_configuration_) {
    return {};
  }

  std::uint64_t const current_time_ps = current_time_ps_.load();

  if (current_time_ps >= time_stop_ps_) {
    return {.start_ps = time_stop_ps_, .stop_ps = time_stop_ps_};
  }

  std::uint64_t const remaining_time_ps = time_stop_ps_ - current_time_ps;
  std::uint64_t const window_width_ps =
      std::min(time_step_ps_, remaining_time_ps);

  return {.start_ps = current_time_ps,
          .stop_ps = current_time_ps + window_width_ps};
}

// -----------------------------------------------------------------------------

auto GGEMSRun::SetRandom(std::shared_ptr<random::GGEMSRandom> random) -> void {
  GGEMS_CHECK_RECOVERABLE(random != nullptr,
                          "Cannot attach a null GGEMSRandom to GGEMSRun.");

  GGEMS_CHECK_RECOVERABLE(!initialised_,
                          "Cannot change GGEMSRandom after Initialise.");

  random_ = std::move(random);

  GGEMS_INFO("Random", "GGEMSRun random engine set to '{}'.",
             random_->GetEngineName());
}

// -----------------------------------------------------------------------------

auto GGEMSRun::SetSource(std::shared_ptr<sources::GGEMSSource> source) -> void {
  GGEMS_CHECK_RECOVERABLE(source != nullptr,
                          "Cannot attach a null GGEMSSource to GGEMSRun.");

  GGEMS_CHECK_RECOVERABLE(!initialised_,
                          "Cannot change GGEMSSource after Initialise.");

  sources_.clear();
  sources_.push_back(std::move(source));
  uses_implicit_default_source_ = false;

  GGEMS_INFO("Source", "GGEMSRun source attached.");
}

// -----------------------------------------------------------------------------

auto GGEMSRun::AddSource(std::shared_ptr<sources::GGEMSSource> source) -> void {
  GGEMS_CHECK_RECOVERABLE(source != nullptr,
                          "Cannot attach a null GGEMSSource to GGEMSRun.");

  GGEMS_CHECK_RECOVERABLE(!initialised_,
                          "Cannot add a GGEMSSource after Initalise.");

  if (uses_implicit_default_source_) {
    sources_.clear();
  }

  sources_.push_back(std::move(source));
  uses_implicit_default_source_ = false;

  GGEMS_INFO("Source", "GGEMSRun source attaches at slot {}.",
             sources_.size() - 1U);
}

// -----------------------------------------------------------------------------

auto GGEMSRun::SetObserver(
    std::shared_ptr<observer::GGEMSTransportObserver> observer) -> void {
  GGEMS_CHECK_RECOVERABLE(
      observer != nullptr,
      "Cannot attach a null GGEMSTransportObserver to GGEMSRun.");

  GGEMS_CHECK_RECOVERABLE(
      !initialised_, "Cannot change GGEMSTransportObserver after Initialise.");

  observer_ = std::move(observer);

  GGEMS_INFO("Observer", "GGEMSRun transport observer attached.");
}

// -----------------------------------------------------------------------------

auto GGEMSRun::SetPrimaryCount(std::uint32_t primary_count) -> void {
  GGEMS_CHECK_RECOVERABLE(!initialised_,
                          "Cannot change primary count after Initialise.");

  GGEMS_CHECK_RECOVERABLE(primary_count > 0U,
                          "GGEMSRun primary count must be non-zero.");

  GGEMS_CHECK_RECOVERABLE(
      sources_.size() == 1U,
      "GGEMSRun::SetPrimaryCount is ambiguous with multiple sources. "
      "Configure each GGEMSSource primary count directly.");

  sources_.front()->SetPrimaryCount(primary_count);
}

// -----------------------------------------------------------------------------

auto GGEMSRun::SetWorkerCount(std::uint32_t worker_count) -> void {
  GGEMS_CHECK_RECOVERABLE(worker_count > 0ULL,
                          "GGEMSRun worker count must be non-zero.");

  GGEMS_CHECK_RECOVERABLE(!initialised_,
                          "Cannot change worker count after Initialise.");

  worker_count_ = worker_count;
}

// -----------------------------------------------------------------------------

auto GGEMSRun::Initialise() -> void {
  GGEMS_CHECK_RECOVERABLE(!initialised_,
                          "GGEMSRun::Initialise called more than once.");

  GGEMS_CHECK_INTERNAL(!sources_.empty(),
                       "GGEMSRun source collection must not be empty.");

  bool has_activity_driven_source{false};

  for (std::size_t source_index = 0U; source_index < sources_.size();
       ++source_index) {
    auto const &source = sources_[source_index];
    GGEMS_CHECK_INTERNAL(source != nullptr,
                         "GGEMSRun source collection contains a null entry.");

    if (source->GetPopulationMode() ==
        sources::GGEMSSourcePopulationMode::ActivityDriven) {
      has_activity_driven_source = true;
      GGEMS_CHECK_RECOVERABLE(
          has_time_configuration_ && time_start_ps_ < time_stop_ps_ &&
              time_step_ps_ > 0ULL,
          "ActivityDriven GGEMSRun sources require a configured non-empty "
          "time schedule.");
      GGEMS_CHECK_RECOVERABLE(
          source->GetActivityDrivenConfiguration().reference_time_ps <=
              time_start_ps_,
          std::format(
              "ActivityDriven source slot {} reference time must not follow "
              "the configured GGEMSRun start time.",
              source_index));
    }
  }

  GGEMS_CHECK_INTERNAL(!has_activity_driven_source ||
                           (has_time_configuration_ &&
                            time_start_ps_ < time_stop_ps_ &&
                            time_step_ps_ > 0ULL),
                       "ActivityDriven chronology validation is inconsistent.");

  GGEMS_CHECK_RECOVERABLE(
      sources_.size() <=
          static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max()),
      "GGEMSRun source slot count exceeds uint32 storage.");

  auto const source_count = static_cast<std::uint32_t>(sources_.size());

  GGEMS_CHECK_RECOVERABLE(
      random_ != nullptr,
      "GGEMSRun cannot be initialised without a GGEMSRandom. "
      "Create a ggems.rndm GGEMSRandom object and attach it with "
      "GGEMSRun::SetRandom before calling Initialise.");

  auto &opencl = ocl::GGEMSOpenCL::GetInstance();

  GGEMS_CHECK_RECOVERABLE(!opencl.GetContext().empty(),
                          "GGEMSRun requires initialised OpenCL contexts.");

  GGEMS_CHECK_RECOVERABLE(
      opencl.GetContext().size() <=
          static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max()),
      "GGEMSRun OpenCL context count exceeds uint32 storage.");
  (void)transport::ComputeSafeTransportLaunchPrimaryCount(worker_count_);

  GGEMS_INFO("Core", "Initialising GGEMSRun stable state...");

  GGEMS_INFO("Random", "Random engine ready: {} with seed {}.",
             random_->GetEngineName(), random_->GetSeed());

  std::uint32_t observer_record_capacity =
      observer_ != nullptr ? observer_->GetRecordCapacity() : 1U;

  auto new_radionuclide_emission_planner =
      std::make_unique<radioactivity::GGEMSRadionuclideEmissionPlanner>(
          sources_, *random_);
  auto new_source_configuration =
      sources::BuildSourceConfigurationSnapshot(sources_);

  std::filesystem::path kernel_root{GGEMS_KERNEL_ROOT};

  std::vector<std::unique_ptr<transport::GGEMSTransportWorkload>>
      new_transport_workloads;
  new_transport_workloads.reserve(opencl.GetContext().size());

  for (std::size_t context_index = 0U;
       context_index < opencl.GetContext().size(); ++context_index) {
    GGEMS_CHECK_RECOVERABLE(
        context_index <= std::numeric_limits<std::uint64_t>::max() /
                             static_cast<std::uint64_t>(worker_count_),
        "GGEMSRun random stream offset overflows uint64 storage.");
    std::uint64_t random_stream_offset =
        static_cast<std::uint64_t>(context_index) *
        static_cast<std::uint64_t>(worker_count_);

    new_transport_workloads.push_back(
        std::make_unique<transport::GGEMSTransportWorkload>(
            opencl.GetContext()[context_index], kernel_root, *random_,
            worker_count_, *new_source_configuration, random_stream_offset,
            static_cast<std::uint32_t>(context_index),
            observer_record_capacity));
  }

  GGEMS_INFO("Core", "{} transport workload(s) prepared.",
             new_transport_workloads.size());
  GGEMS_INFO("Source", "GGEMSRun source collection prepared with {} slot(s).",
             source_count);

  primary_stream_.Initialise();

  transport_workloads_.swap(new_transport_workloads);
  source_configuration_snapshot_ = std::move(new_source_configuration);
  radionuclide_emission_planner_ = std::move(new_radionuclide_emission_planner);
  next_run_id_ = 0ULL;
  current_time_ps_.store(has_time_configuration_ ? time_start_ps_ : 0ULL);

  for (auto const &source : sources_) {
    source->FinalizeInitialization();
  }

  initialised_ = true;
}

// -----------------------------------------------------------------------------

auto GGEMSRun::Run() -> void {
  GGEMS_CHECK_RECOVERABLE(initialised_,
                          "GGEMSRun::Run called before Initialise.");

  GGEMS_CHECK_RECOVERABLE(!running_.exchange(true),
                          "GGEMSRun is already running.");

  RunningGuard running_guard{running_};

  GGEMS_CHECK_RECOVERABLE(HasNextTimeStep(),
                          "GGEMSRun time schedule is exhausted.");

  GGEMSTimeWindow const time_window = GetCurrentTimeWindowPicoSecond();
  GGEMS_CHECK_RECOVERABLE(next_run_id_ <
                              std::numeric_limits<std::uint64_t>::max(),
                          "GGEMSRun identifier space is exhausted.");
  std::uint64_t const run_id = next_run_id_;
  GGEMS_CHECK_INTERNAL(
      radionuclide_emission_planner_ != nullptr,
      "GGEMSRun radionuclide emission planner was not initialised.");
  GGEMS_CHECK_RECOVERABLE(
      radionuclide_emission_planner_->GetRevision() <
          std::numeric_limits<std::uint64_t>::max(),
      "GGEMSRun radionuclide emission planner revision is exhausted.");

  auto emission_candidate =
      radionuclide_emission_planner_->BuildCandidate(time_window);

  auto source_snapshot = sources::BuildSourceRunSnapshot(
      sources_, source_configuration_snapshot_, emission_candidate.GetPlan());

  auto const &source_records = source_snapshot.GetRecords();
  auto const &source_ranges = source_snapshot.GetRanges();
  auto const &source_population_records =
      source_snapshot.GetPopulationRecords();
  auto const &radionuclide_group_ranges = source_snapshot.GetGroupRanges();

  GGEMS_CHECK_INTERNAL(
      source_records.size() == source_ranges.size() &&
          source_records.size() == source_population_records.size(),
      "GGEMSRun source snapshot component counts do not match.");

  GGEMS_CHECK_INTERNAL(!source_records.empty(),
                       "GGEMSRun source snapshot must not be empty.");

  std::uint64_t const total_primary_count =
      source_snapshot.GetTotalPrimaryCount();

  GGEMS_CHECK_RECOVERABLE(
      total_primary_count > 0ULL || has_time_configuration_,
      "GGEMSRun requires a non-zero total primary count in static mode.");

  for (std::size_t source_index = 0U; source_index < source_records.size();
       ++source_index) {
    GGEMS_INFOEX("Source", 1, "Run {} source snapshot: {}", run_id,
                 sources::DescribeSourceRunSlot(source_index, source_snapshot));
  }

  observer::GGEMSObserverConfigRecord observer_config{};

  if (observer_ != nullptr) {
    observer_config = observer_->BuildConfigRecord();
  }

  ValidateObserverCapture(observer_config, source_ranges);

  transport::ValidateDiagnosticTransportSources(source_records, source_ranges);

  GGEMS_CHECK_RECOVERABLE(!transport_workloads_.empty(),
                          "No transport workload was initialised.");

  if (total_primary_count == 0ULL) {
    std::unique_ptr<observer::GGEMSTransportObserver> observer_result_candidate;

    if (observer_ != nullptr) {
      observer_result_candidate =
          std::unique_ptr<observer::GGEMSTransportObserver>{
              new observer::GGEMSTransportObserver{false}};
    }

    std::unique_lock snapshot_lock{source_run_snapshot_mutex_};

    GGEMS_INFO("Core",
               "GGEMSRun projection {} completed as an empty time window "
               "[{} ps, {} ps).",
               run_id, time_window.start_ps, time_window.stop_ps);

    radionuclide_emission_planner_->CommitCandidate(emission_candidate);
    ++next_run_id_;

    if (observer_result_candidate != nullptr) {
      observer_->SwapRunResult(*observer_result_candidate);
    }

    last_source_run_snapshot_ = std::move(source_snapshot);
    current_time_ps_.store(time_window.stop_ps);
    return;
  }

  std::vector<transport::GGEMSTransportWorkloadPlan> workload_plan =
      transport::BuildEqualTransportWorkloadPlan(
          0ULL, total_primary_count,
          static_cast<std::uint32_t>(transport_workloads_.size()),
          worker_count_);

  std::vector<transport::GGEMSTransportRunReport> reports{workload_plan.size()};

  std::vector<std::exception_ptr> exceptions{workload_plan.size()};

  std::vector<transport::GGEMSTransportRunConfig> transport_configs{
      workload_plan.size()};

  for (std::size_t plan_index = 0U; plan_index < workload_plan.size();
       ++plan_index) {
    transport::GGEMSTransportWorkloadPlan const &workload =
        workload_plan[plan_index];

    if (workload.primary_count == 0U) {
      continue;
    }

    auto &config = transport_configs[plan_index];
    config.run_id = run_id;
    config.observer_config = observer_config;
    config.total_primary_count = workload.primary_count;
    config.projection_history_offset = 0ULL;
    config.device_primary_offset = workload.device_primary_offset;
    config.source_records = source_records;
    config.source_population_records = source_population_records;
    config.source_ranges = source_ranges;
    config.radionuclide_group_ranges = radionuclide_group_ranges;
    transport_workloads_[workload.context_index]->ValidateRunConfig(config);
  }

  auto primary_view = primary_stream_.PrepareRun(run_id, total_primary_count);
  std::uint64_t const reserved_primary_count =
      primary_view.source_primary_count;
  ++next_run_id_;

  for (std::size_t plan_index = 0U; plan_index < workload_plan.size();
       ++plan_index) {
    workload_plan[plan_index].projection_history_offset =
        primary_view.global_history_offset;
    transport_configs[plan_index].projection_history_offset =
        primary_view.global_history_offset;
  }

  GGEMS_INFO("Core", "GGEMSRun projection {} started.", run_id);
  GGEMS_INFOEX("Core", 1,
               "Projection {} primary stream: {} primaries, global history "
               "offset {}.",
               run_id, reserved_primary_count,
               primary_view.global_history_offset);
  GGEMS_INFOEX("Core", 1, "Projection {} worker count: {}.", run_id,
               worker_count_);

  {
    std::vector<std::jthread> transport_threads;
    transport_threads.reserve(workload_plan.size());

    for (std::size_t plan_index = 0U; plan_index < workload_plan.size();
         ++plan_index) {
      transport::GGEMSTransportWorkloadPlan const &workload =
          workload_plan[plan_index];

      if (workload.primary_count == 0U) {
        continue;
      }

      std::uint32_t context_index = workload.context_index;

      transport_threads.emplace_back(
          [&, plan_index, context_index,
           config = std::move(transport_configs[plan_index])]() -> void {
            try {
              reports[plan_index] =
                  transport_workloads_[context_index]->Run(config);
            } catch (...) {
              exceptions[plan_index] = std::current_exception();
            }
          });
    }
  }

  for (std::exception_ptr const &exception : exceptions) {
    if (exception != nullptr) {
      std::rethrow_exception(exception);
    }
  }

  transport::GGEMSTransportLogicalCounters merged_counters{};

  std::uint64_t accumulated_host_time_ps{0ULL};
  std::uint64_t accumulated_command_time_ps{0ULL};
  std::uint64_t accumulated_kernel_time_ps{0ULL};

  for (std::size_t plan_index = 0U; plan_index < workload_plan.size();
       ++plan_index) {
    transport::GGEMSTransportWorkloadPlan &workload = workload_plan[plan_index];

    if (workload.primary_count == 0U) {
      continue;
    }

    auto const &report = reports[plan_index];
    auto const &counters = report.counters;

    AccumulateTransportCounters(merged_counters, counters);

    CheckedAccumulate(accumulated_host_time_ps, report.host_time.value,
                      "Transport host-time aggregation overflows uint64.");
    CheckedAccumulate(accumulated_command_time_ps, report.command_time.value,
                      "Transport command-time aggregation overflows uint64.");
    CheckedAccumulate(accumulated_kernel_time_ps, report.kernel_time.value,
                      "Transport kernel-time aggregation overflows uint64.");

    GGEMS_INFO(
        "Core",
        "Projection {} device workload {} [{}: '{}'] report: "
        "primary_offset={}, assigned_primaries={}, consumed_primaries={}, "
        "histories={}, secondaries={}, kernel_time={}, host_time={}, "
        "kernel_histories/s={}.",
        run_id, workload.workload_index, workload.context_index,
        report.device_name, workload.device_primary_offset,
        workload.primary_count, counters.consumed_primary_count,
        counters.completed_history_count, counters.created_secondary_count,
        report.kernel_time, report.host_time,
        report.kernel_histories_per_second);
  }

  GGEMS_CHECK_RECOVERABLE(merged_counters.overflow_count == 0U,
                          "Transport reported an internal overflow.");

  GGEMS_CHECK_RECOVERABLE(
      merged_counters.consumed_primary_count == total_primary_count,
      "Transport consumed primary count does not match projection count.");

  GGEMS_CHECK_RECOVERABLE(
      merged_counters.next_primary_id == total_primary_count,
      "Transport logical primary cursor does not match projection count.");

  GGEMS_CHECK_RECOVERABLE(
      merged_counters.completed_history_count == total_primary_count,
      "Transport completed history count does not match projection count.");

  GGEMS_CHECK_RECOVERABLE(
      merged_counters.terminal_particle_count == total_primary_count,
      "Transport terminal particle count does not match projection count.");

  GGEMS_CHECK_RECOVERABLE(
      merged_counters.created_secondary_count == 0U,
      "Transport unexpectedly created secondary particles.");

  GGEMS_CHECK_RECOVERABLE(
      merged_counters.aionino_to_gamma_count == 0U &&
          merged_counters.gamma_to_electron_count == 0U &&
          merged_counters.electron_to_electron_count == 0U,
      "Transport unexpectedly reported a dummy process transition.");

  GGEMS_CHECK_RECOVERABLE(merged_counters.max_stack_depth == 0U,
                          "Transport unexpectedly used a secondary stack.");

  GGEMS_CHECK_RECOVERABLE(merged_counters.total_fake_step_count == 0U,
                          "Transport unexpectedly reported dummy fake steps.");

  GGEMS_INFO("Core",
             "Projection {} merged transport report: primaries={}, "
             "histories={}, secondaries={}, terminal_particles={}, "
             "fake_steps={}, max_stack_depth={}, overflow={}.",
             run_id, merged_counters.consumed_primary_count,
             merged_counters.completed_history_count,
             merged_counters.created_secondary_count,
             merged_counters.terminal_particle_count,
             merged_counters.total_fake_step_count,
             merged_counters.max_stack_depth, merged_counters.overflow_count);

  GGEMS_INFO("Core",
             "Projection {} accumulated timing report: host_time={}, "
             "command_time={}, kernel_time={}.",
             run_id, ggems::units::Time{accumulated_host_time_ps},
             ggems::units::Time{accumulated_command_time_ps},
             ggems::units::Time{accumulated_kernel_time_ps});

  std::unique_ptr<observer::GGEMSTransportObserver> observer_result_candidate;
  std::string observer_dump;

  if (observer_ != nullptr) {
    observer_result_candidate =
        std::unique_ptr<observer::GGEMSTransportObserver>{
            new observer::GGEMSTransportObserver{false}};
    observer_result_candidate->max_stored_record_count_ =
        observer_->max_stored_record_count_;

    std::uint64_t logical_record_count{0ULL};
    std::uint64_t logical_overflow_count{0ULL};
    std::uint64_t logical_captured_primary_count{0ULL};

    for (std::size_t plan_index = 0U; plan_index < workload_plan.size();
         ++plan_index) {
      if (workload_plan[plan_index].primary_count == 0ULL) {
        continue;
      }

      auto const &report = reports[plan_index];
      GGEMS_CHECK_INTERNAL(
          report.logical_observer_counters.record_count ==
              report.observer_records.size(),
          "Transport logical Observer record count does not match its "
          "candidate records.");
      CheckedAccumulate(
          logical_record_count, report.logical_observer_counters.record_count,
          "Observer logical record aggregation overflows uint64.");
      CheckedAccumulate(
          logical_overflow_count,
          report.logical_observer_counters.overflow_count,
          "Observer logical overflow aggregation overflows uint64.");
      CheckedAccumulate(
          logical_captured_primary_count,
          report.logical_observer_counters.captured_primary_count,
          "Observer logical captured-primary aggregation overflows uint64.");
    }

    std::size_t const stored_record_limit =
        observer_result_candidate->max_stored_record_count_;
    auto const reserve_count = static_cast<std::size_t>(
        std::min<std::uint64_t>(logical_record_count, stored_record_limit));
    observer_result_candidate->records_.reserve(reserve_count);

    for (std::size_t plan_index = 0U; plan_index < workload_plan.size();
         ++plan_index) {
      if (workload_plan[plan_index].primary_count == 0ULL) {
        continue;
      }

      auto const &records = reports[plan_index].observer_records;
      std::size_t const remaining_capacity =
          observer_result_candidate->records_.size() < stored_record_limit
              ? stored_record_limit - observer_result_candidate->records_.size()
              : 0U;
      std::size_t const copied_record_count =
          std::min(records.size(), remaining_capacity);

      observer_result_candidate->records_.insert(
          observer_result_candidate->records_.end(), records.begin(),
          records.begin() + static_cast<std::ptrdiff_t>(copied_record_count));
      CheckedAccumulate(logical_overflow_count,
                        records.size() - copied_record_count,
                        "Observer host-drop aggregation overflows uint64.");
    }

    observer_result_candidate->counters_.record_count =
        static_cast<std::uint32_t>(observer_result_candidate->records_.size());
    observer_result_candidate->counters_.overflow_count =
        SaturateToUint32(logical_overflow_count);
    observer_result_candidate->counters_.captured_primary_count =
        SaturateToUint32(logical_captured_primary_count);

    if (observer_config.enabled != 0U) {
      observer_dump = observer_result_candidate->BuildDump();
    }
  }

  std::unique_lock snapshot_lock{source_run_snapshot_mutex_};

  if (!observer_dump.empty()) {
    GGEMS_INFO("Observer", "{}", observer_dump);
  }

  GGEMS_INFO("Core", "GGEMSRun projection {} completed.", run_id);

  radionuclide_emission_planner_->CommitCandidate(emission_candidate);

  if (observer_result_candidate != nullptr) {
    observer_->SwapRunResult(*observer_result_candidate);
  }

  last_source_run_snapshot_ = std::move(source_snapshot);

  current_time_ps_.store(time_window.stop_ps);
}
} // namespace ggems::core
