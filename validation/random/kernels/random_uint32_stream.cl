#include "random/GGEMSRandom.clh"

static inline uint4 GGEMSValidation_RndmUInt4(__global GGEMSRandomState *states,
                                              uint index) {
#if GGEMS_RANDOM_ENGINE == GGEMS_RANDOM_ENGINE_JKISS
  return (uint4)(GGEMS_JKissNextUInt(states, index),
                 GGEMS_JKissNextUInt(states, index),
                 GGEMS_JKissNextUInt(states, index),
                 GGEMS_JKissNextUInt(states, index));

#elif GGEMS_RANDOM_ENGINE == GGEMS_RANDOM_ENGINE_PCG32
  return (uint4)(GGEMS_PCG32NextUInt(states, index),
                 GGEMS_PCG32NextUInt(states, index),
                 GGEMS_PCG32NextUInt(states, index),
                 GGEMS_PCG32NextUInt(states, index));

#elif GGEMS_RANDOM_ENGINE == GGEMS_RANDOM_ENGINE_PHILOX
  return GGEMS_PhiloxNextUInt4(states, index);
#else
#error "Unsupported GGEMS random engine for validation."
#endif
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

__kernel void random_uint32_stream(__global GGEMSRandomState *states,
                                   __global uint *values, uint particle_count,
                                   uint words_per_particle) {
  uint particle_index = (ulong)get_global_id(0);

  if (particle_index >= particle_count) {
    return;
  }

  ulong block_count = (words_per_particle + 3UL) / 4UL;

  for (ulong block_index = 0UL; block_index < block_count; ++block_index) {
    uint4 random_values = GGEMSValidation_RndmUInt4(states, particle_index);

    uint word_index = block_index * 4U;

    ulong output_index =
        (ulong)particle_index * (ulong)words_per_particle + (ulong)word_index;

    if (word_index + 0U < words_per_particle) {
      values[output_index + 0UL] = random_values.x;
    }

    if (word_index + 1U < words_per_particle) {
      values[output_index + 1UL] = random_values.y;
    }

    if (word_index + 2U < words_per_particle) {
      values[output_index + 2UL] = random_values.z;
    }

    if (word_index + 3U < words_per_particle) {
      values[output_index + 3UL] = random_values.w;
    }
  }
}
