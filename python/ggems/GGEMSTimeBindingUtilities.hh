#pragma once

#include <cmath>
#include <cstdint>
#include <expected>
#include <string_view>

namespace ggems::python::detail {

enum class TimeConversionError : std::uint8_t {
  NonFinite,
  Negative,
  UnsupportedUnit,
  OutOfRange
};

// =============================================================================
// =============================================================================

[[nodiscard]] inline auto
TryGetTimeFactorToPicoSecond(std::string_view unit) noexcept
    -> std::expected<long double, TimeConversionError> {
  long double factor_to_ps{0.0L};

  if (unit == "ps") {
    factor_to_ps = 1.0L;
  } else if (unit == "ns") {
    factor_to_ps = 1.0e3L;
  } else if (unit == "us") {
    factor_to_ps = 1.0e6L;
  } else if (unit == "ms") {
    factor_to_ps = 1.0e9L;
  } else if (unit == "s") {
    factor_to_ps = 1.0e12L;
  } else if (unit == "min") {
    factor_to_ps = 60.0e12L;
  } else if (unit == "h") {
    factor_to_ps = 3'600.0e12L;
  } else {
    return std::unexpected(TimeConversionError::UnsupportedUnit);
  }

  return factor_to_ps;
}

// =============================================================================
// =============================================================================

[[nodiscard]] inline auto
TryConvertTimeToPicoSecond(double time, std::string_view unit) noexcept
    -> std::expected<std::uint64_t, TimeConversionError> {
  if (!std::isfinite(time)) {
    return std::unexpected(TimeConversionError::NonFinite);
  }

  if (time < 0.0) {
    return std::unexpected(TimeConversionError::Negative);
  }

  auto const factor = TryGetTimeFactorToPicoSecond(unit);

  if (!factor.has_value()) {
    return std::unexpected(factor.error());
  }

  long double const time_ps = static_cast<long double>(time) * *factor;

  if (!std::isfinite(time_ps)) {
    return std::unexpected(TimeConversionError::OutOfRange);
  }

  long double const rounded_time_ps = std::round(time_ps);
  constexpr long double k_uint64_upper_exclusive{18'446'744'073'709'551'616.0L};

  if (rounded_time_ps >= k_uint64_upper_exclusive) {
    return std::unexpected(TimeConversionError::OutOfRange);
  }

  return static_cast<std::uint64_t>(rounded_time_ps);
}

// =============================================================================
// =============================================================================

[[nodiscard]] inline auto
TryConvertPicoSecondToTime(std::uint64_t time_ps,
                           std::string_view unit) noexcept
    -> std::expected<double, TimeConversionError> {
  auto const factor = TryGetTimeFactorToPicoSecond(unit);

  if (!factor.has_value()) {
    return std::unexpected(factor.error());
  }

  long double const converted = static_cast<long double>(time_ps) / *factor;
  return static_cast<double>(converted);
}

} // namespace ggems::python::detail
