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
 * \brief Declares strongly typed area units and literals.
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
 * \brief Marker type identifying the area unit registry.
 */
struct AreaUnitSet {};

/*!
 * \brief Defines the supported area units and their canonical scale factors.
 */
template <> struct UnitRegistry<AreaUnitSet> {
  /*!
   * \brief Registered unit definitions for this quantity family.
   */
  static constexpr std::array<UnitDefinition, 7U> units{{
      {.symbol = "pm2",
       .scale = DecimalScale(0),
       .unicode_symbol = "pm²"},
      {.symbol = "nm2",
       .scale = DecimalScale(6),
       .unicode_symbol = "nm²"},
      {.symbol = "um2",
       .scale = DecimalScale(12),
       .unicode_symbol = "µm²"},
      {.symbol = "mm2",
       .scale = DecimalScale(18),
       .unicode_symbol = "mm²"},
      {.symbol = "cm2",
       .scale = DecimalScale(20),
       .unicode_symbol = "cm²",
       .automatic_display = false},
      {.symbol = "m2",
       .scale = DecimalScale(24),
       .unicode_symbol = "m²"},
      {.symbol = "km2",
       .scale = DecimalScale(30),
       .unicode_symbol = "km²"},
  }};
};

/*!
 * \brief Tag type identifying area quantities.
 */
struct AreaTag {};

/*!
 * \brief Defines conversion and formatting traits for Area quantities.
 */
template <> struct QuantityTraits<AreaTag> {
  /*!
   * \brief Unit registry associated with this quantity type.
   */
  using unit_set = AreaUnitSet;
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
 * \brief Strongly typed area quantity stored canonically in square picometers.
 */
using Area = Quantity<AreaTag, long double>;

static_assert(ValidateUnitSet<AreaUnitSet>());
static_assert(ValidateQuantityTraits<AreaTag, long double>());

/*!
 * \brief Creates a area quantity from a \c _pm2 literal.
 *
 * \param[in] value Literal value expressed in pm2.
 * \return Area converted to its canonical GGEMS representation.
 */
consteval auto operator""_pm2(unsigned long long value) -> Area {
  return detail::MakeLiteralQuantity<Area>(value, "pm2");
}

/*!
 * \brief Creates a area quantity from a \c _pm2 literal.
 *
 * \param[in] value Literal value expressed in pm2.
 * \return Area converted to its canonical GGEMS representation.
 */
consteval auto operator""_pm2(long double value) -> Area {
  return detail::MakeLiteralQuantity<Area>(value, "pm2");
}

/*!
 * \brief Creates a area quantity from a \c _nm2 literal.
 *
 * \param[in] value Literal value expressed in nm2.
 * \return Area converted to its canonical GGEMS representation.
 */
consteval auto operator""_nm2(unsigned long long value) -> Area {
  return detail::MakeLiteralQuantity<Area>(value, "nm2");
}

/*!
 * \brief Creates a area quantity from a \c _nm2 literal.
 *
 * \param[in] value Literal value expressed in nm2.
 * \return Area converted to its canonical GGEMS representation.
 */
consteval auto operator""_nm2(long double value) -> Area {
  return detail::MakeLiteralQuantity<Area>(value, "nm2");
}

/*!
 * \brief Creates a area quantity from a \c _um2 literal.
 *
 * \param[in] value Literal value expressed in um2.
 * \return Area converted to its canonical GGEMS representation.
 */
consteval auto operator""_um2(unsigned long long value) -> Area {
  return detail::MakeLiteralQuantity<Area>(value, "um2");
}

/*!
 * \brief Creates a area quantity from a \c _um2 literal.
 *
 * \param[in] value Literal value expressed in um2.
 * \return Area converted to its canonical GGEMS representation.
 */
consteval auto operator""_um2(long double value) -> Area {
  return detail::MakeLiteralQuantity<Area>(value, "um2");
}

/*!
 * \brief Creates a area quantity from a \c _mm2 literal.
 *
 * \param[in] value Literal value expressed in mm2.
 * \return Area converted to its canonical GGEMS representation.
 */
consteval auto operator""_mm2(unsigned long long value) -> Area {
  return detail::MakeLiteralQuantity<Area>(value, "mm2");
}

/*!
 * \brief Creates a area quantity from a \c _mm2 literal.
 *
 * \param[in] value Literal value expressed in mm2.
 * \return Area converted to its canonical GGEMS representation.
 */
consteval auto operator""_mm2(long double value) -> Area {
  return detail::MakeLiteralQuantity<Area>(value, "mm2");
}

/*!
 * \brief Creates a area quantity from a \c _cm2 literal.
 *
 * \param[in] value Literal value expressed in cm2.
 * \return Area converted to its canonical GGEMS representation.
 */
consteval auto operator""_cm2(unsigned long long value) -> Area {
  return detail::MakeLiteralQuantity<Area>(value, "cm2");
}

/*!
 * \brief Creates a area quantity from a \c _cm2 literal.
 *
 * \param[in] value Literal value expressed in cm2.
 * \return Area converted to its canonical GGEMS representation.
 */
consteval auto operator""_cm2(long double value) -> Area {
  return detail::MakeLiteralQuantity<Area>(value, "cm2");
}

/*!
 * \brief Creates a area quantity from a \c _m2 literal.
 *
 * \param[in] value Literal value expressed in m2.
 * \return Area converted to its canonical GGEMS representation.
 */
consteval auto operator""_m2(unsigned long long value) -> Area {
  return detail::MakeLiteralQuantity<Area>(value, "m2");
}

/*!
 * \brief Creates a area quantity from a \c _m2 literal.
 *
 * \param[in] value Literal value expressed in m2.
 * \return Area converted to its canonical GGEMS representation.
 */
consteval auto operator""_m2(long double value) -> Area {
  return detail::MakeLiteralQuantity<Area>(value, "m2");
}

/*!
 * \brief Creates a area quantity from a \c _km2 literal.
 *
 * \param[in] value Literal value expressed in km2.
 * \return Area converted to its canonical GGEMS representation.
 */
consteval auto operator""_km2(unsigned long long value) -> Area {
  return detail::MakeLiteralQuantity<Area>(value, "km2");
}

/*!
 * \brief Creates a area quantity from a \c _km2 literal.
 *
 * \param[in] value Literal value expressed in km2.
 * \return Area converted to its canonical GGEMS representation.
 */
consteval auto operator""_km2(long double value) -> Area {
  return detail::MakeLiteralQuantity<Area>(value, "km2");
}
} // namespace ggems::units
