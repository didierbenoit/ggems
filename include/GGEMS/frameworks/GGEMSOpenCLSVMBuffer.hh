#pragma once
// ************************************************************************
// ************************************************************************


#include "GGEMS/frameworks/GGEMSOpenCLExternal.hh"
#include "GGEMS/frameworks/GGEMSSVMMemoryKind.hh"
#include "GGEMS/core/units/GGEMSBytesUnits.hh"

namespace ggems::ocl {
class GGEMSOpenCLContext;
using units::operator""_B;

class GGEMSOpenCLSVMBuffer {
public:
  GGEMSOpenCLSVMBuffer(GGEMSOpenCLContext &context, units::Bytes size,
                       cl_svm_mem_flags flags, SVMMemoryKind kind,
                       units::Bytes alignment = 0_B);

  ~GGEMSOpenCLSVMBuffer();

  GGEMSOpenCLSVMBuffer(GGEMSOpenCLSVMBuffer &&other) noexcept;

  GGEMSOpenCLSVMBuffer &operator=(GGEMSOpenCLSVMBuffer &&other) noexcept;

  GGEMSOpenCLSVMBuffer(GGEMSOpenCLSVMBuffer const &) = delete;
  GGEMSOpenCLSVMBuffer &operator=(GGEMSOpenCLSVMBuffer const &) = delete;

public:
  [[nodiscard]] void *GetData() noexcept { return ptr_; }

  [[nodiscard]] void const *GetData() const noexcept { return ptr_; }

  [[nodiscard]] units::Bytes GetSize() const noexcept { return size_; }

  [[nodiscard]] cl_svm_mem_flags GetFlags() const noexcept { return flags_; }

  [[nodiscard]] SVMMemoryKind GetKind() const noexcept { return kind_; }

  void Map(cl_map_flags flags = CL_MAP_READ | CL_MAP_WRITE);

  void Unmap();

private:
  void Release() noexcept;

private:
  GGEMSOpenCLContext *context_{nullptr};
  void *ptr_{nullptr};
  units::Bytes size_{0ULL};
  cl_svm_mem_flags flags_{
      0};
  SVMMemoryKind kind_{SVMMemoryKind::None};
};
} // namespace ggems::ocl
