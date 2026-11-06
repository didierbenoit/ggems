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
 * \brief Routes viewport mouse and keyboard input into camera deltas.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <array>
#include <cstdint>
#include <cstddef>
#include <optional>

#include <imgui.h>
#include <imgui_internal.h>

#include "GGEMS/ui/detail/GGEMSImGuiViewportPanel.hh"
#include "GGEMS/ui/detail/GGEMSImGuiLayout.hh"
#include "GGEMS/ui/detail/GGEMSImGuiTheme.hh"
#include "GGEMS/ui/detail/GGEMSSceneCamera.hh"

namespace {

/*!
 * \brief Builds compact projection commands above the scene image.
 *
 * \param[in] icons Borrowed orthographic and perspective images.
 * \param[in] projection Camera projection defining the selected button.
 * \return Projection requested by a click, or no command.
 */
auto BuildProjectionToolbar(
  std::array<ImTextureID, 2U> const &icons,
  ggems::ui::detail::GGEMSSceneCamera::Projection projection)
  -> std::optional<ggems::ui::detail::GGEMSSceneCamera::Projection> {
  using Projection = ggems::ui::detail::GGEMSSceneCamera::Projection;

  std::optional<Projection> requested;

  float const scale = ImGui::GetFontSize() / ggems::ui::k_ggems_base_font_size;

  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,
                      ImVec2{3.0F * scale, 3.0F * scale});
  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                      ImVec2{4.0F * scale, 4.0F * scale});

  for (std::size_t index = 0U; index < icons.size(); ++index) {
    Projection const mode =
      index == 0U ? Projection::Orthographic : Projection::Perspective;

    char const *id =
      index == 0U ? "##orthographic_projection" : "##perspective_projection";

    char const *tooltip =
      index == 0U ? "Orthographic projection" : "Perspective projection";

    if (index != 0U) {
      ImGui::SameLine();
    }

    ImGui::PushStyleColor(ImGuiCol_Button,
                          ggems::ui::GetThemeColor(
                            projection == mode
                              ? ggems::ui::GGEMSThemeRole::Selection
                              : ggems::ui::GGEMSThemeRole::WindowBackground));

    // Both PNG sizes share transparent margins. Crop only those margins so
    // the original artwork occupies the compact image area at either size.
    if (ImGui::ImageButton(id, icons[index], {22.0F * scale, 22.0F * scale},
                           {0.125F, 0.125F}, {0.875F, 0.875F})) {
      requested = mode;
    }

    ImGui::PopStyleColor();
    ImGui::SetItemTooltip("%s", tooltip);
  }

  ImGui::PopStyleVar(2);

  return requested;
}

// =============================================================================
// =============================================================================

/*! \brief Unmodified keyboard pan speed in logical pixels per second. */
constexpr float k_keyboard_pan_speed_pixels_per_second{360.0F};

/*! \brief Keyboard pan multiplier while Shift is held. */
constexpr float k_keyboard_pan_fast_factor{3.0F};

/*! \brief Keyboard pan multiplier while Control is held. */
constexpr float k_keyboard_pan_precise_factor{0.25F};

// =============================================================================
// =============================================================================

/*! \brief Maps a routed arrow key to a screen-space pan direction. */
struct KeyboardPanKey {
  /*! \brief Arrow key whose held state drives panning. */
  ImGuiKey key;

  /*! \brief Signed horizontal pan direction. */
  float direction_x;

  /*! \brief Signed vertical pan direction. */
  float direction_y;
};

// =============================================================================
// =============================================================================

/*! \brief Arrow-key bindings for camera-plane panning. */
constexpr std::array<KeyboardPanKey, 4U> k_keyboard_pan_keys{
  {
    {.key = ImGuiKey_LeftArrow, .direction_x = -1.0F, .direction_y = 0.0F},
    {.key = ImGuiKey_RightArrow, .direction_x = 1.0F, .direction_y = 0.0F},
    {.key = ImGuiKey_UpArrow, .direction_x = 0.0F, .direction_y = -1.0F},
    {.key = ImGuiKey_DownArrow, .direction_x = 0.0F, .direction_y = 1.0F},
  },
};

// =============================================================================
// =============================================================================

/*! \brief Admitted modifier combinations for viewport pan shortcuts. */
constexpr std::array<ImGuiKeyChord, 4U> k_keyboard_pan_modifiers{
  ImGuiMod_None,
  ImGuiMod_Shift,
  ImGuiMod_Ctrl,
  ImGuiMod_Shift | ImGuiMod_Ctrl,
};

// =============================================================================
// =============================================================================

/*!
 * \brief Accumulates routed arrow-key motion for the current frame.
 *
 * \param[in] scene_item_id ImGui item identity owning the scene keyboard
 *   shortcuts.
 * \param[in,out] result Camera deltas receiving time-scaled keyboard pan
 *   motion.
 */
auto AddKeyboardPan(ImGuiID scene_item_id,
                    ggems::ui::detail::GGEMSImGuiViewportPanel::Result &result)
  -> void {
  ImGuiIO const &imgui_io = ImGui::GetIO();

  float keyboard_pan_speed_pixels_per_second =
    k_keyboard_pan_speed_pixels_per_second;

  if (imgui_io.KeyShift) {
    keyboard_pan_speed_pixels_per_second *= k_keyboard_pan_fast_factor;
  }

  if (imgui_io.KeyCtrl) {
    keyboard_pan_speed_pixels_per_second *= k_keyboard_pan_precise_factor;
  }

  float const keyboard_pan_delta_pixels =
    keyboard_pan_speed_pixels_per_second * imgui_io.DeltaTime;

  for (KeyboardPanKey const &pan_key : k_keyboard_pan_keys) {
    bool current_chord_routed{false};

    for (ImGuiKeyChord modifiers : k_keyboard_pan_modifiers) {
      bool const granted = ImGui::SetShortcutRouting(
        pan_key.key | modifiers, ImGuiInputFlags_RouteFocused, scene_item_id);

      current_chord_routed =
        current_chord_routed || (granted && modifiers == imgui_io.KeyMods);
    }

    if (!current_chord_routed || imgui_io.WantTextInput) {
      continue;
    }

    ImGui::SetKeyOwner(pan_key.key, scene_item_id);

    if (ImGui::IsKeyDown(pan_key.key, scene_item_id)) {
      result.pan_delta_x_pixels +=
        pan_key.direction_x * keyboard_pan_delta_pixels;
      result.pan_delta_y_pixels +=
        pan_key.direction_y * keyboard_pan_delta_pixels;
    }
  }
}

} // namespace

namespace ggems::ui::detail {

// =============================================================================
// =============================================================================

auto GGEMSImGuiViewportPanel::Build(
  bool &show_window, ImTextureID scene_texture_id, std::uint32_t texture_width,
  std::uint32_t texture_height,
  std::array<ImTextureID, 2U> const &projection_icons,
  GGEMSSceneCamera::Projection projection) -> Result {
  Result result{};

  ImGuiWindowFlags const window_flags =
    ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

  if (!ImGui::Begin(k_viewport_window_name, &show_window, window_flags)) {
    CancelGesture();
    ImGui::End();
    return result;
  }

  result.projection_request =
    BuildProjectionToolbar(projection_icons, projection);

  ImVec2 const available_size = ImGui::GetContentRegionAvail();

  result.visible = true;
  result.width =
    available_size.x > 1.0F ? static_cast<std::uint32_t>(available_size.x) : 1U;
  result.height =
    available_size.y > 1.0F ? static_cast<std::uint32_t>(available_size.y) : 1U;

  bool const texture_matches_viewport = scene_texture_id != ImTextureID{} &&
                                        texture_width == result.width &&
                                        texture_height == result.height;

  if (!texture_matches_viewport) {
    ImGui::TextDisabled("Vulkan scene renderer: preparing render target...");
    ImGui::TextDisabled("Viewport extent: %u x %u", result.width,
                        result.height);
    CancelGesture();
    ImGui::End();
    return result;
  }

  ImVec2 const image_size{static_cast<float>(result.width),
                          static_cast<float>(result.height)};

  ImGuiContext const &imgui_context = *ImGui::GetCurrentContext();
  ImGuiID const scene_item_id = ImGui::GetID("##scene");

  // Check ownership before the widget consumes a possible button release.
  if (imgui_context.ActiveId != scene_item_id) {
    CancelGesture();
  }

  ImGui::InvisibleButton("##scene", image_size,
                         ImGuiButtonFlags_MouseButtonLeft |
                           ImGuiButtonFlags_MouseButtonMiddle);

  ImGui::GetWindowDrawList()->AddImage(
    scene_texture_id, ImGui::GetItemRectMin(), ImGui::GetItemRectMax());

  ImGuiIO const &imgui_io = ImGui::GetIO();

  if (ImGui::IsItemHovered(ImGuiHoveredFlags_NoNavOverride) &&
      ImGui::IsMousePosValid(&imgui_io.MousePos)) {
    ImVec2 const image_origin = ImGui::GetItemRectMin();
    result.cursor_pixels = ImVec2{imgui_io.MousePos.x - image_origin.x,
                                  imgui_io.MousePos.y - image_origin.y};
  }

  ImGuiMouseButton const activated_button =
    ImGui::IsItemActivated() &&
        imgui_context.ActiveIdSource == ImGuiInputSource_Mouse
      ? imgui_context.ActiveIdMouseButton
      : -1;

  AddMouseGestures(activated_button, result);

  if (!ImGui::IsItemActive()) {
    CancelGesture();
  }

  if (ImGui::IsItemHovered(ImGuiHoveredFlags_NoNavOverride) &&
      ImGui::SetItemKeyOwner(ImGuiKey_MouseWheelY) &&
      imgui_io.MouseWheel != 0.0F) {
    result.zoom_delta = imgui_io.MouseWheel;
  }

  AddKeyboardPan(ImGui::GetItemID(), result);

  ImGui::End();

  return result;
}

// -----------------------------------------------------------------------------

auto GGEMSImGuiViewportPanel::CancelGesture() noexcept -> void {
  gesture_ = Gesture::None;
}

// -----------------------------------------------------------------------------

auto GGEMSImGuiViewportPanel::AddMouseGestures(
  ImGuiMouseButton activated_button, Result &result) -> void {
  ImGuiContext const &imgui_context = *ImGui::GetCurrentContext();
  ImVec2 mouse_position = imgui_context.IO.MousePosPrev;

  // The trail contains only events consumed by NewFrame, in their original
  // order. MouseDelta spans the whole frame and cannot delimit a gesture.
  for (ImGuiInputEvent const &event : imgui_context.InputEventsTrail) {
    switch (event.Type) {
    case ImGuiInputEventType_MousePos: {
      ImVec2 next_position{event.MousePos.PosX, event.MousePos.PosY};

      if (ImGui::IsMousePosValid(&next_position)) {
        next_position = ImFloor(next_position);
      }

      // Match ImGui's logical-pixel rounding and suppress jumps across an
      // invalid native cursor position, while retaining captured ownership.
      if (ImGui::IsMousePosValid(&mouse_position) &&
          ImGui::IsMousePosValid(&next_position)) {
        float const delta_x = next_position.x - mouse_position.x;
        float const delta_y = next_position.y - mouse_position.y;

        if (gesture_ == Gesture::Orbit) {
          result.orbit_delta_x_pixels += delta_x;
          result.orbit_delta_y_pixels += delta_y;
        } else if (gesture_ == Gesture::Pan) {
          result.pan_delta_x_pixels += delta_x;
          result.pan_delta_y_pixels += delta_y;
        }
      }

      mouse_position = next_position;
      break;
    }
    case ImGuiInputEventType_MouseButton:
      if (event.MouseButton.Down &&
          event.MouseButton.Button == activated_button) {
        gesture_ = activated_button == ImGuiMouseButton_Left ? Gesture::Orbit
                                                             : Gesture::Pan;
        activated_button = -1;
      } else if (!event.MouseButton.Down &&
                 ((gesture_ == Gesture::Orbit &&
                   event.MouseButton.Button == ImGuiMouseButton_Left) ||
                  (gesture_ == Gesture::Pan &&
                   event.MouseButton.Button == ImGuiMouseButton_Middle))) {
        CancelGesture();
      }
      break;
    case ImGuiInputEventType_Focus:
      if (!event.AppFocused.Focused) {
        CancelGesture();
      }
      break;
    default:
      break;
    }
  }
}

} // namespace ggems::ui::detail
