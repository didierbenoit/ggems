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
 * \brief Interns exact material identities and copies elemental EM properties
 * into flat host arrays.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <algorithm>
#include <cstdint>
#include <span>
#include <vector>
/// \endcond

#include "GGEMS/materials/GGEMSEMMaterialPackage.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"

namespace ggems::core::materials {

// =============================================================================
// =============================================================================

GGEMSEMMaterialPackage::GGEMSEMMaterialPackage(
  std::span<GGEMSMaterial const> materials) {
  material_ids_.reserve(materials.size());
  descriptors_.reserve(materials.size());

  std::vector<GGEMSMaterial const *> compiled;

  for (auto const &material : materials) {
    auto const interned = std::ranges::find_if(
      compiled, [&material](GGEMSMaterial const *candidate) -> bool {
        return HasSameScientificIdentity(*candidate, material);
      });

    if (interned != compiled.end()) {
      material_ids_.push_back(
        static_cast<std::uint32_t>(interned - compiled.begin()));
      continue;
    }

    material_ids_.push_back(static_cast<std::uint32_t>(compiled.size()));
    compiled.push_back(&material);

    auto const constituents = material.GetElementalConstituents();

    descriptors_.push_back({
      .density = material.GetDensity(),
      .first_constituent =
        static_cast<std::uint32_t>(elemental_constituents_.size()),
      .constituent_count = static_cast<std::uint32_t>(constituents.size()),
      .total_atom_density_per_cubic_centimeter =
        material.GetTotalAtomDensityPerCubicCentimeter(),
      .electron_density_per_cubic_centimeter =
        material.GetElectronDensityPerCubicCentimeter(),
    });

    for (auto const &constituent : constituents) {
      elemental_constituents_.push_back({
        .atomic_number = constituent.atomic_number,
        .mass_fraction = constituent.mass_fraction,
        .number_density_per_cubic_centimeter =
          constituent.number_density_per_cubic_centimeter,
        .electron_density_per_cubic_centimeter =
          constituent.electron_density_per_cubic_centimeter,
      });
    }
  }
}

} // namespace ggems::core::materials
