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
 * \brief Defines independent per-occurrence display overrides and defaults.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>
#include <optional>
#include <cstddef>
#include <array>
#include <span>
#include <vector>

namespace ggems::render {

struct GGEMSGeometryOccurrence;
struct GGEMSGeometryScene;

/*! \brief Selects display-only visibility and topology for one occurrence. */
enum class GGEMSGeometryDisplayMode : std::uint8_t {
  /*! \brief Omits this occurrence without hiding its physical children. */
  None,

  /*! \brief Draws the edges of the displayed triangles. */
  Wireframe,

  /*! \brief Fills the displayed triangles. */
  Solid,
};

/*!
 * \brief Stores an explicit display mode separately from hierarchy defaults.
 */
struct GGEMSGeometryAppearance {
  /*! \brief Explicit mode, or no override to follow the physical hierarchy. */
  std::optional<GGEMSGeometryDisplayMode> mode_override;
};

/*!
 * \brief Resolves one occurrence's mode without inheriting its parent's choice.
 *
 * World and physical mothers default to Wireframe; physical leaves default to
 * Solid. World never resolves to Solid, including when explicitly requested.
 * \param[in] occurrence Physical identity and snapshot-built child links.
 * \param[in] appearance Explicit choices for this occurrence alone.
 * \return Effective display mode, independent of scientific visibility.
 */
[[nodiscard]] auto
ResolveGeometryDisplayMode(GGEMSGeometryOccurrence const &occurrence,
                           GGEMSGeometryAppearance const &appearance) noexcept
  -> GGEMSGeometryDisplayMode;

/*!
 * \brief Combines an occurrence mode with its voxel mapping or OBJ object mode.
 * \param[in] occurrence_mode Whole-occurrence visibility and topology.
 * \param[in] child_mode Independently retained child row mode.
 * \return None if either is hidden, otherwise the less solid of the two modes.
 */
[[nodiscard]] auto
ResolveChildDisplayMode(GGEMSGeometryDisplayMode occurrence_mode,
                        GGEMSGeometryDisplayMode child_mode) noexcept
  -> GGEMSGeometryDisplayMode;

/*!
 * \brief Applies a whole-grid mode over one independent mapping mode.
 * \param[in] occurrence_mode Whole-grid visibility and topology.
 * \param[in] mapping_mode Original mapping row's display mode.
 * \return Effective mapping mode.
 */
[[nodiscard]] auto
ResolveVoxelDisplayMode(GGEMSGeometryDisplayMode occurrence_mode,
                        GGEMSGeometryDisplayMode mapping_mode) noexcept
  -> GGEMSGeometryDisplayMode;

/*!
 * \brief Owns snapshot-local presentation edits and cached group summaries.
 *
 * Reset on every scene replacement. Methods borrow that same scene only for
 * the duration of the call; no scene pointers are retained. IDs and group or
 * mapping indices must belong to the reset scene. Only edit methods mutate
 * modes, keeping mixed-state counts coherent without per-frame member scans.
 */
class GGEMSGeometryAppearanceState {
public:
  /*!
   * \brief Clears edits and prepares defaults for a new immutable scene.
   * \param[in] scene New snapshot whose prepared memberships are used.
   */
  auto Reset(GGEMSGeometryScene const &scene) -> void;

  /*! \brief Drops all snapshot-local presentation state. */
  auto Clear() noexcept -> void;

  /*!
   * \brief Borrows per-VolumeID overrides until the next Reset or Clear.
   * \return Immutable override span suitable for the existing renderer.
   */
  [[nodiscard]] auto GetOccurrences() const noexcept
    -> std::span<GGEMSGeometryAppearance const>;

  /*!
   * \brief Resolves one physical occurrence's effective mode.
   * \param[in] scene The snapshot supplied to Reset.
   * \param[in] volume_id Physical ID in that snapshot.
   * \return Its independent effective mode.
   */
  [[nodiscard]] auto GetOccurrenceMode(GGEMSGeometryScene const &scene,
                                       std::uint32_t volume_id) const noexcept
    -> GGEMSGeometryDisplayMode;

  /*!
   * \brief Changes one physical occurrence and its cached group summaries.
   * \param[in] scene The snapshot supplied to Reset.
   * \param[in] volume_id Only physical ID to change.
   * \param[in] mode New independent mode; World remains None or Wireframe.
   */
  auto SetOccurrenceMode(GGEMSGeometryScene const &scene,
                         std::uint32_t volume_id,
                         GGEMSGeometryDisplayMode mode) noexcept -> void;

  /*!
   * \brief Returns a cached uniform group mode without scanning members.
   * \param[in] group_index Prepared scene group index.
   * \return Uniform mode, or nullopt for a mixed group.
   */
  [[nodiscard]] auto GetGroupMode(std::uint32_t group_index) const noexcept
    -> std::optional<GGEMSGeometryDisplayMode>;

  /*!
   * \brief Assigns one mode to prepared group members once, on user command.
   * \param[in] scene The snapshot supplied to Reset.
   * \param[in] group_index Prepared group defining the exact assignment scope.
   * \param[in] mode New mode, with no persistent parent filter.
   */
  auto SetGroupMode(GGEMSGeometryScene const &scene, std::uint32_t group_index,
                    GGEMSGeometryDisplayMode mode) noexcept -> void;

  /*!
   * \brief Reads a retained voxel mapping or OBJ range mode.
   * \param[in] volume_id Physical occurrence in the reset scene.
   * \param[in] index Snapshot-local mapping or mesh range index.
   * \return Display choice independent of the parent mode.
   */
  [[nodiscard]] auto GetChildMode(std::uint32_t volume_id,
                                  std::uint32_t index) const noexcept
    -> GGEMSGeometryDisplayMode;

  /*!
   * \brief Changes a child display choice in the current snapshot.
   * \param[in] volume_id Physical occurrence in the reset scene.
   * \param[in] index Snapshot-local mapping or mesh range index.
   * \param[in] mode Display override independent of the parent's mode.
   */
  auto SetChildMode(std::uint32_t volume_id, std::uint32_t index,
                    GGEMSGeometryDisplayMode mode) noexcept -> void;

  /*!
   * \brief Resolves the current mode of one exact voxel mapping row.
   * \param[in] volume_id Physical voxel occurrence in the reset scene.
   * \param[in] mapping_index Index in its immutable voxel_slices.mappings.
   * \return Retained mapping mode, independent of the whole-grid choice.
   */
  [[nodiscard]] auto
  GetVoxelMappingMode(std::uint32_t volume_id,
                      std::uint32_t mapping_index) const noexcept
    -> GGEMSGeometryDisplayMode;

  /*!
   * \brief Changes one mapping without rebuilding immutable central indices.
   * \param[in] volume_id Physical voxel occurrence in the reset scene.
   * \param[in] mapping_index Exact original mapping row, not a MaterialID.
   * \param[in] mode Transient presentation mode; color is preserved.
   */
  auto SetVoxelMappingMode(std::uint32_t volume_id, std::uint32_t mapping_index,
                           GGEMSGeometryDisplayMode mode) noexcept -> void;

private:
  /*! \brief Independent physical overrides indexed by VolumeID. */
  std::vector<GGEMSGeometryAppearance> occurrences_;

  /*! \brief None/Wireframe/Solid member counts indexed by scene group. */
  std::vector<std::array<std::size_t, 3>> group_counts_;

  /*! \brief Modes by VolumeID and voxel mapping or mesh range index. */
  std::vector<std::vector<GGEMSGeometryDisplayMode>> child_modes_;
};

} // namespace ggems::render
