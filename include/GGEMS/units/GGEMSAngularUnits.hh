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
 * Literals use MakeQuantity() during constant evaluation. Integral
 * representations round floating inputs to the nearest canonical integer, with
 * halfway values away from zero. A failed conversion makes the literal invalid
 * in a constant expression. Direct aggregate construction bypasses conversion
 * checks.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <array>
#include <cstdint>
#include <numbers>
#include <string_view>

#include "GGEMS/units/GGEMSQuantity.hh"
#include "GGEMS/units/GGEMSUnitConversion.hh"

namespace ggems::units {

/*! \brief Marker type identifying the angle unit registry. */
struct AngleUnitSet {};

/*!
 * \brief Defines the supported angle units and their canonical scale factors.
 */
template <> struct UnitRegistry<AngleUnitSet> {
  /*! \brief Registered unit definitions for this quantity family. */
  static constexpr std::array<UnitDefinition, 2U> units{{
    {.symbol = "rad", .scale = DecimalScale(12), .automatic_display = false},
    {.symbol = "deg",
     .scale = SpecialScale(std::numbers::pi_v<long double> * 1.0e12L / 180.0L)},
  }};
};

/*! \brief Tag type identifying angle quantities. */
struct AngleTag {};

/*! \brief Defines conversion and formatting traits for Angle quantities. */
template <> struct QuantityTraits<AngleTag> {
  /*! \brief Unit registry associated with this quantity type. */
  using unit_set = AngleUnitSet;

  /*! \brief Allowed sign domain for this quantity type. */
  static constexpr QuantityDomain domain{QuantityDomain::Signed};

  /*! \brief Formatting policy used for human-readable output. */
  static constexpr QuantityFormatPolicy format_policy{
    QuantityFormatPolicy::FixedUnit};

  /*!
   * \brief Fixed display unit, or an empty string when the policy selects units
   * automatically.
   */
  static constexpr std::string_view fixed_display_unit{"deg"};

  /*!
   * \brief Default number of digits after the decimal point for formatted
   * output.
   */
  static constexpr std::int8_t default_precision{3};
};

/*! \brief Signed angle stored in integer picoradians (10^-12 rad). */
using Angle = Quantity<AngleTag, std::int64_t>;

/*!
 * \brief Creates an angle from checked radians, nearest with ties away.
 *
 * \param[in] radians Angle value in radians, without angular wrapping.
 * \return Angle rounded to the canonical integer picoradian lattice.
 * \throws std::bad_expected_access<UnitConversionError> If the value is
 * nonfinite or cannot be represented after conversion.
 */
constexpr auto MakeRadians(long double radians) -> Angle {
  return MakeQuantity<Angle>(radians, "rad").value();
}

/*!
 * \brief Creates an angle from checked degrees, nearest with ties away.
 *
 * \param[in] degrees Angle value in degrees, without angular wrapping.
 * \return Angle rounded to the canonical integer picoradian lattice.
 * \throws std::bad_expected_access<UnitConversionError> If the value is
 * nonfinite or cannot be represented after conversion.
 */
constexpr auto MakeDegrees(long double degrees) -> Angle {
  return MakeQuantity<Angle>(degrees, "deg").value();
}

/*!
 * \brief Returns an angle expressed in radians.
 *
 * \param[in] angle Angle to convert.
 * \return Angle value in radians.
 */
constexpr auto ToRadians(Angle angle) noexcept -> long double {
  return static_cast<long double>(angle.value) /
         detail::ScaleFactor(FindUnit<AngleUnitSet>("rad")->scale);
}

/*!
 * \brief Returns an angle expressed in degrees.
 *
 * \param[in] angle Angle to convert.
 * \return Angle value in degrees.
 */
constexpr auto ToDegrees(Angle angle) noexcept -> long double {
  return static_cast<long double>(angle.value) /
         detail::ScaleFactor(FindUnit<AngleUnitSet>("deg")->scale);
}

/*!
 * \brief Creates an angle quantity from a \c _rad literal.
 *
 * \param[in] value Literal value expressed in rad.
 * \return Angle converted to its canonical GGEMS representation.
 */
consteval auto operator""_rad(long double value) -> Angle {
  return detail::MakeLiteralQuantity<Angle>(value, "rad");
}

/*!
 * \brief Creates an angle quantity from a \c _rad literal.
 *
 * \param[in] value Literal value expressed in rad.
 * \return Angle converted to its canonical GGEMS representation.
 */
consteval auto operator""_rad(unsigned long long value) -> Angle {
  return detail::MakeLiteralQuantity<Angle>(value, "rad");
}

/*!
 * \brief Creates an angle quantity from a \c _deg literal.
 *
 * \param[in] value Literal value expressed in deg.
 * \return Angle converted to its canonical GGEMS representation.
 */
consteval auto operator""_deg(long double value) -> Angle {
  return detail::MakeLiteralQuantity<Angle>(value, "deg");
}

/*!
 * \brief Creates an angle quantity from a \c _deg literal.
 *
 * \param[in] value Literal value expressed in deg.
 * \return Angle converted to its canonical GGEMS representation.
 */
consteval auto operator""_deg(unsigned long long value) -> Angle {
  return detail::MakeLiteralQuantity<Angle>(value, "deg");
}

} // namespace ggems::units
