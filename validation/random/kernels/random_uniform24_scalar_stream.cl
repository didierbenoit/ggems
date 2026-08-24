#include "random/GGEMSRandom.clh"

#define GGEMS_VALIDATION_LAYOUT_WORKER_MAJOR 0U
#define GGEMS_VALIDATION_LAYOUT_INTERLEAVED 1U

__kernel void random_uniform24_scalar_stream(__global GGEMSRandomState *states,
                                             __global float *values,
                                             uint worker_count,
                                             uint samples_per_worker,
                                             uint layout) {
  uint const worker_index = (uint)get_global_id(0);

  if (worker_index >= worker_count) {
    return;
  }

  ulong output_index = 0UL;
  ulong output_stride = 0UL;

  if (layout == GGEMS_VALIDATION_LAYOUT_WORKER_MAJOR) {
    output_index = (ulong)worker_index * (ulong)samples_per_worker;
    output_stride = 1UL;
  } else {
    output_index = (ulong)worker_index;
    output_stride = (ulong)worker_count;
  }

  for (uint sample_index = 0U; sample_index < samples_per_worker;
       ++sample_index) {
    values[output_index] = GGEMS_RndmUniform(states, worker_index);

    output_index += output_stride;
  }
}
