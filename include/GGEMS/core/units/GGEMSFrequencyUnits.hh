#pragma once

#include <array>
#include <cstdint>
#include <string_view>

#include "GGEMS/core/units/GGEMSQuantity.hh"
#include "GGEMS/core/units/GGEMSUnitConversion.hh"

namespace ggems::units {

struct FrequencyUnitSet {};

template <> struct UnitRegistry<FrequencyUnitSet> {
  static constexpr std::array<UnitDefinition, 5U> units{{
      {.symbol = "Hz",
       .scale = DecimalScale(0)},
      {.symbol = "kHz",
       .scale = DecimalScale(3)},
      {.symbol = "MHz",
       .scale = DecimalScale(6)},
      {.symbol = "GHz",
       .scale = DecimalScale(9)},
      {.symbol = "THz",
       .scale = DecimalScale(12)},
  }};
};

struct FrequencyTag {};

template <> struct QuantityTraits<FrequencyTag> {
  using unit_set = FrequencyUnitSet;
  static constexpr QuantityDomain domain{QuantityDomain::NonNegative};
  static constexpr QuantityFormatPolicy format_policy{
      QuantityFormatPolicy::AutomaticScale};
  static constexpr std::string_view fixed_display_unit{};
  static constexpr std::int8_t default_precision{7};
};

using Frequency = Quantity<FrequencyTag, std::uint64_t>;

static_assert(ValidateUnitSet<FrequencyUnitSet>());
static_assert(ValidateQuantityTraits<FrequencyTag, std::uint64_t>());

consteval auto operator""_Hz(unsigned long long value) -> Frequency {
  return detail::MakeLiteralQuantity<Frequency>(value, "Hz");
}

consteval auto operator""_Hz(long double value) -> Frequency {
  return detail::MakeLiteralQuantity<Frequency>(value, "Hz");
}

consteval auto operator""_kHz(unsigned long long value) -> Frequency {
  return detail::MakeLiteralQuantity<Frequency>(value, "kHz");
}

consteval auto operator""_kHz(long double value) -> Frequency {
  return detail::MakeLiteralQuantity<Frequency>(value, "kHz");
}

consteval auto operator""_MHz(unsigned long long value) -> Frequency {
  return detail::MakeLiteralQuantity<Frequency>(value, "MHz");
}

consteval auto operator""_MHz(long double value) -> Frequency {
  return detail::MakeLiteralQuantity<Frequency>(value, "MHz");
}
consteval auto operator""_GHz(unsigned long long value) -> Frequency {
  return detail::MakeLiteralQuantity<Frequency>(value, "GHz");
}

consteval auto operator""_GHz(long double value) -> Frequency {
  return detail::MakeLiteralQuantity<Frequency>(value, "GHz");
}

consteval auto operator""_THz(unsigned long long value) -> Frequency {
  return detail::MakeLiteralQuantity<Frequency>(value, "THz");
}

consteval auto operator""_THz(long double value) -> Frequency {
  return detail::MakeLiteralQuantity<Frequency>(value, "THz");
}
} // namespace ggems::units
