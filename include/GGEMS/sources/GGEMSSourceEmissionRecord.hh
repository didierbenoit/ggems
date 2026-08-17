#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "GGEMS/particles/GGEMSParticleTypes.hh"

namespace ggems::core::sources {

struct GGEMSSourceEmissionRecord {
  std::uint32_t particle_type{
      particles::ToKernelParticleType(particles::GGEMSParticleType::Unknown)};
  std::uint32_t energy_distribution_record_index{0U};
  std::uint64_t mono_energy_milli_eV{0ULL};
};

static_assert(std::is_standard_layout_v<GGEMSSourceEmissionRecord>);
static_assert(std::is_trivially_copyable_v<GGEMSSourceEmissionRecord>);
static_assert(sizeof(GGEMSSourceEmissionRecord) == 16U);
static_assert(alignof(GGEMSSourceEmissionRecord) == 8U);
static_assert(offsetof(GGEMSSourceEmissionRecord, particle_type) == 0U);
static_assert(offsetof(GGEMSSourceEmissionRecord,
                       energy_distribution_record_index) == 4U);
static_assert(offsetof(GGEMSSourceEmissionRecord, mono_energy_milli_eV) == 8U);

} // namespace ggems::core::sources
