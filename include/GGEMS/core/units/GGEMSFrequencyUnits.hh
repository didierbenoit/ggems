#pragma once

#include <array>
#include <cstdint>
#include <string_view>

#include "GGEMS/core/units/GGEMSQuantity.hh"

namespace ggems::units {

struct FrequencyUnitSet {
  using dimension = FrequencyDim;
};

template <> struct UnitRegistry<FrequencyUnitSet> {
  static constexpr std::array<UnitDefinition, 5U> units{{
      {.canonical_name = "Hertz",
       .symbol = "Hz",
       .display_symbol = "Hz",
       .aliases = {},
       .literal_suffix = "_Hz",
       .scale = DecimalScale(0),
       .canonical = true,
       .automatic_display = true},
      {.canonical_name = "Kilohertz",
       .symbol = "kHz",
       .display_symbol = "kHz",
       .aliases = {},
       .literal_suffix = "_kHz",
       .scale = DecimalScale(3),
       .automatic_display = true},
      {.canonical_name = "Megahertz",
       .symbol = "MHz",
       .display_symbol = "MHz",
       .aliases = {},
       .literal_suffix = "_MHz",
       .scale = DecimalScale(6),
       .automatic_display = true},
      {.canonical_name = "Gigahertz",
       .symbol = "GHz",
       .display_symbol = "GHz",
       .aliases = {},
       .literal_suffix = "_GHz",
       .scale = DecimalScale(9),
       .automatic_display = true},
      {.canonical_name = "Terahertz",
       .symbol = "THz",
       .display_symbol = "THz",
       .aliases = {},
       .literal_suffix = "_THz",
       .scale = DecimalScale(12),
       .automatic_display = true},
  }};
};

struct FrequencyFamily {
  using dimension = FrequencyDim;
  using unit_set = FrequencyUnitSet;
  using representation = std::uint64_t;
  static constexpr std::string_view name{"Frequency"};
  static constexpr QuantityDomain domain{QuantityDomain::NonNegative};
  static constexpr QuantityFormatPolicy format_policy{
      QuantityFormatPolicy::AutomaticScale};
  static constexpr std::string_view fixed_display_unit{};
  static constexpr std::int8_t default_precision{7};
};

using Frequency = Quantity<FrequencyFamily>;

static_assert(ValidateUnitSet<FrequencyUnitSet>());
static_assert(ValidateFamily<FrequencyFamily>());

consteval auto operator""_Hz(unsigned long long value) -> Frequency {
  return MakeQuantity<Frequency>(value, "Hz");
}

consteval auto operator""_Hz(long double value) -> Frequency {
  return MakeQuantity<Frequency>(value, "Hz");
}

consteval auto operator""_kHz(unsigned long long value) -> Frequency {
  return MakeQuantity<Frequency>(value, "kHz");
}

consteval auto operator""_kHz(long double value) -> Frequency {
  return MakeQuantity<Frequency>(value, "kHz");
}

consteval auto operator""_MHz(unsigned long long value) -> Frequency {
  return MakeQuantity<Frequency>(value, "MHz");
}

consteval auto operator""_MHz(long double value) -> Frequency {
  return MakeQuantity<Frequency>(value, "MHz");
}
consteval auto operator""_GHz(unsigned long long value) -> Frequency {
  return MakeQuantity<Frequency>(value, "GHz");
}

consteval auto operator""_GHz(long double value) -> Frequency {
  return MakeQuantity<Frequency>(value, "GHz");
}

consteval auto operator""_THz(unsigned long long value) -> Frequency {
  return MakeQuantity<Frequency>(value, "THz");
}

consteval auto operator""_THz(long double value) -> Frequency {
  return MakeQuantity<Frequency>(value, "THz");
}
} // namespace ggems::units
