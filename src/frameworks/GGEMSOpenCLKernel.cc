#include "GGEMS/frameworks/GGEMSOpenCLKernel.hh"
#include "GGEMS/core/GGEMSLogMacros.hh"
#include "GGEMS/frameworks/GGEMSOpenCLUtils.hh"

using namespace ggems::units;

namespace ggems::ocl {

/* ---------------------------------------------------------------------------*/

GGEMSOpenCLKernel::GGEMSOpenCLKernel(GGEMSOpenCLContext &ctx, cl::Kernel kernel,
                                     std::string kernel_name)
    : context_(ctx), kernel_(std::move(kernel)),
      kernel_name_(std::move(kernel_name)) {
  GGEMS_INFOEX("OpenCL", 3, "OpenCL kernel '{}' created.", kernel_name_);
}

/* ------------------------------------------------------------------------ */

void GGEMSOpenCLKernel::SetArgSVMPointer(cl_uint index, void *ptr) {
  cl_int err = clSetKernelArgSVMPointer(kernel_(), index, ptr);
  CheckCLError(err, std::format("Failed to set SVM arg {}", index));
}

/* ------------------------------------------------------------------------ */

void GGEMSOpenCLKernel::Run(std::array<std::size_t, 1> const &global,
                            std::array<std::size_t, 1> const &local) {
  (void)RunAndGetEvent(global, local);
}

/* ------------------------------------------------------------------------ */

cl::Event
GGEMSOpenCLKernel::RunAndGetEvent(std::array<std::size_t, 1> const &global,
                                  std::array<std::size_t, 1> const &local) {
  cl::Event event;

  auto &queue = context_.GetCommandQueueNative();

  cl_int err =
      queue.enqueueNDRangeKernel(kernel_, cl::NullRange, cl::NDRange(global[0]),
                                 cl::NDRange(local[0]), nullptr, &event);
  CheckCLError(err,
               std::format("Failed to enqueue kernel '{}'", kernel_name_));

  queue.finish();

  return event;
}

/* ---------------------------------------------------------------------- */

std::string GGEMSOpenCLKernel::GetFunctionName() const {
  return GetInfo<CL_KERNEL_FUNCTION_NAME>(kernel_);
}

/* -------------------------------------------------------------------------- */

cl_uint GGEMSOpenCLKernel::GetNumArgs() const {
  return GetInfo<CL_KERNEL_NUM_ARGS>(kernel_);
}

/* -------------------------------------------------------------------------- */

cl_uint GGEMSOpenCLKernel::GetReferenceCount() const {
  return GetInfo<CL_KERNEL_REFERENCE_COUNT>(kernel_);
}

/* -------------------------------------------------------------------------- */

cl::Context GGEMSOpenCLKernel::GetContextNative() const {
  return GetInfo<CL_KERNEL_CONTEXT>(kernel_);
}

/* -------------------------------------------------------------------------- */

cl::Program GGEMSOpenCLKernel::GetProgramNative() const {
  return GetInfo<CL_KERNEL_PROGRAM>(kernel_);
}

/* -------------------------------------------------------------------------- */

std::string GGEMSOpenCLKernel::GetAttributes() const {
  return GetInfo<CL_KERNEL_ATTRIBUTES>(kernel_);
}

/* -------------------------------------------------------------------------- */

[[nodiscard]] std::size_t GGEMSOpenCLKernel::GetWorkGroupSize() const {
  auto const &devs = context_.GetNativeDevices();
  return GetWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(kernel_, devs.front());
}

/* -------------------------------------------------------------------------- */

[[nodiscard]] std::size_t
GGEMSOpenCLKernel::GetPreferredWorkGroupSizeMultiple() const {
  auto devs = context_.GetNativeDevices();
  return GetWorkGroupInfo<CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE>(
      kernel_, devs.front());
}

/* -------------------------------------------------------------------------- */

[[nodiscard]] std::array<std::size_t, 3>
GGEMSOpenCLKernel::GetCompileWorkGroupSize() const {
  auto devs = context_.GetNativeDevices();
  return GetWorkGroupInfo<CL_KERNEL_COMPILE_WORK_GROUP_SIZE>(kernel_,
                                                             devs.front());
}

/* -------------------------------------------------------------------------- */

[[nodiscard]] cl_ulong GGEMSOpenCLKernel::GetLocalMemSize() const {
  auto devs = context_.GetNativeDevices();
  return GetWorkGroupInfo<CL_KERNEL_LOCAL_MEM_SIZE>(kernel_, devs.front());
}

/* -------------------------------------------------------------------------- */

[[nodiscard]] cl_ulong GGEMSOpenCLKernel::GetPrivateMemSize() const {
  auto devs = context_.GetNativeDevices();
  return GetWorkGroupInfo<CL_KERNEL_PRIVATE_MEM_SIZE>(kernel_, devs.front());
}

/* -------------------------------------------------------------------------- */

[[nodiscard]] std::string
GGEMSOpenCLKernel::GetArgAddressQualifier(cl_uint index) const {
  return ArgAddressQualifierToString(
      GetArgInfo<CL_KERNEL_ARG_ADDRESS_QUALIFIER>(kernel_, index));
}

/* -------------------------------------------------------------------------- */
[[nodiscard]] std::string
GGEMSOpenCLKernel::GetArgAccessQualifier(cl_uint index) const {
  return ArgAccessQualifierToString(
      GetArgInfo<CL_KERNEL_ARG_ACCESS_QUALIFIER>(kernel_, index));
}

/* -------------------------------------------------------------------------- */
[[nodiscard]] std::string
GGEMSOpenCLKernel::GetArgTypeName(cl_uint index) const {
  return GetArgInfo<CL_KERNEL_ARG_TYPE_NAME>(kernel_, index);
}

/* -------------------------------------------------------------------------- */
[[nodiscard]] std::string
GGEMSOpenCLKernel::GetArgTypeQualifier(cl_uint index) const {
  return ArgTypeQualifierToString(
      GetArgInfo<CL_KERNEL_ARG_TYPE_QUALIFIER>(kernel_, index));
}

/* -------------------------------------------------------------------------- */
[[nodiscard]] std::string GGEMSOpenCLKernel::GetArgName(cl_uint index) const {
  return GetArgInfo<CL_KERNEL_ARG_NAME>(kernel_, index);
}

} // namespace ggems::ocl
