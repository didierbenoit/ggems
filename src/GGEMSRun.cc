#include <filesystem>
#include <algorithm>
#include <memory>
#include <mutex>
#include <optional>
#include <exception>
#include <thread>
#include <atomic>
#include <vector>
#include <utility>
#include <cstdint>
#include <cstddef>
#include <format>
#include <span>
#include <string>

#include "GGEMS/GGEMSRun.hh"
#include "GGEMS/GGEMSException.hh"
#include "GGEMS/logging/GGEMSLogMacros.hh"
#include "GGEMS/units/GGEMSTimeUnits.hh"
#include "GGEMS/random/GGEMSRandom.hh"
#include "GGEMS/sources/GGEMSSourceDescription.hh"
#include "GGEMS/sources/GGEMSSourcePopulationPlan.hh"
#include "GGEMS/sources/GGEMSSourceRunSnapshot.hh"
#include "GGEMS/sources/GGEMSSourceRunRange.hh"
#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/transport/GGEMSTransportWorkloadPlan.hh"
#include "GGEMS/transport/GGEMSTransportWorkload.hh"
#include "GGEMS/observer/GGEMSTransportObserver.hh"
#include "GGEMS/observer/GGEMSObserverRecord.hh"
#include "GGEMS/GGEMSTimeWindow.hh"

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

auto AccumulateTransportCounters(
  transport::GGEMSTransportLogicalCounters &dst,
  transport::GGEMSTransportLogicalCounters const &src) noexcept -> void {
  dst.next_primary_id += src.next_primary_id;
  dst.consumed_primary_count += src.consumed_primary_count;
  dst.completed_history_count += src.completed_history_count;
  dst.terminal_particle_count += src.terminal_particle_count;
  dst.created_secondary_count += src.created_secondary_count;
  dst.aionino_to_gamma_count += src.aionino_to_gamma_count;
  dst.gamma_to_electron_count += src.gamma_to_electron_count;
  dst.electron_to_electron_count += src.electron_to_electron_count;
  dst.overflow_count += src.overflow_count;
  dst.total_fake_step_count += src.total_fake_step_count;

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

  if (!(static_cast<std::size_t>(source_index) < source_ranges.size())) {
    throw ggems::core::GGEMSRecoverable(std::format(
      "Observer source index {} is outside the current source snapshot.",
      source_index));
  }

  std::uint64_t const source_primary_count =
    source_ranges[static_cast<std::size_t>(source_index)].primary_count;

  if (!(config.capture_source_local_primary_id < source_primary_count)) {
    throw ggems::core::GGEMSRecoverable(
      std::format("Observer primary index {} is outside source slot {}, which "
                  "contains {} primaries.",
                  config.capture_source_local_primary_id, source_index,
                  source_primary_count));
  }
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
  if (initialized_) {
    throw ggems::core::GGEMSRecoverable(
      "Cannot configure GGEMSRun time after Initialize.");
  }
  if (start_ps >= stop_ps) {
    throw ggems::core::GGEMSRecoverable(
      "GGEMSRun time start must be strictly less than time stop.");
  }
  if (step_ps == 0ULL) {
    throw ggems::core::GGEMSRecoverable("GGEMSRun time step must be non-zero");
  }

  time_start_ps_ = start_ps;
  time_stop_ps_ = stop_ps;
  time_step_ps_ = step_ps;
  current_time_ps_.store(start_ps);
  has_time_configuration_ = true;
}

// -----------------------------------------------------------------------------

auto GGEMSRun::ResetTime() -> void {
  if (running_.exchange(true)) {
    throw ggems::core::GGEMSRecoverable(
      "Cannot reset GGEMSRun time while Run is executing.");
  }

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

  return {
    .start_ps = current_time_ps,
    .stop_ps = current_time_ps + window_width_ps,
  };
}

// -----------------------------------------------------------------------------

auto GGEMSRun::SetRandom(std::shared_ptr<random::GGEMSRandom> random) -> void {
  if (random == nullptr) {
    throw ggems::core::GGEMSRecoverable(
      "Cannot attach a null GGEMSRandom to GGEMSRun.");
  }

  if (initialized_) {
    throw ggems::core::GGEMSRecoverable(
      "Cannot change GGEMSRandom after Initialize.");
  }

  random_ = std::move(random);

  GGEMS_INFO("Random", "GGEMSRun random engine set to '{}'.",
             random_->GetEngineName());
}

// -----------------------------------------------------------------------------

auto GGEMSRun::AddSource(std::shared_ptr<sources::GGEMSSource> source) -> void {
  if (initialized_) {
    throw ggems::core::GGEMSRecoverable(
      "Cannot add a GGEMSSource after Initialize.");
  }

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
  if (observer == nullptr) {
    throw ggems::core::GGEMSRecoverable(
      "Cannot attach a null GGEMSTransportObserver to GGEMSRun.");
  }

  if (initialized_) {
    throw ggems::core::GGEMSRecoverable(
      "Cannot change GGEMSTransportObserver after Initialize.");
  }

  observer_ = std::move(observer);

  GGEMS_INFO("Observer", "GGEMSRun transport observer attached.");
}

// -----------------------------------------------------------------------------

auto GGEMSRun::Initialize() -> void {
  if (initialized_) {
    throw ggems::core::GGEMSRecoverable(
      "GGEMSRun::Initialize called more than once.");
  }

  for (auto const &source : sources_) {
    if (source == nullptr) {
      throw ggems::core::GGEMSInternal(
        "GGEMSRun source collection contains a null entry.");
    }
  }

  auto const initial_time_window =
    has_time_configuration_
      ? std::make_optional(GetCurrentTimeWindowPicoSecond())
      : std::optional<GGEMSTimeWindow>{};

  for (auto const &source : sources_) {
    source->ValidatePopulationForRunInitialization(initial_time_window);
  }

  auto const source_count = static_cast<std::uint32_t>(sources_.size());

  if (random_ == nullptr) {
    throw ggems::core::GGEMSRecoverable(
      "GGEMSRun cannot be initialized without a GGEMSRandom. "
      "Create a ggems.rndm GGEMSRandom object and attach it with "
      "GGEMSRun::SetRandom before calling Initialize.");
  }

  auto &opencl = ocl::GGEMSOpenCL::GetInstance();

  if (opencl.GetContext().empty()) {
    throw ggems::core::GGEMSRecoverable(
      "GGEMSRun requires initialized OpenCL contexts.");
  }

  GGEMS_INFO("Core", "Initializing GGEMSRun stable state...");

  GGEMS_INFO("Random", "Random engine ready: {} with seed {}.",
             random_->GetEngineName(), random_->GetSeed());

  std::uint32_t const observer_record_capacity =
    observer_ != nullptr ? observer_->GetRecordCapacity() : 1U;

  auto new_source_population_planner =
    std::make_unique<sources::GGEMSSourcePopulationPlanner>(sources_, *random_);
  auto new_source_configuration =
    sources::BuildSourceConfigurationSnapshot(sources_);

  std::filesystem::path const kernel_root{GGEMS_KERNEL_ROOT};

  std::vector<std::unique_ptr<transport::GGEMSTransportWorkload>>
    new_transport_workloads;
  new_transport_workloads.reserve(opencl.GetContext().size());

  for (std::size_t context_index = 0U;
       context_index < opencl.GetContext().size(); ++context_index) {
    std::uint64_t const random_stream_offset =
      static_cast<std::uint64_t>(context_index) *
      static_cast<std::uint64_t>(opencl.GetWorkerCount());

    new_transport_workloads.push_back(
      std::make_unique<transport::GGEMSTransportWorkload>(
        opencl.GetContext()[context_index], kernel_root, *random_,
        opencl.GetWorkerCount(), *new_source_configuration,
        random_stream_offset, static_cast<std::uint32_t>(context_index),
        observer_record_capacity));
  }

  GGEMS_INFO("Core", "{} transport workload(s) prepared.",
             new_transport_workloads.size());
  GGEMS_INFO("Source", "GGEMSRun source collection prepared with {} slot(s).",
             source_count);

  transport_workloads_.swap(new_transport_workloads);
  source_configuration_snapshot_ = std::move(new_source_configuration);
  source_population_planner_ = std::move(new_source_population_planner);
  next_run_id_ = 0ULL;
  current_time_ps_.store(has_time_configuration_ ? time_start_ps_ : 0ULL);

  for (auto const &source : sources_) {
    source->FinalizeInitialization();
  }

  initialized_ = true;
}

// -----------------------------------------------------------------------------

auto GGEMSRun::Run() -> void {
  if (!initialized_) {
    throw ggems::core::GGEMSRecoverable(
      "GGEMSRun::Run called before Initialize.");
  }

  if (running_.exchange(true)) {
    throw ggems::core::GGEMSRecoverable("GGEMSRun is already running.");
  }

  RunningGuard running_guard{running_};

  if (!HasNextTimeStep()) {
    throw ggems::core::GGEMSRecoverable("GGEMSRun time schedule is exhausted.");
  }

  GGEMSTimeWindow const time_window = GetCurrentTimeWindowPicoSecond();

  std::uint64_t const run_id = next_run_id_;

  auto population_candidate =
    source_population_planner_->BuildCandidate(time_window);

  auto source_snapshot = sources::BuildSourceRunSnapshot(
    sources_, source_configuration_snapshot_, population_candidate.GetPlan());

  auto const &source_records = source_snapshot.GetRecords();
  auto const &source_ranges = source_snapshot.GetRanges();
  auto const &source_population_records =
    source_snapshot.GetPopulationRecords();
  auto const &source_emission_ranges = source_snapshot.GetGroupRanges();

  std::uint64_t const total_primary_count =
    source_snapshot.GetTotalPrimaryCount();

  if (total_primary_count == 0ULL && !has_time_configuration_) {
    throw ggems::core::GGEMSRecoverable(
      "GGEMSRun requires a non-zero total primary count in static mode.");
  }

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

  if (total_primary_count == 0ULL) {
    std::unique_ptr<observer::GGEMSTransportObserver> observer_result_candidate;

    if (observer_ != nullptr) {
      observer_result_candidate = observer_->CreateRunResultCandidate();
    }

    std::unique_lock snapshot_lock{source_run_snapshot_mutex_};

    GGEMS_INFO("Core",
               "GGEMSRun projection {} completed as an empty time window "
               "[{} ps, {} ps).",
               run_id, time_window.start_ps, time_window.stop_ps);

    source_population_planner_->CommitCandidate(population_candidate);
    ++next_run_id_;

    if (observer_result_candidate != nullptr) {
      observer_->CommitRunResult(*observer_result_candidate);
    }

    last_source_run_snapshot_ = std::move(source_snapshot);
    current_time_ps_.store(time_window.stop_ps);
    return;
  }

  auto &opencl = ocl::GGEMSOpenCL::GetInstance();

  auto const workload_plan = transport::BuildEqualTransportWorkloadPlan(
    0ULL, total_primary_count,
    static_cast<std::uint32_t>(transport_workloads_.size()),
    opencl.GetWorkerCount());

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
    config.source_emission_ranges = source_emission_ranges;
    transport_workloads_[workload.context_index]->ValidateRunConfig(config);
  }

  auto const primary_view =
    primary_stream_.PrepareRun(run_id, total_primary_count);
  std::uint64_t const reserved_primary_count =
    primary_view.source_primary_count;
  ++next_run_id_;

  for (auto &config : transport_configs) {
    config.projection_history_offset = primary_view.global_history_offset;
  }

  GGEMS_INFO("Core", "GGEMSRun projection {} started.", run_id);
  GGEMS_INFOEX("Core", 1,
               "Projection {} primary stream: {} primaries, global history "
               "offset {}.",
               run_id, reserved_primary_count,
               primary_view.global_history_offset);
  GGEMS_INFOEX("Core", 1, "Projection {} worker count: {}.", run_id,
               opencl.GetWorkerCount());

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

      std::uint32_t const context_index = workload.context_index;

      transport_threads.emplace_back(
        [&, plan_index, context_index,
         config = std::move(transport_configs[plan_index])] -> void {
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
    auto const &workload = workload_plan[plan_index];

    if (workload.primary_count == 0U) {
      continue;
    }

    auto const &report = reports[plan_index];
    auto const &counters = report.counters;

    AccumulateTransportCounters(merged_counters, counters);

    accumulated_host_time_ps += report.host_time.value;
    accumulated_command_time_ps += report.command_time.value;
    accumulated_kernel_time_ps += report.kernel_time.value;

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
      report.kernel_time, report.host_time, report.kernel_histories_per_second);
  }

  if (merged_counters.overflow_count != 0ULL) {
    throw ggems::core::GGEMSRecoverable(
      "Transport reported an internal overflow.");
  }

  if (merged_counters.consumed_primary_count != total_primary_count) {
    throw ggems::core::GGEMSRecoverable(
      "Transport consumed primary count does not match projection count.");
  }

  if (merged_counters.completed_history_count != total_primary_count) {
    throw ggems::core::GGEMSRecoverable(
      "Transport completed history count does not match projection count.");
  }

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
    observer_result_candidate = observer_->CreateRunResultCandidate();

    for (std::size_t plan_index = 0U; plan_index < workload_plan.size();
         ++plan_index) {
      if (workload_plan[plan_index].primary_count == 0ULL) {
        continue;
      }

      auto const &report = reports[plan_index];
      auto const &logical_counters = report.logical_observer_counters;
      observer_result_candidate->AccumulateRunResult(
        report.observer_records,
        observer::GGEMSObserverRunResultCounters{
          .record_count = logical_counters.record_count,
          .overflow_count = logical_counters.overflow_count,
          .captured_primary_count = logical_counters.captured_primary_count,
        });
    }

    if (observer_config.enabled != 0U) {
      observer_dump = observer_result_candidate->BuildDump();
    }
  }

  std::unique_lock snapshot_lock{source_run_snapshot_mutex_};

  if (!observer_dump.empty()) {
    GGEMS_INFO("Observer", "{}", observer_dump);
  }

  GGEMS_INFO("Core", "GGEMSRun projection {} completed.", run_id);

  source_population_planner_->CommitCandidate(population_candidate);

  if (observer_result_candidate != nullptr) {
    observer_->CommitRunResult(*observer_result_candidate);
  }

  last_source_run_snapshot_ = std::move(source_snapshot);

  current_time_ps_.store(time_window.stop_ps);
}
} // namespace ggems::core
