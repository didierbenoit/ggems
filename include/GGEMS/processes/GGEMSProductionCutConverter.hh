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
 * \brief Declares host conversion from material-dependent cut lengths to energy
 * thresholds.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <cstdint>
/// \endcond

#include "GGEMS/materials/GGEMSEMMaterialPackage.hh"
#include "GGEMS/processes/GGEMSProductionCutPolicy.hh"
#include "GGEMS/units/GGEMSEnergyUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace ggems::core::processes {

/*!
 * \brief Converts a production-cut length into a canonical secondary-energy
 * threshold.
 *
 * Gamma inverts the approximate absorption proxy 5/Sigma_abs from 990 eV to the
 * smallest elemental absorption-minimum energy in the material. Electron and
 * Positron use distinct surrogate stopping/range relations, with a 10 keV
 * low-energy branch and density correction below 30 keV; both provisional and
 * corrected energies must be at least 990 eV, and the upper bound is 10 GeV.
 * Proton uses the material-independent convention 100 keV per mm and permits
 * zero length. Gamma and lepton conversion require nonempty matter; Proton does
 * not. These relations are converter models, not transport stopping ranges. The
 * final energy is quantized once through Units to integer micro-eV using
 * nearest rounding, with halfway values rounded away from zero.
 *
 * \pre material_id must index GetDescriptors(); the package must contain valid
 * material data.
 *
 * \param[in] channel Requested production-cut channel.
 * \param[in] length Nonnegative cut length stored in canonical integer pm.
 * \param[in] materials Host EM package; borrowed only during conversion.
 * \param[in] material_id Descriptor ID in this package, not an authoring
 * material index.
 * \return A secondary-production threshold in canonical integer micro-eV.
 * \throws GGEMSRecoverable If the channel is unknown, required matter is empty,
 * a converter domain is exceeded, or Proton conversion overflows Energy.
 */
[[nodiscard]] auto
ConvertProductionCutLength(GGEMSProductionCutChannel channel,
                           units::Length length,
                           materials::GGEMSEMMaterialPackage const &materials,
                           std::uint32_t material_id) -> units::Energy;

} // namespace ggems::core::processes
