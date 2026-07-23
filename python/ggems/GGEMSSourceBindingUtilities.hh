#pragma once

#include <cmath>
#include <cstdint>
#include <expected>
#include <string_view>

namespace ggems::python::detail {

enum class DistanceToPicometreError : std::uint8_t {
  NonFinite,
  UnsupportedUnit,
  OutOfRange
};

[[nodiscard]] inline auto
TryConvertDistanceToPicometre(double distance, std::string_view unit) noexcept
    -> std::expected<std::int64_t, DistanceToPicometreError> {
  if (!std::isfinite(distance)) {
    return std::unexpected(DistanceToPicometreError::NonFinite);
  }

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

  long double const distance_pm =
      static_cast<long double>(distance) * factor_to_pm;

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

} // namespace ggems::python::detail
