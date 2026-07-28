#pragma once

#include <array>
#include <cmath>
#include <cstdint>
#include <expected>
#include <format>
#include <string>
#include <string_view>
#include <type_traits>

#include "GGEMS/core/units/GGEMSQuantity.hh"
#include "GGEMS/core/units/GGEMSFrequencyUnits.hh"

namespace ggems::units {

using Activity = Quantity<FrequencyDim, long double>;
static_assert(!std::is_same_v<Activity, Frequency>);

enum class ActivityConversionError : std::uint8_t {
  NonFinite,
  Negative,
  UnsupportedUnit,
  OutOfRange
};

[[nodiscard]] inline auto
TryConvertActivityToBecquerel(long double activity,
                              std::string_view unit) noexcept
    -> std::expected<Activity, ActivityConversionError> {
  if (!std::isfinite(activity)) {
    return std::unexpected(ActivityConversionError::NonFinite);
  }

  if (activity < 0.0L) {
    return std::unexpected(ActivityConversionError::Negative);
  }

  long double factor_to_becquerel{0.0L};

  if (unit == "Bq") {
    factor_to_becquerel = 1.0L;
  } else if (unit == "kBq") {
    factor_to_becquerel = 1.0e3L;
  } else if (unit == "MBq") {
    factor_to_becquerel = 1.0e6L;
  } else if (unit == "GBq") {
    factor_to_becquerel = 1.0e9L;
  } else if (unit == "TBq") {
    factor_to_becquerel = 1.0e12L;
  } else if (unit == "Ci") {
    factor_to_becquerel = 37'000'000'000.0L;
  } else if (unit == "mCi") {
    factor_to_becquerel = 37'000'000.0L;
  } else if (unit == "uCi" || unit == "\xC2\xB5"
                                      "Ci") {
    factor_to_becquerel = 37'000.0L;
  } else {
    return std::unexpected(ActivityConversionError::UnsupportedUnit);
  }

  long double const converted = activity * factor_to_becquerel;

  if (!std::isfinite(converted)) {
    return std::unexpected(ActivityConversionError::OutOfRange);
  }

  return Activity{converted};
}

[[nodiscard]] inline auto HumanReadable(Activity const &activity,
                                        std::int8_t precision = 7,
                                        std::int8_t width = -1) -> std::string {
  struct Scale {
    long double threshold;
    std::string_view suffix;
    long double scale;
  };

  static constexpr std::array<Scale, 5> units{
      {{.threshold = 1.0e12L, .suffix = " TBq", .scale = 1.0e12L},
       {.threshold = 1.0e9L, .suffix = " GBq", .scale = 1.0e9L},
       {.threshold = 1.0e6L, .suffix = " MBq", .scale = 1.0e6L},
       {.threshold = 1.0e3L, .suffix = " kBq", .scale = 1.0e3L},
       {.threshold = 0.0L, .suffix = " Bq", .scale = 1.0L}}};

  for (auto const &unit : units) {
    if (activity.value >= unit.threshold) {
      long double const scaled = activity.value / unit.scale;
      std::string format;

      if (width < 0) {
        format = std::format("{{:.{}f}}{}", precision, unit.suffix);
      } else {
        format = std::format("{{:{}.{}f}}{}", width, precision, unit.suffix);
      }

      return std::vformat(format, std::make_format_args(scaled));
    }
  }

  return std::format("{:.{}f} Bq", activity.value, precision);
}

consteval auto operator""_Bq(unsigned long long value) noexcept -> Activity {
  return Activity{static_cast<long double>(value)};
}

consteval auto operator""_Bq(long double value) noexcept -> Activity {
  return Activity{value};
}

consteval auto operator""_kBq(unsigned long long value) noexcept -> Activity {
  return Activity{static_cast<long double>(value) * 1.0e3L};
}

consteval auto operator""_kBq(long double value) noexcept -> Activity {
  return Activity{value * 1.0e3L};
}

consteval auto operator""_MBq(unsigned long long value) noexcept -> Activity {
  return Activity{static_cast<long double>(value) * 1.0e6L};
}

consteval auto operator""_MBq(long double value) noexcept -> Activity {
  return Activity{value * 1.0e6L};
}

consteval auto operator""_GBq(unsigned long long value) noexcept -> Activity {
  return Activity{static_cast<long double>(value) * 1.0e9L};
}

consteval auto operator""_GBq(long double value) noexcept -> Activity {
  return Activity{value * 1.0e9L};
}

consteval auto operator""_TBq(unsigned long long value) noexcept -> Activity {
  return Activity{static_cast<long double>(value) * 1.0e12L};
}

consteval auto operator""_TBq(long double value) noexcept -> Activity {
  return Activity{value * 1.0e12L};
}

consteval auto operator""_Ci(unsigned long long value) noexcept -> Activity {
  return Activity{static_cast<long double>(value) * 37'000'000'000.0L};
}

consteval auto operator""_Ci(long double value) noexcept -> Activity {
  return Activity{value * 37'000'000'000.0L};
}

consteval auto operator""_mCi(unsigned long long value) noexcept -> Activity {
  return Activity{static_cast<long double>(value) * 37'000'000.0L};
}

consteval auto operator""_mCi(long double value) noexcept -> Activity {
  return Activity{value * 37'000'000.0L};
}

consteval auto operator""_uCi(unsigned long long value) noexcept -> Activity {
  return Activity{static_cast<long double>(value) * 37'000.0L};
}

consteval auto operator""_uCi(long double value) noexcept -> Activity {
  return Activity{value * 37'000.0L};
}
} // namespace ggems::units
