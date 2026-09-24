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
 * \brief Defines chemical element identity and borrowed catalog metadata.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <cstdint>
#include <string_view>
/// \endcond

#include "GGEMS/GGEMSException.hh"

/*!
 * \namespace ggems::core::materials
 * \brief Prepares host isotope compositions, material properties, and EM
 * packages.
 */
namespace ggems::core::materials {

/*!
 * \brief Describes a chemical element identified by its proton number.
 *
 * An element identifies Z, not an isotope mixture. Its representative molar
 * mass is catalog metadata; isotope-resolved material preparation uses
 * GetIsotopeMassAuthority() instead. Symbol and name strings are borrowed.
 */
class GGEMSElement {
public:
  /*!
   * \brief Stores element metadata after checking the atomic number.
   *
   * \pre The symbol and name storage must outlive all uses of this element.
   *
   * \param[in] atomic_number Proton number Z in [1, 99].
   * \param[in] symbol Borrowed chemical symbol.
   * \param[in] name Borrowed element name.
   * \param[in] molar_mass Representative molar mass in g/mol; stored without
   * validation.
   * \throws GGEMSRecoverable If atomic_number is outside [1, 99].
   */
  constexpr GGEMSElement(std::uint32_t atomic_number, std::string_view symbol,
                         std::string_view name, long double molar_mass)
      : atomic_number_{atomic_number}, symbol_{symbol}, name_{name},
        molar_mass_{molar_mass} {
    if (atomic_number < 1U || atomic_number > 99U) {
      throw GGEMSRecoverable{"Atomic number must be in [1, 99]."};
    }
  }

  /*!
   * \brief Returns the chemical identity Z.
   *
   * \return The proton number in [1, 99].
   */
  [[nodiscard]] constexpr auto GetAtomicNumber() const noexcept
    -> std::uint32_t {
    return atomic_number_;
  }

  /*!
   * \brief Returns the borrowed chemical symbol.
   *
   * \return A view with the lifetime of the constructor-supplied string
   * storage.
   */
  [[nodiscard]] constexpr auto GetSymbol() const noexcept -> std::string_view {
    return symbol_;
  }

  /*!
   * \brief Returns the borrowed element name.
   *
   * \return A view with the lifetime of the constructor-supplied string
   * storage.
   */
  [[nodiscard]] constexpr auto GetName() const noexcept -> std::string_view {
    return name_;
  }

  /*!
   * \brief Returns the representative catalog molar mass.
   *
   * \return The stored value in g/mol, independent of an authored isotope
   * mixture.
   */
  [[nodiscard]] constexpr auto GetMolarMass() const noexcept -> long double {
    return molar_mass_;
  }

private:
  std::uint32_t atomic_number_; /*!< Chemical identity Z. */
  std::string_view symbol_;     /*!< Borrowed chemical symbol. */
  std::string_view name_;       /*!< Borrowed element name. */
  long double molar_mass_;      /*!< Representative molar mass in g/mol. */
};

} // namespace ggems::core::materials
