#pragma once

#include <cstdint>

#include "GGEMS/observer/GGEMSObserverTypes.hh"
#include "GGEMS/particles/GGEMSParticleTypes.hh"

namespace ggems::core::observer {

struct GGEMSObserverConfigRecord {
  std::uint32_t enabled{0U};
  std::uint32_t capture_first_primary_count_per_source{0U};
  std::uint32_t capture_specific_primary_enabled{0U};
  std::uint32_t capture_source_index{particles::k_invalid_id_u32};
  std::uint64_t capture_source_local_primary_id{particles::k_invalid_id_u64};
};

struct GGEMSObserverCounters {
  std::uint32_t record_count{0U};
  std::uint32_t overflow_count{0U};
  std::uint32_t captured_primary_count{0U};
  std::uint32_t reserved_0{0U};
};

struct GGEMSObserverRecord {
  std::uint64_t run_id{0ULL};
  std::uint64_t global_primary_id{particles::k_invalid_id_u64};
  std::uint64_t source_local_primary_id{particles::k_invalid_id_u64};
  std::uint64_t global_particle_id{particles::k_invalid_id_u64};
  std::uint64_t track_id{particles::k_invalid_id_u64};
  std::uint64_t parent_track_id{particles::k_invalid_id_u64};
  std::uint64_t time_ps{0ULL};
  std::int64_t position_x_pm{0LL};
  std::int64_t position_y_pm{0LL};
  std::int64_t position_z_pm{0LL};
  std::uint32_t record_kind{
    ToKernelObserverRecordKind(GGEMSObserverRecordKind::Unknown)};
  std::uint32_t particle_type{
    particles::ToKernelParticleType(particles::GGEMSParticleType::Unknown)};
  std::uint32_t status{particles::ToKernelParticleStatus(
    particles::GGEMSParticleStatus::Inactive)};
  std::uint32_t generation{0U};
  float direction_x{0.0F};
  float direction_y{0.0F};
  float direction_z{1.0F};
  float direction_w{0.0F};
  std::uint64_t energy_micro_eV{0ULL};
  std::uint64_t deposited_energy_micro_eV{0ULL};
  std::uint32_t source_index{particles::k_invalid_id_u32};
  std::uint32_t position_valid{1U};
};

} // namespace ggems::core::observer
