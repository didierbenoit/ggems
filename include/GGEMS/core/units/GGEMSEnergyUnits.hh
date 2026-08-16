#pragma once

#include <array>
#include <cstdint>
#include <string_view>

#include "GGEMS/core/units/GGEMSQuantity.hh"
#include "GGEMS/core/units/GGEMSUnitConversion.hh"

namespace ggems::units {

struct EnergyUnitSet {};

template <> struct UnitRegistry<EnergyUnitSet> {
  static constexpr std::array<UnitDefinition, 6U> units{{
      {.symbol = "meV",
       .scale = DecimalScale(0)},
      {.symbol = "eV",
       .scale = DecimalScale(3)},
      {.symbol = "keV",
       .scale = DecimalScale(6)},
      {.symbol = "MeV",
       .scale = DecimalScale(9)},
      {.symbol = "GeV",
       .scale = DecimalScale(12)},
      {.symbol = "TeV",
       .scale = DecimalScale(15)},
  }};
};

struct EnergyTag {};

template <> struct QuantityTraits<EnergyTag> {
  using unit_set = EnergyUnitSet;
  static constexpr QuantityDomain domain{QuantityDomain::NonNegative};
  static constexpr QuantityFormatPolicy format_policy{
      QuantityFormatPolicy::AutomaticScale};
  static constexpr std::string_view fixed_display_unit{};
  static constexpr std::int8_t default_precision{7};
};

struct EnergyChangeTag {};

template <> struct QuantityTraits<EnergyChangeTag> {
  using unit_set = EnergyUnitSet;
  static constexpr QuantityDomain domain{QuantityDomain::Signed};
  static constexpr QuantityFormatPolicy format_policy{
      QuantityFormatPolicy::AutomaticScale};
  static constexpr std::string_view fixed_display_unit{};
  static constexpr std::int8_t default_precision{7};
};

using Energy = Quantity<EnergyTag, std::uint64_t>;
using EnergyChange = Quantity<EnergyChangeTag, std::int64_t>;

static_assert(ValidateUnitSet<EnergyUnitSet>());
static_assert(ValidateQuantityTraits<EnergyTag, std::uint64_t>());
static_assert(ValidateQuantityTraits<EnergyChangeTag, std::int64_t>());

consteval auto operator""_meV(unsigned long long value) -> Energy {
  return detail::MakeLiteralQuantity<Energy>(value, "meV");
}

consteval auto operator""_meV(long double value) -> Energy {
  return detail::MakeLiteralQuantity<Energy>(value, "meV");
}

consteval auto operator""_eV(unsigned long long value) -> Energy {
  return detail::MakeLiteralQuantity<Energy>(value, "eV");
}

consteval auto operator""_eV(long double value) -> Energy {
  return detail::MakeLiteralQuantity<Energy>(value, "eV");
}

consteval auto operator""_keV(unsigned long long value) -> Energy {
  return detail::MakeLiteralQuantity<Energy>(value, "keV");
}

consteval auto operator""_keV(long double value) -> Energy {
  return detail::MakeLiteralQuantity<Energy>(value, "keV");
}

consteval auto operator""_MeV(unsigned long long value) -> Energy {
  return detail::MakeLiteralQuantity<Energy>(value, "MeV");
}

consteval auto operator""_MeV(long double value) -> Energy {
  return detail::MakeLiteralQuantity<Energy>(value, "MeV");
}

consteval auto operator""_GeV(unsigned long long value) -> Energy {
  return detail::MakeLiteralQuantity<Energy>(value, "GeV");
}

consteval auto operator""_GeV(long double value) -> Energy {
  return detail::MakeLiteralQuantity<Energy>(value, "GeV");
}

consteval auto operator""_TeV(unsigned long long value) -> Energy {
  return detail::MakeLiteralQuantity<Energy>(value, "TeV");
}

consteval auto operator""_TeV(long double value) -> Energy {
  return detail::MakeLiteralQuantity<Energy>(value, "TeV");
}
} // namespace ggems::units
