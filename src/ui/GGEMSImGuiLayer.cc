#include <imgui.h>
#include <imgui_internal.h>

#include "GGEMS/ui/detail/GGEMSImGuiLayer.hh"
#include "GGEMS/ui/detail/GGEMSDeviceStatus.hh"
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

  bool const reset_camera_requested = BuildMainDockspace(workbench);

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
                            inputs.scene_texture_id, inputs.scene_texture_width,
                            inputs.scene_texture_height);

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

  if (reset_camera_requested) {
    workbench.camera.Reset();
  }

  if (workbench.show_inspector_panel) {
    detail::BuildInspectorPanel(workbench.show_inspector_panel,
                                workbench.selected_scene_item);
  }
}

// -----------------------------------------------------------------------------

auto GGEMSImGuiLayer::BuildMainDockspace(detail::GGEMSWorkbenchState &workbench)
  -> bool {
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
  ImGui::Begin("GGEMS Main Dockspace", &dockspace_open, window_flags);

  ImGui::PopStyleVar(3);

  bool const reset_camera_requested = BuildMainMenuBar(workbench);

  ImGuiID dockspace_id = ImGui::GetID("GGEMS_Dockspace");

  ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

  ImGui::DockSpace(dockspace_id, ImVec2{0.0F, 0.0F}, dockspace_flags);

  if (!dockspace_layout_built_) {
    BuildDefaultDockspaceLayout(dockspace_id, viewport->WorkSize);
    dockspace_layout_built_ = true;
  }

  ImGui::End();

  return reset_camera_requested;
}

// -----------------------------------------------------------------------------

auto GGEMSImGuiLayer::BuildMainMenuBar(detail::GGEMSWorkbenchState &workbench)
  -> bool {
  if (!ImGui::BeginMainMenuBar()) {
    return false;
  }

  bool reset_camera_requested{false};

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

    reset_camera_requested = ImGui::MenuItem("Reset Camera");

    ImGui::EndMenu();
  }

  ImGui::EndMainMenuBar();
  return reset_camera_requested;
}

// -----------------------------------------------------------------------------

auto GGEMSImGuiLayer::BuildDefaultDockspaceLayout(ImGuiID dockspace_id,
                                                  ImVec2 const &dockspace_size)
  -> void {
  ImGui::DockBuilderRemoveNode(dockspace_id);

  ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
  ImGui::DockBuilderSetNodeSize(dockspace_id, dockspace_size);

  ImGuiID dockspace_main_id = dockspace_id;

  ImGuiID dock_bottom_id = ImGui::DockBuilderSplitNode(
    dockspace_main_id, ImGuiDir_Down, 0.30F, nullptr, &dockspace_main_id);

  ImGuiID dock_left_id = ImGui::DockBuilderSplitNode(
    dockspace_main_id, ImGuiDir_Left, 0.24F, nullptr, &dockspace_main_id);

  ImGuiID dock_right_id = ImGui::DockBuilderSplitNode(
    dockspace_main_id, ImGuiDir_Right, 0.26F, nullptr, &dockspace_main_id);

  ImGui::DockBuilderDockWindow("GGEMS Output", dock_bottom_id);
  ImGui::DockBuilderDockWindow("GGEMS Status", dock_left_id);
  ImGui::DockBuilderDockWindow("GGEMS Scene", dock_left_id);
  ImGui::DockBuilderDockWindow("GGEMS Viewport", dockspace_main_id);
  ImGui::DockBuilderDockWindow("GGEMS Inspector", dock_right_id);

  ImGui::DockBuilderFinish(dockspace_id);
}

} // namespace ggems::ui
