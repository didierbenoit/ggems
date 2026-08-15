#pragma once

#include <array>
#include <cstdint>
#include <string_view>

#include "GGEMS/core/units/GGEMSQuantity.hh"

namespace ggems::units {

struct BytesUnitSet {
  using dimension = InformationDim;
};

template <> struct UnitRegistry<BytesUnitSet> {
  static constexpr std::array<UnitDefinition, 9U> units{{
      {.symbol = "B",
       .scale = DecimalScale(0)},
      {.symbol = "kB",
       .scale = DecimalScale(3),
       .automatic_display = false},
      {.symbol = "MB",
       .scale = DecimalScale(6),
       .automatic_display = false},
      {.symbol = "GB",
       .scale = DecimalScale(9),
       .automatic_display = false},
      {.symbol = "TB",
       .scale = DecimalScale(12),
       .automatic_display = false},
      {.symbol = "KiB",
       .scale = DecimalScale(0, 1'024ULL)},
      {.symbol = "MiB",
       .scale = DecimalScale(0, 1'048'576ULL)},
      {.symbol = "GiB",
       .scale = DecimalScale(0, 1'073'741'824ULL)},
      {.symbol = "TiB",
       .scale = DecimalScale(0, 1'099'511'627'776ULL)},
  }};
};

struct BytesFamily {
  using dimension = InformationDim;
  using unit_set = BytesUnitSet;
  using representation = std::uint64_t;
  static constexpr std::string_view name{"Bytes"};
  static constexpr QuantityDomain domain{QuantityDomain::NonNegative};
  static constexpr QuantityFormatPolicy format_policy{
      QuantityFormatPolicy::AutomaticScale};
  static constexpr std::string_view fixed_display_unit{};
  static constexpr std::int8_t default_precision{7};
};

using Bytes = Quantity<BytesFamily>;

static_assert(ValidateUnitSet<BytesUnitSet>());
static_assert(ValidateFamily<BytesFamily>());

consteval auto operator""_B(unsigned long long value) -> Bytes {
  return MakeQuantity<Bytes>(value, "B");
}

consteval auto operator""_B(long double value) -> Bytes {
  return MakeQuantity<Bytes>(value, "B");
}

consteval auto operator""_kB(unsigned long long value) -> Bytes {
  return MakeQuantity<Bytes>(value, "kB");
}

consteval auto operator""_kB(long double value) -> Bytes {
  return MakeQuantity<Bytes>(value, "kB");
}

consteval auto operator""_MB(unsigned long long value) -> Bytes {
  return MakeQuantity<Bytes>(value, "MB");
}

consteval auto operator""_MB(long double value) -> Bytes {
  return MakeQuantity<Bytes>(value, "MB");
}

consteval auto operator""_GB(unsigned long long value) -> Bytes {
  return MakeQuantity<Bytes>(value, "GB");
}

consteval auto operator""_GB(long double value) -> Bytes {
  return MakeQuantity<Bytes>(value, "GB");
}

consteval auto operator""_TB(unsigned long long value) -> Bytes {
  return MakeQuantity<Bytes>(value, "TB");
}

consteval auto operator""_TB(long double value) -> Bytes {
  return MakeQuantity<Bytes>(value, "TB");
}

consteval auto operator""_KiB(unsigned long long value) -> Bytes {
  return MakeQuantity<Bytes>(value, "KiB");
}

consteval auto operator""_KiB(long double value) -> Bytes {
  return MakeQuantity<Bytes>(value, "KiB");
}

consteval auto operator""_MiB(unsigned long long value) -> Bytes {
  return MakeQuantity<Bytes>(value, "MiB");
}

consteval auto operator""_MiB(long double value) -> Bytes {
  return MakeQuantity<Bytes>(value, "MiB");
}

consteval auto operator""_GiB(unsigned long long value) -> Bytes {
  return MakeQuantity<Bytes>(value, "GiB");
}

consteval auto operator""_GiB(long double value) -> Bytes {
  return MakeQuantity<Bytes>(value, "GiB");
}

consteval auto operator""_TiB(unsigned long long value) -> Bytes {
  return MakeQuantity<Bytes>(value, "TiB");
}

consteval auto operator""_TiB(long double value) -> Bytes {
  return MakeQuantity<Bytes>(value, "TiB");
}
} // namespace ggems::units
