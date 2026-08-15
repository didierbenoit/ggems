#pragma once

#include <cstdint>
#include <string_view>
#include <array>

#include "GGEMS/core/units/GGEMSQuantity.hh"

namespace ggems::units {

struct CrossSectionUnitSet {
  using dimension = AreaDim;
};

template <> struct UnitRegistry<CrossSectionUnitSet> {
  static constexpr std::array<UnitDefinition, 6U> units{{
      {.symbol = "pb",
       .scale = DecimalScale(0)},
      {.symbol = "nb",
       .scale = DecimalScale(3)},
      {.symbol = "ub",
       .scale = DecimalScale(6),
       .unicode_symbol = "µb"},
      {.symbol = "mb",
       .scale = DecimalScale(9)},
      {.symbol = "barn",
       .scale = DecimalScale(12)},
      {.symbol = "kbarn",
       .scale = DecimalScale(15)},
  }};
};

struct CrossSectionFamily {
  using dimension = AreaDim;
  using unit_set = CrossSectionUnitSet;
  using representation = std::uint64_t;
  static constexpr std::string_view name{"CrossSection"};
  static constexpr QuantityDomain domain{QuantityDomain::NonNegative};
  static constexpr QuantityFormatPolicy format_policy{
      QuantityFormatPolicy::AutomaticScale};
  static constexpr std::string_view fixed_display_unit{};
  static constexpr std::int8_t default_precision{7};
};

using CrossSection = Quantity<CrossSectionFamily>;

static_assert(ValidateUnitSet<CrossSectionUnitSet>());
static_assert(ValidateFamily<CrossSectionFamily>());

consteval auto operator""_pb(unsigned long long value) -> CrossSection {
  return MakeQuantity<CrossSection>(value, "pb");
}

consteval auto operator""_pb(long double value) -> CrossSection {
  return MakeQuantity<CrossSection>(value, "pb");
}

consteval auto operator""_nb(unsigned long long value) -> CrossSection {
  return MakeQuantity<CrossSection>(value, "nb");
}

consteval auto operator""_nb(long double value) -> CrossSection {
  return MakeQuantity<CrossSection>(value, "nb");
}

consteval auto operator""_ub(unsigned long long value) -> CrossSection {
  return MakeQuantity<CrossSection>(value, "ub");
}

consteval auto operator""_ub(long double value) -> CrossSection {
  return MakeQuantity<CrossSection>(value, "ub");
}

consteval auto operator""_mb(unsigned long long value) -> CrossSection {
  return MakeQuantity<CrossSection>(value, "mb");
}

consteval auto operator""_mb(long double value) -> CrossSection {
  return MakeQuantity<CrossSection>(value, "mb");
}

consteval auto operator""_barn(unsigned long long value) -> CrossSection {
  return MakeQuantity<CrossSection>(value, "barn");
}

consteval auto operator""_barn(long double value) -> CrossSection {
  return MakeQuantity<CrossSection>(value, "barn");
}

consteval auto operator""_kbarn(unsigned long long value) -> CrossSection {
  return MakeQuantity<CrossSection>(value, "kbarn");
}

consteval auto operator""_kbarn(long double value) -> CrossSection {
  return MakeQuantity<CrossSection>(value, "kbarn");
}

consteval auto operator""_pbarn(unsigned long long value) -> CrossSection {
  return MakeQuantity<CrossSection>(value, "pb");
}

consteval auto operator""_pbarn(long double value) -> CrossSection {
  return MakeQuantity<CrossSection>(value, "pb");
}

consteval auto operator""_nbarn(unsigned long long value) -> CrossSection {
  return MakeQuantity<CrossSection>(value, "nb");
}

consteval auto operator""_nbarn(long double value) -> CrossSection {
  return MakeQuantity<CrossSection>(value, "nb");
}

consteval auto operator""_ubarn(unsigned long long value) -> CrossSection {
  return MakeQuantity<CrossSection>(value, "ub");
}

consteval auto operator""_ubarn(long double value) -> CrossSection {
  return MakeQuantity<CrossSection>(value, "ub");
}

consteval auto operator""_mbarn(unsigned long long value) -> CrossSection {
  return MakeQuantity<CrossSection>(value, "mb");
}

consteval auto operator""_mbarn(long double value) -> CrossSection {
  return MakeQuantity<CrossSection>(value, "mb");
}
} // namespace ggems::units
