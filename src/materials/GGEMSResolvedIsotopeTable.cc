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
 * \brief Implements sorting and exact lookup of owned isotope molar-mass
 * records.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <algorithm>
#include <format>
#include <utility>
#include <vector>
/// \endcond

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/materials/GGEMSIsotope.hh"
#include "GGEMS/materials/GGEMSResolvedIsotopeTable.hh"

namespace ggems::core::materials {

// =============================================================================
// =============================================================================

GGEMSResolvedIsotopeTable::GGEMSResolvedIsotopeTable(
  std::vector<GGEMSResolvedIsotope> resolved_isotopes)
    : resolved_isotopes_{std::move(resolved_isotopes)} {
  std::ranges::sort(resolved_isotopes_, {}, &GGEMSResolvedIsotope::isotope);
}

// -----------------------------------------------------------------------------

[[nodiscard]] auto
GGEMSResolvedIsotopeTable::Find(GGEMSIsotope const &isotope) const noexcept
  -> GGEMSResolvedIsotope const * {
  auto const found = std::ranges::lower_bound(resolved_isotopes_, isotope, {},
                                              &GGEMSResolvedIsotope::isotope);

  if (found == resolved_isotopes_.end() || found->isotope != isotope) {
    return nullptr;
  }

  return &*found;
}

// -----------------------------------------------------------------------------

[[nodiscard]] auto
GGEMSResolvedIsotopeTable::Require(GGEMSIsotope const &isotope) const
  -> GGEMSResolvedIsotope const & {
  auto const *resolved_isotope = Find(isotope);

  if (resolved_isotope == nullptr) {
    throw GGEMSRecoverable{
      std::format("No resolved molar mass for isotope (Z={}, A={}, M={}).",
                  isotope.GetAtomicNumber(), isotope.GetMassNumber(),
                  isotope.GetIsomerState())};
  }

  return *resolved_isotope;
}

} // namespace ggems::core::materials
