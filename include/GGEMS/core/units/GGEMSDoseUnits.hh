#pragma once

#include <array>
#include <cstdint>
#include <string_view>

#include "GGEMS/core/units/GGEMSQuantity.hh"

namespace ggems::units {

struct DoseUnitSet {
  using dimension = DoseDim;
};

template <> struct UnitRegistry<DoseUnitSet> {
  static constexpr std::array<UnitDefinition, 4U> units{{
      {.canonical_name = "MillielectronvoltPerPicogram",
       .symbol = "meV/pg",
       .display_symbol = "meV/pg",
       .aliases = {},
       .literal_suffix = "_meV_pg",
       .scale = DecimalScale(0),
       .canonical = true,
       .automatic_display = false},
      {.canonical_name = "Gray",
       .symbol = "Gy",
       .display_symbol = "Gy",
       .aliases = {},
       .literal_suffix = "_Gy",
       .scale = SpecialScale(1.0L / 1.602176634e-7L),
       .automatic_display = true},
      {.canonical_name = "Milligray",
       .symbol = "mGy",
       .display_symbol = "mGy",
       .aliases = {},
       .literal_suffix = "_mGy",
       .scale = SpecialScale(1.0e-3L / 1.602176634e-7L),
       .automatic_display = true},
      {.canonical_name = "Microgray",
       .symbol = "uGy",
       .display_symbol = "µGy",
       .aliases = {"µGy", "μGy"},
       .literal_suffix = "_uGy",
       .scale = SpecialScale(1.0e-6L / 1.602176634e-7L),
       .automatic_display = true},
  }};
};

struct DoseFamily {
  using dimension = DoseDim;
  using unit_set = DoseUnitSet;
  using representation = std::uint64_t;
  static constexpr std::string_view name{"Dose"};
  static constexpr QuantityDomain domain{QuantityDomain::NonNegative};
  static constexpr QuantityFormatPolicy format_policy{
      QuantityFormatPolicy::AutomaticScale};
  static constexpr std::string_view fixed_display_unit{};
  static constexpr std::int8_t default_precision{7};
};

using Dose = Quantity<DoseFamily>;

static_assert(ValidateUnitSet<DoseUnitSet>());
static_assert(ValidateFamily<DoseFamily>());

consteval auto operator""_meV_pg(unsigned long long value) -> Dose {
  return MakeQuantity<Dose>(value, "meV/pg");
}

consteval auto operator""_meV_pg(long double value) -> Dose {
  return MakeQuantity<Dose>(value, "meV/pg");
}

consteval auto operator""_Gy(unsigned long long value) -> Dose {
  return MakeQuantity<Dose>(value, "Gy");
}

consteval auto operator""_Gy(long double value) -> Dose {
  return MakeQuantity<Dose>(value, "Gy");
}

consteval auto operator""_mGy(unsigned long long value) -> Dose {
  return MakeQuantity<Dose>(value, "mGy");
}

consteval auto operator""_mGy(long double value) -> Dose {
  return MakeQuantity<Dose>(value, "mGy");
}

consteval auto operator""_uGy(unsigned long long value) -> Dose {
  return MakeQuantity<Dose>(value, "uGy");
}

consteval auto operator""_uGy(long double value) -> Dose {
  return MakeQuantity<Dose>(value, "uGy");
}
} // namespace ggems::units
