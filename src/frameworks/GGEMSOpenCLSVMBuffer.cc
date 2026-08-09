// ************************************************************************
// ************************************************************************


#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSLogMacros.hh"
#include "GGEMS/frameworks/GGEMSOpenCLContext.hh"

using namespace ggems::units;

namespace ggems::ocl {

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

GGEMSOpenCLSVMBuffer::GGEMSOpenCLSVMBuffer(GGEMSOpenCLContext &context,
                                           Bytes size, cl_svm_mem_flags flags,
                                           SVMMemoryKind kind, Bytes alignment)
    : context_(&context), ptr_{nullptr}, size_{size}, flags_{flags},
      kind_{kind} {
  if (!(size_.value > 0)) {
    throw ggems::core::GGEMSFatal("Cannot allocate zero-sized SVM buffer.");
  }

  auto const &svm = context.GetSVMSupport();
  if (!(svm.HasAny())) {
    throw ggems::core::GGEMSFatal("Device does not support any form of SVM.");
  }

  auto &ctx = context.GetContextNative();

  void *p = clSVMAlloc(ctx(), flags_, static_cast<std::size_t>(size.value),
                       static_cast<cl_uint>(alignment.value));
  if (!(p)) {
    throw ggems::core::GGEMSFatal("clSVMAlloc failed: returned nullptr.");
  }

  ptr_ = p;
  context_->RegisterSVMAllocation(size_);

  GGEMS_INFOEX("OpenCL", 3, "Allocated SVM Buffer: {} ({}).",
               HumanReadable(size_), ToString(kind_));
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
    kind_ = other.kind_;

    other.context_ = nullptr;
    other.ptr_ = nullptr;
    other.size_ = 0_B;
    other.flags_ = 0;
    other.kind_ = SVMMemoryKind::None;
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
  if (!RequiresExplicitMap(kind_)) {
    return;
  }

  context_->EnqueueSVMMap(ptr_, size_, flags);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

void GGEMSOpenCLSVMBuffer::Unmap() {
  if (!RequiresExplicitMap(kind_)) {
    return;
  }

  context_->EnqueueSVMUnmap(ptr_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

void GGEMSOpenCLSVMBuffer::Release() noexcept {
  if (context_ && ptr_) {
    auto &queue = context_->GetCommandQueueNative();
    clFinish(queue());

    context_->RegisterSVMRelease(size_);

    auto &ctx = context_->GetContextNative();
    clSVMFree(ctx(), ptr_);

    GGEMS_INFOEX("OpenCL", 3, "Release SVM Buffer: {} ({}).",
                 HumanReadable(size_), ToString(kind_));
  }

  context_ = nullptr;
  ptr_ = nullptr;
  size_ = 0_B;
  flags_ = 0;
  kind_ = SVMMemoryKind::None;
}
} // namespace ggems::ocl
