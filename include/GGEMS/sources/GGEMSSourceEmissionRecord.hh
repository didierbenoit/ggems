#pragma once

#include <cstdint>

#include "GGEMS/particles/GGEMSParticleTypes.hh"

namespace ggems::core::sources {

struct GGEMSSourceEmissionRecord {
  std::uint32_t particle_type{
    particles::ToKernelParticleType(particles::GGEMSParticleType::Unknown)};
  std::uint32_t energy_distribution_record_index{0U};
  std::uint64_t mono_energy_micro_eV{0ULL};
};

} // namespace ggems::core::sources
