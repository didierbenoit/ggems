// *****************************************************************************
// * This file is part of GGEMS.                                               *
// *                                                                           *
// * SPDX-License-Identifier: GPL-3.0-or-later                                 *
// * Copyright (C) 2017-2026 CHRU de Brest, Université de Bretagne Occidentale,*
// * Inserm.                                                                   *
// *                                                                           *
// * GGEMS is free software: you can redistribute it and/or modify             *
// * it under the terms of the GNU General Public License as published by      *
// * the Free Software Foundation, either version 3 of the License, or         *
// * (at your option) any later version.                                       *
// *                                                                           *
// * GGEMS is distributed in the hope that it will be useful,                  *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
// * GNU General Public License for more details.                              *
// *                                                                           *
// * You should have received a copy of the GNU General Public License         *
// * along with GGEMS. If not, see <https://www.gnu.org/licenses/>.            *
// *****************************************************************************

/*!
 * \file
 * \brief Implements the GGEMS OpenCL kernel wrapper.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <utility>
#include <cstddef>
#include <array>
#include <format>
#include <string>
/// \endcond

#include "GGEMS/logging/GGEMSLogMacros.hh"
#include "GGEMS/opencl/GGEMSOpenCLUtils.hh"
#include "GGEMS/opencl/GGEMSOpenCLKernel.hh"
#include "GGEMS/opencl/GGEMSOpenCLStrings.hh"

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

  CheckCLError(queue.finish(),
               std::format("Failed to finish kernel '{}'", kernel_name_));

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
