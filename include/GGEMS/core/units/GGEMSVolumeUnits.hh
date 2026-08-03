#pragma once

#include <array>
#include <cstdint>
#include <string_view>

#include "GGEMS/core/units/GGEMSQuantity.hh"

namespace ggems::units {

struct VolumeUnitSet {
  using dimension = VolumeDim;
};

template <> struct UnitRegistry<VolumeUnitSet> {
  static constexpr std::array<UnitDefinition, 7U> units{{
      {.canonical_name = "CubicPicometer",
       .symbol = "pm3",
       .display_symbol = "pm³",
       .aliases = {"pm³"},
       .literal_suffix = "_pm3",
       .scale = DecimalScale(0),
       .canonical = true,
       .automatic_display = true},
      {.canonical_name = "CubicNanometer",
       .symbol = "nm3",
       .display_symbol = "nm³",
       .aliases = {"nm³"},
       .literal_suffix = "_nm3",
       .scale = DecimalScale(9),
       .automatic_display = true},
      {.canonical_name = "CubicMicrometer",
       .symbol = "um3",
       .display_symbol = "µm³",
       .aliases = {"µm3", "μm3", "um³", "µm³", "μm³"},
       .literal_suffix = "_um3",
       .scale = DecimalScale(18),
       .automatic_display = true},
      {.canonical_name = "CubicMillimeter",
       .symbol = "mm3",
       .display_symbol = "mm³",
       .aliases = {"mm³"},
       .literal_suffix = "_mm3",
       .scale = DecimalScale(27),
       .automatic_display = true},
      {.canonical_name = "CubicCentimeter",
       .symbol = "cm3",
       .display_symbol = "cm³",
       .aliases = {"cm³"},
       .literal_suffix = "_cm3",
       .scale = DecimalScale(30),
       .automatic_display = false},
      {.canonical_name = "CubicMeter",
       .symbol = "m3",
       .display_symbol = "m³",
       .aliases = {"m³"},
       .literal_suffix = "_m3",
       .scale = DecimalScale(36),
       .automatic_display = true},
      {.canonical_name = "CubicKilometer",
       .symbol = "km3",
       .display_symbol = "km³",
       .aliases = {"km³"},
       .literal_suffix = "_km3",
       .scale = DecimalScale(45),
       .automatic_display = true},
  }};
};

struct VolumeFamily {
  using dimension = VolumeDim;
  using unit_set = VolumeUnitSet;
  using representation = long double;
  static constexpr std::string_view name{"Volume"};
  static constexpr QuantityDomain domain{QuantityDomain::NonNegative};
  static constexpr QuantityFormatPolicy format_policy{
      QuantityFormatPolicy::AutomaticScale};
  static constexpr std::string_view fixed_display_unit{};
  static constexpr std::int8_t default_precision{7};
};

using Volume = Quantity<VolumeFamily>;

static_assert(ValidateUnitSet<VolumeUnitSet>());
static_assert(ValidateFamily<VolumeFamily>());

consteval auto operator""_pm3(unsigned long long value) -> Volume {
  return MakeQuantity<Volume>(value, "pm3");
}

consteval auto operator""_pm3(long double value) -> Volume {
  return MakeQuantity<Volume>(value, "pm3");
}

consteval auto operator""_nm3(unsigned long long value) -> Volume {
  return MakeQuantity<Volume>(value, "nm3");
}

consteval auto operator""_nm3(long double value) -> Volume {
  return MakeQuantity<Volume>(value, "nm3");
}

consteval auto operator""_um3(unsigned long long value) -> Volume {
  return MakeQuantity<Volume>(value, "um3");
}

consteval auto operator""_um3(long double value) -> Volume {
  return MakeQuantity<Volume>(value, "um3");
}

consteval auto operator""_mm3(unsigned long long value) -> Volume {
  return MakeQuantity<Volume>(value, "mm3");
}

consteval auto operator""_mm3(long double value) -> Volume {
  return MakeQuantity<Volume>(value, "mm3");
}

consteval auto operator""_cm3(unsigned long long value) -> Volume {
  return MakeQuantity<Volume>(value, "cm3");
}

consteval auto operator""_cm3(long double value) -> Volume {
  return MakeQuantity<Volume>(value, "cm3");
}

consteval auto operator""_m3(unsigned long long value) -> Volume {
  return MakeQuantity<Volume>(value, "m3");
}

consteval auto operator""_m3(long double value) -> Volume {
  return MakeQuantity<Volume>(value, "m3");
}

consteval auto operator""_km3(unsigned long long value) -> Volume {
  return MakeQuantity<Volume>(value, "km3");
}

consteval auto operator""_km3(long double value) -> Volume {
  return MakeQuantity<Volume>(value, "km3");
}
} // namespace ggems::units
