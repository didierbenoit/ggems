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
      {.symbol = "pm3",
       .scale = DecimalScale(0),
       .unicode_symbol = "pm³"},
      {.symbol = "nm3",
       .scale = DecimalScale(9),
       .unicode_symbol = "nm³"},
      {.symbol = "um3",
       .scale = DecimalScale(18),
       .unicode_symbol = "µm³"},
      {.symbol = "mm3",
       .scale = DecimalScale(27),
       .unicode_symbol = "mm³"},
      {.symbol = "cm3",
       .scale = DecimalScale(30),
       .unicode_symbol = "cm³",
       .automatic_display = false},
      {.symbol = "m3",
       .scale = DecimalScale(36),
       .unicode_symbol = "m³"},
      {.symbol = "km3",
       .scale = DecimalScale(45),
       .unicode_symbol = "km³"},
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
