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
 * \brief Declares strongly typed cross-section units and literals.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <cstdint>
#include <string_view>
#include <array>
/// \endcond

#include "GGEMS/units/GGEMSQuantity.hh"
#include "GGEMS/units/GGEMSUnitConversion.hh"

namespace ggems::units {

/*!
 * \brief Marker type identifying the cross section unit registry.
 */
struct CrossSectionUnitSet {};

/*!
 * \brief Defines the supported cross section units and their canonical scale factors.
 */
template <> struct UnitRegistry<CrossSectionUnitSet> {
  /*!
   * \brief Registered unit definitions for this quantity family.
   */
  static constexpr std::array<UnitDefinition, 6U> units{{
      {.symbol = "pb",
       .scale = DecimalScale(0)},
      {.symbol = "nb",
       .scale = DecimalScale(3)},
      {.symbol = "ub",
       .scale = DecimalScale(6),
       .unicode_symbol = "µb"},
      {.symbol = "mb",
       .scale = DecimalScale(9)},
      {.symbol = "barn",
       .scale = DecimalScale(12)},
      {.symbol = "kbarn",
       .scale = DecimalScale(15)},
  }};
};

/*!
 * \brief Tag type identifying cross-section quantities.
 */
struct CrossSectionTag {};

/*!
 * \brief Defines conversion and formatting traits for CrossSection quantities.
 */
template <> struct QuantityTraits<CrossSectionTag> {
  /*!
   * \brief Unit registry associated with this quantity type.
   */
  using unit_set = CrossSectionUnitSet;
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
 * \brief Strongly typed cross-section quantity stored canonically in picobarns.
 */
using CrossSection = Quantity<CrossSectionTag, std::uint64_t>;

static_assert(ValidateUnitSet<CrossSectionUnitSet>());
static_assert(ValidateQuantityTraits<CrossSectionTag, std::uint64_t>());

/*!
 * \brief Creates a cross-section quantity from a \c _pb literal.
 *
 * \param[in] value Literal value expressed in pb.
 * \return CrossSection converted to its canonical GGEMS representation.
 */
consteval auto operator""_pb(unsigned long long value) -> CrossSection {
  return detail::MakeLiteralQuantity<CrossSection>(value, "pb");
}

/*!
 * \brief Creates a cross-section quantity from a \c _pb literal.
 *
 * \param[in] value Literal value expressed in pb.
 * \return CrossSection converted to its canonical GGEMS representation.
 */
consteval auto operator""_pb(long double value) -> CrossSection {
  return detail::MakeLiteralQuantity<CrossSection>(value, "pb");
}

/*!
 * \brief Creates a cross-section quantity from a \c _nb literal.
 *
 * \param[in] value Literal value expressed in nb.
 * \return CrossSection converted to its canonical GGEMS representation.
 */
consteval auto operator""_nb(unsigned long long value) -> CrossSection {
  return detail::MakeLiteralQuantity<CrossSection>(value, "nb");
}

/*!
 * \brief Creates a cross-section quantity from a \c _nb literal.
 *
 * \param[in] value Literal value expressed in nb.
 * \return CrossSection converted to its canonical GGEMS representation.
 */
consteval auto operator""_nb(long double value) -> CrossSection {
  return detail::MakeLiteralQuantity<CrossSection>(value, "nb");
}

/*!
 * \brief Creates a cross-section quantity from a \c _ub literal.
 *
 * \param[in] value Literal value expressed in ub.
 * \return CrossSection converted to its canonical GGEMS representation.
 */
consteval auto operator""_ub(unsigned long long value) -> CrossSection {
  return detail::MakeLiteralQuantity<CrossSection>(value, "ub");
}

/*!
 * \brief Creates a cross-section quantity from a \c _ub literal.
 *
 * \param[in] value Literal value expressed in ub.
 * \return CrossSection converted to its canonical GGEMS representation.
 */
consteval auto operator""_ub(long double value) -> CrossSection {
  return detail::MakeLiteralQuantity<CrossSection>(value, "ub");
}

/*!
 * \brief Creates a cross-section quantity from a \c _mb literal.
 *
 * \param[in] value Literal value expressed in mb.
 * \return CrossSection converted to its canonical GGEMS representation.
 */
consteval auto operator""_mb(unsigned long long value) -> CrossSection {
  return detail::MakeLiteralQuantity<CrossSection>(value, "mb");
}

/*!
 * \brief Creates a cross-section quantity from a \c _mb literal.
 *
 * \param[in] value Literal value expressed in mb.
 * \return CrossSection converted to its canonical GGEMS representation.
 */
consteval auto operator""_mb(long double value) -> CrossSection {
  return detail::MakeLiteralQuantity<CrossSection>(value, "mb");
}

/*!
 * \brief Creates a cross-section quantity from a \c _barn literal.
 *
 * \param[in] value Literal value expressed in barn.
 * \return CrossSection converted to its canonical GGEMS representation.
 */
consteval auto operator""_barn(unsigned long long value) -> CrossSection {
  return detail::MakeLiteralQuantity<CrossSection>(value, "barn");
}

/*!
 * \brief Creates a cross-section quantity from a \c _barn literal.
 *
 * \param[in] value Literal value expressed in barn.
 * \return CrossSection converted to its canonical GGEMS representation.
 */
consteval auto operator""_barn(long double value) -> CrossSection {
  return detail::MakeLiteralQuantity<CrossSection>(value, "barn");
}

/*!
 * \brief Creates a cross-section quantity from a \c _kbarn literal.
 *
 * \param[in] value Literal value expressed in kbarn.
 * \return CrossSection converted to its canonical GGEMS representation.
 */
consteval auto operator""_kbarn(unsigned long long value) -> CrossSection {
  return detail::MakeLiteralQuantity<CrossSection>(value, "kbarn");
}

/*!
 * \brief Creates a cross-section quantity from a \c _kbarn literal.
 *
 * \param[in] value Literal value expressed in kbarn.
 * \return CrossSection converted to its canonical GGEMS representation.
 */
consteval auto operator""_kbarn(long double value) -> CrossSection {
  return detail::MakeLiteralQuantity<CrossSection>(value, "kbarn");
}

/*!
 * \brief Creates a cross-section quantity from a \c _pbarn literal.
 *
 * \param[in] value Literal value expressed in pb.
 * \return CrossSection converted to its canonical GGEMS representation.
 */
consteval auto operator""_pbarn(unsigned long long value) -> CrossSection {
  return detail::MakeLiteralQuantity<CrossSection>(value, "pb");
}

/*!
 * \brief Creates a cross-section quantity from a \c _pbarn literal.
 *
 * \param[in] value Literal value expressed in pb.
 * \return CrossSection converted to its canonical GGEMS representation.
 */
consteval auto operator""_pbarn(long double value) -> CrossSection {
  return detail::MakeLiteralQuantity<CrossSection>(value, "pb");
}

/*!
 * \brief Creates a cross-section quantity from a \c _nbarn literal.
 *
 * \param[in] value Literal value expressed in nb.
 * \return CrossSection converted to its canonical GGEMS representation.
 */
consteval auto operator""_nbarn(unsigned long long value) -> CrossSection {
  return detail::MakeLiteralQuantity<CrossSection>(value, "nb");
}

/*!
 * \brief Creates a cross-section quantity from a \c _nbarn literal.
 *
 * \param[in] value Literal value expressed in nb.
 * \return CrossSection converted to its canonical GGEMS representation.
 */
consteval auto operator""_nbarn(long double value) -> CrossSection {
  return detail::MakeLiteralQuantity<CrossSection>(value, "nb");
}

/*!
 * \brief Creates a cross-section quantity from a \c _ubarn literal.
 *
 * \param[in] value Literal value expressed in ub.
 * \return CrossSection converted to its canonical GGEMS representation.
 */
consteval auto operator""_ubarn(unsigned long long value) -> CrossSection {
  return detail::MakeLiteralQuantity<CrossSection>(value, "ub");
}

/*!
 * \brief Creates a cross-section quantity from a \c _ubarn literal.
 *
 * \param[in] value Literal value expressed in ub.
 * \return CrossSection converted to its canonical GGEMS representation.
 */
consteval auto operator""_ubarn(long double value) -> CrossSection {
  return detail::MakeLiteralQuantity<CrossSection>(value, "ub");
}

/*!
 * \brief Creates a cross-section quantity from a \c _mbarn literal.
 *
 * \param[in] value Literal value expressed in mb.
 * \return CrossSection converted to its canonical GGEMS representation.
 */
consteval auto operator""_mbarn(unsigned long long value) -> CrossSection {
  return detail::MakeLiteralQuantity<CrossSection>(value, "mb");
}

/*!
 * \brief Creates a cross-section quantity from a \c _mbarn literal.
 *
 * \param[in] value Literal value expressed in mb.
 * \return CrossSection converted to its canonical GGEMS representation.
 */
consteval auto operator""_mbarn(long double value) -> CrossSection {
  return detail::MakeLiteralQuantity<CrossSection>(value, "mb");
}
} // namespace ggems::units
