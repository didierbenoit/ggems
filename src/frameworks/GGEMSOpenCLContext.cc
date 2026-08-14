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
 * \brief Implements the GGEMS OpenCL context wrapper.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <format>
#include <array>
#include <vector>
#include <cstdint>
#include <cstddef>
/// \endcond

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSLogMacros.hh"
#include "GGEMS/core/units/GGEMSQuantity.hh"
#include "GGEMS/core/units/GGEMSBytesUnits.hh"
#include "GGEMS/frameworks/GGEMSOpenCLUtils.hh"
#include "GGEMS/frameworks/GGEMSOpenCLContext.hh"
#include "GGEMS/frameworks/GGEMSOpenCLDevice.hh"
#include "GGEMS/frameworks/GGEMSOpenCLSVMMemoryKind.hh"
#include "GGEMS/frameworks/GGEMSOpenCLSVMBuffer.hh"

using namespace ggems::units;

namespace ggems::ocl {

// =============================================================================
// =============================================================================

GGEMSOpenCLContext::GGEMSOpenCLContext(GGEMSOpenCLDevice const &device)
    : device_{device} {
  GGEMS_INFOEX("OpenCL", 3, "Creating OpenCL context for device '{}'.",
               device.GetName());

  CreateContext();
  CreateCommandQueue();
  InitSVMSupport();
  InitVRAMUsage();

  GGEMS_INFOEX("OpenCL", 2, "OpenCL context ready for device '{}'.",
               device_.GetName());
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLContext::CreateContext() -> void {
  GGEMS_INFOEX("OpenCL", 3, "Creating compute OpenCL context");

  cl_int error{CL_SUCCESS};

  std::array<cl_context_properties, 3> props{
      CL_CONTEXT_PLATFORM,
      reinterpret_cast<cl_context_properties>(device_.GetPlatformID()), 0};

  context_ = cl::Context({device_.GetDeviceNative()}, props.data(), nullptr,
                         nullptr, &error);

  CheckCLError(error, "Failed to create OpenCL context");

  GGEMS_INFOEX("OpenCL", 3, "Compute OpenCL context created.");
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLContext::CreateCommandQueue() -> void {
  GGEMS_INFOEX("OpenCL", 3, "Creating OpenCL command queue.");

  cl_int error{CL_SUCCESS};

  cl_command_queue_properties props{0};
  props |= CL_QUEUE_PROFILING_ENABLE;

  command_queue_ =
      cl::CommandQueue(context_, device_.GetDeviceNative(), props, &error);

  CheckCLError(error, "Failed to create command queue.");

  GGEMS_INFOEX("OpenCL", 3,
               "OpenCL command queue created with profiling enabled.");
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLContext::InitSVMSupport() -> void {
  auto const capabilities = device_.GetSVMCapabilities();

  if ((capabilities & CL_DEVICE_SVM_COARSE_GRAIN_BUFFER) != 0) {
    svm_support_.coarse_grain_buffer = true;
  }
  if ((capabilities & CL_DEVICE_SVM_FINE_GRAIN_BUFFER) != 0) {
    svm_support_.fine_grain_buffer = true;
  }
  if ((capabilities & CL_DEVICE_SVM_FINE_GRAIN_SYSTEM) != 0) {
    svm_support_.fine_grain_system = true;
  }
  if ((capabilities & CL_DEVICE_SVM_ATOMICS) != 0) {
    svm_support_.atomics = true;
  }

  GGEMS_INFOEX(
      "OpenCL", 2,
      "SVM support for '{}': coarse={}, fine-buffer={}, fine-system={}, "
      "atomics={}, auto={}",
      device_.GetName(), svm_support_.coarse_grain_buffer,
      svm_support_.fine_grain_buffer, svm_support_.fine_grain_system,
      svm_support_.atomics, ToString(svm_support_.DefaultKind()));
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLContext::InitVRAMUsage() -> void {
  vram_usage_.total =
      units::Bytes{static_cast<std::uint64_t>(device_.GetGlobalMemSize())};

  UpdateVRAMUsage();
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLContext::UpdateVRAMUsage() noexcept -> void {
  if (vram_usage_.allocated.value <= vram_usage_.total.value) {
    vram_usage_.available =
        units::Bytes{vram_usage_.total.value - vram_usage_.allocated.value};
  } else {
    vram_usage_.available = 0_B;
  }

  if (vram_usage_.allocated.value > vram_usage_.peak.value) {
    vram_usage_.peak = vram_usage_.allocated;
  }
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLContext::CreateSVMBuffer(Bytes size, SVMMemoryKind kind,
                                         Bytes alignment)
    -> GGEMSOpenCLSVMBuffer {
  auto const &svm_support = svm_support_;

  if (!svm_support.HasAny()) {
    throw ggems::core::GGEMSFatal("This context/device does not support SVM.");
  }

  if (kind == SVMMemoryKind::None) {
    throw ggems::core::GGEMSFatal(
        "Invalid SVMMemoryKind::None for allocation.");
  }

  SVMMemoryKind const selected =
      kind == SVMMemoryKind::Auto ? svm_support.DefaultKind() : kind;

  if (selected == SVMMemoryKind::None) {
    throw ggems::core::GGEMSFatal("No supported SVM memory kind is available.");
  }

  if (!svm_support.Supports(selected)) {
    throw ggems::core::GGEMSFatal(std::format(
        "Requested SVM memory kind '{}' is not supported by device '{}'.",
        ToString(selected), device_.GetName()));
  }

  cl_svm_mem_flags flags = 0;

  switch (selected) {
  case SVMMemoryKind::CoarseGrainBuffer:
    flags = CL_MEM_READ_WRITE;
    break;
  case SVMMemoryKind::FineGrainBuffer:
    flags = CL_MEM_READ_WRITE | CL_MEM_SVM_FINE_GRAIN_BUFFER;
    break;
  case SVMMemoryKind::FineGrainBufferAtomics:
    flags =
        CL_MEM_READ_WRITE | CL_MEM_SVM_FINE_GRAIN_BUFFER | CL_MEM_SVM_ATOMICS;
    break;
  case SVMMemoryKind::FineGrainSystem:
    flags = CL_MEM_READ_WRITE;
    break;
  default:
    throw core::GGEMSFatal("Unsupported SVMMemoryKind in CreateSVMBuffer.");
  }

  GGEMS_INFOEX("OpenCL", 2,
               "Creating SVM buffer: size={}, requested={}, selected={}, "
               "flags=0x{:X}, alignment={}",
               HumanReadable(size), ToString(kind), ToString(selected),
               static_cast<std::uint64_t>(flags), HumanReadable(alignment));

  constexpr auto k_large_svm_warning_threshold{64_MiB};

  if (selected == SVMMemoryKind::FineGrainBuffer &&
      size >= k_large_svm_warning_threshold) {
    GGEMS_WARN("OpenCL",
               "Fine-grain SVM buffer selected for a large allocation ({}). "
               "This may severely reduce GPU throughput on some OpenCL "
               "drivers. Prefer CoarseGrainBuffer for large performance "
               "buffers when possible.",
               HumanReadable(size));
  }

  if (selected == SVMMemoryKind::FineGrainBufferAtomics &&
      size >= k_large_svm_warning_threshold) {
    GGEMS_WARN("OpenCL",
               "Fine-grain atomic SVM buffer selected for a large allocation "
               "({}). This is a synchronization-heavy memory mode and should "
               "not be used for large performance buffers unless atomics are "
               "strictly required.",
               HumanReadable(size));
  }

  return GGEMSOpenCLSVMBuffer{*this, size, flags, selected, alignment};
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLContext::EnqueueSVMMap(void *pointer, Bytes size,
                                       cl_map_flags flags) const -> void {
  cl_int error =
      clEnqueueSVMMap(command_queue_(), // raw command queue
                      CL_TRUE,          // blocking map for simplicity
                      flags, pointer, static_cast<std::size_t>(size.value), 0,
                      nullptr, nullptr);

  if (error != CL_SUCCESS) {
    throw ggems::core::GGEMSFatal(
        std::format("SVMMap failed: {}", GetLongErrorString(error)));
  }
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLContext::EnqueueSVMUnmap(void *pointer) const -> void {
  cl_event unmap_event{nullptr};

  cl_int error =
      clEnqueueSVMUnmap(command_queue_(), pointer, 0, nullptr, &unmap_event);

  if (error != CL_SUCCESS) {
    throw ggems::core::GGEMSFatal(
        std::format("SVMUnmap failed: {}", GetLongErrorString(error)));
  }

  error = clWaitForEvents(1, &unmap_event);

  clReleaseEvent(unmap_event);
  if (error != CL_SUCCESS) {
    throw ggems::core::GGEMSFatal(
        std::format("SVMUnmap wait failed: {}", GetLongErrorString(error)));
  }
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLContext::RegisterSVMAllocation(units::Bytes size) noexcept
    -> void {
  vram_usage_.allocated = vram_usage_.allocated + size;
  ++vram_usage_.allocation_count;

  UpdateVRAMUsage();
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLContext::RegisterSVMRelease(units::Bytes size) noexcept
    -> void {
  if (size.value >= vram_usage_.allocated.value) {
    vram_usage_.allocated = 0_B;
  } else {
    vram_usage_.allocated = vram_usage_.allocated - size;
  }

  if (vram_usage_.allocation_count > 0) {
    --vram_usage_.allocation_count;
  }

  UpdateVRAMUsage();
}

// -----------------------------------------------------------------------------

[[nodiscard]] auto GGEMSOpenCLContext::GetReferenceCount() const -> cl_uint {
  return GetInfo<CL_CONTEXT_REFERENCE_COUNT>(context_);
}

// -----------------------------------------------------------------------------

[[nodiscard]] auto GGEMSOpenCLContext::GetNumDevices() const -> cl_uint {
  return GetInfo<CL_CONTEXT_NUM_DEVICES>(context_);
}

// -----------------------------------------------------------------------------

[[nodiscard]] auto GGEMSOpenCLContext::GetNativeDevices() const
    -> std::vector<cl::Device> {
  return GetInfo<CL_CONTEXT_DEVICES>(context_);
}

// -----------------------------------------------------------------------------

[[nodiscard]] auto GGEMSOpenCLContext::GetProperties() const
    -> std::vector<cl_context_properties> {
  return GetInfo<CL_CONTEXT_PROPERTIES>(context_);
}
// -----------------------------------------------------------------------------

[[nodiscard]] auto GGEMSOpenCLContext::GetQueueContext() const -> cl::Context {
  return GetInfo<CL_QUEUE_CONTEXT>(command_queue_);
}

// -----------------------------------------------------------------------------

[[nodiscard]] auto GGEMSOpenCLContext::GetQueueDevice() const -> cl::Device {
  return GetInfo<CL_QUEUE_DEVICE>(command_queue_);
}

// -----------------------------------------------------------------------------

[[nodiscard]] auto GGEMSOpenCLContext::GetQueueReferenceCount() const
    -> cl_uint {
  return GetInfo<CL_QUEUE_REFERENCE_COUNT>(command_queue_);
}

// -----------------------------------------------------------------------------

[[nodiscard]] auto GGEMSOpenCLContext::GetQueueProperties() const
    -> cl_command_queue_properties {
  return GetInfo<CL_QUEUE_PROPERTIES>(command_queue_);
}

// -----------------------------------------------------------------------------

[[nodiscard]] auto GGEMSOpenCLContext::GetQueuePropertiesArray() const
    -> std::vector<cl_queue_properties> {
  return GetInfo<CL_QUEUE_PROPERTIES_ARRAY>(command_queue_);
}

// -----------------------------------------------------------------------------

[[nodiscard]] auto GGEMSOpenCLContext::GetQueueSize() const -> cl_uint {
  auto const properties = GetInfo<CL_QUEUE_PROPERTIES>(command_queue_);
  if ((properties & CL_QUEUE_ON_DEVICE) != 0) {
    return GetInfo<CL_QUEUE_SIZE>(command_queue_);
  }

  return 0U;
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLContext::PrintContext() const -> void {
  PrintInfo<CL_CONTEXT_NUM_DEVICES>(context_);
  PrintInfo<CL_CONTEXT_DEVICES>(context_);
  PrintInfo<CL_CONTEXT_PROPERTIES>(context_);
  PrintInfo<CL_CONTEXT_REFERENCE_COUNT>(context_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLContext::PrintCommandQueue() const -> void {
  PrintInfo<CL_QUEUE_DEVICE>(command_queue_);
  PrintInfo<CL_QUEUE_REFERENCE_COUNT>(command_queue_);
  PrintInfo<CL_QUEUE_PROPERTIES>(command_queue_);
  PrintInfo<CL_QUEUE_PROPERTIES_ARRAY>(command_queue_);
}
} // namespace ggems::ocl
