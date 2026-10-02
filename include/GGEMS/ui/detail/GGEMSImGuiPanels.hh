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
 * \brief Builds device status, scene hierarchy, and selection panels.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>

#include "GGEMSDeviceStatus.hh"
#include "GGEMSWorkbenchState.hh"
#include "GGEMS/sources/GGEMSSourceRunSnapshot.hh"

namespace ggems::ui::detail {

/*!
 * \brief Displays renderer, compute, and presentation status.
 *
 * \param[in,out] show_window Panel visibility, updated when the window closes.
 * \param[in] device_status Captured renderer and compute device descriptions.
 * \param[in] swapchain_width Presentation image width in physical pixels.
 * \param[in] swapchain_height Presentation image height in physical pixels.
 * \param[in] show_axes Whether reference axes are visible.
 * \param[in] particle_traces_visible Global diagnostic trace visibility.
 */
auto BuildStatusPanel(bool &show_window,
                      GGEMSDeviceStatusSnapshot const &device_status,
                      std::uint32_t swapchain_width,
                      std::uint32_t swapchain_height, bool show_axes,
                      bool particle_traces_visible) -> void;

/*!
 * \brief Displays submitted sources and scene presentation controls.
 *
 * \param[in,out] show_window Panel visibility, updated when the window closes.
 * \param[in,out] workbench Scene selection and trace visibility controls.
 * \param[in] source_run_snapshot Borrowed completed source snapshot, or null
 *   when absent.
 */
auto BuildScenePanel(
  bool &show_window, GGEMSWorkbenchState &workbench,
  core::sources::GGEMSSourceRunSnapshot const *source_run_snapshot) -> void;

/*!
 * \brief Displays the selected category and unavailable-property labels.
 *
 * \param[in,out] show_window Panel visibility, updated when the window closes.
 * \param[in] selection Scene category currently selected for inspection.
 */
auto BuildInspectorPanel(bool &show_window,
                         GGEMSWorkbenchState::SceneSelection selection) -> void;

} // namespace ggems::ui::detail
