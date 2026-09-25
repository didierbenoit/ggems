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
 * \brief Declares configured-cut inspection and independent material
 * reference-cut reports.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
/// \endcond

#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/processes/GGEMSMaterialCutCouplePackage.hh"
#include "GGEMS/processes/GGEMSProductionCutPolicy.hh"
#include "GGEMS/units/GGEMSEnergyUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace ggems::core::processes {

/*!
 * \brief Copies one channel's resolved length, scope, and production threshold.
 */
struct GGEMSProductionCutChannelInspection {
  /*! \brief Secondary-production channel. */
  GGEMSProductionCutChannel channel;

  /*! \brief Resolved length in canonical pm. */
  units::Length effective_length;

  /*! \brief Scope that supplied this length. */
  GGEMSProductionCutScope winning_scope;

  /*! \brief Resolved base threshold in micro-eV. */
  units::Energy production_threshold;
};

/*!
 * \brief Copies context provenance and its resolved package-local identities.
 */
struct GGEMSProductionCutContextInspection {
  /*! \brief Position in the original context sequence. */
  std::size_t context_index;

  /*! \brief Position in the EM package's input sequence. */
  std::uint32_t authoring_material_index;

  /*! \brief Deduplicated EM descriptor ID. */
  std::uint32_t snapshot_material_id;

  /*! \brief Deduplicated material/threshold tuple ID. */
  std::uint32_t snapshot_couple_id;

  /*! \brief Gamma/e-/e+/Proton data. */
  std::array<GGEMSProductionCutChannelInspection, 4U> channels;
};

/*!
 * \brief Copies one resolved context without retaining package views.
 *
 * \pre context_index must be in range; this accessor does not check it.
 *
 * \param[in] package Package containing the resolved contexts.
 * \param[in] context_index Valid index in the package's context mapping and
 * provenance arrays.
 * \return An independent value record of the context and all four channels.
 */
[[nodiscard]] auto
InspectProductionCutContext(GGEMSMaterialCutCouplePackage const &package,
                            std::size_t context_index)
  -> GGEMSProductionCutContextInspection;

/*!
 * \brief Formats a resolved context with IDs, effective lengths, scopes, and
 * thresholds.
 *
 * \pre context_index must be in range.
 *
 * \param[in] package Package containing the resolved contexts.
 * \param[in] context_index Valid index in the package's context mapping and
 * provenance arrays.
 * \return Owned human-readable text using the current unit presentation policy.
 */
[[nodiscard]] auto
DescribeProductionCutContext(GGEMSMaterialCutCouplePackage const &package,
                             std::size_t context_index) -> std::string;

/*!
 * \brief Logs a resolved context at INFO level under Cuts.
 *
 * \pre context_index must be in range.
 *
 * \param[in] package Package containing the resolved contexts.
 * \param[in] context_index Valid index in the package's context mapping and
 * provenance arrays.
 */
auto VerboseProductionCutContext(GGEMSMaterialCutCouplePackage const &package,
                                 std::size_t context_index) -> void;

/*!
 * \brief Formats the four currently configured global cut lengths.
 *
 * This reads the live global policy; it does not resolve a material context or
 * report material-dependent energy thresholds.
 *
 * \return Owned text in Gamma, Electron, Positron, Proton order.
 */
[[nodiscard]] auto DescribeProductionCuts() -> std::string;

/*!
 * \brief Logs the currently configured global cut lengths at INFO level under
 * Cuts.
 */
auto VerboseProductionCuts() -> void;

/*!
 * \brief Formats reference thresholds at one supplied length independently of
 * policy.
 *
 * The Python material reports call this with exactly 1 mm, regardless of global
 * cut configuration. Empty matter is reported as not applicable. Each channel
 * that raises GGEMSRecoverable is reported as outside converter domain, while
 * other channels are still reported. The material is not registered or changed.
 *
 * \param[in] material Material for which to build a temporary one-material EM
 * package.
 * \param[in] reference_length Reference length used for every channel, in
 * canonical pm.
 * \return Owned reference text, not a report of configured simulation
 * thresholds.
 */
[[nodiscard]] auto
DescribeProductionCutsForMaterial(materials::GGEMSMaterial const &material,
                                  units::Length reference_length)
  -> std::string;

} // namespace ggems::core::processes
