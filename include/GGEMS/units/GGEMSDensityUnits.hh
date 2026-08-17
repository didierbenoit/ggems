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
 * \brief Declares strongly typed density units and literals.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <array>
#include <cstdint>
#include <string_view>
/// \endcond

#include "GGEMS/units/GGEMSQuantity.hh"
#include "GGEMS/units/GGEMSUnitConversion.hh"

namespace ggems::units {

/*!
 * \brief Marker type identifying the density unit registry.
 */
struct DensityUnitSet {};

/*!
 * \brief Defines the supported density units and their canonical scale factors.
 */
template <> struct UnitRegistry<DensityUnitSet> {
  /*!
   * \brief Registered unit definitions for this quantity family.
   */
  static constexpr std::array<UnitDefinition, 2U> units{{
      {.symbol = "pg/pm3",
       .scale = DecimalScale(0),
       .unicode_symbol = "pg/pm³",
       .automatic_display = false},
      {.symbol = "g/cm3",
       .scale = DecimalScale(-18),
       .unicode_symbol = "g/cm³"},
  }};
};

/*!
 * \brief Tag type identifying density quantities.
 */
struct DensityTag {};

/*!
 * \brief Defines conversion and formatting traits for Density quantities.
 */
template <> struct QuantityTraits<DensityTag> {
  /*!
   * \brief Unit registry associated with this quantity type.
   */
  using unit_set = DensityUnitSet;
  /*!
   * \brief Allowed sign domain for this quantity type.
   */
  static constexpr QuantityDomain domain{QuantityDomain::NonNegative};
  /*!
   * \brief Formatting policy used for human-readable output.
   */
  static constexpr QuantityFormatPolicy format_policy{
      QuantityFormatPolicy::FixedUnit};
  /*!
   * \brief Fixed display unit, or an empty string when the policy selects units automatically.
   */
  static constexpr std::string_view fixed_display_unit{"g/cm3"};
  /*!
   * \brief Default number of digits after the decimal point for formatted output.
   */
  static constexpr std::int8_t default_precision{7};
};

/*!
 * \brief Strongly typed density quantity stored canonically in picograms per cubic picometer.
 */
using Density = Quantity<DensityTag, long double>;

static_assert(ValidateUnitSet<DensityUnitSet>());
static_assert(ValidateQuantityTraits<DensityTag, long double>());

/*!
 * \brief Creates a density quantity from a \c _pg_pm3 literal.
 *
 * \param[in] value Literal value expressed in pg/pm3.
 * \return Density converted to its canonical GGEMS representation.
 */
consteval auto operator""_pg_pm3(unsigned long long value) -> Density {
  return detail::MakeLiteralQuantity<Density>(value, "pg/pm3");
}

/*!
 * \brief Creates a density quantity from a \c _pg_pm3 literal.
 *
 * \param[in] value Literal value expressed in pg/pm3.
 * \return Density converted to its canonical GGEMS representation.
 */
consteval auto operator""_pg_pm3(long double value) -> Density {
  return detail::MakeLiteralQuantity<Density>(value, "pg/pm3");
}

/*!
 * \brief Creates a density quantity from a \c _g_cm3 literal.
 *
 * \param[in] value Literal value expressed in g/cm3.
 * \return Density converted to its canonical GGEMS representation.
 */
consteval auto operator""_g_cm3(unsigned long long value) -> Density {
  return detail::MakeLiteralQuantity<Density>(value, "g/cm3");
}

/*!
 * \brief Creates a density quantity from a \c _g_cm3 literal.
 *
 * \param[in] value Literal value expressed in g/cm3.
 * \return Density converted to its canonical GGEMS representation.
 */
consteval auto operator""_g_cm3(long double value) -> Density {
  return detail::MakeLiteralQuantity<Density>(value, "g/cm3");
}
} // namespace ggems::units
