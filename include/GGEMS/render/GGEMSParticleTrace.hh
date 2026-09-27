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
 * \brief Converts captured observer positions into particle-colored diagnostic
 * lines.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

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

/*! \brief Stores a binary32 global position for visualization, in meters. */
struct GGEMSParticleTracePoint {
  /*! \brief Global x coordinate in meters. */
  float x_m{0.0F};

  /*! \brief Global y coordinate in meters. */
  float y_m{0.0F};

  /*! \brief Global z coordinate in meters. */
  float z_m{0.0F};
};

/*!
 * \brief Joins two distinct captured positions belonging to one particle trace.
 *
 * Identifiers and endpoint times preserve observer metadata. These diagnostic
 * segments do not reconstruct unobserved interactions or physical trajectories.
 */
struct GGEMSParticleTraceSegment {
  /*! \brief Execution run identifier shared by both captured endpoints. */
  std::uint64_t run_id{0ULL};

  /*! \brief Global root-primary identifier for this trace. */
  std::uint64_t global_primary_id{core::particles::k_invalid_id_u64};

  /*! \brief Primary identifier relative to its source population. */
  std::uint64_t source_local_primary_id{core::particles::k_invalid_id_u64};

  /*! \brief Track identifier within the primary history. */
  std::uint64_t track_id{core::particles::k_invalid_id_u64};

  /*! \brief Parent track identifier copied from the ending record. */
  std::uint64_t parent_track_id{core::particles::k_invalid_id_u64};

  /*! \brief Source slot index used to group and filter displayed lines. */
  std::uint32_t source_index{core::particles::k_invalid_id_u32};

  /*! \brief Particle kind copied from the ending record. */
  core::particles::GGEMSParticleType particle_type{
    core::particles::GGEMSParticleType::Unknown};

  /*! \brief Capture kind at the beginning of the segment. */
  core::observer::GGEMSObserverRecordKind begin_kind{
    core::observer::GGEMSObserverRecordKind::Unknown};

  /*! \brief Capture kind at the end of the segment. */
  core::observer::GGEMSObserverRecordKind end_kind{
    core::observer::GGEMSObserverRecordKind::Unknown};

  /*! \brief Captured beginning time in unsigned picoseconds. */
  std::uint64_t begin_time_ps{0ULL};

  /*! \brief Captured ending time in unsigned picoseconds. */
  std::uint64_t end_time_ps{0ULL};

  /*! \brief Beginning position in global meters. */
  GGEMSParticleTracePoint begin{};

  /*! \brief Ending position in global meters. */
  GGEMSParticleTracePoint end{};
};

/*! \brief Stores one line-list endpoint and its normalized RGBA color. */
struct GGEMSParticleTraceVertex {
  /*! \brief Global x, y, and z coordinates in meters. */
  std::array<float, 3U> position{};

  /*! \brief Normalized red, green, blue, and alpha channels. */
  std::array<float, 4U> color{};
};

/*! \brief Selects consecutive vertices belonging to one source slot. */
struct GGEMSParticleTraceDrawRange {
  /*! \brief Source slot index used to group and filter displayed lines. */
  std::uint32_t source_index{core::particles::k_invalid_id_u32};

  /*! \brief Zero-based starting element in the owned vertex array. */
  std::size_t first_vertex{0U};

  /*! \brief Number of consecutive vertices; two per segment. */
  std::size_t vertex_count{0U};
};

/*! \brief Owns source-grouped line-list vertices and their drawing ranges. */
struct GGEMSParticleTraceDrawData {
  /*! \brief Owned line-list vertices, grouped by source slot. */
  std::vector<GGEMSParticleTraceVertex> vertices;

  /*! \brief Source ranges referring into vertices by element index. */
  std::vector<GGEMSParticleTraceDrawRange> draw_ranges;
};

/*!
 * \brief Combines a global display switch with remembered per-source switches.
 */
class GGEMSParticleTraceVisibility {
public:
  /*!
   * \brief Resizes remembered source switches to match the current source list.
   *
   * Existing retained slots keep their state; new slots are visible. Shrinking
   * discards trailing switches.
   *
   * \param[in] source_count Number of source slots.
   */
  auto ReconcileSourceCount(std::size_t source_count) -> void;

  /*!
   * \brief Sets the global particle-trace display switch.
   *
   * \param[in] visible Whether traces may be displayed.
   */
  auto SetGlobalVisible(bool visible) noexcept -> void;

  /*!
   * \brief Reads the global display switch.
   *
   * \return True when the global display gate is enabled.
   */
  [[nodiscard]] auto IsGlobalVisible() const noexcept -> bool;

  /*!
   * \brief Sets the display switch of an existing source slot.
   *
   * \param[in] source_index Index previously established by
   * ReconcileSourceCount.
   * \param[in] visible Whether this source may be displayed.
   * \throws std::out_of_range If source_index is outside the remembered slots.
   */
  auto SetSourceVisible(std::size_t source_index, bool visible) -> void;

  /*!
   * \brief Reads a source switch, treating unregistered slots as visible.
   *
   * \param[in] source_index Source slot to inspect.
   * \return Stored visibility, or true for an index beyond the switch array.
   */
  [[nodiscard]] auto IsSourceVisible(std::uint32_t source_index) const noexcept
    -> bool;

  /*!
   * \brief Combines global and per-source visibility.
   *
   * \param[in] source_index Source slot to inspect.
   * \return True only when both display switches permit drawing.
   */
  [[nodiscard]] auto ShouldDraw(std::uint32_t source_index) const noexcept
    -> bool;

private:
  /*! \brief Global display gate, initially enabled. */
  bool global_visible_{true};

  /*! \brief Per-source switches: zero hides and nonzero shows. */
  std::vector<std::uint8_t> source_visibility_;
};

/*!
 * \brief Converts captured canonical coordinates to display meters.
 *
 * \param[in] record Observer record containing signed picometer coordinates.
 * \return Binary32 global point; conversion may lose canonical precision.
 */
[[nodiscard]] auto ToParticleTracePointMeter(
  core::observer::GGEMSObserverRecord const &record) noexcept
  -> GGEMSParticleTracePoint;

/*!
 * \brief Connects adjacent traceable records with distinct canonical positions.
 *
 * Filters invalid primary/track IDs and Unknown/Anomaly kinds. A stable sort
 * orders by run ID, global particle ID, track ID, then capture kind (Source,
 * SecondaryStep, Step, Terminal). Matching also requires equal global-primary,
 * source-slot, and source-local-primary IDs. Time is retained as metadata, not
 * used as the sort key.
 *
 * \param[in] records Borrowed captures; input order breaks equal sort-key ties.
 * \return Owned segments; empty when no distinct adjacent trace points remain.
 */
[[nodiscard]] auto BuildParticleTraceSegments(
  std::span<core::observer::GGEMSObserverRecord const> records)
  -> std::vector<GGEMSParticleTraceSegment>;

/*!
 * \brief Expands segments into a particle-colored line list.
 *
 * \param[in] segments Segments in desired drawing order.
 * \return Two vertices per segment, preserving input order; alpha is one.
 */
[[nodiscard]] auto
BuildParticleTraceVertices(std::span<GGEMSParticleTraceSegment const> segments)
  -> std::vector<GGEMSParticleTraceVertex>;

/*!
 * \brief Groups line-list segments by source slot for selective drawing.
 *
 * Stable grouping preserves relative segment order within each source.
 *
 * \param[in] segments Borrowed diagnostic segments.
 * \return Owned vertices and ranges ordered by ascending source index.
 */
[[nodiscard]] auto
BuildParticleTraceDrawData(std::span<GGEMSParticleTraceSegment const> segments)
  -> GGEMSParticleTraceDrawData;
} // namespace ggems::render
