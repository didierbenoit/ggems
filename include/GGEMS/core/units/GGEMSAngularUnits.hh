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
 * \brief Declares strongly typed angular units, conversions, and literals.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <array>
#include <cstdint>
#include <numbers>
#include <string_view>
/// \endcond

#include "GGEMS/core/units/GGEMSQuantity.hh"
#include "GGEMS/core/units/GGEMSUnitConversion.hh"

namespace ggems::units {

/*!
 * \brief Marker type identifying the angle unit registry.
 */
struct AngleUnitSet {};

/*!
 * \brief Defines the supported angle units and their canonical scale factors.
 */
template <> struct UnitRegistry<AngleUnitSet> {
  /*!
   * \brief Registered unit definitions for this quantity family.
   */
  static constexpr std::array<UnitDefinition, 2U> units{{
      {.symbol = "rad",
       .scale = DecimalScale(0),
       .automatic_display = false},
      {.symbol = "deg",
       .scale = SpecialScale(std::numbers::pi_v<long double> / 180.0L)},
  }};
};

/*!
 * \brief Tag type identifying angle quantities.
 */
struct AngleTag {};

/*!
 * \brief Defines conversion and formatting traits for Angle quantities.
 */
template <> struct QuantityTraits<AngleTag> {
  /*!
   * \brief Unit registry associated with this quantity type.
   */
  using unit_set = AngleUnitSet;
  /*!
   * \brief Allowed sign domain for this quantity type.
   */
  static constexpr QuantityDomain domain{QuantityDomain::Signed};
  /*!
   * \brief Formatting policy used for human-readable output.
   */
  static constexpr QuantityFormatPolicy format_policy{
      QuantityFormatPolicy::FixedUnit};
  /*!
   * \brief Fixed display unit, or an empty string when the policy selects units automatically.
   */
  static constexpr std::string_view fixed_display_unit{"deg"};
  /*!
   * \brief Default number of digits after the decimal point for formatted output.
   */
  static constexpr std::int8_t default_precision{3};
};

/*!
 * \brief Strongly typed angle quantity stored canonically in radians.
 */
using Angle = Quantity<AngleTag, long double>;

static_assert(ValidateUnitSet<AngleUnitSet>());
static_assert(ValidateQuantityTraits<AngleTag, long double>());

/// \cond
namespace detail {
inline constexpr long double k_pi{std::numbers::pi_v<long double>};
}
/// \endcond

/*!
 * \brief Creates an angle directly from a value expressed in radians.
 *
 * \param[in] radians Angle value in radians.
 * \return Angle storing the supplied radian value.
 */
constexpr auto MakeRadians(long double radians) noexcept -> Angle {
  return Angle{radians};
}

/*!
 * \brief Creates an angle from a value expressed in degrees.
 *
 * \param[in] degrees Angle value in degrees.
 * \return Angle converted to the canonical radian representation.
 */
constexpr auto MakeDegrees(long double degrees) noexcept -> Angle {
  return Angle{degrees *
               detail::ScaleFactor(FindUnit<AngleUnitSet>("deg")->scale)};
}

/*!
 * \brief Returns an angle expressed in radians.
 *
 * \param[in] angle Angle to convert.
 * \return Angle value in radians.
 */
constexpr auto ToRadians(Angle angle) noexcept -> long double {
  return angle.value;
}

/*!
 * \brief Returns an angle expressed in degrees.
 *
 * \param[in] angle Angle to convert.
 * \return Angle value in degrees.
 */
constexpr auto ToDegrees(Angle angle) noexcept -> long double {
  return angle.value /
         detail::ScaleFactor(FindUnit<AngleUnitSet>("deg")->scale);
}

/*!
 * \brief Creates a angle quantity from a \c _rad literal.
 *
 * \param[in] value Literal value expressed in rad.
 * \return Angle converted to its canonical GGEMS representation.
 */
consteval auto operator""_rad(long double value) -> Angle {
  return detail::MakeLiteralQuantity<Angle>(value, "rad");
}

/*!
 * \brief Creates a angle quantity from a \c _rad literal.
 *
 * \param[in] value Literal value expressed in rad.
 * \return Angle converted to its canonical GGEMS representation.
 */
consteval auto operator""_rad(unsigned long long value) -> Angle {
  return detail::MakeLiteralQuantity<Angle>(value, "rad");
}

/*!
 * \brief Creates a angle quantity from a \c _deg literal.
 *
 * \param[in] value Literal value expressed in deg.
 * \return Angle converted to its canonical GGEMS representation.
 */
consteval auto operator""_deg(long double value) -> Angle {
  return detail::MakeLiteralQuantity<Angle>(value, "deg");
}

/*!
 * \brief Creates a angle quantity from a \c _deg literal.
 *
 * \param[in] value Literal value expressed in deg.
 * \return Angle converted to its canonical GGEMS representation.
 */
consteval auto operator""_deg(unsigned long long value) -> Angle {
  return detail::MakeLiteralQuantity<Angle>(value, "deg");
}

} // namespace ggems::units
