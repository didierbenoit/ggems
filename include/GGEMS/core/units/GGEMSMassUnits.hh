#pragma once

#include <array>
#include <cstdint>
#include <string_view>

#include "GGEMS/core/units/GGEMSQuantity.hh"

namespace ggems::units {

struct MassUnitSet {
  using dimension = MassDim;
};

template <> struct UnitRegistry<MassUnitSet> {
  static constexpr std::array<UnitDefinition, 6U> units{{
      {.symbol = "pg",
       .scale = DecimalScale(0)},
      {.symbol = "ng",
       .scale = DecimalScale(3)},
      {.symbol = "ug",
       .scale = DecimalScale(6),
       .unicode_symbol = "µg"},
      {.symbol = "mg",
       .scale = DecimalScale(9)},
      {.symbol = "g",
       .scale = DecimalScale(12)},
      {.symbol = "kg",
       .scale = DecimalScale(15)},
  }};
};

struct MassFamily {
  using dimension = MassDim;
  using unit_set = MassUnitSet;
  using representation = std::uint64_t;
  static constexpr std::string_view name{"Mass"};
  static constexpr QuantityDomain domain{QuantityDomain::NonNegative};
  static constexpr QuantityFormatPolicy format_policy{
      QuantityFormatPolicy::AutomaticScale};
  static constexpr std::string_view fixed_display_unit{};
  static constexpr std::int8_t default_precision{7};
};

using Mass = Quantity<MassFamily>;

static_assert(ValidateUnitSet<MassUnitSet>());
static_assert(ValidateFamily<MassFamily>());

consteval auto operator""_pg(unsigned long long value) -> Mass {
  return detail::MakeLiteralQuantity<Mass>(value, "pg");
}

consteval auto operator""_pg(long double value) -> Mass {
  return detail::MakeLiteralQuantity<Mass>(value, "pg");
}

consteval auto operator""_ng(unsigned long long value) -> Mass {
  return detail::MakeLiteralQuantity<Mass>(value, "ng");
}

consteval auto operator""_ng(long double value) -> Mass {
  return detail::MakeLiteralQuantity<Mass>(value, "ng");
}

consteval auto operator""_ug(unsigned long long value) -> Mass {
  return detail::MakeLiteralQuantity<Mass>(value, "ug");
}

consteval auto operator""_ug(long double value) -> Mass {
  return detail::MakeLiteralQuantity<Mass>(value, "ug");
}

consteval auto operator""_mg(unsigned long long value) -> Mass {
  return detail::MakeLiteralQuantity<Mass>(value, "mg");
}

consteval auto operator""_mg(long double value) -> Mass {
  return detail::MakeLiteralQuantity<Mass>(value, "mg");
}

consteval auto operator""_g(unsigned long long value) -> Mass {
  return detail::MakeLiteralQuantity<Mass>(value, "g");
}

consteval auto operator""_g(long double value) -> Mass {
  return detail::MakeLiteralQuantity<Mass>(value, "g");
}

consteval auto operator""_kg(unsigned long long value) -> Mass {
  return detail::MakeLiteralQuantity<Mass>(value, "kg");
}

consteval auto operator""_kg(long double value) -> Mass {
  return detail::MakeLiteralQuantity<Mass>(value, "kg");
}
} // namespace ggems::units
