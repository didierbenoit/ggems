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
 * \brief Checks viewport gesture boundaries through the real ImGui event queue.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <cfloat>
#include <cstdint>

#include <gtest/gtest.h>
#include <imgui.h>
#include <imgui_internal.h>

#include "GGEMS/ui/detail/GGEMSImGuiViewportPanel.hh"
#include "GGEMS/ui/detail/GGEMSImGuiTheme.hh"
#include "GGEMS/ui/detail/GGEMSSceneCamera.hh"

namespace {

using Panel = ggems::ui::detail::GGEMSImGuiViewportPanel;
using Camera = ggems::ui::detail::GGEMSSceneCamera;

// A real ImGui context and panel need no GLFW window or Vulkan device to route
// queued input. Settings and logs are disabled to keep this fixture in memory.
class GGEMSImGuiViewportInput : public ::testing::Test {
protected:
  auto SetUp() -> void override {
    context_ = ImGui::CreateContext();
    auto &io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.DisplaySize = {800.0F, 600.0F};
    io.DeltaTime = 1.0F / 60.0F;
    unsigned char *pixels{};
    int atlas_width{};
    int atlas_height{};
    io.Fonts->GetTexDataAsRGBA32(&pixels, &atlas_width, &atlas_height);
    for (int frame = 0; frame < 3; ++frame) {
      Frame();
    }
    io.AddMousePosEvent(100.0F, 100.0F);
    Frame();
  }

  auto TearDown() -> void override { ImGui::DestroyContext(context_); }

  auto Frame(bool stale_texture = false) -> Panel::Result {
    ImGui::NewFrame();
    ImGui::SetNextWindowPos({0.0F, 0.0F});
    ImGui::SetNextWindowSize(window_size_);
    bool visible = true;
    auto const result = panel_.Build(
      visible, ImTextureID{1}, stale_texture ? 1U : width_, height_,
      {ImTextureID{2}, ImTextureID{3}}, camera_.GetDiagnostics().projection);
    if (result.projection_request) {
      camera_.SetProjection(*result.projection_request);
    }
    width_ = result.width;
    height_ = result.height;
    ImGui::Render();
    return result;
  }

  // Measure the actual submitted image vertices instead of reconstructing
  // the toolbar offset. Each fake texture identifies one real image command.
  static auto ImageBounds(ImTextureID texture) -> ImRect {
    auto const *window =
      ImGui::FindWindowByName("GGEMS Viewport###GGEMS.Viewport");
    EXPECT_NE(window, nullptr);
    ImRect bounds{{FLT_MAX, FLT_MAX}, {-FLT_MAX, -FLT_MAX}};
    if (window == nullptr) {
      return bounds;
    }
    auto const &draw_list = *window->DrawList;
    for (auto const &command : draw_list.CmdBuffer) {
      if (command.GetTexID() != texture) {
        continue;
      }
      for (unsigned int index = 0U; index < command.ElemCount; ++index) {
        // These offsets refer to the same ImGui buffers, whose sizes are int.
        int const element_index = static_cast<int>(command.IdxOffset + index);
        int const vertex_index = static_cast<int>(
          command.VtxOffset + draw_list.IdxBuffer[element_index]);
        bounds.Add(draw_list.VtxBuffer[vertex_index].pos);
      }
    }
    EXPECT_FALSE(bounds.IsInverted());
    return bounds;
  }

  auto ClickImage(ImTextureID texture, bool stale_texture = false)
    -> Panel::Result {
    ImVec2 const center = ImageBounds(texture).GetCenter();
    auto &io = ImGui::GetIO();
    io.AddMousePosEvent(center.x, center.y);
    Frame(stale_texture);
    io.AddMouseButtonEvent(ImGuiMouseButton_Left, true);
    ExpectNoMouseMotion(Frame(stale_texture));
    io.AddMouseButtonEvent(ImGuiMouseButton_Left, false);
    return Frame(stale_texture);
  }

  static auto ExpectNoMouseMotion(Panel::Result const &result) -> void {
    EXPECT_FLOAT_EQ(result.orbit_delta_x_pixels, 0.0F);
    EXPECT_FLOAT_EQ(result.orbit_delta_y_pixels, 0.0F);
    EXPECT_FLOAT_EQ(result.pan_delta_x_pixels, 0.0F);
    EXPECT_FLOAT_EQ(result.pan_delta_y_pixels, 0.0F);
  }

  Panel panel_;
  Camera camera_;
  ImVec2 window_size_{600.0F, 500.0F};

private:
  ImGuiContext *context_{};
  std::uint32_t width_{};
  std::uint32_t height_{};
};

class GGEMSImGuiViewportGesture
    : public GGEMSImGuiViewportInput,
      public ::testing::WithParamInterface<ImGuiMouseButton> {
protected:
  static auto Button(bool down) -> void {
    ImGui::GetIO().AddMouseButtonEvent(GetParam(), down);
  }

  [[nodiscard]] static auto Motion(Panel::Result const &result) -> ImVec2 {
    if (GetParam() == ImGuiMouseButton_Left) {
      EXPECT_FLOAT_EQ(result.pan_delta_x_pixels, 0.0F);
      EXPECT_FLOAT_EQ(result.pan_delta_y_pixels, 0.0F);
      return {result.orbit_delta_x_pixels, result.orbit_delta_y_pixels};
    }
    EXPECT_FLOAT_EQ(result.orbit_delta_x_pixels, 0.0F);
    EXPECT_FLOAT_EQ(result.orbit_delta_y_pixels, 0.0F);
    return {result.pan_delta_x_pixels, result.pan_delta_y_pixels};
  }

  static auto ExpectMotion(Panel::Result const &result, float x, float y = 0.0F)
    -> void {
    auto const motion = Motion(result);
    EXPECT_FLOAT_EQ(motion.x, x);
    EXPECT_FLOAT_EQ(motion.y, y);
  }

  // The default trickle queue can spread a complete click/drag/release batch
  // over several frames. Include idle frames and check the whole held interval.
  auto DrainMotion() -> ImVec2 {
    ImVec2 total{};
    for (int frame = 0; frame < 4; ++frame) {
      auto const motion = Motion(Frame());
      total.x += motion.x;
      total.y += motion.y;
    }
    return total;
  }
};

// Hover movement before the press never belongs to the acquired gesture.
TEST_P(GGEMSImGuiViewportGesture, ExcludesMovementBeforePress) {
  ImGui::GetIO().AddMousePosEvent(140.0F, 100.0F);
  Button(true);
  ExpectMotion(Frame(), 0.0F);
  Button(false);
  ExpectMotion(Frame(), 0.0F);
}

// Releasing after a final move preserves that move as well as earlier frames.
TEST_P(GGEMSImGuiViewportGesture, PreservesMovementBeforeRelease) {
  Button(true);
  ExpectMotion(Frame(), 0.0F);
  ImGui::GetIO().AddMousePosEvent(120.0F, 100.0F);
  ExpectMotion(Frame(), 20.0F);
  ImGui::GetIO().AddMousePosEvent(140.0F, 100.0F);
  Button(false);
  auto const final = DrainMotion();
  EXPECT_FLOAT_EQ(final.x, 20.0F);
  EXPECT_FLOAT_EQ(final.y, 0.0F);
}

// A complete short drag remains visible even when queued between two frames.
TEST_P(GGEMSImGuiViewportGesture, PreservesCompleteDragQueuedBetweenFrames) {
  Button(true);
  ImGui::GetIO().AddMousePosEvent(140.0F, 100.0F);
  Button(false);
  auto const total = DrainMotion();
  EXPECT_FLOAT_EQ(total.x, 40.0F);
  EXPECT_FLOAT_EQ(total.y, 0.0F);
}

// Repositioning the released pointer in the opposite direction is not
// orbit/pan.
TEST_P(GGEMSImGuiViewportGesture, RepressExcludesOppositeHoverMovement) {
  Button(true);
  ExpectMotion(Frame(), 0.0F);
  ImGui::GetIO().AddMousePosEvent(140.0F, 100.0F);
  ExpectMotion(Frame(), 40.0F);
  Button(false);
  ExpectMotion(Frame(), 0.0F);
  ImGui::GetIO().AddMousePosEvent(100.0F, 100.0F);
  Button(true);
  ExpectMotion(Frame(), 0.0F);
  ImGui::GetIO().AddMousePosEvent(120.0F, 100.0F);
  ExpectMotion(Frame(), 20.0F);
}

// Queued hover movement after release cannot add to the held displacement.
TEST_P(GGEMSImGuiViewportGesture, ExcludesMovementAfterReleaseInSameBatch) {
  Button(true);
  ExpectMotion(Frame(), 0.0F);
  ImGui::GetIO().AddMousePosEvent(140.0F, 100.0F);
  Button(false);
  ImGui::GetIO().AddMousePosEvent(80.0F, 100.0F);
  auto const total = DrainMotion();
  EXPECT_FLOAT_EQ(total.x, 40.0F);
  EXPECT_FLOAT_EQ(total.y, 0.0F);
}

// A press and its movement can be queued between the same two frames.
TEST_P(GGEMSImGuiViewportGesture, IncludesMovementAfterPressInSameBatch) {
  Button(true);
  ImGui::GetIO().AddMousePosEvent(140.0F, 100.0F);
  auto const total = DrainMotion();
  EXPECT_FLOAT_EQ(total.x, 40.0F);
  EXPECT_FLOAT_EQ(total.y, 0.0F);
}

// An owned gesture keeps signed X/Y movement when leaving and reentering.
TEST_P(GGEMSImGuiViewportGesture, RetainsSignsOutsideViewportWhileOwned) {
  Button(true);
  ExpectMotion(Frame(), 0.0F);
  ImGui::GetIO().AddMousePosEvent(700.0F, 120.0F);
  ExpectMotion(Frame(), 600.0F, 20.0F);
  ImGui::GetIO().AddMousePosEvent(200.0F, 90.0F);
  ExpectMotion(Frame(), -500.0F, -30.0F);
  Button(false);
  ExpectMotion(Frame(), 0.0F);
}

// Explicit cancellation remains effective until a fresh press is admitted.
TEST_P(GGEMSImGuiViewportGesture, ExplicitCancelDoesNotRestartWhileHeld) {
  Button(true);
  ExpectMotion(Frame(), 0.0F);
  ImGui::GetIO().AddMousePosEvent(120.0F, 100.0F);
  ExpectMotion(Frame(), 20.0F);
  panel_.CancelGesture();
  ImGui::GetIO().AddMousePosEvent(140.0F, 100.0F);
  ExpectMotion(Frame(), 0.0F);
  ImGui::GetIO().AddMousePosEvent(160.0F, 100.0F);
  Button(false);
  ExpectMotion(Frame(), 0.0F);
  Button(true);
  ExpectMotion(Frame(), 0.0F);
  ImGui::GetIO().AddMousePosEvent(180.0F, 100.0F);
  ExpectMotion(Frame(), 20.0F);
}

// The resize/preparation placeholder cancels ownership even if the button
// stays.
TEST_P(GGEMSImGuiViewportGesture, StaleTextureCancelsUntilFreshPress) {
  Button(true);
  ExpectMotion(Frame(), 0.0F);
  ImGui::GetIO().AddMousePosEvent(120.0F, 100.0F);
  ExpectMotion(Frame(), 20.0F);
  ImGui::GetIO().AddMousePosEvent(140.0F, 100.0F);
  ExpectMotion(Frame(true), 0.0F);
  ImGui::GetIO().AddMousePosEvent(160.0F, 100.0F);
  ExpectMotion(Frame(), 0.0F);
  Button(false);
  ExpectMotion(Frame(), 0.0F);
  Button(true);
  ExpectMotion(Frame(), 0.0F);
  ImGui::GetIO().AddMousePosEvent(180.0F, 100.0F);
  ExpectMotion(Frame(), 20.0F);
}

// Focus loss cancels the gesture; merely regaining focus must not acquire one.
TEST_P(GGEMSImGuiViewportGesture, FocusLossRequiresFreshPress) {
  Button(true);
  ExpectMotion(Frame(), 0.0F);
  ImGui::GetIO().AddMousePosEvent(120.0F, 100.0F);
  ExpectMotion(Frame(), 20.0F);
  ImGui::GetIO().AddFocusEvent(false);
  ExpectMotion(Frame(), 0.0F);
  ImGui::GetIO().AddFocusEvent(true);
  ImGui::GetIO().AddMousePosEvent(140.0F, 100.0F);
  ExpectMotion(Frame(), 0.0F);
  Button(true);
  ExpectMotion(Frame(), 0.0F);
  ImGui::GetIO().AddMousePosEvent(160.0F, 100.0F);
  ExpectMotion(Frame(), 20.0F);
}

// A final focused state must not erase a focus-loss event within the frame.
TEST_P(GGEMSImGuiViewportGesture, FocusLossAndRegainInSameBatchCancelGesture) {
  Button(true);
  ExpectMotion(Frame(), 0.0F);
  ImGui::GetIO().AddFocusEvent(false);
  ImGui::GetIO().AddFocusEvent(true);
  ImGui::GetIO().AddMousePosEvent(140.0F, 100.0F);
  ExpectMotion(Frame(), 0.0F);
  ImGui::GetIO().AddMousePosEvent(160.0F, 100.0F);
  ExpectMotion(Frame(), 0.0F);
  Button(false);
  ExpectMotion(Frame(), 0.0F);
  Button(true);
  ExpectMotion(Frame(), 0.0F);
  ImGui::GetIO().AddMousePosEvent(180.0F, 100.0F);
  ExpectMotion(Frame(), 20.0F);
}

// Missing native cursor coordinates cannot become a drag or a reentry jump.
TEST_P(GGEMSImGuiViewportGesture, InvalidCursorPositionRebasesWithoutJump) {
  Button(true);
  ExpectMotion(Frame(), 0.0F);
  ImGui::GetIO().AddMousePosEvent(120.0F, 100.0F);
  ExpectMotion(Frame(), 20.0F);
  ImGui::GetIO().AddMousePosEvent(-FLT_MAX, -FLT_MAX);
  ExpectMotion(Frame(), 0.0F);
  ImGui::GetIO().AddMousePosEvent(200.0F, 100.0F);
  ExpectMotion(Frame(), 0.0F);
  ImGui::GetIO().AddMousePosEvent(220.0F, 100.0F);
  ExpectMotion(Frame(), 20.0F);
}

// A new press after focus returns can acquire a fresh owned gesture.
TEST_P(GGEMSImGuiViewportGesture,
       FreshPressAfterFocusReturnInSameBatchAcquires) {
  ImGui::GetIO().AddFocusEvent(false);
  ImGui::GetIO().AddFocusEvent(true);
  Button(true);
  ExpectMotion(Frame(), 0.0F);
  ImGui::GetIO().AddMousePosEvent(140.0F, 100.0F);
  ExpectMotion(Frame(), 40.0F);
}

// After cancellation, a fresh press can own the image while another button
// remains held.
TEST_P(GGEMSImGuiViewportGesture, UsesButtonThatAcquiredTheViewport) {
  auto const other_button = GetParam() == ImGuiMouseButton_Left
                              ? ImGuiMouseButton_Middle
                              : ImGuiMouseButton_Left;
  ImGui::GetIO().AddMouseButtonEvent(other_button, true);
  ExpectMotion(Frame(), 0.0F);
  ExpectMotion(Frame(true), 0.0F);
  ExpectMotion(Frame(), 0.0F);
  Button(true);
  ExpectMotion(Frame(), 0.0F);
  ImGui::GetIO().AddMousePosEvent(120.0F, 100.0F);
  ExpectMotion(Frame(), 20.0F);
}

// ImGui reserves a gesture started outside every window for the application.
TEST_P(GGEMSImGuiViewportGesture, OutsidePressCannotBeStolenBySecondButton) {
  auto const other_button = GetParam() == ImGuiMouseButton_Left
                              ? ImGuiMouseButton_Middle
                              : ImGuiMouseButton_Left;
  ImGui::GetIO().AddMousePosEvent(700.0F, 100.0F);
  ImGui::GetIO().AddMouseButtonEvent(other_button, true);
  ExpectMotion(Frame(), 0.0F);
  ImGui::GetIO().AddMousePosEvent(100.0F, 100.0F);
  ExpectMotion(Frame(), 0.0F);
  Button(true);
  ExpectMotion(Frame(), 0.0F);
  ImGui::GetIO().AddMousePosEvent(120.0F, 100.0F);
  ExpectMotion(Frame(), 0.0F);
}

// Another held button neither switches an active gesture nor inherits its end.
TEST_P(GGEMSImGuiViewportGesture, AnotherButtonDoesNotReplaceTheOwner) {
  auto const other_button = GetParam() == ImGuiMouseButton_Left
                              ? ImGuiMouseButton_Middle
                              : ImGuiMouseButton_Left;
  Button(true);
  ExpectMotion(Frame(), 0.0F);
  ImGui::GetIO().AddMousePosEvent(120.0F, 100.0F);
  ExpectMotion(Frame(), 20.0F);
  ImGui::GetIO().AddMouseButtonEvent(other_button, true);
  ExpectMotion(Frame(), 0.0F);
  ImGui::GetIO().AddMousePosEvent(140.0F, 100.0F);
  ExpectMotion(Frame(), 20.0F);
  ImGui::GetIO().AddMousePosEvent(160.0F, 100.0F);
  Button(false);
  auto const final = DrainMotion();
  EXPECT_FLOAT_EQ(final.x, 20.0F);
  EXPECT_FLOAT_EQ(final.y, 0.0F);
  ImGui::GetIO().AddMousePosEvent(180.0F, 100.0F);
  ExpectMotion(Frame(), 0.0F);
}

INSTANTIATE_TEST_SUITE_P(MouseButtons, GGEMSImGuiViewportGesture,
                         ::testing::Values(ImGuiMouseButton_Left,
                                           ImGuiMouseButton_Middle));

// Unheld pointer motion cannot create orbit or pan commands.
TEST_F(GGEMSImGuiViewportInput, PointerMotionWithoutButtonsHasNoCameraMotion) {
  ImGui::GetIO().AddMousePosEvent(140.0F, 120.0F);
  ExpectNoMouseMotion(Frame());
  ImGui::GetIO().AddMousePosEvent(80.0F, 90.0F);
  ExpectNoMouseMotion(Frame());
}

// Wheel zoom keeps its hovered-viewport ownership without requiring a drag.
TEST_F(GGEMSImGuiViewportInput, WheelZoomRequiresHoveredViewport) {
  ImGui::GetIO().AddMouseWheelEvent(0.0F, 2.0F);
  auto const hovered = Frame();
  ExpectNoMouseMotion(hovered);
  EXPECT_FLOAT_EQ(hovered.zoom_delta, 2.0F);
  ImGui::GetIO().AddMousePosEvent(700.0F, 100.0F);
  Frame();
  ImGui::GetIO().AddMouseWheelEvent(0.0F, 1.0F);
  auto const outside = Frame();
  ExpectNoMouseMotion(outside);
  EXPECT_FLOAT_EQ(outside.zoom_delta, 0.0F);
}

// Existing focused keyboard routes retain their speed and modifier policy.
TEST_F(GGEMSImGuiViewportInput, FocusedKeyboardPanKeepsModifierSpeeds) {
  auto &io = ImGui::GetIO();
  io.AddMouseButtonEvent(ImGuiMouseButton_Left, true);
  Frame();
  io.AddMouseButtonEvent(ImGuiMouseButton_Left, false);
  Frame();
  io.AddKeyEvent(ImGuiKey_RightArrow, true);
  EXPECT_FLOAT_EQ(Frame().pan_delta_x_pixels, 6.0F);
  io.AddKeyEvent(ImGuiMod_Shift, true);
  EXPECT_FLOAT_EQ(Frame().pan_delta_x_pixels, 18.0F);
  io.AddKeyEvent(ImGuiMod_Ctrl, true);
  EXPECT_FLOAT_EQ(Frame().pan_delta_x_pixels, 4.5F);
  io.AddKeyEvent(ImGuiMod_Shift, false);
  EXPECT_FLOAT_EQ(Frame().pan_delta_x_pixels, 1.5F);
  io.AddKeyEvent(ImGuiKey_RightArrow, false);
  ExpectNoMouseMotion(Frame());
}

TEST_F(GGEMSImGuiViewportInput, CursorUsesLogicalImageCoordinatesAndClears) {
  auto const initial = Frame();
  ImVec2 const origin = ImageBounds(ImTextureID{1}).Min;
  auto &io = ImGui::GetIO();

  for (float const density : {1.0F, 1.5F, 2.0F}) {
    io.DisplayFramebufferScale = {density, density};
    io.AddMousePosEvent(origin.x + 120.0F, origin.y + 80.0F);
    auto const result = Frame();
    ASSERT_TRUE(result.cursor_pixels);
    EXPECT_FLOAT_EQ(result.cursor_pixels->x, 120.0F);
    EXPECT_FLOAT_EQ(result.cursor_pixels->y, 80.0F);
    EXPECT_EQ(result.width, initial.width);
    EXPECT_EQ(result.height, initial.height);
    ExpectNoMouseMotion(result);
  }

  EXPECT_FALSE(Frame(true).cursor_pixels);
  EXPECT_TRUE(Frame().cursor_pixels);
  io.AddMousePosEvent(750.0F, 550.0F);
  EXPECT_FALSE(Frame().cursor_pixels);
  io.AddMousePosEvent(-10000.0F, -10000.0F);
  EXPECT_FALSE(Frame().cursor_pixels);
}

TEST_F(GGEMSImGuiViewportInput, ProjectionButtonsIssueOnlyProjectionCommands) {
  auto const initial = camera_.GetDiagnostics();
  auto const perspective = ClickImage(ImTextureID{3});
  ASSERT_TRUE(perspective.projection_request);
  EXPECT_EQ(*perspective.projection_request, Camera::Projection::Perspective);
  ExpectNoMouseMotion(perspective);
  EXPECT_FLOAT_EQ(perspective.zoom_delta, 0.0F);
  EXPECT_FALSE(perspective.cursor_pixels);
  EXPECT_FLOAT_EQ(camera_.GetDiagnostics().yaw_degrees, initial.yaw_degrees);
  EXPECT_FLOAT_EQ(camera_.GetDiagnostics().pitch_degrees,
                  initial.pitch_degrees);

  auto const orthographic = ClickImage(ImTextureID{2}, true);
  ASSERT_TRUE(orthographic.projection_request);
  EXPECT_EQ(*orthographic.projection_request, Camera::Projection::Orthographic);
  ExpectNoMouseMotion(orthographic);
  EXPECT_FLOAT_EQ(orthographic.zoom_delta, 0.0F);
  EXPECT_FALSE(orthographic.cursor_pixels);
}

TEST_F(GGEMSImGuiViewportInput, ToolbarDragAndWheelNeverAcquireSceneInput) {
  for (auto const button : {ImGuiMouseButton_Left, ImGuiMouseButton_Middle}) {
    auto &io = ImGui::GetIO();
    ImVec2 const start = ImageBounds(ImTextureID{3}).GetCenter();
    io.AddMousePosEvent(start.x, start.y);
    Frame();
    io.AddMouseWheelEvent(0.0F, 2.0F);
    auto const wheel = Frame();
    ExpectNoMouseMotion(wheel);
    EXPECT_FLOAT_EQ(wheel.zoom_delta, 0.0F);
    EXPECT_FALSE(wheel.cursor_pixels);
    io.AddMouseButtonEvent(button, true);
    ExpectNoMouseMotion(Frame());
    ImVec2 const scene = ImageBounds(ImTextureID{1}).GetCenter();
    io.AddMousePosEvent(scene.x, scene.y);
    auto const held = Frame();
    ExpectNoMouseMotion(held);
    EXPECT_FALSE(held.projection_request);
    io.AddMouseButtonEvent(button, false);
    ExpectNoMouseMotion(Frame());
  }
}

TEST_F(GGEMSImGuiViewportInput, ToolbarScalesAndSceneCoordinatesFollowResize) {
  auto &io = ImGui::GetIO();
  for (float const scale : {1.0F, 1.5F, 2.0F}) {
    ImGui::GetStyle() = ggems::ui::BuildGGEMSStyle(1.0F, scale);
    window_size_ = {500.0F + (50.0F * scale), 400.0F + (40.0F * scale)};
    Frame();
    auto const settled = Frame();
    auto const orthographic = ImageBounds(ImTextureID{2});
    auto const perspective = ImageBounds(ImTextureID{3});
    auto const scene = ImageBounds(ImTextureID{1});
    // ImGui rounds the effective font size, including at fractional DPI.
    auto const *window =
      ImGui::FindWindowByName("GGEMS Viewport###GGEMS.Viewport");
    ASSERT_NE(window, nullptr);
    float const font_scale =
      window->FontRefSize / ggems::ui::k_ggems_base_font_size;
    EXPECT_FLOAT_EQ(orthographic.GetWidth(), 22.0F * font_scale);
    EXPECT_FLOAT_EQ(orthographic.GetHeight(), 22.0F * font_scale);
    EXPECT_FLOAT_EQ(perspective.GetWidth(), orthographic.GetWidth());
    EXPECT_FLOAT_EQ(perspective.Min.x - orthographic.Min.x, 32.0F * font_scale);
    EXPECT_GT(scene.Min.y, orthographic.Max.y);
    EXPECT_FLOAT_EQ(scene.GetWidth(), static_cast<float>(settled.width));
    EXPECT_FLOAT_EQ(scene.GetHeight(), static_cast<float>(settled.height));
    io.AddMousePosEvent(scene.Min.x + 120.0F, scene.Min.y + 80.0F);
    auto const cursor = Frame();
    ASSERT_TRUE(cursor.cursor_pixels);
    EXPECT_FLOAT_EQ(cursor.cursor_pixels->x, 120.0F);
    EXPECT_FLOAT_EQ(cursor.cursor_pixels->y, 80.0F);
    auto const command = ClickImage(ImTextureID{3});
    ASSERT_TRUE(command.projection_request);
    EXPECT_EQ(*command.projection_request, Camera::Projection::Perspective);
    ExpectNoMouseMotion(command);
  }
}

} // namespace
