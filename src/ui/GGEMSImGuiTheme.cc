#include <imgui.h>

#include "GGEMS/ui/GGEMSImGuiTheme.hh"
#include "GGEMS/render/GGEMSColour.hh"
#include "GGEMS/render/GGEMSColourNames.hh"

namespace ggems::ui {

// =============================================================================
// =============================================================================

auto ToImGuiColour(render::ColourKey const &colour) -> ImVec4 {
  render::RGB rgb =
      render::GetColourRGB(colour.family, colour.shade, colour.variant);

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

  ImVec4 *colours = style.Colors;

  ImVec4 const transparent{0.0F, 0.0F, 0.0F, 0.0F};

  auto const WithAlpha = [](ImVec4 colour, float const alpha) -> ImVec4 {
    colour.w = alpha;
    return colour;
  };

  ImVec4 const gunmetal = ToImGuiColour(render::BLUE_Gunmetal);
  ImVec4 const gunmetal_soft = WithAlpha(gunmetal, 0.38F);
  ImVec4 const gunmetal_hovered = WithAlpha(gunmetal, 0.72F);
  ImVec4 const gunmetal_faint = WithAlpha(gunmetal, 0.20F);

  ImVec4 const cryo = ToImGuiColour(render::CYAN_Cryo);
  ImVec4 const cryo_soft = WithAlpha(cryo, 0.60F);

  ImVec4 const amber = ToImGuiColour(render::YELLOW_MotherAmber);

  // Main surfaces
  colours[ImGuiCol_WindowBg] = ToImGuiColour(render::BLUE_Abyss);
  colours[ImGuiCol_ChildBg] = transparent;
  colours[ImGuiCol_PopupBg] = ToImGuiColour(render::BLUE_Abyss);
  colours[ImGuiCol_Border] = ToImGuiColour(render::BLUE_Gunmetal);
  colours[ImGuiCol_BorderShadow] = transparent;

  // Title bars
  colours[ImGuiCol_TitleBg] = ToImGuiColour(render::BLUE_Abyss);
  colours[ImGuiCol_TitleBgActive] = gunmetal;
  colours[ImGuiCol_TitleBgCollapsed] = ToImGuiColour(render::BLUE_Abyss);

  // Text
  colours[ImGuiCol_Text] = ToImGuiColour(render::WHITE_Bone);
  colours[ImGuiCol_TextDisabled] = ToImGuiColour(render::GREY_Concrete);
  colours[ImGuiCol_TextLink] = cryo;
  colours[ImGuiCol_TextSelectedBg] = WithAlpha(gunmetal, 0.65F);
  colours[ImGuiCol_InputTextCursor] = cryo;

  // Menu bar
  colours[ImGuiCol_MenuBarBg] = ToImGuiColour(render::BLUE_Abyss);

  // Tabs
  colours[ImGuiCol_Tab] = ToImGuiColour(render::GREY_Deep);
  colours[ImGuiCol_TabHovered] = gunmetal_hovered;
  colours[ImGuiCol_TabSelected] = gunmetal;
  colours[ImGuiCol_TabSelectedOverline] = cryo_soft;
  colours[ImGuiCol_TabDimmed] = ToImGuiColour(render::GREY_Void);
  colours[ImGuiCol_TabDimmedSelected] = ToImGuiColour(render::BLUE_Abyss);
  colours[ImGuiCol_TabDimmedSelectedOverline] = gunmetal_soft;

  // Scrollbars
  colours[ImGuiCol_ScrollbarBg] = ToImGuiColour(render::GREY_Void);
  colours[ImGuiCol_ScrollbarGrab] = gunmetal_soft;
  colours[ImGuiCol_ScrollbarGrabHovered] = gunmetal_hovered;
  colours[ImGuiCol_ScrollbarGrabActive] = gunmetal;

  // Frames: checkbox, radio button, input, plot backgrounds...
  colours[ImGuiCol_FrameBg] = ToImGuiColour(render::BLUE_Abyss);
  colours[ImGuiCol_FrameBgHovered] = gunmetal_hovered;
  colours[ImGuiCol_FrameBgActive] = gunmetal;

  // Headers: tree nodes, collapsing headers, selectable rows...
  colours[ImGuiCol_Header] = ToImGuiColour(render::BLUE_Abyss);
  colours[ImGuiCol_HeaderHovered] = gunmetal_hovered;
  colours[ImGuiCol_HeaderActive] = gunmetal;

  // Separators
  colours[ImGuiCol_Separator] = ToImGuiColour(render::BLUE_Gunmetal);
  colours[ImGuiCol_SeparatorHovered] = ToImGuiColour(render::CYAN_Cryo);
  colours[ImGuiCol_SeparatorActive] = ToImGuiColour(render::CYAN_Cryo);

  // Checkboxes and sliders
  colours[ImGuiCol_CheckMark] = ToImGuiColour(render::WHITE_Bone);
  colours[ImGuiCol_CheckboxSelectedBg] = gunmetal;
  colours[ImGuiCol_SliderGrab] = WithAlpha(gunmetal, 0.85F);
  colours[ImGuiCol_SliderGrabActive] = WithAlpha(cryo, 0.85F);

  // Buttons
  colours[ImGuiCol_Button] = ToImGuiColour(render::BLUE_Abyss);
  colours[ImGuiCol_ButtonHovered] = gunmetal_hovered;
  colours[ImGuiCol_ButtonActive] = gunmetal;

  // Resize grips
  colours[ImGuiCol_ResizeGrip] = WithAlpha(gunmetal, 0.18F);
  colours[ImGuiCol_ResizeGripHovered] = WithAlpha(gunmetal, 0.55F);
  colours[ImGuiCol_ResizeGripActive] = gunmetal;

  // Docking
  colours[ImGuiCol_DockingPreview] = WithAlpha(gunmetal, 0.45F);
  colours[ImGuiCol_DockingEmptyBg] = ToImGuiColour(render::BLUE_Abyss);

  // Plots
  colours[ImGuiCol_PlotLines] = cryo;
  colours[ImGuiCol_PlotLinesHovered] = ToImGuiColour(render::CYAN_Cryo_B);
  colours[ImGuiCol_PlotHistogram] = amber;
  colours[ImGuiCol_PlotHistogramHovered] =
      ToImGuiColour(render::YELLOW_MotherAmber_B);

  // Tables
  colours[ImGuiCol_TableHeaderBg] = ToImGuiColour(render::BLUE_Abyss);
  colours[ImGuiCol_TableBorderStrong] = WithAlpha(gunmetal, 0.48F);
  colours[ImGuiCol_TableBorderLight] = gunmetal_faint;
  colours[ImGuiCol_TableRowBg] = transparent;
  colours[ImGuiCol_TableRowBgAlt] =
      WithAlpha(ToImGuiColour(render::BLUE_Abyss), 0.45F);

  // Trees
  colours[ImGuiCol_TreeLines] = gunmetal_soft;

  // Drag and drop
  colours[ImGuiCol_DragDropTarget] = amber;
  colours[ImGuiCol_DragDropTargetBg] = WithAlpha(amber, 0.16F);

  // Markers
  colours[ImGuiCol_UnsavedMarker] = amber;

  // Navigation
  colours[ImGuiCol_NavCursor] = WithAlpha(cryo, 0.85F);
  colours[ImGuiCol_NavWindowingHighlight] = WithAlpha(cryo, 0.55F);
  colours[ImGuiCol_NavWindowingDimBg] =
      WithAlpha(ToImGuiColour(render::GREY_Void), 0.65F);
  colours[ImGuiCol_ModalWindowDimBg] =
      WithAlpha(ToImGuiColour(render::GREY_Void), 0.80F);
}

} // namespace ggems::ui
