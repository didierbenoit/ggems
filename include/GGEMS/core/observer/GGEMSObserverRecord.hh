#pragma once

#include <cstdint>
#include <type_traits>

#include "GGEMS/core/observer/GGEMSObserverTypes.hh"
#include "GGEMS/core/particles/GGEMSParticleTypes.hh"

namespace ggems::core::observer {

struct GGEMSObserverConfigRecord {
  std::uint32_t enabled{0U};
  std::uint32_t capture_first_primary_count{0U};
  std::uint32_t capture_specific_primary_enabled{0U};
  std::uint32_t reserved_0{0U};
  std::uint64_t capture_global_primary_id{particles::k_invalid_id_u64};
};

static_assert(std::is_standard_layout_v<GGEMSObserverConfigRecord>);
static_assert(std::is_trivially_copyable_v<GGEMSObserverConfigRecord>);
static_assert(sizeof(GGEMSObserverConfigRecord) == 24U);

struct GGEMSObserverCounters {
  std::uint32_t record_count{0U};
  std::uint32_t overflow_count{0U};
  std::uint32_t captured_primary_count{0U};
  std::uint32_t reserved_0{0U};
};

static_assert(std::is_standard_layout_v<GGEMSObserverCounters>);
static_assert(std::is_trivially_copyable_v<GGEMSObserverCounters>);
static_assert(sizeof(GGEMSObserverCounters) == 16U);

struct GGEMSObserverRecord {
  std::uint64_t run_id{0ULL};
  std::uint64_t global_primary_id{particles::k_invalid_id_u64};
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
  std::uint64_t energy_milli_eV{0ULL};
  float weight{1.0F};
  float reserved_0{0.0F};
};

static_assert(std::is_standard_layout_v<GGEMSObserverRecord>);
static_assert(std::is_trivially_copyable_v<GGEMSObserverRecord>);
static_assert(sizeof(GGEMSObserverRecord) == 120U);

} // namespace ggems::core::observer
