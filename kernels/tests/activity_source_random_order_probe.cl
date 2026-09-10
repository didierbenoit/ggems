#include "sources/GGEMSSource.clh"

// =============================================================================
// =============================================================================

__kernel void activity_source_random_order_probe(
    __global GGEMSRandomState *sample_states,
    __global GGEMSRandomState *reference_states,
    __global GGEMSSourceRecord const *source,
    __global GGEMSSourcePopulationRecord const *population,
    __global GGEMSSourceEmissionRecord const *emission,
    __global GGEMSEnergyDistributionRecord const *energy_distribution,
    __global ulong const *energy_values_micro_eV,
    __global ulong const *cumulative_ticket_upper,
    __global long *sampled_positions, __global float *sampled_directions,
    __global ulong *sampled_values, __global uint *next_words) {
  if (get_global_id(0) != 0U) {
    return;
  }

  GGEMSParticleState const particle =
      GGEMS_SourceInitializeActivityDrivenPrimary(
          19UL, 23UL, source, population, emission, energy_distribution,
          energy_values_micro_eV, cumulative_ticket_upper, sample_states, 0U);

  uint const reference_time_word = GGEMS_RndmUInt32(reference_states, 0U);
  float4 const reference_position_uniforms =
      GGEMS_RndmUniform4(reference_states, 0U);
  float4 const reference_direction_uniforms =
      GGEMS_RndmUniform4(reference_states, 0U);
  uint const reference_energy_word = GGEMS_RndmUInt32(reference_states, 0U);

  ulong const reference_time = GGEMS_SampleRadioactiveTimeFromRaw(
      source->time_start_ps, source->time_stop_ps, population->scaled_decay,
      reference_time_word);
  long3 const reference_position = GGEMS_SourceSamplePositionFromUniforms(
      source, reference_position_uniforms);
  float3 const reference_direction = GGEMS_SourceSampleDirectionFromUniforms(
      source, reference_position, reference_direction_uniforms);
  ulong const reference_energy = GGEMS_EnergyDistributionSampleWithTicket(
      energy_distribution, energy_values_micro_eV, cumulative_ticket_upper,
      reference_energy_word);

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
  sampled_values[1] = reference_time;
  sampled_values[2] = particle.energy_micro_eV;
  sampled_values[3] = reference_energy;

  next_words[0] = GGEMS_RndmUInt32(sample_states, 0U);
  next_words[1] = GGEMS_RndmUInt32(reference_states, 0U);
}
