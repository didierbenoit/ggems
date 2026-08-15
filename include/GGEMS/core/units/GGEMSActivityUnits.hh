#pragma once

#include <array>
#include <cstdint>
#include <string_view>

#include "GGEMS/core/units/GGEMSQuantity.hh"

namespace ggems::units {

struct ActivityUnitSet {
  using dimension = FrequencyDim;
};

template <> struct UnitRegistry<ActivityUnitSet> {
  static constexpr std::array<UnitDefinition, 8U> units{{
      {.symbol = "Bq",
       .scale = DecimalScale(0)},
      {.symbol = "kBq",
       .scale = DecimalScale(3)},
      {.symbol = "MBq",
       .scale = DecimalScale(6)},
      {.symbol = "GBq",
       .scale = DecimalScale(9)},
      {.symbol = "TBq",
       .scale = DecimalScale(12)},
      {.symbol = "Ci",
       .scale = DecimalScale(0, 37'000'000'000ULL),
       .automatic_display = false},
      {.symbol = "mCi",
       .scale = DecimalScale(0, 37'000'000ULL),
       .automatic_display = false},
      {.symbol = "uCi",
       .scale = DecimalScale(0, 37'000ULL),
       .unicode_symbol = "µCi",
       .automatic_display = false},
  }};
};

struct ActivityFamily {
  using dimension = FrequencyDim;
  using unit_set = ActivityUnitSet;
  using representation = long double;
  static constexpr std::string_view name{"Activity"};
  static constexpr QuantityDomain domain{QuantityDomain::NonNegative};
  static constexpr QuantityFormatPolicy format_policy{
      QuantityFormatPolicy::AutomaticScale};
  static constexpr std::string_view fixed_display_unit{};
  static constexpr std::int8_t default_precision{7};
};

using Activity = Quantity<ActivityFamily>;

static_assert(ValidateUnitSet<ActivityUnitSet>());
static_assert(ValidateFamily<ActivityFamily>());

consteval auto operator""_Bq(unsigned long long value) -> Activity {
  return MakeQuantity<Activity>(value, "Bq");
}

consteval auto operator""_Bq(long double value) -> Activity {
  return MakeQuantity<Activity>(value, "Bq");
}

consteval auto operator""_kBq(unsigned long long value) -> Activity {
  return MakeQuantity<Activity>(value, "kBq");
}

consteval auto operator""_kBq(long double value) -> Activity {
  return MakeQuantity<Activity>(value, "kBq");
}

consteval auto operator""_MBq(unsigned long long value) -> Activity {
  return MakeQuantity<Activity>(value, "MBq");
}

consteval auto operator""_MBq(long double value) -> Activity {
  return MakeQuantity<Activity>(value, "MBq");
}

consteval auto operator""_GBq(unsigned long long value) -> Activity {
  return MakeQuantity<Activity>(value, "GBq");
}

consteval auto operator""_GBq(long double value) -> Activity {
  return MakeQuantity<Activity>(value, "GBq");
}

consteval auto operator""_TBq(unsigned long long value) -> Activity {
  return MakeQuantity<Activity>(value, "TBq");
}

consteval auto operator""_TBq(long double value) -> Activity {
  return MakeQuantity<Activity>(value, "TBq");
}

consteval auto operator""_Ci(unsigned long long value) -> Activity {
  return MakeQuantity<Activity>(value, "Ci");
}

consteval auto operator""_Ci(long double value) -> Activity {
  return MakeQuantity<Activity>(value, "Ci");
}

consteval auto operator""_mCi(unsigned long long value) -> Activity {
  return MakeQuantity<Activity>(value, "mCi");
}

consteval auto operator""_mCi(long double value) -> Activity {
  return MakeQuantity<Activity>(value, "mCi");
}

consteval auto operator""_uCi(unsigned long long value) -> Activity {
  return MakeQuantity<Activity>(value, "uCi");
}

consteval auto operator""_uCi(long double value) -> Activity {
  return MakeQuantity<Activity>(value, "uCi");
}
} // namespace ggems::units
