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
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include "sources/GGEMSEnergyDistribution.clh"
#include "sources/GGEMSSourceRecord.clh"
#include "sources/GGEMSEnergyDistribution.clh"
#include "sources/GGEMSEnergyDistributionRecord.clh"

__kernel void energy_distribution_sampling_probe(
  __global GGEMSRandomState *sample_states,
  __global GGEMSRandomState *reference_states,
  __global GGEMSSourceRecord const *source,
  __global GGEMSEnergyDistributionRecord const *distribution,
  __global ulong const *energy_values_micro_eV,
  __global ulong const *cumulative_ticket_upper, uint expected_draw_count,
  __global ulong *sampled_energy) {
  sampled_energy[0] = GGEMS_EnergyDistributionSample(
    source->energy_micro_eV, distribution, energy_values_micro_eV,
    cumulative_ticket_upper, sample_states, 0U);

  for (uint draw = 0U; draw < expected_draw_count; ++draw) {
    (void)(GGEMS_RndmUInt32(reference_states, 0U));
  }
}

// =============================================================================
// =============================================================================

__kernel void energy_distribution_find_index_probe(
  __global GGEMSEnergyDistributionRecord const *distribution,
  __global ulong const *cumulative_ticket_upper, uint raw_ticket,
  __global uint *selected_index) {
  selected_index[0] = GGEMS_EnergyDistributionFindIndex(
    distribution, cumulative_ticket_upper, raw_ticket);
}

// =============================================================================
// =============================================================================

__kernel void energy_distribution_sample_ticket_probe(
  __global GGEMSEnergyDistributionRecord const *distribution,
  __global ulong const *energy_values_micro_eV,
  __global ulong const *cumulative_ticket_upper, uint raw_ticket,
  __global ulong *sampled_energy) {
  sampled_energy[0] = GGEMS_EnergyDistributionSampleWithTicket(
    distribution, energy_values_micro_eV, cumulative_ticket_upper, raw_ticket);
}

// =============================================================================
// =============================================================================

__kernel void energy_distribution_regular_offset_probe(ulong width,
                                                       ulong ticket_span,
                                                       ulong local_ticket,
                                                       __global ulong *offset) {
  offset[0] =
    GGEMS_EnergyDistributionMapRegularOffset(width, ticket_span, local_ticket);
}

// =============================================================================
// =============================================================================

__kernel void
random_raw_scalar_equivalence_probe(__global GGEMSRandomState *raw_states,
                                    __global GGEMSRandomState *uniform_states,
                                    __global uint *raw_output,
                                    __global float *uniform_output) {
  raw_output[0] = GGEMS_RndmUInt32(raw_states, 0U);
  uniform_output[0] = GGEMS_RndmUniform(uniform_states, 0U);
}
