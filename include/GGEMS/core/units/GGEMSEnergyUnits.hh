#pragma once

/// \cond
#include <array>
#include <cmath>
#include <string>
#include <format>
#include <cstdint>
#include <expected>
#include <string_view>
/// \endcond

#include "GGEMS/core/units/GGEMSQuantity.hh"

namespace ggems::units {

using Energy = Quantity<EnergyDim, std::uint64_t>;

enum class EnergyConversionError : std::uint8_t {
  NonFinite,
  NonPositive,
  UnsupportedUnit,
  OutOfRange
};

[[nodiscard]] inline auto
TryConvertEnergyToMilliElectronVolt(double energy,
                                    std::string_view unit) noexcept
    -> std::expected<std::uint64_t, EnergyConversionError> {
  if (!std::isfinite(energy)) {
    return std::unexpected(EnergyConversionError::NonFinite);
  }

  if (energy <= 0.0) {
    return std::unexpected(EnergyConversionError::NonPositive);
  }

  long double factor_to_milli_eV{0.0L};

  if (unit == "milli_eV" || unit == "meV") {
    factor_to_milli_eV = 1.0L;
  } else if (unit == "eV") {
    factor_to_milli_eV = 1.0e3L;
  } else if (unit == "keV") {
    factor_to_milli_eV = 1.0e6L;
  } else if (unit == "MeV") {
    factor_to_milli_eV = 1.0e9L;
  } else if (unit == "GeV") {
    factor_to_milli_eV = 1.0e12L;
  } else {
    return std::unexpected(EnergyConversionError::UnsupportedUnit);
  }

  long double const converted =
      static_cast<long double>(energy) * factor_to_milli_eV;

  if (!std::isfinite(converted)) {
    return std::unexpected(EnergyConversionError::OutOfRange);
  }

  long double const rounded = std::round(converted);

  if (rounded <= 0.0L) {
    return std::unexpected(EnergyConversionError::NonPositive);
  }

  long double const uint64_upper_exclusive = std::ldexp(1.0L, 64);

  if (rounded >= uint64_upper_exclusive) {
    return std::unexpected(EnergyConversionError::OutOfRange);
  }

  return static_cast<std::uint64_t>(rounded);
}

inline auto HumanReadable(Energy const &energy, std::int8_t precision = 7,
                          std::int8_t width = -1) -> std::string {
  auto value = static_cast<long double>(energy.value);

  struct Scale {
    long double threshold;
    std::string_view suffix;
    long double scale;
  };

  static constexpr std::array<Scale, 6> units{
      {{.threshold = 1.0e15L, .suffix = " TeV", .scale = 1.0e15L},
       {.threshold = 1.0e12L, .suffix = " GeV", .scale = 1.0e12L},
       {.threshold = 1.0e9L, .suffix = " MeV", .scale = 1.0e9L},
       {.threshold = 1.0e6L, .suffix = " keV", .scale = 1.0e6L},
       {.threshold = 1.0e3L, .suffix = " eV", .scale = 1.0e3L},
       {.threshold = 0.0L, .suffix = " meV", .scale = 1.0L}}};

  for (auto const &unit : units) {
    if (value >= unit.threshold) {

      long double scaled = value / unit.scale;

      std::string fmt;
      if (width < 0) {
        fmt = std::format("{{:.{}f}}{}", precision, unit.suffix);
      } else {
        fmt = std::format("{{:{}.{}f}}{}", width, precision, unit.suffix);
      }

      return std::vformat(fmt, std::make_format_args(scaled));
    }
  }

  return std::format("{:.{}f} meV", value, precision);
}

consteval Energy operator""_meV(unsigned long long v) noexcept {
  return Energy{v};
}

consteval Energy operator""_meV(long double v) noexcept {
  return Energy{static_cast<std::uint64_t>(v)};
}

consteval Energy operator""_eV(unsigned long long v) noexcept {
  return Energy{static_cast<std::uint64_t>(v) * 1000ULL};
}

consteval Energy operator""_eV(long double v) noexcept {
  return Energy{static_cast<std::uint64_t>(v * 1.0e3L)};
}

consteval Energy operator""_keV(unsigned long long v) noexcept {
  return Energy{static_cast<std::uint64_t>(v) * 1'000'000ULL};
}

consteval Energy operator""_keV(long double v) noexcept {
  return Energy{static_cast<std::uint64_t>(v * 1.0e6L)};
}

consteval Energy operator""_MeV(unsigned long long v) noexcept {
  return Energy{static_cast<std::uint64_t>(v) * 1'000'000'000ULL};
}

consteval Energy operator""_MeV(long double v) noexcept {
  return Energy{static_cast<std::uint64_t>(v * 1.0e9L)};
}

consteval Energy operator""_GeV(unsigned long long v) noexcept {
  return Energy{static_cast<std::uint64_t>(v) * 1'000'000'000'000ULL};
}

consteval Energy operator""_GeV(long double v) noexcept {
  return Energy{static_cast<std::uint64_t>(v * 1.0e12L)};
}

consteval Energy operator""_TeV(unsigned long long v) noexcept {
  return Energy{static_cast<std::uint64_t>(v) * 1'000'000'000'000'000ULL};
}

consteval Energy operator""_TeV(long double v) noexcept {
  return Energy{static_cast<std::uint64_t>(v * 1.0e15L)};
}
} // namespace ggems::units
