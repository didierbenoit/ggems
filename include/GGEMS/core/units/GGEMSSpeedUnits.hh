#pragma once

#include <array>
#include <cstdint>
#include <string_view>

#include "GGEMS/core/units/GGEMSQuantity.hh"
#include "GGEMS/core/units/GGEMSLengthUnits.hh"
#include "GGEMS/core/units/GGEMSTimeUnits.hh"

namespace ggems::units {

struct SpeedUnitSet {
  using dimension = SpeedDim;
};

template <> struct UnitRegistry<SpeedUnitSet> {
  static constexpr std::array<UnitDefinition, 2U> units{{
      {.symbol = "pm/ps",
       .scale = DecimalScale(0),
       .automatic_display = false},
      {.symbol = "m/s",
       .scale = DecimalScale(0)},
  }};
};

struct SpeedFamily {
  using dimension = SpeedDim;
  using unit_set = SpeedUnitSet;
  using representation = long double;
  static constexpr std::string_view name{"Speed"};
  static constexpr QuantityDomain domain{QuantityDomain::NonNegative};
  static constexpr QuantityFormatPolicy format_policy{
      QuantityFormatPolicy::FixedUnit};
  static constexpr std::string_view fixed_display_unit{"m/s"};
  static constexpr std::int8_t default_precision{7};
};

using Speed = Quantity<SpeedFamily>;

static_assert(ValidateUnitSet<SpeedUnitSet>());
static_assert(ValidateFamily<SpeedFamily>());

consteval auto operator""_pm_ps(unsigned long long value) -> Speed {
  return detail::MakeLiteralQuantity<Speed>(value, "pm/ps");
}

consteval auto operator""_pm_ps(long double value) -> Speed {
  return detail::MakeLiteralQuantity<Speed>(value, "pm/ps");
}

consteval auto operator""_m_s(unsigned long long value) -> Speed {
  return detail::MakeLiteralQuantity<Speed>(value, "m/s");
}

consteval auto operator""_m_s(long double value) -> Speed {
  return detail::MakeLiteralQuantity<Speed>(value, "m/s");
}

inline auto MakeSpeed(Length length, Time duration) noexcept -> Speed {
  return Speed{static_cast<long double>(length.value) /
               static_cast<long double>(duration.value)};
}
} // namespace ggems::units
