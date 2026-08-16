#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <span>
#include <functional>

#include "GGEMS/render/GGEMSColor.hh"
#include "GGEMS/render/GGEMSParticleTrace.hh"
#include "GGEMS/render/GGEMSParticleColors.hh"
#include "GGEMS/core/observer/GGEMSObserverRecord.hh"
#include "GGEMS/core/observer/GGEMSObserverTypes.hh"
#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/units/GGEMSLengthUnits.hh"
#include "GGEMS/core/units/GGEMSUnitConversion.hh"

namespace ggems::render {
namespace {

// =============================================================================
// =============================================================================

struct ObserverRecordView {
  std::uint32_t original_index{0U};
  core::observer::GGEMSObserverRecord const *record{nullptr};
};

// =============================================================================
// =============================================================================

[[nodiscard]] auto
RecordKindSortOrder(core::observer::GGEMSObserverRecordKind kind) noexcept
    -> std::uint32_t {
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

[[nodiscard]] auto
IsTraceableRecord(core::observer::GGEMSObserverRecord const &record) noexcept
    -> bool {
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

[[nodiscard]] auto
SameTrace(core::observer::GGEMSObserverRecord const &first,
          core::observer::GGEMSObserverRecord const &second) noexcept -> bool {
  return first.run_id == second.run_id &&
         first.global_particle_id == second.global_particle_id &&
         first.global_primary_id == second.global_primary_id &&
         first.track_id == second.track_id &&
         first.source_index == second.source_index &&
         first.source_local_primary_id == second.source_local_primary_id;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto
SamePosition(core::observer::GGEMSObserverRecord const &first,
             core::observer::GGEMSObserverRecord const &second) noexcept
    -> auto {
  return first.position_x_pm == second.position_x_pm &&
         first.position_y_pm == second.position_y_pm &&
         first.position_z_pm == second.position_z_pm;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto
GetParticleType(core::observer::GGEMSObserverRecord const &record) noexcept
    -> core::particles::GGEMSParticleType {
  return core::particles::FromKernelParticleType(record.particle_type);
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto
GetRecordKind(core::observer::GGEMSObserverRecord const &record) noexcept
    -> core::observer::GGEMSObserverRecordKind {
  return core::observer::FromKernelObserverRecordKind(record.record_kind);
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto
MakeTraceVertex(GGEMSParticleTracePoint const &point,
                core::particles::GGEMSParticleType particle_type) noexcept
    -> GGEMSParticleTraceVertex {
  RGB rgb = GetParticleRGB(particle_type);

  constexpr float inverse_255{1.0F / 255.0F};

  return GGEMSParticleTraceVertex{
      .position = {point.x_m, point.y_m, point.z_m},
      .color = {static_cast<float>(rgb.red) * inverse_255,
                static_cast<float>(rgb.green) * inverse_255,
                static_cast<float>(rgb.blue) * inverse_255, 1.0F}};
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto BuildSortedRecordView(
    std::span<core::observer::GGEMSObserverRecord const> records)
    -> std::vector<ObserverRecordView> {
  std::vector<ObserverRecordView> views{};
  views.reserve(records.size());

  for (std::size_t index = 0U; index < records.size(); ++index) {
    if (!IsTraceableRecord(records[index])) {
      continue;
    }

    views.push_back(
        ObserverRecordView{.original_index = static_cast<std::uint32_t>(index),
                           .record = &records[index]});
  }

  std::ranges::stable_sort(
      views,
      [](ObserverRecordView const &first,
         ObserverRecordView const &second) -> bool {
        core::observer::GGEMSObserverRecord const &left = *first.record;
        core::observer::GGEMSObserverRecord const &right = *second.record;

        if (left.run_id != right.run_id) {
          return left.run_id < right.run_id;
        }

        if (left.global_particle_id != right.global_particle_id) {
          return left.global_particle_id < right.global_particle_id;
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

        return first.original_index < second.original_index;
      });

  return views;
}
} // namespace

// =============================================================================
// =============================================================================

auto ToParticleTracePointMeter(
    core::observer::GGEMSObserverRecord const &record) noexcept
    -> GGEMSParticleTracePoint {
  return GGEMSParticleTracePoint{
      .x_m = static_cast<float>(*units::ConvertTo(
          units::PositionCoordinate{record.position_x_pm}, "m")),
      .y_m = static_cast<float>(*units::ConvertTo(
          units::PositionCoordinate{record.position_y_pm}, "m")),
      .z_m = static_cast<float>(*units::ConvertTo(
          units::PositionCoordinate{record.position_z_pm}, "m"))};
}

// =============================================================================
// =============================================================================

auto GGEMSParticleTraceVisibility::ReconcileSourceCount(
    std::size_t source_count) -> void {
  source_visibility_.resize(source_count, std::uint8_t{1U});
}

// -----------------------------------------------------------------------------

auto GGEMSParticleTraceVisibility::SetGlobalVisible(bool visible) noexcept
    -> void {
  global_visible_ = visible;
}

// -----------------------------------------------------------------------------

auto GGEMSParticleTraceVisibility::IsGlobalVisible() const noexcept -> bool {
  return global_visible_;
}

// -----------------------------------------------------------------------------

auto GGEMSParticleTraceVisibility::SetSourceVisible(std::size_t source_index,
                                                    bool visible) -> void {
  source_visibility_.at(source_index) =
      visible ? std::uint8_t{1U} : std::uint8_t{0U};
}

// -----------------------------------------------------------------------------

auto GGEMSParticleTraceVisibility::IsSourceVisible(
    std::uint32_t source_index) const noexcept -> bool {
  auto const index = static_cast<std::size_t>(source_index);

  return index >= source_visibility_.size() ||
         source_visibility_[index] != std::uint8_t{0U};
}

// -----------------------------------------------------------------------------

auto GGEMSParticleTraceVisibility::ShouldDraw(
    std::uint32_t source_index) const noexcept -> bool {
  return global_visible_ && IsSourceVisible(source_index);
}

// =============================================================================
// =============================================================================

auto BuildParticleTraceSegments(
    std::span<core::observer::GGEMSObserverRecord const> records)
    -> std::vector<GGEMSParticleTraceSegment> {
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
          .source_local_primary_id = current.source_local_primary_id,
          .track_id = current.track_id,
          .parent_track_id = current.parent_track_id,
          .source_index = current.source_index,
          .particle_type = GetParticleType(current),
          .begin_kind = GetRecordKind(*previous),
          .end_kind = GetRecordKind(current),
          .begin_time_ps = previous->time_ps,
          .end_time_ps = current.time_ps,
          .begin = ToParticleTracePointMeter(*previous),
          .end = ToParticleTracePointMeter(current)});
    }

    previous = &current;
  }

  return segments;
}

// =============================================================================
// =============================================================================

auto BuildParticleTraceVertices(
    std::span<GGEMSParticleTraceSegment const> segments)
    -> std::vector<GGEMSParticleTraceVertex> {
  std::vector<GGEMSParticleTraceVertex> vertices{};

  vertices.reserve(segments.size() * 2U);

  for (GGEMSParticleTraceSegment const &segment : segments) {
    vertices.push_back(MakeTraceVertex(segment.begin, segment.particle_type));
    vertices.push_back(MakeTraceVertex(segment.end, segment.particle_type));
  }

  return vertices;
}

// =============================================================================
// =============================================================================

auto BuildParticleTraceDrawData(
    std::span<GGEMSParticleTraceSegment const> segments)
    -> GGEMSParticleTraceDrawData {
  std::vector<GGEMSParticleTraceSegment const *> grouped_segments{};
  grouped_segments.reserve(segments.size());

  for (GGEMSParticleTraceSegment const &segment : segments) {
    grouped_segments.push_back(&segment);
  }

  std::ranges::stable_sort(grouped_segments, std::ranges::less{},
                           &GGEMSParticleTraceSegment::source_index);

  GGEMSParticleTraceDrawData draw_data{};
  draw_data.vertices.reserve(segments.size() * 2U);
  draw_data.draw_ranges.reserve(segments.size());

  for (GGEMSParticleTraceSegment const *segment : grouped_segments) {
    if (draw_data.draw_ranges.empty() ||
        draw_data.draw_ranges.back().source_index != segment->source_index) {
      draw_data.draw_ranges.push_back(
          GGEMSParticleTraceDrawRange{.source_index = segment->source_index,
                                      .first_vertex = draw_data.vertices.size(),
                                      .vertex_count = 0U});
    }

    draw_data.vertices.push_back(
        MakeTraceVertex(segment->begin, segment->particle_type));
    draw_data.vertices.push_back(
        MakeTraceVertex(segment->end, segment->particle_type));

    draw_data.draw_ranges.back().vertex_count += 2U;
  }

  return draw_data;
}
} // namespace ggems::render
