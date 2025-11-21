#pragma once

#include "GGEMSQuantity.hh"

namespace ggems::units {
// base: bytes
using Bytes = Quantity<InfoBytesDim, std::uint64_t>;

inline std::string HumanReadable(Bytes const &b) {
  long double const v = static_cast<long double>(b.value);

  if (v >= 1.0L * 1024.0L * 1024.0L * 1024.0L * 1024.0L)
    return std::format("{:6.1f} TB",
                       v / (1024.0L * 1024.0L * 1024.0L * 1024.0L));

  if (v >= 1.0L * 1024.0L * 1024.0L * 1024.0L)
    return std::format("{:6.1f} GB", v / (1024.0L * 1024.0L * 1024.0L));

  if (v >= 1.0L * 1024.0L * 1024.0L)
    return std::format("{:6.1f} MB", v / (1024.0L * 1024.0L));

  if (v >= 1.0L * 1024.0L)
    return std::format("{:6.1f} KB", v / 1024.0L);

  return std::format("{:4.0f} B", v);
}

// B
consteval Bytes operator""_B(unsigned long long v) {
  return Bytes{static_cast<std::uint64_t>(v)};
}

consteval Bytes operator""_B(long double v) {
  return Bytes{static_cast<std::uint64_t>(v)};
}

// KB (base 1024)
consteval Bytes operator""_KB(unsigned long long v) {
  return Bytes{static_cast<std::uint64_t>(v) * 1024ull};
}

consteval Bytes operator""_KB(long double v) {
  return Bytes{static_cast<std::uint64_t>(v * 1024.0L)};
}

// MB
consteval Bytes operator""_MB(unsigned long long v) {
  return Bytes{static_cast<std::uint64_t>(v) * 1024ull * 1024ull};
}

consteval Bytes operator""_MB(long double v) {
  return Bytes{static_cast<std::uint64_t>(v * 1024.0L * 1024.0L)};
}

// GB
consteval Bytes operator""_GB(unsigned long long v) {
  return Bytes{static_cast<std::uint64_t>(v) * 1024ull * 1024ull * 1024ull};
}

consteval Bytes operator""_GB(long double v) {
  return Bytes{static_cast<std::uint64_t>(v * 1024.0L * 1024.0L * 1024.0L)};
}

constexpr std::size_t ToSizeT(Bytes const &b) noexcept {
  return static_cast<std::size_t>(b.value);
}

} // namespace ggems::units
