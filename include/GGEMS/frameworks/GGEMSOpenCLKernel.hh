#pragma once

#include <array>
#include <cstddef>
#include <format>
#include <string>
#include <string_view>

#include "GGEMS/frameworks/GGEMSOpenCLContext.hh"
#include "GGEMS/frameworks/GGEMSOpenCLUtils.hh"

namespace ggems::ocl {

class GGEMSOpenCLKernel {
public:
  GGEMSOpenCLKernel(GGEMSOpenCLContext const &context, cl::Kernel kernel,
                    std::string kernel_name);

  ~GGEMSOpenCLKernel() = default;

  GGEMSOpenCLKernel(GGEMSOpenCLKernel const &) = delete;
  auto operator=(GGEMSOpenCLKernel const &) -> GGEMSOpenCLKernel & = delete;
  GGEMSOpenCLKernel(GGEMSOpenCLKernel &&) noexcept = delete;
  auto operator=(GGEMSOpenCLKernel &&) noexcept -> GGEMSOpenCLKernel & = delete;

  [[nodiscard]] auto GetContext() const noexcept -> GGEMSOpenCLContext const & {
    return context_;
  }

  [[nodiscard]] auto GetKernelName() const noexcept -> std::string_view {
    return kernel_name_;
  }

  template <typename T> auto SetArg(cl_uint index, T const &value) -> void {
    cl_int error = kernel_.setArg(index, value);

    CheckCLError(error,
                 std::format("Failed to set kernel argument {} for kernel '{}'",
                             index, kernel_name_));
  }

  auto SetArgSVMPointer(cl_uint index, void const *pointer) -> void;

  auto Run(std::array<std::size_t, 1> const &global,
           std::array<std::size_t, 1> const &local) -> void;

  [[nodiscard]] auto RunAndGetEvent(std::array<std::size_t, 1> const &global,
                                    std::array<std::size_t, 1> const &local)
      -> cl::Event;

  [[nodiscard]] auto GetFunctionName() const -> std::string;
  [[nodiscard]] auto GetNumArgs() const -> cl_uint;
  [[nodiscard]] auto GetReferenceCount() const -> cl_uint;
  [[nodiscard]] auto GetContextNative() const -> cl::Context;
  [[nodiscard]] auto GetProgramNative() const -> cl::Program;
  [[nodiscard]] auto GetAttributes() const -> std::string;

  [[nodiscard]] auto GetWorkGroupSize() const -> std::size_t;
  [[nodiscard]] auto GetPreferredWorkGroupSizeMultiple() const -> std::size_t;
  [[nodiscard]] auto GetCompileWorkGroupSize() const
      -> std::array<std::size_t, 3>;
  [[nodiscard]] auto GetLocalMemSize() const -> cl_ulong;
  [[nodiscard]] auto GetPrivateMemSize() const -> cl_ulong;

  [[nodiscard]] auto GetArgAddressQualifier(cl_uint index) const -> std::string;
  [[nodiscard]] auto GetArgAccessQualifier(cl_uint index) const -> std::string;
  [[nodiscard]] auto GetArgTypeName(cl_uint index) const -> std::string;
  [[nodiscard]] auto GetArgTypeQualifier(cl_uint index) const -> std::string;
  [[nodiscard]] auto GetArgName(cl_uint index) const -> std::string;

private:
  GGEMSOpenCLContext const &context_;
  cl::Kernel kernel_;
  std::string kernel_name_;
};
} // namespace ggems::ocl
