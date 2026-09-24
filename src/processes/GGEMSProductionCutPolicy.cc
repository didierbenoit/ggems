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
 * \brief Stores global cut lengths and resolves per-channel
 * Volume/Material/Global precedence.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <algorithm>
#include <optional>
/// \endcond

#include "GGEMS/processes/GGEMSProductionCutPolicy.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace ggems::core::processes {

namespace {

/*!
 * \brief Owns the process-wide cut configuration, initially 1 mm in every
 * channel.
 */
GGEMSProductionCutPolicy g_production_cut_policy{};

// =============================================================================
// =============================================================================

/*!
 * \brief Reads one optional channel value from a length record.
 *
 * \param[in] lengths Optional per-channel lengths.
 * \param[in] channel Requested channel.
 * \return The channel value, or std::nullopt for an absent or unrecognized
 * channel.
 */
[[nodiscard]] auto FindLength(GGEMSProductionCutLengths const &lengths,
                              GGEMSProductionCutChannel channel) noexcept
  -> std::optional<units::Length> {
  switch (channel) {
  case GGEMSProductionCutChannel::Gamma:
    return lengths.gamma;
  case GGEMSProductionCutChannel::Electron:
    return lengths.electron;
  case GGEMSProductionCutChannel::Positron:
    return lengths.positron;
  case GGEMSProductionCutChannel::Proton:
    return lengths.proton;
  }
  return std::nullopt;
}

} // namespace

// =============================================================================
// =============================================================================

auto SetProductionCuts(GGEMSProductionCutLengths const &cuts) -> void {
  if (cuts.gamma.has_value()) {
    g_production_cut_policy.global.gamma = cuts.gamma;
  }
  if (cuts.electron.has_value()) {
    g_production_cut_policy.global.electron = cuts.electron;
  }
  if (cuts.positron.has_value()) {
    g_production_cut_policy.global.positron = cuts.positron;
  }
  if (cuts.proton.has_value()) {
    g_production_cut_policy.global.proton = cuts.proton;
  }
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto GetProductionCutPolicy() noexcept
  -> GGEMSProductionCutPolicy const & {
  return g_production_cut_policy;
}

// =============================================================================
// =============================================================================

auto ResolveProductionCuts(GGEMSProductionCutPolicy const &policy,
                           GGEMSProductionCutContext const &context)
  -> GGEMSResolvedProductionCuts {
  auto const material_override =
    std::ranges::find(policy.materials, context.material_index,
                      &GGEMSMaterialProductionCuts::material_index);

  auto const *material = material_override != policy.materials.end()
                           ? &material_override->lengths
                           : nullptr;

  GGEMSResolvedProductionCuts resolved{};

  for (auto const channel : k_production_cut_channels) {
    auto const index = ProductionCutChannelIndex(channel);

    auto length = FindLength(context.volume, channel);
    auto scope = GGEMSProductionCutScope::Volume;

    if (!length.has_value() && material != nullptr) {
      length = FindLength(*material, channel);
      scope = GGEMSProductionCutScope::Material;
    }

    if (!length.has_value()) {
      length = FindLength(policy.global, channel);
      scope = GGEMSProductionCutScope::Global;
    }

    resolved.lengths[index] = *length;
    resolved.scopes[index] = scope;
  }

  return resolved;
}

} // namespace ggems::core::processes
