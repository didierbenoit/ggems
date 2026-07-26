#include "core/random/GGEMSRandom.clh"
#include "core/sources/GGEMSSource.clh"

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

  float4 const primary_random_values =
      (float4)(uniform_values[0], uniform_values[1], uniform_values[2],
               uniform_values[3]);
  float4 const secondary_random_values =
      (float4)(uniform_values[4], uniform_values[5], uniform_values[6],
               uniform_values[7]);

  GGEMSParticleState const particle = GGEMS_SourceInitialisePrimary(
      0UL, 0UL, source, primary_random_values, secondary_random_values);

  sampled_position[0] = particle.position_x_pm;
  sampled_position[1] = particle.position_y_pm;
  sampled_position[2] = particle.position_z_pm;
  sampled_direction[0] = particle.direction_x;
  sampled_direction[1] = particle.direction_y;
  sampled_direction[2] = particle.direction_z;
}

// =============================================================================
// =============================================================================

__kernel void source_random_draw_plan_probe(
    __global GGEMSRandomState *sample_states,
    __global GGEMSRandomState *reference_states,
    __global GGEMSSourceRecord const *source, uint expected_vector_draw_count,
    uint expected_energy_raw_draw_count, __global uint *observed) {
  if (get_global_id(0) != 0U) {
    return;
  }

  uint const actual_vector_draw_count =
      GGEMS_SourceRandomVectorDrawCount(source);

  for (uint draw = 0U; draw < actual_vector_draw_count; ++draw) {
    (void)(GGEMS_RndmUniform4(sample_states, 0U));
  }

  uint sample_raw = 0U;
  if (expected_energy_raw_draw_count != 0U) {
    sample_raw = GGEMS_RndmUInt32(sample_states, 0U);
  }

  for (uint draw = 0U; draw < expected_vector_draw_count; ++draw) {
    (void)(GGEMS_RndmUniform4(reference_states, 0U));
  }

  uint reference_raw = 0U;
  if (expected_energy_raw_draw_count != 0U) {
    reference_raw = GGEMS_RndmUInt32(reference_states, 0U);
  }

  observed[0] = actual_vector_draw_count;
  observed[1] = sample_raw;
  observed[2] = reference_raw;
}
