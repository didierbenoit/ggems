#include <filesystem>
#include <algorithm>
#include <memory>
#include <exception>
#include <thread>
#include <atomic>
#include <vector>
#include <limits>

#include "GGEMS/core/GGEMSRun.hh"
#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/core/random/GGEMSRandom.hh"
#include "GGEMS/frameworks/GGEMSOpenCL.hh"
#include "GGEMS/frameworks/GGEMSOpenCLProfiler.hh"
#include "GGEMS/core/transport/GGEMSTransportWorkloadPlan.hh"

using namespace ggems::units;

namespace ggems::core {

namespace {

void AccumulateTransportCounters(transport::GGEMSTransportCounters &dst,
                                 transport::GGEMSTransportCounters const &src) {
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

} // namespace

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSRun::GGEMSRun() { GGEMS_INFOEX("Core", 3, "GGEMSRun instance created."); }

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSRun::SetRandom(std::shared_ptr<random::GGEMSRandom> random) {
  GGEMS_CHECK_RECOVERABLE(random != nullptr,
                          "Cannot attach a null GGEMSRandom to GGEMSRun.");

  GGEMS_CHECK_RECOVERABLE(!initialised_,
                          "Cannot change GGEMSRandom after Initialise.");

  random_ = std::move(random);

  GGEMS_INFO("Random", "GGEMSRun random engine set to '{}'.",
             random_->GetEngineName());
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSRun::SetSource(std::shared_ptr<sources::GGEMSSource> source) {
  GGEMS_CHECK_RECOVERABLE(source != nullptr,
                          "Cannot attach a null GGEMSSource to GGEMSRun.");

  GGEMS_CHECK_RECOVERABLE(!initialised_,
                          "Cannot change GGEMSSource after Initialise.");

  source_ = std::move(source);

  GGEMS_INFO("Source", "GGEMSRun source attached.");
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSRun::SetPrimaryCount(std::uint32_t primary_count) {
  GGEMS_CHECK_RECOVERABLE(!initialised_,
                          "Cannot change primary count after Initialise.");

  primary_stream_.SetPrimaryCount(primary_count);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSRun::SetWorkerCount(std::uint32_t worker_count) {
  GGEMS_CHECK_RECOVERABLE(worker_count > 0ULL,
                          "GGEMSRun worker count must be non-zero.");

  GGEMS_CHECK_RECOVERABLE(!initialised_,
                          "Cannot change worker count after Initialise.");

  worker_count_ = worker_count;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSRun::Initialise() {
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

  if (source_ == nullptr) {
    source_ = std::make_shared<sources::GGEMSSource>();

    GGEMS_INFO("Source", "No GGEMSSource attached to GGEMSRun. "
                         "Using default analytic gamma point source.");
  }

  source_->Verbose();

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
            worker_count_, random_stream_offset,
            static_cast<std::uint32_t>(context_index)));
  }

  GGEMS_INFO("Core", "{} dummy transport workload(s) initialised.",
             dummy_transports_.size());

  next_run_id_ = 0ULL;
  initialised_ = true;

  GGEMS_INFO("Core", "GGEMSRun Initialised.");
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSRun::Run() {
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
  auto primary_view = primary_stream_.PrepareRun(run_id);

  GGEMS_CHECK_RECOVERABLE(
      primary_view.source_primary_count <=
          static_cast<std::uint64_t>(std::numeric_limits<std::uint32_t>::max()),
      "Dummy transport currently supports at most uint32_t primaries per "
      "projection.");

  std::uint32_t source_primary_count =
      static_cast<std::uint32_t>(primary_view.source_primary_count);

  GGEMS_INFO("Core", "GGEMSRun projection {} started.", run_id);

  GGEMS_INFOEX("Core", 1,
               "Projection {} primary stream: {} primaries, global history "
               "offset {}.",
               run_id, primary_view.source_primary_count,
               primary_view.global_history_offset);

  GGEMS_INFOEX("Core", 1, "Projection {} worker count: {}.", run_id,
               worker_count_);

  GGEMS_CHECK_RECOVERABLE(!dummy_transports_.empty(),
                          "No dummy transport workload was initialised.");

  sources::GGEMSSourceRecord source_record = source_->BuildRecord();

  std::uint64_t projection_history_offset = primary_view.global_history_offset;

  std::vector<transport::GGEMSTransportWorkloadPlan> workload_plan =
      transport::BuildEqualTransportWorkloadPlan(
          projection_history_offset, source_primary_count,
          static_cast<std::uint32_t>(dummy_transports_.size()), worker_count_);

  std::vector<transport::GGEMSDummyTransportRunReport> reports{
      workload_plan.size()};

  std::vector<std::exception_ptr> exceptions{workload_plan.size()};

  std::vector<std::thread> transport_threads;
  transport_threads.reserve(workload_plan.size());

  for (std::size_t plan_index = 0U; plan_index < workload_plan.size();
       ++plan_index) {
    transport::GGEMSTransportWorkloadPlan workload = workload_plan[plan_index];

    if (workload.primary_count == 0U) {
      continue;
    }

    transport_threads.emplace_back([&, plan_index, workload, source_record]() {
      try {
        transport::GGEMSDummyTransportRunConfig config{};
        config.total_primary_count = workload.primary_count;
        config.projection_history_offset = workload.projection_history_offset;
        config.device_primary_offset = workload.device_primary_offset;
        config.source_record = source_record;

        reports[plan_index] =
            dummy_transports_[workload.context_index]->Run(config);
      } catch (...) {
        exceptions[plan_index] = std::current_exception();
      }
    });
  }

  for (std::thread &thread : transport_threads) {
    if (thread.joinable()) {
      thread.join();
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

  GGEMS_CHECK_RECOVERABLE(
      merged_counters.completed_history_count == source_primary_count,
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

  GGEMS_INFO("Core", "GGEMSRun projection {} completed.", run_id);
}
} // namespace ggems::core
