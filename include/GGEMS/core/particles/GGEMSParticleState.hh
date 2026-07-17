#pragma once

#include <cstdint>
#include <type_traits>

#include "GGEMS/core/particles/GGEMSParticleTypes.hh"

namespace ggems::core::particles {

struct GGEMSParticleState {
  std::uint64_t global_particle_id{k_invalid_id_u64};
  std::uint64_t track_id{k_invalid_id_u64};
  std::uint64_t parent_track_id{k_invalid_id_u64};
  std::uint64_t time_ps{0LL};
  std::int64_t position_x_pm{0LL};
  std::int64_t position_y_pm{0LL};
  std::int64_t position_z_pm{0LL};
  std::uint32_t particle_type{ToKernelParticleType(GGEMSParticleType::Unknown)};
  std::uint32_t status{ToKernelParticleStatus(GGEMSParticleStatus::Inactive)};
  std::uint32_t generation{0U};
  std::uint32_t flags{0U};
  std::uint32_t current_navigator_id{k_invalid_id_u32};
  std::uint32_t current_volume_id{k_invalid_id_u32};
  std::uint32_t material_id{k_invalid_id_u32};
  std::uint32_t region_id{k_invalid_id_u32};
  float direction_x{0.0f};
  float direction_y{0.0f};
  float direction_z{1.0f};
  float direction_w{0.0f};
  std::uint64_t energy_milli_eV{0ULL};
  float weight{1.0f};
};

static_assert(std::is_standard_layout_v<GGEMSParticleState>);
static_assert(std::is_trivially_copyable_v<GGEMSParticleState>);
static_assert(sizeof(GGEMSParticleState) == 120U);

} // namespace ggems::core::particles
