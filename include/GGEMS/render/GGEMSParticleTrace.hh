#pragma once

#include <cstdint>
#include <span>
#include <vector>
#include <array>

#include "GGEMS/core/observer/GGEMSObserverRecord.hh"
#include "GGEMS/core/observer/GGEMSObserverTypes.hh"
#include "GGEMS/core/particles/GGEMSParticleTypes.hh"

namespace ggems::render {

struct GGEMSParticleTracePoint {
  float x_m{0.0F};
  float y_m{0.0F};
  float z_m{0.0F};
};

struct GGEMSParticleTraceSegment {
  std::uint64_t run_id{0ULL};
  std::uint64_t global_primary_id{core::particles::k_invalid_id_u64};
  std::uint64_t source_local_primary_id{core::particles::k_invalid_id_u64};
  std::uint64_t track_id{core::particles::k_invalid_id_u64};
  std::uint64_t parent_track_id{core::particles::k_invalid_id_u64};

  std::uint32_t source_index{core::particles::k_invalid_id_u32};

  core::particles::GGEMSParticleType particle_type{
      core::particles::GGEMSParticleType::Unknown};

  core::observer::GGEMSObserverRecordKind begin_kind{
      core::observer::GGEMSObserverRecordKind::Unknown};
  core::observer::GGEMSObserverRecordKind end_kind{
      core::observer::GGEMSObserverRecordKind::Unknown};

  std::uint64_t begin_time_ps{0ULL};
  std::uint64_t end_time_ps{0ULL};

  GGEMSParticleTracePoint begin{};
  GGEMSParticleTracePoint end{};
};

struct GGEMSParticleTraceVertex {
  std::array<float, 3U> position{};
  std::array<float, 4U> colour{};
};

[[nodiscard]] auto ToParticleTracePointMetre(
    core::observer::GGEMSObserverRecord const &record) noexcept
    -> GGEMSParticleTracePoint;

[[nodiscard]] auto BuildParticleTraceSegments(
    std::span<core::observer::GGEMSObserverRecord const> records)
    -> std::vector<GGEMSParticleTraceSegment>;

[[nodiscard]] auto
BuildParticleTraceVertices(std::span<GGEMSParticleTraceSegment const> segments)
    -> std::vector<GGEMSParticleTraceVertex>;
} // namespace ggems::render
