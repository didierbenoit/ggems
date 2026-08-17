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
 * \brief Declares the OpenCL shared virtual memory buffer wrapper.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include "GGEMS/opencl/GGEMSOpenCLExternal.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMMemoryKind.hh"
#include "GGEMS/units/GGEMSBytesUnits.hh"

namespace ggems::ocl {

class GGEMSOpenCLContext;

/*!
 * \brief Makes the GGEMS byte literal available in the OpenCL namespace.
 */
using units::operator""_B;

/*!
 * \brief Owns an OpenCL shared virtual memory allocation.
 *
 * The buffer stores its allocation metadata, registers the allocation with its
 * OpenCL context, and performs explicit map and unmap operations when required
 * by the selected SVM memory kind.
 */
class GGEMSOpenCLSVMBuffer {
public:
  /*!
   * \brief Constructs an OpenCL shared virtual memory buffer.
   *
   * \param[in] context OpenCL context that owns the SVM allocation.
   * \param[in] size Requested allocation size.
   * \param[in] flags OpenCL SVM allocation flags.
   * \param[in] kind SVM memory kind associated with the allocation.
   * \param[in] alignment Requested allocation alignment in bytes.
   * \throws ggems::core::GGEMSFatal If the requested size is zero, the device
   *                                does not support SVM, or the allocation
   *                                fails.
   */
  GGEMSOpenCLSVMBuffer(GGEMSOpenCLContext &context, units::Bytes size,
                       cl_svm_mem_flags flags, SVMMemoryKind kind,
                       units::Bytes alignment = 0_B);

  /*!
   * \brief Destroys the OpenCL shared virtual memory buffer.
   */
  ~GGEMSOpenCLSVMBuffer() noexcept;

  /*!
   * \brief Move-constructs an OpenCL shared virtual memory buffer.
   *
   * \param[in,out] other Buffer whose allocation is transferred.
   */
  GGEMSOpenCLSVMBuffer(GGEMSOpenCLSVMBuffer &&other) noexcept;

  /*!
   * \brief Move-assigns an OpenCL shared virtual memory buffer.
   *
   * \param[in,out] other Buffer whose allocation is transferred.
   * \return Reference to this buffer.
   */
  auto operator=(GGEMSOpenCLSVMBuffer &&other) noexcept
      -> GGEMSOpenCLSVMBuffer &;

  /*!
   * \brief Disables copy construction.
   */
  GGEMSOpenCLSVMBuffer(GGEMSOpenCLSVMBuffer const &) = delete;

  /*!
   * \brief Disables copy assignment.
   */
  auto operator=(GGEMSOpenCLSVMBuffer const &)
      -> GGEMSOpenCLSVMBuffer & = delete;

  /*!
   * \brief Returns the SVM allocation pointer.
   *
   * \return Pointer to the SVM allocation.
   */
  [[nodiscard]] auto GetData() noexcept -> void * { return ptr_; }

  /*!
   * \brief Returns the SVM allocation pointer.
   *
   * \return Const pointer to the SVM allocation.
   */
  [[nodiscard]] auto GetData() const noexcept -> void const * { return ptr_; }

  /*!
   * \brief Returns the SVM allocation size.
   *
   * \return SVM allocation size.
   */
  [[nodiscard]] auto GetSize() const noexcept -> units::Bytes { return size_; }

  /*!
   * \brief Returns the OpenCL SVM allocation flags.
   *
   * \return OpenCL SVM allocation flags.
   */
  [[nodiscard]] auto GetFlags() const noexcept -> cl_svm_mem_flags {
    return flags_;
  }

  /*!
   * \brief Returns the SVM memory kind.
   *
   * \return SVM memory kind.
   */
  [[nodiscard]] auto GetKind() const noexcept -> SVMMemoryKind { return kind_; }

  /*!
   * \brief Maps the SVM allocation for host access when required.
   *
   * No mapping operation is performed for SVM memory kinds that do not require
   * explicit host mapping.
   *
   * \param[in] flags OpenCL map flags.
   */
  auto Map(cl_map_flags flags = CL_MAP_READ | CL_MAP_WRITE) -> void;

  /*!
   * \brief Unmaps the SVM allocation after host access when required.
   *
   * No unmap operation is performed for SVM memory kinds that do not require
   * explicit host mapping.
   */
  auto Unmap() -> void;

private:
  /*!
   * \brief Releases the SVM allocation and resets the buffer state.
   */
  auto Release() noexcept -> void;

  GGEMSOpenCLContext *context_{
      nullptr};               /*!< OpenCL context owning the allocation. */
  void *ptr_{nullptr};        /*!< Pointer to the SVM allocation. */
  units::Bytes size_{0ULL};   /*!< SVM allocation size. */
  cl_svm_mem_flags flags_{0}; /*!< OpenCL SVM allocation flags. */
  SVMMemoryKind kind_{SVMMemoryKind::None}; /*!< SVM memory kind. */
};
} // namespace ggems::ocl
