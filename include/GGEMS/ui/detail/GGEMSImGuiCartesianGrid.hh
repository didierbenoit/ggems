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
 * \brief Presents World-grid spacing and Cartesian/polar cursor coordinates.
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
 * \brief Displays a ray-plane intersection and physical grid spacing.
 * Polar coordinates use (u,v) = (X,Y), (X,Z), or (Y,Z) on the reference
 * plane. Azimuth runs from +u toward +v in [0,360) degrees. It is omitted when
 * the radius rounds to zero under the existing picometer length policy.
 * Both coordinate forms use the same ray-plane intersection.
 * This read-only presentation does not select a plane or change visibility.
 *
 * \param[in] camera Camera used for the displayed scene image.
 * \param[in] grid Current reference plane and shared shader spacing.
 * \param[in] input Current image-relative cursor, absent when unavailable.
 * \param[in] world_loaded Whether physical World bounds have been submitted.
 */
auto BuildCartesianGridReadout(GGEMSSceneCamera const &camera,
                               GGEMSCartesianGrid const &grid,
                               GGEMSImGuiViewportPanel::Result const &input,
                               bool world_loaded) -> void;

} // namespace ggems::ui::detail
