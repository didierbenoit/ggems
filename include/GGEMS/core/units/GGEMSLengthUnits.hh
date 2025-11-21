#pragma once

#include "GGEMSQuantity.hh"

namespace ggems::units {
using Length = Quantity<LengthDim, std::uint64_t>; // base: picometres

inline std::string HumanReadable(Length const &L) {
  long double const pm = static_cast<long double>(L.value);

  if (pm >= 1.0e15L) // m
    return std::format("{:5.1f} km", pm / 1.0e15L);

  if (pm >= 1.0e12L) // m
    return std::format("{:5.1f} m", pm / 1.0e12L);

  if (pm >= 1.0e10L) // cm
    return std::format("{:5.1f} cm", pm / 1.0e10L);

  if (pm >= 1.0e9L) // mm
    return std::format("{:5.1f} mm", pm / 1.0e9L);

  if (pm >= 1.0e6L) // um
    return std::format("{:5.1f} um", pm / 1.0e6L);

  if (pm >= 1.0e3L) // nm
    return std::format("{:5.1f} nm", pm / 1.0e3L);

  return std::format("{:3.0f} pm", pm); // base
}

// picometres
consteval Length operator""_pm(unsigned long long v) {
  return Length{static_cast<std::uint64_t>(v)};
}

consteval Length operator""_pm(long double v) {
  return Length{static_cast<std::uint64_t>(v)};
}

// nanometres
consteval Length operator""_nm(unsigned long long v) {
  return Length{static_cast<std::uint64_t>(v) * 1000ull};
}

consteval Length operator""_nm(long double v) {
  return Length{static_cast<std::uint64_t>(v * 1.0e3L)};
}

// micrometres
consteval Length operator""_um(unsigned long long v) {
  return Length{static_cast<std::uint64_t>(v) * 1'000'000ull};
}

consteval Length operator""_um(long double v) {
  return Length{static_cast<std::uint64_t>(v * 1.0e6L)};
}

// millimetres
consteval Length operator""_mm(unsigned long long v) {
  return Length{static_cast<std::uint64_t>(v) * 1'000'000'000ull};
}

consteval Length operator""_mm(long double v) {
  return Length{static_cast<std::uint64_t>(v * 1.0e9L)};
}

// centimetres
consteval Length operator""_cm(unsigned long long v) {
  return Length{static_cast<std::uint64_t>(v) * 10'000'000'000ull};
}

consteval Length operator""_cm(long double v) {
  return Length{static_cast<std::uint64_t>(v * 1.0e10L)};
}

// metres
consteval Length operator""_m(unsigned long long v) {
  return Length{static_cast<std::uint64_t>(v) * 1'000'000'000'000ull};
}

consteval Length operator""_m(long double v) {
  return Length{static_cast<std::uint64_t>(v * 1.0e12L)};
}

} // namespace ggems::units
