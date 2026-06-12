#include <cmath>
#include <algorithm>

#include <imgui.h>
#include <imgui_internal.h>

#include "GGEMSImGuiLayer.hh"
#include "GGEMS/core/GGEMSOutputMode.hh"

namespace ggems::ui {

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

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

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSImGuiLayer::BuildFrame(vk::Extent2D const &swapchain_extent) {
  BuildMainDockspace();

  if (show_output_panel_) {
    render::GGEMSBanner &banner = core::GetOutputBanner();
    core::GGEMSOutputState &output_state = core::GetOutputState();
    render::GGEMSProgressBar &progress_bar = core::GetProgressBar();

    output_panel_.Render(banner, output_state, progress_bar);
  }

  if (show_status_panel_) {
    BuildStatusPanel(swapchain_extent);
  }

  if (show_scene_panel_) {
    BuildScenePanel();
  }

  if (show_viewport_placeholder_) {
    BuildViewportPlaceholder();
  }

  if (show_inspector_panel_) {
    BuildInspectorPanel();
  }
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

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

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

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

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSImGuiLayer::BuildStatusPanel(vk::Extent2D const &swapchain_extent) {
  ImGui::Begin("GGEMS Status", &show_status_panel_);

  ImGui::TextUnformatted("GuiMode bootstrap");
  ImGui::Separator();

  ImGui::TextUnformatted("Vulkan renderer: initialised");
  ImGui::Text("Swapchain extent: %u x %u", swapchain_extent.width,
              swapchain_extent.height);

  ImGui::Separator();

  ImGui::TextUnformatted("Scene renderer: not connected yet");
  ImGui::TextUnformatted("GGEMSWorld: not loaded yet");
  ImGui::TextUnformatted("Output console: connected");

  ImGui::End();
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSImGuiLayer::BuildViewportPlaceholder() {
  ImGui::Begin("GGEMS Viewport", &show_viewport_placeholder_);

  ImGui::TextUnformatted("Future Vulkan scene viewport");
  ImGui::Separator();

  ImGui::TextDisabled("Vulkan scene renderer: not connected yet");
  ImGui::TextDisabled("This panel will receive a Vulkan-rendered image.");

  ImGui::End();
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

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

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

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

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

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

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

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
      ImGui::TextDisabled("No particle track loaded yet");
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
