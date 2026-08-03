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
 * \file GGEMSOpenCLContext.cc
 * \brief Declaration of GGEMSOpenCLContext providing creation and management
 *        of OpenCL contexts, command queues, SVM capabilities, and future
 *        interoperability features (OpenCL ↔ Vulkan).
 *
 * This class encapsulates all OpenCL context-level operations used by GGEMS.
 * It creates and manages the native cl::Context, command queue, and SVM
 * capability detection. The context serves as the central point of memory
 * allocation, queue submission, kernel setup, and (in future versions)
 * interoperability with external APIs such as Vulkan.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-12-08
 * \version 2.0
 * \copyright
 * GNU General Public License v3.0
 */

#include "GGEMS/frameworks/GGEMSOpenCLContext.hh"
#include "GGEMS/core/units/GGEMSBytesUnits.hh"

using namespace ggems::units;

namespace ggems::ocl {

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

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

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void GGEMSOpenCLContext::CreateContext() {
  GGEMS_INFOEX("OpenCL", 3, "Creating compute OpenCL context");

  cl_int err{CL_SUCCESS};

  // --- Standard compute-only context (no OpenGL interop) --------------------
  cl_context_properties props[] = {
      CL_CONTEXT_PLATFORM,
      reinterpret_cast<cl_context_properties>(device_.GetPlatformID()), 0};

  context_ = cl::Context({device_.GetDeviceNative()}, // devices
                         props,                       // context properties
                         nullptr,                     // notification callback
                         nullptr,                     // user data
                         &err);

  GGEMS_OCL_CHECK(err, "Failed to create OpenCL context");

  GGEMS_INFOEX("OpenCL", 3, "Compute OpenCL context created.");
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void GGEMSOpenCLContext::CreateCommandQueue() {
  GGEMS_INFOEX("OpenCL", 3, "Creating OpenCL command queue.");

  cl_int err{CL_SUCCESS};

  cl_command_queue_properties props = 0;
  props |= CL_QUEUE_PROFILING_ENABLE; // needed for profiling

  command_queue_ =
      cl::CommandQueue(context_, device_.GetDeviceNative(), props, &err);

  GGEMS_OCL_CHECK(err, "Failed to create command queue.");

  GGEMS_INFOEX("OpenCL", 3,
               "OpenCL command queue created with profiling enabled.");
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void GGEMSOpenCLContext::InitSVMSupport() {
  auto const caps = device_.GetSVMCapabilities();

  if (caps & CL_DEVICE_SVM_COARSE_GRAIN_BUFFER) {
    svm_support_.coarse_grain_buffer = true;
  }
  if (caps & CL_DEVICE_SVM_FINE_GRAIN_BUFFER) {
    svm_support_.fine_grain_buffer = true;
  }
  if (caps & CL_DEVICE_SVM_FINE_GRAIN_SYSTEM) {
    svm_support_.fine_grain_system = true;
  }
  if (caps & CL_DEVICE_SVM_ATOMICS) {
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

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void GGEMSOpenCLContext::InitVRAMUsage() {
  vram_usage_.total =
      units::Bytes{static_cast<std::uint64_t>(device_.GetGlobalMemSize())};

  UpdateVRAMUsage();
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void GGEMSOpenCLContext::UpdateVRAMUsage() noexcept {
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

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

GGEMSOpenCLSVMBuffer GGEMSOpenCLContext::CreateSVMBuffer(Bytes size,
                                                         SVMMemoryKind kind,
                                                         Bytes alignment) {
  auto const &svm = svm_support_;

  GGEMS_CHECK_FATAL(svm.HasAny(), "This context/device does not support SVM.");

  GGEMS_CHECK_FATAL(kind != SVMMemoryKind::None,
                    "Invalid SVMMemoryKind::None for allocation.");

  SVMMemoryKind const selected =
      kind == SVMMemoryKind::Auto ? svm.DefaultKind() : kind;

  GGEMS_CHECK_FATAL(selected != SVMMemoryKind::None,
                    "No supported SVM memory kind is available.");

  GGEMS_CHECK_FATAL(
      svm.Supports(selected),
      std::format("Requested SVM memory kind '{}' is not supported by "
                  "device '{}'.",
                  ToString(selected), device_.GetName()));

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
    core::Throw<core::GGEMSFatal>(
        "Unsupported SVMMemoryKind in CreateSVMBuffer.");
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
               "({}). This is a synchronisation-heavy memory mode and should "
               "not be used for large performance buffers unless atomics are "
               "strictly required.",
               HumanReadable(size));
  }

  return GGEMSOpenCLSVMBuffer{*this, size, flags, selected, alignment};
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void GGEMSOpenCLContext::EnqueueSVMMap(void *ptr, Bytes size,
                                       cl_map_flags flags) const {
  cl_int err = clEnqueueSVMMap(command_queue_(), // raw command queue
                               CL_TRUE,          // blocking map pour simplifier
                               flags, ptr, static_cast<std::size_t>(size.value),
                               0, nullptr, nullptr);

  GGEMS_CHECK_FATAL(err == CL_SUCCESS,
                    std::format("SVMMap failed: {}", GetLongErrorString(err)));
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void GGEMSOpenCLContext::EnqueueSVMUnmap(void *ptr) const {
  cl_event unmap_event{nullptr};

  cl_int err =
      clEnqueueSVMUnmap(command_queue_(), ptr, 0, nullptr, &unmap_event);

  GGEMS_CHECK_FATAL(err == CL_SUCCESS, std::format("SVMUnmap failed: {}",
                                                   GetLongErrorString(err)));

  err = clWaitForEvents(1, &unmap_event);

  clReleaseEvent(unmap_event);
  GGEMS_CHECK_FATAL(err == CL_SUCCESS, std::format("SVMUnmap wait failed: {}",
                                                   GetLongErrorString(err)));
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void GGEMSOpenCLContext::SetSVMPointer(cl::Kernel &kernel, cl_uint index,
                                       void *ptr) const {
  cl_int err = clSetKernelArgSVMPointer(kernel(), index, ptr);

  GGEMS_CHECK_FATAL(err == CL_SUCCESS,
                    std::format("SetSVMPointer failed at arg {}: {}", index,
                                GetLongErrorString(err)));
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void GGEMSOpenCLContext::RegisterSVMAllocation(units::Bytes size) noexcept {
  vram_usage_.allocated = vram_usage_.allocated + size;
  ++vram_usage_.allocation_count;

  UpdateVRAMUsage();
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void GGEMSOpenCLContext::RegisterSVMRelease(units::Bytes size) noexcept {
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

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

[[nodiscard]] cl_uint GGEMSOpenCLContext::GetReferenceCount() const {
  return GetInfo<CL_CONTEXT_REFERENCE_COUNT>(context_);
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

[[nodiscard]] cl_uint GGEMSOpenCLContext::GetNumDevices() const {
  return GetInfo<CL_CONTEXT_NUM_DEVICES>(context_);
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

[[nodiscard]] std::vector<cl::Device>
GGEMSOpenCLContext::GetNativeDevices() const {
  return GetInfo<CL_CONTEXT_DEVICES>(context_);
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

[[nodiscard]] std::vector<cl_context_properties>
GGEMSOpenCLContext::GetProperties() const {
  return GetInfo<CL_CONTEXT_PROPERTIES>(context_);
}
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

[[nodiscard]] cl::Context GGEMSOpenCLContext::GetQueueContext() const {
  return GetInfo<CL_QUEUE_CONTEXT>(command_queue_);
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

[[nodiscard]] cl::Device GGEMSOpenCLContext::GetQueueDevice() const {
  return GetInfo<CL_QUEUE_DEVICE>(command_queue_);
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

[[nodiscard]] cl_uint GGEMSOpenCLContext::GetQueueReferenceCount() const {
  return GetInfo<CL_QUEUE_REFERENCE_COUNT>(command_queue_);
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

[[nodiscard]] cl_command_queue_properties
GGEMSOpenCLContext::GetQueueProperties() const {
  return GetInfo<CL_QUEUE_PROPERTIES>(command_queue_);
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

[[nodiscard]] std::vector<cl_queue_properties>
GGEMSOpenCLContext::GetQueuePropertiesArray() const {
  return GetInfo<CL_QUEUE_PROPERTIES_ARRAY>(command_queue_);
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

[[nodiscard]] cl_uint GGEMSOpenCLContext::GetQueueSize() const {
  auto props = GetInfo<CL_QUEUE_PROPERTIES>(command_queue_);
  if (props & CL_QUEUE_ON_DEVICE) {
    return GetInfo<CL_QUEUE_SIZE>(command_queue_);
  } else {
    return 0;
  }
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void GGEMSOpenCLContext::PrintContext() const {
  PrintInfo<CL_CONTEXT_NUM_DEVICES>(context_);
  PrintInfo<CL_CONTEXT_DEVICES>(context_);
  PrintInfo<CL_CONTEXT_PROPERTIES>(context_);
  PrintInfo<CL_CONTEXT_REFERENCE_COUNT>(context_);
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void GGEMSOpenCLContext::PrintCommandQueue() const {
  PrintInfo<CL_QUEUE_DEVICE>(command_queue_);
  PrintInfo<CL_QUEUE_REFERENCE_COUNT>(command_queue_);
  PrintInfo<CL_QUEUE_PROPERTIES>(command_queue_);
  PrintInfo<CL_QUEUE_PROPERTIES_ARRAY>(command_queue_);
}
} // namespace ggems::ocl
