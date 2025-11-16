#pragma once

#include "GGEMSQuantity.hh"

namespace ggems::units {
using Time = Quantity<TimeDim, std::uint64_t>; // base: picoseconds

// Human-readable time
inline std::string HumanReadable(Time const &t) {
  long double const v = static_cast<long double>(t.value);

  // >= 60 s → h / min / s / ms
  if (v >= 60.0L * 1.0e12L) {
    std::uint64_t total_ps = t.value;

    std::uint64_t total_s = total_ps / 1'000'000'000'000ull;
    std::uint64_t ps_rest = total_ps % 1'000'000'000'000ull;

    std::uint64_t hours = total_s / 3600ull;
    std::uint64_t minutes = (total_s % 3600ull) / 60ull;
    std::uint64_t seconds = total_s % 60ull;

    std::uint64_t ms = ps_rest / 1'000'000'000ull; // 1 ms = 1e9 ps

    if (hours > 0) {
      return std::format("{} h {} min {} s {} ms", hours, minutes, seconds, ms);
    }

    return std::format("{} min {} s {} ms", minutes, seconds, ms);
  }

  if (v >= 1.0e12L) // s
    return std::format("{:.3} s", v / 1.0e12L);

  if (v >= 1.0e9L) // ms
    return std::format("{:.3} ms", v / 1.0e9L);

  if (v >= 1.0e6L) // us
    return std::format("{:.3} us", v / 1.0e6L);

  if (v >= 1.0e3L) // ns
    return std::format("{:.3} ns", v / 1.0e3L);

  return std::format("{} ps", v); // base
}

// User-defined literals for time (base = ps)

consteval Time operator""_ps(unsigned long long v) {
  return Time{static_cast<std::uint64_t>(v)};
}

consteval Time operator""_ps(long double v) {
  return Time{static_cast<std::uint64_t>(v)};
}

// ns
consteval Time operator""_ns(unsigned long long v) {
  return Time{static_cast<std::uint64_t>(v) * 1000ull};
}

consteval Time operator""_ns(long double v) {
  return Time{static_cast<std::uint64_t>(v * 1.0e3L)};
}

// us
consteval Time operator""_us(unsigned long long v) {
  return Time{static_cast<std::uint64_t>(v) * 1'000'000ull};
}

consteval Time operator""_us(long double v) {
  return Time{static_cast<std::uint64_t>(v * 1.0e6L)};
}

// ms
consteval Time operator""_ms(unsigned long long v) {
  return Time{static_cast<std::uint64_t>(v) * 1'000'000'000ull};
}

consteval Time operator""_ms(long double v) {
  return Time{static_cast<std::uint64_t>(v * 1.0e9L)};
}

// s
consteval Time operator""_s(unsigned long long v) {
  return Time{static_cast<std::uint64_t>(v) * 1'000'000'000'000ull};
}

consteval Time operator""_s(long double v) {
  return Time{static_cast<std::uint64_t>(v * 1.0e12L)};
}

// minutes
consteval Time operator""_min(unsigned long long v) {
  return Time{static_cast<std::uint64_t>(v) * 60ull * 1'000'000'000'000ull};
}

consteval Time operator""_min(long double v) {
  return Time{static_cast<std::uint64_t>(v * 60.0L * 1.0e12L)};
}

// hours
consteval Time operator""_h(unsigned long long v) {
  return Time{static_cast<std::uint64_t>(v) * 3600ull * 1'000'000'000'000ull};
}

consteval Time operator""_h(long double v) {
  return Time{static_cast<std::uint64_t>(v * 3600.0L * 1.0e12L)};
}

} // namespace ggems::units
