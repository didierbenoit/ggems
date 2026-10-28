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
 * \brief Claims source primaries and executes the selected transport path.
 *
 * Process-free species traverse the host-authored finite World and its
 * independent analytical occurrences and exit geometrically. Other species keep
 * the diagnostic one-meter projection and Killed status until their Processes
 * exist. No interaction, energy loss, time-of-flight update, or secondary is
 * modeled.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include "observer/GGEMSObserverRecord.clh"
#include "particles/GGEMSParticleState.clh"
#include "random/GGEMSRandom.clh"
#include "transport/GGEMSTransport.clh"

#include "transport/GGEMSTransportCounters.clh"
#include "transport/GGEMSDiagnosticProjection.clh"
#include "sources/GGEMSSourcePopulationRecord.clh"
#include "sources/GGEMSSourceEmissionRecord.clh"
#include "sources/GGEMSSourceEmissionRange.clh"
#include "sources/GGEMSEnergyDistributionRecord.clh"
#include "geometry/GGEMSAnalyticVolumeRecord.clh"
#include "navigation/GGEMSAnalyticNavigation.clh"
#include "geometry/GGEMSWorldRecord.clh"
#include "navigation/GGEMSNavigation.clh"
#include "sources/GGEMSSource.clh"
#include "sources/GGEMSSourceRecord.clh"
#include "sources/GGEMSSourceRunRange.clh"

#ifndef GGEMS_ENABLE_TRANSPORT_OBSERVER

/*! \brief Enables optional source and terminal observer writes when nonzero. */
#define GGEMS_ENABLE_TRANSPORT_OBSERVER 0
#endif

// =============================================================================
// =============================================================================

/*!
 * \brief Out-of-line geometry-only history: generic Navigation through the
 * World and its independent analytical occurrences until the World exit.
 *
 * The birth owner is located first. Each iteration performs one accepted
 * Transport transaction from the current owner (query, canonical move, owner
 * update); a World exit completes the history with EscapedWorld. In the
 * admitted straight-flight scene, the host sums each occurrence's maximum
 * crossing count and includes the World exit. This is a finite geometric
 * bound, not a silent traversal cap. Exhaustion is reported as unresolved,
 * as unresolved, never as a silent completion. The loop carries no shape
 * intersection code.
 *
 * This function and the Transport transaction it calls are deliberately kept
 * out of line. When the geometry loop is inlined into the claim/initialize/
 * complete loop below, the former -cl-fast-relaxed-math profile exposed
 * Intel CPU OpenCL compiler mixing of private particle states of neighboring
 * work-items: multi-source histories receive each other's positions or end
 * unresolved, including Gamma histories that never enter this branch. This
 * is a documented device-compiler boundary, not a performance choice; the
 * multi-source tests in tests/navigation guard it. Current production
 * kernels use standard optimized math without -cl-fast-relaxed-math.
 *
 * \param[in] world Immutable device World parameters.
 * \param[in] volume Immutable analytical array or one None record.
 * \param[in] box_pose Single N1 auxiliary view or occurrence descriptors,
 * rigid poses, and shared immutable profile data.
 * \param[in,out] particle Private state; moved to the World boundary on
 * success, owner identities updated at every accepted boundary.
 * \param[in,out] counters Transport counters receiving the outcome.
 * \return Zero when the history completed at the World boundary; nonzero when
 * the birth was outside the World or a transaction was unresolved.
 */
static /*! \cond */ __attribute__((noinline)) /*! \endcond */ uint
GGEMS_TransportGeometry(__global GGEMSWorldRecord const *world,
                        __global GGEMSAnalyticVolumeRecord const *volume,
                        __global void const *box_pose,
                        GGEMSParticleState *particle,
                        volatile __global GGEMSTransportCounters *counters) {
  ulong owner;

  uint const locate_status =
    GGEMS_SceneLocate(world, volume, box_pose,
                      (float3)(particle->position_x_pm, particle->position_y_pm,
                               particle->position_z_pm),
                      (float3)(particle->direction_x, particle->direction_y,
                               particle->direction_z),
                      &owner);

  if (locate_status != GGEMS_NAVIGATION_RESOLVED) {
    if (locate_status == GGEMS_NAVIGATION_OUTSIDE_WORLD) {
      atomic_inc(&counters->outside_world_count);
    } else {
      atomic_inc(&counters->unresolved_geometry_count);
    }
    return 1U;
  }

  particle->current_volume_id = (uint)(owner >> 32U);
  particle->material_id = (uint)(owner & 0xFFFFFFFFUL);

  uint const max_boundary_events = world->max_boundary_events;

  uint event = GGEMS_NAVIGATION_EVENT_NONE;
  uint faces = 0U;
  uint previous_volume = GGEMS_VOLUME_ID_WORLD;

  for (uint event_index = 0U; event_index < max_boundary_events;
       ++event_index) {
    uint const status = GGEMS_TransportToNextBoundary(
      world, volume, box_pose, particle, &event, &faces, &previous_volume);

    if (status == GGEMS_NAVIGATION_OUTSIDE_WORLD) {
      atomic_inc(&counters->outside_world_count);
      return 1U;
    }

    if (status != GGEMS_NAVIGATION_RESOLVED) {
      atomic_inc(&counters->unresolved_geometry_count);
      return 1U;
    }

    if (event == GGEMS_NAVIGATION_EVENT_WORLD_EXIT) {
      return 0U;
    }
  }

  atomic_inc(&counters->unresolved_geometry_count);

  return 1U;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Processes one host-partitioned chunk of source primaries.
 *
 * Active workers atomically claim chunk-local indices until the claim reaches
 * total_primary_count. The host supplies a zeroed claim counter, valid buffer
 * capacities, at least worker_count work-items, representable worker indices,
 * and enough uint headroom for the final unsuccessful claim from each worker.
 * Each active worker exclusively owns random_states[worker_id]; dynamic
 * claiming does not fix the primary-to-worker assignment.
 *
 * The run-wide population index is device_primary_offset plus the claimed
 * index. Adding projection_history_offset produces the global primary
 * identity; host preflight proves both additions representable and source
 * ranges contiguous and covering this chunk. The matching half-open source
 * range selects the source slot and source-local index. Birth dispatch is
 * infallible for the validated descriptors.
 *
 * A Process-free species (GGEMS_ParticleIsProcessFree) navigates the World
 * and its analytical occurrences boundary by boundary and commits its final
 * accepted interval before EscapedWorld completion. Outside births and
 * unresolved queries increment distinct failure counters without terminal
 * publication or history completion. Other species use the same FP32 movement
 * for a one-meter diagnostic projection and diagnostic Killed completion until
 * their physical Processes exist. A nonfinite endpoint fails without terminal
 * publication or completion. Energy, time and direction are unchanged in both
 * paths.
 *
 * With observer support enabled, capture selection precedes initialization
 * and optional source/terminal records have zero deposited energy. All
 * observer arguments remain in the signature when support is disabled, but
 * are unused. The private particle state is not returned in a global particle
 * array.
 *
 * \param[in,out] random_states Global host-initialized RNG states, with at
 * least worker_count elements.
 * \param[in,out] counters Global per-launch counters, initialized by the host
 * and atomically updated.
 * \param[in] source_records Global source records, with source_count
 * elements.
 * \param[in] source_ranges Global run-wide source ranges parallel to
 * source_records.
 * \param[in] source_count Number of source slots searched for each primary.
 * \param[in] total_primary_count Number of primaries in this chunk, with uint
 * claim-counter headroom.
 * \param[in] projection_history_offset Global identity offset for this run's
 * primary population.
 * \param[in] device_primary_offset Run-wide population index at the start of
 * this device chunk.
 * \param[in] observer_config Global capture configuration, read only when
 * observer support is enabled.
 * \param[in,out] observer_counters Global atomic capture/record counters,
 * used only with observer support.
 * \param[out] observer_records Global optional record buffer, writable for
 * observer_record_capacity entries.
 * \param[in] observer_record_capacity Capacity in observer records, not
 * bytes.
 * \param[in] run_id Run identity attached to optional observer records.
 * \param[in] worker_count Number of active workers and valid worker-owned RNG
 * states.
 * \param[in] energy_distribution_records Global source-indexed and
 * emission-indexed energy descriptors.
 * \param[in] energy_values_micro_eV Global energy table in unsigned integer
 * microelectronvolts.
 * \param[in] cumulative_ticket_upper Global exclusive cumulative ticket
 * bounds for energy tables.
 * \param[in] source_population_records Global population records parallel to
 * source_records.
 * \param[in] source_emissions Global emission descriptors referenced by
 * population records.
 * \param[in] source_emission_ranges Global source-local primary ranges
 * parallel to source_emissions.
 * \param[in] world Immutable World parameters; required for navigated
 * species.
 * \param[in] volume Analytical record array, or one None record.
 * \param[in] box_pose Single N1 auxiliary view or flat occurrence descriptors,
 * independent poses, and shared immutable profile data.
 */
__kernel void particle_stream_transport(
  __global GGEMSRandomState *random_states,
  volatile __global GGEMSTransportCounters *counters,
  __global GGEMSSourceRecord const *source_records,
  __global GGEMSSourceRunRange const *source_ranges, uint source_count,
  uint total_primary_count, ulong projection_history_offset,
  ulong device_primary_offset,
  __global GGEMSObserverConfigRecord const *observer_config,
  volatile __global GGEMSObserverCounters *observer_counters,
  __global GGEMSObserverRecord *observer_records, uint observer_record_capacity,
  ulong run_id, uint worker_count,
  __global GGEMSEnergyDistributionRecord const *energy_distribution_records,
  __global ulong const *energy_values_micro_eV,
  __global ulong const *cumulative_ticket_upper,
  __global GGEMSSourcePopulationRecord const *source_population_records,
  __global GGEMSSourceEmissionRecord const *source_emissions,
  __global GGEMSSourceEmissionRange const *source_emission_ranges,
  __global GGEMSWorldRecord const *world,
  __global GGEMSAnalyticVolumeRecord const *volume,
  __global void const *box_pose) {

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

  uint completed_count = 0U;
  uint escaped_count = 0U;

  while (1) {
    uint local_primary_id = atomic_add(&counters->next_primary_id, 1U);

    if (local_primary_id >= total_primary_count) {
      break;
    }

    ulong local_primary_id_u64 = (ulong)(local_primary_id);
    ulong projection_primary_id = device_primary_offset + local_primary_id_u64;

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

    __global GGEMSSourceRecord const *source =
      &source_records[selected_source_index];

    __global GGEMSSourcePopulationRecord const *population =
      &source_population_records[selected_source_index];

    GGEMSParticleState particle;
    GGEMS_SourceTryInitializePrimary(
      global_primary_id, source_local_primary_id, selected_source_index, source,
      population, source_emissions, source_emission_ranges,
      energy_distribution_records, energy_values_micro_eV,
      cumulative_ticket_upper, random_states, worker_id, &particle);

#if GGEMS_ENABLE_TRANSPORT_OBSERVER
    if (capture_history != 0U) {
      GGEMS_ObserverRecordParticle(
        observer_counters, observer_records, observer_record_capacity,
        GGEMS_OBSERVER_RECORD_KIND_SOURCE, run_id, global_primary_id,
        source_local_primary_id, selected_source_index, particle, 0UL);
    }
#endif

    if (GGEMS_ParticleIsProcessFree(particle.particle_type) != 0U) {
      if (GGEMS_TransportGeometry(world, volume, box_pose, &particle,
                                  counters) != 0U) {
        continue;
      }

      ++escaped_count;
    } else {
      if (GGEMS_TransportMoveToBoundary(GGEMS_DIAGNOSTIC_PROJECTION_DISTANCE_PM,
                                        &particle) == 0U) {
        atomic_inc(&counters->overflow_count);
        continue;
      }

      particle.status = GGEMS_PARTICLE_STATUS_KILLED;
    }

#if GGEMS_ENABLE_TRANSPORT_OBSERVER
    if (capture_history != 0U) {
      GGEMS_ObserverRecordParticle(
        observer_counters, observer_records, observer_record_capacity,
        GGEMS_OBSERVER_RECORD_KIND_TERMINAL, run_id, global_primary_id,
        source_local_primary_id, selected_source_index, particle, 0UL);
    }
#endif

    ++completed_count;
  }

  // Every worker publishes actual outcomes once. Claim scheduling is unchanged.
  // The host derives consumed from the exhausted chunk and terminal from
  // completed.
  if (completed_count != 0U) {
    atomic_add(&counters->completed_history_count, completed_count);
  }

  if (escaped_count != 0U) {
    atomic_add(&counters->escaped_world_count, escaped_count);
  }
}
