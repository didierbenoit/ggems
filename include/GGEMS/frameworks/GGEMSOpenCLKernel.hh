#pragma once

#include "GGEMS/core/units/GGEMSUnits.hh"
#include "GGEMS/frameworks/GGEMSOpenCLContext.hh"

namespace ggems::ocl {

struct GGEMSKernelExecutionStats {
  std::string kernel_name{""};
  std::string device_name{""};

  std::size_t global_work_items{0};
  std::size_t local_work_size{0};

  units::Bytes bytes_moved{0U}; // pour le calcul bande passante

  units::Time time_queued{0U};
  units::Time time_submit{0U};
  units::Time time_start{0U};
  units::Time time_end{0U};

  units::Time kernel_time{0U}; // start → end
  units::Time wall_time{0U};   // queued → end

  units::Bandwidth bandwidth{0U};
};

class GGEMSOpenCLKernel {
public:
  GGEMSOpenCLKernel(GGEMSOpenCLContext &ctx, cl::Kernel kernel,
                    std::string kernel_name);

  ~GGEMSOpenCLKernel() noexcept;

  GGEMSOpenCLKernel(GGEMSOpenCLKernel const &) = delete;
  GGEMSOpenCLKernel &operator=(GGEMSOpenCLKernel const &) = delete;

  GGEMSOpenCLKernel(GGEMSOpenCLKernel &&) noexcept = delete;
  GGEMSOpenCLKernel &operator=(GGEMSOpenCLKernel &&) noexcept = delete;

public:
  GGEMSOpenCLContext const &GetContext() const { return context_; }
  std::string_view GetKernelName() const { return kernel_name_; }

  /* --------- Arguments --------------------------------*/
  template <typename T> void SetArg(cl_uint index, T const &value) {
    kernel_.setArg(index, value);
  }

  void SetArgSVMPointer(cl_uint index, void *ptr);

  /* -------- Running -----------------------------*/
  // Exécution simple (1D pour l’instant)
  void Run(std::array<size_t, 1> const &global,
           std::array<size_t, 1> const &local);

  /* ------------- Profiling ----------------------*/
  GGEMSKernelExecutionStats ProfiledEnqueue(cl::NDRange global,
                                            cl::NDRange local,
                                            units::Bytes bytes_moved) const;

  std::vector<GGEMSKernelExecutionStats>
  ProfileWorkGroups(std::size_t global_size, units::Bytes bytes_moved) const;

  std::vector<GGEMSKernelExecutionStats>
  ProfileBandwidthSweep(std::vector<std::size_t> const &sizes,
                        units::Bytes elements_per_item) const;

  Time ProfileDriverOverhead() const;

  /* ------------- Kernel Info ------------------- */
  [[nodiscard]] std::string GetFunctionName() const;
  [[nodiscard]] cl_uint GetNumArgs() const;
  [[nodiscard]] cl_uint GetReferenceCount() const;
  [[nodiscard]] cl::Context GetContextNative() const;
  [[nodiscard]] cl::Program GetProgramNative() const;
  [[nodiscard]] std::string GetAttributes() const;

  /* ------------- Kernel Workgroup Info ------------------- */
  [[nodiscard]] std::size_t GetWorkGroupSize() const;
  [[nodiscard]] std::size_t GetPreferredWorkGroupSizeMultiple() const;
  [[nodiscard]] std::array<std::size_t, 3> GetCompileWorkGroupSize() const;
  [[nodiscard]] cl_ulong GetLocalMemSize() const;
  [[nodiscard]] cl_ulong GetPrivateMemSize() const;

  /* ------------- Kernel Workgroup Info ------------------- */
  [[nodiscard]] std::string GetArgAddressQualifier(cl_uint index) const;
  [[nodiscard]] std::string GetArgAccessQualifier(cl_uint index) const;
  [[nodiscard]] std::string GetArgTypeName(cl_uint index) const;
  [[nodiscard]] std::string GetArgTypeQualifier(cl_uint index) const;
  [[nodiscard]] std::string GetArgName(cl_uint index) const;

private:
  GGEMSOpenCLContext &context_;
  cl::Kernel kernel_;
  std::string kernel_name_;
};
} // namespace ggems::ocl
