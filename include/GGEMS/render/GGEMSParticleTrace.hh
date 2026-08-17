#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <array>
#include <vector>

#include "GGEMS/observer/GGEMSObserverRecord.hh"
#include "GGEMS/observer/GGEMSObserverTypes.hh"
#include "GGEMS/particles/GGEMSParticleTypes.hh"

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
  std::array<float, 4U> color{};
};

struct GGEMSParticleTraceDrawRange {
  std::uint32_t source_index{core::particles::k_invalid_id_u32};
  std::size_t first_vertex{0U};
  std::size_t vertex_count{0U};
};

struct GGEMSParticleTraceDrawData {
  std::vector<GGEMSParticleTraceVertex> vertices;
  std::vector<GGEMSParticleTraceDrawRange> draw_ranges;
};

class GGEMSParticleTraceVisibility {
public:
  auto ReconcileSourceCount(std::size_t source_count) -> void;

  auto SetGlobalVisible(bool visible) noexcept -> void;
  [[nodiscard]] auto IsGlobalVisible() const noexcept -> bool;

  auto SetSourceVisible(std::size_t source_index, bool visible) -> void;
  [[nodiscard]] auto IsSourceVisible(std::uint32_t source_index) const noexcept
      -> bool;

  [[nodiscard]] auto ShouldDraw(std::uint32_t source_index) const noexcept
      -> bool;

private:
  bool global_visible_{true};
  std::vector<std::uint8_t> source_visibility_;
};

[[nodiscard]] auto ToParticleTracePointMeter(
    core::observer::GGEMSObserverRecord const &record) noexcept
    -> GGEMSParticleTracePoint;

[[nodiscard]] auto BuildParticleTraceSegments(
    std::span<core::observer::GGEMSObserverRecord const> records)
    -> std::vector<GGEMSParticleTraceSegment>;

[[nodiscard]] auto
BuildParticleTraceVertices(std::span<GGEMSParticleTraceSegment const> segments)
    -> std::vector<GGEMSParticleTraceVertex>;

[[nodiscard]] auto
BuildParticleTraceDrawData(std::span<GGEMSParticleTraceSegment const> segments)
    -> GGEMSParticleTraceDrawData;
} // namespace ggems::render
