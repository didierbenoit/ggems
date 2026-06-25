#pragma once

#include <atomic>
#include <cstdint>
#include <thread>
#include <vector>
#include <memory>

#include "GGEMS/core/particles/GGEMSPrimaryStream.hh"

namespace ggems::core::random {
class GGEMSRandom;
}

namespace ggems::core {
class GGEMSRun {
public:
  GGEMSRun();
  ~GGEMSRun() = default;

  GGEMSRun(GGEMSRun const &) = delete;
  GGEMSRun(GGEMSRun &&) = delete;
  GGEMSRun &operator=(GGEMSRun const &) = delete;
  GGEMSRun &operator=(GGEMSRun &&) = delete;

  void Initialise();
  void Run();

  void SetRandom(std::shared_ptr<random::GGEMSRandom> random);
  void SetPrimaryCount(std::uint64_t primary_count);
  void SetWorkerCount(std::uint64_t worker_count);

private:
  std::vector<std::thread> workers_;
  std::atomic<bool> running_{false};

  std::shared_ptr<random::GGEMSRandom> random_{nullptr};

  particles::GGEMSPrimaryStream primary_stream_{};

  bool initialised_{false};
  std::uint64_t next_run_id_{0ULL};
  std::uint64_t worker_count_{256ULL};
};
} // namespace ggems::core
