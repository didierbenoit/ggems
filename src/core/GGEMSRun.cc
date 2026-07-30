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

#include "GGEMS/core/GGEMSRun.hh"
#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/core/units/GGEMSTimeUnits.hh"
#include "GGEMS/core/random/GGEMSRandom.hh"
#include "GGEMS/core/sources/GGEMSSourceDescription.hh"
#include "GGEMS/core/sources/GGEMSSourcePopulation.hh"
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

auto AccumulateTransportCounters(transport::GGEMSTransportCounters &dst,
                                 transport::GGEMSTransportCounters const &src)
    -> void {
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

// =============================================================================
// =============================================================================

auto ValidateTransportWorkloadCapacity(std::uint64_t total_primary_count,
                                       std::size_t workload_count,
                                       std::uint32_t worker_count) -> void {
  GGEMS_CHECK_INTERNAL(workload_count > 0U,
                       "No transport workload was initialised.");

  auto const workload_count_u64 = static_cast<std::uint64_t>(workload_count);

  auto const largest_workload_primary_count =
      (total_primary_count / workload_count_u64) +
      (total_primary_count % workload_count_u64 != 0ULL ? 1ULL : 0ULL);

  auto const safe_atomic_primary_count = static_cast<std::uint64_t>(
      std::numeric_limits<std::uint32_t>::max() - worker_count);

  GGEMS_CHECK_RECOVERABLE(
      largest_workload_primary_count <= safe_atomic_primary_count,
      std::format(
          "Current transport assigns {} primaries to its largest workload, "
          "exceeding the safe uint32 atomic stream limit {} for {} workers.",
          largest_workload_primary_count, safe_atomic_primary_count,
          worker_count));
}

} // namespace

// =============================================================================
// =============================================================================

GGEMSRun::GGEMSRun() : sources_{std::make_shared<sources::GGEMSSource>()} {
  GGEMS_INFOEX("Core", 3, "GGEMSRun instance created.");
}

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

  for (std::size_t source_index = 0U; source_index < sources_.size();
       ++source_index) {
    auto const &source = sources_[source_index];
    GGEMS_CHECK_INTERNAL(source != nullptr,
                         "GGEMSRun source collection contains a null entry.");

    GGEMS_CHECK_RECOVERABLE(
        source->GetPopulationMode() ==
            sources::GGEMSSourcePopulationMode::CountDriven,
        std::format("GGEMSRun source slot {} is ActivityDriven; B3.2 device "
                    "integration is not implemented.",
                    source_index));
  }

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

  GGEMS_INFO("Core", "Initialising GGEMSRun stable state...");

  GGEMS_INFO("Random", "Random engine ready: {} with seed {}.",
             random_->GetEngineName(), random_->GetSeed());

  std::uint32_t observer_record_capacity =
      observer_ != nullptr ? observer_->GetRecordCapacity() : 1U;

  auto new_source_configuration =
      sources::BuildSourceConfigurationSnapshot(sources_);

  std::filesystem::path kernel_root{GGEMS_KERNEL_ROOT};

  std::vector<std::unique_ptr<transport::GGEMSTransportWorkload>>
      new_transport_workloads;
  new_transport_workloads.reserve(opencl.GetContext().size());

  for (std::size_t context_index = 0U;
       context_index < opencl.GetContext().size(); ++context_index) {
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
  next_run_id_ = 0ULL;
  current_time_ps_.store(has_time_configuration_ ? time_start_ps_ : 0ULL);
  initialised_ = true;

  for (auto const &source : sources_) {
    source->FinalizeInitialization();
  }
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
  std::uint64_t const run_id = next_run_id_++;

  auto source_snapshot = sources::BuildSourceRunSnapshot(
      sources_, source_configuration_snapshot_, time_window);

  auto const &source_records = source_snapshot.GetRecords();
  auto const &source_ranges = source_snapshot.GetRanges();

  GGEMS_CHECK_INTERNAL(
      source_records.size() == source_ranges.size(),
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

  if (total_primary_count == 0ULL) {
    if (observer_ != nullptr) {
      observer_->Clear();
    }

    {
      std::scoped_lock lock{source_run_snapshot_mutex_};
      last_source_run_snapshot_ = std::move(source_snapshot);
    }

    GGEMS_INFO("Core",
               "GGEMSRun projection {} completed as an empty time window "
               "[{} ps, {} ps).",
               run_id, time_window.start_ps, time_window.stop_ps);

    current_time_ps_.store(time_window.stop_ps);
    return;
  }

  observer::GGEMSObserverConfigRecord observer_config{};

  if (observer_ != nullptr) {
    observer_config = observer_->BuildConfigRecord();
  }

  ValidateObserverCapture(observer_config, source_ranges);

  transport::ValidateDiagnosticTransportSources(source_records, source_ranges);

  GGEMS_CHECK_RECOVERABLE(
      total_primary_count <=
          static_cast<std::uint64_t>(std::numeric_limits<std::uint32_t>::max()),
      "Current transport supports at most uint32_t primaries per "
      "projection.");

  GGEMS_CHECK_RECOVERABLE(!transport_workloads_.empty(),
                          "No transport workload was initialised.");

  ValidateTransportWorkloadCapacity(total_primary_count,
                                    transport_workloads_.size(), worker_count_);

  auto primary_view = primary_stream_.PrepareRun(run_id, total_primary_count);

  std::uint64_t const reserved_primary_count =
      primary_view.source_primary_count;

  auto projection_primary_count =
      static_cast<std::uint32_t>(reserved_primary_count);

  GGEMS_INFO("Core", "GGEMSRun projection {} started.", run_id);

  GGEMS_INFOEX("Core", 1,
               "Projection {} primary stream: {} primaries, global history "
               "offset {}.",
               run_id, reserved_primary_count,
               primary_view.global_history_offset);

  GGEMS_INFOEX("Core", 1, "Projection {} worker count: {}.", run_id,
               worker_count_);

  std::uint64_t projection_history_offset = primary_view.global_history_offset;

  std::vector<transport::GGEMSTransportWorkloadPlan> workload_plan =
      transport::BuildEqualTransportWorkloadPlan(
          projection_history_offset, projection_primary_count,
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
    config.projection_history_offset = workload.projection_history_offset;
    config.device_primary_offset = workload.device_primary_offset;
    config.source_records = source_records;
    config.source_ranges = source_ranges;
  }

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

  transport::GGEMSTransportCounters merged_counters{};

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
        report.kernel_time, report.host_time,
        report.kernel_histories_per_second);
  }

  GGEMS_CHECK_RECOVERABLE(merged_counters.overflow_count == 0U,
                          "Transport reported an internal overflow.");

  GGEMS_CHECK_RECOVERABLE(
      merged_counters.consumed_primary_count == projection_primary_count,
      "Transport consumed primary count does not match projection count.");

  GGEMS_CHECK_RECOVERABLE(
      merged_counters.completed_history_count == projection_primary_count,
      "Transport completed history count does not match projection count.");

  GGEMS_CHECK_RECOVERABLE(
      merged_counters.terminal_particle_count == projection_primary_count,
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

  if (observer_ != nullptr) {
    observer_->Clear();
    if (observer_config.enabled != 0U) {
      for (std::size_t plan_index = 0U; plan_index < workload_plan.size();
           ++plan_index) {
        transport::GGEMSTransportWorkloadPlan const &workload =
            workload_plan[plan_index];

        if (workload.primary_count == 0U) {
          continue;
        }

        auto const &report = reports[plan_index];

        observer_->Accumulate(report.observer_records,
                              report.observer_counters);
      }

      GGEMS_INFO("Observer", "{}", observer_->BuildDump());
    }
  }

  {
    std::scoped_lock lock{source_run_snapshot_mutex_};
    last_source_run_snapshot_ = std::move(source_snapshot);
  }

  GGEMS_INFO("Core", "GGEMSRun projection {} completed.", run_id);
  current_time_ps_.store(time_window.stop_ps);
}
} // namespace ggems::core
