// ************************************************************************
// * This file is part of GGEMS.                                          *
// *                                                                      *
// * GGEMS is free software: you can redistribute it and/or modify        *
// * it under the terms of the GNU General Public License as published by *
// * the Free Software Foundation, either version 3 of the License, or    *
// * (at your option) any later version.                                  *
// *                                                                      *
// * GGEMS is distributed in the hope that it will be useful,             *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
// * GNU General Public License for more details.                         *
// *                                                                      *
// * You should have received a copy of the GNU General Public License    *
// * along with GGEMS.  If not, see <https://www.gnu.org/licenses/>.      *
// *                                                                      *
// ************************************************************************

/*!
 * \file GGEMSOpenCLSVMBuffer.cc
 * \brief Declaration of GGEMSOpenCLSVMBuffer, a RAII wrapper for OpenCL SVM
 * buffers.
 *
 * This class encapsulates creation, mapping, unmapping and destruction of
 * Shared Virtual Memory (SVM) buffers as defined in OpenCL 2.x. It provides a
 * high-level, exception-safe abstraction while preserving direct access to
 * the underlying pointer when required by compute kernels.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-12-08
 * \version 2.0
 * \copyright
 * GNU General Public License v3.0
 */

#include "GGEMS/frameworks/GGEMSOpenCLContext.hh"

using namespace ggems::units;

namespace ggems::ocl {

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

GGEMSOpenCLSVMBuffer::GGEMSOpenCLSVMBuffer(GGEMSOpenCLContext &context,
                                           Bytes size, cl_svm_mem_flags flags,
                                           Bytes alignment)
    : context_(&context), ptr_{nullptr}, size_{size}, flags_{flags} {
  GGEMS_CHECK(size_.value > 0, "Cannot allocate zero-sized SVM buffer.");

  auto const &svm = context.GetSVMSupport();
  GGEMS_CHECK(svm.HasAny(), "Device does not support any form of SVM.");

  auto &ctx = context.GetContextNative();

  void *p = clSVMAlloc(ctx(), flags_, static_cast<std::size_t>(size.value),
                       static_cast<cl_uint>(alignment.value));
  GGEMS_CHECK(p, "clSVMalloc failed: returned nullptr.");

  ptr_ = p;
  context_->RegisterSVMAllocation(size_);

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

  if (svm.fine_grain_system) {
    return;
  }

  context_->EnqueueSVMMap(ptr_, size_, flags);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

void GGEMSOpenCLSVMBuffer::Unmap() {
  auto const &svm = context_->GetSVMSupport();

  if (svm.fine_grain_system) {
    return;
  }

  context_->EnqueueSVMUnmap(ptr_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

void GGEMSOpenCLSVMBuffer::Release() noexcept {
  if (context_ && ptr_) {
    context_->RegisterSVMRelease(size_);

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
