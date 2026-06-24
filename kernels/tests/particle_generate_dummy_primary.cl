#include "core/particles/GGEMSParticleState.clh"

__kernel void particle_generate_dummy_primary(
    __global GGEMSParticleState *particles, ulong global_particle_offset,
    uint particle_count, uint particle_type, ulong energy_milli_eV) {
  uint particle_index = get_global_id(0);

  if (particle_index >= particle_count) {
    return;
  }

  ulong global_particle_id = global_particle_offset + (ulong)(particle_index);

  GGEMSParticleState particle;

  particle.global_particle_id = global_particle_id;
  particle.track_id = global_particle_id;
  particle.parent_track_id = GGEMS_INVALID_ID_U64;
  particle.time_ps = 0ULL;

  particle.position_x_pm = 0L;
  particle.position_y_pm = 0L;
  particle.position_z_pm = 0L;

  particle.particle_type = particle_type;
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

  particle.energy_milli_eV = energy_milli_eV;
  particle.weight = 1.0f;

  particles[particle_index] = particle;
}
