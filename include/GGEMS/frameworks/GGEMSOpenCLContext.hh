#pragma once
// ************************************************************************
// ************************************************************************


#include "GGEMS/frameworks/GGEMSOpenCLDevice.hh"
#include "GGEMS/frameworks/GGEMSSVMMemoryKind.hh"
#include "GGEMS/frameworks/GGEMSOpenCLSVMBuffer.hh"

namespace ggems::ocl {
using units::operator""_B;

struct SVMSupport {
  bool coarse_grain_buffer{false};
  bool fine_grain_buffer{false};
  bool fine_grain_system{false};
  bool atomics{false};

  [[nodiscard]] SVMMemoryKind DefaultKind() const noexcept {
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

  [[nodiscard]] bool Supports(SVMMemoryKind kind) const noexcept {
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

  [[nodiscard]] bool HasAny() const noexcept {
    return coarse_grain_buffer || fine_grain_buffer || fine_grain_system;
  }
};

struct VRAMUsage {
  units::Bytes total{
      0_B};
  units::Bytes allocated{
      0_B};
  units::Bytes available{
      0_B};
  units::Bytes peak{
      0_B};
  std::size_t allocation_count{
      0};

  [[nodiscard]] std::uint8_t GetPercent() const noexcept {
    if (total.value == 0LL) {
      return 0U;
    }

    return static_cast<std::uint8_t>((100ULL * allocated.value) / total.value);
  }
};

class GGEMSOpenCLContext {
public:
  explicit GGEMSOpenCLContext(GGEMSOpenCLDevice const &device);

  ~GGEMSOpenCLContext() = default;

  GGEMSOpenCLContext(GGEMSOpenCLContext const &) = default;

  GGEMSOpenCLContext(GGEMSOpenCLContext &&) = default;

  GGEMSOpenCLContext &operator=(GGEMSOpenCLContext const &) = delete;
  GGEMSOpenCLContext &operator=(GGEMSOpenCLContext &&) = delete;

public:
  [[nodiscard]] cl::Context const &GetContextNative() const noexcept {
    return context_;
  }

  [[nodiscard]] GGEMSOpenCLDevice const &GetDevice() const noexcept {
    return device_;
  }

  [[nodiscard]] cl::CommandQueue const &GetCommandQueueNative() const noexcept {
    return command_queue_;
  }

  [[nodiscard]] SVMSupport const &GetSVMSupport() const noexcept {
    return svm_support_;
  }

  void RegisterSVMAllocation(units::Bytes size) noexcept;

  void RegisterSVMRelease(units::Bytes size) noexcept;

  [[nodiscard]] VRAMUsage const &GetVRAMUsage() const noexcept {
    return vram_usage_;
  }

  [[nodiscard]] units::Bytes GetTotalVRAM() const noexcept {
    return vram_usage_.total;
  }

  [[nodiscard]] units::Bytes GetAllocatedVRAM() const noexcept {
    return vram_usage_.allocated;
  }

  [[nodiscard]] units::Bytes GetAvailableVRAM() const noexcept {
    return vram_usage_.available;
  }

  [[nodiscard]] units::Bytes GetPeakVRAM() const noexcept {
    return vram_usage_.peak;
  }

  [[nodiscard]] std::size_t GetAllocationCountVRAM() const noexcept {
    return vram_usage_.allocation_count;
  }

  [[nodiscard]] std::uint8_t GetPercentVRAM() const noexcept {
    return vram_usage_.GetPercent();
  }

  [[nodiscard]] GGEMSOpenCLSVMBuffer
  CreateSVMBuffer(units::Bytes size, SVMMemoryKind kind = SVMMemoryKind::Auto,
                  units::Bytes alignment = 0_B);

  void EnqueueSVMMap(void *ptr, units::Bytes size,
                     cl_map_flags flags = CL_MAP_READ | CL_MAP_WRITE) const;

  void EnqueueSVMUnmap(void *ptr) const;

  auto SetSVMPointer(cl::Kernel &kernel, cl_uint index, void const *ptr) const
      -> void;

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

  void InitSVMSupport();

  void InitVRAMUsage();

  void UpdateVRAMUsage() noexcept;

private:
  GGEMSOpenCLDevice const &device_;
  cl::Context context_;
  cl::CommandQueue command_queue_;
  SVMSupport svm_support_{};
  VRAMUsage vram_usage_{};
};
} // namespace ggems::ocl
