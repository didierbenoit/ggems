#include <algorithm>
#include <array>

#include <imgui.h>
#include <imgui_internal.h>

#include "GGEMS/ui/detail/GGEMSImGuiLayout.hh"

namespace {

// =============================================================================
// =============================================================================

constexpr std::array<char const *, 5U> k_layout_window_names{
  ggems::ui::detail::k_output_window_name,
  ggems::ui::detail::k_status_window_name,
  ggems::ui::detail::k_scene_window_name,
  ggems::ui::detail::k_viewport_window_name,
  ggems::ui::detail::k_inspector_window_name,
};

// =============================================================================
// =============================================================================

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

} // namespace

namespace ggems::ui::detail {

// =============================================================================
// =============================================================================

auto GetMainDockspaceID() -> ImGuiID {
  return ImHashStr("GGEMS.MainDockspaceNode");
}

// -----------------------------------------------------------------------------

auto HasSavedLayout() -> bool {
  if (ImGui::DockBuilderGetNode(GetMainDockspaceID()) != nullptr) {
    return true;
  }

  return std::ranges::any_of(k_layout_window_names, IsUsableWindowSettings);
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
  ImGui::DockBuilderDockWindow(k_scene_window_name, dock_left_id);
  ImGui::DockBuilderDockWindow(k_viewport_window_name, dockspace_main_id);
  ImGui::DockBuilderDockWindow(k_inspector_window_name, dock_right_id);

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
