// SPDX-License-Identifier: GPL-3.0-or-later
// Authors: Julien BERT <julien.bert@univ-brest.fr>
//          Didier BENOIT <didier.benoit@inserm.fr>
// E1 observations call the real dispatch and canonical Transport commit.
#include "particles/GGEMSParticleState.clh"
#include "transport/GGEMSTransport.clh"
#include "sources/GGEMSSource.clh"

// Capture the actual production angular law once, before the four navigators.
__kernel void sphere_precision_capture(__global GGEMSSourceRecord const *source,
                                       __global float4 const *uniforms,
                                       __global float4 *directions) {
  uint index = (uint)get_global_id(0);
  float3 direction = GGEMS_SourceSampleDirectionFromUniforms(
    source, (long3)(-500000000000L, 0L, 0L), uniforms[index]);
  directions[index] = (float4)(direction, 0.0f);
}

#if GGEMS_SPHERE_INTEGER
static inline GGEMSSphereIntegerInputs
sphere_precision_inputs(__global GGEMSSphereRecord const *sphere,
                        long3 position, float3 direction) {
#if GGEMS_SPHERE_INTEGER == 64
  return GGEMS_SphereINT64Preflight(sphere, position, direction);
#else
  return GGEMS_SphereINT32Preflight(sphere, position, direction);
#endif
}

static inline void
sphere_precision_quantization(GGEMSSphereIntegerInputs inputs,
                              __global long *trace) {
  trace[0] = inputs.reason;
  trace[1] = inputs.position.x;
  trace[2] = inputs.position.y;
  trace[3] = inputs.position.z;
  trace[4] = (long)inputs.radius;
  trace[5] = inputs.direction.x;
  trace[6] = inputs.direction.y;
  trace[7] = inputs.direction.z;
}
#endif

// Eight header longs, then sixteen longs per attempted transaction (at most 3).
// Header: locate status, initial owner, final navigation status, accepted
// count, attempt count, failure stage (0/locate/query/transport/budget),
// unchanged state, final particle status. Attempts: query/transport status,
// event, faces, departure, arrival, Material, parameter bits, start XYZ,
// endpoint XYZ, particle status, accepted flag. Slot 56 is the initial
// Material. A failed query has no meaningful selected event/parameter.
// Parameters describe the diagnostic pre-query; Transport repeats the query
// internally before commit. E2 appends eight initial quantization fields at57
// and thirteen fields per attempt at65: query reason, rational numerator,
// denominator, binary direction shift, spatial shift, local XYZ, radius,
// integer direction XYZ, and preflight reason. Total104 longs.
__kernel void
sphere_precision_probe(__global GGEMSWorldRecord const *world,
                       __global GGEMSAnalyticVolumeRecord const *volume,
                       __global GGEMSParticleState const *input,
                       __global long *trace) {
  if (get_global_id(0) != 0U) {
    return;
  }
  for (uint index = 0U; index < 104U; ++index) {
    trace[index] = -1L;
  }
  trace[3] = 0L;
  trace[4] = 0L;
  trace[5] = 0L;
  GGEMSParticleState particle = *input;
  float3 const direction =
    (float3)(particle.direction_x, particle.direction_y, particle.direction_z);
#if GGEMS_SPHERE_INTEGER
  sphere_precision_quantization(
    sphere_precision_inputs(&volume->parameters.sphere,
                            (long3)(particle.position_x_pm,
                                    particle.position_y_pm,
                                    particle.position_z_pm),
                            direction),
    trace + 57U);
#endif
  ulong owner = 0UL;
  uint status =
    GGEMS_AnalyticLocate(world, volume, volume,
                         (long3)(particle.position_x_pm, particle.position_y_pm,
                                 particle.position_z_pm),
                         direction, &owner);
  trace[0] = status;
  if (status != GGEMS_NAVIGATION_RESOLVED) {
    trace[5] = 1L;
  } else {
    particle.current_volume_id = (uint)(owner >> 32U);
    particle.material_id = (uint)owner;
    trace[56] = particle.material_id;
    trace[1] = particle.current_volume_id;
    uint event = GGEMS_NAVIGATION_EVENT_NONE;
    uint faces = 0U;
    for (uint index = 0U; index < 3U; ++index) {
      uint const offset = 8U + 16U * index;
#if GGEMS_SPHERE_INTEGER
      uint const extra = 65U + 13U * index;
#endif
      trace[4] = index + 1U;
      trace[offset + 4U] = particle.current_volume_id;
      trace[offset + 8U] = particle.position_x_pm;
      trace[offset + 9U] = particle.position_y_pm;
      trace[offset + 10U] = particle.position_z_pm;
      trace[offset + 15U] = 0L;
      GGEMSAnalyticBoundary selected;
#if GGEMS_SPHERE_INTEGER
      GGEMSSphereIntegerInputs inputs = sphere_precision_inputs(
        &volume->parameters.sphere,
        (long3)(particle.position_x_pm, particle.position_y_pm,
                particle.position_z_pm),
        direction);
      trace[extra + 5U] = inputs.position.x;
      trace[extra + 6U] = inputs.position.y;
      trace[extra + 7U] = inputs.position.z;
      trace[extra + 8U] = (long)inputs.radius;
      trace[extra + 9U] = inputs.direction.x;
      trace[extra + 10U] = inputs.direction.y;
      trace[extra + 11U] = inputs.direction.z;
      trace[extra + 12U] = inputs.reason;
#endif
      status = GGEMS_AnalyticFindBoundary(
        world, volume, volume,
        (long3)(particle.position_x_pm, particle.position_y_pm,
                particle.position_z_pm),
        direction, particle.current_volume_id, event, &selected);
      trace[offset] = status;
#if GGEMS_SPHERE_INTEGER
      trace[extra] = selected.sphere_integer.reason;
#endif
      if (status != GGEMS_NAVIGATION_RESOLVED) {
        trace[5] = 2L;
        break;
      }
      trace[offset + 2U] = selected.boundary.event;
      trace[offset + 3U] = selected.boundary.faces;
      trace[offset + 5U] = selected.boundary.arrival_volume_id;
      trace[offset + 6U] = selected.boundary.arrival_material_id;
#if GGEMS_SPHERE_INTEGER
      if (selected.boundary.event == GGEMS_NAVIGATION_EVENT_WORLD_EXIT) {
        trace[extra + 1U] = as_long(selected.boundary.gap_pm);
        trace[extra + 2U] = selected.boundary.mantissa;
        trace[extra + 3U] = -selected.boundary.exponent;
        trace[extra + 4U] = 0L;
      } else {
        trace[extra + 1U] = as_long(selected.sphere_integer.numerator);
        trace[extra + 2U] = as_long(selected.sphere_integer.denominator);
        trace[extra + 3U] = selected.sphere_integer.direction_shift;
        trace[extra + 4U] = selected.sphere_integer.spatial_shift;
      }
#elif GGEMS_SPHERE_FP32
      trace[offset + 7U] = (long)as_uint(selected.sphere_parameter);
#else
      trace[offset + 7U] = as_long(selected.parameter);
#endif
      status = GGEMS_TransportToNextBoundary(world, volume, volume, &particle,
                                             &event, &faces);
      trace[offset + 1U] = status;
      trace[offset + 11U] = particle.position_x_pm;
      trace[offset + 12U] = particle.position_y_pm;
      trace[offset + 13U] = particle.position_z_pm;
      trace[offset + 14U] = particle.status;
      if (status != GGEMS_NAVIGATION_RESOLVED) {
        trace[5] = 3L;
        break;
      }
      // Accepted event/owner fields come from the committing transaction.
      trace[offset + 2U] = event;
      trace[offset + 3U] = faces;
      trace[offset + 5U] = particle.current_volume_id;
      trace[offset + 6U] = particle.material_id;
      trace[offset + 15U] = 1L;
      trace[3] += 1L;
      if (event == GGEMS_NAVIGATION_EVENT_WORLD_EXIT) {
        break;
      }
      if (index == 2U) {
        status = GGEMS_NAVIGATION_UNRESOLVED;
        trace[5] = 4L;
      }
    }
  }
  trace[2] = status;
  trace[6] = as_uint(particle.direction_x) == as_uint(input->direction_x) &&
             as_uint(particle.direction_y) == as_uint(input->direction_y) &&
             as_uint(particle.direction_z) == as_uint(input->direction_z) &&
             particle.energy_micro_eV == input->energy_micro_eV &&
             particle.time_ps == input->time_ps;
  trace[7] = particle.status;
}
