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
 * \brief Describes shared virtual memory capabilities available to an OpenCL context.
 */
struct SVMSupport {
  /*!
   * \brief Whether coarse-grain buffer SVM is supported.
   */
  bool coarse_grain_buffer{false};
  /*!
   * \brief Whether fine-grain buffer SVM is supported.
   */
  bool fine_grain_buffer{false};
  /*!
   * \brief Whether fine-grain system SVM is supported.
   */
  bool fine_grain_system{false};
  /*!
   * \brief Whether SVM atomic operations are supported.
   */
  bool atomics{false};

  /*!
   * \brief Returns the default supported SVM memory kind.
   *
   * \return Preferred supported SVM memory kind, or SVMMemoryKind::None when SVM is unavailable.
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
 */
struct VRAMUsage {
  /*!
   * \brief Total device global memory capacity.
   */
  units::Bytes total{0_B};
  /*!
   * \brief Currently allocated SVM memory.
   */
  units::Bytes allocated{0_B};
  /*!
   * \brief Currently available memory after GGEMS allocations.
   */
  units::Bytes available{0_B};
  /*!
   * \brief Peak GGEMS SVM allocation observed.
   */
  units::Bytes peak{0_B};
  /*!
   * \brief Number of currently owned SVM allocations.
   */
  std::size_t allocation_count{0};

  /*!
   * \brief Returns the percentage of total VRAM currently allocated by GGEMS.
   *
   * \return Allocated VRAM percentage in the range 0 to 100.
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
 * The wrapper also records SVM capabilities and tracks GGEMS-owned SVM allocation accounting for the device.
 */
class GGEMSOpenCLContext {
public:
  /*!
   * \brief Constructs an OpenCL context for a device.
   *
   * \param[in] device OpenCL device associated with the context.
   */
  explicit GGEMSOpenCLContext(GGEMSOpenCLDevice const &device);

  /*!
   * \brief Destroys the OpenCL context wrapper.
   */
  ~GGEMSOpenCLContext() = default;

  /*!
   * \brief Disables copy construction.
   */
  GGEMSOpenCLContext(GGEMSOpenCLContext const &) = delete;
  /*!
   * \brief Move-constructs an OpenCL context wrapper.
   */
  GGEMSOpenCLContext(GGEMSOpenCLContext &&) noexcept = default;
  /*!
   * \brief Disables copy assignment.
   *
   * \return Reference to this context wrapper.
   */
  auto operator=(GGEMSOpenCLContext const &) -> GGEMSOpenCLContext & = delete;
  /*!
   * \brief Disables move assignment.
   *
   * \return Reference to this context wrapper.
   */
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
   * \brief Returns the detected SVM capability set.
   *
   * \return Detected SVM capabilities.
   */
  [[nodiscard]] auto GetSVMSupport() const noexcept -> SVMSupport const & {
    return svm_support_;
  }

  /*!
   * \brief Registers a newly created SVM allocation in VRAM accounting.
   *
   * \param[in] size Allocated byte count.
   */
  auto RegisterSVMAllocation(units::Bytes size) noexcept -> void;

  /*!
   * \brief Registers an SVM release in VRAM accounting.
   *
   * \param[in] size Released byte count.
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
   * \brief Returns the currently available memory after GGEMS allocations.
   *
   * \return Currently available memory.
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
   * \param[in] size Requested buffer size.
   * \param[in] kind Requested SVM memory kind.
   * \param[in] alignment Requested allocation alignment in bytes.
   * \return Owned SVM buffer.
   */
  [[nodiscard]] auto CreateSVMBuffer(units::Bytes size,
                                     SVMMemoryKind kind = SVMMemoryKind::Auto,
                                     units::Bytes alignment = 0_B)
      -> GGEMSOpenCLSVMBuffer;

  /*!
   * \brief Maps SVM memory for host access and waits for the blocking map to complete.
   *
   * \param[in,out] pointer SVM allocation to map.
   * \param[in] size Mapped byte count.
   * \param[in] flags OpenCL host mapping flags.
   */
  auto EnqueueSVMMap(void *pointer, units::Bytes size,
                     cl_map_flags flags = CL_MAP_READ | CL_MAP_WRITE) const
      -> void;

  /*!
   * \brief Unmaps SVM memory and waits for the unmap event to complete.
   *
   * \param[in,out] pointer SVM allocation to unmap.
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

  /*!
   * \brief Prints native context information.
   */
  auto PrintContext() const -> void;

  /*!
   * \brief Prints native command-queue information.
   */
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
   * \return Command-queue size.
   */
  [[nodiscard]] auto GetQueueSize() const -> cl_uint;

private:
  /*!
   * \brief Creates the native OpenCL context.
   */
  auto CreateContext() -> void;

  /*!
   * \brief Creates the profiling-enabled native command queue.
   */
  auto CreateCommandQueue() -> void;

  /*!
   * \brief Detects SVM support for the associated device.
   */
  auto InitSVMSupport() -> void;

  /*!
   * \brief Initializes device-memory accounting.
   */
  auto InitVRAMUsage() -> void;

  /*!
   * \brief Recomputes available and peak VRAM accounting fields.
   */
  auto UpdateVRAMUsage() noexcept -> void;

  /*!
   * \brief GGEMS device associated with this context.
   */
  GGEMSOpenCLDevice const &device_;
  /*!
   * \brief Native OpenCL context.
   */
  cl::Context context_;
  /*!
   * \brief Native OpenCL command queue.
   */
  cl::CommandQueue command_queue_;
  /*!
   * \brief Detected SVM capabilities.
   */
  SVMSupport svm_support_{};
  /*!
   * \brief GGEMS SVM allocation accounting.
   */
  VRAMUsage vram_usage_{};
};
} // namespace ggems::ocl
