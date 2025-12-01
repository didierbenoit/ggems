#pragma once

#include "GGEMS/frameworks/GGEMSOpenCLExternal.hh"
#include "GGEMS/frameworks/GGEMSOpenCLDevice.hh"
#include "GGEMS/frameworks/GGEMSOpenCLSVMBuffer.hh"

namespace ggems::ocl {
enum class SVMMemoryKind {
  None,
  Auto,
  CoarseGrainBuffer,
  FineGrainBuffer,
  FineGrainSystem
};

struct SVMSupport {
  bool coarse_grain_buffer_{false};
  bool fine_grain_buffer_{false};
  bool fine_grain_system_{false};
  bool atomics_{false};

  [[nodiscard]] SVMMemoryKind DefaultKind() const noexcept {
    if (fine_grain_system_)
      return SVMMemoryKind::FineGrainSystem;
    if (fine_grain_buffer_)
      return SVMMemoryKind::FineGrainBuffer;
    if (coarse_grain_buffer_)
      return SVMMemoryKind::CoarseGrainBuffer;
    return SVMMemoryKind::None;
  }

  [[nodiscard]] bool HasAny() const noexcept {
    return coarse_grain_buffer_ || fine_grain_buffer_ || fine_grain_system_;
  }
};

class GGEMSOpenCLContext {
public:
  //! Construct a context from a device.
  explicit GGEMSOpenCLContext(GGEMSOpenCLDevice const &device);

  //! Destructor.
  ~GGEMSOpenCLContext();

  //! No copy (contexts are unique).
  GGEMSOpenCLContext(GGEMSOpenCLContext const &) = default;
  GGEMSOpenCLContext &operator=(GGEMSOpenCLContext const &) = delete;

  //! Allow move.
  GGEMSOpenCLContext(GGEMSOpenCLContext &&) noexcept = default;
  GGEMSOpenCLContext &operator=(GGEMSOpenCLContext &&) noexcept = delete;

public:
  [[nodiscard]] cl::Context const &GetContextNative() const noexcept {
    return context_;
  }
  [[nodiscard]] GGEMSOpenCLDevice const &GetDevice() const noexcept {
    return device_;
  }

  [[nodiscard]] cl::CommandQueue const &GetCommandQueueNative() noexcept {
    return command_queue_;
  }

  // ----- SVM buffers
  [[nodiscard]] SVMSupport const &GetSVMSupport() const noexcept {
    return svm_support_;
  }

  void InitSVMSupport();

  [[nodiscard]] GGEMSOpenCLSVMBuffer
  CreateSVMBuffer(units::Bytes size, SVMMemoryKind kind = SVMMemoryKind::Auto,
                  cl_uint alignment = 0);

  void EnqueueSVMMap(void *ptr, units::Bytes size,
                     cl_map_flags flags = CL_MAP_READ | CL_MAP_WRITE) const;

  void EnqueueSVMUnmap(void *ptr) const;

  void SetSVMPointer(cl::Kernel &kernel, cl_uint index, void *ptr) const;

  // ----- Context -----------------------------------
  [[nodiscard]] cl_uint GetReferenceCount() const;
  [[nodiscard]] cl_uint GetNumDevices() const;
  [[nodiscard]] std::vector<cl::Device> GetNativeDevices() const;
  [[nodiscard]] std::vector<cl_context_properties> GetProperties() const;
  void PrintContext() const;
  void PrintCommandQueue() const;

  // ----- Command Queue -----------------------------
  [[nodiscard]] cl::Context GetQueueContext() const;
  [[nodiscard]] cl::Device GetQueueDevice() const;
  [[nodiscard]] cl_uint GetQueueReferenceCount() const;
  [[nodiscard]] cl_command_queue_properties GetQueueProperties() const;
  [[nodiscard]] std::vector<cl_queue_properties>
  GetQueuePropertiesArray() const;
  [[nodiscard]] cl_uint GetQueueSize() const;

private:
  void CreateContext();
  void CreateGLSharedContext();
  void CreateCommandQueue();

private:
  GGEMSOpenCLDevice const &device_;
  cl::Context context_;
  cl::CommandQueue command_queue_;
  SVMSupport svm_support_{};
};
} // namespace ggems::ocl
