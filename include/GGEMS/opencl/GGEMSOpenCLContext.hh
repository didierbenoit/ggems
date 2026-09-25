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
 * \brief Declares the GGEMS OpenCL context and SVM/VRAM state wrappers.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <cstddef>
#include <cstdint>
#include <vector>
/// \endcond

#include "GGEMS/opencl/GGEMSOpenCLDevice.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMMemoryKind.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMBuffer.hh"
#include "GGEMS/units/GGEMSBytesUnits.hh"

namespace ggems::ocl {
using units::operator""_B;

/*!
 * \brief Describes the SVM modes GGEMS can use with an OpenCL context.
 *
 * On native OpenCL backends, this support reflects device-reported SVM
 * capabilities. On Apple, it reflects the coarse-only GGEMS compatibility
 * implementation while the native device capability remains unavailable.
 */
struct SVMSupport {
  /*! \brief Whether coarse-grain buffer SVM is supported. */
  bool coarse_grain_buffer{false};

  /*! \brief Whether fine-grain buffer SVM is supported. */
  bool fine_grain_buffer{false};

  /*! \brief Whether fine-grain system SVM is supported. */
  bool fine_grain_system{false};

  /*! \brief Whether SVM atomic operations are supported. */
  bool atomics{false};

  /*!
   * \brief Returns the default supported SVM memory kind.
   *
   * \return Preferred supported SVM memory kind, or SVMMemoryKind::None when
   * SVM is unavailable.
   *
   * Preference is coarse-grain buffer, then fine-grain buffer without atomics,
   * then fine-grain system. Atomics are selected only by an explicit request.
   */
  [[nodiscard]] auto DefaultKind() const noexcept -> SVMMemoryKind {
    if (coarse_grain_buffer) {
      return SVMMemoryKind::CoarseGrainBuffer;
    }

    if (fine_grain_buffer) {
      return SVMMemoryKind::FineGrainBuffer;
    }

    if (fine_grain_system) {
      return SVMMemoryKind::FineGrainSystem;
    }

    return SVMMemoryKind::None;
  }

  /*!
   * \brief Checks whether a specific SVM memory kind is supported.
   *
   * \param[in] kind SVM memory kind to test.
   * \return True if the requested memory kind is supported, false otherwise.
   */
  [[nodiscard]] auto Supports(SVMMemoryKind kind) const noexcept -> bool {
    switch (kind) {
    case SVMMemoryKind::None:
      return false;
    case SVMMemoryKind::Auto:
      return HasAny();
    case SVMMemoryKind::CoarseGrainBuffer:
      return coarse_grain_buffer;
    case SVMMemoryKind::FineGrainBuffer:
      return fine_grain_buffer;
    case SVMMemoryKind::FineGrainBufferAtomics:
      return fine_grain_buffer && atomics;
    case SVMMemoryKind::FineGrainSystem:
      return fine_grain_system;
    }

    return false;
  }

  /*!
   * \brief Checks whether any SVM mode is supported.
   *
   * \return True if at least one SVM mode is supported, false otherwise.
   */
  [[nodiscard]] auto HasAny() const noexcept -> bool {
    return coarse_grain_buffer || fine_grain_buffer || fine_grain_system;
  }
};

/*!
 * \brief Tracks OpenCL device memory usage attributed to GGEMS SVM allocations.
 *
 * The accounting includes only SVM allocations registered through this
 * context. It does not query real-time memory consumed by the driver, other
 * contexts, or other processes.
 */
struct VRAMUsage {
  /*! \brief Total device global memory. */
  units::Bytes total{0_B};

  /*! \brief Allocated GGEMS SVM memory. */
  units::Bytes allocated{0_B};

  /*! \brief Remaining tracked device-memory budget. */
  units::Bytes available{0_B};

  /*! \brief Peak GGEMS SVM allocation. */
  units::Bytes peak{0_B};

  /*! \brief Current SVM allocation count. */
  std::size_t allocation_count{0};

  /*!
   * \brief Computes the integer percentage of tracked allocated memory.
   * \pre For an ordinary percentage, 100 times allocated.value must fit
   * uint64_t and the quotient must fit uint8_t. This value record does not
   * clamp either arithmetic step.
   * \return Zero when total is zero; otherwise the truncated integer percentage
   * cast to uint8_t.
   */
  [[nodiscard]] auto GetPercent() const noexcept -> std::uint8_t {
    if (total.value == 0LL) {
      return 0U;
    }

    return static_cast<std::uint8_t>((100ULL * allocated.value) / total.value);
  }
};

/*!
 * \brief Owns an OpenCL context and command queue for one GGEMS OpenCL device.
 *
 * The wrapper also records effective GGEMS SVM support and tracks GGEMS-owned
 * SVM allocation accounting for the device.
 *
 * The device is borrowed and must outlive this wrapper. The wrapper must stay
 * at a stable address while SVM buffers or kernels borrow it. Its queue is in
 * order and profiling-enabled. Allocation accounting is not synchronized;
 * callers serialize allocation, release, and accounting reads. Native
 * information getters propagate GGEMSRecoverable on failed OpenCL queries.
 */
class GGEMSOpenCLContext {
public:
  /*!
   * \brief Constructs an OpenCL context for a device.
   *
   * \param[in] device OpenCL device associated with the context.
   *
   * \throws ggems::core::GGEMSFatal If native context or queue creation fails.
   * \throws ggems::core::GGEMSRecoverable If a required device query fails.
   */
  explicit GGEMSOpenCLContext(GGEMSOpenCLDevice const &device);

  /*! \brief Destroys the OpenCL context wrapper. */
  ~GGEMSOpenCLContext() = default;

  /*! \brief Disables copy construction. */
  GGEMSOpenCLContext(GGEMSOpenCLContext const &) = delete;

  /*! \brief Move-constructs an OpenCL context wrapper. */
  GGEMSOpenCLContext(GGEMSOpenCLContext &&) noexcept = default;

  /*! \brief Disables copy assignment. */
  auto operator=(GGEMSOpenCLContext const &) -> GGEMSOpenCLContext & = delete;

  /*! \brief Disables move assignment. */
  auto operator=(GGEMSOpenCLContext &&) -> GGEMSOpenCLContext & = delete;

  /*!
   * \brief Returns the native OpenCL context.
   *
   * \return Native OpenCL context.
   */
  [[nodiscard]] auto GetContextNative() const noexcept -> cl::Context const & {
    return context_;
  }

  /*!
   * \brief Returns the GGEMS device associated with this context.
   *
   * \return Associated GGEMS OpenCL device.
   */
  [[nodiscard]] auto GetDevice() const noexcept -> GGEMSOpenCLDevice const & {
    return device_;
  }

  /*!
   * \brief Returns the native OpenCL command queue.
   *
   * \return Native OpenCL command queue.
   */
  [[nodiscard]] auto GetCommandQueueNative() const noexcept
    -> cl::CommandQueue const & {
    return command_queue_;
  }

  /*!
   * \brief Returns the effective GGEMS SVM support set.
   *
   * \return Effective GGEMS SVM support.
   */
  [[nodiscard]] auto GetSVMSupport() const noexcept -> SVMSupport const & {
    return svm_support_;
  }

  /*!
   * \brief Registers a newly created SVM allocation in VRAM accounting.
   *
   * \param[in] size Allocated byte count.
   *
   * \pre Registration must correspond to one live allocation, and byte/count
   * additions must fit their storage types. This updates counters only and
   * performs no capacity check.
   */
  auto RegisterSVMAllocation(units::Bytes size) noexcept -> void;

  /*!
   * \brief Registers an SVM release in VRAM accounting.
   *
   * \param[in] size Released byte count.
   *
   * Subtracts the byte count with saturation at zero and decrements the
   * allocation count only when nonzero. It does not free memory or verify
   * allocation identity.
   */
  auto RegisterSVMRelease(units::Bytes size) noexcept -> void;

  /*!
   * \brief Returns the current VRAM accounting state.
   *
   * \return VRAM accounting state.
   */
  [[nodiscard]] auto GetVRAMUsage() const noexcept -> VRAMUsage const & {
    return vram_usage_;
  }

  /*!
   * \brief Returns the total device memory capacity.
   *
   * \return Total device memory.
   */
  [[nodiscard]] auto GetTotalVRAM() const noexcept -> units::Bytes {
    return vram_usage_.total;
  }

  /*!
   * \brief Returns the memory currently allocated by GGEMS SVM buffers.
   *
   * \return Currently allocated memory.
   */
  [[nodiscard]] auto GetAllocatedVRAM() const noexcept -> units::Bytes {
    return vram_usage_.allocated;
  }

  /*!
   * \brief Returns the remaining GGEMS-tracked device memory budget.
   *
   * This value is the declared device global-memory capacity minus SVM
   * allocations registered by this context, clamped to zero. It is not a
   * real-time query of
   * memory used outside GGEMS.
   *
   * \return Remaining GGEMS-tracked device memory.
   */
  [[nodiscard]] auto GetAvailableVRAM() const noexcept -> units::Bytes {
    return vram_usage_.available;
  }

  /*!
   * \brief Returns the peak GGEMS SVM allocation.
   *
   * \return Peak allocated memory.
   */
  [[nodiscard]] auto GetPeakVRAM() const noexcept -> units::Bytes {
    return vram_usage_.peak;
  }

  /*!
   * \brief Returns the current SVM allocation count.
   *
   * \return Current SVM allocation count.
   */
  [[nodiscard]] auto GetAllocationCountVRAM() const noexcept -> std::size_t {
    return vram_usage_.allocation_count;
  }

  /*!
   * \brief Returns the current GGEMS VRAM usage percentage.
   *
   * \return Current VRAM usage percentage.
   */
  [[nodiscard]] auto GetPercentVRAM() const noexcept -> std::uint8_t {
    return vram_usage_.GetPercent();
  }

  /*!
   * \brief Creates an SVM buffer supported by this context.
   *
   * Rejects allocations larger than CL_DEVICE_MAX_MEM_ALLOC_SIZE or the
   * remaining GGEMS-tracked device memory. Oversized requests are not split or
   * oversubscribed automatically.
   *
   * \param[in] size Requested buffer size.
   * \param[in] kind Requested SVM memory kind.
   * \param[in] alignment Requested allocation alignment in bytes.
   * \return Owned SVM buffer.
   * \throws ggems::core::GGEMSFatal If SVM is unavailable, the requested kind
   * is invalid or unsupported, the requested size exceeds an allocation limit,
   * or the OpenCL allocation fails.
   *
   * The returned buffer borrows this wrapper until release. A zero size, an
   * alignment outside cl_uint, or a failed allocation also raises GGEMSFatal.
   * Alignment zero requests the backend default; other alignment constraints
   * are delegated to the allocator.
   */
  [[nodiscard]] auto CreateSVMBuffer(units::Bytes size,
                                     SVMMemoryKind kind = SVMMemoryKind::Auto,
                                     units::Bytes alignment = 0_B)
    -> GGEMSOpenCLSVMBuffer;

  /*!
   * \brief Maps SVM memory for host access and waits for the blocking map to
   * complete.
   *
   * \param[in,out] pointer SVM allocation to map.
   * \param[in] size Mapped byte count.
   * \param[in] flags OpenCL host mapping flags.
   *
   * \throws ggems::core::GGEMSFatal If the OpenCL map/unmap request or required
   * wait fails.
   */
  auto EnqueueSVMMap(void *pointer, units::Bytes size,
                     cl_map_flags flags = CL_MAP_READ | CL_MAP_WRITE) const
    -> void;

  /*!
   * \brief Unmaps SVM memory and waits for the unmap event to complete.
   *
   * \param[in,out] pointer SVM allocation to unmap.
   *
   * \throws ggems::core::GGEMSFatal If the OpenCL map/unmap request or required
   * wait fails.
   */
  auto EnqueueSVMUnmap(void *pointer) const -> void;

  /*!
   * \brief Returns the OpenCL context reference count.
   *
   * \return OpenCL context reference count.
   */
  [[nodiscard]] auto GetReferenceCount() const -> cl_uint;

  /*!
   * \brief Returns the number of devices in the native context.
   *
   * \return Number of devices in the native context.
   */
  [[nodiscard]] auto GetNumDevices() const -> cl_uint;

  /*!
   * \brief Returns the devices contained in the native context.
   *
   * \return Native OpenCL devices.
   */
  [[nodiscard]] auto GetNativeDevices() const -> std::vector<cl::Device>;

  /*!
   * \brief Returns the OpenCL context property list.
   *
   * \return OpenCL context properties.
   */
  [[nodiscard]] auto GetProperties() const
    -> std::vector<cl_context_properties>;

  /*! \brief Prints native context information. */
  auto PrintContext() const -> void;

  /*! \brief Prints native command-queue information. */
  auto PrintCommandQueue() const -> void;

  /*!
   * \brief Returns the context associated with the command queue.
   *
   * \return Native context associated with the command queue.
   */
  [[nodiscard]] auto GetQueueContext() const -> cl::Context;

  /*!
   * \brief Returns the device associated with the command queue.
   *
   * \return Native device associated with the command queue.
   */
  [[nodiscard]] auto GetQueueDevice() const -> cl::Device;

  /*!
   * \brief Returns the OpenCL command-queue reference count.
   *
   * \return Command-queue reference count.
   */
  [[nodiscard]] auto GetQueueReferenceCount() const -> cl_uint;

  /*!
   * \brief Returns the OpenCL command-queue property bitfield.
   *
   * \return Command-queue property bitfield.
   */
  [[nodiscard]] auto GetQueueProperties() const -> cl_command_queue_properties;

  /*!
   * \brief Returns the OpenCL command-queue property array.
   *
   * \return Command-queue property array.
   */
  [[nodiscard]] auto GetQueuePropertiesArray() const
    -> std::vector<cl_queue_properties>;

  /*!
   * \brief Returns the OpenCL device-queue size.
   *
   * \return On-device queue size in bytes, or zero for the host command queue.
   */
  [[nodiscard]] auto GetQueueSize() const -> cl_uint;

private:
  /*! \brief Creates the native OpenCL context. */
  auto CreateContext() -> void;

  /*! \brief Creates the profiling-enabled native command queue. */
  auto CreateCommandQueue() -> void;

  /*!
   * \brief Initializes effective GGEMS SVM support for the associated device.
   */
  auto InitSVMSupport() -> void;

  /*! \brief Initializes device-memory accounting. */
  auto InitVRAMUsage() -> void;

  /*! \brief Recomputes available and peak VRAM accounting fields. */
  auto UpdateVRAMUsage() noexcept -> void;

  /*! \brief Device associated with this context. */
  GGEMSOpenCLDevice const &device_;

  /*! \brief Native OpenCL context. */
  cl::Context context_;

  /*! \brief Native OpenCL command queue. */
  cl::CommandQueue command_queue_;

  /*! \brief Effective GGEMS SVM support. */
  SVMSupport svm_support_{};

  /*! \brief GGEMS SVM allocation accounting. */
  VRAMUsage vram_usage_{};
};
} // namespace ggems::ocl
