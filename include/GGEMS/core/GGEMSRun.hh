#pragma once

#include <atomic>
#include <cstdint>
#include <thread>
#include <vector>
#include <memory>
#include <mutex>
#include <optional>

#include "GGEMS/core/GGEMSTimeWindow.hh"
#include "GGEMS/core/sources/GGEMSSource.hh"
#include "GGEMS/core/sources/GGEMSSourceRunSnapshot.hh"
#include "GGEMS/core/particles/GGEMSPrimaryStream.hh"
#include "GGEMS/core/transport/GGEMSTransportWorkload.hh"

namespace ggems::core::random {
class GGEMSRandom;
}

namespace ggems::core::sources {
class GGEMSSourcePopulationPlanner;
}

namespace ggems::core::observer {
class GGEMSTransportObserver;
}

namespace ggems::core {

class GGEMSRun {
public:
  GGEMSRun();
  ~GGEMSRun();

  GGEMSRun(GGEMSRun const &) = delete;
  GGEMSRun(GGEMSRun &&) = delete;
  auto operator=(GGEMSRun const &) -> GGEMSRun & = delete;
  auto operator=(GGEMSRun &&) -> GGEMSRun & = delete;

  void Initialize();
  void Run();

  auto SetTimePicoSecond(std::uint64_t start_ps, std::uint64_t stop_ps,
                         std::uint64_t step_ps) -> void;
  auto ResetTime() -> void;

  [[nodiscard]] auto HasTimeConfiguration() const noexcept -> bool;
  [[nodiscard]] auto HasNextTimeStep() const noexcept -> bool;
  [[nodiscard]] auto GetCurrentTimePicoSecond() const noexcept -> std::uint64_t;
  [[nodiscard]] auto GetCurrentTimeWindowPicoSecond() const noexcept
      -> GGEMSTimeWindow;

  [[nodiscard]] auto GetLastSourceRunSnapshot() const
      -> std::optional<sources::GGEMSSourceRunSnapshot>;
  [[nodiscard]] auto HasObserver() const noexcept -> bool;

  void SetRandom(std::shared_ptr<random::GGEMSRandom> random);
  void SetPrimaryCount(std::uint32_t primary_count);
  void SetWorkerCount(std::uint32_t worker_count);
  void SetSource(std::shared_ptr<sources::GGEMSSource> source);
  void AddSource(std::shared_ptr<sources::GGEMSSource> source);
  void SetObserver(std::shared_ptr<observer::GGEMSTransportObserver> observer);

private:
  std::vector<std::thread> workers_;
  std::atomic<bool> running_{false};

  std::shared_ptr<random::GGEMSRandom> random_{nullptr};
  std::shared_ptr<observer::GGEMSTransportObserver> observer_{nullptr};
  std::vector<std::shared_ptr<sources::GGEMSSource>> sources_;
  bool uses_implicit_default_source_{true};

  mutable std::mutex source_run_snapshot_mutex_;
  std::optional<sources::GGEMSSourceRunSnapshot> last_source_run_snapshot_;

  particles::GGEMSPrimaryStream primary_stream_;

  bool initialized_{false};
  std::uint64_t next_run_id_{0ULL};

  bool has_time_configuration_{false};
  std::uint64_t time_start_ps_{0ULL};
  std::uint64_t time_stop_ps_{0ULL};
  std::uint64_t time_step_ps_{0ULL};
  std::atomic<std::uint64_t> current_time_ps_{0ULL};

  sources::GGEMSSourceConfigurationSnapshotPtr source_configuration_snapshot_;
  std::unique_ptr<sources::GGEMSSourcePopulationPlanner>
      source_population_planner_;

  std::vector<std::unique_ptr<transport::GGEMSTransportWorkload>>
      transport_workloads_;

  std::uint32_t worker_count_{256U};
};
} // namespace ggems::core
