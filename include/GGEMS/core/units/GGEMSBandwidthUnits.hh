#pragma once

#include "GGEMSBytesUnits.hh"
#include "GGEMSQuantity.hh"
#include "GGEMSTimeUnits.hh"

namespace ggems::units {
using Bandwidth = Quantity<BandwidthDim, long double>;

inline std::string HumanReadable(Bandwidth const &bw) {
  long double const v = bw.value * 1.0e12L; // conv to B/s

  if (v >= 1.0e9L)
    return std::format("{:.3} GB/s", v / 1.0e9L);

  if (v >= 1.0e6L)
    return std::format("{:.3} MB/s", v / 1.0e6L);

  if (v >= 1.0e3L)
    return std::format("{:.3} KB/s", v / 1.0e3L);

  return std::format("{:.3} B/s", v);
}

// Direct literals in B/s
consteval Bandwidth operator""_Bps(unsigned long long v) {
  return Bandwidth{static_cast<long double>(v)};
}

consteval Bandwidth operator""_Bps(long double v) {
  return Bandwidth{static_cast<long double>(v)};
}

consteval Bandwidth operator""_MBps(unsigned long long v) {
  return Bandwidth{static_cast<long double>(v) * 1.0e6L};
}

consteval Bandwidth operator""_MBps(long double v) {
  return Bandwidth{static_cast<long double>(v) * 1.0e6L};
}

consteval Bandwidth operator""_GBps(unsigned long long v) {
  return Bandwidth{static_cast<long double>(v) * 1.0e9L};
}

consteval Bandwidth operator""_GBps(long double v) {
  return Bandwidth{static_cast<long double>(v) * 1.0e9L};
}

// Helper: Bytes / Time -> Bandwidth in B/s
inline Bandwidth MakeBandwidth(Bytes bytes, Time time) {
  long double B = static_cast<long double>(bytes.value);
  long double ps = static_cast<long double>(time.value); // base ps
  long double s = ps / 1.0e12L;
  return Bandwidth{B / s};
}

} // namespace ggems::units
