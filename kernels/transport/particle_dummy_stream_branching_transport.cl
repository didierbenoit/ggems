// *****************************************************************************
// * This file is part of GGEMS.                                               *
// *                                                                           *
// * SPDX-License-Identifier: GPL-3.0-or-later                                 *
// * Copyright (C) 2017-2026 CHRU de Brest, Université de Bretagne Occidentale,*
// * Inserm.                                                                   *
// *                                                                           *
// * GGEMS is free software: you can redistribute it and/or modify             *
// * it under the terms of the GNU General Public License as published by      *
// * the Free Software Foundation, either version 3 of the License, or         *
// * (at your option) any later version.                                       *
// *                                                                           *
// * GGEMS is distributed in the hope that it will be useful,                  *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
// * GNU General Public License for more details.                              *
// *                                                                           *
// * You should have received a copy of the GNU General Public License         *
// * along with GGEMS. If not, see <https://www.gnu.org/licenses/>.            *
// *****************************************************************************
/*!
 * \file
 * \brief Exercises synthetic branching and bounded per-worker history stacks.
 *
 * This diagnostic workload does not implement physical particle interactions.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */
#include "particles/GGEMSParticleState.clh"
#include "transport/GGEMSTransportCounters.clh"
#include "random/GGEMSRandom.clh"
#include "sources/GGEMSSource.clh"
#include "sources/GGEMSSourceRecord.clh"
#include "sources/GGEMSSourceRunRange.clh"
#include "observer/GGEMSObserverRecord.clh"

#ifndef GGEMS_DUMMY_LOCAL_STACK_CAPACITY
/*! \brief Maximum number of suspended parent tracks per diagnostic worker. */
#define GGEMS_DUMMY_LOCAL_STACK_CAPACITY 16U
#endif

#ifndef GGEMS_ENABLE_TRANSPORT_OBSERVER
/*! \brief Compiles optional diagnostic capture when nonzero. */
#define GGEMS_ENABLE_TRANSPORT_OBSERVER 0
#endif

// =============================================================================
// =============================================================================

/*!
 * \brief Constructs the private sentinel used before a worker processes a primary.
 *
 * \return Inactive Aionino at the origin, with zero energy/time, +Z direction,
 * and invalid history, track, and geometry identities.
 */
static inline GGEMSParticleState GGEMS_MakeInactiveAionino(void) {
  GGEMSParticleState particle;

  particle.global_particle_id = GGEMS_INVALID_ID_U64;
  particle.track_id = GGEMS_INVALID_ID_U32;
  particle.parent_track_id = GGEMS_INVALID_ID_U32;
  particle.time_ps = 0UL;

  particle.position_x_pm = 0.0F;
  particle.position_y_pm = 0.0F;
  particle.position_z_pm = 0.0F;

  particle.particle_type = GGEMS_PARTICLE_TYPE_AIONINO;
  particle.status = GGEMS_PARTICLE_STATUS_INACTIVE;
  particle.generation = 0U;

  particle.current_navigator_id = GGEMS_INVALID_ID_U32;
  particle.current_volume_id = GGEMS_INVALID_ID_U32;
  particle.material_id = GGEMS_INVALID_ID_U32;
  particle.region_id = GGEMS_INVALID_ID_U32;

  particle.direction_x = 0.0f;
  particle.direction_y = 0.0f;
  particle.direction_z = 1.0f;

  particle.energy_micro_eV = 0UL;

  return particle;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Creates a synthetic electron by copying its parent state.
 *
 * Position, time, geometry identities, and global history identity are
 * inherited. The new track is alive and points along global +X. The caller
 * has already split the parent's integer energy; no physical model is used.
 *
 * \param[in] parent Parent state after the synthetic energy split.
 * \param[in] energy_micro_eV Assigned child kinetic energy in micro-eV.
 * \param[in] generation Child generation; integer halving bounds this chain.
 * \param[in] local_track_index Next history-local track identifier.
 * \return Alive synthetic electron with the supplied energy and track ID.
 */
static inline GGEMSParticleState
GGEMS_DummyMakeElectronSecondary(GGEMSParticleState parent,
                                 ulong energy_micro_eV, uint generation,
                                 uint local_track_index) {
  GGEMSParticleState secondary = parent;

  secondary.track_id = local_track_index;
  secondary.parent_track_id = parent.track_id;
  secondary.particle_type = GGEMS_PARTICLE_TYPE_ELECTRON;
  secondary.status = GGEMS_PARTICLE_STATUS_ALIVE;
  // Repeated integer-energy halving bounds this synthetic chain to 64 levels.
  secondary.generation = (uchar)generation;

  secondary.energy_micro_eV = energy_micro_eV;

  secondary.direction_x = 1.0f;
  secondary.direction_y = 0.0f;
  secondary.direction_z = 0.0f;

  return secondary;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Advances the diagnostic species by fixed global offsets and time ticks.
 *
 * Gamma receives +100 mm in Z. Electron receives +50 mm in X and +25 mm
 * in Z. Every species receives 10000 ps; stored direction is not consulted.
 * These synthetic offsets are not Navigation or physical time of flight.
 *
 * \param[in,out] p Private state whose position and time are updated.
 */
static inline void GGEMS_DummyMoveParticle(__private GGEMSParticleState *p) {
  if (p->particle_type == GGEMS_PARTICLE_TYPE_GAMMA) {
    p->position_z_pm += 100000000000.0f;
  } else if (p->particle_type == GGEMS_PARTICLE_TYPE_ELECTRON) {
    p->position_x_pm += 50000000000.0f;
    p->position_z_pm += 25000000000.0f;
  }

  p->time_ps += 10000UL;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Marks a synthetic track killed at its diagnostic energy or step limit.
 *
 * \param[in,out] p Private state; status changes when either limit is reached.
 * \param[in] min_energy_micro_eV Inclusive synthetic stopping threshold.
 * \param[in] max_steps_per_track Inclusive synthetic step-count limit.
 * \param[in] step_count Steps already performed by this track.
 */
static inline void GGEMS_DummyKillIfFinished(__private GGEMSParticleState *p,
                                             ulong min_energy_micro_eV,
                                             uint max_steps_per_track,
                                             uint step_count) {
  if (p->energy_micro_eV <= min_energy_micro_eV ||
      step_count >= max_steps_per_track) {
    p->status = GGEMS_PARTICLE_STATUS_KILLED;
  }
}

// =============================================================================
// =============================================================================

/*!
 * \brief Runs the diagnostic depth-first branching workload on claimed primaries.
 *
 * Each active worker exclusively owns one RNG state and a bounded private
 * stack. Dynamic atomic claims select source ranges; births copy the source
 * record directly, bypassing the physical source geometry and energy laws.
 * Synthetic Gamma/Electron branching halves integer energy and uses fixed
 * global offsets. This is not physical Process or Navigation behavior.
 *
 * A missing source range increments overflow_count and skips that claim.
 * Stack exhaustion also increments overflow_count after the parent's energy
 * has been split; no child is stored in that case. These diagnostic failure
 * semantics must not be interpreted as a production Transport contract.
 * The host supplies valid capacities and claim-counter headroom and checks
 * counters after completion. Workers with no primary leave final state
 * slots untouched. Observer record order is scheduling dependent.
 *
 * \param[in,out] random_states Host-initialized global states, one per worker.
 * \param[out] worker_final_states Last terminal state per worker that ran work.
 * \param[in,out] counters Host-initialized global atomic diagnostic counters.
 * \param[in] source_records Immutable global source records.
 * \param[in] source_ranges Parallel half-open run-population ranges.
 * \param[in] source_count Number of source slots.
 * \param[in] total_primary_count Number of primaries in this launch chunk.
 * \param[in] projection_history_offset Run-wide global history-ID offset.
 * \param[in] device_primary_offset Run-population offset of this device chunk.
 * \param[in] min_energy_micro_eV Inclusive synthetic stopping energy.
 * \param[in] max_generation Maximum allowed synthetic descendant generation.
 * \param[in] max_steps_per_track Inclusive synthetic per-track step limit.
 * \param[in] observer_config Optional immutable global capture selection.
 * \param[in,out] observer_counters Optional global atomic capture counters.
 * \param[out] observer_records Optional global buffer of diagnostic records.
 * \param[in] observer_record_capacity Writable record count, not bytes.
 * \param[in] run_id Run identity attached to captured records.
 * \param[in] worker_count Active worker count and minimum per-worker capacity.
 */
__kernel void particle_dummy_stream_branching_transport(
  __global GGEMSRandomState *random_states,
  __global GGEMSParticleState *worker_final_states,
  volatile __global GGEMSTransportCounters *counters,
  __global GGEMSSourceRecord const *source_records,
  __global GGEMSSourceRunRange const *source_ranges, uint source_count,
  uint total_primary_count, ulong projection_history_offset,
  ulong device_primary_offset, ulong min_energy_micro_eV, uint max_generation,
  uint max_steps_per_track,
  __global GGEMSObserverConfigRecord const *observer_config,
  volatile __global GGEMSObserverCounters *observer_counters,
  __global GGEMSObserverRecord *observer_records, uint observer_record_capacity,
  ulong run_id, uint worker_count) {
#if GGEMS_ENABLE_TRANSPORT_OBSERVER == 0
  (void)(observer_config);
  (void)(observer_counters);
  (void)(observer_records);
  (void)(observer_record_capacity);
  (void)(run_id);
#endif

  uint worker_id = (uint)(get_global_id(0));

  if (worker_id >= worker_count) {
    return;
  }

  GGEMSParticleState stack[GGEMS_DUMMY_LOCAL_STACK_CAPACITY];

  // Step accounting belongs to this diagnostic algorithm, not particle state.
  uint stack_step_counts[GGEMS_DUMMY_LOCAL_STACK_CAPACITY];
  uint stack_size = 0U;
  uint local_max_stack_depth = 0U;
  uint processed_any_primary = 0U;

  GGEMSParticleState last_state = GGEMS_MakeInactiveAionino();
  last_state.status = GGEMS_PARTICLE_STATUS_INACTIVE;

  while (1) {
    uint local_primary_id = atomic_add(&counters->next_primary_id, 1U);

    if (local_primary_id >= total_primary_count) {
      break;
    }

    ulong projection_primary_id =
      device_primary_offset + (ulong)(local_primary_id);

    ulong global_primary_id = projection_history_offset + projection_primary_id;

    uint selected_source_index = source_count;

    for (uint source_index = 0U; source_index < source_count; ++source_index) {
      ulong begin = source_ranges[source_index].projection_primary_begin;
      ulong count = source_ranges[source_index].primary_count;

      if (count != 0UL && projection_primary_id >= begin &&
          projection_primary_id - begin < count) {
        selected_source_index = source_index;
        break;
      }
    }

    if (selected_source_index == source_count) {
      atomic_inc(&counters->overflow_count);
      continue;
    }

    ulong source_local_primary_id =
      projection_primary_id -
      source_ranges[selected_source_index].projection_primary_begin;

#if GGEMS_ENABLE_TRANSPORT_OBSERVER
    uint capture_history = GGEMS_ObserverShouldCapturePrimary(
      observer_config, selected_source_index, source_local_primary_id);

    if (capture_history != 0U) {
      atomic_inc(&observer_counters->captured_primary_count);
    }
#endif

    processed_any_primary = 1U;

    atomic_inc(&counters->consumed_primary_count);

    __global GGEMSSourceRecord const *source_record =
      &source_records[selected_source_index];

    GGEMSParticleState current = GGEMS_SourceReadAionino(
      global_primary_id, source_local_primary_id, source_record);

#if GGEMS_ENABLE_TRANSPORT_OBSERVER
    if (capture_history != 0) {
      GGEMS_ObserverRecordParticle(
        observer_counters, observer_records, observer_record_capacity,
        GGEMS_OBSERVER_RECORD_KIND_SOURCE, run_id, global_primary_id,
        source_local_primary_id, selected_source_index, current, 0UL);
    }
#endif

    uint step_count = 0U;

    if (current.particle_type == GGEMS_PARTICLE_TYPE_GAMMA) {
      atomic_inc(&counters->aionino_to_gamma_count);
    }

    stack_size = 0U;
    uint local_track_index = 1U;
    uint history_done = 0U;

    while (history_done == 0U) {
      if (current.status == GGEMS_PARTICLE_STATUS_ALIVE) {
        GGEMS_DummyMoveParticle(&current);
        ++step_count;
        atomic_inc(&counters->total_fake_step_count);

#if GGEMS_ENABLE_TRANSPORT_OBSERVER
        if (capture_history != 0U) {
          GGEMS_ObserverRecordParticle(
            observer_counters, observer_records, observer_record_capacity,
            GGEMS_OBSERVER_RECORD_KIND_STEP, run_id, global_primary_id,
            source_local_primary_id, selected_source_index, current, 0UL);
        }
#endif

        GGEMS_DummyKillIfFinished(&current, min_energy_micro_eV,
                                  max_steps_per_track, step_count);

        // Alive implies energy > min_energy, so subtraction is safe even
        // when twice the canonical energy threshold would overflow ulong.
        if (current.status == GGEMS_PARTICLE_STATUS_ALIVE && step_count == 1U &&
            current.generation < max_generation &&
            current.energy_micro_eV - min_energy_micro_eV >
              min_energy_micro_eV) {
          float u = GGEMS_RndmUniform(random_states, worker_id);

          if (current.particle_type == GGEMS_PARTICLE_TYPE_GAMMA && u > 0.8f) {
            ulong secondary_energy = current.energy_micro_eV >> 1;
            current.energy_micro_eV -= secondary_energy;

            if (stack_size < GGEMS_DUMMY_LOCAL_STACK_CAPACITY) {
              stack[stack_size] = current;
              stack_step_counts[stack_size] = step_count;
              stack_size += 1U;

              if (stack_size > local_max_stack_depth) {
                local_max_stack_depth = stack_size;
              }

              current = GGEMS_DummyMakeElectronSecondary(
                current, secondary_energy, current.generation + 1U,
                local_track_index);
              step_count = 0U;

              local_track_index += 1U;

#if GGEMS_ENABLE_TRANSPORT_OBSERVER
              if (capture_history != 0U) {
                GGEMS_ObserverRecordParticle(
                  observer_counters, observer_records, observer_record_capacity,
                  GGEMS_OBSERVER_RECORD_KIND_SECONDARY_STEP, run_id,
                  global_primary_id, source_local_primary_id,
                  selected_source_index, current, 0UL);
              }
#endif

              atomic_inc(&counters->created_secondary_count);
              atomic_inc(&counters->gamma_to_electron_count);
            } else {
              atomic_inc(&counters->overflow_count);
            }
          } else if (current.particle_type == GGEMS_PARTICLE_TYPE_ELECTRON &&
                     u > 0.5f) {
            ulong secondary_energy = current.energy_micro_eV >> 1;
            current.energy_micro_eV -= secondary_energy;

            if (stack_size < GGEMS_DUMMY_LOCAL_STACK_CAPACITY) {
              stack[stack_size] = current;
              stack_step_counts[stack_size] = step_count;
              stack_size += 1U;

              if (stack_size > local_max_stack_depth) {
                local_max_stack_depth = stack_size;
              }

              current = GGEMS_DummyMakeElectronSecondary(
                current, secondary_energy, current.generation + 1U,
                local_track_index);
              step_count = 0U;

              local_track_index += 1U;

#if GGEMS_ENABLE_TRANSPORT_OBSERVER
              if (capture_history != 0U) {
                GGEMS_ObserverRecordParticle(
                  observer_counters, observer_records, observer_record_capacity,
                  GGEMS_OBSERVER_RECORD_KIND_SECONDARY_STEP, run_id,
                  global_primary_id, source_local_primary_id,
                  selected_source_index, current, 0UL);
              }
#endif

              atomic_inc(&counters->created_secondary_count);
              atomic_inc(&counters->electron_to_electron_count);
            } else {
              atomic_inc(&counters->overflow_count);
            }
          }
        }
      }

      if (current.status != GGEMS_PARTICLE_STATUS_ALIVE) {

#if GGEMS_ENABLE_TRANSPORT_OBSERVER
        if (capture_history != 0U) {
          GGEMS_ObserverRecordParticle(
            observer_counters, observer_records, observer_record_capacity,
            GGEMS_OBSERVER_RECORD_KIND_TERMINAL, run_id, global_primary_id,
            source_local_primary_id, selected_source_index, current, 0UL);
        }
#endif

        atomic_inc(&counters->terminal_particle_count);
        last_state = current;

        if (stack_size > 0U) {
          stack_size -= 1;
          current = stack[stack_size];
          step_count = stack_step_counts[stack_size];
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
