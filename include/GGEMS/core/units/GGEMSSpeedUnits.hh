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
 * \brief Declares strongly typed speed units, literals, and length-over-time construction.
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
#include "GGEMS/core/units/GGEMSLengthUnits.hh"
#include "GGEMS/core/units/GGEMSTimeUnits.hh"

namespace ggems::units {

/*!
 * \brief Marker type identifying the speed unit registry.
 */
struct SpeedUnitSet {};

/*!
 * \brief Defines the supported speed units and their canonical scale factors.
 */
template <> struct UnitRegistry<SpeedUnitSet> {
  /*!
   * \brief Registered unit definitions for this quantity family.
   */
  static constexpr std::array<UnitDefinition, 2U> units{{
      {.symbol = "pm/ps",
       .scale = DecimalScale(0),
       .automatic_display = false},
      {.symbol = "m/s",
       .scale = DecimalScale(0)},
  }};
};

/*!
 * \brief Tag type identifying speed quantities.
 */
struct SpeedTag {};

/*!
 * \brief Defines conversion and formatting traits for Speed quantities.
 */
template <> struct QuantityTraits<SpeedTag> {
  /*!
   * \brief Unit registry associated with this quantity type.
   */
  using unit_set = SpeedUnitSet;
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
  static constexpr std::string_view fixed_display_unit{"m/s"};
  /*!
   * \brief Default number of digits after the decimal point for formatted output.
   */
  static constexpr std::int8_t default_precision{7};
};

/*!
 * \brief Strongly typed speed quantity stored canonically in picometers per picosecond.
 */
using Speed = Quantity<SpeedTag, long double>;

static_assert(ValidateUnitSet<SpeedUnitSet>());
static_assert(ValidateQuantityTraits<SpeedTag, long double>());

/*!
 * \brief Creates a speed quantity from a \c _pm_ps literal.
 *
 * \param[in] value Literal value expressed in pm/ps.
 * \return Speed converted to its canonical GGEMS representation.
 */
consteval auto operator""_pm_ps(unsigned long long value) -> Speed {
  return detail::MakeLiteralQuantity<Speed>(value, "pm/ps");
}

/*!
 * \brief Creates a speed quantity from a \c _pm_ps literal.
 *
 * \param[in] value Literal value expressed in pm/ps.
 * \return Speed converted to its canonical GGEMS representation.
 */
consteval auto operator""_pm_ps(long double value) -> Speed {
  return detail::MakeLiteralQuantity<Speed>(value, "pm/ps");
}

/*!
 * \brief Creates a speed quantity from a \c _m_s literal.
 *
 * \param[in] value Literal value expressed in m/s.
 * \return Speed converted to its canonical GGEMS representation.
 */
consteval auto operator""_m_s(unsigned long long value) -> Speed {
  return detail::MakeLiteralQuantity<Speed>(value, "m/s");
}

/*!
 * \brief Creates a speed quantity from a \c _m_s literal.
 *
 * \param[in] value Literal value expressed in m/s.
 * \return Speed converted to its canonical GGEMS representation.
 */
consteval auto operator""_m_s(long double value) -> Speed {
  return detail::MakeLiteralQuantity<Speed>(value, "m/s");
}

/*!
 * \brief Computes a speed from a length and a duration.
 *
 * \param[in] length Distance traveled.
 * \param[in] duration Elapsed duration.
 * \return Speed computed from the canonical length and time representations.
 */
inline auto MakeSpeed(Length length, Time duration) noexcept -> Speed {
  return Speed{static_cast<long double>(length.value) /
               static_cast<long double>(duration.value)};
}
} // namespace ggems::units
