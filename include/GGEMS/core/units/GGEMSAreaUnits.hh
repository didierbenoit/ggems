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
      {.canonical_name = "SquarePicometer",
       .symbol = "pm2",
       .display_symbol = "pm²",
       .aliases = {"pm²"},
       .literal_suffix = "_pm2",
       .scale = DecimalScale(0),
       .canonical = true,
       .automatic_display = true},
      {.canonical_name = "SquareNanometer",
       .symbol = "nm2",
       .display_symbol = "nm²",
       .aliases = {"nm²"},
       .literal_suffix = "_nm2",
       .scale = DecimalScale(6),
       .automatic_display = true},
      {.canonical_name = "SquareMicrometer",
       .symbol = "um2",
       .display_symbol = "µm²",
       .aliases = {"µm2", "μm2", "um²", "µm²", "μm²"},
       .literal_suffix = "_um2",
       .scale = DecimalScale(12),
       .automatic_display = true},
      {.canonical_name = "SquareMillimeter",
       .symbol = "mm2",
       .display_symbol = "mm²",
       .aliases = {"mm²"},
       .literal_suffix = "_mm2",
       .scale = DecimalScale(18),
       .automatic_display = true},
      {.canonical_name = "SquareCentimeter",
       .symbol = "cm2",
       .display_symbol = "cm²",
       .aliases = {"cm²"},
       .literal_suffix = "_cm2",
       .scale = DecimalScale(20),
       .automatic_display = false},
      {.canonical_name = "SquareMeter",
       .symbol = "m2",
       .display_symbol = "m²",
       .aliases = {"m²"},
       .literal_suffix = "_m2",
       .scale = DecimalScale(24),
       .automatic_display = true},
      {.canonical_name = "SquareKilometer",
       .symbol = "km2",
       .display_symbol = "km²",
       .aliases = {"km²"},
       .literal_suffix = "_km2",
       .scale = DecimalScale(30),
       .automatic_display = true},
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
  return MakeQuantity<Area>(value, "pm2");
}

consteval auto operator""_pm2(long double value) -> Area {
  return MakeQuantity<Area>(value, "pm2");
}

consteval auto operator""_nm2(unsigned long long value) -> Area {
  return MakeQuantity<Area>(value, "nm2");
}

consteval auto operator""_nm2(long double value) -> Area {
  return MakeQuantity<Area>(value, "nm2");
}

consteval auto operator""_um2(unsigned long long value) -> Area {
  return MakeQuantity<Area>(value, "um2");
}

consteval auto operator""_um2(long double value) -> Area {
  return MakeQuantity<Area>(value, "um2");
}

consteval auto operator""_mm2(unsigned long long value) -> Area {
  return MakeQuantity<Area>(value, "mm2");
}

consteval auto operator""_mm2(long double value) -> Area {
  return MakeQuantity<Area>(value, "mm2");
}

consteval auto operator""_cm2(unsigned long long value) -> Area {
  return MakeQuantity<Area>(value, "cm2");
}

consteval auto operator""_cm2(long double value) -> Area {
  return MakeQuantity<Area>(value, "cm2");
}

consteval auto operator""_m2(unsigned long long value) -> Area {
  return MakeQuantity<Area>(value, "m2");
}

consteval auto operator""_m2(long double value) -> Area {
  return MakeQuantity<Area>(value, "m2");
}

consteval auto operator""_km2(unsigned long long value) -> Area {
  return MakeQuantity<Area>(value, "km2");
}

consteval auto operator""_km2(long double value) -> Area {
  return MakeQuantity<Area>(value, "km2");
}
} // namespace ggems::units
