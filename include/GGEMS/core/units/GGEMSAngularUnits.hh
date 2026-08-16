#pragma once

#include <array>
#include <cstdint>
#include <numbers>
#include <string_view>

#include "GGEMS/core/units/GGEMSQuantity.hh"
#include "GGEMS/core/units/GGEMSUnitConversion.hh"

namespace ggems::units {

struct AngleUnitSet {};

template <> struct UnitRegistry<AngleUnitSet> {
  static constexpr std::array<UnitDefinition, 2U> units{{
      {.symbol = "rad",
       .scale = DecimalScale(0),
       .automatic_display = false},
      {.symbol = "deg",
       .scale = SpecialScale(std::numbers::pi_v<long double> / 180.0L)},
  }};
};

struct AngleTag {};

template <> struct QuantityTraits<AngleTag> {
  using unit_set = AngleUnitSet;
  static constexpr QuantityDomain domain{QuantityDomain::Signed};
  static constexpr QuantityFormatPolicy format_policy{
      QuantityFormatPolicy::FixedUnit};
  static constexpr std::string_view fixed_display_unit{"deg"};
  static constexpr std::int8_t default_precision{3};
};

using Angle = Quantity<AngleTag, long double>;

static_assert(ValidateUnitSet<AngleUnitSet>());
static_assert(ValidateQuantityTraits<AngleTag, long double>());

namespace detail {
inline constexpr long double k_pi{std::numbers::pi_v<long double>};
}

constexpr auto MakeRadians(long double radians) noexcept -> Angle {
  return Angle{radians};
}

constexpr auto MakeDegrees(long double degrees) noexcept -> Angle {
  return Angle{degrees *
               detail::ScaleFactor(FindUnit<AngleUnitSet>("deg")->scale)};
}

constexpr auto ToRadians(Angle angle) noexcept -> long double {
  return angle.value;
}

constexpr auto ToDegrees(Angle angle) noexcept -> long double {
  return angle.value /
         detail::ScaleFactor(FindUnit<AngleUnitSet>("deg")->scale);
}

consteval auto operator""_rad(long double value) -> Angle {
  return detail::MakeLiteralQuantity<Angle>(value, "rad");
}

consteval auto operator""_rad(unsigned long long value) -> Angle {
  return detail::MakeLiteralQuantity<Angle>(value, "rad");
}

consteval auto operator""_deg(long double value) -> Angle {
  return detail::MakeLiteralQuantity<Angle>(value, "deg");
}

consteval auto operator""_deg(unsigned long long value) -> Angle {
  return detail::MakeLiteralQuantity<Angle>(value, "deg");
}

} // namespace ggems::units
