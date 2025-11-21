#pragma once

#include "GGEMSQuantity.hh"

namespace ggems::units {
// base: Hz (1/s)
using Frequency = Quantity<FrequencyDim, std::uint64_t>;

inline std::string HumanReadable(Frequency const &f) {
  long double const v = static_cast<long double>(f.value);

  if (v >= 1.0e12L)
    return std::format("{:5.1f} THz", v / 1.0e12L);

  if (v >= 1.0e9L)
    return std::format("{:5.1f} GHz", v / 1.0e9L);

  if (v >= 1.0e6L)
    return std::format("{:5.1f} MHz", v / 1.0e6L);

  if (v >= 1.0e3L)
    return std::format("{:5.1f} kHz", v / 1.0e3L);

  return std::format("{:3.0f} Hz", v);
}

consteval Frequency operator""_Hz(unsigned long long v) {
  return Frequency{static_cast<std::uint64_t>(v)};
}

consteval Frequency operator""_Hz(long double v) {
  return Frequency{static_cast<std::uint64_t>(v)};
}

consteval Frequency operator""_kHz(unsigned long long v) {
  return Frequency{static_cast<std::uint64_t>(v) * 1000ull};
}

consteval Frequency operator""_kHz(long double v) {
  return Frequency{static_cast<std::uint64_t>(v * 1.0e3L)};
}

consteval Frequency operator""_MHz(unsigned long long v) {
  return Frequency{static_cast<std::uint64_t>(v) * 1'000'000ull};
}

consteval Frequency operator""_MHz(long double v) {
  return Frequency{static_cast<std::uint64_t>(v * 1.0e6L)};
}

consteval Frequency operator""_GHz(unsigned long long v) {
  return Frequency{static_cast<std::uint64_t>(v) * 1'000'000'000ull};
}

consteval Frequency operator""_GHz(long double v) {
  return Frequency{static_cast<std::uint64_t>(v * 1.0e9L)};
}

} // namespace ggems::units
