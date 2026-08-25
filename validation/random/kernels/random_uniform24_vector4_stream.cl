#include "random/GGEMSRandom.clh"

#define GGEMS_VALIDATION_LAYOUT_WORKER_MAJOR 0U
#define GGEMS_VALIDATION_LAYOUT_INTERLEAVED 1U

__kernel void random_uniform24_vector4_stream(__global GGEMSRandomState *states,
                                              __global float *values,
                                              uint worker_count,
                                              uint samples_per_worker,
                                              uint layout, uint lanes_used) {
  uint const worker_index = (uint)get_global_id(0);

  if (worker_index >= worker_count) {
    return;
  }

  uint const calls_per_worker = samples_per_worker / lanes_used;

  for (uint call_index = 0U; call_index < calls_per_worker; ++call_index) {
    float4 const random_values = GGEMS_RndmUniform4(states, worker_index);
    float const lane_values[4] = {random_values.x, random_values.y,
                                  random_values.z, random_values.w};
    uint const first_sample_index = call_index * lanes_used;

    for (uint lane_index = 0U; lane_index < lanes_used; ++lane_index) {
      if (layout == GGEMS_VALIDATION_LAYOUT_WORKER_MAJOR) {
        ulong const output_index =
            ((ulong)worker_index * (ulong)samples_per_worker) +
            (ulong)first_sample_index + (ulong)lane_index;

        values[output_index] = lane_values[lane_index];
      } else {
        ulong const sample_index =
            (ulong)first_sample_index + (ulong)lane_index;
        ulong const output_index =
            (sample_index * (ulong)worker_count) + (ulong)worker_index;

        values[output_index] = lane_values[lane_index];
      }
    }
  }
}
