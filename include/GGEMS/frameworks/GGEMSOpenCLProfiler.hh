#pragma once

#include "GGEMS/core/units/GGEMSUnits.hh"
#include "GGEMS/frameworks/GGEMSOpenCLKernel.hh"

using namespace ggems::units;

namespace ggems::ocl {
struct GGEMSKernelProfileReport {
  std::string kernel_name;
  std::string device_name;

  // --- Work-group sweep --------------------------------
  std::vector<GGEMSKernelExecutionStats> workgroup_stats;
  std::size_t best_workgroup_size{0};
  units::Bandwidth best_workgroup_bandwidth{0ULL};

  // --- Bandwidth sweep ---------------------------------
  std::vector<GGEMSKernelExecutionStats> bandwidth_stats;
  units::Bandwidth max_bandwidth{0LL};

  // --- Driver overhead ---------------------------------
  units::Time driver_overhead{0LL};
};

GGEMSKernelProfileReport ProfileKernel(GGEMSOpenCLKernel const &kernel,
                                       std::vector<std::size_t> const &sizes,
                                       units::Bytes bytes_per_item);
} // namespace ggems::ocl
