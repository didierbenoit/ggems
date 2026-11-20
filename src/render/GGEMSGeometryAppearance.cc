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
 * \brief Resolves independent physical hierarchy display modes.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

#include "GGEMS/render/GGEMSGeometryAppearance.hh"
#include "GGEMS/render/GGEMSGeometryScene.hh"

namespace ggems::render {

// =============================================================================
// =============================================================================

auto ResolveGeometryDisplayMode(
  GGEMSGeometryOccurrence const &occurrence,
  GGEMSGeometryAppearance const &appearance) noexcept
  -> GGEMSGeometryDisplayMode {
  if (occurrence.volume_id == 0U) {
    return appearance.mode_override == GGEMSGeometryDisplayMode::None
             ? GGEMSGeometryDisplayMode::None
             : GGEMSGeometryDisplayMode::Wireframe;
  }

  return appearance.mode_override.value_or(
    occurrence.children.empty() ? GGEMSGeometryDisplayMode::Solid
                                : GGEMSGeometryDisplayMode::Wireframe);
}

// =============================================================================
// =============================================================================

auto ResolveChildDisplayMode(GGEMSGeometryDisplayMode occurrence_mode,
                             GGEMSGeometryDisplayMode child_mode) noexcept
  -> GGEMSGeometryDisplayMode {
  if (occurrence_mode == GGEMSGeometryDisplayMode::None ||
      child_mode == GGEMSGeometryDisplayMode::None) {
    return GGEMSGeometryDisplayMode::None;
  }

  return occurrence_mode == GGEMSGeometryDisplayMode::Wireframe
           ? GGEMSGeometryDisplayMode::Wireframe
           : child_mode;
}

// =============================================================================
// =============================================================================

auto ResolveVoxelDisplayMode(GGEMSGeometryDisplayMode occurrence_mode,
                             GGEMSGeometryDisplayMode mapping_mode) noexcept
  -> GGEMSGeometryDisplayMode {
  return ResolveChildDisplayMode(occurrence_mode, mapping_mode);
}

// -----------------------------------------------------------------------------

auto GGEMSGeometryAppearanceState::Reset(GGEMSGeometryScene const &scene)
  -> void {
  occurrences_.assign(scene.occurrences.size(), {});
  group_counts_.assign(scene.groups.size(), {});

  child_modes_.assign(scene.occurrences.size(), {});
  for (auto const &occurrence : scene.occurrences) {
    auto const mode = GetOccurrenceMode(scene, occurrence.volume_id);

    for (auto const group : occurrence.controlling_groups) {
      ++group_counts_[group][static_cast<std::size_t>(mode)];
    }

    auto &modes = child_modes_[occurrence.volume_id];
    if (occurrence.voxel_slices) {
      modes.reserve(occurrence.voxel_slices->mappings.size());
      for (auto const &mapping : occurrence.voxel_slices->mappings) {
        modes.push_back(mapping.mode);
      }
    } else if (occurrence.indexed_mesh &&
               occurrence.indexed_mesh->has_objects) {
      modes.assign(occurrence.indexed_mesh->ranges.size(),
                   GGEMSGeometryDisplayMode::Solid);
    }
  }
}

// -----------------------------------------------------------------------------

auto GGEMSGeometryAppearanceState::Clear() noexcept -> void {
  occurrences_.clear();
  group_counts_.clear();
  child_modes_.clear();
}

// -----------------------------------------------------------------------------

auto GGEMSGeometryAppearanceState::GetOccurrences() const noexcept
  -> std::span<GGEMSGeometryAppearance const> {
  return occurrences_;
}

// -----------------------------------------------------------------------------

auto GGEMSGeometryAppearanceState::GetOccurrenceMode(
  GGEMSGeometryScene const &scene, std::uint32_t volume_id) const noexcept
  -> GGEMSGeometryDisplayMode {
  return ResolveGeometryDisplayMode(scene.occurrences[volume_id],
                                    occurrences_[volume_id]);
}

// -----------------------------------------------------------------------------

auto GGEMSGeometryAppearanceState::SetOccurrenceMode(
  GGEMSGeometryScene const &scene, std::uint32_t volume_id,
  GGEMSGeometryDisplayMode mode) noexcept -> void {
  auto const previous = GetOccurrenceMode(scene, volume_id);
  occurrences_[volume_id].mode_override = mode;
  auto const current = GetOccurrenceMode(scene, volume_id);

  if (previous == current) {
    return;
  }

  for (auto const group : scene.occurrences[volume_id].controlling_groups) {
    --group_counts_[group][static_cast<std::size_t>(previous)];
    ++group_counts_[group][static_cast<std::size_t>(current)];
  }
}

// -----------------------------------------------------------------------------

auto GGEMSGeometryAppearanceState::GetGroupMode(std::uint32_t group_index)
  const noexcept -> std::optional<GGEMSGeometryDisplayMode> {
  std::optional<GGEMSGeometryDisplayMode> result;
  auto const &counts = group_counts_[group_index];

  for (std::size_t index = 0U; index < counts.size(); ++index) {
    if (counts[index] != 0U) {
      if (result) {
        return std::nullopt;
      }
      result = static_cast<GGEMSGeometryDisplayMode>(index);
    }
  }
  return result;
}

// -----------------------------------------------------------------------------

auto GGEMSGeometryAppearanceState::SetGroupMode(
  GGEMSGeometryScene const &scene, std::uint32_t group_index,
  GGEMSGeometryDisplayMode mode) noexcept -> void {
  for (auto const member : scene.groups[group_index].members) {
    SetOccurrenceMode(scene, member, mode);
  }
}

// -----------------------------------------------------------------------------

auto GGEMSGeometryAppearanceState::GetChildMode(
  std::uint32_t volume_id, std::uint32_t index) const noexcept
  -> GGEMSGeometryDisplayMode {
  return child_modes_[volume_id][index];
}

// -----------------------------------------------------------------------------

auto GGEMSGeometryAppearanceState::SetChildMode(
  std::uint32_t volume_id, std::uint32_t index,
  GGEMSGeometryDisplayMode mode) noexcept -> void {
  child_modes_[volume_id][index] = mode;
}

// -----------------------------------------------------------------------------

auto GGEMSGeometryAppearanceState::GetVoxelMappingMode(
  std::uint32_t volume_id, std::uint32_t mapping_index) const noexcept
  -> GGEMSGeometryDisplayMode {
  return GetChildMode(volume_id, mapping_index);
}

// -----------------------------------------------------------------------------

auto GGEMSGeometryAppearanceState::SetVoxelMappingMode(
  std::uint32_t volume_id, std::uint32_t mapping_index,
  GGEMSGeometryDisplayMode mode) noexcept -> void {
  SetChildMode(volume_id, mapping_index, mode);
}

} // namespace ggems::render
