#include "GGEMS/frameworks/GGEMSOpenCLSVMBuffer.hh"
#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/core/units/GGEMSBytesUnits.hh"
#include "GGEMS/frameworks/GGEMSOpenCLContext.hh"

using namespace ggems::units;

namespace ggems::ocl {
/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

GGEMSOpenCLSVMBuffer::GGEMSOpenCLSVMBuffer(GGEMSOpenCLContext &context,
                                           Bytes size, cl_svm_mem_flags flags,
                                           cl_uint alignment)
    : context_(&context), ptr_{nullptr}, size_{size}, flags_{flags} {
  GGEMS_CHECK(size_.value > 0, "Cannot allocate zero-sized SVM buffer.");

  auto const &svm = context.GetSVMSupport();
  GGEMS_CHECK(svm.HasAny(), "Device does not support any form of SVM.");

  auto &ctx = context.GetContextNative();

  void *p = clSVMAlloc(ctx(), flags_, ToSizeT(size), alignment);
  GGEMS_CHECK(p, "clSVMalloc failed: returned nullptr.");

  ptr_ = p;
  GGEMS_INFOEX("OpenCL", 3, "Allocated SVM Buffer of {}.",
               HumanReadable(size_));
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
    size_ = other.size_;
    flags_ = other.flags_;

    other.context_ = nullptr;
    other.ptr_ = nullptr;
    other.size_ = 0_B;
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

void GGEMSOpenCLSVMBuffer::Map(cl_map_flags flags) {
  auto const &svm = context_->GetSVMSupport();

  if (svm.fine_grain_system_) {
    return;
  }

  // Pour coarse-grain ou fine-grain buffer :
  context_->EnqueueSVMMap(ptr_, size_, flags);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

void GGEMSOpenCLSVMBuffer::Unmap() {
  auto const &svm = context_->GetSVMSupport();

  if (svm.fine_grain_system_) {
    return;
  }

  context_->EnqueueSVMUnmap(ptr_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

void GGEMSOpenCLSVMBuffer::Release() noexcept {
  if (context_ && ptr_) {
    auto &ctx = context_->GetContextNative();
    clSVMFree(ctx(), ptr_);
    GGEMS_INFOEX("OpenCL", 3, "Release SVM Buffer of {}.",
                 HumanReadable(size_));
  }
  context_ = nullptr;
  ptr_ = nullptr;
  size_ = 0_B;
  flags_ = 0;
}
} // namespace ggems::ocl
