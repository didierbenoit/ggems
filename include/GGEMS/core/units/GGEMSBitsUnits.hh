#pragma once

#include <array>
#include <cstdint>
#include <expected>
#include <limits>
#include <string_view>
#include <type_traits>

#include "GGEMS/core/units/GGEMSQuantity.hh"
#include "GGEMS/core/units/GGEMSBytesUnits.hh"

namespace ggems::units {

struct BitsUnitSet {
  using dimension = InformationDim;
};

template <> struct UnitRegistry<BitsUnitSet> {
  static constexpr std::array<UnitDefinition, 9U> units{{
      {.symbol = "bit",
       .scale = DecimalScale(0)},
      {.symbol = "kbit",
       .scale = DecimalScale(3)},
      {.symbol = "Mbit",
       .scale = DecimalScale(6)},
      {.symbol = "Gbit",
       .scale = DecimalScale(9)},
      {.symbol = "Tbit",
       .scale = DecimalScale(12)},
      {.symbol = "Kibit",
       .scale = DecimalScale(0, 1'024ULL),
       .automatic_display = false},
      {.symbol = "Mibit",
       .scale = DecimalScale(0, 1'048'576ULL),
       .automatic_display = false},
      {.symbol = "Gibit",
       .scale = DecimalScale(0, 1'073'741'824ULL),
       .automatic_display = false},
      {.symbol = "Tibit",
       .scale = DecimalScale(0, 1'099'511'627'776ULL),
       .automatic_display = false},
  }};
};

struct BitsFamily {
  using dimension = InformationDim;
  using unit_set = BitsUnitSet;
  using representation = std::uint64_t;
  static constexpr std::string_view name{"Bits"};
  static constexpr QuantityDomain domain{QuantityDomain::NonNegative};
  static constexpr QuantityFormatPolicy format_policy{
      QuantityFormatPolicy::AutomaticScale};
  static constexpr std::string_view fixed_display_unit{};
  static constexpr std::int8_t default_precision{7};
};

using Bits = Quantity<BitsFamily>;

static_assert(ValidateUnitSet<BitsUnitSet>());
static_assert(ValidateFamily<BitsFamily>());
static_assert(std::is_same_v<Bits::dimension, Bytes::dimension>);
static_assert(!std::is_same_v<Bits, Bytes>);

[[nodiscard]] constexpr auto TryConvertBytesToBits(Bytes bytes) noexcept
    -> std::expected<Bits, UnitConversionError> {
  if (bytes.value > std::numeric_limits<std::uint64_t>::max() / 8ULL) {
    return std::unexpected(UnitConversionError::OutOfRange);
  }

  return Bits{bytes.value * 8ULL};
}

[[nodiscard]] constexpr auto TryConvertBitsToBytes(Bits bits) noexcept
    -> std::expected<Bytes, UnitConversionError> {
  if (bits.value % 8ULL != 0ULL) {
    return std::unexpected(UnitConversionError::InexactConversion);
  }
  return Bytes{bits.value / 8ULL};
}

consteval auto operator""_bit(unsigned long long value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "bit");
}

consteval auto operator""_bit(long double value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "bit");
}

consteval auto operator""_b(unsigned long long value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "bit");
}

consteval auto operator""_b(long double value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "bit");
}

consteval auto operator""_kbit(unsigned long long value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "kbit");
}

consteval auto operator""_kbit(long double value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "kbit");
}

consteval auto operator""_Mbit(unsigned long long value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "Mbit");
}

consteval auto operator""_Mbit(long double value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "Mbit");
}

consteval auto operator""_Gbit(unsigned long long value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "Gbit");
}

consteval auto operator""_Gbit(long double value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "Gbit");
}

consteval auto operator""_Tbit(unsigned long long value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "Tbit");
}

consteval auto operator""_Tbit(long double value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "Tbit");
}

consteval auto operator""_Kibit(unsigned long long value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "Kibit");
}

consteval auto operator""_Kibit(long double value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "Kibit");
}

consteval auto operator""_Mibit(unsigned long long value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "Mibit");
}

consteval auto operator""_Mibit(long double value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "Mibit");
}

consteval auto operator""_Gibit(unsigned long long value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "Gibit");
}

consteval auto operator""_Gibit(long double value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "Gibit");
}

consteval auto operator""_Tibit(unsigned long long value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "Tibit");
}

consteval auto operator""_Tibit(long double value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "Tibit");
}

consteval auto operator""_kb(unsigned long long value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "kbit");
}

consteval auto operator""_kb(long double value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "kbit");
}

consteval auto operator""_Mb(unsigned long long value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "Mbit");
}

consteval auto operator""_Mb(long double value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "Mbit");
}

consteval auto operator""_Gb(unsigned long long value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "Gbit");
}

consteval auto operator""_Gb(long double value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "Gbit");
}

consteval auto operator""_Tb(unsigned long long value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "Tbit");
}

consteval auto operator""_Tb(long double value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "Tbit");
}

} // namespace ggems::units
