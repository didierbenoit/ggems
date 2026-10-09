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
 * \brief Claims source primaries and performs a diagnostic endpoint projection.
 *
 * Aionino traverses the host-authored finite World and exits geometrically.
 * Other species keep the diagnostic one-meter projection and Killed status.
 * No interaction, energy loss, time-of-flight update, or secondary is modeled.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include "observer/GGEMSObserverRecord.clh"
#include "particles/GGEMSParticleState.clh"
#include "random/GGEMSRandom.clh"
#include "transport/GGEMSWorldTransport.clh"
#include "transport/GGEMSDiagnosticProjection.clh"
#include "transport/GGEMSTransportCounters.clh"
#include "sources/GGEMSSourcePopulationRecord.clh"
#include "sources/GGEMSSourceEmissionRecord.clh"
#include "sources/GGEMSSourceEmissionRange.clh"
#include "sources/GGEMSEnergyDistributionRecord.clh"
#include "geometry/GGEMSWorldRecord.clh"
#include "navigation/GGEMSWorldNavigation.clh"
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
 * \brief Out-of-line Aionino step: World transport plus its counters.
 *
 * This function and GGEMS_TransportAioninoToWorld are deliberately kept out of
 * line. When the Aionino branch is inlined into the claim/initialize/complete
 * loop below, the Intel CPU OpenCL compiler with -cl-fast-relaxed-math mixes
 * the private particle states of neighboring work-items: multi-source
 * histories receive each other's positions, including Gamma histories that
 * never enter this branch. Two explicit function boundaries (this step and
 * the Transport helper) restore correct results on every available device;
 * one boundary alone does not. This is a documented device-compiler boundary,
 * not a performance choice; the multi-source tests in tests/navigation guard
 * it.
 *
 * \param[in] world Immutable device World parameters.
 * \param[in,out] particle Private state; moved to the boundary on success.
 * \param[in,out] counters Transport counters receiving the outcome.
 * \return Zero when the history completed at the World boundary; nonzero when
 * the birth was outside the World or the query was unresolved.
 */
static __attribute__((noinline)) uint GGEMS_AioninoWorldStep(
  __global GGEMSWorldRecord const *world, GGEMSParticleState *particle,
  volatile __global GGEMSTransportCounters *counters) {
  uint world_exit_faces = 0U;
  uint outcome =
    GGEMS_TransportAioninoToWorld(world, particle, &world_exit_faces);

  if (outcome == GGEMS_WORLD_EXIT) {
    atomic_inc(&counters->escaped_world_count);
    return 0U;
  }

  if (outcome == GGEMS_WORLD_OUTSIDE) {
    atomic_inc(&counters->outside_world_count);
  } else {
    atomic_inc(&counters->unresolved_geometry_count);
  }

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
 * index. Adding projection_history_offset produces the global primary identity;
 * both additions are checked. The first nonempty matching half-open source
 * range selects the source slot and source-local index. Missing ranges and
 * failed birth initialization increment overflow_count and skip completion.
 *
 * Aionino calls World Navigation and commits its final accepted interval before
 * EscapedWorld completion. Outside births and unresolved queries increment
 * distinct failure counters without terminal publication or history completion.
 * Other species retain the checked one-meter projection and diagnostic Killed
 * completion. Energy, time and direction are unchanged in both paths.
 *
 * With observer support enabled, capture selection precedes initialization and
 * optional source/terminal records have zero deposited energy. All observer
 * arguments remain in the signature when support is disabled, but are unused.
 * The private particle state is not returned in a global particle array.
 *
 * \param[in,out] random_states Global host-initialized RNG states, with at
 * least worker_count elements.
 * \param[in,out] counters Global per-launch counters, initialized by the host
 * and atomically updated.
 * \param[in] source_records Global source records, with source_count elements.
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
 * \param[in,out] observer_counters Global atomic capture/record counters, used
 * only with observer support.
 * \param[out] observer_records Global optional record buffer, writable for
 * observer_record_capacity entries.
 * \param[in] observer_record_capacity Capacity in observer records, not bytes.
 * \param[in] run_id Run identity attached to optional observer records.
 * \param[in] worker_count Number of active workers and valid worker-owned RNG
 * states.
 * \param[in] energy_distribution_records Global source-indexed and
 * emission-indexed energy descriptors.
 * \param[in] energy_values_micro_eV Global energy table in unsigned integer
 * microelectronvolts.
 * \param[in] cumulative_ticket_upper Global exclusive cumulative ticket bounds
 * for energy tables.
 * \param[in] source_population_records Global population records parallel to
 * source_records.
 * \param[in] source_emissions Global emission descriptors referenced by
 * population records.
 * \param[in] source_emission_ranges Global source-local primary ranges parallel
 * to source_emissions.
 * \param[in] world Immutable box parameters; required for Aionino.
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
  __global GGEMSWorldRecord const *world) {

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

  while (1) {
    uint local_primary_id = atomic_add(&counters->next_primary_id, 1U);

    if (local_primary_id >= total_primary_count) {
      break;
    }

    ulong local_primary_id_u64 = (ulong)(local_primary_id);

    if (device_primary_offset >
        GGEMS_DIAGNOSTIC_UINT64_MAX - local_primary_id_u64) {
      atomic_inc(&counters->overflow_count);
      continue;
    }

    ulong projection_primary_id = device_primary_offset + local_primary_id_u64;

    if (projection_history_offset >
        GGEMS_DIAGNOSTIC_UINT64_MAX - projection_primary_id) {
      atomic_inc(&counters->overflow_count);
      continue;
    }

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

    atomic_inc(&counters->consumed_primary_count);

    __global GGEMSSourceRecord const *source =
      &source_records[selected_source_index];
    __global GGEMSSourcePopulationRecord const *population =
      &source_population_records[selected_source_index];

    GGEMSParticleState particle;
    uint initialized = GGEMS_SourceTryInitializePrimary(
      global_primary_id, source_local_primary_id, selected_source_index, source,
      population, source_emissions, source_emission_ranges,
      energy_distribution_records, energy_values_micro_eV,
      cumulative_ticket_upper, random_states, worker_id, &particle);

    if (initialized == 0U) {
      atomic_inc(&counters->overflow_count);
      continue;
    }

#if GGEMS_ENABLE_TRANSPORT_OBSERVER
    if (capture_history != 0U) {
      GGEMS_ObserverRecordParticle(
        observer_counters, observer_records, observer_record_capacity,
        GGEMS_OBSERVER_RECORD_KIND_SOURCE, run_id, global_primary_id,
        source_local_primary_id, selected_source_index, particle, 0UL);
    }
#endif

    if (particle.particle_type == GGEMS_PARTICLE_TYPE_AIONINO) {
      if (GGEMS_AioninoWorldStep(world, &particle, counters) != 0U) {
        continue;
      }
    } else {
      long displacement_x_pm = 0L;
      long displacement_y_pm = 0L;
      long displacement_z_pm = 0L;

      uint valid_projection = GGEMS_TryScaleDiagnosticProjectionComponent(
        particle.direction_x, &displacement_x_pm);

      if (valid_projection != 0U) {
        valid_projection = GGEMS_TryScaleDiagnosticProjectionComponent(
          particle.direction_y, &displacement_y_pm);
      }

      if (valid_projection != 0U) {
        valid_projection = GGEMS_TryScaleDiagnosticProjectionComponent(
          particle.direction_z, &displacement_z_pm);
      }

      long endpoint_x_pm = particle.position_x_pm;
      long endpoint_y_pm = particle.position_y_pm;
      long endpoint_z_pm = particle.position_z_pm;

      if (valid_projection != 0U) {
        valid_projection = GGEMS_TryAddDiagnosticProjectionDisplacement(
          particle.position_x_pm, displacement_x_pm, &endpoint_x_pm);
      }

      if (valid_projection != 0U) {
        valid_projection = GGEMS_TryAddDiagnosticProjectionDisplacement(
          particle.position_y_pm, displacement_y_pm, &endpoint_y_pm);
      }

      if (valid_projection != 0U) {
        valid_projection = GGEMS_TryAddDiagnosticProjectionDisplacement(
          particle.position_z_pm, displacement_z_pm, &endpoint_z_pm);
      }

      if (valid_projection != 0U) {
        particle.position_x_pm = endpoint_x_pm;
        particle.position_y_pm = endpoint_y_pm;
        particle.position_z_pm = endpoint_z_pm;
      } else {
        atomic_inc(&counters->overflow_count);
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

    atomic_inc(&counters->terminal_particle_count);
    atomic_inc(&counters->completed_history_count);
  }
}
