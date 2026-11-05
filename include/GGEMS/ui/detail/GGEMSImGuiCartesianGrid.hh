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
 * \brief Presents World-grid controls and read-only reference-plane positions.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include "GGEMSCartesianGrid.hh"
#include "GGEMSImGuiViewportPanel.hh"
#include "GGEMSSceneCamera.hh"

namespace ggems::ui::detail {

/*!
 * \brief Builds the View submenus for grid visibility and cursor plane.
 *
 * \param[in,out] grid Presentation options changed only by menu activation.
 */
auto BuildCartesianGridMenus(GGEMSCartesianGrid &grid) -> void;

/*!
 * \brief Displays a ray-plane intersection and physical grid spacing.
 *
 * \param[in] camera Camera used for the displayed scene image.
 * \param[in] grid Current reference plane and grid intervals.
 * \param[in] input Current image-relative cursor, absent when unavailable.
 * \param[in] world_loaded Whether physical World bounds have been submitted.
 */
auto BuildCartesianGridReadout(GGEMSSceneCamera const &camera,
                               GGEMSCartesianGrid const &grid,
                               GGEMSImGuiViewportPanel::Result const &input,
                               bool world_loaded) -> void;

} // namespace ggems::ui::detail
