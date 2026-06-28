#include "core/particles/GGEMSParticleState.clh"
#include "core/transport/GGEMSTransportCounters.clh"
#include "core/random/GGEMSRandom.clh"

#ifndef GGEMS_DUMMY_LOCAL_STACK_CAPACITY
#define GGEMS_DUMMY_LOCAL_STACK_CAPACITY 16U
#endif

#ifndef GGEMS_DUMMY_TRACK_BRANCH_BITS
#define GGEMS_DUMMY_TRACK_BRANCH_BITS 16U
#endif

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

static inline GGEMSParticleState
GGEMS_DummyMakeAionino(ulong global_primary_id, ulong initial_energy_milli_eV) {
  GGEMSParticleState particle;

  ulong base_track_id = global_primary_id << GGEMS_DUMMY_TRACK_BRANCH_BITS;

  particle.global_particle_id = global_primary_id;
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
                                 uint local_track_index, ulong base_track_id) {
  GGEMSParticleState secondary = parent;

  secondary.track_id = base_track_id + (ulong)(local_track_index);
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
    volatile __global GGEMSTransportCounters *counters,
    uint total_primary_count, ulong projection_history_offset,
    ulong device_primary_offset, ulong initial_energy_milli_eV,
    ulong min_energy_milli_eV, uint max_generation, uint max_steps_per_track) {
  uint worker_id = (uint)(get_global_id(0));

  GGEMSParticleState stack[GGEMS_DUMMY_LOCAL_STACK_CAPACITY];
  uint stack_size = 0U;
  uint local_max_stack_depth = 0U;
  uint processed_any_primary = 0U;

  GGEMSParticleState last_state = GGEMS_DummyMakeAionino(0U, 0U);
  last_state.status = GGEMS_PARTICLE_STATUS_INACTIVE;

  while (1) {
    uint local_primary_id = atomic_add(&counters->next_primary_id, 1U);

    if (local_primary_id >= total_primary_count) {
      break;
    }

    ulong global_primary_id = projection_history_offset +
                              device_primary_offset + (ulong)(local_primary_id);

    processed_any_primary = 1U;

    atomic_inc(&counters->consumed_primary_count);

    GGEMSParticleState current =
        GGEMS_DummyMakeAionino(global_primary_id, initial_energy_milli_eV);

    ulong base_track_id = current.track_id;

    current.particle_type = GGEMS_PARTICLE_TYPE_GAMMA;
    atomic_inc(&counters->aionino_to_gamma_count);

    stack_size = 0U;
    uint local_track_index = 1U;
    uint history_done = 0U;

    while (history_done == 0U) {
      if (current.status == GGEMS_PARTICLE_STATUS_ALIVE) {
        GGEMS_DummyMoveParticle(&current);
        atomic_inc(&counters->total_fake_step_count);

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
                  local_track_index, base_track_id);

              local_track_index += 1U;

              atomic_inc(&counters->created_secondary_count);
              atomic_inc(&counters->gamma_to_electron_count);
            } else {
              atomic_inc(&counters->overflow_count);
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
                  local_track_index, base_track_id);

              local_track_index += 1U;

              atomic_inc(&counters->created_secondary_count);
              atomic_inc(&counters->electron_to_electron_count);
            } else {
              atomic_inc(&counters->overflow_count);
            }
          }
        }
      }

      if (current.status != GGEMS_PARTICLE_STATUS_ALIVE) {
        atomic_inc(&counters->terminal_particle_count);
        last_state = current;

        if (stack_size > 0U) {
          stack_size -= 1;
          current = stack[stack_size];
        } else {
          history_done = 1U;
          atomic_inc(&counters->completed_history_count);
        }
      }
    }
  }

  if (processed_any_primary != 0U) {
    worker_final_states[worker_id] = last_state;
  }

  atomic_max(&counters->max_stack_depth, local_max_stack_depth);
}
