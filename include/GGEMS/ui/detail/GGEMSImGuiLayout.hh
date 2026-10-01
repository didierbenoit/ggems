#pragma once

#include <imgui.h>

namespace ggems::ui::detail {

inline constexpr char const *k_main_dockspace_window_name =
  "GGEMS Main Dockspace###GGEMS.MainDockspace";

inline constexpr char const *k_output_window_name =
  "GGEMS Output###GGEMS.Output";

inline constexpr char const *k_status_window_name =
  "GGEMS Status###GGEMS.Status";

inline constexpr char const *k_scene_window_name = "GGEMS Scene###GGEMS.Scene";

inline constexpr char const *k_viewport_window_name =
  "GGEMS Viewport###GGEMS.Viewport";

inline constexpr char const *k_inspector_window_name =
  "GGEMS Inspector###GGEMS.Inspector";

[[nodiscard]] auto GetMainDockspaceID() -> ImGuiID;

[[nodiscard]] auto HasSavedLayout() -> bool;

auto BuildDefaultLayout(ImVec2 const &dockspace_size) -> void;

auto ResetLayout(ImVec2 const &dockspace_size) -> void;

} // namespace ggems::ui::detail
