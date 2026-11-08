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
 * \brief Defines persistent panel identities and the default dock layout.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <imgui.h>

namespace ggems::ui::detail {

struct GGEMSWorkbenchState;

/*! \brief Display label and persistent ID of the dockspace host. */
inline constexpr char const *k_main_dockspace_window_name =
  "GGEMS Main Dockspace###GGEMS.MainDockspace";

/*! \brief Display label and persistent ID of the output panel. */
inline constexpr char const *k_output_window_name =
  "GGEMS Output###GGEMS.Output";

/*! \brief Display label and persistent ID of the status panel. */
inline constexpr char const *k_status_window_name =
  "GGEMS Status###GGEMS.Status";

/*! \brief Display label and persistent ID of the scene panel. */
inline constexpr char const *k_scene_window_name = "GGEMS Scene###GGEMS.Scene";

/*! \brief Display label and persistent ID of the Source panel. */
inline constexpr char const *k_source_window_name =
  "GGEMS Source###GGEMS.Source";

/*! \brief Display label and persistent ID of the physical-volume explorer. */
inline constexpr char const *k_volumes_window_name =
  "GGEMS Volumes###GGEMS.Volumes";

/*! \brief Display label and persistent ID of the viewport panel. */
inline constexpr char const *k_viewport_window_name =
  "GGEMS Viewport###GGEMS.Viewport";

/*! \brief Display label and persistent ID of the inspector panel. */
inline constexpr char const *k_inspector_window_name =
  "GGEMS Inspector###GGEMS.Inspector";

/*! \brief Display label and persistent ID of the camera diagnostics panel. */
inline constexpr char const *k_camera_window_name =
  "GGEMS Camera###GGEMS.Camera";

/*!
 * \brief Returns the persistent main docking-node identity.
 *
 * \return Docking-node ID used by layout construction and restoration.
 */
[[nodiscard]] auto GetMainDockspaceID() -> ImGuiID;

/*!
 * \brief Checks for a usable saved panel placement.
 *
 * \return True when at least one panel has usable window settings.
 */
[[nodiscard]] auto HasSavedLayout() -> bool;

/*!
 * \brief Restores panel placements or creates the default docking layout.
 *
 * Existing placements are preserved. Missing Source and Volumes panels join
 * an available saved side dock; legacy panels with usable settings reopen.
 * \param[in] dockspace_size Available dockspace size in logical UI pixels.
 * \param[in,out] workbench Panel visibility admitting retained legacy panels.
 */
auto InitializeLayout(ImVec2 const &dockspace_size,
                      GGEMSWorkbenchState &workbench) -> void;

/*!
 * \brief Restores admitted dock selections after initial windows open.
 *
 * Call after submitting the first frame's panels or resetting the layout.
 * Saved selections and the default tabs survive automatic opening focus.
 */
auto RestoreInitialDockTabs() -> void;

/*!
 * \brief Replaces the main docking tree with the default panel layout.
 *
 * \param[in] dockspace_size Available dockspace size in logical UI pixels.
 */
auto BuildDefaultLayout(ImVec2 const &dockspace_size) -> void;

/*!
 * \brief Clears saved panel placements and requests layout persistence.
 *
 * \param[in] dockspace_size Logical UI extent for the rebuilt default layout.
 */
auto ResetLayout(ImVec2 const &dockspace_size) -> void;

} // namespace ggems::ui::detail
