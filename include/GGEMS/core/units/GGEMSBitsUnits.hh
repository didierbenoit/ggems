#pragma once

#include "GGEMSQuantity.hh"

#include <format>
#include <string>

namespace ggems::units {
//
// Base unit: bit (1 or 0)
//
using Bits = Quantity<InfoBitsDim, std::uint64_t>;

//
// Human-readable conversion for bits
//
inline std::string HumanReadable(Bits const &b) {
  long double const v = static_cast<long double>(b.value);

  if (v >= 1.0e9L)
    return std::format("{:6.1f} Tb", v / 1.0e9L);

  if (v >= 1.0e9L)
    return std::format("{:6.1f} Gb", v / 1.0e9L);

  if (v >= 1.0e6L)
    return std::format("{:6.1f} Mb", v / 1.0e6L);

  if (v >= 1.0e3L)
    return std::format("{:6.1f} Kb", v / 1.0e3L);

  return std::format("{:4.0f} b", v);
}

//
// User-defined literals
// Base: bits
//

// bits
consteval Bits operator""_b(unsigned long long v) {
  return Bits{static_cast<std::uint64_t>(v)};
}

consteval Bits operator""_b(long double v) {
  return Bits{static_cast<std::uint64_t>(v)};
}

// kilobits (1000 bits)
consteval Bits operator""_Kb(unsigned long long v) {
  return Bits{static_cast<std::uint64_t>(v) * 1000ull};
}

consteval Bits operator""_Kb(long double v) {
  return Bits{static_cast<std::uint64_t>(v * 1.0e3L)};
}

// megabits (1e6 bits)
consteval Bits operator""_Mb(unsigned long long v) {
  return Bits{static_cast<std::uint64_t>(v) * 1'000'000ull};
}

consteval Bits operator""_Mb(long double v) {
  return Bits{static_cast<std::uint64_t>(v * 1.0e6L)};
}

// gigabits (1e9 bits)
consteval Bits operator""_Gb(unsigned long long v) {
  return Bits{static_cast<std::uint64_t>(v) * 1'000'000'000ull};
}

consteval Bits operator""_Gb(long double v) {
  return Bits{static_cast<std::uint64_t>(v * 1.0e9L)};
}

} // namespace ggems::units
