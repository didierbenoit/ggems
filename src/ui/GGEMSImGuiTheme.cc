#include <imgui.h>

#include "GGEMS/ui/detail/GGEMSImGuiTheme.hh"
#include "GGEMS/render/GGEMSColor.hh"
#include "GGEMS/render/GGEMSColorTypes.hh"
#include "GGEMS/render/GGEMSColorNames.hh"

namespace {

using ggems::ui::GetThemeColor;
using ggems::ui::GGEMSThemeRole;

// =============================================================================
// =============================================================================

[[nodiscard]] auto WithAlpha(ImVec4 color, float alpha) noexcept -> ImVec4 {
  color.w = alpha;
  return color;
}

// =============================================================================
// =============================================================================

auto ApplyGGEMSMetrics(ImGuiStyle &style) -> void {
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

  style.FontSizeBase = ggems::ui::k_ggems_base_font_size;
}

// =============================================================================
// =============================================================================

auto ApplyGGEMSColors(ImGuiStyle &style) -> void {
  ImVec4 *colors = style.Colors;

  ImVec4 const transparent{0.0F, 0.0F, 0.0F, 0.0F};

  ImVec4 const window = GetThemeColor(GGEMSThemeRole::WindowBackground);
  ImVec4 const recessed = GetThemeColor(GGEMSThemeRole::RecessedBackground);
  ImVec4 const selection = GetThemeColor(GGEMSThemeRole::Selection);
  ImVec4 const selection_soft = WithAlpha(selection, 0.38F);
  ImVec4 const selection_hovered = WithAlpha(selection, 0.72F);
  ImVec4 const selection_faint = WithAlpha(selection, 0.20F);
  ImVec4 const accent = GetThemeColor(GGEMSThemeRole::Accent);
  ImVec4 const accent_soft = WithAlpha(accent, 0.60F);
  ImVec4 const attention = GetThemeColor(GGEMSThemeRole::Attention);

  // Main surfaces
  colors[ImGuiCol_WindowBg] = window;
  colors[ImGuiCol_ChildBg] = transparent;
  colors[ImGuiCol_PopupBg] = window;
  colors[ImGuiCol_Border] = selection;
  colors[ImGuiCol_BorderShadow] = transparent;

  // Title bars
  colors[ImGuiCol_TitleBg] = window;
  colors[ImGuiCol_TitleBgActive] = selection;
  colors[ImGuiCol_TitleBgCollapsed] = window;

  // Text
  colors[ImGuiCol_Text] = GetThemeColor(GGEMSThemeRole::PrimaryText);
  colors[ImGuiCol_TextDisabled] = GetThemeColor(GGEMSThemeRole::MutedText);
  colors[ImGuiCol_TextLink] = accent;
  colors[ImGuiCol_TextSelectedBg] = WithAlpha(selection, 0.65F);
  colors[ImGuiCol_InputTextCursor] = accent;

  // Menu bar
  colors[ImGuiCol_MenuBarBg] = window;

  // Tabs
  colors[ImGuiCol_Tab] = GetThemeColor(GGEMSThemeRole::TabBackground);
  colors[ImGuiCol_TabHovered] = selection_hovered;
  colors[ImGuiCol_TabSelected] = selection;
  colors[ImGuiCol_TabSelectedOverline] = accent_soft;
  colors[ImGuiCol_TabDimmed] = recessed;
  colors[ImGuiCol_TabDimmedSelected] = window;
  colors[ImGuiCol_TabDimmedSelectedOverline] = selection_soft;

  // Scrollbars
  colors[ImGuiCol_ScrollbarBg] = recessed;
  colors[ImGuiCol_ScrollbarGrab] = selection_soft;
  colors[ImGuiCol_ScrollbarGrabHovered] = selection_hovered;
  colors[ImGuiCol_ScrollbarGrabActive] = selection;

  // Frames: checkbox, radio button, input, plot backgrounds...
  colors[ImGuiCol_FrameBg] = window;
  colors[ImGuiCol_FrameBgHovered] = selection_hovered;
  colors[ImGuiCol_FrameBgActive] = selection;

  // Headers: tree nodes, collapsing headers, selectable rows...
  colors[ImGuiCol_Header] = window;
  colors[ImGuiCol_HeaderHovered] = selection_hovered;
  colors[ImGuiCol_HeaderActive] = selection;

  // Separators
  colors[ImGuiCol_Separator] = selection;
  colors[ImGuiCol_SeparatorHovered] = accent;
  colors[ImGuiCol_SeparatorActive] = accent;

  // Checkboxes and sliders
  colors[ImGuiCol_CheckMark] = GetThemeColor(GGEMSThemeRole::PrimaryText);
  colors[ImGuiCol_CheckboxSelectedBg] = selection;
  colors[ImGuiCol_SliderGrab] = WithAlpha(selection, 0.85F);
  colors[ImGuiCol_SliderGrabActive] = WithAlpha(accent, 0.85F);

  // Buttons
  colors[ImGuiCol_Button] = window;
  colors[ImGuiCol_ButtonHovered] = selection_hovered;
  colors[ImGuiCol_ButtonActive] = selection;

  // Resize grips
  colors[ImGuiCol_ResizeGrip] = WithAlpha(selection, 0.18F);
  colors[ImGuiCol_ResizeGripHovered] = WithAlpha(selection, 0.55F);
  colors[ImGuiCol_ResizeGripActive] = selection;

  // Docking
  colors[ImGuiCol_DockingPreview] = WithAlpha(selection, 0.45F);
  colors[ImGuiCol_DockingEmptyBg] = window;

  // Plots
  colors[ImGuiCol_PlotLines] = accent;
  colors[ImGuiCol_PlotLinesHovered] =
    GetThemeColor(GGEMSThemeRole::AccentStrong);
  colors[ImGuiCol_PlotHistogram] = attention;
  colors[ImGuiCol_PlotHistogramHovered] =
    GetThemeColor(GGEMSThemeRole::AttentionStrong);

  // Tables
  colors[ImGuiCol_TableHeaderBg] = window;
  colors[ImGuiCol_TableBorderStrong] = WithAlpha(selection, 0.48F);
  colors[ImGuiCol_TableBorderLight] = selection_faint;
  colors[ImGuiCol_TableRowBg] = transparent;
  colors[ImGuiCol_TableRowBgAlt] = WithAlpha(window, 0.45F);

  // Trees
  colors[ImGuiCol_TreeLines] = selection_soft;

  // Drag and drop
  colors[ImGuiCol_DragDropTarget] = attention;
  colors[ImGuiCol_DragDropTargetBg] = WithAlpha(attention, 0.16F);

  // Markers
  colors[ImGuiCol_UnsavedMarker] = attention;

  // Navigation
  colors[ImGuiCol_NavCursor] = WithAlpha(accent, 0.85F);
  colors[ImGuiCol_NavWindowingHighlight] = WithAlpha(accent, 0.55F);
  colors[ImGuiCol_NavWindowingDimBg] = WithAlpha(recessed, 0.65F);
  colors[ImGuiCol_ModalWindowDimBg] = WithAlpha(recessed, 0.80F);
}

} // namespace

namespace ggems::ui {

// =============================================================================
// =============================================================================

auto GetThemeColorKey(GGEMSThemeRole role) noexcept -> render::ColorKey {
  switch (role) {
  case GGEMSThemeRole::WindowBackground:
  case GGEMSThemeRole::SceneBackground:
    return render::BLUE_Abyss;
  case GGEMSThemeRole::RecessedBackground:
    return render::GRAY_Void;
  case GGEMSThemeRole::Selection:
    return render::BLUE_Gunmetal;
  case GGEMSThemeRole::TabBackground:
    return render::GRAY_Deep;
  case GGEMSThemeRole::PrimaryText:
    return render::WHITE_Bone;
  case GGEMSThemeRole::MutedText:
    return render::GRAY_Concrete;
  case GGEMSThemeRole::Accent:
    return render::CYAN_Cryo;
  case GGEMSThemeRole::AccentStrong:
    return render::CYAN_Cryo_B;
  case GGEMSThemeRole::Attention:
    return render::YELLOW_MotherAmber;
  case GGEMSThemeRole::AttentionStrong:
    return render::YELLOW_MotherAmber_B;
  }

  return render::WHITE_Bone;
}

// -----------------------------------------------------------------------------

auto GetThemeColor(GGEMSThemeRole role) noexcept -> ImVec4 {
  return ToImGuiColor(GetThemeColorKey(role));
}

// -----------------------------------------------------------------------------

auto ToImGuiColor(render::ColorKey const &color) noexcept -> ImVec4 {
  render::RGB rgb =
    render::GetColorRGB(color.family, color.shade, color.variant);

  constexpr float k_inverse_255{1.0F / 255.0F};

  return ImVec4{static_cast<float>(rgb.red) * k_inverse_255,
                static_cast<float>(rgb.green) * k_inverse_255,
                static_cast<float>(rgb.blue) * k_inverse_255, 1.0F};
}

// -----------------------------------------------------------------------------

auto BuildGGEMSStyle(float user_scale, float content_scale) -> ImGuiStyle {
  ImGuiStyle style{};

  ImGui::StyleColorsDark(&style);
  ApplyGGEMSMetrics(style);
  ApplyGGEMSColors(style);

  style.ScaleAllSizes(user_scale * content_scale);
  style.FontScaleMain = user_scale;
  style.FontScaleDpi = content_scale;

  return style;
}

} // namespace ggems::ui
