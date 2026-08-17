#include "particles/GGEMSParticleState.clh"

__kernel void particle_dummy_kill_alive(__global GGEMSParticleState *particles,
                                        __global uint *active_count,
                                        uint particle_count) {
  uint particle_index = get_global_id(0);

  if (particle_index == 0U) {
    active_count[0] = 0U;
  }

  if (particle_index >= particle_count) {
    return;
  }

  GGEMSParticleState particle = particles[particle_index];

  if (particle.status == GGEMS_PARTICLE_STATUS_ALIVE) {
    particle.status = GGEMS_PARTICLE_STATUS_KILLED;
    particle.time_ps += 1ULL;
    particle.flags |= 1U;
  }

  particles[particle_index] = particle;
}
