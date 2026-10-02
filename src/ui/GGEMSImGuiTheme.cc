// *****************************************************************************
// * This file is part of GGEMS.                                               *
// *                                                                           *
// * SPDX-License-Identifier: GPL-3.0-or-later                                 *
// * Copyright (C) 2017-2026 CHRU de Brest, Université de Bretagne Occidentale,*
// * Inserm.                                                                   *
// *                                                                           *
// * GGEMS is free software: you can redistribute it and/or modify             *
// * it under the terms of the GNU General Public License as published by      *
// * the Free Software Foundation, either version 3 of the License, or         *
// * (at your option) any later version.                                       *
// *                                                                           *
// * GGEMS is distributed in the hope that it will be useful,                  *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
// * GNU General Public License for more details.                              *
// *                                                                           *
// * You should have received a copy of the GNU General Public License         *
// * along with GGEMS. If not, see <https://www.gnu.org/licenses/>.            *
// *****************************************************************************

/*!
 * \file
 * \brief Builds the GGEMS linear-color Dear ImGui style and scaled metrics.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

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

/*!
 * \brief Replaces the alpha channel of a presentation color.
 *
 * \param[in] color Linear RGBA color to copy.
 * \param[in] alpha Replacement opacity.
 * \return Color with the requested alpha and unchanged RGB channels.
 */
[[nodiscard]] auto WithAlpha(ImVec4 color, float alpha) noexcept -> ImVec4 {
  color.w = alpha;
  return color;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Sets the unscaled GGEMS spacing, borders, rounding, and font size.
 *
 * \param[in,out] style Style whose geometry and font metrics are updated.
 */
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

/*!
 * \brief Applies semantic GGEMS colors to Dear ImGui style slots.
 *
 * \param[in,out] style Style whose color table is updated.
 */
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
  ImVec4 const attention = GetThemeColor(GGEMSThemeRole::Attention);
  ImVec4 const tab = GetThemeColor(GGEMSThemeRole::TabBackground);
  ImVec4 const tab_accent = GetThemeColor(GGEMSThemeRole::TabAccent);
  ImVec4 const separator = GetThemeColor(GGEMSThemeRole::Separator);
  ImVec4 const border = WithAlpha(GetThemeColor(GGEMSThemeRole::Border), 0.30F);
  ImVec4 const scrollbar_accent =
    GetThemeColor(GGEMSThemeRole::ScrollbarAccent);
  ImVec4 const app_background =
    GetThemeColor(GGEMSThemeRole::ApplicationBackground);
  ImVec4 const popup = GetThemeColor(GGEMSThemeRole::PopupBackground);

  // Main surfaces
  colors[ImGuiCol_WindowBg] = app_background;
  colors[ImGuiCol_ChildBg] = app_background;
  colors[ImGuiCol_PopupBg] = popup;
  colors[ImGuiCol_Border] = border;
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
  colors[ImGuiCol_MenuBarBg] = app_background;

  // Tabs
  colors[ImGuiCol_Tab] = tab;
  colors[ImGuiCol_TabHovered] = selection_hovered;
  colors[ImGuiCol_TabSelected] = selection;
  colors[ImGuiCol_TabSelectedOverline] = tab_accent;
  colors[ImGuiCol_TabDimmed] = tab;
  colors[ImGuiCol_TabDimmedSelected] = selection;
  colors[ImGuiCol_TabDimmedSelectedOverline] = tab_accent;

  // Scrollbars
  colors[ImGuiCol_ScrollbarBg] = recessed;
  colors[ImGuiCol_ScrollbarGrab] = selection;
  colors[ImGuiCol_ScrollbarGrabHovered] = WithAlpha(scrollbar_accent, 0.72F);
  colors[ImGuiCol_ScrollbarGrabActive] = scrollbar_accent;

  // Frames: checkbox, radio button, input, plot backgrounds...
  colors[ImGuiCol_FrameBg] = selection;
  colors[ImGuiCol_FrameBgHovered] = selection_hovered;
  colors[ImGuiCol_FrameBgActive] = selection;

  // Headers: tree nodes, collapsing headers, selectable rows...
  colors[ImGuiCol_Header] = selection;
  colors[ImGuiCol_HeaderHovered] = selection_hovered;
  colors[ImGuiCol_HeaderActive] = selection;

  // Separators
  colors[ImGuiCol_Separator] = separator;
  colors[ImGuiCol_SeparatorHovered] = accent;
  colors[ImGuiCol_SeparatorActive] = accent;

  // Checkboxes and sliders
  colors[ImGuiCol_CheckMark] = GetThemeColor(GGEMSThemeRole::PrimaryText);
  colors[ImGuiCol_CheckboxSelectedBg] = selection;
  colors[ImGuiCol_SliderGrab] = WithAlpha(selection, 0.85F);
  colors[ImGuiCol_SliderGrabActive] = WithAlpha(accent, 0.85F);

  // Buttons
  colors[ImGuiCol_Button] = selection;
  colors[ImGuiCol_ButtonHovered] = selection_hovered;
  colors[ImGuiCol_ButtonActive] = selection;

  // Resize grips
  colors[ImGuiCol_ResizeGrip] = WithAlpha(selection, 0.18F);
  colors[ImGuiCol_ResizeGripHovered] = WithAlpha(selection, 0.55F);
  colors[ImGuiCol_ResizeGripActive] = selection;

  // Docking
  colors[ImGuiCol_DockingPreview] = WithAlpha(selection, 0.45F);
  colors[ImGuiCol_DockingEmptyBg] = app_background;

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
    return render::BLUE_Gunmetal_F;
  case GGEMSThemeRole::RecessedBackground:
    return render::GRAY_Void;
  case GGEMSThemeRole::Selection:
    return render::BLUE_Gunmetal;
  case GGEMSThemeRole::TabBackground:
    return render::GRAY_Deep;
  case GGEMSThemeRole::TabAccent:
    return render::YELLOW_Shade5;
  case GGEMSThemeRole::PrimaryText:
    return render::WHITE_WhiteSmoke_B;
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
  case GGEMSThemeRole::OutputBackground:
    return render::BLUE_Gunmetal_F;
  case GGEMSThemeRole::Separator:
    return render::GRAY_Neutral80;
  case GGEMSThemeRole::Border:
  case GGEMSThemeRole::ScrollbarAccent:
    return render::GREEN_MediumSeaGreen_B;
  case GGEMSThemeRole::ApplicationBackground:
    return render::BLUE_Gunmetal_F;
  case GGEMSThemeRole::PopupBackground:
    return render::BLUE_Gunmetal;
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

  return ImVec4{
    render::SRGBChannelToLinear(rgb.red),
    render::SRGBChannelToLinear(rgb.green),
    render::SRGBChannelToLinear(rgb.blue),
    1.0F,
  };
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
