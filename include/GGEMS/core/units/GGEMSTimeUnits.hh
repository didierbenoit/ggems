#pragma once

#include <array>
#include <cstdint>
#include <string_view>

#include "GGEMS/core/units/GGEMSQuantity.hh"

namespace ggems::units {

struct TimeUnitSet {
  using dimension = TimeDim;
};

template <> struct UnitRegistry<TimeUnitSet> {
  static constexpr std::array<UnitDefinition, 7U> units{{
      {.symbol = "ps",
       .scale = DecimalScale(0)},
      {.symbol = "ns",
       .scale = DecimalScale(3)},
      {.symbol = "us",
       .scale = DecimalScale(6),
       .unicode_symbol = "µs"},
      {.symbol = "ms",
       .scale = DecimalScale(9)},
      {.symbol = "s",
       .scale = DecimalScale(12)},
      {.symbol = "min",
       .scale = DecimalScale(12, 60ULL),
       .automatic_display = false},
      {.symbol = "h",
       .scale = DecimalScale(12, 3'600ULL),
       .automatic_display = false},
  }};
};

struct DurationFamily {
  using dimension = TimeDim;
  using unit_set = TimeUnitSet;
  using representation = std::uint64_t;
  static constexpr std::string_view name{"Duration"};
  static constexpr QuantityDomain domain{QuantityDomain::NonNegative};
  static constexpr QuantityFormatPolicy format_policy{
      QuantityFormatPolicy::DurationBreakdown};
  static constexpr std::string_view fixed_display_unit{};
  static constexpr std::int8_t default_precision{7};
};

struct TimePointFamily {
  using dimension = TimeDim;
  using unit_set = TimeUnitSet;
  using representation = std::uint64_t;
  static constexpr std::string_view name{"TimePoint"};
  static constexpr QuantityDomain domain{QuantityDomain::NonNegative};
  static constexpr QuantityFormatPolicy format_policy{
      QuantityFormatPolicy::AutomaticScale};
  static constexpr std::string_view fixed_display_unit{};
  static constexpr std::int8_t default_precision{7};
};

using Duration = Quantity<DurationFamily>;
using TimePoint = Quantity<TimePointFamily>;
using Time = Duration;

static_assert(ValidateUnitSet<TimeUnitSet>());
static_assert(ValidateFamily<DurationFamily>());
static_assert(ValidateFamily<TimePointFamily>());

consteval auto operator""_ps(unsigned long long value) -> Time {
  return MakeQuantity<Time>(value, "ps");
}

consteval auto operator""_ps(long double value) -> Time {
  return MakeQuantity<Time>(value, "ps");
}

consteval auto operator""_ns(unsigned long long value) -> Time {
  return MakeQuantity<Time>(value, "ns");
}

consteval auto operator""_ns(long double value) -> Time {
  return MakeQuantity<Time>(value, "ns");
}

consteval auto operator""_us(unsigned long long value) -> Time {
  return MakeQuantity<Time>(value, "us");
}

consteval auto operator""_us(long double value) -> Time {
  return MakeQuantity<Time>(value, "us");
}

consteval auto operator""_ms(unsigned long long value) -> Time {
  return MakeQuantity<Time>(value, "ms");
}

consteval auto operator""_ms(long double value) -> Time {
  return MakeQuantity<Time>(value, "ms");
}

consteval auto operator""_s(unsigned long long value) -> Time {
  return MakeQuantity<Time>(value, "s");
}

consteval auto operator""_s(long double value) -> Time {
  return MakeQuantity<Time>(value, "s");
}

consteval auto operator""_min(unsigned long long value) -> Time {
  return MakeQuantity<Time>(value, "min");
}

consteval auto operator""_min(long double value) -> Time {
  return MakeQuantity<Time>(value, "min");
}

consteval auto operator""_h(unsigned long long value) -> Time {
  return MakeQuantity<Time>(value, "h");
}

consteval auto operator""_h(long double value) -> Time {
  return MakeQuantity<Time>(value, "h");
}
} // namespace ggems::units
