#pragma once
// ************************************************************************
// * This file is part of GGEMS.                                          *
// *                                                                      *
// * GGEMS is free software: you can redistribute it and/or modify        *
// * it under the terms of the GNU General Public License as published by *
// * the Free Software Foundation, either version 3 of the License, or    *
// * (at your option) any later version.                                  *
// *                                                                      *
// * GGEMS is distributed in the hope that it will be useful,             *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
// * GNU General Public License for more details.                         *
// *                                                                      *
// * You should have received a copy of the GNU General Public License    *
// * along with GGEMS.  If not, see <https://www.gnu.org/licenses/>.      *
// *                                                                      *
// ************************************************************************

/*!
 * \file GGEMSOpenCLSVMBuffer.hh
 * \brief Declaration of GGEMSOpenCLSVMBuffer, a RAII wrapper for OpenCL SVM
 * buffers.
 *
 * This class encapsulates creation, mapping, unmapping and destruction of
 * Shared Virtual Memory (SVM) buffers as defined in OpenCL 2.x. It provides a
 * high-level, exception-safe abstraction while preserving direct access to
 * the underlying pointer when required by compute kernels.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-12-08
 * \version 2.0
 * \copyright
 * GNU General Public License v3.0
 */

#include "GGEMS/frameworks/GGEMSOpenCLExternal.hh"
#include "GGEMS/core/units/GGEMSUnits.hh"

namespace ggems::ocl {
class GGEMSOpenCLContext;
using units::operator""_B;

/*!
 * \class GGEMSOpenCLSVMBuffer
 * \brief RAII manager for OpenCL Shared Virtual Memory (SVM).
 *
 * This class provides a safe interface to allocate, map, unmap and release
 * SVM buffers. Its lifetime semantics ensure that memory is always released
 * correctly, even in the presence of exceptions or early returns. Mapping and
 * unmapping operations transparently account for fine-grain vs coarse-grain
 * SVM capabilities.
 */
class GGEMSOpenCLSVMBuffer {
public:
  /*!
   * \brief Constructs an SVM buffer using the given context and flags.
   *
   * The constructor allocates SVM memory with \c clSVMAlloc and verifies that
   * the device supports SVM. A non-zero size must be supplied.
   *
   * \param context   Reference to the OpenCL context managing this allocation.
   * \param size      Size of the buffer in bytes.
   * \param flags     OpenCL SVM memory flags (e.g., CL_MEM_READ_WRITE).
   * \param alignment Optional alignment requirement (0 = implementation
   * default).
   */
  GGEMSOpenCLSVMBuffer(GGEMSOpenCLContext &context, units::Bytes size,
                       cl_svm_mem_flags flags, units::Bytes alignment = 0_B);

  /*!
   * \brief Destructor.
   *
   * Ensures that any allocated SVM memory is released using \c clSVMFree.
   */
  ~GGEMSOpenCLSVMBuffer();

  /*!
   * \brief Move constructor.
   *
   * Transfers ownership of the SVM pointer and associated metadata. The
   * moved-from buffer is left in a valid but empty state.
   *
   * \param other Instance to move from.
   */
  GGEMSOpenCLSVMBuffer(GGEMSOpenCLSVMBuffer &&other) noexcept;

  /*!
   * \brief Move assignment operator.
   *
   * Releases any currently held SVM memory, then transfers ownership from
   * \p other. After assignment, the \p other buffer becomes empty.
   *
   * \param other SVM buffer to move from.
   * \return Reference to this object.
   */
  GGEMSOpenCLSVMBuffer &operator=(GGEMSOpenCLSVMBuffer &&other) noexcept;

  GGEMSOpenCLSVMBuffer(GGEMSOpenCLSVMBuffer const &) = delete;
  GGEMSOpenCLSVMBuffer &operator=(GGEMSOpenCLSVMBuffer const &) = delete;

public:
  /*!
   * \brief Returns a mutable pointer to the SVM memory.
   * \return Raw pointer to SVM data.
   */
  [[nodiscard]] void *GetData() noexcept { return ptr_; }

  /*!
   * \brief Returns a const pointer to the SVM memory.
   * \return Raw const pointer to SVM data.
   */
  [[nodiscard]] void const *GetData() const noexcept { return ptr_; }

  /*!
   * \brief Returns the size of the allocated SVM buffer.
   * \return Buffer size in bytes.
   */
  [[nodiscard]] units::Bytes GetSize() const noexcept { return size_; }

  /*!
   * \brief Returns the SVM flags used during allocation.
   * \return SVM memory flags.
   */
  [[nodiscard]] cl_svm_mem_flags GetFlags() const noexcept { return flags_; }

  /*!
   * \brief Maps the SVM memory into the host address space.
   *
   * For coarse-grain SVM, this requests the runtime to synchronise and
   * prepare the buffer for host access. Fine-grain system SVM does not
   * require explicit mapping.
   *
   * \param flags Mapping flags (e.g., CL_MAP_READ | CL_MAP_WRITE).
   */
  void Map(cl_map_flags flags = CL_MAP_READ | CL_MAP_WRITE);

  /*!
   * \brief Unmaps a previously mapped SVM buffer.
   *
   * On coarse-grain SVM devices, synchronisation back to the device is
   * performed. Fine-grain systems ignore this call.
   */
  void Unmap();

private:
  /*!
   * \brief Releases the SVM allocation if owned.
   *
   * Called by the destructor and move assignment operator. After this call,
   * the buffer becomes empty and safe to destroy.
   */
  void Release() noexcept;

private:
  GGEMSOpenCLContext *context_{nullptr}; /*!< Owning OpenCL context. */
  void *ptr_{nullptr};      /*!< Raw SVM pointer returned by clSVMAlloc. */
  units::Bytes size_{0ULL}; /*!< Size of the allocated SVM buffer. */
  cl_svm_mem_flags flags_{
      0}; /*!< OpenCL SVM memory flags used for allocation. */
};
} // namespace ggems::ocl
