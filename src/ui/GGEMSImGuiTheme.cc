#include "GGEMSImGuiTheme.hh"

#include "GGEMS/render/GGEMSColourNames.hh"

namespace ggems::ui {

// =============================================================================
// =============================================================================

ImVec4 ToImGuiColour(render::ColourKey const &colour) {
  render::RGB rgb =
      render::GetColourRGB(colour.family, colour.shade, colour.variant);

  constexpr float k_inverse_255{1.0f / 255.0f};

  return ImVec4{static_cast<float>(rgb.r) * k_inverse_255,
                static_cast<float>(rgb.g) * k_inverse_255,
                static_cast<float>(rgb.b) * k_inverse_255, 1.0F};
}

// =============================================================================
// =============================================================================

void ApplyGGEMSImGuiTheme() {
  ImGuiIO &io = ImGui::GetIO();

  io.FontGlobalScale = 1.10F;

  ImGuiStyle &style = ImGui::GetStyle();

  style.WindowPadding = ImVec2{10.0F, 8.0F};
  style.FramePadding = ImVec2{8.0F, 4.0F};
  style.CellPadding = ImVec2{6.0F, 4.0F};
  style.ItemSpacing = ImVec2{8.0F, 6.0F};
  style.ItemInnerSpacing = ImVec2{6.0F, 4.0F};

  style.WindowRounding = 2.0F;
  style.ChildRounding = 2.0F;
  style.FrameRounding = 3.0F;
  style.PopupRounding = 3.0F;
  style.ScrollbarRounding = 3.0F;
  style.GrabRounding = 2.0F;
  style.TabRounding = 3.0F;

  style.WindowBorderSize = 1.0F;
  style.ChildBorderSize = 1.0F;
  style.PopupBorderSize = 1.0F;
  style.FrameBorderSize = 0.0F;
  style.TabBorderSize = 0.0F;

  ImVec4 *colours = style.Colors;

  colours[ImGuiCol_Text] = ToImGuiColour(render::GGEMS_THEME_IMGUI_TEXT);
  colours[ImGuiCol_TextDisabled] =
      ToImGuiColour(render::GGEMS_THEME_IMGUI_TEXT_DISABLED);

  colours[ImGuiCol_WindowBg] =
      ToImGuiColour(render::GGEMS_THEME_IMGUI_WINDOW_BG);
  colours[ImGuiCol_ChildBg] = ToImGuiColour(render::GGEMS_THEME_IMGUI_CHILD_BG);
  colours[ImGuiCol_PopupBg] = ToImGuiColour(render::GGEMS_THEME_IMGUI_POPUP_BG);

  colours[ImGuiCol_Border] = ToImGuiColour(render::GGEMS_THEME_IMGUI_BORDER);
  colours[ImGuiCol_BorderShadow] = ImVec4{0.0F, 0.0F, 0.0F, 0.0F};
  colours[ImGuiCol_Separator] =
      ToImGuiColour(render::GGEMS_THEME_IMGUI_SEPARATOR);

  colours[ImGuiCol_FrameBg] = ToImGuiColour(render::GGEMS_THEME_IMGUI_FRAME_BG);
  colours[ImGuiCol_FrameBgHovered] =
      ToImGuiColour(render::GGEMS_THEME_IMGUI_FRAME_HOVERED);
  colours[ImGuiCol_FrameBgActive] =
      ToImGuiColour(render::GGEMS_THEME_IMGUI_FRAME_ACTIVE);

  colours[ImGuiCol_TitleBg] =
      ToImGuiColour(render::GGEMS_THEME_IMGUI_WINDOW_BG);
  colours[ImGuiCol_TitleBgActive] =
      ToImGuiColour(render::GGEMS_THEME_IMGUI_FRAME_BG);
  colours[ImGuiCol_TitleBgCollapsed] =
      ToImGuiColour(render::GGEMS_THEME_IMGUI_WINDOW_BG);

  colours[ImGuiCol_MenuBarBg] =
      ToImGuiColour(render::GGEMS_THEME_IMGUI_FRAME_BG);

  colours[ImGuiCol_Button] = ToImGuiColour(render::GGEMS_THEME_IMGUI_BUTTON);
  colours[ImGuiCol_ButtonHovered] =
      ToImGuiColour(render::GGEMS_THEME_IMGUI_BUTTON_HOVERED);
  colours[ImGuiCol_ButtonActive] =
      ToImGuiColour(render::GGEMS_THEME_IMGUI_BUTTON_ACTIVE);

  colours[ImGuiCol_Header] = ToImGuiColour(render::GGEMS_THEME_IMGUI_HEADER);
  colours[ImGuiCol_HeaderHovered] =
      ToImGuiColour(render::GGEMS_THEME_IMGUI_HEADER_HOVERED);
  colours[ImGuiCol_HeaderActive] =
      ToImGuiColour(render::GGEMS_THEME_IMGUI_HEADER_ACTIVE);

  colours[ImGuiCol_Tab] = ToImGuiColour(render::GGEMS_THEME_IMGUI_TAB);
  colours[ImGuiCol_TabHovered] =
      ToImGuiColour(render::GGEMS_THEME_IMGUI_TAB_HOVERED);
  colours[ImGuiCol_TabSelected] =
      ToImGuiColour(render::GGEMS_THEME_IMGUI_TAB_ACTIVE);
  colours[ImGuiCol_TabDimmed] = ToImGuiColour(render::GGEMS_THEME_IMGUI_TAB);
  colours[ImGuiCol_TabDimmedSelected] =
      ToImGuiColour(render::GGEMS_THEME_IMGUI_TAB_ACTIVE);
  colours[ImGuiCol_TabSelectedOverline] =
      ToImGuiColour(render::GGEMS_THEME_IMGUI_TAB_ACTIVE_OVERLINE);
  colours[ImGuiCol_TabDimmedSelectedOverline] =
      ToImGuiColour(render::GGEMS_THEME_IMGUI_TAB_ACTIVE_OVERLINE);

  colours[ImGuiCol_ScrollbarBg] =
      ToImGuiColour(render::GGEMS_THEME_IMGUI_SCROLLBAR_BG);
  colours[ImGuiCol_ScrollbarGrab] =
      ToImGuiColour(render::GGEMS_THEME_IMGUI_SCROLLBAR_GRAB);
  colours[ImGuiCol_ScrollbarGrabHovered] =
      ToImGuiColour(render::GGEMS_THEME_IMGUI_SCROLLBAR_GRAB_HOVERED);
  colours[ImGuiCol_ScrollbarGrabActive] =
      ToImGuiColour(render::GGEMS_THEME_IMGUI_SCROLLBAR_GRAB_ACTIVE);

  colours[ImGuiCol_CheckMark] =
      ToImGuiColour(render::GGEMS_THEME_IMGUI_CHECK_MARK);
  colours[ImGuiCol_SliderGrab] =
      ToImGuiColour(render::GGEMS_THEME_IMGUI_SLIDER_GRAB);
  colours[ImGuiCol_SliderGrabActive] =
      ToImGuiColour(render::GGEMS_THEME_IMGUI_BUTTON_ACTIVE);

  colours[ImGuiCol_ResizeGrip] =
      ToImGuiColour(render::GGEMS_THEME_IMGUI_RESIZE_GRIP);
  colours[ImGuiCol_ResizeGripHovered] =
      ToImGuiColour(render::GGEMS_THEME_IMGUI_BUTTON_HOVERED);
  colours[ImGuiCol_ResizeGripActive] =
      ToImGuiColour(render::GGEMS_THEME_IMGUI_BUTTON_ACTIVE);

  colours[ImGuiCol_DockingPreview] =
      ToImGuiColour(render::GGEMS_THEME_IMGUI_BUTTON_ACTIVE);
  colours[ImGuiCol_DockingEmptyBg] =
      ToImGuiColour(render::GGEMS_THEME_IMGUI_WINDOW_BG);
}

} // namespace ggems::ui
