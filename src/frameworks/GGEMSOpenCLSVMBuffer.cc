// *****************************************************************************
// * This file is part of GGEMS.                                               *
// *                                                                           *
// * SPDX-License-Identifier: GPL-3.0-or-later                                 *
// * Copyright (C) 2017-2026 CHRU de Brest, Université de Bretagne Occidentale,*
// * Inserm.                                                                   *
// *                                                                           *
// * GGEMS is free software: you can redistribute it and/or modify             *
// * it under the terms of the GNU General Public License as published by      *
// * the Free Software Foundation, either version 3 of the License, or         *
// * (at your option) any later version.                                       *
// *                                                                           *
// * GGEMS is distributed in the hope that it will be useful,                  *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
// * GNU General Public License for more details.                              *
// *                                                                           *
// * You should have received a copy of the GNU General Public License         *
// * along with GGEMS. If not, see <https://www.gnu.org/licenses/>.            *
// *****************************************************************************

/*!
 * \file
 * \brief Implements the OpenCL shared virtual memory buffer wrapper.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <utility>
#include <exception>
#include <cstddef>
/// \endcond

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/frameworks/GGEMSOpenCLContext.hh"
#include "GGEMS/frameworks/GGEMSOpenCLSVMBuffer.hh"
#include "GGEMS/frameworks/GGEMSOpenCLSVMMemoryKind.hh"
#include "GGEMS/core/units/GGEMSBytesUnits.hh"

namespace ggems::ocl {

// =============================================================================
// =============================================================================

GGEMSOpenCLSVMBuffer::GGEMSOpenCLSVMBuffer(GGEMSOpenCLContext &context,
                                           units::Bytes size,
                                           cl_svm_mem_flags flags,
                                           SVMMemoryKind kind,
                                           units::Bytes alignment)
    : context_(&context), size_{size}, flags_{flags}, kind_{kind} {
  if (!(size_.value > 0)) {
    throw ggems::core::GGEMSFatal("Cannot allocate zero-sized SVM buffer.");
  }

  auto const &svm = context.GetSVMSupport();
  if (!(svm.HasAny())) {
    throw ggems::core::GGEMSFatal("Device does not support any form of SVM.");
  }

  auto const &ctx = context.GetContextNative();

  void *svm_ptr =
      clSVMAlloc(ctx(), flags_, static_cast<std::size_t>(size.value),
                 static_cast<cl_uint>(alignment.value));
  if (svm_ptr == nullptr) {
    throw ggems::core::GGEMSFatal("clSVMAlloc failed: returned nullptr.");
  }

  ptr_ = svm_ptr;
  context_->RegisterSVMAllocation(size_);
}

// -----------------------------------------------------------------------------

GGEMSOpenCLSVMBuffer::GGEMSOpenCLSVMBuffer(
    GGEMSOpenCLSVMBuffer &&other) noexcept {
  *this = std::move(other);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLSVMBuffer::operator=(GGEMSOpenCLSVMBuffer &&other) noexcept
    -> GGEMSOpenCLSVMBuffer & {
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

// -----------------------------------------------------------------------------

GGEMSOpenCLSVMBuffer::~GGEMSOpenCLSVMBuffer() noexcept { Release(); }

// -----------------------------------------------------------------------------

auto GGEMSOpenCLSVMBuffer::Map(cl_map_flags flags) -> void {
  if (!RequiresExplicitMap(kind_)) {
    return;
  }

  context_->EnqueueSVMMap(ptr_, size_, flags);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLSVMBuffer::Unmap() -> void {
  if (!RequiresExplicitMap(kind_)) {
    return;
  }

  context_->EnqueueSVMUnmap(ptr_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLSVMBuffer::Release() noexcept -> void {
  if (context_ == nullptr || ptr_ == nullptr) {
    context_ = nullptr;
    ptr_ = nullptr;
    size_ = 0_B;
    flags_ = 0;
    kind_ = SVMMemoryKind::None;
    return;
  }

  auto *context = context_;
  auto *ptr = ptr_;
  auto const size = size_;

  auto const &queue = context->GetCommandQueueNative();
  if (clFinish(queue()) != CL_SUCCESS) {
    std::terminate();
  }

  auto const &native_context = context->GetContextNative();
  clSVMFree(native_context(), ptr);

  context_ = nullptr;
  ptr_ = nullptr;
  size_ = 0_B;
  flags_ = 0;
  kind_ = SVMMemoryKind::None;

  context->RegisterSVMRelease(size);
}
} // namespace ggems::ocl
