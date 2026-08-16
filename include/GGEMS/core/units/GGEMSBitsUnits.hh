// *****************************************************************************
// * This file is part of GGEMS.                                               *
// *                                                                           *
// * SPDX-License-Identifier: GPL-3.0-or-later                                 *
// * Copyright (C) 2017-2026 CHRU de Brest, Université de Bretagne Occidentale,*
// * Inserm.                                                                   *
// *                                                                           *
// * GGEMS is free software: you can redistribute it and/or modify             *
// * it under the terms of the GNU General Public License as published by      *
// * the Free Software Foundation, either version 3 of the License, or         *
// * (at your option) any later version.                                       *
// *                                                                           *
// * GGEMS is distributed in the hope that it will be useful,                  *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
// * GNU General Public License for more details.                              *
// *                                                                           *
// * You should have received a copy of the GNU General Public License         *
// * along with GGEMS. If not, see <https://www.gnu.org/licenses/>.            *
// *****************************************************************************

/*!
 * \file
 * \brief Declares strongly typed bit-count units, byte conversions, and literals.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <array>
#include <cstdint>
#include <expected>
#include <limits>
#include <string_view>
#include <type_traits>
/// \endcond

#include "GGEMS/core/units/GGEMSQuantity.hh"
#include "GGEMS/core/units/GGEMSUnitConversion.hh"
#include "GGEMS/core/units/GGEMSBytesUnits.hh"

namespace ggems::units {

/*!
 * \brief Marker type identifying the bit count unit registry.
 */
struct BitsUnitSet {};

/*!
 * \brief Defines the supported bit count units and their canonical scale factors.
 */
template <> struct UnitRegistry<BitsUnitSet> {
  /*!
   * \brief Registered unit definitions for this quantity family.
   */
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

/*!
 * \brief Tag type identifying bit-count quantities.
 */
struct BitsTag {};

/*!
 * \brief Defines conversion and formatting traits for Bits quantities.
 */
template <> struct QuantityTraits<BitsTag> {
  /*!
   * \brief Unit registry associated with this quantity type.
   */
  using unit_set = BitsUnitSet;
  /*!
   * \brief Allowed sign domain for this quantity type.
   */
  static constexpr QuantityDomain domain{QuantityDomain::NonNegative};
  /*!
   * \brief Formatting policy used for human-readable output.
   */
  static constexpr QuantityFormatPolicy format_policy{
      QuantityFormatPolicy::AutomaticScale};
  /*!
   * \brief Fixed display unit, or an empty string when the policy selects units automatically.
   */
  static constexpr std::string_view fixed_display_unit{};
  /*!
   * \brief Default number of digits after the decimal point for formatted output.
   */
  static constexpr std::int8_t default_precision{7};
};

/*!
 * \brief Strongly typed bit-count quantity stored canonically in bits.
 */
using Bits = Quantity<BitsTag, std::uint64_t>;

static_assert(ValidateUnitSet<BitsUnitSet>());
static_assert(ValidateQuantityTraits<BitsTag, std::uint64_t>());
static_assert(!std::is_same_v<Bits, Bytes>);

/*!
 * \brief Converts an exact byte count to bits with overflow checking.
 *
 * \param[in] bytes Byte count to convert.
 * \return Converted bit count, or UnitConversionError::OutOfRange on overflow.
 */
[[nodiscard]] constexpr auto TryConvertBytesToBits(Bytes bytes) noexcept
    -> std::expected<Bits, UnitConversionError> {
  if (bytes.value > std::numeric_limits<std::uint64_t>::max() / 8ULL) {
    return std::unexpected(UnitConversionError::OutOfRange);
  }

  return Bits{bytes.value * 8ULL};
}

/*!
 * \brief Converts a bit count to bytes when the conversion is exact.
 *
 * \param[in] bits Bit count to convert.
 * \return Converted byte count, or UnitConversionError::InexactConversion when the bit count is not byte-aligned.
 */
[[nodiscard]] constexpr auto TryConvertBitsToBytes(Bits bits) noexcept
    -> std::expected<Bytes, UnitConversionError> {
  if (bits.value % 8ULL != 0ULL) {
    return std::unexpected(UnitConversionError::InexactConversion);
  }
  return Bytes{bits.value / 8ULL};
}

/*!
 * \brief Creates a bit-count quantity from a \c _bit literal.
 *
 * \param[in] value Literal value expressed in bit.
 * \return Bits converted to its canonical GGEMS representation.
 */
consteval auto operator""_bit(unsigned long long value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "bit");
}

/*!
 * \brief Creates a bit-count quantity from a \c _bit literal.
 *
 * \param[in] value Literal value expressed in bit.
 * \return Bits converted to its canonical GGEMS representation.
 */
consteval auto operator""_bit(long double value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "bit");
}

/*!
 * \brief Creates a bit-count quantity from a \c _b literal.
 *
 * \param[in] value Literal value expressed in bit.
 * \return Bits converted to its canonical GGEMS representation.
 */
consteval auto operator""_b(unsigned long long value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "bit");
}

/*!
 * \brief Creates a bit-count quantity from a \c _b literal.
 *
 * \param[in] value Literal value expressed in bit.
 * \return Bits converted to its canonical GGEMS representation.
 */
consteval auto operator""_b(long double value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "bit");
}

/*!
 * \brief Creates a bit-count quantity from a \c _kbit literal.
 *
 * \param[in] value Literal value expressed in kbit.
 * \return Bits converted to its canonical GGEMS representation.
 */
consteval auto operator""_kbit(unsigned long long value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "kbit");
}

/*!
 * \brief Creates a bit-count quantity from a \c _kbit literal.
 *
 * \param[in] value Literal value expressed in kbit.
 * \return Bits converted to its canonical GGEMS representation.
 */
consteval auto operator""_kbit(long double value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "kbit");
}

/*!
 * \brief Creates a bit-count quantity from a \c _Mbit literal.
 *
 * \param[in] value Literal value expressed in Mbit.
 * \return Bits converted to its canonical GGEMS representation.
 */
consteval auto operator""_Mbit(unsigned long long value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "Mbit");
}

/*!
 * \brief Creates a bit-count quantity from a \c _Mbit literal.
 *
 * \param[in] value Literal value expressed in Mbit.
 * \return Bits converted to its canonical GGEMS representation.
 */
consteval auto operator""_Mbit(long double value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "Mbit");
}

/*!
 * \brief Creates a bit-count quantity from a \c _Gbit literal.
 *
 * \param[in] value Literal value expressed in Gbit.
 * \return Bits converted to its canonical GGEMS representation.
 */
consteval auto operator""_Gbit(unsigned long long value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "Gbit");
}

/*!
 * \brief Creates a bit-count quantity from a \c _Gbit literal.
 *
 * \param[in] value Literal value expressed in Gbit.
 * \return Bits converted to its canonical GGEMS representation.
 */
consteval auto operator""_Gbit(long double value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "Gbit");
}

/*!
 * \brief Creates a bit-count quantity from a \c _Tbit literal.
 *
 * \param[in] value Literal value expressed in Tbit.
 * \return Bits converted to its canonical GGEMS representation.
 */
consteval auto operator""_Tbit(unsigned long long value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "Tbit");
}

/*!
 * \brief Creates a bit-count quantity from a \c _Tbit literal.
 *
 * \param[in] value Literal value expressed in Tbit.
 * \return Bits converted to its canonical GGEMS representation.
 */
consteval auto operator""_Tbit(long double value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "Tbit");
}

/*!
 * \brief Creates a bit-count quantity from a \c _Kibit literal.
 *
 * \param[in] value Literal value expressed in Kibit.
 * \return Bits converted to its canonical GGEMS representation.
 */
consteval auto operator""_Kibit(unsigned long long value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "Kibit");
}

/*!
 * \brief Creates a bit-count quantity from a \c _Kibit literal.
 *
 * \param[in] value Literal value expressed in Kibit.
 * \return Bits converted to its canonical GGEMS representation.
 */
consteval auto operator""_Kibit(long double value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "Kibit");
}

/*!
 * \brief Creates a bit-count quantity from a \c _Mibit literal.
 *
 * \param[in] value Literal value expressed in Mibit.
 * \return Bits converted to its canonical GGEMS representation.
 */
consteval auto operator""_Mibit(unsigned long long value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "Mibit");
}

/*!
 * \brief Creates a bit-count quantity from a \c _Mibit literal.
 *
 * \param[in] value Literal value expressed in Mibit.
 * \return Bits converted to its canonical GGEMS representation.
 */
consteval auto operator""_Mibit(long double value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "Mibit");
}

/*!
 * \brief Creates a bit-count quantity from a \c _Gibit literal.
 *
 * \param[in] value Literal value expressed in Gibit.
 * \return Bits converted to its canonical GGEMS representation.
 */
consteval auto operator""_Gibit(unsigned long long value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "Gibit");
}

/*!
 * \brief Creates a bit-count quantity from a \c _Gibit literal.
 *
 * \param[in] value Literal value expressed in Gibit.
 * \return Bits converted to its canonical GGEMS representation.
 */
consteval auto operator""_Gibit(long double value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "Gibit");
}

/*!
 * \brief Creates a bit-count quantity from a \c _Tibit literal.
 *
 * \param[in] value Literal value expressed in Tibit.
 * \return Bits converted to its canonical GGEMS representation.
 */
consteval auto operator""_Tibit(unsigned long long value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "Tibit");
}

/*!
 * \brief Creates a bit-count quantity from a \c _Tibit literal.
 *
 * \param[in] value Literal value expressed in Tibit.
 * \return Bits converted to its canonical GGEMS representation.
 */
consteval auto operator""_Tibit(long double value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "Tibit");
}

/*!
 * \brief Creates a bit-count quantity from a \c _kb literal.
 *
 * \param[in] value Literal value expressed in kbit.
 * \return Bits converted to its canonical GGEMS representation.
 */
consteval auto operator""_kb(unsigned long long value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "kbit");
}

/*!
 * \brief Creates a bit-count quantity from a \c _kb literal.
 *
 * \param[in] value Literal value expressed in kbit.
 * \return Bits converted to its canonical GGEMS representation.
 */
consteval auto operator""_kb(long double value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "kbit");
}

/*!
 * \brief Creates a bit-count quantity from a \c _Mb literal.
 *
 * \param[in] value Literal value expressed in Mbit.
 * \return Bits converted to its canonical GGEMS representation.
 */
consteval auto operator""_Mb(unsigned long long value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "Mbit");
}

/*!
 * \brief Creates a bit-count quantity from a \c _Mb literal.
 *
 * \param[in] value Literal value expressed in Mbit.
 * \return Bits converted to its canonical GGEMS representation.
 */
consteval auto operator""_Mb(long double value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "Mbit");
}

/*!
 * \brief Creates a bit-count quantity from a \c _Gb literal.
 *
 * \param[in] value Literal value expressed in Gbit.
 * \return Bits converted to its canonical GGEMS representation.
 */
consteval auto operator""_Gb(unsigned long long value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "Gbit");
}

/*!
 * \brief Creates a bit-count quantity from a \c _Gb literal.
 *
 * \param[in] value Literal value expressed in Gbit.
 * \return Bits converted to its canonical GGEMS representation.
 */
consteval auto operator""_Gb(long double value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "Gbit");
}

/*!
 * \brief Creates a bit-count quantity from a \c _Tb literal.
 *
 * \param[in] value Literal value expressed in Tbit.
 * \return Bits converted to its canonical GGEMS representation.
 */
consteval auto operator""_Tb(unsigned long long value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "Tbit");
}

/*!
 * \brief Creates a bit-count quantity from a \c _Tb literal.
 *
 * \param[in] value Literal value expressed in Tbit.
 * \return Bits converted to its canonical GGEMS representation.
 */
consteval auto operator""_Tb(long double value) -> Bits {
  return detail::MakeLiteralQuantity<Bits>(value, "Tbit");
}

} // namespace ggems::units
