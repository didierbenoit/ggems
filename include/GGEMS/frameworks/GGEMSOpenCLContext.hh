#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "GGEMS/frameworks/GGEMSOpenCLDevice.hh"
#include "GGEMS/frameworks/GGEMSOpenCLSVMMemoryKind.hh"
#include "GGEMS/frameworks/GGEMSOpenCLSVMBuffer.hh"
#include "GGEMS/core/units/GGEMSBytesUnits.hh"

namespace ggems::ocl {
using units::operator""_B;

struct SVMSupport {
  bool coarse_grain_buffer{false};
  bool fine_grain_buffer{false};
  bool fine_grain_system{false};
  bool atomics{false};

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

  [[nodiscard]] auto HasAny() const noexcept -> bool {
    return coarse_grain_buffer || fine_grain_buffer || fine_grain_system;
  }
};

struct VRAMUsage {
  units::Bytes total{0_B};
  units::Bytes allocated{0_B};
  units::Bytes available{0_B};
  units::Bytes peak{0_B};
  std::size_t allocation_count{0};

  [[nodiscard]] auto GetPercent() const noexcept -> std::uint8_t {
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
  GGEMSOpenCLContext(GGEMSOpenCLContext &&) noexcept = default;
  auto operator=(GGEMSOpenCLContext const &) -> GGEMSOpenCLContext & = delete;
  auto operator=(GGEMSOpenCLContext &&) -> GGEMSOpenCLContext & = delete;

  [[nodiscard]] auto GetContextNative() const noexcept -> cl::Context const & {
    return context_;
  }

  [[nodiscard]] auto GetDevice() const noexcept -> GGEMSOpenCLDevice const & {
    return device_;
  }

  [[nodiscard]] auto GetCommandQueueNative() const noexcept
      -> cl::CommandQueue const & {
    return command_queue_;
  }

  [[nodiscard]] auto GetSVMSupport() const noexcept -> SVMSupport const & {
    return svm_support_;
  }

  auto RegisterSVMAllocation(units::Bytes size) noexcept -> void;

  auto RegisterSVMRelease(units::Bytes size) noexcept -> void;

  [[nodiscard]] auto GetVRAMUsage() const noexcept -> VRAMUsage const & {
    return vram_usage_;
  }

  [[nodiscard]] auto GetTotalVRAM() const noexcept -> units::Bytes {
    return vram_usage_.total;
  }

  [[nodiscard]] auto GetAllocatedVRAM() const noexcept -> units::Bytes {
    return vram_usage_.allocated;
  }

  [[nodiscard]] auto GetAvailableVRAM() const noexcept -> units::Bytes {
    return vram_usage_.available;
  }

  [[nodiscard]] auto GetPeakVRAM() const noexcept -> units::Bytes {
    return vram_usage_.peak;
  }

  [[nodiscard]] auto GetAllocationCountVRAM() const noexcept -> std::size_t {
    return vram_usage_.allocation_count;
  }

  [[nodiscard]] auto GetPercentVRAM() const noexcept -> std::uint8_t {
    return vram_usage_.GetPercent();
  }

  [[nodiscard]] auto CreateSVMBuffer(units::Bytes size,
                                     SVMMemoryKind kind = SVMMemoryKind::Auto,
                                     units::Bytes alignment = 0_B)
      -> GGEMSOpenCLSVMBuffer;

  auto EnqueueSVMMap(void *pointer, units::Bytes size,
                     cl_map_flags flags = CL_MAP_READ | CL_MAP_WRITE) const
      -> void;

  auto EnqueueSVMUnmap(void *pointer) const -> void;

  auto SetSVMPointer(cl::Kernel &kernel, cl_uint index,
                     void const *pointer) const -> void;

  [[nodiscard]] auto GetReferenceCount() const -> cl_uint;

  [[nodiscard]] auto GetNumDevices() const -> cl_uint;

  [[nodiscard]] auto GetNativeDevices() const -> std::vector<cl::Device>;

  [[nodiscard]] auto GetProperties() const
      -> std::vector<cl_context_properties>;

  auto PrintContext() const -> void;

  auto PrintCommandQueue() const -> void;

  [[nodiscard]] auto GetQueueContext() const -> cl::Context;

  [[nodiscard]] auto GetQueueDevice() const -> cl::Device;

  [[nodiscard]] auto GetQueueReferenceCount() const -> cl_uint;

  [[nodiscard]] auto GetQueueProperties() const -> cl_command_queue_properties;

  [[nodiscard]] auto GetQueuePropertiesArray() const
      -> std::vector<cl_queue_properties>;

  [[nodiscard]] auto GetQueueSize() const -> cl_uint;

private:
  auto CreateContext() -> void;

  auto CreateGLSharedContext() -> void;

  auto CreateCommandQueue() -> void;

  auto InitSVMSupport() -> void;

  auto InitVRAMUsage() -> void;

  auto UpdateVRAMUsage() noexcept -> void;

  GGEMSOpenCLDevice const &device_;
  cl::Context context_;
  cl::CommandQueue command_queue_;
  SVMSupport svm_support_{};
  VRAMUsage vram_usage_{};
};
} // namespace ggems::ocl
