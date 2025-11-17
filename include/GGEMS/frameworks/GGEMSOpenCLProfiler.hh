#pragma once

#include "GGEMS/core/units/GGEMSUnits.hh"
#include "GGEMS/frameworks/GGEMSOpenCLKernel.hh"

namespace ggems::ocl {

struct GGEMSKernelArgInfo {
  cl_uint index{0};

  std::string name;
  std::string type_name;
  std::string type_qualifier;
  std::string address_qualifier;
  std::string access_qualifier;

  bool has_full_metadata{false};
};

struct GGEMSKernelWorkGroupStaticInfo {
  std::size_t max_work_group_size{0};
  std::array<std::size_t, 3> compile_work_group_size{0, 0, 0};
  std::size_t preferred_work_group_size_multiple{0};

  units::Bytes local_mem_size{0LL};
  units::Bytes private_mem_size{0LL};

  bool has_any_info{false};
};

struct GGEMSKernelStaticInfo {
  std::string kernel_name;
  std::string function_name;
  std::string device_name;

  GGEMSKernelWorkGroupStaticInfo work_group;
  std::vector<GGEMSKernelArgInfo> args;
};

struct GGEMSKernelDynamicStats {
  std::vector<GGEMSKernelExecutionStats> workgroup_sweep;
  std::vector<GGEMSKernelExecutionStats> bandwidth_sweep;

  std::size_t best_workgroup_size{0};
  units::Bandwidth best_workgroup_bandwidth{0LL};

  units::Bandwidth max_bandwidth{0ULL};
  units::Time driver_overhead{0LL};
};

struct GGEMSKernelProfileReport {
  GGEMSKernelStaticInfo static_info;
  GGEMSKernelDynamicStats dynamic_stats;
};

class GGEMSOpenCLProfiler {
public:
  struct Options {
    std::vector<std::size_t> sizes;   // N pour le sweep
    units::Bytes bytes_per_item{0LL}; // ex: 3*sizeof(float) pour vec_add
    bool enable_workgroup_sweep{true};
    bool enable_bandwidth_sweep{true};
    bool enable_driver_overhead{true};
  };

  GGEMSOpenCLProfiler() = default;

  GGEMSKernelProfileReport ProfileKernel(GGEMSOpenCLKernel &kernel,
                                         Options const &opts) const;

private:
};

GGEMSKernelStaticInfo ExtractKernelStaticInfo(GGEMSOpenCLKernel const &kernel);
} // namespace ggems::ocl
