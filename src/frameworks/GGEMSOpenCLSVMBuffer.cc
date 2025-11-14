#include "GGEMS/frameworks/GGEMSOpenCLSVMBuffer.hh"
#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/frameworks/GGEMSOpenCLContext.hh"

namespace ggems::ocl {
/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

GGEMSOpenCLSVMBuffer::GGEMSOpenCLSVMBuffer(GGEMSOpenCLContext &context,
                                           std::size_t size_in_bytes,
                                           cl_svm_mem_flags flags,
                                           cl_uint alignment)
    : context_(&context), ptr_{nullptr}, size_in_bytes_{size_in_bytes},
      flags_{flags} {
  GGEMS_CHECK(size_in_bytes_ > 0, "Cannot allocate zero-sized SVM buffer.");

  auto const &svm = context.GetSVMSupport();
  GGEMS_CHECK(svm.HasAny(), "Device does not support any form of SVM.");

  cl_context raw_ctx = context.GetRawContext();

  void *p = clSVMAlloc(raw_ctx, flags_, size_in_bytes_, alignment);
  GGEMS_CHECK(p, "clSVMalloc failed: returned nullptr.");

  ptr_ = p;
  GGEMS_INFOEX("OpenCL", 3, "Allocated SVM Buffer of {} bytes.",
               size_in_bytes_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

GGEMSOpenCLSVMBuffer::GGEMSOpenCLSVMBuffer(
    GGEMSOpenCLSVMBuffer &&other) noexcept {
  *this = std::move(other);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

GGEMSOpenCLSVMBuffer &
GGEMSOpenCLSVMBuffer::operator=(GGEMSOpenCLSVMBuffer &&other) noexcept {
  if (this != &other) {
    Release();
    context_ = other.context_;
    ptr_ = other.ptr_;
    size_in_bytes_ = other.size_in_bytes_;
    flags_ = other.flags_;

    other.context_ = nullptr;
    other.ptr_ = nullptr;
    other.size_in_bytes_ = 0;
    other.flags_ = 0;
  }
  return *this;
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

GGEMSOpenCLSVMBuffer::~GGEMSOpenCLSVMBuffer() { Release(); }

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

void GGEMSOpenCLSVMBuffer::Map(GGEMSOpenCLContext const &ctx,
                               cl_map_flags flags) {
  ctx.EnqueueSVMMap(ptr_, size_in_bytes_, flags);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

void GGEMSOpenCLSVMBuffer::Unmap(GGEMSOpenCLContext const &ctx) {
  ctx.EnqueueSVMUnmap(ptr_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

void GGEMSOpenCLSVMBuffer::Release() noexcept {
  if (context_ && ptr_) {
    cl_context raw_ctx = context_->GetRawContext();
    clSVMFree(raw_ctx, ptr_);
    GGEMS_INFOEX("OpenCL", 3, "Release SVM Buffer of {} bytes.",
                 size_in_bytes_);
  }
  context_ = nullptr;
  ptr_ = nullptr;
  size_in_bytes_ = 0;
  flags_ = 0;
}
} // namespace ggems::ocl
