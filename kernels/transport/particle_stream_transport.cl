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
#define GGEMS_ENABLE_TRANSPORT_OBSERVER 0
#endif

// =============================================================================
// =============================================================================

__kernel void particle_stream_transport(
    __global GGEMSRandomState *random_states,
    volatile __global GGEMSTransportCounters *counters,
    __global GGEMSSourceRecord const *source_records,
    __global GGEMSSourceRunRange const *source_ranges, uint source_count,
    uint total_primary_count, ulong projection_history_offset,
    ulong device_primary_offset,
    __global GGEMSObserverConfigRecord const *observer_config,
    volatile __global GGEMSObserverCounters *observer_counters,
    __global GGEMSObserverRecord *observer_records,
    uint observer_record_capacity, ulong run_id, uint worker_count,
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
        global_primary_id, source_local_primary_id, selected_source_index,
        source, population, source_emissions, source_emission_ranges,
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
