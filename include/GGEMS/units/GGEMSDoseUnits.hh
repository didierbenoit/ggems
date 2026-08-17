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
 * \brief Declares strongly typed absorbed-dose units and literals.
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
 * \brief Marker type identifying the absorbed dose unit registry.
 */
struct DoseUnitSet {};

/*!
 * \brief Defines the supported absorbed dose units and their canonical scale factors.
 */
template <> struct UnitRegistry<DoseUnitSet> {
  /*!
   * \brief Registered unit definitions for this quantity family.
   */
  static constexpr std::array<UnitDefinition, 4U> units{{
      {.symbol = "meV/pg",
       .scale = DecimalScale(0),
       .automatic_display = false},
      {.symbol = "Gy",
       .scale = SpecialScale(1.0L / 1.602176634e-7L)},
      {.symbol = "mGy",
       .scale = SpecialScale(1.0e-3L / 1.602176634e-7L)},
      {.symbol = "uGy",
       .scale = SpecialScale(1.0e-6L / 1.602176634e-7L),
       .unicode_symbol = "µGy"},
  }};
};

/*!
 * \brief Tag type identifying absorbed-dose quantities.
 */
struct DoseTag {};

/*!
 * \brief Defines conversion and formatting traits for Dose quantities.
 */
template <> struct QuantityTraits<DoseTag> {
  /*!
   * \brief Unit registry associated with this quantity type.
   */
  using unit_set = DoseUnitSet;
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
 * \brief Strongly typed absorbed-dose quantity stored canonically in milli-electron-volts per picogram.
 */
using Dose = Quantity<DoseTag, std::uint64_t>;

static_assert(ValidateUnitSet<DoseUnitSet>());
static_assert(ValidateQuantityTraits<DoseTag, std::uint64_t>());

/*!
 * \brief Creates a absorbed-dose quantity from a \c _meV_pg literal.
 *
 * \param[in] value Literal value expressed in meV/pg.
 * \return Dose converted to its canonical GGEMS representation.
 */
consteval auto operator""_meV_pg(unsigned long long value) -> Dose {
  return detail::MakeLiteralQuantity<Dose>(value, "meV/pg");
}

/*!
 * \brief Creates a absorbed-dose quantity from a \c _meV_pg literal.
 *
 * \param[in] value Literal value expressed in meV/pg.
 * \return Dose converted to its canonical GGEMS representation.
 */
consteval auto operator""_meV_pg(long double value) -> Dose {
  return detail::MakeLiteralQuantity<Dose>(value, "meV/pg");
}

/*!
 * \brief Creates a absorbed-dose quantity from a \c _Gy literal.
 *
 * \param[in] value Literal value expressed in Gy.
 * \return Dose converted to its canonical GGEMS representation.
 */
consteval auto operator""_Gy(unsigned long long value) -> Dose {
  return detail::MakeLiteralQuantity<Dose>(value, "Gy");
}

/*!
 * \brief Creates a absorbed-dose quantity from a \c _Gy literal.
 *
 * \param[in] value Literal value expressed in Gy.
 * \return Dose converted to its canonical GGEMS representation.
 */
consteval auto operator""_Gy(long double value) -> Dose {
  return detail::MakeLiteralQuantity<Dose>(value, "Gy");
}

/*!
 * \brief Creates a absorbed-dose quantity from a \c _mGy literal.
 *
 * \param[in] value Literal value expressed in mGy.
 * \return Dose converted to its canonical GGEMS representation.
 */
consteval auto operator""_mGy(unsigned long long value) -> Dose {
  return detail::MakeLiteralQuantity<Dose>(value, "mGy");
}

/*!
 * \brief Creates a absorbed-dose quantity from a \c _mGy literal.
 *
 * \param[in] value Literal value expressed in mGy.
 * \return Dose converted to its canonical GGEMS representation.
 */
consteval auto operator""_mGy(long double value) -> Dose {
  return detail::MakeLiteralQuantity<Dose>(value, "mGy");
}

/*!
 * \brief Creates a absorbed-dose quantity from a \c _uGy literal.
 *
 * \param[in] value Literal value expressed in uGy.
 * \return Dose converted to its canonical GGEMS representation.
 */
consteval auto operator""_uGy(unsigned long long value) -> Dose {
  return detail::MakeLiteralQuantity<Dose>(value, "uGy");
}

/*!
 * \brief Creates a absorbed-dose quantity from a \c _uGy literal.
 *
 * \param[in] value Literal value expressed in uGy.
 * \return Dose converted to its canonical GGEMS representation.
 */
consteval auto operator""_uGy(long double value) -> Dose {
  return detail::MakeLiteralQuantity<Dose>(value, "uGy");
}
} // namespace ggems::units
