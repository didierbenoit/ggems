#pragma once

#include "GGEMSQuantity.hh"

/// \cond
#include <array>
/// \endcond

namespace ggems::units {
//
// Base unit: bit (1 or 0)
//
using Bits = Quantity<InfoBitsDim, std::uint64_t>;

//
// Human-readable conversion for bits
//
inline std::string HumanReadable(Bits const &b, std::int8_t precision = 7,
                                 std::int8_t width = -1) {
  long double const v = static_cast<long double>(b.value);

  struct Unit {
    long double threshold;
    std::string_view suffix;
    long double scale;
  };

  static constexpr std::array<Unit, 5> units{{{1.0e12L, " Tb", 1.0e12L},
                                              {1.0e9L, " Gb", 1.0e9L},
                                              {1.0e6L, " Mb", 1.0e6L},
                                              {1.0e3L, " kb", 1.0e3L},
                                              {0.0L, " b", 1.0L}}};

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

  return std::format("{:.{}f} b", v, precision);
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
