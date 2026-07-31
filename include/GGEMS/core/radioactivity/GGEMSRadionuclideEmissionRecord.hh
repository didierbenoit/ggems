#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "GGEMS/core/particles/GGEMSParticleTypes.hh"

namespace ggems::core::radioactivity {

struct GGEMSRadionuclideEmissionRecord {
  std::uint32_t particle_type{
      particles::ToKernelParticleType(particles::GGEMSParticleType::Unknown)};
  std::uint32_t energy_distribution_record_index{0U};
  std::uint64_t mono_energy_milli_eV{0ULL};
};

static_assert(std::is_standard_layout_v<GGEMSRadionuclideEmissionRecord>);
static_assert(std::is_trivially_copyable_v<GGEMSRadionuclideEmissionRecord>);
static_assert(sizeof(GGEMSRadionuclideEmissionRecord) == 16U);
static_assert(alignof(GGEMSRadionuclideEmissionRecord) == 8U);
static_assert(offsetof(GGEMSRadionuclideEmissionRecord, particle_type) == 0U);
static_assert(offsetof(GGEMSRadionuclideEmissionRecord,
                       energy_distribution_record_index) == 4U);
static_assert(offsetof(GGEMSRadionuclideEmissionRecord, mono_energy_milli_eV) ==
              8U);

} // namespace ggems::core::radioactivity
