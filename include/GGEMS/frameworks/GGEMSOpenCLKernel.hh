#pragma once

#include "GGEMS/frameworks/GGEMSOpenCLContext.hh"
#include "GGEMS/frameworks/GGEMSOpenCLUtils.hh"

namespace ggems::ocl {

class GGEMSOpenCLKernel {
public:
  GGEMSOpenCLKernel(GGEMSOpenCLContext &ctx, cl::Kernel kernel,
                    std::string kernel_name);

  ~GGEMSOpenCLKernel() = default;

  GGEMSOpenCLKernel(GGEMSOpenCLKernel const &) = delete;
  GGEMSOpenCLKernel &operator=(GGEMSOpenCLKernel const &) = delete;
  GGEMSOpenCLKernel(GGEMSOpenCLKernel &&) noexcept = delete;
  GGEMSOpenCLKernel &operator=(GGEMSOpenCLKernel &&) noexcept = delete;

public:
  GGEMSOpenCLContext const &GetContext() const { return context_; }
  std::string_view GetKernelName() const { return kernel_name_; }

  /* --------- Arguments --------------------------------*/
  template <typename T> void SetArg(cl_uint index, T const &value) {
    cl_int err = kernel_.setArg(index, value);

    CheckCLError(err, std::format(
                          "Failed to set kernel argument {} for kernel '{}'",
                          index, kernel_name_));
  }

  void SetArgSVMPointer(cl_uint index, void *ptr);

  /* -------- Running -----------------------------*/
  // Exécution simple (1D pour l’instant)
  void Run(std::array<size_t, 1> const &global,
           std::array<size_t, 1> const &local);

  [[nodiscard]] cl::Event
  RunAndGetEvent(std::array<std::size_t, 1> const &global,
                 std::array<std::size_t, 1> const &local);

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
