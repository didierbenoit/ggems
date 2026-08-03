#pragma once

#include <array>
#include <cstdint>
#include <string_view>

#include "GGEMS/core/units/GGEMSQuantity.hh"

namespace ggems::units {

struct DensityUnitSet {
  using dimension = DensityDim;
};

template <> struct UnitRegistry<DensityUnitSet> {
  static constexpr std::array<UnitDefinition, 2U> units{{
      {.canonical_name = "PicogramPerCubicPicometer",
       .symbol = "pg/pm3",
       .display_symbol = "pg/pm³",
       .aliases = {"pg/pm³"},
       .literal_suffix = "_pg_pm3",
       .scale = DecimalScale(0),
       .canonical = true,
       .automatic_display = false},
      {.canonical_name = "GramPerCubicCentimeter",
       .symbol = "g/cm3",
       .display_symbol = "g/cm³",
       .aliases = {"g/cm³"},
       .literal_suffix = "_g_cm3",
       .scale = DecimalScale(-18),
       .automatic_display = true},
  }};
};

struct DensityFamily {
  using dimension = DensityDim;
  using unit_set = DensityUnitSet;
  using representation = long double;
  static constexpr std::string_view name{"Density"};
  static constexpr QuantityDomain domain{QuantityDomain::NonNegative};
  static constexpr QuantityFormatPolicy format_policy{
      QuantityFormatPolicy::FixedUnit};
  static constexpr std::string_view fixed_display_unit{"g/cm3"};
  static constexpr std::int8_t default_precision{7};
};

using Density = Quantity<DensityFamily>;

static_assert(ValidateUnitSet<DensityUnitSet>());
static_assert(ValidateFamily<DensityFamily>());

consteval auto operator""_pg_pm3(unsigned long long value) -> Density {
  return MakeQuantity<Density>(value, "pg/pm3");
}

consteval auto operator""_pg_pm3(long double value) -> Density {
  return MakeQuantity<Density>(value, "pg/pm3");
}

consteval auto operator""_g_cm3(unsigned long long value) -> Density {
  return MakeQuantity<Density>(value, "g/cm3");
}

consteval auto operator""_g_cm3(long double value) -> Density {
  return MakeQuantity<Density>(value, "g/cm3");
}
} // namespace ggems::units
