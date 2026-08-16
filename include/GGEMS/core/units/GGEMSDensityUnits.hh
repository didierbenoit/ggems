#pragma once

#include <array>
#include <cstdint>
#include <string_view>

#include "GGEMS/core/units/GGEMSQuantity.hh"
#include "GGEMS/core/units/GGEMSUnitConversion.hh"

namespace ggems::units {

struct DensityUnitSet {};

template <> struct UnitRegistry<DensityUnitSet> {
  static constexpr std::array<UnitDefinition, 2U> units{{
      {.symbol = "pg/pm3",
       .scale = DecimalScale(0),
       .unicode_symbol = "pg/pm³",
       .automatic_display = false},
      {.symbol = "g/cm3",
       .scale = DecimalScale(-18),
       .unicode_symbol = "g/cm³"},
  }};
};

struct DensityTag {};

template <> struct QuantityTraits<DensityTag> {
  using unit_set = DensityUnitSet;
  static constexpr QuantityDomain domain{QuantityDomain::NonNegative};
  static constexpr QuantityFormatPolicy format_policy{
      QuantityFormatPolicy::FixedUnit};
  static constexpr std::string_view fixed_display_unit{"g/cm3"};
  static constexpr std::int8_t default_precision{7};
};

using Density = Quantity<DensityTag, long double>;

static_assert(ValidateUnitSet<DensityUnitSet>());
static_assert(ValidateQuantityTraits<DensityTag, long double>());

consteval auto operator""_pg_pm3(unsigned long long value) -> Density {
  return detail::MakeLiteralQuantity<Density>(value, "pg/pm3");
}

consteval auto operator""_pg_pm3(long double value) -> Density {
  return detail::MakeLiteralQuantity<Density>(value, "pg/pm3");
}

consteval auto operator""_g_cm3(unsigned long long value) -> Density {
  return detail::MakeLiteralQuantity<Density>(value, "g/cm3");
}

consteval auto operator""_g_cm3(long double value) -> Density {
  return detail::MakeLiteralQuantity<Density>(value, "g/cm3");
}
} // namespace ggems::units
