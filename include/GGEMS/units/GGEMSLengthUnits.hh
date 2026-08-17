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
 * \brief Declares strongly typed length, position-coordinate, and displacement units and literals.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <array>
#include <cstdint>
#include <string>
#include <string_view>
/// \endcond

#include "GGEMS/units/GGEMSQuantity.hh"
#include "GGEMS/units/GGEMSUnitConversion.hh"
#include "GGEMS/units/GGEMSUnitFormatting.hh"

namespace ggems::units {

/*!
 * \brief Marker type identifying the length unit registry.
 */
struct LengthUnitSet {};

/*!
 * \brief Defines the supported length units and their canonical scale factors.
 */
template <> struct UnitRegistry<LengthUnitSet> {
  /*!
   * \brief Registered unit definitions for this quantity family.
   */
  static constexpr std::array<UnitDefinition, 7U> units{{
      {.symbol = "pm",
       .scale = DecimalScale(0)},
      {.symbol = "nm",
       .scale = DecimalScale(3)},
      {.symbol = "um",
       .scale = DecimalScale(6),
       .unicode_symbol = "µm"},
      {.symbol = "mm",
       .scale = DecimalScale(9)},
      {.symbol = "cm",
       .scale = DecimalScale(10),
       .automatic_display = false},
      {.symbol = "m",
       .scale = DecimalScale(12)},
      {.symbol = "km",
       .scale = DecimalScale(15)},
  }};
};

/*!
 * \brief Tag type identifying nonnegative length quantities.
 */
struct LengthTag {};

/*!
 * \brief Defines conversion and formatting traits for Length quantities.
 */
template <> struct QuantityTraits<LengthTag> {
  /*!
   * \brief Unit registry associated with this quantity type.
   */
  using unit_set = LengthUnitSet;
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
 * \brief Tag type identifying signed position-coordinate quantities.
 */
struct PositionCoordinateTag {};

/*!
 * \brief Defines conversion and formatting traits for PositionCoordinate quantities.
 */
template <> struct QuantityTraits<PositionCoordinateTag> {
  /*!
   * \brief Unit registry associated with this quantity type.
   */
  using unit_set = LengthUnitSet;
  /*!
   * \brief Allowed sign domain for this quantity type.
   */
  static constexpr QuantityDomain domain{QuantityDomain::Signed};
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
 * \brief Tag type identifying signed displacement quantities.
 */
struct DisplacementTag {};

/*!
 * \brief Defines conversion and formatting traits for Displacement quantities.
 */
template <> struct QuantityTraits<DisplacementTag> {
  /*!
   * \brief Unit registry associated with this quantity type.
   */
  using unit_set = LengthUnitSet;
  /*!
   * \brief Allowed sign domain for this quantity type.
   */
  static constexpr QuantityDomain domain{QuantityDomain::Signed};
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
 * \brief Strongly typed nonnegative length quantity stored canonically in picometers.
 */
using Length = Quantity<LengthTag, std::uint64_t>;
/*!
 * \brief Strongly typed signed position coordinate stored canonically in picometers.
 */
using PositionCoordinate = Quantity<PositionCoordinateTag, std::int64_t>;
/*!
 * \brief Strongly typed signed displacement stored canonically in picometers.
 */
using Displacement = Quantity<DisplacementTag, std::int64_t>;

static_assert(ValidateUnitSet<LengthUnitSet>());
static_assert(ValidateQuantityTraits<LengthTag, std::uint64_t>());
static_assert(ValidateQuantityTraits<PositionCoordinateTag, std::int64_t>());
static_assert(ValidateQuantityTraits<DisplacementTag, std::int64_t>());

/*!
 * \brief Formats a signed picometer length using the GGEMS automatic length scale.
 *
 * \param[in] value_pm Signed length value in picometers.
 * \param[in] precision Number of digits after the decimal point.
 * \param[in] width Optional formatted numeric field width; negative selects the default width.
 * \return Human-readable length string.
 */
[[nodiscard]] inline auto HumanReadableSignedLength(std::int64_t value_pm,
                                                    std::int8_t precision = 7,
                                                    std::int8_t width = -1)
    -> std::string {
  return HumanReadable(PositionCoordinate{value_pm}, precision, width);
}

/*!
 * \brief Creates a length quantity from a \c _pm literal.
 *
 * \param[in] value Literal value expressed in pm.
 * \return Length converted to its canonical GGEMS representation.
 */
consteval auto operator""_pm(unsigned long long value) -> Length {
  return detail::MakeLiteralQuantity<Length>(value, "pm");
}

/*!
 * \brief Creates a length quantity from a \c _pm literal.
 *
 * \param[in] value Literal value expressed in pm.
 * \return Length converted to its canonical GGEMS representation.
 */
consteval auto operator""_pm(long double value) -> Length {
  return detail::MakeLiteralQuantity<Length>(value, "pm");
}

/*!
 * \brief Creates a length quantity from a \c _nm literal.
 *
 * \param[in] value Literal value expressed in nm.
 * \return Length converted to its canonical GGEMS representation.
 */
consteval auto operator""_nm(unsigned long long value) -> Length {
  return detail::MakeLiteralQuantity<Length>(value, "nm");
}

/*!
 * \brief Creates a length quantity from a \c _nm literal.
 *
 * \param[in] value Literal value expressed in nm.
 * \return Length converted to its canonical GGEMS representation.
 */
consteval auto operator""_nm(long double value) -> Length {
  return detail::MakeLiteralQuantity<Length>(value, "nm");
}

/*!
 * \brief Creates a length quantity from a \c _um literal.
 *
 * \param[in] value Literal value expressed in um.
 * \return Length converted to its canonical GGEMS representation.
 */
consteval auto operator""_um(unsigned long long value) -> Length {
  return detail::MakeLiteralQuantity<Length>(value, "um");
}

/*!
 * \brief Creates a length quantity from a \c _um literal.
 *
 * \param[in] value Literal value expressed in um.
 * \return Length converted to its canonical GGEMS representation.
 */
consteval auto operator""_um(long double value) -> Length {
  return detail::MakeLiteralQuantity<Length>(value, "um");
}

/*!
 * \brief Creates a length quantity from a \c _mm literal.
 *
 * \param[in] value Literal value expressed in mm.
 * \return Length converted to its canonical GGEMS representation.
 */
consteval auto operator""_mm(unsigned long long value) -> Length {
  return detail::MakeLiteralQuantity<Length>(value, "mm");
}

/*!
 * \brief Creates a length quantity from a \c _mm literal.
 *
 * \param[in] value Literal value expressed in mm.
 * \return Length converted to its canonical GGEMS representation.
 */
consteval auto operator""_mm(long double value) -> Length {
  return detail::MakeLiteralQuantity<Length>(value, "mm");
}

/*!
 * \brief Creates a length quantity from a \c _cm literal.
 *
 * \param[in] value Literal value expressed in cm.
 * \return Length converted to its canonical GGEMS representation.
 */
consteval auto operator""_cm(unsigned long long value) -> Length {
  return detail::MakeLiteralQuantity<Length>(value, "cm");
}

/*!
 * \brief Creates a length quantity from a \c _cm literal.
 *
 * \param[in] value Literal value expressed in cm.
 * \return Length converted to its canonical GGEMS representation.
 */
consteval auto operator""_cm(long double value) -> Length {
  return detail::MakeLiteralQuantity<Length>(value, "cm");
}

/*!
 * \brief Creates a length quantity from a \c _m literal.
 *
 * \param[in] value Literal value expressed in m.
 * \return Length converted to its canonical GGEMS representation.
 */
consteval auto operator""_m(unsigned long long value) -> Length {
  return detail::MakeLiteralQuantity<Length>(value, "m");
}

/*!
 * \brief Creates a length quantity from a \c _m literal.
 *
 * \param[in] value Literal value expressed in m.
 * \return Length converted to its canonical GGEMS representation.
 */
consteval auto operator""_m(long double value) -> Length {
  return detail::MakeLiteralQuantity<Length>(value, "m");
}

/*!
 * \brief Creates a length quantity from a \c _km literal.
 *
 * \param[in] value Literal value expressed in km.
 * \return Length converted to its canonical GGEMS representation.
 */
consteval auto operator""_km(unsigned long long value) -> Length {
  return detail::MakeLiteralQuantity<Length>(value, "km");
}

/*!
 * \brief Creates a length quantity from a \c _km literal.
 *
 * \param[in] value Literal value expressed in km.
 * \return Length converted to its canonical GGEMS representation.
 */
consteval auto operator""_km(long double value) -> Length {
  return detail::MakeLiteralQuantity<Length>(value, "km");
}
} // namespace ggems::units
