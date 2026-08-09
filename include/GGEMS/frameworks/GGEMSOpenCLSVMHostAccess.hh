#pragma once

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


#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/frameworks/GGEMSOpenCLSVMBuffer.hh"

namespace ggems::ocl {

template <typename T>
concept SVMHostTransferValue =
    std::is_trivially_copyable_v<T> && !std::is_volatile_v<T>;

namespace detail {

template <typename T> struct IsSpan : std::false_type {};

template <typename T, std::size_t Extent>
struct IsSpan<std::span<T, Extent>> : std::true_type {};

template <SVMHostTransferValue T>
void CheckSVMHostAccessElementCount(std::size_t count) {
  if (!(count <=
                           std::numeric_limits<std::size_t>::max() / sizeof(T))) {
    throw ggems::core::GGEMSInternal("SVM host access byte count exceeds std::size_t.");
  }
}

inline void CheckSVMHostAccessByteCapacity(GGEMSOpenCLSVMBuffer const &buffer,
                                           std::size_t byte_count) {
  if (!(std::cmp_less_equal(byte_count, buffer.GetSize().value))) {
    throw ggems::core::GGEMSInternal(
        "SVM buffer capacity is insufficient for the requested host access.");
  }
}

template <typename Operation>
  requires std::is_nothrow_invocable_v<Operation &>
void WithMappedSVMHostAccess(GGEMSOpenCLSVMBuffer &buffer, cl_map_flags flags,
                             Operation &&operation) {
  buffer.Map(flags);
  std::invoke(operation);
  buffer.Unmap();
}

inline void WriteSVMBytesFromHost(GGEMSOpenCLSVMBuffer &buffer,
                                  std::span<std::byte const> source) {
  CheckSVMHostAccessByteCapacity(buffer, source.size());

  if (source.empty()) {
    return;
  }

  WithMappedSVMHostAccess(buffer, CL_MAP_WRITE, [&buffer, source]() noexcept {
    std::memcpy(buffer.GetData(), source.data(), source.size());
  });
}

inline void ReadSVMBytesToHost(GGEMSOpenCLSVMBuffer &buffer,
                               std::span<std::byte> destination) {
  CheckSVMHostAccessByteCapacity(buffer, destination.size());
  if (destination.empty()) {
    return;
  }

  WithMappedSVMHostAccess(
      buffer, CL_MAP_READ, [&buffer, destination]() noexcept {
        std::memcpy(destination.data(), buffer.GetData(), destination.size());
      });
}
} // namespace detail

template <typename T, std::size_t Extent>
  requires SVMHostTransferValue<T>
void WriteSVMFromHost(GGEMSOpenCLSVMBuffer &buffer,
                      std::span<T, Extent> values) {
  detail::CheckSVMHostAccessElementCount<T>(values.size());
  detail::WriteSVMBytesFromHost(buffer, std::as_bytes(values));
}

template <SVMHostTransferValue T>
  requires(!detail::IsSpan<std::remove_cvref_t<T>>::value)
void WriteSVMFromHost(GGEMSOpenCLSVMBuffer &buffer, T const &value) {
  WriteSVMFromHost(buffer, std::span<T const, 1U>{std::addressof(value), 1U});
}

template <SVMHostTransferValue T>
[[nodiscard]] T ReadSVMToHost(GGEMSOpenCLSVMBuffer &buffer) {
  std::array<std::byte, sizeof(T)> representation{};

  detail::ReadSVMBytesToHost(buffer, std::span{representation});

  return std::bit_cast<T>(representation);
}

template <typename T, std::size_t Extent>
  requires SVMHostTransferValue<T> && (!std::is_const_v<T>)
void ReadSVMToHost(GGEMSOpenCLSVMBuffer &buffer,
                   std::span<T, Extent> destination) {
  detail::CheckSVMHostAccessElementCount<T>(destination.size());
  detail::ReadSVMBytesToHost(buffer, std::as_writable_bytes(destination));
}

template <SVMHostTransferValue T>
void FillSVMFromHost(GGEMSOpenCLSVMBuffer &buffer, std::size_t count,
                     T const &value) {
  detail::CheckSVMHostAccessElementCount<T>(count);
  detail::CheckSVMHostAccessByteCapacity(buffer, count * sizeof(T));

  if (count == 0U) {
    return;
  }

  detail::WithMappedSVMHostAccess(
      buffer, CL_MAP_WRITE, [&buffer, count, &value]() noexcept {
        auto *destination = static_cast<std::byte *>(buffer.GetData());
        for (std::size_t index = 0U; index < count; ++index) {
          std::memcpy(destination, std::addressof(value), sizeof(T));
          destination += sizeof(T);
        }
      });
}

template <SVMHostTransferValue T, typename Generator>
  requires std::is_nothrow_invocable_r_v<T, Generator &, std::size_t>
void GenerateSVMFromHost(GGEMSOpenCLSVMBuffer &buffer, std::size_t count,
                         Generator &&generator) {
  detail::CheckSVMHostAccessElementCount<T>(count);
  detail::CheckSVMHostAccessByteCapacity(buffer, count * sizeof(T));

  if (count == 0U) {
    return;
  }

  detail::WithMappedSVMHostAccess(
      buffer, CL_MAP_WRITE, [&buffer, count, &generator]() noexcept {
        auto *destination = static_cast<std::byte *>(buffer.GetData());

        for (std::size_t index = 0U; index < count; ++index) {
          T value = std::invoke(generator, index);
          std::memcpy(destination, std::addressof(value), sizeof(T));
          destination += sizeof(T);
        }
      });
}
} // namespace ggems::ocl
