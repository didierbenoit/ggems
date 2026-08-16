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
 * \brief Declares strongly typed activity units and literals.
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
 * \brief Marker type identifying the activity unit registry.
 */
struct ActivityUnitSet {};

/*!
 * \brief Defines the supported activity units and their canonical scale factors.
 */
template <> struct UnitRegistry<ActivityUnitSet> {
  /*!
   * \brief Registered unit definitions for this quantity family.
   */
  static constexpr std::array<UnitDefinition, 8U> units{{
      {.symbol = "Bq", .scale = DecimalScale(0)},
      {.symbol = "kBq", .scale = DecimalScale(3)},
      {.symbol = "MBq", .scale = DecimalScale(6)},
      {.symbol = "GBq", .scale = DecimalScale(9)},
      {.symbol = "TBq", .scale = DecimalScale(12)},
      {.symbol = "Ci",
       .scale = DecimalScale(0, 37'000'000'000ULL),
       .automatic_display = false},
      {.symbol = "mCi",
       .scale = DecimalScale(0, 37'000'000ULL),
       .automatic_display = false},
      {.symbol = "uCi",
       .scale = DecimalScale(0, 37'000ULL),
       .unicode_symbol = "µCi",
       .automatic_display = false},
  }};
};

/*!
 * \brief Tag type identifying activity quantities.
 */
struct ActivityTag {};

/*!
 * \brief Defines conversion and formatting traits for Activity quantities.
 */
template <> struct QuantityTraits<ActivityTag> {
  /*!
   * \brief Unit registry associated with this quantity type.
   */
  using unit_set = ActivityUnitSet;
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
 * \brief Strongly typed activity quantity stored canonically in becquerels.
 */
using Activity = Quantity<ActivityTag, long double>;

static_assert(ValidateUnitSet<ActivityUnitSet>());
static_assert(ValidateQuantityTraits<ActivityTag, long double>());

/*!
 * \brief Creates a activity quantity from a \c _Bq literal.
 *
 * \param[in] value Literal value expressed in Bq.
 * \return Activity converted to its canonical GGEMS representation.
 */
consteval auto operator""_Bq(unsigned long long value) -> Activity {
  return detail::MakeLiteralQuantity<Activity>(value, "Bq");
}

/*!
 * \brief Creates a activity quantity from a \c _Bq literal.
 *
 * \param[in] value Literal value expressed in Bq.
 * \return Activity converted to its canonical GGEMS representation.
 */
consteval auto operator""_Bq(long double value) -> Activity {
  return detail::MakeLiteralQuantity<Activity>(value, "Bq");
}

/*!
 * \brief Creates a activity quantity from a \c _kBq literal.
 *
 * \param[in] value Literal value expressed in kBq.
 * \return Activity converted to its canonical GGEMS representation.
 */
consteval auto operator""_kBq(unsigned long long value) -> Activity {
  return detail::MakeLiteralQuantity<Activity>(value, "kBq");
}

/*!
 * \brief Creates a activity quantity from a \c _kBq literal.
 *
 * \param[in] value Literal value expressed in kBq.
 * \return Activity converted to its canonical GGEMS representation.
 */
consteval auto operator""_kBq(long double value) -> Activity {
  return detail::MakeLiteralQuantity<Activity>(value, "kBq");
}

/*!
 * \brief Creates a activity quantity from a \c _MBq literal.
 *
 * \param[in] value Literal value expressed in MBq.
 * \return Activity converted to its canonical GGEMS representation.
 */
consteval auto operator""_MBq(unsigned long long value) -> Activity {
  return detail::MakeLiteralQuantity<Activity>(value, "MBq");
}

/*!
 * \brief Creates a activity quantity from a \c _MBq literal.
 *
 * \param[in] value Literal value expressed in MBq.
 * \return Activity converted to its canonical GGEMS representation.
 */
consteval auto operator""_MBq(long double value) -> Activity {
  return detail::MakeLiteralQuantity<Activity>(value, "MBq");
}

/*!
 * \brief Creates a activity quantity from a \c _GBq literal.
 *
 * \param[in] value Literal value expressed in GBq.
 * \return Activity converted to its canonical GGEMS representation.
 */
consteval auto operator""_GBq(unsigned long long value) -> Activity {
  return detail::MakeLiteralQuantity<Activity>(value, "GBq");
}

/*!
 * \brief Creates a activity quantity from a \c _GBq literal.
 *
 * \param[in] value Literal value expressed in GBq.
 * \return Activity converted to its canonical GGEMS representation.
 */
consteval auto operator""_GBq(long double value) -> Activity {
  return detail::MakeLiteralQuantity<Activity>(value, "GBq");
}

/*!
 * \brief Creates a activity quantity from a \c _TBq literal.
 *
 * \param[in] value Literal value expressed in TBq.
 * \return Activity converted to its canonical GGEMS representation.
 */
consteval auto operator""_TBq(unsigned long long value) -> Activity {
  return detail::MakeLiteralQuantity<Activity>(value, "TBq");
}

/*!
 * \brief Creates a activity quantity from a \c _TBq literal.
 *
 * \param[in] value Literal value expressed in TBq.
 * \return Activity converted to its canonical GGEMS representation.
 */
consteval auto operator""_TBq(long double value) -> Activity {
  return detail::MakeLiteralQuantity<Activity>(value, "TBq");
}

/*!
 * \brief Creates a activity quantity from a \c _Ci literal.
 *
 * \param[in] value Literal value expressed in Ci.
 * \return Activity converted to its canonical GGEMS representation.
 */
consteval auto operator""_Ci(unsigned long long value) -> Activity {
  return detail::MakeLiteralQuantity<Activity>(value, "Ci");
}

/*!
 * \brief Creates a activity quantity from a \c _Ci literal.
 *
 * \param[in] value Literal value expressed in Ci.
 * \return Activity converted to its canonical GGEMS representation.
 */
consteval auto operator""_Ci(long double value) -> Activity {
  return detail::MakeLiteralQuantity<Activity>(value, "Ci");
}

/*!
 * \brief Creates a activity quantity from a \c _mCi literal.
 *
 * \param[in] value Literal value expressed in mCi.
 * \return Activity converted to its canonical GGEMS representation.
 */
consteval auto operator""_mCi(unsigned long long value) -> Activity {
  return detail::MakeLiteralQuantity<Activity>(value, "mCi");
}

/*!
 * \brief Creates a activity quantity from a \c _mCi literal.
 *
 * \param[in] value Literal value expressed in mCi.
 * \return Activity converted to its canonical GGEMS representation.
 */
consteval auto operator""_mCi(long double value) -> Activity {
  return detail::MakeLiteralQuantity<Activity>(value, "mCi");
}

/*!
 * \brief Creates a activity quantity from a \c _uCi literal.
 *
 * \param[in] value Literal value expressed in uCi.
 * \return Activity converted to its canonical GGEMS representation.
 */
consteval auto operator""_uCi(unsigned long long value) -> Activity {
  return detail::MakeLiteralQuantity<Activity>(value, "uCi");
}

/*!
 * \brief Creates a activity quantity from a \c _uCi literal.
 *
 * \param[in] value Literal value expressed in uCi.
 * \return Activity converted to its canonical GGEMS representation.
 */
consteval auto operator""_uCi(long double value) -> Activity {
  return detail::MakeLiteralQuantity<Activity>(value, "uCi");
}
} // namespace ggems::units
