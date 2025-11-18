#pragma once

/// \cond
#include <atomic>
#include <stop_token>
#include <thread>
/// \endcond

#include "GGEMS/core/GGEMSProgressBar.hh"
#include "GGEMS/frameworks/GGEMSOpenCLContext.hh"

namespace ggems::core {
class GGEMSRun {
public:
  GGEMSRun();
  ~GGEMSRun();

  void Initialise();
  void Run();

private:
  void Banner() const;

private:
  std::vector<std::thread> workers_;
  std::atomic<bool> running_{false};
  GGEMSProgressBar progress_bar_;
  std::vector<GGEMSProgressBar::SlotPtr> progress_slots_;
};
} // namespace ggems::core
