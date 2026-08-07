#include <imgui.h>

#include "GGEMS/ui/GGEMSImGuiTheme.hh"
#include "GGEMS/render/GGEMSColor.hh"
#include "GGEMS/render/GGEMSColorNames.hh"

namespace ggems::ui {

// =============================================================================
// =============================================================================

auto ToImGuiColor(render::ColorKey const &color) -> ImVec4 {
  render::RGB rgb =
      render::GetColorRGB(color.family, color.shade, color.variant);

  constexpr float k_inverse_255{1.0F / 255.0F};

  return ImVec4{static_cast<float>(rgb.r) * k_inverse_255,
                static_cast<float>(rgb.g) * k_inverse_255,
                static_cast<float>(rgb.b) * k_inverse_255, 1.0F};
}

// =============================================================================
// =============================================================================

void ApplyGGEMSImGuiTheme() {
  ImGuiIO &imgui_io = ImGui::GetIO();

  imgui_io.FontGlobalScale = 1.10F;

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

  ImVec4 *colors = style.Colors;

  ImVec4 const transparent{0.0F, 0.0F, 0.0F, 0.0F};

  auto const WithAlpha = [](ImVec4 color, float const alpha) -> ImVec4 {
    color.w = alpha;
    return color;
  };

  ImVec4 const gunmetal = ToImGuiColor(render::BLUE_Gunmetal);
  ImVec4 const gunmetal_soft = WithAlpha(gunmetal, 0.38F);
  ImVec4 const gunmetal_hovered = WithAlpha(gunmetal, 0.72F);
  ImVec4 const gunmetal_faint = WithAlpha(gunmetal, 0.20F);

  ImVec4 const cryo = ToImGuiColor(render::CYAN_Cryo);
  ImVec4 const cryo_soft = WithAlpha(cryo, 0.60F);

  ImVec4 const amber = ToImGuiColor(render::YELLOW_MotherAmber);

  // Main surfaces
  colors[ImGuiCol_WindowBg] = ToImGuiColor(render::BLUE_Abyss);
  colors[ImGuiCol_ChildBg] = transparent;
  colors[ImGuiCol_PopupBg] = ToImGuiColor(render::BLUE_Abyss);
  colors[ImGuiCol_Border] = ToImGuiColor(render::BLUE_Gunmetal);
  colors[ImGuiCol_BorderShadow] = transparent;

  // Title bars
  colors[ImGuiCol_TitleBg] = ToImGuiColor(render::BLUE_Abyss);
  colors[ImGuiCol_TitleBgActive] = gunmetal;
  colors[ImGuiCol_TitleBgCollapsed] = ToImGuiColor(render::BLUE_Abyss);

  // Text
  colors[ImGuiCol_Text] = ToImGuiColor(render::WHITE_Bone);
  colors[ImGuiCol_TextDisabled] = ToImGuiColor(render::GRAY_Concrete);
  colors[ImGuiCol_TextLink] = cryo;
  colors[ImGuiCol_TextSelectedBg] = WithAlpha(gunmetal, 0.65F);
  colors[ImGuiCol_InputTextCursor] = cryo;

  // Menu bar
  colors[ImGuiCol_MenuBarBg] = ToImGuiColor(render::BLUE_Abyss);

  // Tabs
  colors[ImGuiCol_Tab] = ToImGuiColor(render::GRAY_Deep);
  colors[ImGuiCol_TabHovered] = gunmetal_hovered;
  colors[ImGuiCol_TabSelected] = gunmetal;
  colors[ImGuiCol_TabSelectedOverline] = cryo_soft;
  colors[ImGuiCol_TabDimmed] = ToImGuiColor(render::GRAY_Void);
  colors[ImGuiCol_TabDimmedSelected] = ToImGuiColor(render::BLUE_Abyss);
  colors[ImGuiCol_TabDimmedSelectedOverline] = gunmetal_soft;

  // Scrollbars
  colors[ImGuiCol_ScrollbarBg] = ToImGuiColor(render::GRAY_Void);
  colors[ImGuiCol_ScrollbarGrab] = gunmetal_soft;
  colors[ImGuiCol_ScrollbarGrabHovered] = gunmetal_hovered;
  colors[ImGuiCol_ScrollbarGrabActive] = gunmetal;

  // Frames: checkbox, radio button, input, plot backgrounds...
  colors[ImGuiCol_FrameBg] = ToImGuiColor(render::BLUE_Abyss);
  colors[ImGuiCol_FrameBgHovered] = gunmetal_hovered;
  colors[ImGuiCol_FrameBgActive] = gunmetal;

  // Headers: tree nodes, collapsing headers, selectable rows...
  colors[ImGuiCol_Header] = ToImGuiColor(render::BLUE_Abyss);
  colors[ImGuiCol_HeaderHovered] = gunmetal_hovered;
  colors[ImGuiCol_HeaderActive] = gunmetal;

  // Separators
  colors[ImGuiCol_Separator] = ToImGuiColor(render::BLUE_Gunmetal);
  colors[ImGuiCol_SeparatorHovered] = ToImGuiColor(render::CYAN_Cryo);
  colors[ImGuiCol_SeparatorActive] = ToImGuiColor(render::CYAN_Cryo);

  // Checkboxes and sliders
  colors[ImGuiCol_CheckMark] = ToImGuiColor(render::WHITE_Bone);
  colors[ImGuiCol_CheckboxSelectedBg] = gunmetal;
  colors[ImGuiCol_SliderGrab] = WithAlpha(gunmetal, 0.85F);
  colors[ImGuiCol_SliderGrabActive] = WithAlpha(cryo, 0.85F);

  // Buttons
  colors[ImGuiCol_Button] = ToImGuiColor(render::BLUE_Abyss);
  colors[ImGuiCol_ButtonHovered] = gunmetal_hovered;
  colors[ImGuiCol_ButtonActive] = gunmetal;

  // Resize grips
  colors[ImGuiCol_ResizeGrip] = WithAlpha(gunmetal, 0.18F);
  colors[ImGuiCol_ResizeGripHovered] = WithAlpha(gunmetal, 0.55F);
  colors[ImGuiCol_ResizeGripActive] = gunmetal;

  // Docking
  colors[ImGuiCol_DockingPreview] = WithAlpha(gunmetal, 0.45F);
  colors[ImGuiCol_DockingEmptyBg] = ToImGuiColor(render::BLUE_Abyss);

  // Plots
  colors[ImGuiCol_PlotLines] = cryo;
  colors[ImGuiCol_PlotLinesHovered] = ToImGuiColor(render::CYAN_Cryo_B);
  colors[ImGuiCol_PlotHistogram] = amber;
  colors[ImGuiCol_PlotHistogramHovered] =
      ToImGuiColor(render::YELLOW_MotherAmber_B);

  // Tables
  colors[ImGuiCol_TableHeaderBg] = ToImGuiColor(render::BLUE_Abyss);
  colors[ImGuiCol_TableBorderStrong] = WithAlpha(gunmetal, 0.48F);
  colors[ImGuiCol_TableBorderLight] = gunmetal_faint;
  colors[ImGuiCol_TableRowBg] = transparent;
  colors[ImGuiCol_TableRowBgAlt] =
      WithAlpha(ToImGuiColor(render::BLUE_Abyss), 0.45F);

  // Trees
  colors[ImGuiCol_TreeLines] = gunmetal_soft;

  // Drag and drop
  colors[ImGuiCol_DragDropTarget] = amber;
  colors[ImGuiCol_DragDropTargetBg] = WithAlpha(amber, 0.16F);

  // Markers
  colors[ImGuiCol_UnsavedMarker] = amber;

  // Navigation
  colors[ImGuiCol_NavCursor] = WithAlpha(cryo, 0.85F);
  colors[ImGuiCol_NavWindowingHighlight] = WithAlpha(cryo, 0.55F);
  colors[ImGuiCol_NavWindowingDimBg] =
      WithAlpha(ToImGuiColor(render::GRAY_Void), 0.65F);
  colors[ImGuiCol_ModalWindowDimBg] =
      WithAlpha(ToImGuiColor(render::GRAY_Void), 0.80F);
}

} // namespace ggems::ui
