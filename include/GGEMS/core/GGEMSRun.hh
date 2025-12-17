#pragma once

/// \cond
#include <atomic>
#include <thread>
/// \endcond

#include "GGEMS/core/GGEMSProgressBar.hh"

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
  //  void Banner() const noexcept;
  //  void BannerAscii() const noexcept;

private:
  std::vector<std::thread> workers_;
  std::atomic<bool> running_{false};
  GGEMSProgressBar progress_bar_;
};
} // namespace ggems::core
