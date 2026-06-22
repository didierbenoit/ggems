#pragma once

#include <atomic>
#include <thread>
#include <vector>
#include <memory>

namespace ggems::core::random {
class GGEMSRandom;
}

namespace ggems::core {
class GGEMSRun {
public:
  GGEMSRun();
  ~GGEMSRun();

  GGEMSRun(GGEMSRun const &) = delete;
  GGEMSRun(GGEMSRun &&) = delete;
  GGEMSRun &operator=(GGEMSRun const &) = delete;
  GGEMSRun &operator=(GGEMSRun &&) = delete;

  void Initialise();
  void Run();

  void SetRandom(std::shared_ptr<random::GGEMSRandom> random);

private:
  std::vector<std::thread> workers_;
  std::atomic<bool> running_{false};
  std::shared_ptr<random::GGEMSRandom> random_{nullptr};
};
} // namespace ggems::core
