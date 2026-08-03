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
      {.canonical_name = "Becquerel",
       .symbol = "Bq",
       .display_symbol = "Bq",
       .aliases = {},
       .literal_suffix = "_Bq",
       .scale = DecimalScale(0),
       .canonical = true,
       .automatic_display = true},
      {.canonical_name = "Kilobecquerel",
       .symbol = "kBq",
       .display_symbol = "kBq",
       .aliases = {},
       .literal_suffix = "_kBq",
       .scale = DecimalScale(3),
       .automatic_display = true},
      {.canonical_name = "Megabecquerel",
       .symbol = "MBq",
       .display_symbol = "MBq",
       .aliases = {},
       .literal_suffix = "_MBq",
       .scale = DecimalScale(6),
       .automatic_display = true},
      {.canonical_name = "Gigabecquerel",
       .symbol = "GBq",
       .display_symbol = "GBq",
       .aliases = {},
       .literal_suffix = "_GBq",
       .scale = DecimalScale(9),
       .automatic_display = true},
      {.canonical_name = "Terabecquerel",
       .symbol = "TBq",
       .display_symbol = "TBq",
       .aliases = {},
       .literal_suffix = "_TBq",
       .scale = DecimalScale(12),
       .automatic_display = true},
      {.canonical_name = "Curie",
       .symbol = "Ci",
       .display_symbol = "Ci",
       .aliases = {},
       .literal_suffix = "_Ci",
       .scale = DecimalScale(0, 37'000'000'000ULL),
       .automatic_display = false},
      {.canonical_name = "Millicurie",
       .symbol = "mCi",
       .display_symbol = "mCi",
       .aliases = {},
       .literal_suffix = "_mCi",
       .scale = DecimalScale(0, 37'000'000ULL),
       .automatic_display = false},
      {.canonical_name = "Microcurie",
       .symbol = "uCi",
       .display_symbol = "µCi",
       .aliases = {"µCi", "μCi"},
       .literal_suffix = "_uCi",
       .scale = DecimalScale(0, 37'000ULL),
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
