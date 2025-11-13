#pragma once

/// \cond
#include <string>
#include <vector>
/// \endcond

#include "GGEMS/frameworks/GGEMSOpenCLContext.hh"
#include "GGEMS/frameworks/GGEMSOpenCLDevice.hh"

namespace ggems::run {
class GGEMSExecutor {
public:
  GGEMSExecutor();
  ~GGEMSExecutor();

  void Initialize();
  void Run();
  void SelectDevices(std::vector<std::string> const &filters);

private:
  void RunMT();
  void Banner() const;
  void CreateContexts();

private:
  std::vector<ggems::ocl::GGEMSOpenCLDevice> devices_;
  std::vector<ggems::ocl::GGEMSOpenCLContext> contexts_;
};
} // namespace ggems::run
