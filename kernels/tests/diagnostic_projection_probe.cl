#include "core/transport/GGEMSDiagnosticProjection.clh"

__kernel void diagnostic_projection_probe(
    __global float const *components, __global long *scaled_components,
    __global uint *scale_success, uint component_count,
    __global long const *positions, __global long const *displacements,
    __global long *endpoints, __global uint *addition_success,
    uint addition_count, __global ulong *distance_pm) {
  uint index = (uint)(get_global_id(0));

  if (index == 0U) {
    distance_pm[0] = GGEMS_DIAGNOSTIC_PROJECTION_DISTANCE_PM;
  }

  if (index < component_count) {
    long scaled = scaled_components[index];

    uint success =
        GGEMS_TryScaleDiagnosticProjectionComponent(components[index], &scaled);

    scaled_components[index] = scaled;
    scale_success[index] = success;
  }

  if (index < addition_count) {
    long endpoint = endpoints[index];

    uint success = GGEMS_TryAddDiagnosticProjectionDisplacement(
        positions[index], displacements[index], &endpoint);

    endpoints[index] = endpoint;
    addition_success[index] = success;
  }
}
