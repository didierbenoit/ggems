#include "core/particles/GGEMSParticleState.clh"
#include "core/random/GGEMSRandom.clh"

#ifndef GGEMS_DUMMY_LOCAL_STACK_CAPACITY
#define GGEMS_DUMMY_LOCAL_STACK_CAPACITY
#endif

#define GGEMS_DUMMY_COUNTER_NEXT_PRIMARY 0U
#define GGEMS_DUMMY_COUNTER_CONSUMED_PRIMARY 1U
#define GGEMS_DUMMY_COUNTER_COMPLETED_HISTORIES 2U
#define GGEMS_DUMMY_COUNTER_TERMINAL_PARTICLES 3U
#define GGEMS_DUMMY_COUNTER_CREATED_SECONDARIES 4U
#define GGEMS_DUMMY_COUNTER_AIONINO_TO_GAMMA 5U
#define GGEMS_DUMMY_COUNTER_GAMMA_TO_ELECTRON 6U
#define GGEMS_DUMMY_COUNTER_ELECTRON_TO_ELECTRON 7U
#define GGEMS_DUMMY_COUNTER_OVERFLOW 8U
#define GGEMS_DUMMY_COUNTER_MAX_STACK_DEPTH 9U
#define GGEMS_DUMMY_COUNTER_TOTAL_FAKE_STEPS 10U

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

__kernel void particle_dummy_stream_branching_transport(
    __global GGEMSRandomState *random_states,
    __global GGEMSParticleState *worker_final_states,
    volatile __global uint *counters, uint total_primary_count,
    ulong initial_energy_milli_eV, ulong min_energy_milli_eV,
    uint max_generation, uint max_steps_per_track) {
  uint worker_id = (uint)(get_global_id(0));

  GGEMSParticleState stack[GGEMS_DUMMY_LOCAL_STACK_CAPACITY];
  uint stack_size = 0U;
  uint local_max_stack_depth = 0U;
  uint processed_any_primary = 0U;
}
