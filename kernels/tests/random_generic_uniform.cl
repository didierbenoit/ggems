#include "core/random/GGEMSRandom.clh"

__kernel void random_generic_uniform(__global GGEMSRandomState *states,
                                     __global float *values,
                                     uint particle_count,
                                     uint samples_per_particle) {
  uint particle_index = get_global_id(0);

  if (particle_index >= particle_count) {
    return;
  }

  for (uint sample_index = 0U; sample_index < samples_per_particle;
       ++sample_index) {
    uint output_index = particle_index * samples_per_particle + sample_index;

    values[output_index] = GGEMS_RndmUniform(states, particle_index);
  }
}
