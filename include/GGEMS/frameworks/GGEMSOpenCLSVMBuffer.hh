#pragma once

#define CL_HPP_TARGET_OPENCL_VERSION 300
#define CL_TARGET_OPENCL_VERSION 300
#define CL_ENABLE_SPIRV_EXTENSIONS

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-conversion"
#pragma clang diagnostic ignored "-Wunused-parameter"
#endif

/// \cond
#include <CL/opencl.hpp>
/// \endcond

namespace ggems::ocl {
class GGEMSOpenCLContext;

class GGEMSOpenCLSVMBuffer {
public:
  GGEMSOpenCLSVMBuffer(GGEMSOpenCLContext &context, std::size_t size_in_bytes,
                       cl_svm_mem_flags flags, cl_uint alignment = 0);

  GGEMSOpenCLSVMBuffer(GGEMSOpenCLSVMBuffer const &) = delete;
  GGEMSOpenCLSVMBuffer &operator=(GGEMSOpenCLSVMBuffer const &) = delete;

  GGEMSOpenCLSVMBuffer(GGEMSOpenCLSVMBuffer &&other) noexcept;
  GGEMSOpenCLSVMBuffer &operator=(GGEMSOpenCLSVMBuffer &&other) noexcept;

  ~GGEMSOpenCLSVMBuffer();

  [[nodiscard]] void *Data() noexcept { return ptr_; }
  [[nodiscard]] void const *Data() const noexcept { return ptr_; }

  [[nodiscard]] std::size_t Size() const noexcept { return size_in_bytes_; }
  [[nodiscard]] cl_svm_mem_flags Flags() const noexcept { return flags_; }

  void Map(GGEMSOpenCLContext const &ctx,
           cl_map_flags flags = CL_MAP_READ | CL_MAP_WRITE);

  void Unmap(GGEMSOpenCLContext const &ctx);

  [[nodiscard]]
  bool NeedsMap(GGEMSOpenCLContext const &ctx) const noexcept;

private:
  void Release() noexcept;

private:
  GGEMSOpenCLContext *context_{nullptr};
  void *ptr_{nullptr};
  std::size_t size_in_bytes_{0};
  cl_svm_mem_flags flags_{0};
};
} // namespace ggems::ocl
