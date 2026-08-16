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
 * \brief Declares strongly typed duration and time-point units and literals.
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
 * \brief Marker type identifying the time unit registry.
 */
struct TimeUnitSet {};

/*!
 * \brief Defines the supported time units and their canonical scale factors.
 */
template <> struct UnitRegistry<TimeUnitSet> {
  /*!
   * \brief Registered unit definitions for this quantity family.
   */
  static constexpr std::array<UnitDefinition, 7U> units{{
      {.symbol = "ps",
       .scale = DecimalScale(0)},
      {.symbol = "ns",
       .scale = DecimalScale(3)},
      {.symbol = "us",
       .scale = DecimalScale(6),
       .unicode_symbol = "µs"},
      {.symbol = "ms",
       .scale = DecimalScale(9)},
      {.symbol = "s",
       .scale = DecimalScale(12)},
      {.symbol = "min",
       .scale = DecimalScale(12, 60ULL),
       .automatic_display = false},
      {.symbol = "h",
       .scale = DecimalScale(12, 3'600ULL),
       .automatic_display = false},
  }};
};

/*!
 * \brief Tag type identifying duration quantities.
 */
struct DurationTag {};

/*!
 * \brief Defines conversion and formatting traits for Duration quantities.
 */
template <> struct QuantityTraits<DurationTag> {
  /*!
   * \brief Unit registry associated with this quantity type.
   */
  using unit_set = TimeUnitSet;
  /*!
   * \brief Allowed sign domain for this quantity type.
   */
  static constexpr QuantityDomain domain{QuantityDomain::NonNegative};
  /*!
   * \brief Formatting policy used for human-readable output.
   */
  static constexpr QuantityFormatPolicy format_policy{
      QuantityFormatPolicy::DurationBreakdown};
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
 * \brief Tag type identifying time-point quantities.
 */
struct TimePointTag {};

/*!
 * \brief Defines conversion and formatting traits for TimePoint quantities.
 */
template <> struct QuantityTraits<TimePointTag> {
  /*!
   * \brief Unit registry associated with this quantity type.
   */
  using unit_set = TimeUnitSet;
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
 * \brief Strongly typed duration stored canonically in picoseconds.
 */
using Duration = Quantity<DurationTag, std::uint64_t>;
/*!
 * \brief Strongly typed nonnegative time point stored canonically in picoseconds.
 */
using TimePoint = Quantity<TimePointTag, std::uint64_t>;
/*!
 * \brief Compatibility alias for Duration.
 */
using Time = Duration;

static_assert(ValidateUnitSet<TimeUnitSet>());
static_assert(ValidateQuantityTraits<DurationTag, std::uint64_t>());
static_assert(ValidateQuantityTraits<TimePointTag, std::uint64_t>());

/*!
 * \brief Creates a duration quantity from a \c _ps literal.
 *
 * \param[in] value Literal value expressed in ps.
 * \return Time converted to its canonical GGEMS representation.
 */
consteval auto operator""_ps(unsigned long long value) -> Time {
  return detail::MakeLiteralQuantity<Time>(value, "ps");
}

/*!
 * \brief Creates a duration quantity from a \c _ps literal.
 *
 * \param[in] value Literal value expressed in ps.
 * \return Time converted to its canonical GGEMS representation.
 */
consteval auto operator""_ps(long double value) -> Time {
  return detail::MakeLiteralQuantity<Time>(value, "ps");
}

/*!
 * \brief Creates a duration quantity from a \c _ns literal.
 *
 * \param[in] value Literal value expressed in ns.
 * \return Time converted to its canonical GGEMS representation.
 */
consteval auto operator""_ns(unsigned long long value) -> Time {
  return detail::MakeLiteralQuantity<Time>(value, "ns");
}

/*!
 * \brief Creates a duration quantity from a \c _ns literal.
 *
 * \param[in] value Literal value expressed in ns.
 * \return Time converted to its canonical GGEMS representation.
 */
consteval auto operator""_ns(long double value) -> Time {
  return detail::MakeLiteralQuantity<Time>(value, "ns");
}

/*!
 * \brief Creates a duration quantity from a \c _us literal.
 *
 * \param[in] value Literal value expressed in us.
 * \return Time converted to its canonical GGEMS representation.
 */
consteval auto operator""_us(unsigned long long value) -> Time {
  return detail::MakeLiteralQuantity<Time>(value, "us");
}

/*!
 * \brief Creates a duration quantity from a \c _us literal.
 *
 * \param[in] value Literal value expressed in us.
 * \return Time converted to its canonical GGEMS representation.
 */
consteval auto operator""_us(long double value) -> Time {
  return detail::MakeLiteralQuantity<Time>(value, "us");
}

/*!
 * \brief Creates a duration quantity from a \c _ms literal.
 *
 * \param[in] value Literal value expressed in ms.
 * \return Time converted to its canonical GGEMS representation.
 */
consteval auto operator""_ms(unsigned long long value) -> Time {
  return detail::MakeLiteralQuantity<Time>(value, "ms");
}

/*!
 * \brief Creates a duration quantity from a \c _ms literal.
 *
 * \param[in] value Literal value expressed in ms.
 * \return Time converted to its canonical GGEMS representation.
 */
consteval auto operator""_ms(long double value) -> Time {
  return detail::MakeLiteralQuantity<Time>(value, "ms");
}

/*!
 * \brief Creates a duration quantity from a \c _s literal.
 *
 * \param[in] value Literal value expressed in s.
 * \return Time converted to its canonical GGEMS representation.
 */
consteval auto operator""_s(unsigned long long value) -> Time {
  return detail::MakeLiteralQuantity<Time>(value, "s");
}

/*!
 * \brief Creates a duration quantity from a \c _s literal.
 *
 * \param[in] value Literal value expressed in s.
 * \return Time converted to its canonical GGEMS representation.
 */
consteval auto operator""_s(long double value) -> Time {
  return detail::MakeLiteralQuantity<Time>(value, "s");
}

/*!
 * \brief Creates a duration quantity from a \c _min literal.
 *
 * \param[in] value Literal value expressed in min.
 * \return Time converted to its canonical GGEMS representation.
 */
consteval auto operator""_min(unsigned long long value) -> Time {
  return detail::MakeLiteralQuantity<Time>(value, "min");
}

/*!
 * \brief Creates a duration quantity from a \c _min literal.
 *
 * \param[in] value Literal value expressed in min.
 * \return Time converted to its canonical GGEMS representation.
 */
consteval auto operator""_min(long double value) -> Time {
  return detail::MakeLiteralQuantity<Time>(value, "min");
}

/*!
 * \brief Creates a duration quantity from a \c _h literal.
 *
 * \param[in] value Literal value expressed in h.
 * \return Time converted to its canonical GGEMS representation.
 */
consteval auto operator""_h(unsigned long long value) -> Time {
  return detail::MakeLiteralQuantity<Time>(value, "h");
}

/*!
 * \brief Creates a duration quantity from a \c _h literal.
 *
 * \param[in] value Literal value expressed in h.
 * \return Time converted to its canonical GGEMS representation.
 */
consteval auto operator""_h(long double value) -> Time {
  return detail::MakeLiteralQuantity<Time>(value, "h");
}
} // namespace ggems::units
