#include "core/random/GGEMSRandom.clh"

__kernel void random_generic_uniform4(__global GGEMSRandomState *states,
                                      __global float *values,
                                      uint particle_count,
                                      uint blocks_per_particle) {
  uint particle_index = get_global_id(0);

  if (particle_index >= particle_count) {
    return;
  }

  for (uint block_index = 0U; block_index < blocks_per_particle;
       ++block_index) {
    float4 random_values = GGEMS_RndmUniform4(states, particle_index);

    uint output_index =
        (particle_index * blocks_per_particle + block_index) * 4U;

    values[output_index + 0U] = random_values.x;
    values[output_index + 1U] = random_values.y;
    values[output_index + 2U] = random_values.z;
    values[output_index + 3U] = random_values.w;
  }
}
