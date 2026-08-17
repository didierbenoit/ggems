#include "random/GGEMSRandom.clh"
#include "sources/GGEMSSource.clh"
#include "sources/GGEMSEnergyDistribution.clh"

// =============================================================================
// =============================================================================

__kernel void
source_sampling_imposed_probe(__global GGEMSSourceRecord const *source,
                              __global float const *uniform_values,
                              __global long *sampled_position,
                              __global float *sampled_direction) {
  if (get_global_id(0) != 0U) {
    return;
  }

  float4 const position_uniforms =
      (float4)(uniform_values[0], uniform_values[1], uniform_values[2],
               uniform_values[3]);
  float4 const direction_uniforms =
      (float4)(uniform_values[4], uniform_values[5], uniform_values[6],
               uniform_values[7]);

  long3 const position =
      GGEMS_SourceSamplePositionFromUniforms(source, position_uniforms);
  float3 const direction = GGEMS_SourceSampleDirectionFromUniforms(
      source, position, direction_uniforms);

  sampled_position[0] = position.x;
  sampled_position[1] = position.y;
  sampled_position[2] = position.z;
  sampled_direction[0] = direction.x;
  sampled_direction[1] = direction.y;
  sampled_direction[2] = direction.z;
}

// =============================================================================
// =============================================================================

__kernel void source_initialization_random_state_probe(
    __global GGEMSRandomState *sample_states,
    __global GGEMSRandomState *reference_states,
    __global GGEMSSourceRecord const *source,
    __global GGEMSEnergyDistributionRecord const *energy_distribution,
    __global ulong const *energy_values_milli_eV,
    __global ulong const *cumulative_ticket_upper,
    uint expected_position_vector_draw_count,
    uint expected_direction_vector_draw_count,
    uint expected_energy_raw_draw_count, ulong source_local_primary_id,
    __global long *sampled_positions, __global float *sampled_directions,
    __global ulong *sampled_values) {
  if (get_global_id(0) != 0U) {
    return;
  }

  GGEMSParticleState const particle = GGEMS_SourceInitializePrimary(
      19UL, source_local_primary_id, source, energy_distribution,
      energy_values_milli_eV, cumulative_ticket_upper, sample_states, 0U);

  float4 reference_position_uniforms = (float4)(0.0f);
  for (uint draw = 0U; draw < expected_position_vector_draw_count; ++draw) {
    reference_position_uniforms = GGEMS_RndmUniform4(reference_states, 0U);
  }

  float4 reference_direction_uniforms = (float4)(0.0f);
  for (uint draw = 0U; draw < expected_direction_vector_draw_count; ++draw) {
    reference_direction_uniforms = GGEMS_RndmUniform4(reference_states, 0U);
  }

  uint reference_raw_ticket = 0U;
  for (uint draw = 0U; draw < expected_energy_raw_draw_count; ++draw) {
    reference_raw_ticket = GGEMS_RndmUInt32(reference_states, 0U);
  }

  long3 const reference_position = GGEMS_SourceSamplePositionFromUniforms(
      source, reference_position_uniforms);
  float3 const reference_direction = GGEMS_SourceSampleDirectionFromUniforms(
      source, reference_position, reference_direction_uniforms);

  ulong reference_energy = source->energy_milli_eV;
  if (expected_energy_raw_draw_count != 0U) {
    reference_energy = GGEMS_EnergyDistributionSampleWithTicket(
        energy_distribution, energy_values_milli_eV, cumulative_ticket_upper,
        reference_raw_ticket);
  }

  sampled_positions[0] = particle.position_x_pm;
  sampled_positions[1] = particle.position_y_pm;
  sampled_positions[2] = particle.position_z_pm;
  sampled_positions[3] = reference_position.x;
  sampled_positions[4] = reference_position.y;
  sampled_positions[5] = reference_position.z;

  sampled_directions[0] = particle.direction_x;
  sampled_directions[1] = particle.direction_y;
  sampled_directions[2] = particle.direction_z;
  sampled_directions[3] = reference_direction.x;
  sampled_directions[4] = reference_direction.y;
  sampled_directions[5] = reference_direction.z;

  sampled_values[0] = particle.time_ps;
  sampled_values[1] = source->time_start_ps;
  sampled_values[2] = particle.energy_milli_eV;
  sampled_values[3] = reference_energy;
}
