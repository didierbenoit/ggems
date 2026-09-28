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
 * This kernel initializes a private particle, projects its direction by the
 * nominal one-meter diagnostic scale with checked integer arithmetic, and marks
 * it killed. It does not perform physical transport, navigation, interactions,
 * time-of-flight updates, or secondary production. Only RNG states, transport
 * counters, and optional observer records are written to global memory.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include "observer/GGEMSObserverRecord.clh"
#include "particles/GGEMSParticleState.clh"
#include "random/GGEMSRandom.clh"
#include "transport/GGEMSDiagnosticProjection.clh"
#include "transport/GGEMSTransportCounters.clh"
#include "sources/GGEMSSourcePopulationRecord.clh"
#include "sources/GGEMSSourceEmissionRecord.clh"
#include "sources/GGEMSSourceEmissionRange.clh"
#include "sources/GGEMSEnergyDistribution.clh"
#include "sources/GGEMSSource.clh"
#include "sources/GGEMSSourceRunRange.clh"

#ifndef GGEMS_ENABLE_TRANSPORT_OBSERVER

/*! \brief Enables optional source and terminal observer writes when nonzero. */
#define GGEMS_ENABLE_TRANSPORT_OBSERVER 0
#endif

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
 * After successful birth, all three direction components are scaled and all
 * three endpoint additions must succeed before any particle coordinate changes.
 * A failure retains the entire birth position and increments overflow_count. In
 * either case the particle is marked killed, its energy and time remain
 * unchanged, and terminal_particle_count and completed_history_count are
 * incremented. Thus completed_history_count alone does not certify a valid
 * projection. consumed_primary_count is incremented before birth initialization
 * and can exceed completed_history_count.
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
  __global GGEMSSourceEmissionRange const *source_emission_ranges) {

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
