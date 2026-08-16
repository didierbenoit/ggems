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
 * \brief Declares strongly typed mass units and literals.
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

#include "GGEMS/core/units/GGEMSQuantity.hh"
#include "GGEMS/core/units/GGEMSUnitConversion.hh"

namespace ggems::units {

/*!
 * \brief Marker type identifying the mass unit registry.
 */
struct MassUnitSet {};

/*!
 * \brief Defines the supported mass units and their canonical scale factors.
 */
template <> struct UnitRegistry<MassUnitSet> {
  /*!
   * \brief Registered unit definitions for this quantity family.
   */
  static constexpr std::array<UnitDefinition, 6U> units{{
      {.symbol = "pg",
       .scale = DecimalScale(0)},
      {.symbol = "ng",
       .scale = DecimalScale(3)},
      {.symbol = "ug",
       .scale = DecimalScale(6),
       .unicode_symbol = "µg"},
      {.symbol = "mg",
       .scale = DecimalScale(9)},
      {.symbol = "g",
       .scale = DecimalScale(12)},
      {.symbol = "kg",
       .scale = DecimalScale(15)},
  }};
};

/*!
 * \brief Tag type identifying mass quantities.
 */
struct MassTag {};

/*!
 * \brief Defines conversion and formatting traits for Mass quantities.
 */
template <> struct QuantityTraits<MassTag> {
  /*!
   * \brief Unit registry associated with this quantity type.
   */
  using unit_set = MassUnitSet;
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
 * \brief Strongly typed mass quantity stored canonically in picograms.
 */
using Mass = Quantity<MassTag, std::uint64_t>;

static_assert(ValidateUnitSet<MassUnitSet>());
static_assert(ValidateQuantityTraits<MassTag, std::uint64_t>());

/*!
 * \brief Creates a mass quantity from a \c _pg literal.
 *
 * \param[in] value Literal value expressed in pg.
 * \return Mass converted to its canonical GGEMS representation.
 */
consteval auto operator""_pg(unsigned long long value) -> Mass {
  return detail::MakeLiteralQuantity<Mass>(value, "pg");
}

/*!
 * \brief Creates a mass quantity from a \c _pg literal.
 *
 * \param[in] value Literal value expressed in pg.
 * \return Mass converted to its canonical GGEMS representation.
 */
consteval auto operator""_pg(long double value) -> Mass {
  return detail::MakeLiteralQuantity<Mass>(value, "pg");
}

/*!
 * \brief Creates a mass quantity from a \c _ng literal.
 *
 * \param[in] value Literal value expressed in ng.
 * \return Mass converted to its canonical GGEMS representation.
 */
consteval auto operator""_ng(unsigned long long value) -> Mass {
  return detail::MakeLiteralQuantity<Mass>(value, "ng");
}

/*!
 * \brief Creates a mass quantity from a \c _ng literal.
 *
 * \param[in] value Literal value expressed in ng.
 * \return Mass converted to its canonical GGEMS representation.
 */
consteval auto operator""_ng(long double value) -> Mass {
  return detail::MakeLiteralQuantity<Mass>(value, "ng");
}

/*!
 * \brief Creates a mass quantity from a \c _ug literal.
 *
 * \param[in] value Literal value expressed in ug.
 * \return Mass converted to its canonical GGEMS representation.
 */
consteval auto operator""_ug(unsigned long long value) -> Mass {
  return detail::MakeLiteralQuantity<Mass>(value, "ug");
}

/*!
 * \brief Creates a mass quantity from a \c _ug literal.
 *
 * \param[in] value Literal value expressed in ug.
 * \return Mass converted to its canonical GGEMS representation.
 */
consteval auto operator""_ug(long double value) -> Mass {
  return detail::MakeLiteralQuantity<Mass>(value, "ug");
}

/*!
 * \brief Creates a mass quantity from a \c _mg literal.
 *
 * \param[in] value Literal value expressed in mg.
 * \return Mass converted to its canonical GGEMS representation.
 */
consteval auto operator""_mg(unsigned long long value) -> Mass {
  return detail::MakeLiteralQuantity<Mass>(value, "mg");
}

/*!
 * \brief Creates a mass quantity from a \c _mg literal.
 *
 * \param[in] value Literal value expressed in mg.
 * \return Mass converted to its canonical GGEMS representation.
 */
consteval auto operator""_mg(long double value) -> Mass {
  return detail::MakeLiteralQuantity<Mass>(value, "mg");
}

/*!
 * \brief Creates a mass quantity from a \c _g literal.
 *
 * \param[in] value Literal value expressed in g.
 * \return Mass converted to its canonical GGEMS representation.
 */
consteval auto operator""_g(unsigned long long value) -> Mass {
  return detail::MakeLiteralQuantity<Mass>(value, "g");
}

/*!
 * \brief Creates a mass quantity from a \c _g literal.
 *
 * \param[in] value Literal value expressed in g.
 * \return Mass converted to its canonical GGEMS representation.
 */
consteval auto operator""_g(long double value) -> Mass {
  return detail::MakeLiteralQuantity<Mass>(value, "g");
}

/*!
 * \brief Creates a mass quantity from a \c _kg literal.
 *
 * \param[in] value Literal value expressed in kg.
 * \return Mass converted to its canonical GGEMS representation.
 */
consteval auto operator""_kg(unsigned long long value) -> Mass {
  return detail::MakeLiteralQuantity<Mass>(value, "kg");
}

/*!
 * \brief Creates a mass quantity from a \c _kg literal.
 *
 * \param[in] value Literal value expressed in kg.
 * \return Mass converted to its canonical GGEMS representation.
 */
consteval auto operator""_kg(long double value) -> Mass {
  return detail::MakeLiteralQuantity<Mass>(value, "kg");
}
} // namespace ggems::units
