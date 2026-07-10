#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "GGEMS/render/GGEMSParticleTrace.hh"
#include "GGEMS/render/GGEMSParticleColours.hh"

namespace ggems::render {
namespace {

constexpr float k_picometre_to_metre{1.0e-12f};

// =============================================================================
// =============================================================================

struct ObserverRecordView {
  std::uint32_t original_index{0U};
  core::observer::GGEMSObserverRecord const *record{nullptr};
};

// =============================================================================
// =============================================================================

[[nodiscard]] std::uint32_t
RecordKindSortOrder(core::observer::GGEMSObserverRecordKind kind) noexcept {
  using core::observer::GGEMSObserverRecordKind;

  switch (kind) {
  case GGEMSObserverRecordKind::Source:
    return 0U;
  case GGEMSObserverRecordKind::SecondaryStep:
    return 1U;
  case GGEMSObserverRecordKind::Step:
    return 2U;
  case GGEMSObserverRecordKind::Terminal:
    return 3U;
  case GGEMSObserverRecordKind::Anomaly:
    return 4U;
  case GGEMSObserverRecordKind::Unknown:
    return 5U;
  }

  return 5U;
}

// =============================================================================
// =============================================================================

[[nodiscard]] bool
IsTraceableRecord(core::observer::GGEMSObserverRecord const &record) noexcept {
  if (record.global_primary_id == core::particles::k_invalid_id_u64) {
    return false;
  }

  if (record.track_id == core::particles::k_invalid_id_u64) {
    return false;
  }

  core::observer::GGEMSObserverRecordKind kind =
      core::observer::FromKernelObserverRecordKind(record.record_kind);

  return kind != core::observer::GGEMSObserverRecordKind::Unknown &&
         kind != core::observer::GGEMSObserverRecordKind::Anomaly;
}

// =============================================================================
// =============================================================================

[[nodiscard]] bool
SameTrace(core::observer::GGEMSObserverRecord const &a,
          core::observer::GGEMSObserverRecord const &b) noexcept {
  return a.run_id == b.run_id && a.global_particle_id == b.global_particle_id &&
         a.track_id == b.track_id;
}

// =============================================================================
// =============================================================================

[[nodiscard]] bool
SamePosition(core::observer::GGEMSObserverRecord const &a,
             core::observer::GGEMSObserverRecord const &b) noexcept {
  return a.position_x_pm == b.position_x_pm &&
         a.position_y_pm == b.position_y_pm &&
         a.position_z_pm == b.position_z_pm;
}

// =============================================================================
// =============================================================================

[[nodiscard]] core::particles::GGEMSParticleType
GetParticleType(core::observer::GGEMSObserverRecord const &record) noexcept {
  return core::particles::FromKernelParticleType(record.particle_type);
}

// =============================================================================
// =============================================================================

[[nodiscard]] core::observer::GGEMSObserverRecordKind
GetRecordKind(core::observer::GGEMSObserverRecord const &record) noexcept {
  return core::observer::FromKernelObserverRecordKind(record.record_kind);
}

// =============================================================================
// =============================================================================

[[nodiscard]] GGEMSParticleTraceVertex
MakeTraceVertex(GGEMSParticleTracePoint const &point,
                core::particles::GGEMSParticleType particle_type) noexcept {
  RGB rgb = GetParticleRGB(particle_type);

  constexpr float inverse_255{1.0f / 255.0f};

  return GGEMSParticleTraceVertex{
      .position = {point.x_m, point.y_m, point.z_m},
      .colour = {static_cast<float>(rgb.r) * inverse_255,
                 static_cast<float>(rgb.g) * inverse_255,
                 static_cast<float>(rgb.b) * inverse_255, 1.0F}};
}

// =============================================================================
// =============================================================================

[[nodiscard]] std::vector<ObserverRecordView> BuildSortedRecordView(
    std::span<core::observer::GGEMSObserverRecord const> records) {
  std::vector<ObserverRecordView> views{};
  views.reserve(records.size());

  for (std::size_t index = 0U; index < records.size(); ++index) {
    if (!IsTraceableRecord(records[index])) {
      continue;
    }

    views.push_back(
        ObserverRecordView{static_cast<std::uint32_t>(index), &records[index]});
  }

  std::stable_sort(
      views.begin(), views.end(),
      [](ObserverRecordView const &a, ObserverRecordView const &b) {
        core::observer::GGEMSObserverRecord const &left = *a.record;
        core::observer::GGEMSObserverRecord const &right = *b.record;

        if (left.run_id != right.run_id) {
          return left.run_id < right.run_id;
        }

        if (left.global_particle_id != right.global_particle_id) {
          return left.global_primary_id < right.global_primary_id;
        }

        if (left.track_id != right.track_id) {
          return left.track_id < right.track_id;
        }

        std::uint32_t left_kind_order =
            RecordKindSortOrder(GetRecordKind(left));
        std::uint32_t right_kind_order =
            RecordKindSortOrder(GetRecordKind(right));

        if (left_kind_order != right_kind_order) {
          return left_kind_order < right_kind_order;
        }

        return a.original_index < b.original_index;
      });

  return views;
}
} // namespace

// =============================================================================
// =============================================================================

GGEMSParticleTracePoint ToParticleTracePointMetre(
    core::observer::GGEMSObserverRecord const &record) noexcept {
  return GGEMSParticleTracePoint{
      .x_m = static_cast<float>(record.position_x_pm) * k_picometre_to_metre,
      .y_m = static_cast<float>(record.position_y_pm) * k_picometre_to_metre,
      .z_m = static_cast<float>(record.position_z_pm) * k_picometre_to_metre};
}

// =============================================================================
// =============================================================================

std::vector<GGEMSParticleTraceSegment> BuildParticleTraceSegments(
    std::span<core::observer::GGEMSObserverRecord const> records) {
  std::vector<ObserverRecordView> views = BuildSortedRecordView(records);

  std::vector<GGEMSParticleTraceSegment> segments{};

  if (views.size() < 2U) {
    return segments;
  }

  segments.reserve(views.size() - 1U);

  core::observer::GGEMSObserverRecord const *previous = nullptr;

  for (ObserverRecordView const &view : views) {
    core::observer::GGEMSObserverRecord const &current = *view.record;

    if (previous == nullptr || !SameTrace(*previous, current)) {
      previous = &current;
      continue;
    }

    if (!SamePosition(*previous, current)) {
      segments.push_back(GGEMSParticleTraceSegment{
          .run_id = current.run_id,
          .global_primary_id = current.global_primary_id,
          .track_id = current.track_id,
          .parent_track_id = current.parent_track_id,
          .particle_type = GetParticleType(current),
          .begin_kind = GetRecordKind(*previous),
          .end_kind = GetRecordKind(current),
          .begin_time_ps = previous->time_ps,
          .end_time_ps = current.time_ps,
          .begin = ToParticleTracePointMetre(*previous),
          .end = ToParticleTracePointMetre(current)});
    }

    previous = &current;
  }

  return segments;
}

// =============================================================================
// =============================================================================

std::vector<GGEMSParticleTraceVertex> BuildParticleTraceVertices(
    std::span<GGEMSParticleTraceSegment const> segments) {
  std::vector<GGEMSParticleTraceVertex> vertices{};

  vertices.reserve(segments.size() * 2U);

  for (GGEMSParticleTraceSegment const &segment : segments) {
    vertices.push_back(MakeTraceVertex(segment.begin, segment.particle_type));
    vertices.push_back(MakeTraceVertex(segment.end, segment.particle_type));
  }

  return vertices;
}

} // namespace ggems::render
