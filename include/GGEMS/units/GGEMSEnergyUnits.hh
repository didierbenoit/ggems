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
 * \brief Declares strongly typed energy and signed energy-change units and literals.
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
 * \brief Marker type identifying the energy unit registry.
 */
struct EnergyUnitSet {};

/*!
 * \brief Defines the supported energy units and their canonical scale factors.
 */
template <> struct UnitRegistry<EnergyUnitSet> {
  /*!
   * \brief Registered unit definitions for this quantity family.
   */
  static constexpr std::array<UnitDefinition, 6U> units{{
      {.symbol = "meV",
       .scale = DecimalScale(0)},
      {.symbol = "eV",
       .scale = DecimalScale(3)},
      {.symbol = "keV",
       .scale = DecimalScale(6)},
      {.symbol = "MeV",
       .scale = DecimalScale(9)},
      {.symbol = "GeV",
       .scale = DecimalScale(12)},
      {.symbol = "TeV",
       .scale = DecimalScale(15)},
  }};
};

/*!
 * \brief Tag type identifying nonnegative energy quantities.
 */
struct EnergyTag {};

/*!
 * \brief Defines conversion and formatting traits for Energy quantities.
 */
template <> struct QuantityTraits<EnergyTag> {
  /*!
   * \brief Unit registry associated with this quantity type.
   */
  using unit_set = EnergyUnitSet;
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
 * \brief Tag type identifying signed energy-change quantities.
 */
struct EnergyChangeTag {};

/*!
 * \brief Defines conversion and formatting traits for EnergyChange quantities.
 */
template <> struct QuantityTraits<EnergyChangeTag> {
  /*!
   * \brief Unit registry associated with this quantity type.
   */
  using unit_set = EnergyUnitSet;
  /*!
   * \brief Allowed sign domain for this quantity type.
   */
  static constexpr QuantityDomain domain{QuantityDomain::Signed};
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
 * \brief Strongly typed nonnegative energy quantity stored canonically in milli-electron-volts.
 */
using Energy = Quantity<EnergyTag, std::uint64_t>;
/*!
 * \brief Strongly typed signed energy-change quantity stored canonically in milli-electron-volts.
 */
using EnergyChange = Quantity<EnergyChangeTag, std::int64_t>;

static_assert(ValidateUnitSet<EnergyUnitSet>());
static_assert(ValidateQuantityTraits<EnergyTag, std::uint64_t>());
static_assert(ValidateQuantityTraits<EnergyChangeTag, std::int64_t>());

/*!
 * \brief Creates a energy quantity from a \c _meV literal.
 *
 * \param[in] value Literal value expressed in meV.
 * \return Energy converted to its canonical GGEMS representation.
 */
consteval auto operator""_meV(unsigned long long value) -> Energy {
  return detail::MakeLiteralQuantity<Energy>(value, "meV");
}

/*!
 * \brief Creates a energy quantity from a \c _meV literal.
 *
 * \param[in] value Literal value expressed in meV.
 * \return Energy converted to its canonical GGEMS representation.
 */
consteval auto operator""_meV(long double value) -> Energy {
  return detail::MakeLiteralQuantity<Energy>(value, "meV");
}

/*!
 * \brief Creates a energy quantity from a \c _eV literal.
 *
 * \param[in] value Literal value expressed in eV.
 * \return Energy converted to its canonical GGEMS representation.
 */
consteval auto operator""_eV(unsigned long long value) -> Energy {
  return detail::MakeLiteralQuantity<Energy>(value, "eV");
}

/*!
 * \brief Creates a energy quantity from a \c _eV literal.
 *
 * \param[in] value Literal value expressed in eV.
 * \return Energy converted to its canonical GGEMS representation.
 */
consteval auto operator""_eV(long double value) -> Energy {
  return detail::MakeLiteralQuantity<Energy>(value, "eV");
}

/*!
 * \brief Creates a energy quantity from a \c _keV literal.
 *
 * \param[in] value Literal value expressed in keV.
 * \return Energy converted to its canonical GGEMS representation.
 */
consteval auto operator""_keV(unsigned long long value) -> Energy {
  return detail::MakeLiteralQuantity<Energy>(value, "keV");
}

/*!
 * \brief Creates a energy quantity from a \c _keV literal.
 *
 * \param[in] value Literal value expressed in keV.
 * \return Energy converted to its canonical GGEMS representation.
 */
consteval auto operator""_keV(long double value) -> Energy {
  return detail::MakeLiteralQuantity<Energy>(value, "keV");
}

/*!
 * \brief Creates a energy quantity from a \c _MeV literal.
 *
 * \param[in] value Literal value expressed in MeV.
 * \return Energy converted to its canonical GGEMS representation.
 */
consteval auto operator""_MeV(unsigned long long value) -> Energy {
  return detail::MakeLiteralQuantity<Energy>(value, "MeV");
}

/*!
 * \brief Creates a energy quantity from a \c _MeV literal.
 *
 * \param[in] value Literal value expressed in MeV.
 * \return Energy converted to its canonical GGEMS representation.
 */
consteval auto operator""_MeV(long double value) -> Energy {
  return detail::MakeLiteralQuantity<Energy>(value, "MeV");
}

/*!
 * \brief Creates a energy quantity from a \c _GeV literal.
 *
 * \param[in] value Literal value expressed in GeV.
 * \return Energy converted to its canonical GGEMS representation.
 */
consteval auto operator""_GeV(unsigned long long value) -> Energy {
  return detail::MakeLiteralQuantity<Energy>(value, "GeV");
}

/*!
 * \brief Creates a energy quantity from a \c _GeV literal.
 *
 * \param[in] value Literal value expressed in GeV.
 * \return Energy converted to its canonical GGEMS representation.
 */
consteval auto operator""_GeV(long double value) -> Energy {
  return detail::MakeLiteralQuantity<Energy>(value, "GeV");
}

/*!
 * \brief Creates a energy quantity from a \c _TeV literal.
 *
 * \param[in] value Literal value expressed in TeV.
 * \return Energy converted to its canonical GGEMS representation.
 */
consteval auto operator""_TeV(unsigned long long value) -> Energy {
  return detail::MakeLiteralQuantity<Energy>(value, "TeV");
}

/*!
 * \brief Creates a energy quantity from a \c _TeV literal.
 *
 * \param[in] value Literal value expressed in TeV.
 * \return Energy converted to its canonical GGEMS representation.
 */
consteval auto operator""_TeV(long double value) -> Energy {
  return detail::MakeLiteralQuantity<Energy>(value, "TeV");
}
} // namespace ggems::units
