#pragma once

#include "GGEMSLengthUnits.hh"
#include "GGEMSQuantity.hh"
#include "GGEMSTimeUnits.hh"

namespace ggems::units {
using Speed = Quantity<SpeedDim, long double>; // m/s

inline std::string HumanReadable(Speed const &v) {
  long double const x = v.value;

  if (x >= 100.0L) // ~ 360 km/h
    return std::format("{} km/h", x * 3.6L);

  if (x >= 1.0L)
    return std::format("{} m/s", x);

  if (x >= 1.0e-3L)
    return std::format("{} mm/ms", x * 1.0e3L);

  if (x >= 1.0e-6L)
    return std::format("{} mm/us", x * 1.0e6L);

  return std::format("{} m/s", x); // fallback
}

// Littéral direct en m/s
consteval Speed operator""_m_s(unsigned long long v) {
  return Speed{static_cast<long double>(v)};
}

consteval Speed operator""_m_s(long double v) {
  return Speed{static_cast<long double>(v)};
}

// km/h → conversion en m/s
consteval Speed operator""_km_h(unsigned long long v) {
  long double mps = static_cast<long double>(v) * (1000.0L / 3600.0L);
  return Speed{mps};
}

consteval Speed operator""_km_h(long double v) {
  long double mps = v * (1000.0L / 3600.0L);
  return Speed{mps};
}

// Helper: Length / Time -> Speed
// Length in m, Time in s soft-interpretation:
// ici on suppose Length et Time déjà "normalisés" par leurs bases.
inline Speed MakeSpeed(Length L, Time t) {
  // Length in pm, Time in ps -> 1 m/s = 1
  long double m = static_cast<long double>(L.value) / 1.0e12L;
  long double s = static_cast<long double>(t.value) / 1.0e12L;
  return Speed{m / s};
}

} // namespace ggems::units
