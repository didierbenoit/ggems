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
 * \brief Declares the GGEMS OpenCL kernel wrapper.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <array>
#include <cstddef>
#include <format>
#include <string>
#include <string_view>
/// \endcond

#include "GGEMS/opencl/GGEMSOpenCLContext.hh"
#include "GGEMS/opencl/GGEMSOpenCLUtils.hh"

namespace ggems::ocl {

/*!
 * \brief Wraps a native OpenCL kernel with its GGEMS context and identity.
 */
class GGEMSOpenCLKernel {
public:
  /*!
   * \brief Constructs an OpenCL kernel wrapper.
   *
   * \param[in] context GGEMS OpenCL context used to execute the kernel.
   * \param[in] kernel Native OpenCL kernel.
   * \param[in] kernel_name Kernel function name.
   */
  GGEMSOpenCLKernel(GGEMSOpenCLContext const &context, cl::Kernel kernel,
                    std::string kernel_name);

  /*!
   * \brief Destroys the OpenCL kernel wrapper.
   */
  ~GGEMSOpenCLKernel() = default;

  /*!
   * \brief Disables copy construction.
   */
  GGEMSOpenCLKernel(GGEMSOpenCLKernel const &) = delete;

  /*!
   * \brief Disables copy assignment.
   */
  auto operator=(GGEMSOpenCLKernel const &) -> GGEMSOpenCLKernel & = delete;

  /*!
   * \brief Disables move construction.
   */
  GGEMSOpenCLKernel(GGEMSOpenCLKernel &&) noexcept = delete;

  /*!
   * \brief Disables move assignment.
   */
  auto operator=(GGEMSOpenCLKernel &&) noexcept -> GGEMSOpenCLKernel & = delete;

  /*!
   * \brief Returns the GGEMS OpenCL context used by this kernel.
   *
   * \return Associated GGEMS OpenCL context.
   */
  [[nodiscard]] auto GetContext() const noexcept -> GGEMSOpenCLContext const & {
    return context_;
  }

  /*!
   * \brief Returns the kernel function name.
   *
   * \return Kernel function name.
   */
  [[nodiscard]] auto GetKernelName() const noexcept -> std::string_view {
    return kernel_name_;
  }

  /*!
   * \brief Sets a typed OpenCL kernel argument.
   *
   * \tparam T Kernel argument value type.
   * \param[in] index Kernel argument index.
   * \param[in] value Kernel argument value.
   */
  template <typename T> auto SetArg(cl_uint index, T const &value) -> void {
    cl_int error = kernel_.setArg(index, value);

    CheckCLError(error,
                 std::format("Failed to set kernel argument {} for kernel '{}'",
                             index, kernel_name_));
  }

  /*!
   * \brief Sets an OpenCL SVM pointer kernel argument.
   *
   * \param[in] index Kernel argument index.
   * \param[in] pointer SVM pointer value.
   */
  auto SetArgSVMPointer(cl_uint index, void const *pointer) -> void;

  /*!
   * \brief Executes the kernel and waits for completion.
   *
   * \param[in] global One-dimensional global work size.
   * \param[in] local One-dimensional local work size.
   */
  auto Run(std::array<std::size_t, 1> const &global,
           std::array<std::size_t, 1> const &local) -> void;

  /*!
   * \brief Executes the kernel, waits for completion, and returns its event.
   *
   * \param[in] global One-dimensional global work size.
   * \param[in] local One-dimensional local work size.
   * \return Completed OpenCL kernel event.
   */
  [[nodiscard]] auto RunAndGetEvent(std::array<std::size_t, 1> const &global,
                                    std::array<std::size_t, 1> const &local)
      -> cl::Event;

  /*!
   * \brief Returns the OpenCL kernel function name.
   *
   * \return Kernel function name.
   */
  [[nodiscard]] auto GetFunctionName() const -> std::string;

  /*!
   * \brief Returns the number of kernel arguments.
   *
   * \return Kernel argument count.
   */
  [[nodiscard]] auto GetNumArgs() const -> cl_uint;

  /*!
   * \brief Returns the OpenCL kernel reference count.
   *
   * \return Kernel reference count.
   */
  [[nodiscard]] auto GetReferenceCount() const -> cl_uint;

  /*!
   * \brief Returns the native OpenCL context associated with the kernel.
   *
   * \return Associated native OpenCL context.
   */
  [[nodiscard]] auto GetContextNative() const -> cl::Context;

  /*!
   * \brief Returns the native OpenCL program associated with the kernel.
   *
   * \return Associated native OpenCL program.
   */
  [[nodiscard]] auto GetProgramNative() const -> cl::Program;

  /*!
   * \brief Returns the OpenCL kernel attribute string.
   *
   * \return Kernel attribute string.
   */
  [[nodiscard]] auto GetAttributes() const -> std::string;

  /*!
   * \brief Returns the maximum work-group size for this kernel and device.
   *
   * \return Maximum work-group size.
   */
  [[nodiscard]] auto GetWorkGroupSize() const -> std::size_t;

  /*!
   * \brief Returns the preferred work-group-size multiple.
   *
   * \return Preferred work-group-size multiple.
   */
  [[nodiscard]] auto GetPreferredWorkGroupSizeMultiple() const -> std::size_t;

  /*!
   * \brief Returns the compile-time work-group size.
   *
   * \return Compile-time work-group dimensions.
   */
  [[nodiscard]] auto GetCompileWorkGroupSize() const
      -> std::array<std::size_t, 3>;

  /*!
   * \brief Returns the kernel local-memory usage.
   *
   * \return Local-memory usage in bytes.
   */
  [[nodiscard]] auto GetLocalMemSize() const -> cl_ulong;

  /*!
   * \brief Returns the kernel private-memory usage.
   *
   * \return Private-memory usage in bytes.
   */
  [[nodiscard]] auto GetPrivateMemSize() const -> cl_ulong;

  /*!
   * \brief Returns the address-space qualifier for a kernel argument.
   *
   * \param[in] index Kernel argument index.
   * \return Kernel argument address-space qualifier.
   */
  [[nodiscard]] auto GetArgAddressQualifier(cl_uint index) const -> std::string;

  /*!
   * \brief Returns the access qualifier for a kernel argument.
   *
   * \param[in] index Kernel argument index.
   * \return Kernel argument access qualifier.
   */
  [[nodiscard]] auto GetArgAccessQualifier(cl_uint index) const -> std::string;

  /*!
   * \brief Returns the type name for a kernel argument.
   *
   * \param[in] index Kernel argument index.
   * \return Kernel argument type name.
   */
  [[nodiscard]] auto GetArgTypeName(cl_uint index) const -> std::string;

  /*!
   * \brief Returns the type qualifier for a kernel argument.

   * \param[in] index Kernel argument index.
   * \return Kernel argument type qualifier.
   */
  [[nodiscard]] auto GetArgTypeQualifier(cl_uint index) const -> std::string;

  /*!
   * \brief Returns the name for a kernel argument.
   *
   * \param[in] index Kernel argument index.
   * \return Kernel argument name.
   */
  [[nodiscard]] auto GetArgName(cl_uint index) const -> std::string;

private:
  GGEMSOpenCLContext const &context_; /*!< OpenCL context used for execution. */
  cl::Kernel kernel_;                 /*!< Native OpenCL kernel. */
  std::string kernel_name_;           /*!< Kernel function name. */
};

} // namespace ggems::ocl
