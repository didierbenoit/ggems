#pragma once

#include <cstdint>
#include <type_traits>
#include <cstddef>

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

static_assert(std::is_standard_layout_v<GGEMSObserverConfigRecord>);
static_assert(std::is_trivially_copyable_v<GGEMSObserverConfigRecord>);
static_assert(sizeof(GGEMSObserverConfigRecord) == 24U);
static_assert(alignof(GGEMSObserverConfigRecord) == 8U);
static_assert(offsetof(GGEMSObserverConfigRecord, enabled) == 0U);
static_assert(offsetof(GGEMSObserverConfigRecord,
                       capture_first_primary_count_per_source) == 4U);
static_assert(offsetof(GGEMSObserverConfigRecord,
                       capture_specific_primary_enabled) == 8U);
static_assert(offsetof(GGEMSObserverConfigRecord, capture_source_index) == 12U);
static_assert(offsetof(GGEMSObserverConfigRecord,
                       capture_source_local_primary_id) == 16U);

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
  float weight{1.0F};
  std::uint32_t source_index{particles::k_invalid_id_u32};
};

static_assert(std::is_standard_layout_v<GGEMSObserverRecord>);
static_assert(std::is_trivially_copyable_v<GGEMSObserverRecord>);
static_assert(sizeof(GGEMSObserverRecord) == 136U);
static_assert(alignof(GGEMSObserverRecord) == 8U);

static_assert(offsetof(GGEMSObserverRecord, run_id) == 0U);
static_assert(offsetof(GGEMSObserverRecord, global_primary_id) == 8U);
static_assert(offsetof(GGEMSObserverRecord, source_local_primary_id) == 16U);
static_assert(offsetof(GGEMSObserverRecord, global_particle_id) == 24U);
static_assert(offsetof(GGEMSObserverRecord, track_id) == 32U);
static_assert(offsetof(GGEMSObserverRecord, parent_track_id) == 40U);
static_assert(offsetof(GGEMSObserverRecord, time_ps) == 48U);
static_assert(offsetof(GGEMSObserverRecord, position_x_pm) == 56U);
static_assert(offsetof(GGEMSObserverRecord, position_y_pm) == 64U);
static_assert(offsetof(GGEMSObserverRecord, position_z_pm) == 72U);
static_assert(offsetof(GGEMSObserverRecord, record_kind) == 80U);
static_assert(offsetof(GGEMSObserverRecord, particle_type) == 84U);
static_assert(offsetof(GGEMSObserverRecord, status) == 88U);
static_assert(offsetof(GGEMSObserverRecord, generation) == 92U);
static_assert(offsetof(GGEMSObserverRecord, direction_x) == 96U);
static_assert(offsetof(GGEMSObserverRecord, direction_y) == 100U);
static_assert(offsetof(GGEMSObserverRecord, direction_z) == 104U);
static_assert(offsetof(GGEMSObserverRecord, direction_w) == 108U);
static_assert(offsetof(GGEMSObserverRecord, energy_micro_eV) == 112U);
static_assert(offsetof(GGEMSObserverRecord, deposited_energy_micro_eV) == 120U);
static_assert(offsetof(GGEMSObserverRecord, weight) == 128U);
static_assert(offsetof(GGEMSObserverRecord, source_index) == 132U);

} // namespace ggems::core::observer
