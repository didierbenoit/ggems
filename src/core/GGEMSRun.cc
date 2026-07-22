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
#include "GGEMS/core/sources/GGEMSSourceRunSnapshot.hh"
#include "GGEMS/core/sources/GGEMSSourceRunRange.hh"
#include "GGEMS/frameworks/GGEMSOpenCL.hh"
#include "GGEMS/core/transport/GGEMSTransportWorkloadPlan.hh"
#include "GGEMS/core/transport/GGEMSTransportCounters.hh"
#include "GGEMS/core/transport/GGEMSDummyTransportWorkload.hh"
#include "GGEMS/core/observer/GGEMSTransportObserver.hh"
#include "GGEMS/core/observer/GGEMSObserverRecord.hh"

using namespace ggems::units;

namespace ggems::core {

namespace {

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

  for (auto const &source : sources_) {
    GGEMS_CHECK_INTERNAL(source != nullptr,
                         "GGEMSRun source collection contains a null entry.");
  }

  GGEMS_CHECK_RECOVERABLE(
      sources_.size() <=
          static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max()),
      "Dummy transport currently supports at most uint32_t source slots.");

  auto source_count = static_cast<std::uint32_t>(sources_.size());

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

  primary_stream_.Initialise();

  std::uint32_t observer_record_capacity =
      observer_ != nullptr ? observer_->GetRecordCapacity() : 1U;

  std::filesystem::path kernel_root{GGEMS_KERNEL_ROOT};

  dummy_transports_.clear();
  dummy_transports_.reserve(opencl.GetContext().size());

  for (std::size_t context_index = 0U;
       context_index < opencl.GetContext().size(); ++context_index) {
    std::uint64_t random_stream_offset =
        static_cast<std::uint64_t>(context_index) *
        static_cast<std::uint64_t>(worker_count_);

    dummy_transports_.push_back(
        std::make_unique<transport::GGEMSDummyTransportWorkload>(
            opencl.GetContext()[context_index], kernel_root, *random_,
            worker_count_, source_count, random_stream_offset,
            static_cast<std::uint32_t>(context_index),
            observer_record_capacity));
  }

  GGEMS_INFO("Core", "{} dummy transport workload(s) initialised.",
             dummy_transports_.size());

  next_run_id_ = 0ULL;
  initialised_ = true;

  GGEMS_INFO("Source",
             "GGEMSRun source collection initialised with {} slot(s).",
             source_count);

  GGEMS_INFO("Core", "GGEMSRun Initialised.");
}

// -----------------------------------------------------------------------------

auto GGEMSRun::Run() -> void {
  GGEMS_CHECK_RECOVERABLE(initialised_,
                          "GGEMSRun::Run called before Initialise.");

  GGEMS_CHECK_RECOVERABLE(!running_.exchange(true),
                          "GGEMSRun is already running.");

  struct RunningGuard {
    std::atomic<bool> &running;
    ~RunningGuard() { running.store(false); }
  };

  RunningGuard running_guard{running_};

  std::uint64_t run_id = next_run_id_++;

  auto source_snapshot = sources::BuildSourceRunSnapshot(sources_);

  auto const &source_records = source_snapshot.GetRecords();
  auto const &source_ranges = source_snapshot.GetRanges();

  GGEMS_CHECK_INTERNAL(
      source_records.size() == source_ranges.size(),
      "GGEMSRun source snapshot record and range counts do not match.");

  GGEMS_CHECK_INTERNAL(!source_records.empty(),
                       "GGEMSRun source snapshot must not be empty.");

  observer::GGEMSObserverConfigRecord observer_config{};

  if (observer_ != nullptr) {
    observer_config = observer_->BuildConfigRecord();
  }

  ValidateObserverCapture(observer_config, source_ranges);

  std::uint64_t total_primary_count = source_snapshot.GetTotalPrimaryCount();

  GGEMS_CHECK_RECOVERABLE(
      total_primary_count > 0ULL,
      "GGEMSRun requires at least one active source per Run.");

  GGEMS_CHECK_RECOVERABLE(
      total_primary_count <=
          static_cast<std::uint64_t>(std::numeric_limits<std::uint32_t>::max()),
      "Dummy transport currently supports at most uint32_t primaries per "
      "projection.");

  auto primary_view = primary_stream_.PrepareRun(run_id, total_primary_count);

  std::uint64_t const reserved_primary_count =
      primary_view.source_primary_count;

  for (std::size_t source_index = 0U; source_index < source_records.size();
       ++source_index) {
    GGEMS_INFOEX("Source", 1, "Run {} source snapshot: {}", run_id,
                 sources::DescribeSourceRunSlot(source_index,
                                                source_records[source_index],
                                                source_ranges[source_index]));
  }

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

  GGEMS_CHECK_RECOVERABLE(!dummy_transports_.empty(),
                          "No dummy transport workload was initialised.");

  std::uint64_t projection_history_offset = primary_view.global_history_offset;

  std::vector<transport::GGEMSTransportWorkloadPlan> workload_plan =
      transport::BuildEqualTransportWorkloadPlan(
          projection_history_offset, projection_primary_count,
          static_cast<std::uint32_t>(dummy_transports_.size()), worker_count_);

  std::vector<transport::GGEMSDummyTransportRunReport> reports{
      workload_plan.size()};

  std::vector<std::exception_ptr> exceptions{workload_plan.size()};

  std::vector<transport::GGEMSDummyTransportRunConfig> transport_configs{
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
                  dummy_transports_[context_index]->Run(config);
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

    if (observer_ != nullptr && observer_->IsEnabled()) {
      observer_->Accumulate(report.observer_records, report.observer_counters);
    }

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

  GGEMS_CHECK_RECOVERABLE(
      merged_counters.completed_history_count == projection_primary_count,
      "Dummy transport completed history count does not match primary count.");

  GGEMS_CHECK_RECOVERABLE(
      merged_counters.terminal_particle_count ==
          merged_counters.consumed_primary_count +
              merged_counters.created_secondary_count,
      "Dummy transport terminal particle count is inconsistent.");

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

  {
    std::scoped_lock lock{source_run_snapshot_mutex_};
    last_source_run_snapshot_ = std::move(source_snapshot);
  }

  GGEMS_INFO("Core", "GGEMSRun projection {} completed.", run_id);
}
} // namespace ggems::core
