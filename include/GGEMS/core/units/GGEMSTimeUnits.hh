#pragma once

#include "GGEMSQuantity.hh"

/// \cond
#include <array>
/// \endcond

namespace ggems::units {
using Time = Quantity<TimeDim, uint64_t>; // base: picoseconds

// Human-readable time
inline std::string HumanReadable(Time const &t, std::int8_t precision = 7,
                                 std::int8_t width = -1) {
  long double const v = static_cast<long double>(t.value);

  // >= 60 s → h / min / s / ms
  if (v >= 60.0L * 1.0e12L) {
    uint64_t total_ps = t.value;

    uint64_t total_s = total_ps / 1'000'000'000'000ull;
    uint64_t ps_rest = total_ps % 1'000'000'000'000ull;

    uint64_t hours = total_s / 3600ull;
    uint64_t minutes = (total_s % 3600ull) / 60ull;
    uint64_t seconds = total_s % 60ull;

    uint64_t ms = ps_rest / 1'000'000'000ull; // 1 ms = 1e9 ps

    if (hours > 0) {
      return std::format("{} h {} min {} s {} ms", hours, minutes, seconds, ms);
    }

    return std::format("{} min {} s {} ms", minutes, seconds, ms);
  }

  struct Unit {
    long double threshold_ps;
    long double scale;
    std::string_view suffix;
  };

  static constexpr std::array<Unit, 5> units{{
      {1.0e12L, 1.0e12L, " s"}, // >= 1s
      {1.0e9L, 1.0e9L, " ms"},  // >= 1ms
      {1.0e6L, 1.0e6L, " us"},  // >= 1us
      {1.0e3L, 1.0e3L, " ns"},  // >= 1ns
      {0.0L, 1.0L, " ps"}       // < 1ns
  }};

  for (auto const &u : units) {
    if (v >= u.threshold_ps) {

      long double scaled = v / u.scale;

      // Construire format dynamique : libre ou fixe
      std::string fmt;
      if (width < 0) {
        fmt = std::format("{{:.{}f}}{}", precision, u.suffix);
      } else {
        fmt = std::format("{{:{}.{}f}}{}", width, precision, u.suffix);
      }

      return std::vformat(fmt, std::make_format_args(scaled));
    }
  }

  // fallback jamais atteint
  return std::format("{:.{}f} ps", v, precision);
}

// User-defined literals for time (base = ps)

consteval Time operator""_ps(unsigned long long v) {
  return Time{static_cast<uint64_t>(v)};
}

consteval Time operator""_ps(long double v) {
  return Time{static_cast<uint64_t>(v)};
}

// ns
consteval Time operator""_ns(unsigned long long v) {
  return Time{static_cast<uint64_t>(v) * 1000ull};
}

consteval Time operator""_ns(long double v) {
  return Time{static_cast<uint64_t>(v * 1.0e3L)};
}

// us
consteval Time operator""_us(unsigned long long v) {
  return Time{static_cast<uint64_t>(v) * 1'000'000ull};
}

consteval Time operator""_us(long double v) {
  return Time{static_cast<uint64_t>(v * 1.0e6L)};
}

// ms
consteval Time operator""_ms(unsigned long long v) {
  return Time{static_cast<uint64_t>(v) * 1'000'000'000ull};
}

consteval Time operator""_ms(long double v) {
  return Time{static_cast<uint64_t>(v * 1.0e9L)};
}

// s
consteval Time operator""_s(unsigned long long v) {
  return Time{static_cast<uint64_t>(v) * 1'000'000'000'000ull};
}

consteval Time operator""_s(long double v) {
  return Time{static_cast<uint64_t>(v * 1.0e12L)};
}

// minutes
consteval Time operator""_min(unsigned long long v) {
  return Time{static_cast<uint64_t>(v) * 60ull * 1'000'000'000'000ull};
}

consteval Time operator""_min(long double v) {
  return Time{static_cast<uint64_t>(v * 60.0L * 1.0e12L)};
}

// hours
consteval Time operator""_h(unsigned long long v) {
  return Time{static_cast<uint64_t>(v) * 3600ull * 1'000'000'000'000ull};
}

consteval Time operator""_h(long double v) {
  return Time{static_cast<uint64_t>(v * 3600.0L * 1.0e12L)};
}

} // namespace ggems::units
