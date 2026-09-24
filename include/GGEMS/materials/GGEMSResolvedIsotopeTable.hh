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
 * \brief Declares owned isotope molar-mass records and exact-key lookup.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <vector>
/// \endcond

#include "GGEMS/materials/GGEMSIsotope.hh"

namespace ggems::core::materials {

/*!
 * \brief Associates an isotope key with its neutral-atom molar mass.
 */
struct GGEMSResolvedIsotope {
  GGEMSIsotope isotope; /*!< Exact (Z, A, M) key. */
  long double
    molar_mass_grams_per_mole; /*!< Neutral-atom molar mass in g/mol. */
};

/*!
 * \brief Owns isotope mass records sorted by exact isotope identity.
 *
 * Construction sorts supplied rows without checking duplicates or mass values.
 * Borrowed rows remain valid until the table is destroyed, assigned to, or
 * moved from. This table does not resolve missing isotopes by substitution.
 */
class GGEMSResolvedIsotopeTable {
public:
  /*!
   * \brief Takes ownership of isotope mass rows and sorts them by (Z, A, M).
   *
   * \param[in] resolved_isotopes Rows to store; provide unique keys and finite
   * positive molar masses.
   */
  explicit GGEMSResolvedIsotopeTable(
    std::vector<GGEMSResolvedIsotope> resolved_isotopes);

  /*!
   * \brief Finds the first sorted row with the exact isotope key.
   *
   * \param[in] isotope Exact isotope identity to resolve.
   * \return A borrowed row pointer, or nullptr if the key is absent.
   */
  [[nodiscard]] auto Find(GGEMSIsotope const &isotope) const noexcept
    -> GGEMSResolvedIsotope const *;

  /*!
   * \brief Requires a resolved mass for the exact isotope key.
   *
   * \param[in] isotope Exact isotope identity to resolve.
   * \return A borrowed row reference.
   * \throws GGEMSRecoverable If no row has the requested key.
   */
  [[nodiscard]] auto Require(GGEMSIsotope const &isotope) const
    -> GGEMSResolvedIsotope const &;

private:
  std::vector<GGEMSResolvedIsotope>
    resolved_isotopes_; /*!< Owned rows ordered by isotope key. */
};

} // namespace ggems::core::materials
