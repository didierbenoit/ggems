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
 * \brief Builds and restores the persistent GGEMS docking layout.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <algorithm>
#include <array>

#include <imgui.h>
#include <imgui_internal.h>

#include "GGEMS/ui/detail/GGEMSImGuiLayout.hh"
#include "GGEMS/ui/detail/GGEMSWorkbenchState.hh"

namespace {

// =============================================================================
// =============================================================================

/*! \brief Panel identities participating in saved layout restoration. */
constexpr std::array<char const *, 8U> k_layout_window_names{
  ggems::ui::detail::k_output_window_name,
  ggems::ui::detail::k_status_window_name,
  ggems::ui::detail::k_scene_window_name,
  ggems::ui::detail::k_viewport_window_name,
  ggems::ui::detail::k_inspector_window_name,
  ggems::ui::detail::k_camera_window_name,
  ggems::ui::detail::k_source_window_name,
  ggems::ui::detail::k_volumes_window_name,
};

// =============================================================================
// =============================================================================

/*!
 * \brief Checks saved docking or floating-window placement for one panel.
 *
 * \param[in] name Panel label containing its persistent ImGui identity.
 * \return True for an existing dock node or a positive saved floating size.
 */
[[nodiscard]] auto IsUsableWindowSettings(char const *name) -> bool {
  ImGuiWindowSettings const *settings =
    ImGui::FindWindowSettingsByID(ImHashStr(name));

  if (settings == nullptr) {
    return false;
  }

  if (settings->DockId != 0U) {
    return ImGui::DockBuilderGetNode(settings->DockId) != nullptr;
  }

  return settings->Size.x > 0 && settings->Size.y > 0;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Finds a saved leaf dock that can admit a genuinely new panel.
 *
 * \param[in] name Existing panel's persistent identity.
 * \return Existing leaf node, or null for floating or missing placement.
 */
[[nodiscard]] auto SavedLeafDock(char const *name) -> ImGuiDockNode * {
  auto const *settings = ImGui::FindWindowSettingsByID(ImHashStr(name));

  if (settings == nullptr || settings->DockId == 0U) {
    return nullptr;
  }

  auto *node = ImGui::DockBuilderGetNode(settings->DockId);
  return node != nullptr && node->IsLeafNode() ? node : nullptr;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Admits a new panel without changing any existing saved placement.
 *
 * \param[in] name New panel's persistent identity.
 * \param[in] node Existing leaf dock, or null when none is available.
 */
auto DockMissingPanel(char const *name, ImGuiDockNode *node) -> void {
  if (node == nullptr ||
      ImGui::FindWindowSettingsByID(ImHashStr(name)) != nullptr) {
    return;
  }

  ImGui::DockBuilderDockWindow(name, node->ID);
  ImGui::DockBuilderFinish(node->ID);
  ImGui::GetIO().WantSaveIniSettings = true;
}

} // namespace

namespace ggems::ui::detail {

// =============================================================================
// =============================================================================

auto GetMainDockspaceID() -> ImGuiID {
  return ImHashStr("GGEMS.MainDockspaceNode");
}

// -----------------------------------------------------------------------------

auto HasSavedLayout() -> bool {
  return std::ranges::any_of(k_layout_window_names, IsUsableWindowSettings);
}

// -----------------------------------------------------------------------------

auto InitializeLayout(ImVec2 const &dockspace_size,
                      GGEMSWorkbenchState &workbench) -> void {
  if (!HasSavedLayout()) {
    BuildDefaultLayout(dockspace_size);
    return;
  }

  workbench.show_scene_panel =
    workbench.show_scene_panel || IsUsableWindowSettings(k_scene_window_name);

  workbench.show_inspector_panel =
    workbench.show_inspector_panel ||
    IsUsableWindowSettings(k_inspector_window_name);

  auto *side_dock = SavedLeafDock(k_inspector_window_name);

  DockMissingPanel(k_camera_window_name, side_dock);

  if (side_dock == nullptr) {
    side_dock = SavedLeafDock(k_scene_window_name);
  }

  if (side_dock == nullptr) {
    side_dock = SavedLeafDock(k_status_window_name);
  }

  DockMissingPanel(k_source_window_name, side_dock);
  DockMissingPanel(k_volumes_window_name, side_dock);
}

// -----------------------------------------------------------------------------

auto RestoreInitialDockTabs() -> void {
  for (char const *name : k_layout_window_names) {
    auto const *panel = ImGui::FindWindowByName(name);

    if (panel == nullptr || panel->DockNode == nullptr) {
      continue;
    }

    auto const *node = panel->DockNode;

    for (auto const *window : node->Windows) {
      if (window->TabId == node->SelectedTabId) {
        ImGui::SetWindowFocus(window->Name);
        break;
      }
    }
  }
}

// -----------------------------------------------------------------------------

auto BuildDefaultLayout(ImVec2 const &dockspace_size) -> void {
  ImGuiID const dockspace_id = GetMainDockspaceID();

  ImGui::DockBuilderRemoveNode(dockspace_id);

  ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
  ImGui::DockBuilderSetNodeSize(dockspace_id, dockspace_size);

  ImGuiID dockspace_main_id = dockspace_id;

  ImGuiID const dock_bottom_id = ImGui::DockBuilderSplitNode(
    dockspace_main_id, ImGuiDir_Down, 0.30F, nullptr, &dockspace_main_id);

  ImGuiID const dock_left_id = ImGui::DockBuilderSplitNode(
    dockspace_main_id, ImGuiDir_Left, 0.24F, nullptr, &dockspace_main_id);

  ImGuiID const dock_right_id = ImGui::DockBuilderSplitNode(
    dockspace_main_id, ImGuiDir_Right, 0.26F, nullptr, &dockspace_main_id);

  ImGui::DockBuilderDockWindow(k_output_window_name, dock_bottom_id);
  ImGui::DockBuilderDockWindow(k_status_window_name, dock_left_id);
  ImGui::DockBuilderDockWindow(k_camera_window_name, dock_left_id);
  ImGui::DockBuilderDockWindow(k_viewport_window_name, dockspace_main_id);
  ImGui::DockBuilderDockWindow(k_source_window_name, dock_right_id);
  ImGui::DockBuilderDockWindow(k_volumes_window_name, dock_right_id);

  // ImGui seeds each dock-tab identity with its window identity.
  ImGui::DockBuilderGetNode(dock_left_id)->SelectedTabId =
    ImHashStr("#TAB", 0U, ImHashStr(k_status_window_name));

  ImGui::DockBuilderGetNode(dock_right_id)->SelectedTabId =
    ImHashStr("#TAB", 0U, ImHashStr(k_source_window_name));

  ImGui::DockBuilderFinish(dockspace_id);
}

// -----------------------------------------------------------------------------

auto ResetLayout(ImVec2 const &dockspace_size) -> void {
  for (char const *name : k_layout_window_names) {
    ImGui::ClearWindowSettings(name);
  }

  BuildDefaultLayout(dockspace_size);
  ImGui::GetIO().WantSaveIniSettings = true;
}

} // namespace ggems::ui::detail
