#include "core/sources/GGEMSEnergyDistribution.clh"

__kernel void energy_distribution_sampling_probe(
    __global GGEMSRandomState *sample_states,
    __global GGEMSRandomState *reference_states,
    __global GGEMSSourceRecord const *source,
    __global GGEMSEnergyDistributionRecord const *distribution,
    __global ulong const *energy_values_milli_eV,
    __global ulong const *cumulative_ticket_upper, uint expected_draw_count,
    __global ulong *sampled_energy) {
  sampled_energy[0] = GGEMS_EnergyDistributionSample(
      source->energy_milli_eV, distribution, energy_values_milli_eV,
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
    __global ulong const *energy_values_milli_eV,
    __global ulong const *cumulative_ticket_upper, uint raw_ticket,
    __global ulong *sampled_energy) {
  sampled_energy[0] = GGEMS_EnergyDistributionSampleWithTicket(
      distribution, energy_values_milli_eV, cumulative_ticket_upper,
      raw_ticket);
}

// =============================================================================
// =============================================================================

__kernel void energy_distribution_regular_offset_probe(ulong width,
                                                       ulong ticket_span,
                                                       ulong local_ticket,
                                                       __global ulong *offset) {
  offset[0] = GGEMS_EnergyDistributionMapRegularOffset(width, ticket_span,
                                                       local_ticket);
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
