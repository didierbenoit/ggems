#pragma once

#define CL_HPP_TARGET_OPENCL_VERSION 300
#define CL_TARGET_OPENCL_VERSION 300

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma clang diagnostic ignored "-Wunused-parameter"
#endif

/// \cond
#include <CL/opencl.hpp>
/// \endcond

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#include "GGEMS/core/units/GGEMSUnits.hh"

namespace ggems::ocl {
class GGEMSOpenCLContext;

class GGEMSOpenCLSVMBuffer {
public:
  GGEMSOpenCLSVMBuffer(GGEMSOpenCLContext &context, units::Bytes size,
                       cl_svm_mem_flags flags, cl_uint alignment = 0);

  GGEMSOpenCLSVMBuffer(GGEMSOpenCLSVMBuffer const &) = delete;
  GGEMSOpenCLSVMBuffer &operator=(GGEMSOpenCLSVMBuffer const &) = delete;

  GGEMSOpenCLSVMBuffer(GGEMSOpenCLSVMBuffer &&other) noexcept;
  GGEMSOpenCLSVMBuffer &operator=(GGEMSOpenCLSVMBuffer &&other) noexcept;

  ~GGEMSOpenCLSVMBuffer();

  [[nodiscard]] void *Data() noexcept { return ptr_; }
  [[nodiscard]] void const *Data() const noexcept { return ptr_; }

  [[nodiscard]] units::Bytes Size() const noexcept { return size_; }
  [[nodiscard]] cl_svm_mem_flags Flags() const noexcept { return flags_; }

  void Map(cl_map_flags flags = CL_MAP_READ | CL_MAP_WRITE);

  void Unmap();

private:
  void Release() noexcept;

private:
  GGEMSOpenCLContext *context_{nullptr};
  void *ptr_{nullptr};
  units::Bytes size_{0ULL};
  cl_svm_mem_flags flags_{0};
};
} // namespace ggems::ocl
