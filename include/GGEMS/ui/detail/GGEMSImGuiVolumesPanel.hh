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
 * \brief Inspects immutable physical occurrences and requests camera framing.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>
#include <optional>

#include "GGEMS/render/GGEMSGeometryScene.hh"
#include "GGEMS/render/GGEMSGeometryAppearance.hh"

namespace ggems::ui::detail {

/*!
 * \brief Displays named controls and physical rows above owned metadata.
 *
 * Single clicks select a group or occurrence; physical double clicks frame it.
 * The tree child owns its resizable height through Dear ImGui window state.
 * \param[in,out] show_window Panel visibility, updated on close.
 * \param[in,out] selected_volume_id Snapshot-local selection, or no selection.
 * \param[in,out] selected_geometry_group Viewer-only group index, or no
 * selection.
 * \param[in] scene Borrowed immutable scene, or null before World submission.
 * \param[in,out] appearance Snapshot-local modes and cached group summaries.
 * \param[in] presentation_revision Separates widget state between snapshots.
 * \return VolumeID to frame after a label double click, otherwise no request.
 */
[[nodiscard]] auto BuildVolumesPanel(
  bool &show_window, std::optional<std::uint32_t> &selected_volume_id,
  std::optional<std::uint32_t> &selected_geometry_group,
  render::GGEMSGeometryScene const *scene,
  render::GGEMSGeometryAppearanceState &appearance,
  std::uint32_t presentation_revision) -> std::optional<std::uint32_t>;

} // namespace ggems::ui::detail
