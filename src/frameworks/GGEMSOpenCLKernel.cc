#include <utility>
#include <cstddef>
#include <array>
#include <format>
#include <string>

#include "GGEMS/core/GGEMSLogMacros.hh"
#include "GGEMS/frameworks/GGEMSOpenCLUtils.hh"
#include "GGEMS/frameworks/GGEMSOpenCLKernel.hh"
#include "GGEMS/frameworks/GGEMSOpenCLStrings.hh"

using namespace ggems::units;

namespace ggems::ocl {

// -----------------------------------------------------------------------------

GGEMSOpenCLKernel::GGEMSOpenCLKernel(GGEMSOpenCLContext const &context,
                                     cl::Kernel kernel, std::string kernel_name)
    : context_{context}, kernel_{std::move(kernel)},
      kernel_name_{std::move(kernel_name)} {
  GGEMS_INFOEX("OpenCL", 3, "OpenCL kernel '{}' created.", kernel_name_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLKernel::SetArgSVMPointer(cl_uint index, void const *pointer)
    -> void {
  cl_int error = clSetKernelArgSVMPointer(kernel_(), index, pointer);
  CheckCLError(error, std::format("Failed to set SVM arg {}", index));
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLKernel::Run(std::array<std::size_t, 1> const &global,
                            std::array<std::size_t, 1> const &local) -> void {
  (void)RunAndGetEvent(global, local);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLKernel::RunAndGetEvent(std::array<std::size_t, 1> const &global,
                                       std::array<std::size_t, 1> const &local)
    -> cl::Event {
  cl::Event event;

  auto const &queue = context_.GetCommandQueueNative();

  cl_int error =
      queue.enqueueNDRangeKernel(kernel_, cl::NullRange, cl::NDRange(global[0]),
                                 cl::NDRange(local[0]), nullptr, &event);
  CheckCLError(error,
               std::format("Failed to enqueue kernel '{}'", kernel_name_));

  queue.finish();

  return event;
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLKernel::GetFunctionName() const -> std::string {
  return GetInfo<CL_KERNEL_FUNCTION_NAME>(kernel_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLKernel::GetNumArgs() const -> cl_uint {
  return GetInfo<CL_KERNEL_NUM_ARGS>(kernel_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLKernel::GetReferenceCount() const -> cl_uint {
  return GetInfo<CL_KERNEL_REFERENCE_COUNT>(kernel_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLKernel::GetContextNative() const -> cl::Context {
  return GetInfo<CL_KERNEL_CONTEXT>(kernel_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLKernel::GetProgramNative() const -> cl::Program {
  return GetInfo<CL_KERNEL_PROGRAM>(kernel_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLKernel::GetAttributes() const -> std::string {
  return GetInfo<CL_KERNEL_ATTRIBUTES>(kernel_);
}

// -----------------------------------------------------------------------------

[[nodiscard]] auto GGEMSOpenCLKernel::GetWorkGroupSize() const -> std::size_t {
  auto const devices = context_.GetNativeDevices();
  return GetWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(kernel_, devices.front());
}

// -----------------------------------------------------------------------------

[[nodiscard]] auto GGEMSOpenCLKernel::GetPreferredWorkGroupSizeMultiple() const
    -> std::size_t {
  auto const devices = context_.GetNativeDevices();
  return GetWorkGroupInfo<CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE>(
      kernel_, devices.front());
}

// -----------------------------------------------------------------------------

[[nodiscard]] auto GGEMSOpenCLKernel::GetCompileWorkGroupSize() const
    -> std::array<std::size_t, 3> {
  auto const devices = context_.GetNativeDevices();
  return GetWorkGroupInfo<CL_KERNEL_COMPILE_WORK_GROUP_SIZE>(kernel_,
                                                             devices.front());
}

// -----------------------------------------------------------------------------

[[nodiscard]] auto GGEMSOpenCLKernel::GetLocalMemSize() const -> cl_ulong {
  auto const devices = context_.GetNativeDevices();
  return GetWorkGroupInfo<CL_KERNEL_LOCAL_MEM_SIZE>(kernel_, devices.front());
}

// -----------------------------------------------------------------------------

[[nodiscard]] auto GGEMSOpenCLKernel::GetPrivateMemSize() const -> cl_ulong {
  auto const devices = context_.GetNativeDevices();
  return GetWorkGroupInfo<CL_KERNEL_PRIVATE_MEM_SIZE>(kernel_, devices.front());
}

// -----------------------------------------------------------------------------

[[nodiscard]] auto
GGEMSOpenCLKernel::GetArgAddressQualifier(cl_uint index) const -> std::string {
  return ArgAddressQualifierToString(
      GetArgInfo<CL_KERNEL_ARG_ADDRESS_QUALIFIER>(kernel_, index));
}

// -----------------------------------------------------------------------------
[[nodiscard]] auto GGEMSOpenCLKernel::GetArgAccessQualifier(cl_uint index) const
    -> std::string {
  return ArgAccessQualifierToString(
      GetArgInfo<CL_KERNEL_ARG_ACCESS_QUALIFIER>(kernel_, index));
}

// -----------------------------------------------------------------------------
[[nodiscard]] auto GGEMSOpenCLKernel::GetArgTypeName(cl_uint index) const
    -> std::string {
  return GetArgInfo<CL_KERNEL_ARG_TYPE_NAME>(kernel_, index);
}

// -----------------------------------------------------------------------------
[[nodiscard]] auto GGEMSOpenCLKernel::GetArgTypeQualifier(cl_uint index) const
    -> std::string {
  return ArgTypeQualifierToString(
      GetArgInfo<CL_KERNEL_ARG_TYPE_QUALIFIER>(kernel_, index));
}

// -----------------------------------------------------------------------------
[[nodiscard]] auto GGEMSOpenCLKernel::GetArgName(cl_uint index) const
    -> std::string {
  return GetArgInfo<CL_KERNEL_ARG_NAME>(kernel_, index);
}

} // namespace ggems::ocl
