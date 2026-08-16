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
 * \brief Declares strongly typed frequency units and literals.
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
 * \brief Marker type identifying the frequency unit registry.
 */
struct FrequencyUnitSet {};

/*!
 * \brief Defines the supported frequency units and their canonical scale factors.
 */
template <> struct UnitRegistry<FrequencyUnitSet> {
  /*!
   * \brief Registered unit definitions for this quantity family.
   */
  static constexpr std::array<UnitDefinition, 5U> units{{
      {.symbol = "Hz",
       .scale = DecimalScale(0)},
      {.symbol = "kHz",
       .scale = DecimalScale(3)},
      {.symbol = "MHz",
       .scale = DecimalScale(6)},
      {.symbol = "GHz",
       .scale = DecimalScale(9)},
      {.symbol = "THz",
       .scale = DecimalScale(12)},
  }};
};

/*!
 * \brief Tag type identifying frequency quantities.
 */
struct FrequencyTag {};

/*!
 * \brief Defines conversion and formatting traits for Frequency quantities.
 */
template <> struct QuantityTraits<FrequencyTag> {
  /*!
   * \brief Unit registry associated with this quantity type.
   */
  using unit_set = FrequencyUnitSet;
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
 * \brief Strongly typed frequency quantity stored canonically in hertz.
 */
using Frequency = Quantity<FrequencyTag, std::uint64_t>;

static_assert(ValidateUnitSet<FrequencyUnitSet>());
static_assert(ValidateQuantityTraits<FrequencyTag, std::uint64_t>());

/*!
 * \brief Creates a frequency quantity from a \c _Hz literal.
 *
 * \param[in] value Literal value expressed in Hz.
 * \return Frequency converted to its canonical GGEMS representation.
 */
consteval auto operator""_Hz(unsigned long long value) -> Frequency {
  return detail::MakeLiteralQuantity<Frequency>(value, "Hz");
}

/*!
 * \brief Creates a frequency quantity from a \c _Hz literal.
 *
 * \param[in] value Literal value expressed in Hz.
 * \return Frequency converted to its canonical GGEMS representation.
 */
consteval auto operator""_Hz(long double value) -> Frequency {
  return detail::MakeLiteralQuantity<Frequency>(value, "Hz");
}

/*!
 * \brief Creates a frequency quantity from a \c _kHz literal.
 *
 * \param[in] value Literal value expressed in kHz.
 * \return Frequency converted to its canonical GGEMS representation.
 */
consteval auto operator""_kHz(unsigned long long value) -> Frequency {
  return detail::MakeLiteralQuantity<Frequency>(value, "kHz");
}

/*!
 * \brief Creates a frequency quantity from a \c _kHz literal.
 *
 * \param[in] value Literal value expressed in kHz.
 * \return Frequency converted to its canonical GGEMS representation.
 */
consteval auto operator""_kHz(long double value) -> Frequency {
  return detail::MakeLiteralQuantity<Frequency>(value, "kHz");
}

/*!
 * \brief Creates a frequency quantity from a \c _MHz literal.
 *
 * \param[in] value Literal value expressed in MHz.
 * \return Frequency converted to its canonical GGEMS representation.
 */
consteval auto operator""_MHz(unsigned long long value) -> Frequency {
  return detail::MakeLiteralQuantity<Frequency>(value, "MHz");
}

/*!
 * \brief Creates a frequency quantity from a \c _MHz literal.
 *
 * \param[in] value Literal value expressed in MHz.
 * \return Frequency converted to its canonical GGEMS representation.
 */
consteval auto operator""_MHz(long double value) -> Frequency {
  return detail::MakeLiteralQuantity<Frequency>(value, "MHz");
}
/*!
 * \brief Creates a frequency quantity from a \c _GHz literal.
 *
 * \param[in] value Literal value expressed in GHz.
 * \return Frequency converted to its canonical GGEMS representation.
 */
consteval auto operator""_GHz(unsigned long long value) -> Frequency {
  return detail::MakeLiteralQuantity<Frequency>(value, "GHz");
}

/*!
 * \brief Creates a frequency quantity from a \c _GHz literal.
 *
 * \param[in] value Literal value expressed in GHz.
 * \return Frequency converted to its canonical GGEMS representation.
 */
consteval auto operator""_GHz(long double value) -> Frequency {
  return detail::MakeLiteralQuantity<Frequency>(value, "GHz");
}

/*!
 * \brief Creates a frequency quantity from a \c _THz literal.
 *
 * \param[in] value Literal value expressed in THz.
 * \return Frequency converted to its canonical GGEMS representation.
 */
consteval auto operator""_THz(unsigned long long value) -> Frequency {
  return detail::MakeLiteralQuantity<Frequency>(value, "THz");
}

/*!
 * \brief Creates a frequency quantity from a \c _THz literal.
 *
 * \param[in] value Literal value expressed in THz.
 * \return Frequency converted to its canonical GGEMS representation.
 */
consteval auto operator""_THz(long double value) -> Frequency {
  return detail::MakeLiteralQuantity<Frequency>(value, "THz");
}
} // namespace ggems::units
