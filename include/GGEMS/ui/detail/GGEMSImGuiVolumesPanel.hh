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

namespace ggems::render {
struct GGEMSGeometryScene;
}

namespace ggems::ui::detail {

struct GGEMSWorkbenchState;

/*!
 * \brief Displays named controls and physical rows above owned metadata.
 *
 * Single clicks select a group, occurrence or OBJ object. Double clicks
 * frame the selected geometry in its current pose.
 * The tree child owns its resizable height through Dear ImGui window state.
 * \param[in,out] workbench Viewer-owned visibility, selection, modes and
 * camera.
 * \param[in] scene Borrowed immutable scene, or null before World submission.
 */
auto BuildVolumesPanel(GGEMSWorkbenchState &workbench,
                       render::GGEMSGeometryScene const *scene) -> void;
} // namespace ggems::ui::detail
