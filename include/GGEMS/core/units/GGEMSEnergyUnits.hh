#pragma once

#include <array>
#include <cstdint>
#include <string_view>

#include "GGEMS/core/units/GGEMSQuantity.hh"

namespace ggems::units {

struct EnergyUnitSet {
  using dimension = EnergyDim;
};

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

struct EnergyFamily {
  using dimension = EnergyDim;
  using unit_set = EnergyUnitSet;
  using representation = std::uint64_t;
  static constexpr std::string_view name{"Energy"};
  static constexpr QuantityDomain domain{QuantityDomain::NonNegative};
  static constexpr QuantityFormatPolicy format_policy{
      QuantityFormatPolicy::AutomaticScale};
  static constexpr std::string_view fixed_display_unit{};
  static constexpr std::int8_t default_precision{7};
};

struct EnergyChangeFamily {
  using dimension = EnergyDim;
  using unit_set = EnergyUnitSet;
  using representation = std::int64_t;
  static constexpr std::string_view name{"EnergyChange"};
  static constexpr QuantityDomain domain{QuantityDomain::Signed};
  static constexpr QuantityFormatPolicy format_policy{
      QuantityFormatPolicy::AutomaticScale};
  static constexpr std::string_view fixed_display_unit{};
  static constexpr std::int8_t default_precision{7};
};

using Energy = Quantity<EnergyFamily>;
using EnergyChange = Quantity<EnergyChangeFamily>;

static_assert(ValidateUnitSet<EnergyUnitSet>());
static_assert(ValidateFamily<EnergyFamily>());
static_assert(ValidateFamily<EnergyChangeFamily>());

consteval auto operator""_meV(unsigned long long value) -> Energy {
  return MakeQuantity<Energy>(value, "meV");
}

consteval auto operator""_meV(long double value) -> Energy {
  return MakeQuantity<Energy>(value, "meV");
}

consteval auto operator""_eV(unsigned long long value) -> Energy {
  return MakeQuantity<Energy>(value, "eV");
}

consteval auto operator""_eV(long double value) -> Energy {
  return MakeQuantity<Energy>(value, "eV");
}

consteval auto operator""_keV(unsigned long long value) -> Energy {
  return MakeQuantity<Energy>(value, "keV");
}

consteval auto operator""_keV(long double value) -> Energy {
  return MakeQuantity<Energy>(value, "keV");
}

consteval auto operator""_MeV(unsigned long long value) -> Energy {
  return MakeQuantity<Energy>(value, "MeV");
}

consteval auto operator""_MeV(long double value) -> Energy {
  return MakeQuantity<Energy>(value, "MeV");
}

consteval auto operator""_GeV(unsigned long long value) -> Energy {
  return MakeQuantity<Energy>(value, "GeV");
}

consteval auto operator""_GeV(long double value) -> Energy {
  return MakeQuantity<Energy>(value, "GeV");
}

consteval auto operator""_TeV(unsigned long long value) -> Energy {
  return MakeQuantity<Energy>(value, "TeV");
}

consteval auto operator""_TeV(long double value) -> Energy {
  return MakeQuantity<Energy>(value, "TeV");
}
} // namespace ggems::units
