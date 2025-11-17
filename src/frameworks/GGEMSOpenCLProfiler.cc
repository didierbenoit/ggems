#include "GGEMS/frameworks/GGEMSOpenCLProfiler.hh"
#include "GGEMS/core/units/GGEMSBandwidthUnits.hh"

using namespace ggems::units;

namespace ggems::ocl {

GGEMSKernelStaticInfo ExtractKernelStaticInfo(GGEMSOpenCLKernel const &kernel) {
  GGEMSKernelStaticInfo info;
  GGEMSOpenCLContext const &context = kernel.GetContext();
  GGEMSOpenCLDevice const &device = context.GetDevice();

  info.kernel_name = kernel.GetKernelName();
  info.function_name = kernel.GetFunctionName();
  info.device_name = device.GetName();

  // --- Work group -----------------------------------------------------
  GGEMSKernelWorkGroupStaticInfo wg{};
  bool has_any = false;

  wg.max_work_group_size = kernel.GetWorkGroupSize();
  has_any = has_any || (wg.max_work_group_size != 0);

  wg.compile_work_group_size = kernel.GetCompileWorkGroupSize();
  has_any = has_any ||
            (wg.compile_work_group_size[0] || wg.compile_work_group_size[1] ||
             wg.compile_work_group_size[2]);

  wg.preferred_work_group_size_multiple =
      kernel.GetPreferredWorkGroupSizeMultiple();
  has_any = has_any || (wg.preferred_work_group_size_multiple != 0);

  wg.local_mem_size = kernel.GetLocalMemSize() * 1_B;
  has_any = has_any || (wg.local_mem_size != 0_B);

  wg.private_mem_size = kernel.GetPrivateMemSize() * 1_B;
  has_any = has_any || (wg.private_mem_size != 0_B);

  wg.has_any_info = has_any;

  info.work_group = wg;

  // --- Args -------------------------------------------------------
  cl_uint num_args = kernel.GetNumArgs();
  info.args.reserve(num_args);

  for (cl_uint i = 0; i < num_args; ++i) {
    GGEMSKernelArgInfo a;
    a.index = i;
    bool ok = true;

    a.name = kernel.GetArgName(i);
    ok = a.name.empty() ? false : true;

    a.type_name = kernel.GetArgTypeName(i);
    ok = a.type_name.empty() ? false : true;

    a.type_qualifier = kernel.GetArgTypeQualifier(i);
    a.address_qualifier = kernel.GetArgAddressQualifier(i);
    a.access_qualifier = kernel.GetArgAccessQualifier(i);

    a.has_full_metadata = ok;
    info.args.push_back(std::move(a));
  }

  return info;
}

/* -------------------------------------------------------------- */

void GGEMSOpenCLProfiler::ProfileKernel(GGEMSOpenCLKernel &kernel,
                                        Options const &opts) {
  rep_.static_info = ExtractKernelStaticInfo(kernel);

  GGEMSKernelDynamicStats dyn{};

  // 1) Work-group sweep
  if (opts.enable_workgroup_sweep && !opts.sizes.empty()) {
    std::size_t const n = opts.sizes.back();
    Bytes const bytes = n * opts.bytes_per_item;

    dyn.workgroup_sweep = kernel.ProfileWorkGroups(n, bytes);

    Bandwidth best_bw{0LL};
    std::size_t best_wg = 0;

    for (auto const &st : dyn.workgroup_sweep) {
      if (st.bandwidth > best_bw) {
        best_bw = st.bandwidth;
        best_wg = st.local_work_size;
      }
    }

    dyn.best_workgroup_size = best_wg;
    dyn.best_workgroup_bandwidth = best_bw;
  }

  // 2) Bandwidth sweep
  if (opts.enable_bandwidth_sweep && !opts.sizes.empty()) {
    dyn.bandwidth_sweep =
        kernel.ProfileBandwidthSweep(opts.sizes, opts.bytes_per_item);

    Bandwidth max_bw{0LL};
    for (auto const &st : dyn.bandwidth_sweep) {
      if (st.bandwidth > max_bw) {
        max_bw = st.bandwidth;
      }
    }
    dyn.max_bandwidth = max_bw;
  }

  // 3) Driver overhead
  if (opts.enable_driver_overhead) {
    dyn.driver_overhead = kernel.ProfileDriverOverhead();
  }

  rep_.dynamic_stats = std::move(dyn);

  // 4) Log résumé (tu ajusteras INFO/INFOEX)
  /*  GGEMS_INFO("OpenCL",
               "Profile kernel '{}' on device '{}': "
               "WGbest= {} → {}, BWmax= {}, overhead= {}",
               rep_.static_info.kernel_name, rep_.static_info.device_name,
               rep_.dynamic_stats.best_workgroup_size,
               rep_.dynamic_stats.best_workgroup_bandwidth,
               rep_.dynamic_stats.max_bandwidth,
               rep_.dynamic_stats.driver_overhead);*/
}

/* -------------------------------------------------------------- */

void GGEMSOpenCLProfiler::PrintStaticInfo() const {
  GGEMS_INFO("OpenCL", "Kernel static infos:");
  GGEMS_INFO("OpenCL", "====================");
  GGEMS_INFO("OpenCL", "* Kernel: {}", rep_.static_info.kernel_name);
  GGEMS_INFO("OpenCL", "* Function: {}", rep_.static_info.function_name);
  GGEMS_INFO("OpenCL", "* Device: {}", rep_.static_info.device_name);

  if (rep_.static_info.work_group.has_any_info) {
    GGEMS_INFO("OpenCL", "    Work group infos:");
    GGEMS_INFO("OpenCL", "    -----------------");
    GGEMS_INFO("OpenCL", "        * Max work group size: {}",
               rep_.static_info.work_group.max_work_group_size);
    GGEMS_INFO("OpenCL", "        * Compile work group size: {}",
               rep_.static_info.work_group.compile_work_group_size);
    GGEMS_INFO("OpenCL", "        * Preferred_work_group_size_multiple: {}",
               rep_.static_info.work_group.preferred_work_group_size_multiple);
    GGEMS_INFO("OpenCL", "        * Local_mem_size: {}",
               HumanReadable(rep_.static_info.work_group.local_mem_size));
    GGEMS_INFO("OpenCL", "        * Private_mem_size: {}",
               HumanReadable(rep_.static_info.work_group.private_mem_size));
  } else {
    GGEMS_WARN("OpenCL", "Work group infos not available.");
  }

  GGEMS_INFO("OpenCL", "    Argument infos:");
  for (auto const &i : rep_.static_info.args) {
    if (i.has_full_metadata) {
      GGEMS_INFO("OpenCL", "      +++++++++++");
      GGEMS_INFO("OpenCL", "        * Name: {}", i.name);
      GGEMS_INFO("OpenCL", "        * Type: {}", i.type_name);
      GGEMS_INFO("OpenCL", "        * Type qualifier: {}", i.type_qualifier);
      GGEMS_INFO("OpenCL", "        * Address qualifier: {}",
                 i.address_qualifier);
      GGEMS_INFO("OpenCL", "        * Access qualifier: {}",
                 i.access_qualifier);
    }
  }
}

/* -------------------------------------------------------------- */

void GGEMSOpenCLProfiler::PrintDynamicInfo() const { ; }

/* -------------------------------------------------------------- */

void GGEMSOpenCLProfiler::PrintAllInfo() const {
  PrintStaticInfo();
  PrintDynamicInfo();
}

} // namespace ggems::ocl
