#pragma once

#include <cmath>
#include <cstdint>
#include <expected>
#include <string_view>

namespace ggems::python::detail {

// =============================================================================
// =============================================================================

enum class DistanceToPicometreError : std::uint8_t {
  NonFinite,
  NonPositive,
  UnsupportedUnit,
  OutOfRange
};

// =============================================================================
// =============================================================================

[[nodiscard]] inline auto
TryGetDistanceFactorToPicometre(std::string_view unit) noexcept
    -> std::expected<long double, DistanceToPicometreError> {
  long double factor_to_pm{0.0L};

  if (unit == "pm") {
    factor_to_pm = 1.0L;
  } else if (unit == "nm") {
    factor_to_pm = 1.0e3L;
  } else if (unit == "um") {
    factor_to_pm = 1.0e6L;
  } else if (unit == "mm") {
    factor_to_pm = 1.0e9L;
  } else if (unit == "cm") {
    factor_to_pm = 1.0e10L;
  } else if (unit == "m") {
    factor_to_pm = 1.0e12L;
  } else {
    return std::unexpected(DistanceToPicometreError::UnsupportedUnit);
  }

  return factor_to_pm;
}

// =============================================================================
// =============================================================================

[[nodiscard]] inline auto
TryConvertDistanceToPicometre(double distance, std::string_view unit) noexcept
    -> std::expected<std::int64_t, DistanceToPicometreError> {
  if (!std::isfinite(distance)) {
    return std::unexpected(DistanceToPicometreError::NonFinite);
  }

  auto const factor = TryGetDistanceFactorToPicometre(unit);

  if (!factor.has_value()) {
    return std::unexpected(factor.error());
  }

  long double const distance_pm = static_cast<long double>(distance) * *factor;

  if (!std::isfinite(distance_pm)) {
    return std::unexpected(DistanceToPicometreError::OutOfRange);
  }

  long double const rounded_pm = std::round(distance_pm);

  // These powers of two remain exact when long double has double precision.
  constexpr long double k_int64_lower_inclusive{-9'223'372'036'854'775'808.0L};
  constexpr long double k_int64_upper_exclusive{9'223'372'036'854'775'808.0L};

  if (rounded_pm < k_int64_lower_inclusive ||
      rounded_pm >= k_int64_upper_exclusive) {
    return std::unexpected(DistanceToPicometreError::OutOfRange);
  }

  return static_cast<std::int64_t>(rounded_pm);
}

// =============================================================================
// =============================================================================

[[nodiscard]] inline auto
TryConvertPositiveDistanceToPicometre(double distance,
                                      std::string_view unit) noexcept
    -> std::expected<std::uint64_t, DistanceToPicometreError> {
  if (!std::isfinite(distance)) {
    return std::unexpected(DistanceToPicometreError::NonFinite);
  }

  if (distance <= 0.0) {
    return std::unexpected(DistanceToPicometreError::NonPositive);
  }

  auto const factor = TryGetDistanceFactorToPicometre(unit);

  if (!factor.has_value()) {
    return std::unexpected(factor.error());
  }

  long double const distance_pm = static_cast<long double>(distance) * *factor;

  if (!std::isfinite(distance_pm)) {
    return std::unexpected(DistanceToPicometreError::OutOfRange);
  }

  long double const rounded_pm = std::round(distance_pm);

  if (rounded_pm <= 0.0L) {
    return std::unexpected(DistanceToPicometreError::NonPositive);
  }

  long double const uint64_upper_exclusive = std::ldexp(1.0L, 64);

  if (rounded_pm >= uint64_upper_exclusive) {
    return std::unexpected(DistanceToPicometreError::OutOfRange);
  }

  return static_cast<std::uint64_t>(rounded_pm);
}
} // namespace ggems::python::detail
