#pragma once

#include "GGEMSQuantity.hh"

/// \cond
#include <array>
/// \endcond

namespace ggems::units {
// base: bytes
using Bytes = Quantity<InfoBytesDim, std::uint64_t>;

inline std::string HumanReadable(Bytes const &b, std::int8_t precision = 7,
                                 std::int8_t width = -1) {
  long double const v = static_cast<long double>(b.value);

  struct Unit {
    long double threshold;
    std::string_view suffix;
    long double scale;
  };

  static constexpr std::array<Unit, 5> units{
      {{1099511627776.0L, " TB", 1099511627776.0L},
       {1073741824.0L, " GB", 1073741824.0L},
       {1048576.0L, " MB", 1048576.0L},
       {1024.0L, " kB", 1024.0L},
       {0.0L, " B", 1.0L}}};

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

  return std::format("{:.{}f} B", v, precision);
}

// B
consteval Bytes operator""_B(unsigned long long v) {
  return Bytes{static_cast<std::uint64_t>(v)};
}

consteval Bytes operator""_B(long double v) {
  return Bytes{static_cast<std::uint64_t>(v)};
}

// KB (base 1024)
consteval Bytes operator""_KB(unsigned long long v) {
  return Bytes{static_cast<std::uint64_t>(v) * 1024ull};
}

consteval Bytes operator""_KB(long double v) {
  return Bytes{static_cast<std::uint64_t>(v * 1024.0L)};
}

// MB
consteval Bytes operator""_MB(unsigned long long v) {
  return Bytes{static_cast<std::uint64_t>(v) * 1024ull * 1024ull};
}

consteval Bytes operator""_MB(long double v) {
  return Bytes{static_cast<std::uint64_t>(v * 1024.0L * 1024.0L)};
}

// GB
consteval Bytes operator""_GB(unsigned long long v) {
  return Bytes{static_cast<std::uint64_t>(v) * 1024ull * 1024ull * 1024ull};
}

consteval Bytes operator""_GB(long double v) {
  return Bytes{static_cast<std::uint64_t>(v * 1024.0L * 1024.0L * 1024.0L)};
}

constexpr std::size_t ToSizeT(Bytes const &b) noexcept {
  return static_cast<std::size_t>(b.value);
}

} // namespace ggems::units
