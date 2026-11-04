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
 * \brief Checks the initial visible workbench configuration.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <gtest/gtest.h>

#include "GGEMS/ui/detail/GGEMSSceneCamera.hh"
#include "GGEMS/ui/detail/GGEMSWorkbenchState.hh"

namespace {

using Workbench = ggems::ui::detail::GGEMSWorkbenchState;

// =============================================================================
// =============================================================================

// The initial workbench must expose its panels and use the reset view before a
// viewport is opened.
TEST(GGEMSWorkbenchState, DefaultsPresentEverythingWithCanonicalCamera) {
  Workbench const workbench{};

  EXPECT_TRUE(workbench.show_output_panel);
  EXPECT_TRUE(workbench.show_status_panel);
  EXPECT_TRUE(workbench.show_scene_panel);
  EXPECT_TRUE(workbench.show_viewport_panel);
  EXPECT_TRUE(workbench.show_inspector_panel);
  EXPECT_TRUE(workbench.show_camera_panel);
  EXPECT_TRUE(workbench.show_axes);
  EXPECT_TRUE(workbench.trace_visibility.IsGlobalVisible());
  EXPECT_EQ(workbench.selected_scene_item, Workbench::SceneSelection::World);
  EXPECT_FALSE(workbench.viewport.visible);
  EXPECT_EQ(workbench.viewport.width, 0U);
  EXPECT_EQ(workbench.viewport.height, 0U);

  ggems::ui::detail::GGEMSSceneCamera reset_camera{};
  reset_camera.Orbit(30.0F, 10.0F);
  reset_camera.Reset();

  auto const initial_matrix = workbench.camera.BuildWorldToClipMatrix();
  auto const reset_matrix = reset_camera.BuildWorldToClipMatrix();
  EXPECT_EQ(initial_matrix.row_0, reset_matrix.row_0);
  EXPECT_EQ(initial_matrix.row_1, reset_matrix.row_1);
  EXPECT_EQ(initial_matrix.row_2, reset_matrix.row_2);
  EXPECT_EQ(initial_matrix.row_3, reset_matrix.row_3);
}

} // namespace
