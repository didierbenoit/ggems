#pragma once

/// \cond
#include <atomic>
#include <stop_token>
#include <thread>
/// \endcond

#include "GGEMS/frameworks/GGEMSOpenCLContext.hh"

namespace ggems::core {
class GGEMSRun {
public:
  GGEMSRun();
  ~GGEMSRun();

  void Initialise();
  void Run();
  void Stop();

private:
  void RunMT(std::stop_token st, ocl::GGEMSOpenCLContext &ctx);
  void Banner() const;

private:
  std::vector<std::jthread> workers_;
  std::atomic<bool> running_{false};
};
} // namespace ggems::core
