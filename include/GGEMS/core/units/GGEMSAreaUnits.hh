#pragma once

#include <array>
#include <cstdint>
#include <string_view>

#include "GGEMS/core/units/GGEMSQuantity.hh"

namespace ggems::units {

struct AreaUnitSet {
  using dimension = AreaDim;
};

template <> struct UnitRegistry<AreaUnitSet> {
  static constexpr std::array<UnitDefinition, 7U> units{{
      {.symbol = "pm2",
       .scale = DecimalScale(0),
       .unicode_symbol = "pm²"},
      {.symbol = "nm2",
       .scale = DecimalScale(6),
       .unicode_symbol = "nm²"},
      {.symbol = "um2",
       .scale = DecimalScale(12),
       .unicode_symbol = "µm²"},
      {.symbol = "mm2",
       .scale = DecimalScale(18),
       .unicode_symbol = "mm²"},
      {.symbol = "cm2",
       .scale = DecimalScale(20),
       .unicode_symbol = "cm²",
       .automatic_display = false},
      {.symbol = "m2",
       .scale = DecimalScale(24),
       .unicode_symbol = "m²"},
      {.symbol = "km2",
       .scale = DecimalScale(30),
       .unicode_symbol = "km²"},
  }};
};

struct AreaFamily {
  using dimension = AreaDim;
  using unit_set = AreaUnitSet;
  using representation = long double;
  static constexpr std::string_view name{"Area"};
  static constexpr QuantityDomain domain{QuantityDomain::NonNegative};
  static constexpr QuantityFormatPolicy format_policy{
      QuantityFormatPolicy::AutomaticScale};
  static constexpr std::string_view fixed_display_unit{};
  static constexpr std::int8_t default_precision{7};
};

using Area = Quantity<AreaFamily>;

static_assert(ValidateUnitSet<AreaUnitSet>());
static_assert(ValidateFamily<AreaFamily>());

consteval auto operator""_pm2(unsigned long long value) -> Area {
  return detail::MakeLiteralQuantity<Area>(value, "pm2");
}

consteval auto operator""_pm2(long double value) -> Area {
  return detail::MakeLiteralQuantity<Area>(value, "pm2");
}

consteval auto operator""_nm2(unsigned long long value) -> Area {
  return detail::MakeLiteralQuantity<Area>(value, "nm2");
}

consteval auto operator""_nm2(long double value) -> Area {
  return detail::MakeLiteralQuantity<Area>(value, "nm2");
}

consteval auto operator""_um2(unsigned long long value) -> Area {
  return detail::MakeLiteralQuantity<Area>(value, "um2");
}

consteval auto operator""_um2(long double value) -> Area {
  return detail::MakeLiteralQuantity<Area>(value, "um2");
}

consteval auto operator""_mm2(unsigned long long value) -> Area {
  return detail::MakeLiteralQuantity<Area>(value, "mm2");
}

consteval auto operator""_mm2(long double value) -> Area {
  return detail::MakeLiteralQuantity<Area>(value, "mm2");
}

consteval auto operator""_cm2(unsigned long long value) -> Area {
  return detail::MakeLiteralQuantity<Area>(value, "cm2");
}

consteval auto operator""_cm2(long double value) -> Area {
  return detail::MakeLiteralQuantity<Area>(value, "cm2");
}

consteval auto operator""_m2(unsigned long long value) -> Area {
  return detail::MakeLiteralQuantity<Area>(value, "m2");
}

consteval auto operator""_m2(long double value) -> Area {
  return detail::MakeLiteralQuantity<Area>(value, "m2");
}

consteval auto operator""_km2(unsigned long long value) -> Area {
  return detail::MakeLiteralQuantity<Area>(value, "km2");
}

consteval auto operator""_km2(long double value) -> Area {
  return detail::MakeLiteralQuantity<Area>(value, "km2");
}
} // namespace ggems::units
