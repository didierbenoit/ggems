#pragma once

#include "GGEMSBytesUnits.hh"
#include "GGEMSQuantity.hh"
#include "GGEMSTimeUnits.hh"

/// \cond
#include <array>
/// \endcond

namespace ggems::units {
using Bandwidth = Quantity<BandwidthDim, long double>;

inline std::string HumanReadable(Bandwidth const &bw, std::int8_t precision = 7,
                                 std::int8_t width = -1) {
  long double const v = bw.value * 1.0e12L; // conv to B/s

  struct Unit {
    long double threshold;
    std::string_view suffix;
    long double scale;
  };

  static constexpr std::array<Unit, 5> units{{{1.0e12L, " TB/s", 1.0e12L},
                                              {1.0e9L, " GB/s", 1.0e9L},
                                              {1.0e6L, " MB/s", 1.0e6L},
                                              {1.0e3L, " KB/s", 1.0e3L},
                                              {0.0L, " B/s", 1.0L}}};

  for (auto const &u : units) {
    if (v >= u.threshold) {

      long double scaled = v / u.scale;

      // Buffer local pour fabriquer le format dynamiquement
      std::string fmt;

      if (width < 0) {
        // {:.7f} + suffix
        fmt = std::format("{{:.{}f}}{}", precision, u.suffix);
      } else {
        // {:10.7f} + suffix
        fmt = std::format("{{:{}.{}f}}{}", width, precision, u.suffix);
      }

      return std::vformat(fmt, std::make_format_args(scaled));
    }
  }

  return std::format("{:.{}f} B/s", v, precision);
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
