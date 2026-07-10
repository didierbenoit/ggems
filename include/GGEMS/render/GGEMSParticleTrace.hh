#pragma once

#include <cstdint>
#include <span>
#include <vector>

#include "GGEMS/core/observer/GGEMSObserverRecord.hh"
#include "GGEMS/core/observer/GGEMSObserverTypes.hh"
#include "GGEMS/core/particles/GGEMSParticleTypes.hh"

namespace ggems::render {

struct GGEMSParticleTracePoint {
  float x_m{0.0f};
  float y_m{0.0f};
  float z_m{0.0f};
};

struct GGEMSParticleTraceSegment {
  std::uint64_t run_id{0ULL};
  std::uint64_t global_primary_id{core::particles::k_invalid_id_u64};
  std::uint64_t track_id{core::particles::k_invalid_id_u64};
  std::uint64_t parent_track_id{core::particles::k_invalid_id_u64};

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
  float position[3]{};
  float colour[4]{};
};

[[nodiscard]] GGEMSParticleTracePoint ToParticleTracePointMetre(
    core::observer::GGEMSObserverRecord const &record) noexcept;

[[nodiscard]] std::vector<GGEMSParticleTraceSegment> BuildParticleTraceSegments(
    std::span<core::observer::GGEMSObserverRecord const> records);

[[nodiscard]] std::vector<GGEMSParticleTraceVertex>
BuildParticleTraceVertices(std::span<GGEMSParticleTraceSegment const> segments);
} // namespace ggems::render
