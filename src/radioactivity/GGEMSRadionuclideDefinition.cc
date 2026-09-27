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
 * \brief Implements radioactive parent-definition admission and yield totals.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <string>
#include <utility>
#include <vector>

#include "GGEMS/GGEMSException.hh"

#include "GGEMS/radioactivity/GGEMSRadionuclideDefinition.hh"
#include "GGEMS/radioactivity/GGEMSRadionuclideEmission.hh"

namespace ggems::core::radioactivity {

// =============================================================================
// =============================================================================

GGEMSRadionuclideDefinition::GGEMSRadionuclideDefinition(
  std::string canonical_name, long double half_life_seconds,
  std::vector<GGEMSRadionuclideEmission> emissions)
    : canonical_name_{std::move(canonical_name)},
      half_life_seconds_{half_life_seconds}, emissions_{std::move(emissions)} {
  if (canonical_name_.empty()) {
    throw ggems::core::GGEMSRecoverable(
      "Radionuclide canonical name must not be empty.");
  }

  if (!(half_life_seconds_ > 0.0L)) {
    throw ggems::core::GGEMSRecoverable(
      "Radionuclide half-life must be strictly positive.");
  }

  if (emissions_.empty()) {
    throw ggems::core::GGEMSRecoverable(
      "Radionuclide definition requires at least one emission channel.");
  }
}

// -----------------------------------------------------------------------------

[[nodiscard]] auto
GGEMSRadionuclideDefinition::GetTotalYieldPerDecay() const noexcept
  -> long double {
  long double total{0.0L};

  for (auto const &emission : emissions_) {
    total += emission.GetYieldPerDecay();
  }

  return total;
}

} // namespace ggems::core::radioactivity
