#include <cmath>
#include <algorithm>

#include <imgui.h>
#include <imgui_internal.h>

#include "GGEMSImGuiLayer.hh"
#include "GGEMS/core/GGEMSOutputMode.hh"

namespace ggems::ui {

// =============================================================================
// =============================================================================

char const *GGEMSImGuiLayer::GetSceneSelectionName() const noexcept {
  switch (selected_scene_item_) {
  case SceneSelection::None:
    return "None";
  case SceneSelection::World:
    return "World";
  case SceneSelection::Sources:
    return "Sources";
  case SceneSelection::Volumes:
    return "Volumes";
  case SceneSelection::Materials:
    return "Materials";
  case SceneSelection::Tracks:
    return "Tracks";
  case SceneSelection::Particles:
    return "Particles";
  case SceneSelection::Processes:
    return "Processes";
  }

  return "Unknown";
}

// =============================================================================
// =============================================================================

void GGEMSImGuiLayer::BuildFrame(vk::Extent2D const &swapchain_extent,
                                 ImTextureID scene_texture_id,
                                 vk::Extent2D const &scene_texture_extent) {
  reset_camera_requested_ = false;

  BuildMainDockspace();

  if (show_output_panel_) {
    render::GGEMSBanner &banner = core::GetOutputBanner();
    core::GGEMSOutputState &output_state = core::GetOutputState();

    output_panel_.Render(banner, output_state);
  }

  if (show_status_panel_) {
    BuildStatusPanel(swapchain_extent);
  }

  if (show_scene_panel_) {
    BuildScenePanel();
  }

  if (show_viewport_placeholder_) {
    BuildViewportPlaceholder(scene_texture_id, scene_texture_extent);
  }

  if (show_inspector_panel_) {
    BuildInspectorPanel();
  }
}

// -----------------------------------------------------------------------------

bool GGEMSImGuiLayer::ShouldShowAxes() const noexcept { return show_axes_; }

// -----------------------------------------------------------------------------

bool GGEMSImGuiLayer::ShouldShowParticleTraces() const noexcept {
  return show_particle_traces_;
}

// -----------------------------------------------------------------------------

bool GGEMSImGuiLayer::ShouldResetCamera() const noexcept {
  return reset_camera_requested_;
}

// -----------------------------------------------------------------------------

GGEMSImGuiLayer::ViewportState const &
GGEMSImGuiLayer::GetViewportState() const noexcept {
  return viewport_state_;
}

// -----------------------------------------------------------------------------

void GGEMSImGuiLayer::BuildMainDockspace() {
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

  BuildMainMenuBar();

  ImGuiID dockspace_id = ImGui::GetID("GGEMS_Dockspace");

  ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

  ImGui::DockSpace(dockspace_id, ImVec2{0.0F, 0.0F}, dockspace_flags);

  if (!dockspace_layout_built_) {
    BuildDefaultDockspaceLayout(dockspace_id, viewport->WorkSize);
    dockspace_layout_built_ = true;
  }

  ImGui::End();
}

// -----------------------------------------------------------------------------

void GGEMSImGuiLayer::BuildMainMenuBar() {
  if (!ImGui::BeginMainMenuBar()) {
    return;
  }

  if (ImGui::BeginMenu("File")) {
    ImGui::MenuItem("Open result...", nullptr, false, false);
    ImGui::MenuItem("Save session...", nullptr, false, false);
    ImGui::Separator();
    ImGui::MenuItem("Exit", nullptr, false, false);
    ImGui::EndMenu();
  }

  if (ImGui::BeginMenu("View")) {
    ImGui::MenuItem("Output", nullptr, &show_output_panel_);
    ImGui::MenuItem("Status", nullptr, &show_status_panel_);
    ImGui::MenuItem("Scene", nullptr, &show_scene_panel_);
    ImGui::MenuItem("Viewport", nullptr, &show_viewport_placeholder_);
    ImGui::MenuItem("Inspector", nullptr, &show_inspector_panel_);

    ImGui::Separator();
    ImGui::MenuItem("Axes", nullptr, &show_axes_);
    ImGui::MenuItem("Particle traces", nullptr, &show_particle_traces_);
    ImGui::MenuItem("Step points", nullptr, &show_step_points_, false);
    ImGui::MenuItem("Interaction points", nullptr, &show_interaction_points_,
                    false);

    if (ImGui::MenuItem("Reset Camera")) {
      reset_camera_requested_ = true;
    }

    ImGui::EndMenu();
  }

  if (ImGui::BeginMenu("Simulation")) {
    ImGui::MenuItem("Start", nullptr, false, false);
    ImGui::MenuItem("Pause", nullptr, false, false);
    ImGui::MenuItem("Stop", nullptr, false, false);
    ImGui::EndMenu();
  }

  if (ImGui::BeginMenu("Debug")) {
    ImGui::MenuItem("Particle tracking", nullptr, false, false);
    ImGui::MenuItem("Geometry inspection", nullptr, false, false);
    ImGui::EndMenu();
  }

  if (ImGui::BeginMenu("Help")) {
    ImGui::MenuItem("About GGEMS", nullptr, false, false);
    ImGui::EndMenu();
  }

  ImGui::EndMainMenuBar();
}

// -----------------------------------------------------------------------------

void GGEMSImGuiLayer::BuildStatusPanel(vk::Extent2D const &swapchain_extent) {
  ImGui::Begin("GGEMS Status", &show_status_panel_);

  ImGui::TextUnformatted("GuiMode bootstrap");
  ImGui::Separator();

  ImGui::TextUnformatted("Vulkan renderer: initialised");
  ImGui::Text("Swapchain extent: %u x %u", swapchain_extent.width,
              swapchain_extent.height);

  ImGui::Separator();

  ImGui::TextUnformatted("Scene renderer: connected");
  ImGui::Text("Axes: %s", show_axes_ ? "visible" : "hidden");
  ImGui::Text("Particle traces: %s",
              show_particle_traces_ ? "visible" : "hidden");
  ImGui::TextUnformatted("GGEMSWorld: not loaded yet");
  ImGui::TextUnformatted("Output console: connected");

  ImGui::End();
}

// -----------------------------------------------------------------------------

void GGEMSImGuiLayer::BuildViewportPlaceholder(
    ImTextureID scene_texture_id, vk::Extent2D const &scene_texture_extent) {
  ImGuiWindowFlags window_flags =
      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

  viewport_state_.orbit_delta_x_pixels = 0.0f;
  viewport_state_.orbit_delta_y_pixels = 0.0f;
  viewport_state_.pan_delta_x_pixels = 0.0f;
  viewport_state_.pan_delta_y_pixels = 0.0f;
  viewport_state_.zoom_delta = 0.0f;
  viewport_state_.hovered = false;
  viewport_state_.focused = false;

  ImGui::Begin("GGEMS Viewport", &show_viewport_placeholder_, window_flags);

  ImVec2 available_size = ImGui::GetContentRegionAvail();

  std::uint32_t width = available_size.x > 1.0f
                            ? static_cast<std::uint32_t>(available_size.x)
                            : 1U;

  std::uint32_t height = available_size.y > 1.0f
                             ? static_cast<std::uint32_t>(available_size.y)
                             : 1U;

  bool texture_matches_viewport = scene_texture_id != ImTextureID{} &&
                                  scene_texture_extent.width == width &&
                                  scene_texture_extent.height == height;

  viewport_state_.extent = vk::Extent2D{.width = width, .height = height};
  viewport_state_.visible = show_viewport_placeholder_;
  viewport_state_.focused =
      ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

  if (texture_matches_viewport) {
    ImGui::Image(scene_texture_id,
                 ImVec2{static_cast<float>(width), static_cast<float>(height)});

    viewport_state_.hovered = ImGui::IsItemHovered();

    ImGuiIO &io = ImGui::GetIO();

    if (viewport_state_.hovered) {

      if (ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0f)) {
        viewport_state_.orbit_delta_x_pixels = io.MouseDelta.x;
        viewport_state_.orbit_delta_y_pixels = io.MouseDelta.y;
      }

      if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle, 0.0f)) {
        viewport_state_.pan_delta_x_pixels = io.MouseDelta.x;
        viewport_state_.pan_delta_y_pixels = io.MouseDelta.y;
      }

      if (io.MouseWheel != 0.0f) {
        viewport_state_.zoom_delta = io.MouseWheel;
      }

      bool allow_keyboard_pan =
          (viewport_state_.hovered || viewport_state_.focused) &&
          !io.WantTextInput;

      if (allow_keyboard_pan) {
        float keyboard_pan_speed_pixels_per_second = 360.0F;

        if (io.KeyShift) {
          keyboard_pan_speed_pixels_per_second *= 3.0F;
        }

        if (io.KeyCtrl) {
          keyboard_pan_speed_pixels_per_second *= 0.25F;
        }

        float keyboard_pan_delta_pixels =
            keyboard_pan_speed_pixels_per_second * io.DeltaTime;

        if (ImGui::IsKeyDown(ImGuiKey_LeftArrow)) {
          viewport_state_.pan_delta_x_pixels -= keyboard_pan_delta_pixels;
        }

        if (ImGui::IsKeyDown(ImGuiKey_RightArrow)) {
          viewport_state_.pan_delta_x_pixels += keyboard_pan_delta_pixels;
        }

        if (ImGui::IsKeyDown(ImGuiKey_UpArrow)) {
          viewport_state_.pan_delta_y_pixels -= keyboard_pan_delta_pixels;
        }

        if (ImGui::IsKeyDown(ImGuiKey_DownArrow)) {
          viewport_state_.pan_delta_y_pixels += keyboard_pan_delta_pixels;
        }
      }
    }
  } else {
    ImGui::TextDisabled("Vulkan scene renderer: preparing render target...");
    ImGui::TextDisabled("Viewport extent: %u x %u", width, height);

    viewport_state_.hovered =
        ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);
  }

  ImGui::End();
}

// -----------------------------------------------------------------------------

void GGEMSImGuiLayer::BuildDefaultDockspaceLayout(
    ImGuiID dockspace_id, ImVec2 const &dockspace_size) {
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

// -----------------------------------------------------------------------------

void GGEMSImGuiLayer::BuildInspectorPanel() {
  ImGui::Begin("GGEMS Inspector", &show_inspector_panel_);

  ImGui::TextUnformatted("Selection");
  ImGui::Separator();

  if (selected_scene_item_ == SceneSelection::None) {
    ImGui::TextDisabled("No GGEMS object selected yet.");
  } else {
    ImGui::Text("Selected: %s", GetSceneSelectionName());
  }

  ImGui::Spacing();

  if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
    if (selected_scene_item_ == SceneSelection::World) {
      ImGui::TextUnformatted("Position: world origin");
      ImGui::TextUnformatted("Rotation: identity");
      ImGui::TextUnformatted("Scale:    1.0");
    } else {
      ImGui::TextUnformatted("Position: not available yet");
      ImGui::TextUnformatted("Rotation: not available yet");
      ImGui::TextUnformatted("Scale:    not available yet");
    }
  }

  if (ImGui::CollapsingHeader("Geometry", ImGuiTreeNodeFlags_DefaultOpen)) {
    switch (selected_scene_item_) {
    case SceneSelection::World:
      ImGui::TextUnformatted("Type: GGEMSWorld boundary");
      ImGui::TextUnformatted("Bounds: not loaded yet");
      break;
    case SceneSelection::Volumes:
      ImGui::TextUnformatted("Type: geometry collection");
      ImGui::TextUnformatted("Bounds: not available yet");
      break;
    default:
      ImGui::TextUnformatted("Type: not available yet");
      ImGui::TextUnformatted("Bounds: not available yet");
      break;
    }
  }

  if (ImGui::CollapsingHeader("Material")) {
    ImGui::TextUnformatted("Material: not available yet");
    ImGui::TextUnformatted("Density:  not available yet");
  }

  if (ImGui::CollapsingHeader("Physics")) {
    ImGui::TextUnformatted("Processes: not available yet");
    ImGui::TextUnformatted("Cross sections: not available yet");
  }

  ImGui::End();
}

// -----------------------------------------------------------------------------

void GGEMSImGuiLayer::BuildScenePanel() {
  ImGui::Begin("GGEMS Scene", &show_scene_panel_);

  ImGui::TextUnformatted("Scene hierarchy");
  ImGui::Separator();

  BuildSceneNode("GGEMSWorld", SceneSelection::World,
                 ImGuiTreeNodeFlags_DefaultOpen);

  BuildSceneNode("Sources", SceneSelection::Sources,
                 ImGuiTreeNodeFlags_DefaultOpen);

  BuildSceneNode("Volumes", SceneSelection::Volumes,
                 ImGuiTreeNodeFlags_DefaultOpen);

  BuildSceneNode("Materials", SceneSelection::Materials);

  BuildSceneNode("Tracks / Replay", SceneSelection::Tracks);

  ImGui::End();
}

// -----------------------------------------------------------------------------

void GGEMSImGuiLayer::BuildSceneNode(char const *label,
                                     SceneSelection selection,
                                     ImGuiTreeNodeFlags extra_flags) {
  ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow |
                             ImGuiTreeNodeFlags_SpanAvailWidth | extra_flags;

  if (selected_scene_item_ == selection) {
    flags |= ImGuiTreeNodeFlags_Selected;
  }

  bool const opened = ImGui::TreeNodeEx(label, flags);

  if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
    selected_scene_item_ = selection;
  }

  if (opened) {
    switch (selection) {
    case SceneSelection::World:
      ImGui::TextDisabled("World volume: not loaded yet");
      break;
    case SceneSelection::Sources:
      ImGui::TextDisabled("No particle source loaded yet");
      break;
    case SceneSelection::Volumes:
      ImGui::TextDisabled("No geometry volume loaded yet");
      break;
    case SceneSelection::Materials:
      ImGui::TextDisabled("No material table loaded yet");
      break;
    case SceneSelection::Tracks:
      ImGui::TextDisabled(
          "Particle traces can be submitted from observer records.");
      ImGui::TextDisabled("Camera pan: middle mouse or arrow keys.");
      ImGui::TextDisabled("Shift + arrows: faster, Ctrl + arrows: precise.");
      ImGui::Checkbox("Show trajectories", &show_particle_traces_);
      ImGui::Checkbox("Show step points", &show_step_points_);
      ImGui::SameLine();
      ImGui::TextDisabled("soon");
      ImGui::Checkbox("Show interaction points", &show_interaction_points_);
      ImGui::SameLine();
      ImGui::TextDisabled("soon");
      break;
    case SceneSelection::Particles:
      ImGui::TextDisabled("No particle loaded yet");
      break;
    case SceneSelection::Processes:
      ImGui::TextDisabled("No process loaded yet");
      break;
    case SceneSelection::None:
      break;
    }

    ImGui::TreePop();
  }
}

} // namespace ggems::ui
