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
 * \brief Composes docked panels and applies their camera and visibility input.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <imgui.h>

#include "GGEMS/ui/detail/GGEMSImGuiLayer.hh"
#include "GGEMS/ui/detail/GGEMSDeviceStatus.hh"
#include "GGEMS/ui/detail/GGEMSImGuiLayout.hh"
#include "GGEMS/ui/detail/GGEMSImGuiPanels.hh"
#include "GGEMS/ui/detail/GGEMSImGuiViewportPanel.hh"
#include "GGEMS/ui/detail/GGEMSWorkbenchState.hh"

namespace ggems::ui {

// =============================================================================
// =============================================================================

auto GGEMSImGuiLayer::BuildFrame(
  FrameInputs const &inputs,
  detail::GGEMSDeviceStatusSnapshot const &device_status,
  detail::GGEMSWorkbenchState &workbench) -> void {
  workbench.viewport.visible = false;

  MenuActions const menu_actions = BuildMainDockspace(workbench);

  if (workbench.show_output_panel) {
    output_panel_.Render(*inputs.output_state);
  }

  if (workbench.show_status_panel) {
    detail::BuildStatusPanel(workbench.show_status_panel, device_status,
                             inputs.swapchain_width, inputs.swapchain_height,
                             workbench.show_axes,
                             workbench.trace_visibility.IsGlobalVisible());
  }

  if (workbench.show_scene_panel) {
    detail::BuildScenePanel(workbench.show_scene_panel, workbench,
                            inputs.source_run_snapshot);
  }

  if (workbench.show_viewport_panel) {
    detail::GGEMSImGuiViewportPanel::Result const result =
      viewport_panel_.Build(workbench.show_viewport_panel,
                            inputs.scene_texture_id,
                            inputs.scene_texture_logical_width,
                            inputs.scene_texture_logical_height);

    workbench.viewport.visible = result.visible;

    if (result.visible) {
      workbench.viewport.width = result.width;
      workbench.viewport.height = result.height;
    }

    workbench.camera.OrbitByPixels(result.orbit_delta_x_pixels,
                                   result.orbit_delta_y_pixels);
    workbench.camera.Pan(result.pan_delta_x_pixels, result.pan_delta_y_pixels);
    workbench.camera.ZoomBy(result.zoom_delta);
  } else {
    viewport_panel_.CancelGesture();
  }

  if (menu_actions.reset_camera) {
    workbench.camera.Reset();
  }

  if (workbench.show_inspector_panel) {
    detail::BuildInspectorPanel(workbench.show_inspector_panel,
                                workbench.selected_scene_item);
  }
}

// -----------------------------------------------------------------------------

auto GGEMSImGuiLayer::BuildMainDockspace(detail::GGEMSWorkbenchState &workbench)
  -> MenuActions {
  ImGuiViewport const *viewport = ImGui::GetMainViewport();

  ImGui::SetNextWindowPos(viewport->WorkPos);
  ImGui::SetNextWindowSize(viewport->WorkSize);
  ImGui::SetNextWindowViewport(viewport->ID);

  ImGuiWindowFlags const window_flags =
    ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking |
    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
    ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
    ImGuiWindowFlags_NoBackground;

  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0F);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0F);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0.0F, 0.0F});

  bool dockspace_open{true};
  ImGui::Begin(detail::k_main_dockspace_window_name, &dockspace_open,
               window_flags);

  ImGui::PopStyleVar(3);

  MenuActions const menu_actions = BuildMainMenuBar(workbench);

  if (!layout_initialized_) {
    if (!detail::HasSavedLayout()) {
      detail::BuildDefaultLayout(viewport->Size);
    }

    layout_initialized_ = true;
  }

  if (menu_actions.reset_layout) {
    detail::ResetLayout(viewport->Size);

    workbench.show_output_panel = true;
    workbench.show_status_panel = true;
    workbench.show_scene_panel = true;
    workbench.show_viewport_panel = true;
    workbench.show_inspector_panel = true;
  }

  ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

  ImGui::DockSpace(detail::GetMainDockspaceID(), ImVec2{0.0F, 0.0F},
                   dockspace_flags);

  ImGui::End();

  return menu_actions;
}

// -----------------------------------------------------------------------------

auto GGEMSImGuiLayer::BuildMainMenuBar(detail::GGEMSWorkbenchState &workbench)
  -> MenuActions {
  MenuActions menu_actions{};

  if (!ImGui::BeginMainMenuBar()) {
    return menu_actions;
  }

  if (ImGui::BeginMenu("View")) {
    ImGui::MenuItem("Output", nullptr, &workbench.show_output_panel);
    ImGui::MenuItem("Status", nullptr, &workbench.show_status_panel);
    ImGui::MenuItem("Scene", nullptr, &workbench.show_scene_panel);
    ImGui::MenuItem("Viewport", nullptr, &workbench.show_viewport_panel);
    ImGui::MenuItem("Inspector", nullptr, &workbench.show_inspector_panel);

    ImGui::Separator();
    ImGui::MenuItem("Axes", nullptr, &workbench.show_axes);

    bool show_particle_traces = workbench.trace_visibility.IsGlobalVisible();
    if (ImGui::MenuItem("Particle traces", nullptr, &show_particle_traces)) {
      workbench.trace_visibility.SetGlobalVisible(show_particle_traces);
    }

    menu_actions.reset_camera = ImGui::MenuItem("Reset Camera");

    ImGui::Separator();
    menu_actions.reset_layout = ImGui::MenuItem("Reset layout");

    ImGui::EndMenu();
  }

  ImGui::EndMainMenuBar();
  return menu_actions;
}

} // namespace ggems::ui
