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
 * \brief Shares camera selection and presentation actions between UI controls.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <cmath>
#include <cstddef>

#include "GGEMS/ui/detail/GGEMSViewportToolbar.hh"
#include "GGEMS/ui/detail/GGEMSSceneCamera.hh"
#include "GGEMS/ui/detail/GGEMSWorkbenchState.hh"

namespace ggems::ui::detail {

auto IsCameraViewSelected(GGEMSSceneCamera::Diagnostics const &current,
                          GGEMSSceneCamera::ViewPlane plane) -> bool {
  GGEMSSceneCamera reference;
  reference.SetView(plane);
  auto const expected = reference.GetDiagnostics();

  for (std::size_t index = 0U; index < current.right.size(); ++index) {
    if (std::abs(current.right[index] - expected.right[index]) > 1.0e-5F ||
        std::abs(current.up[index] - expected.up[index]) > 1.0e-5F) {
      return false;
    }
  }

  return true;
}

// -----------------------------------------------------------------------------

auto ApplyViewportToolbarAction(GGEMSWorkbenchState &workbench,
                                GGEMSViewportToolbarAction action) -> void {
  using Action = GGEMSViewportToolbarAction;
  using Camera = GGEMSSceneCamera;
  auto &camera = workbench.camera;
  auto &grid = workbench.cartesian_grid;

  switch (action) {
  case Action::FreeView:
  case Action::ResetCamera:
    camera.Reset();
    break;
  case Action::ViewXY:
    camera.SetView(Camera::ViewPlane::XY);
    grid.reference_plane = Camera::ViewPlane::XY;
    break;
  case Action::ViewXZ:
    camera.SetView(Camera::ViewPlane::XZ);
    grid.reference_plane = Camera::ViewPlane::XZ;
    break;
  case Action::ViewYZ:
    camera.SetView(Camera::ViewPlane::YZ);
    grid.reference_plane = Camera::ViewPlane::YZ;
    break;
  case Action::Orthographic:
    camera.SetProjection(Camera::Projection::Orthographic);
    break;
  case Action::Perspective:
    camera.SetProjection(Camera::Projection::Perspective);
    break;
  case Action::CartesianXY:
    grid.visible[0] = !grid.visible[0];
    break;
  case Action::CartesianXZ:
    grid.visible[1] = !grid.visible[1];
    break;
  case Action::CartesianYZ:
    grid.visible[2] = !grid.visible[2];
    break;
  case Action::PolarXY:
    grid.polar_visible[0] = !grid.polar_visible[0];
    break;
  case Action::PolarXZ:
    grid.polar_visible[1] = !grid.polar_visible[1];
    break;
  case Action::PolarYZ:
    grid.polar_visible[2] = !grid.polar_visible[2];
    break;
  case Action::Axes:
    workbench.show_axes = !workbench.show_axes;
    break;
  case Action::ParticleTraces:
    workbench.trace_visibility.SetGlobalVisible(
      !workbench.trace_visibility.IsGlobalVisible());
    break;
  }
}

} // namespace ggems::ui::detail
