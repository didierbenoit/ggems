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
      {.canonical_name = "Picogram",
       .symbol = "pg",
       .display_symbol = "pg",
       .aliases = {},
       .literal_suffix = "_pg",
       .scale = DecimalScale(0),
       .canonical = true,
       .automatic_display = true},
      {.canonical_name = "Nanogram",
       .symbol = "ng",
       .display_symbol = "ng",
       .aliases = {},
       .literal_suffix = "_ng",
       .scale = DecimalScale(3),
       .automatic_display = true},
      {.canonical_name = "Microgram",
       .symbol = "ug",
       .display_symbol = "µg",
       .aliases = {"µg", "μg"},
       .literal_suffix = "_ug",
       .scale = DecimalScale(6),
       .automatic_display = true},
      {.canonical_name = "Milligram",
       .symbol = "mg",
       .display_symbol = "mg",
       .aliases = {},
       .literal_suffix = "_mg",
       .scale = DecimalScale(9),
       .automatic_display = true},
      {.canonical_name = "Gram",
       .symbol = "g",
       .display_symbol = "g",
       .aliases = {},
       .literal_suffix = "_g",
       .scale = DecimalScale(12),
       .automatic_display = true},
      {.canonical_name = "Kilogram",
       .symbol = "kg",
       .display_symbol = "kg",
       .aliases = {},
       .literal_suffix = "_kg",
       .scale = DecimalScale(15),
       .automatic_display = true},
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
  return MakeQuantity<Mass>(value, "pg");
}

consteval auto operator""_pg(long double value) -> Mass {
  return MakeQuantity<Mass>(value, "pg");
}

consteval auto operator""_ng(unsigned long long value) -> Mass {
  return MakeQuantity<Mass>(value, "ng");
}

consteval auto operator""_ng(long double value) -> Mass {
  return MakeQuantity<Mass>(value, "ng");
}

consteval auto operator""_ug(unsigned long long value) -> Mass {
  return MakeQuantity<Mass>(value, "ug");
}

consteval auto operator""_ug(long double value) -> Mass {
  return MakeQuantity<Mass>(value, "ug");
}

consteval auto operator""_mg(unsigned long long value) -> Mass {
  return MakeQuantity<Mass>(value, "mg");
}

consteval auto operator""_mg(long double value) -> Mass {
  return MakeQuantity<Mass>(value, "mg");
}

consteval auto operator""_g(unsigned long long value) -> Mass {
  return MakeQuantity<Mass>(value, "g");
}

consteval auto operator""_g(long double value) -> Mass {
  return MakeQuantity<Mass>(value, "g");
}

consteval auto operator""_kg(unsigned long long value) -> Mass {
  return MakeQuantity<Mass>(value, "kg");
}

consteval auto operator""_kg(long double value) -> Mass {
  return MakeQuantity<Mass>(value, "kg");
}
} // namespace ggems::units
