#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <string_view>

#include "GGEMS/core/units/GGEMSQuantity.hh"
#include "GGEMS/core/units/GGEMSUnitConversion.hh"
#include "GGEMS/core/units/GGEMSUnitFormatting.hh"

namespace ggems::units {

struct LengthUnitSet {};

template <> struct UnitRegistry<LengthUnitSet> {
  static constexpr std::array<UnitDefinition, 7U> units{{
      {.symbol = "pm",
       .scale = DecimalScale(0)},
      {.symbol = "nm",
       .scale = DecimalScale(3)},
      {.symbol = "um",
       .scale = DecimalScale(6),
       .unicode_symbol = "µm"},
      {.symbol = "mm",
       .scale = DecimalScale(9)},
      {.symbol = "cm",
       .scale = DecimalScale(10),
       .automatic_display = false},
      {.symbol = "m",
       .scale = DecimalScale(12)},
      {.symbol = "km",
       .scale = DecimalScale(15)},
  }};
};

struct LengthTag {};

template <> struct QuantityTraits<LengthTag> {
  using unit_set = LengthUnitSet;
  static constexpr QuantityDomain domain{QuantityDomain::NonNegative};
  static constexpr QuantityFormatPolicy format_policy{
      QuantityFormatPolicy::AutomaticScale};
  static constexpr std::string_view fixed_display_unit{};
  static constexpr std::int8_t default_precision{7};
};

struct PositionCoordinateTag {};

template <> struct QuantityTraits<PositionCoordinateTag> {
  using unit_set = LengthUnitSet;
  static constexpr QuantityDomain domain{QuantityDomain::Signed};
  static constexpr QuantityFormatPolicy format_policy{
      QuantityFormatPolicy::AutomaticScale};
  static constexpr std::string_view fixed_display_unit{};
  static constexpr std::int8_t default_precision{7};
};

struct DisplacementTag {};

template <> struct QuantityTraits<DisplacementTag> {
  using unit_set = LengthUnitSet;
  static constexpr QuantityDomain domain{QuantityDomain::Signed};
  static constexpr QuantityFormatPolicy format_policy{
      QuantityFormatPolicy::AutomaticScale};
  static constexpr std::string_view fixed_display_unit{};
  static constexpr std::int8_t default_precision{7};
};

using Length = Quantity<LengthTag, std::uint64_t>;
using PositionCoordinate = Quantity<PositionCoordinateTag, std::int64_t>;
using Displacement = Quantity<DisplacementTag, std::int64_t>;

static_assert(ValidateUnitSet<LengthUnitSet>());
static_assert(ValidateQuantityTraits<LengthTag, std::uint64_t>());
static_assert(ValidateQuantityTraits<PositionCoordinateTag, std::int64_t>());
static_assert(ValidateQuantityTraits<DisplacementTag, std::int64_t>());

[[nodiscard]] inline auto HumanReadableSignedLength(std::int64_t value_pm,
                                                    std::int8_t precision = 7,
                                                    std::int8_t width = -1)
    -> std::string {
  return HumanReadable(PositionCoordinate{value_pm}, precision, width);
}

consteval auto operator""_pm(unsigned long long value) -> Length {
  return detail::MakeLiteralQuantity<Length>(value, "pm");
}

consteval auto operator""_pm(long double value) -> Length {
  return detail::MakeLiteralQuantity<Length>(value, "pm");
}

consteval auto operator""_nm(unsigned long long value) -> Length {
  return detail::MakeLiteralQuantity<Length>(value, "nm");
}

consteval auto operator""_nm(long double value) -> Length {
  return detail::MakeLiteralQuantity<Length>(value, "nm");
}

consteval auto operator""_um(unsigned long long value) -> Length {
  return detail::MakeLiteralQuantity<Length>(value, "um");
}

consteval auto operator""_um(long double value) -> Length {
  return detail::MakeLiteralQuantity<Length>(value, "um");
}

consteval auto operator""_mm(unsigned long long value) -> Length {
  return detail::MakeLiteralQuantity<Length>(value, "mm");
}

consteval auto operator""_mm(long double value) -> Length {
  return detail::MakeLiteralQuantity<Length>(value, "mm");
}

consteval auto operator""_cm(unsigned long long value) -> Length {
  return detail::MakeLiteralQuantity<Length>(value, "cm");
}

consteval auto operator""_cm(long double value) -> Length {
  return detail::MakeLiteralQuantity<Length>(value, "cm");
}

consteval auto operator""_m(unsigned long long value) -> Length {
  return detail::MakeLiteralQuantity<Length>(value, "m");
}

consteval auto operator""_m(long double value) -> Length {
  return detail::MakeLiteralQuantity<Length>(value, "m");
}

consteval auto operator""_km(unsigned long long value) -> Length {
  return detail::MakeLiteralQuantity<Length>(value, "km");
}

consteval auto operator""_km(long double value) -> Length {
  return detail::MakeLiteralQuantity<Length>(value, "km");
}
} // namespace ggems::units
