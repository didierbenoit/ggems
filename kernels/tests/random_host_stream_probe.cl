#include "core/random/GGEMSRandom.clh"

__kernel void
random_host_stream_probe(__global GGEMSRandomState *raw_states,
                         __global GGEMSRandomState *uniform_states,
                         __global uint *raw_values,
                         __global float *uniform_values, uint sample_count) {
  if (get_global_id(0) != 0U) {
    return;
  }

  for (uint sample_index = 0U; sample_index < sample_count; ++sample_index) {
    raw_values[sample_index] = GGEMS_RndmUInt32(raw_states, 0U);
    uniform_values[sample_index] = GGEMS_RndmUniform(uniform_states, 0U);
  }
}
