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

#include <imgui.h>
#include <imgui_internal.h>

#include "GGEMS/ui/detail/GGEMSImGuiViewportPanel.hh"
#include "GGEMS/ui/detail/GGEMSImGuiLayout.hh"

namespace {

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

auto GGEMSImGuiViewportPanel::Build(bool &show_window,
                                    ImTextureID scene_texture_id,
                                    std::uint32_t texture_width,
                                    std::uint32_t texture_height) -> Result {
  Result result{};

  ImGuiWindowFlags const window_flags =
    ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

  if (!ImGui::Begin(k_viewport_window_name, &show_window, window_flags)) {
    CancelGesture();
    ImGui::End();
    return result;
  }

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

  ImGui::InvisibleButton("##scene", image_size,
                         ImGuiButtonFlags_MouseButtonLeft |
                           ImGuiButtonFlags_MouseButtonMiddle);
  ImGui::GetWindowDrawList()->AddImage(
    scene_texture_id, ImGui::GetItemRectMin(), ImGui::GetItemRectMax());

  ImGuiIO const &imgui_io = ImGui::GetIO();

  if (ImGui::IsItemActivated()) {
    if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
      gesture_ = Gesture::Orbit;
    } else if (ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
      gesture_ = Gesture::Pan;
    }
  }

  if (!ImGui::IsItemActive()) {
    gesture_ = Gesture::None;
  }

  switch (gesture_) {
  case Gesture::Orbit:
    result.orbit_delta_x_pixels = imgui_io.MouseDelta.x;
    result.orbit_delta_y_pixels = imgui_io.MouseDelta.y;
    break;
  case Gesture::Pan:
    result.pan_delta_x_pixels = imgui_io.MouseDelta.x;
    result.pan_delta_y_pixels = imgui_io.MouseDelta.y;
    break;
  case Gesture::None:
    break;
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

} // namespace ggems::ui::detail
