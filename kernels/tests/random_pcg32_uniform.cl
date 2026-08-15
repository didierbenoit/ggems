#include "core/random/GGEMSPCG32.clh"

__kernel void random_pcg32_uniform(__global GGEMSPCG32State *states,
                                   __global float *values, uint particle_count,
                                   uint samples_per_particle) {
  uint particle_index = get_global_id(0);

  if (particle_index >= particle_count) {
    return;
  }

  for (uint sample_index = 0U; sample_index < samples_per_particle;
       ++sample_index) {
    uint output_index = particle_index * samples_per_particle + sample_index;

    values[output_index] = GGEMS_PCG32Uniform(states, particle_index);
  }
}
