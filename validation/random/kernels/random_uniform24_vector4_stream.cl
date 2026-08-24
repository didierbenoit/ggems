#include "random/GGEMSRandom.clh"

#define GGEMS_VALIDATION_LAYOUT_WORKER_MAJOR 0U
#define GGEMS_VALIDATION_LAYOUT_INTERLEAVED 1U

__kernel void random_uniform24_vector4_stream(__global GGEMSRandomState *states,
                                              __global float *values,
                                              uint worker_count,
                                              uint samples_per_worker,
                                              uint layout) {
  uint const worker_index = (uint)get_global_id(0);

  if (worker_index >= worker_count) {
    return;
  }

  uint const blocks_per_worker = samples_per_worker / 4U;

  for (uint block_index = 0U; block_index < blocks_per_worker; ++block_index) {
    float4 const random_values = GGEMS_RndmUniform4(states, worker_index);
    uint const first_sample_index = block_index * 4U;

    if (layout == GGEMS_VALIDATION_LAYOUT_WORKER_MAJOR) {
      ulong const output_index =
          ((ulong)worker_index * (ulong)samples_per_worker) +
          (ulong)first_sample_index;

      values[output_index + 0UL] = random_values.x;
      values[output_index + 1UL] = random_values.y;
      values[output_index + 2UL] = random_values.z;
      values[output_index + 3UL] = random_values.w;
    } else {
      ulong const worker_count_64 = (ulong)worker_count;
      ulong const worker_index_64 = (ulong)worker_index;
      ulong const sample_index_64 = (ulong)first_sample_index;

      values[((sample_index_64 + 0UL) * worker_count_64) + worker_index_64] =
          random_values.x;
      values[((sample_index_64 + 1UL) * worker_count_64) + worker_index_64] =
          random_values.y;
      values[((sample_index_64 + 2UL) * worker_count_64) + worker_index_64] =
          random_values.z;
      values[((sample_index_64 + 3UL) * worker_count_64) + worker_index_64] =
          random_values.w;
    }
  }
}
