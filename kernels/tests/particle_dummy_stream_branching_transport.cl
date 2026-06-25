#include "core/particles/GGEMSParticleState.clh"
#include "core/random/GGEMSRandom.clh"

#ifndef GGEMS_DUMMY_LOCAL_STACK_CAPACITY
#define GGEMS_DUMMY_LOCAL_STACK_CAPACITY 16U
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

static inline GGEMSParticleState
GGEMS_DummyMakeAionino(uint primary_id, ulong initial_energy_milli_eV) {
  GGEMSParticleState particle;

  ulong base_track_id = ((ulong)(primary_id) << 32U);

  particle.global_particle_id = (ulong)(primary_id);
  particle.track_id = base_track_id;
  particle.parent_track_id = GGEMS_INVALID_ID_U64;
  particle.time_ps = 0UL;

  particle.position_x_pm = 0L;
  particle.position_y_pm = 0L;
  particle.position_z_pm = 0L;

  particle.particle_type = GGEMS_PARTICLE_TYPE_AIONINO;
  particle.status = GGEMS_PARTICLE_STATUS_ALIVE;
  particle.generation = 0U;
  particle.flags = 0U;

  particle.current_navigator_id = GGEMS_INVALID_ID_U32;
  particle.current_volume_id = GGEMS_INVALID_ID_U32;
  particle.material_id = GGEMS_INVALID_ID_U32;
  particle.region_id = GGEMS_INVALID_ID_U32;

  particle.direction_x = 0.0f;
  particle.direction_y = 0.0f;
  particle.direction_z = 1.0f;
  particle.direction_w = 0.0f;

  particle.energy_milli_eV = initial_energy_milli_eV;
  particle.weight = 1.0f;

  return particle;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

static inline GGEMSParticleState
GGEMS_DummyMakeElectronSecondary(GGEMSParticleState parent,
                                 ulong energy_milli_eV, uint generation,
                                 uint local_track_index) {
  GGEMSParticleState secondary = parent;

  secondary.track_id = parent.track_id + (ulong)(local_track_index);
  secondary.parent_track_id = parent.track_id;
  secondary.particle_type = GGEMS_PARTICLE_TYPE_ELECTRON;
  secondary.status = GGEMS_PARTICLE_STATUS_ALIVE;
  secondary.generation = generation;
  secondary.flags = 0U;

  secondary.energy_milli_eV = energy_milli_eV;

  secondary.direction_x = 1.0f;
  secondary.direction_y = 0.0f;
  secondary.direction_z = 0.0f;
  secondary.direction_w = 0.0f;

  return secondary;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

static inline void GGEMS_DummyMoveParticle(__private GGEMSParticleState *p) {
  if (p->particle_type == GGEMS_PARTICLE_TYPE_GAMMA) {
    p->position_z_pm += 1000L;
  } else if (p->particle_type == GGEMS_PARTICLE_TYPE_ELECTRON) {
    p->position_x_pm += 500L;
    p->position_z_pm += 250L;
  }

  p->time_ps += 1UL;
  p->flags += 1U;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

static inline void GGEMS_DummyKillIfFinished(__private GGEMSParticleState *p,
                                             ulong min_energy_milli_eV,
                                             uint max_steps_per_track) {
  if (p->energy_milli_eV <= min_energy_milli_eV ||
      p->flags >= max_steps_per_track) {
    p->status = GGEMS_PARTICLE_STATUS_KILLED;
  }
}

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

  GGEMSParticleState last_state = GGEMS_DummyMakeAionino(0U, 0U);
  last_state.status = GGEMS_PARTICLE_STATUS_INACTIVE;

  while (1) {
    uint primary_id =
        atomic_add(&counters[GGEMS_DUMMY_COUNTER_NEXT_PRIMARY], 1U);

    if (primary_id >= total_primary_count) {
      break;
    }

    processed_any_primary = 1U;

    atomic_inc(&counters[GGEMS_DUMMY_COUNTER_CONSUMED_PRIMARY]);

    GGEMSParticleState current =
        GGEMS_DummyMakeAionino(primary_id, initial_energy_milli_eV);
    current.particle_type = GGEMS_PARTICLE_TYPE_GAMMA;
    atomic_inc(&counters[GGEMS_DUMMY_COUNTER_AIONINO_TO_GAMMA]);

    stack_size = 0U;
    uint local_track_index = 1U;
    uint history_done = 0U;

    while (history_done == 0U) {
      if (current.status == GGEMS_PARTICLE_STATUS_ALIVE) {
        GGEMS_DummyMoveParticle(&current);
        atomic_inc(&counters[GGEMS_DUMMY_COUNTER_TOTAL_FAKE_STEPS]);

        GGEMS_DummyKillIfFinished(&current, min_energy_milli_eV,
                                  max_steps_per_track);

        if (current.status == GGEMS_PARTICLE_STATUS_ALIVE &&
            current.flags == 1U && current.generation < max_generation &&
            current.energy_milli_eV > (2UL * min_energy_milli_eV)) {
          float u = GGEMS_RndmUniform(random_states, worker_id);

          if (current.particle_type == GGEMS_PARTICLE_TYPE_GAMMA && u > 0.8f) {
            ulong secondary_energy = current.energy_milli_eV / 2ULL;
            current.energy_milli_eV -= secondary_energy;

            if (stack_size < GGEMS_DUMMY_LOCAL_STACK_CAPACITY) {
              stack[stack_size] = current;
              stack_size += 1U;

              if (stack_size > local_max_stack_depth) {
                local_max_stack_depth = stack_size;
              }

              current = GGEMS_DummyMakeElectronSecondary(
                  current, secondary_energy, current.generation + 1U,
                  local_track_index);

              local_track_index += 1U;

              atomic_inc(&counters[GGEMS_DUMMY_COUNTER_CREATED_SECONDARIES]);
              atomic_inc(&counters[GGEMS_DUMMY_COUNTER_GAMMA_TO_ELECTRON]);
            } else {
              atomic_inc(&counters[GGEMS_DUMMY_COUNTER_OVERFLOW]);
            }
          } else if (current.particle_type == GGEMS_PARTICLE_TYPE_ELECTRON &&
                     u > 0.5) {
            ulong secondary_energy = current.energy_milli_eV / 2ULL;
            current.energy_milli_eV -= secondary_energy;

            if (stack_size < GGEMS_DUMMY_LOCAL_STACK_CAPACITY) {
              stack[stack_size] = current;
              stack_size += 1U;

              if (stack_size > local_max_stack_depth) {
                local_max_stack_depth = stack_size;
              }

              current = GGEMS_DummyMakeElectronSecondary(
                  current, secondary_energy, current.generation + 1U,
                  local_track_index);

              local_track_index += 1U;

              atomic_inc(&counters[GGEMS_DUMMY_COUNTER_CREATED_SECONDARIES]);
              atomic_inc(&counters[GGEMS_DUMMY_COUNTER_ELECTRON_TO_ELECTRON]);
            } else {
              atomic_inc(&counters[GGEMS_DUMMY_COUNTER_OVERFLOW]);
            }
          }
        }
      }

      if (current.status != GGEMS_PARTICLE_STATUS_ALIVE) {
        atomic_inc(&counters[GGEMS_DUMMY_COUNTER_TERMINAL_PARTICLES]);
        last_state = current;

        if (stack_size > 0U) {
          stack_size -= 1;
          current = stack[stack_size];
        } else {
          history_done = 1U;
          atomic_inc(&counters[GGEMS_DUMMY_COUNTER_COMPLETED_HISTORIES]);
        }
      }
    }
  }

  if (processed_any_primary != 0U) {
    worker_final_states[worker_id] = last_state;
  }

  atomic_max(&counters[GGEMS_DUMMY_COUNTER_MAX_STACK_DEPTH],
             local_max_stack_depth);
}
