#include "GGEMS/frameworks/GGEMSOpenCLContext.hh"
#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/frameworks/GGEMSOpenCLDevice.hh"
#include "GGEMS/frameworks/GGEMSOpenCLUtils.hh"

namespace ggems::ocl {
using core::GGEMSFatal;
using core::Throw;

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

GGEMSOpenCLContext::GGEMSOpenCLContext(GGEMSOpenCLDevice const &device)
    : device_{device} {
  GGEMS_INFOEX("OpenCL", 2, "Allocating GGEMSOpenCLContext: {}",
               device.GetName());

  CreateContext();
  CreateCommandQueue();
  InitSVMSupport();

  auto const &exts = device_.GetDeviceExtensions();
  supports_il_program_ = HasExtension(exts, "cl_khr_il_program");

  GGEMS_INFOEX("OpenCL", 2, "GGEMSOpenCLContext allocated.");
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

GGEMSOpenCLContext::~GGEMSOpenCLContext() {
  GGEMS_INFOEX("OpenCL", 2, "Destroying OpenCL context for device: {}",
               device_.GetName());
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void GGEMSOpenCLContext::CreateContext() {
  GGEMS_INFO("OpenCL", "Creating compute OpenCL context for device: {}",
             device_.GetName());

  cl_int err{CL_SUCCESS};

  // --- Standard compute-only context (no OpenGL interop) --------------------
  cl_context_properties props[] = {
      CL_CONTEXT_PLATFORM,
      reinterpret_cast<cl_context_properties>(device_.GetPlatformID()), 0};

  context_ = cl::Context({device_.GetNative()}, // devices
                         props,                 // context properties
                         nullptr,               // notification callback
                         nullptr,               // user data
                         &err);

  CheckCLError(err, "Failed to create OpenCL context");

  GGEMS_INFOEX("OpenCL", 2, "OpenCL context created.");
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void GGEMSOpenCLContext::CreateCommandQueue() {
  GGEMS_INFO("OpenCL", "Creating command queue...");

  cl_int err{CL_SUCCESS};

  cl_command_queue_properties props = 0;
  props |= CL_QUEUE_PROFILING_ENABLE; // needed for profiling

  command_queue_ = cl::CommandQueue(context_, device_.GetNative(), props, &err);

  CheckCLError(err, "Failed to create command queue.");

  GGEMS_INFOEX("OpenCL", 2, "Command queue created (profiling enabled).");
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void GGEMSOpenCLContext::InitSVMSupport() {
  auto const caps = device_.GetSVMCapabilities();

  if (caps & CL_DEVICE_SVM_COARSE_GRAIN_BUFFER)
    svm_support_.coarse_grain_buffer_ = true;
  if (caps & CL_DEVICE_SVM_FINE_GRAIN_BUFFER)
    svm_support_.fine_grain_buffer_ = true;
  if (caps & CL_DEVICE_SVM_FINE_GRAIN_SYSTEM)
    svm_support_.fine_grain_system_ = true;
  if (caps & CL_DEVICE_SVM_ATOMICS)
    svm_support_.atomics_ = true;
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

GGEMSOpenCLSVMBuffer
GGEMSOpenCLContext::CreateSVMBuffer(std::size_t size_in_bytes,
                                    SVMMemoryKind kind, cl_uint alignment) {
  auto const &svm = svm_support_;

  GGEMS_CHECK(svm.HasAny(), "This context/device does not support SVM.");

  SVMMemoryKind selected = kind;

  GGEMS_CHECK(selected != SVMMemoryKind::None,
              "Invalid SVMMemoryKind::None for allocation.");

  if (selected == SVMMemoryKind::Auto) {
    selected = svm.DefaultKind();
  }

  cl_svm_mem_flags flags = 0;

  switch (selected) {
  case SVMMemoryKind::CoarseGrainBuffer:
    flags = CL_MEM_READ_WRITE; // + SVM coarse-grain implicite
    break;
  case SVMMemoryKind::FineGrainBuffer:
    // fine-grain buffer : SVM + fine-grain buffer = implicite via caps
    flags = CL_MEM_READ_WRITE | CL_MEM_SVM_FINE_GRAIN_BUFFER;
    break;
  case SVMMemoryKind::FineGrainSystem:
    flags = CL_MEM_READ_WRITE;
    break;
  default:
    Throw<GGEMSFatal>("Unsupported SVMMemoryKind in CreateSVMBuffer.");
  }

  if (selected == SVMMemoryKind::FineGrainBuffer && svm.atomics_) {
    flags |= CL_MEM_SVM_ATOMICS;
  }

  cl_uint real_alignment = alignment ? alignment : sizeof(void *);

  return GGEMSOpenCLSVMBuffer{*this, size_in_bytes, flags, real_alignment};
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void GGEMSOpenCLContext::EnqueueSVMMap(void *ptr, std::size_t size,
                                       cl_map_flags flags) const {
  cl_int err = clEnqueueSVMMap(command_queue_(), // raw command queue
                               CL_TRUE,          // blocking map pour simplifier
                               flags, ptr, size, 0, nullptr, nullptr);

  GGEMS_CHECK(err == CL_SUCCESS,
              std::format("SVMMap failed: {}", GetLongErrorString(err)));
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void GGEMSOpenCLContext::EnqueueSVMUnmap(void *ptr) const {
  cl_int err = clEnqueueSVMUnmap(command_queue_(), ptr, 0, nullptr, nullptr);

  GGEMS_CHECK(err == CL_SUCCESS,
              std::format("SVMUnmap failed: {}", GetLongErrorString(err)));
}

/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */
/* ------------------------------------------------------------------------- */

void GGEMSOpenCLContext::SetSVMPointer(cl::Kernel &kernel, cl_uint index,
                                       void *ptr) const {
  cl_int err = clSetKernelArgSVMPointer(kernel(), index, ptr);

  GGEMS_CHECK(err == CL_SUCCESS,
              std::format("SetSVMPointer failed at arg {}: {}", index,
                          GetLongErrorString(err)));
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
