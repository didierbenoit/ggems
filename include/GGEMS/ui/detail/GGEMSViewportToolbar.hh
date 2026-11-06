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
 * \brief Shares viewport icon identities and existing presentation commands.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include <imgui.h>

#include "GGEMSSceneCamera.hh"

namespace ggems::ui::detail {

struct GGEMSWorkbenchState;

/*! \brief Stable left-to-right icon order and presentation command identity. */
enum class GGEMSViewportToolbarAction : std::uint8_t {
  /*! \brief Restores the default free orientation and framing. */
  FreeView,

  /*! \brief Selects the XY camera view and cursor plane. */
  ViewXY,

  /*! \brief Selects the XZ camera view and cursor plane. */
  ViewXZ,

  /*! \brief Selects the YZ camera view and cursor plane. */
  ViewYZ,

  /*! \brief Selects orthographic projection. */
  Orthographic,

  /*! \brief Selects perspective projection. */
  Perspective,

  /*! \brief Toggles the Cartesian XY grid. */
  CartesianXY,

  /*! \brief Toggles the Cartesian XZ grid. */
  CartesianXZ,

  /*! \brief Toggles the Cartesian YZ grid. */
  CartesianYZ,

  /*! \brief Toggles the polar XY grid. */
  PolarXY,

  /*! \brief Toggles the polar XZ grid. */
  PolarXZ,

  /*! \brief Toggles the polar YZ grid. */
  PolarYZ,

  /*! \brief Toggles World axes. */
  Axes,

  /*! \brief Toggles global trace visibility without changing source choices. */
  ParticleTraces,

  /*! \brief Invokes the existing camera reset, without changing the layout. */
  ResetCamera,
};

/*! \brief Number of original images in the viewport toolbar. */
inline constexpr std::size_t k_ggems_viewport_toolbar_icon_count{15U};

/*! \brief Borrows texture descriptors in GGEMSViewportToolbarAction order. */
using GGEMSViewportToolbarIcons =
  std::array<ImTextureID, k_ggems_viewport_toolbar_icon_count>;

/*!
 * \brief Recognizes a preset from the current camera basis, without cached
 * flags.
 *
 * \param[in] current Live camera diagnostics.
 * \param[in] plane Existing camera preset used as the orientation authority.
 * \return Whether right and up match within the existing display tolerance.
 */
[[nodiscard]] auto
IsCameraViewSelected(GGEMSSceneCamera::Diagnostics const &current,
                     GGEMSSceneCamera::ViewPlane plane) -> bool;

/*!
 * \brief Applies an existing View-menu command to shared presentation state.
 *
 * \param[in,out] workbench Camera, grids, axes and trace visibility.
 * \param[in] action Command issued by the toolbar or main View menu.
 */
auto ApplyViewportToolbarAction(GGEMSWorkbenchState &workbench,
                                GGEMSViewportToolbarAction action) -> void;

} // namespace ggems::ui::detail
