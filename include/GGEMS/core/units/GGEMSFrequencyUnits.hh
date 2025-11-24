#pragma once

#include "GGEMSQuantity.hh"

/// \cond
#include <array>
/// \endcond

namespace ggems::units {
// base: Hz (1/s)
using Frequency = Quantity<FrequencyDim, std::uint64_t>;

inline std::string HumanReadable(Frequency const &f, std::int8_t precision = 7,
                                 std::int8_t width = -1) {
  long double const v = static_cast<long double>(f.value);

  struct Unit {
    long double threshold;
    std::string_view suffix;
    long double scale;
  };

  static constexpr std::array<Unit, 5> units{{{1.0e12L, " THz", 1.0e12L},
                                              {1.0e9L, " GHz", 1.0e9L},
                                              {1.0e6L, " MHz", 1.0e6L},
                                              {1.0e3L, " kHz", 1.0e3L},
                                              {0.0L, " Hz", 1.0L}}};

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

  return std::format("{:.{}f} Hz", v, precision);
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
