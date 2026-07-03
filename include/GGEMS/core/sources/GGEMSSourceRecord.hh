#pragma once

#include <cstdint>
#include <type_traits>

#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/sources/GGEMSSourceTypes.hh"

namespace ggems::core::sources {

struct GGEMSSourceRecord {
  std::uint64_t source_id{0ULL};
  std::uint64_t time_start_ps{0ULL};
  std::uint64_t time_stop_ps{0ULL};
  std::uint64_t energy_milli_eV{511'000'000ULL};

  std::int64_t position_x_pm{0ULL};
  std::int64_t position_y_pm{0ULL};
  std::int64_t position_z_pm{0ULL};

  std::uint32_t source_type{ToKernelSourceType(GGEMSSourceType::Analytic)};
  std::uint32_t emitted_particle_type{
      particles::ToKernelParticleType(particles::GGEMSParticleType::Gamma)};
  std::uint32_t flags{0U};
  std::uint32_t reserved_0{0U};

  float direction_x{0.0f};
  float direction_y{0.0f};
  float direction_z{1.0f};
  float direction_w{0.0f};

  float weight{1.0f};
  float reserved_1{0.0f};
  float reserved_2{0.0f};
  float reserved_3{0.0f};
};

static_assert(std::is_standard_layout_v<GGEMSSourceRecord>);
static_assert(std::is_trivially_copyable_v<GGEMSSourceRecord>);
static_assert(sizeof(GGEMSSourceRecord) == 104U);

} // namespace ggems::core::sources
