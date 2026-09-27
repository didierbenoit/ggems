// *****************************************************************************
// * This file is part of GGEMS.                                               *
// *                                                                           *
// * SPDX-License-Identifier: GPL-3.0-or-later                                 *
// * Copyright (C) 2017-2026 CHRU de Brest, Université de Bretagne Occidentale,*
// * Inserm.                                                                   *
// *                                                                           *
// * GGEMS is free software: you can redistribute it and/or modify             *
// * it under the terms of the GNU General Public License as published by      *
// * the Free Software Foundation, either version 3 of the License, or         *
// * (at your option) any later version.                                       *
// *                                                                           *
// * GGEMS is distributed in the hope that it will be useful,                  *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
// * GNU General Public License for more details.                              *
// *                                                                           *
// * You should have received a copy of the GNU General Public License         *
// * along with GGEMS. If not, see <https://www.gnu.org/licenses/>.            *
// *****************************************************************************

/*!
 * \file
 * \brief Orders captured particle records and prepares diagnostic line-list
 * data.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <span>
#include <functional>

#include "GGEMS/render/GGEMSColor.hh"
#include "GGEMS/render/GGEMSParticleTrace.hh"
#include "GGEMS/render/GGEMSParticleColors.hh"
#include "GGEMS/observer/GGEMSObserverRecord.hh"
#include "GGEMS/observer/GGEMSObserverTypes.hh"
#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"
#include "GGEMS/units/GGEMSUnitConversion.hh"

namespace ggems::render {
namespace {

// =============================================================================
// =============================================================================

/*!
 * \brief Assigns capture-kind precedence for display connectivity.
 *
 * \param[in] kind Capture kind to rank.
 * \return Source=0, SecondaryStep=1, Step=2, Terminal=3, Anomaly=4, otherwise
 * 5.
 */
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

/*!
 * \brief Selects records with usable primary/track IDs and ordinary kinds.
 *
 * \param[in] record Capture to classify.
 * \return False for invalid primary or track IDs, Unknown, or Anomaly.
 */
[[nodiscard]] auto
IsTraceableRecord(core::observer::GGEMSObserverRecord const &record) noexcept
  -> bool {
  if (record.global_primary_id == core::particles::k_invalid_id_u64) {
    return false;
  }

  if (record.track_id == core::particles::k_invalid_id_u64) {
    return false;
  }

  core::observer::GGEMSObserverRecordKind const kind =
    core::observer::FromKernelObserverRecordKind(record.record_kind);

  return kind != core::observer::GGEMSObserverRecordKind::Unknown &&
         kind != core::observer::GGEMSObserverRecordKind::Anomaly;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Checks all identity fields required to connect two captures.
 *
 * \param[in] first First captured point.
 * \param[in] second Second captured point.
 * \return True for equal run, global-particle, primary, track, source, and
 * source-local IDs.
 */
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

/*!
 * \brief Compares positions before conversion to display precision.
 *
 * \param[in] first First captured point.
 * \param[in] second Second captured point.
 * \return True when all three signed picometer coordinates match exactly.
 */
[[nodiscard]] auto
SamePosition(core::observer::GGEMSObserverRecord const &first,
             core::observer::GGEMSObserverRecord const &second) noexcept
  -> bool {
  return first.position_x_pm == second.position_x_pm &&
         first.position_y_pm == second.position_y_pm &&
         first.position_z_pm == second.position_z_pm;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Decodes the captured device particle tag.
 *
 * \param[in] record Capture whose particle tag is read.
 * \return Host particle kind, or Unknown for an unsupported tag.
 */
[[nodiscard]] auto
GetParticleType(core::observer::GGEMSObserverRecord const &record) noexcept
  -> core::particles::GGEMSParticleType {
  return core::particles::FromKernelParticleType(record.particle_type);
}

// =============================================================================
// =============================================================================

/*!
 * \brief Decodes the captured device record-kind tag.
 *
 * \param[in] record Capture whose kind tag is read.
 * \return Host capture kind, or Unknown for an unsupported tag.
 */
[[nodiscard]] auto
GetRecordKind(core::observer::GGEMSObserverRecord const &record) noexcept
  -> core::observer::GGEMSObserverRecordKind {
  return core::observer::FromKernelObserverRecordKind(record.record_kind);
}

// =============================================================================
// =============================================================================

/*!
 * \brief Attaches the particle palette color to one display point.
 *
 * \param[in] point Global position in meters.
 * \param[in] particle_type Particle kind selecting the display color.
 * \return Vertex with normalized RGB channels and opaque alpha.
 */
[[nodiscard]] auto
MakeTraceVertex(GGEMSParticleTracePoint const &point,
                core::particles::GGEMSParticleType particle_type) noexcept
  -> GGEMSParticleTraceVertex {
  RGB const rgb = GetParticleRGB(particle_type);

  constexpr float inverse_255{1.0F / 255.0F};

  return GGEMSParticleTraceVertex{
    .position = {point.x_m, point.y_m, point.z_m},
    .color =
      {
        static_cast<float>(rgb.red) * inverse_255,
        static_cast<float>(rgb.green) * inverse_255,
        static_cast<float>(rgb.blue) * inverse_255,
        1.0F,
      },
  };
}

// =============================================================================
// =============================================================================

/*!
 * \brief Filters captures and stably orders pointers for trace connectivity.
 *
 * \param[in] records Captures whose storage must outlive the returned pointers.
 * \return Borrowed pointers ordered by run, global-particle ID, track, and
 * kind.
 */
[[nodiscard]] auto BuildSortedRecordView(
  std::span<core::observer::GGEMSObserverRecord const> records)
  -> std::vector<core::observer::GGEMSObserverRecord const *> {
  std::vector<core::observer::GGEMSObserverRecord const *> views{};
  views.reserve(records.size());

  for (auto const &record : records) {
    if (IsTraceableRecord(record)) {
      views.push_back(&record);
    }
  }

  std::ranges::stable_sort(
    views,
    [](core::observer::GGEMSObserverRecord const *first,
       core::observer::GGEMSObserverRecord const *second) -> bool {
      core::observer::GGEMSObserverRecord const &left = *first;
      core::observer::GGEMSObserverRecord const &right = *second;

      if (left.run_id != right.run_id) {
        return left.run_id < right.run_id;
      }

      if (left.global_particle_id != right.global_particle_id) {
        return left.global_particle_id < right.global_particle_id;
      }

      if (left.track_id != right.track_id) {
        return left.track_id < right.track_id;
      }

      std::uint32_t const left_kind_order =
        RecordKindSortOrder(GetRecordKind(left));
      std::uint32_t const right_kind_order =
        RecordKindSortOrder(GetRecordKind(right));

      if (left_kind_order != right_kind_order) {
        return left_kind_order < right_kind_order;
      }

      return false;
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
    .x_m = static_cast<float>(
      *units::ConvertTo(units::PositionCoordinate{record.position_x_pm}, "m")),
    .y_m = static_cast<float>(
      *units::ConvertTo(units::PositionCoordinate{record.position_y_pm}, "m")),
    .z_m = static_cast<float>(
      *units::ConvertTo(units::PositionCoordinate{record.position_z_pm}, "m")),
  };
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
  auto const views = BuildSortedRecordView(records);

  std::vector<GGEMSParticleTraceSegment> segments{};

  if (views.size() < 2U) {
    return segments;
  }

  segments.reserve(views.size() - 1U);

  core::observer::GGEMSObserverRecord const *previous = nullptr;

  for (auto const *record : views) {
    core::observer::GGEMSObserverRecord const &current = *record;

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
        .end = ToParticleTracePointMeter(current),
      });
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

  for (GGEMSParticleTraceSegment const *segment : grouped_segments) {
    if (draw_data.draw_ranges.empty() ||
        draw_data.draw_ranges.back().source_index != segment->source_index) {
      draw_data.draw_ranges.push_back(GGEMSParticleTraceDrawRange{
        .source_index = segment->source_index,
        .first_vertex = draw_data.vertices.size(),
        .vertex_count = 0U,
      });
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
