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
 * \brief Provides host-side transfer utilities for OpenCL SVM buffers.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <array>
#include <bit>
#include <cstddef>
#include <cstring>
#include <functional>
#include <limits>
#include <memory>
#include <span>
#include <type_traits>
#include <utility>
/// \endcond

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/frameworks/GGEMSOpenCLSVMBuffer.hh"

namespace ggems::ocl {

/*!
 * \brief Restricts SVM host transfers to non-volatile trivially copyable types.
 *
 * \tparam T Value type to validate.
 */
template <typename T>
concept SVMHostTransferValue =
    std::is_trivially_copyable_v<T> && !std::is_volatile_v<T>;

/*!
 * \namespace ggems::ocl::detail
 * \brief Provides internal helpers for SVM host-access operations.
 */
namespace detail {

/*!
 * \brief Identifies whether a type is a standard span.
 *
 * \tparam T Type to inspect.
 */
template <typename T> struct IsSpan : std::false_type {};

/*!
 * \brief Identifies a standard span specialization.
 *
 * \tparam T Span element type.
 * \tparam Extent Span extent.
 */
template <typename T, std::size_t Extent>
struct IsSpan<std::span<T, Extent>> : std::true_type {};

/*!
 * \brief Checks that an element count can be represented as a byte count.
 *
 * \tparam T Element type.
 * \param[in] count Number of elements.
 * \throws ggems::core::GGEMSInternal If the corresponding byte count exceeds
 *                                   the range of std::size_t.
 */
template <SVMHostTransferValue T>
auto CheckSVMHostAccessElementCount(std::size_t count) -> void {
  if (!(count <= std::numeric_limits<std::size_t>::max() / sizeof(T))) {
    throw ggems::core::GGEMSInternal(
        "SVM host access byte count exceeds std::size_t.");
  }
}

/*!
 * \brief Checks that an SVM buffer can hold a requested byte count.
 *
 * \param[in] buffer SVM buffer to check.
 * \param[in] byte_count Number of bytes required by the host operation.
 * \throws ggems::core::GGEMSInternal If the buffer capacity is insufficient.
 */
inline auto CheckSVMHostAccessByteCapacity(GGEMSOpenCLSVMBuffer const &buffer,
                                           std::size_t byte_count) -> void {
  if (!(std::cmp_less_equal(byte_count, buffer.GetSize().value))) {
    throw ggems::core::GGEMSInternal(
        "SVM buffer capacity is insufficient for the requested host access.");
  }
}

/*!
 * \brief Executes a host operation while the SVM buffer is mapped as required.
 *
 * \tparam Operation Nothrow callable type of the host operation.
 * \param[in,out] buffer SVM buffer accessed by the operation.
 * \param[in] flags OpenCL map flags.
 * \param[in] operation Host operation to invoke while the buffer is accessible.
 */
template <typename Operation>
  requires std::is_nothrow_invocable_v<Operation &>
auto WithMappedSVMHostAccess(GGEMSOpenCLSVMBuffer &buffer, cl_map_flags flags,
                             Operation &&operation) -> void {
  buffer.Map(flags);
  std::invoke(operation);
  buffer.Unmap();
}

/*!
 * \brief Copies a byte sequence from host memory to an SVM buffer.
 *
 * \param[in,out] buffer Destination SVM buffer.
 * \param[in] source Host byte sequence to copy.
 * \throws ggems::core::GGEMSInternal If the buffer capacity is insufficient.
 */
inline auto WriteSVMBytesFromHost(GGEMSOpenCLSVMBuffer &buffer,
                                  std::span<std::byte const> source) -> void {
  CheckSVMHostAccessByteCapacity(buffer, source.size());

  if (source.empty()) {
    return;
  }

  WithMappedSVMHostAccess(
      buffer, CL_MAP_WRITE, [&buffer, source]() noexcept -> void {
        std::memcpy(buffer.GetData(), source.data(), source.size());
      });
}

/*!
 * \brief Copies a byte sequence from an SVM buffer to host memory.
 *
 * \param[in,out] buffer Source SVM buffer.
 * \param[out] destination Host byte sequence receiving the data.
 * \throws ggems::core::GGEMSInternal If the buffer capacity is insufficient.
 */
inline auto ReadSVMBytesToHost(GGEMSOpenCLSVMBuffer &buffer,
                               std::span<std::byte> destination) -> void {
  CheckSVMHostAccessByteCapacity(buffer, destination.size());
  if (destination.empty()) {
    return;
  }

  WithMappedSVMHostAccess(
      buffer, CL_MAP_READ, [&buffer, destination]() noexcept -> void {
        std::memcpy(destination.data(), buffer.GetData(), destination.size());
      });
}
} // namespace detail

/*!
 * \brief Copies a span of values from host memory to an SVM buffer.
 *
 * \tparam T Element type.
 * \tparam Extent Span extent.
 * \param[in,out] buffer Destination SVM buffer.
 * \param[in] values Host values to copy.
 * \throws ggems::core::GGEMSInternal If the byte count overflows std::size_t or
 *                                   the buffer capacity is insufficient.
 */
template <typename T, std::size_t Extent>
  requires SVMHostTransferValue<T>
auto WriteSVMFromHost(GGEMSOpenCLSVMBuffer &buffer, std::span<T, Extent> values)
    -> void {
  detail::CheckSVMHostAccessElementCount<T>(values.size());
  detail::WriteSVMBytesFromHost(buffer, std::as_bytes(values));
}

/*!
 * \brief Copies one value from host memory to an SVM buffer.
 *
 * \tparam T Value type.
 * \param[in,out] buffer Destination SVM buffer.
 * \param[in] value Host value to copy.
 * \throws ggems::core::GGEMSInternal If the byte count overflows std::size_t or
 *                                   the buffer capacity is insufficient.
 */
template <SVMHostTransferValue T>
  requires(!detail::IsSpan<std::remove_cvref_t<T>>::value)
auto WriteSVMFromHost(GGEMSOpenCLSVMBuffer &buffer, T const &value) -> void {
  WriteSVMFromHost(buffer, std::span<T const, 1U>{std::addressof(value), 1U});
}

/*!
 * \brief Reads one value from an SVM buffer into host memory.
 *
 * \tparam T Value type.
 * \param[in,out] buffer Source SVM buffer.
 * \return Value read from the SVM buffer.
 * \throws ggems::core::GGEMSInternal If the buffer capacity is insufficient.
 */
template <SVMHostTransferValue T>
[[nodiscard]] auto ReadSVMToHost(GGEMSOpenCLSVMBuffer &buffer) -> T {
  std::array<std::byte, sizeof(T)> representation{};

  detail::ReadSVMBytesToHost(buffer, std::span{representation});

  return std::bit_cast<T>(representation);
}

/*!
 * \brief Copies values from an SVM buffer to a host span.
 *
 * \tparam T Element type.
 * \tparam Extent Span extent.
 * \param[in,out] buffer Source SVM buffer.
 * \param[out] destination Host span receiving the values.
 * \throws ggems::core::GGEMSInternal If the byte count overflows std::size_t or
 *                                   the buffer capacity is insufficient.
 */
template <typename T, std::size_t Extent>
  requires SVMHostTransferValue<T> && (!std::is_const_v<T>)
auto ReadSVMToHost(GGEMSOpenCLSVMBuffer &buffer,
                   std::span<T, Extent> destination) -> void {
  detail::CheckSVMHostAccessElementCount<T>(destination.size());
  detail::ReadSVMBytesToHost(buffer, std::as_writable_bytes(destination));
}

/*!
 * \brief Fills an SVM buffer with repeated copies of a host value.
 *
 * \tparam T Value type.
 * \param[in,out] buffer Destination SVM buffer.
 * \param[in] count Number of values to write.
 * \param[in] value Host value to repeat.
 * \throws ggems::core::GGEMSInternal If the byte count overflows std::size_t or
 *                                   the buffer capacity is insufficient.
 */
template <SVMHostTransferValue T>
auto FillSVMFromHost(GGEMSOpenCLSVMBuffer &buffer, std::size_t count,
                     T const &value) -> void {
  detail::CheckSVMHostAccessElementCount<T>(count);
  detail::CheckSVMHostAccessByteCapacity(buffer, count * sizeof(T));

  if (count == 0U) {
    return;
  }

  detail::WithMappedSVMHostAccess(
      buffer, CL_MAP_WRITE, [&buffer, count, &value]() noexcept -> void {
        auto *destination = static_cast<std::byte *>(buffer.GetData());
        for (std::size_t index = 0U; index < count; ++index) {
          std::memcpy(destination, std::addressof(value), sizeof(T));
          destination += sizeof(T);
        }
      });
}

/*!
 * \brief Generates host values and writes them sequentially to an SVM buffer.
 *
 * \tparam T Generated value type.
 * \tparam Generator Nothrow callable type invoked for each element index.
 * \param[in,out] buffer Destination SVM buffer.
 * \param[in] count Number of values to generate.
 * \param[in] generator Callable that generates a value for each element index.
 * \throws ggems::core::GGEMSInternal If the byte count overflows std::size_t or
 *                                   the buffer capacity is insufficient.
 */
template <SVMHostTransferValue T, typename Generator>
  requires std::is_nothrow_invocable_r_v<T, Generator &, std::size_t>
auto GenerateSVMFromHost(GGEMSOpenCLSVMBuffer &buffer, std::size_t count,
                         Generator &&generator) -> void {
  detail::CheckSVMHostAccessElementCount<T>(count);
  detail::CheckSVMHostAccessByteCapacity(buffer, count * sizeof(T));

  if (count == 0U) {
    return;
  }

  detail::WithMappedSVMHostAccess(
      buffer, CL_MAP_WRITE, [&buffer, count, &generator]() noexcept -> void {
        auto *destination = static_cast<std::byte *>(buffer.GetData());

        for (std::size_t index = 0U; index < count; ++index) {
          T value = std::invoke(generator, index);
          std::memcpy(destination, std::addressof(value), sizeof(T));
          destination += sizeof(T);
        }
      });
}
} // namespace ggems::ocl
