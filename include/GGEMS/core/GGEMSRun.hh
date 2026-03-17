#pragma once

/// \cond
#include <atomic>
#include <thread>
#include <vector>
/// \endcond

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

private:
  std::vector<std::thread> workers_;
  std::atomic<bool> running_{false};
};
} // namespace ggems::core
