#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <string_view>

#include "GGEMS/core/units/GGEMSQuantity.hh"

namespace ggems::units {

struct LengthUnitSet {
  using dimension = LengthDim;
};

template <> struct UnitRegistry<LengthUnitSet> {
  static constexpr std::array<UnitDefinition, 7U> units{{
      {.canonical_name = "Picometer",
       .symbol = "pm",
       .display_symbol = "pm",
       .aliases = {},
       .literal_suffix = "_pm",
       .scale = DecimalScale(0),
       .canonical = true,
       .automatic_display = true},
      {.canonical_name = "Nanometer",
       .symbol = "nm",
       .display_symbol = "nm",
       .aliases = {},
       .literal_suffix = "_nm",
       .scale = DecimalScale(3),
       .automatic_display = true},
      {.canonical_name = "Micrometer",
       .symbol = "um",
       .display_symbol = "µm",
       .aliases = {"µm", "μm"},
       .literal_suffix = "_um",
       .scale = DecimalScale(6),
       .automatic_display = true},
      {.canonical_name = "Millimeter",
       .symbol = "mm",
       .display_symbol = "mm",
       .aliases = {},
       .literal_suffix = "_mm",
       .scale = DecimalScale(9),
       .automatic_display = true},
      {.canonical_name = "Centimeter",
       .symbol = "cm",
       .display_symbol = "cm",
       .aliases = {},
       .literal_suffix = "_cm",
       .scale = DecimalScale(10),
       .automatic_display = false},
      {.canonical_name = "Meter",
       .symbol = "m",
       .display_symbol = "m",
       .aliases = {},
       .literal_suffix = "_m",
       .scale = DecimalScale(12),
       .automatic_display = true},
      {.canonical_name = "Kilometer",
       .symbol = "km",
       .display_symbol = "km",
       .aliases = {},
       .literal_suffix = "_km",
       .scale = DecimalScale(15),
       .automatic_display = true},
  }};
};

struct LengthFamily {
  using dimension = LengthDim;
  using unit_set = LengthUnitSet;
  using representation = std::uint64_t;
  static constexpr std::string_view name{"Length"};
  static constexpr QuantityDomain domain{QuantityDomain::NonNegative};
  static constexpr QuantityFormatPolicy format_policy{
      QuantityFormatPolicy::AutomaticScale};
  static constexpr std::string_view fixed_display_unit{};
  static constexpr std::int8_t default_precision{7};
};

struct PositionCoordinateFamily {
  using dimension = LengthDim;
  using unit_set = LengthUnitSet;
  using representation = std::int64_t;
  static constexpr std::string_view name{"PositionCoordinate"};
  static constexpr QuantityDomain domain{QuantityDomain::Signed};
  static constexpr QuantityFormatPolicy format_policy{
      QuantityFormatPolicy::AutomaticScale};
  static constexpr std::string_view fixed_display_unit{};
  static constexpr std::int8_t default_precision{7};
};

struct DisplacementFamily {
  using dimension = LengthDim;
  using unit_set = LengthUnitSet;
  using representation = std::int64_t;
  static constexpr std::string_view name{"Displacement"};
  static constexpr QuantityDomain domain{QuantityDomain::Signed};
  static constexpr QuantityFormatPolicy format_policy{
      QuantityFormatPolicy::AutomaticScale};
  static constexpr std::string_view fixed_display_unit{};
  static constexpr std::int8_t default_precision{7};
};

using Length = Quantity<LengthFamily>;
using PositionCoordinate = Quantity<PositionCoordinateFamily>;
using Displacement = Quantity<DisplacementFamily>;

static_assert(ValidateUnitSet<LengthUnitSet>());
static_assert(ValidateFamily<LengthFamily>());
static_assert(ValidateFamily<PositionCoordinateFamily>());
static_assert(ValidateFamily<DisplacementFamily>());

[[nodiscard]] inline auto HumanReadableSignedLength(std::int64_t value_pm,
                                                    std::int8_t precision = 7,
                                                    std::int8_t width = -1)
    -> std::string {
  return HumanReadable(PositionCoordinate{value_pm}, precision, width);
}

consteval auto operator""_pm(unsigned long long value) -> Length {
  return MakeQuantity<Length>(value, "pm");
}

consteval auto operator""_pm(long double value) -> Length {
  return MakeQuantity<Length>(value, "pm");
}

consteval auto operator""_nm(unsigned long long value) -> Length {
  return MakeQuantity<Length>(value, "nm");
}

consteval auto operator""_nm(long double value) -> Length {
  return MakeQuantity<Length>(value, "nm");
}

consteval auto operator""_um(unsigned long long value) -> Length {
  return MakeQuantity<Length>(value, "um");
}

consteval auto operator""_um(long double value) -> Length {
  return MakeQuantity<Length>(value, "um");
}

consteval auto operator""_mm(unsigned long long value) -> Length {
  return MakeQuantity<Length>(value, "mm");
}

consteval auto operator""_mm(long double value) -> Length {
  return MakeQuantity<Length>(value, "mm");
}

consteval auto operator""_cm(unsigned long long value) -> Length {
  return MakeQuantity<Length>(value, "cm");
}

consteval auto operator""_cm(long double value) -> Length {
  return MakeQuantity<Length>(value, "cm");
}

consteval auto operator""_m(unsigned long long value) -> Length {
  return MakeQuantity<Length>(value, "m");
}

consteval auto operator""_m(long double value) -> Length {
  return MakeQuantity<Length>(value, "m");
}

consteval auto operator""_km(unsigned long long value) -> Length {
  return MakeQuantity<Length>(value, "km");
}

consteval auto operator""_km(long double value) -> Length {
  return MakeQuantity<Length>(value, "km");
}
} // namespace ggems::units
