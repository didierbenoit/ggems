#include <imgui.h>

#include "GGEMSImGuiLayer.hh"
#include "GGEMS/core/GGEMSOutputMode.hh"

namespace ggems::ui {

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSImGuiLayer::BuildFrame(vk::Extent2D const &swapchain_extent) {
  BuildMainDockspace();
  BuildMainMenuBar();

  if (show_output_panel_) {
    core::GGEMSOutputState &output_state = core::GetOutputState();
    output_panel_.Render(output_state);
  }

  if (show_status_panel_) {
    BuildStatusPanel(swapchain_extent);
  }

  if (show_viewport_placeholder_) {
    BuildViewportPlaceholder();
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

  ImGui::Begin("GGEMS Main Dockspace", nullptr, window_flags);

  ImGui::PopStyleVar(3);

  ImGuiID const dockspace_id = ImGui::GetID("GGEMS_Dockspace");

  ImGuiDockNodeFlags const dockspace_flags =
      ImGuiDockNodeFlags_PassthruCentralNode;

  ImGui::DockSpace(dockspace_id, ImVec2{0.0F, 0.0F}, dockspace_flags);

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
    ImGui::MenuItem("Viewport", nullptr, &show_viewport_placeholder_);
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
  ImGui::TextUnformatted("Here we will display:");
  ImGui::BulletText("GGEMSWorld");
  ImGui::BulletText("global axes");
  ImGui::BulletText("geometry inspection");
  ImGui::BulletText("particle tracking / replay");

  ImGui::End();
}

} // namespace ggems::ui
