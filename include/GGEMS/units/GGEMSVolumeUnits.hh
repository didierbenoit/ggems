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
 * \brief Declares strongly typed volume units and literals.
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
 * \brief Marker type identifying the volume unit registry.
 */
struct VolumeUnitSet {};

/*!
 * \brief Defines the supported volume units and their canonical scale factors.
 */
template <> struct UnitRegistry<VolumeUnitSet> {
  /*!
   * \brief Registered unit definitions for this quantity family.
   */
  static constexpr std::array<UnitDefinition, 7U> units{{
      {.symbol = "pm3",
       .scale = DecimalScale(0),
       .unicode_symbol = "pm³"},
      {.symbol = "nm3",
       .scale = DecimalScale(9),
       .unicode_symbol = "nm³"},
      {.symbol = "um3",
       .scale = DecimalScale(18),
       .unicode_symbol = "µm³"},
      {.symbol = "mm3",
       .scale = DecimalScale(27),
       .unicode_symbol = "mm³"},
      {.symbol = "cm3",
       .scale = DecimalScale(30),
       .unicode_symbol = "cm³",
       .automatic_display = false},
      {.symbol = "m3",
       .scale = DecimalScale(36),
       .unicode_symbol = "m³"},
      {.symbol = "km3",
       .scale = DecimalScale(45),
       .unicode_symbol = "km³"},
  }};
};

/*!
 * \brief Tag type identifying volume quantities.
 */
struct VolumeTag {};

/*!
 * \brief Defines conversion and formatting traits for Volume quantities.
 */
template <> struct QuantityTraits<VolumeTag> {
  /*!
   * \brief Unit registry associated with this quantity type.
   */
  using unit_set = VolumeUnitSet;
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
 * \brief Strongly typed volume quantity stored canonically in cubic picometers.
 */
using Volume = Quantity<VolumeTag, long double>;

static_assert(ValidateUnitSet<VolumeUnitSet>());
static_assert(ValidateQuantityTraits<VolumeTag, long double>());

/*!
 * \brief Creates a volume quantity from a \c _pm3 literal.
 *
 * \param[in] value Literal value expressed in pm3.
 * \return Volume converted to its canonical GGEMS representation.
 */
consteval auto operator""_pm3(unsigned long long value) -> Volume {
  return detail::MakeLiteralQuantity<Volume>(value, "pm3");
}

/*!
 * \brief Creates a volume quantity from a \c _pm3 literal.
 *
 * \param[in] value Literal value expressed in pm3.
 * \return Volume converted to its canonical GGEMS representation.
 */
consteval auto operator""_pm3(long double value) -> Volume {
  return detail::MakeLiteralQuantity<Volume>(value, "pm3");
}

/*!
 * \brief Creates a volume quantity from a \c _nm3 literal.
 *
 * \param[in] value Literal value expressed in nm3.
 * \return Volume converted to its canonical GGEMS representation.
 */
consteval auto operator""_nm3(unsigned long long value) -> Volume {
  return detail::MakeLiteralQuantity<Volume>(value, "nm3");
}

/*!
 * \brief Creates a volume quantity from a \c _nm3 literal.
 *
 * \param[in] value Literal value expressed in nm3.
 * \return Volume converted to its canonical GGEMS representation.
 */
consteval auto operator""_nm3(long double value) -> Volume {
  return detail::MakeLiteralQuantity<Volume>(value, "nm3");
}

/*!
 * \brief Creates a volume quantity from a \c _um3 literal.
 *
 * \param[in] value Literal value expressed in um3.
 * \return Volume converted to its canonical GGEMS representation.
 */
consteval auto operator""_um3(unsigned long long value) -> Volume {
  return detail::MakeLiteralQuantity<Volume>(value, "um3");
}

/*!
 * \brief Creates a volume quantity from a \c _um3 literal.
 *
 * \param[in] value Literal value expressed in um3.
 * \return Volume converted to its canonical GGEMS representation.
 */
consteval auto operator""_um3(long double value) -> Volume {
  return detail::MakeLiteralQuantity<Volume>(value, "um3");
}

/*!
 * \brief Creates a volume quantity from a \c _mm3 literal.
 *
 * \param[in] value Literal value expressed in mm3.
 * \return Volume converted to its canonical GGEMS representation.
 */
consteval auto operator""_mm3(unsigned long long value) -> Volume {
  return detail::MakeLiteralQuantity<Volume>(value, "mm3");
}

/*!
 * \brief Creates a volume quantity from a \c _mm3 literal.
 *
 * \param[in] value Literal value expressed in mm3.
 * \return Volume converted to its canonical GGEMS representation.
 */
consteval auto operator""_mm3(long double value) -> Volume {
  return detail::MakeLiteralQuantity<Volume>(value, "mm3");
}

/*!
 * \brief Creates a volume quantity from a \c _cm3 literal.
 *
 * \param[in] value Literal value expressed in cm3.
 * \return Volume converted to its canonical GGEMS representation.
 */
consteval auto operator""_cm3(unsigned long long value) -> Volume {
  return detail::MakeLiteralQuantity<Volume>(value, "cm3");
}

/*!
 * \brief Creates a volume quantity from a \c _cm3 literal.
 *
 * \param[in] value Literal value expressed in cm3.
 * \return Volume converted to its canonical GGEMS representation.
 */
consteval auto operator""_cm3(long double value) -> Volume {
  return detail::MakeLiteralQuantity<Volume>(value, "cm3");
}

/*!
 * \brief Creates a volume quantity from a \c _m3 literal.
 *
 * \param[in] value Literal value expressed in m3.
 * \return Volume converted to its canonical GGEMS representation.
 */
consteval auto operator""_m3(unsigned long long value) -> Volume {
  return detail::MakeLiteralQuantity<Volume>(value, "m3");
}

/*!
 * \brief Creates a volume quantity from a \c _m3 literal.
 *
 * \param[in] value Literal value expressed in m3.
 * \return Volume converted to its canonical GGEMS representation.
 */
consteval auto operator""_m3(long double value) -> Volume {
  return detail::MakeLiteralQuantity<Volume>(value, "m3");
}

/*!
 * \brief Creates a volume quantity from a \c _km3 literal.
 *
 * \param[in] value Literal value expressed in km3.
 * \return Volume converted to its canonical GGEMS representation.
 */
consteval auto operator""_km3(unsigned long long value) -> Volume {
  return detail::MakeLiteralQuantity<Volume>(value, "km3");
}

/*!
 * \brief Creates a volume quantity from a \c _km3 literal.
 *
 * \param[in] value Literal value expressed in km3.
 * \return Volume converted to its canonical GGEMS representation.
 */
consteval auto operator""_km3(long double value) -> Volume {
  return detail::MakeLiteralQuantity<Volume>(value, "km3");
}
} // namespace ggems::units
