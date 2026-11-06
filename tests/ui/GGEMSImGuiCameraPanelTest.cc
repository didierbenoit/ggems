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
 * \brief Checks current-frame camera diagnostics through real ImGui input.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <algorithm>
#include <array>
#include <cfloat>
#include <cmath>
#include <cstddef>
#include <iomanip>
#include <initializer_list>
#include <sstream>
#include <string>
#include <string_view>

#include <gtest/gtest.h>
#include <imgui.h>
#include <imgui_internal.h>

#include "GGEMS/ui/detail/GGEMSDeviceStatus.hh"
#include "GGEMS/ui/detail/GGEMSImGuiLayer.hh"
#include "GGEMS/ui/detail/GGEMSImGuiLayout.hh"
#include "GGEMS/ui/detail/GGEMSImGuiTheme.hh"
#include "GGEMS/ui/detail/GGEMSSceneCamera.hh"
#include "GGEMS/ui/detail/GGEMSWorkbenchState.hh"
#include "GGEMS/render/GGEMSGeometryScene.hh"

namespace {

// The clipboard callback captures text in memory; no native clipboard,
// settings file, graphics device, or window backend participates in these
// tests.
class GGEMSImGuiCameraDiagnostics : public ::testing::Test {
protected:
  auto SetUp() -> void override {
    context_ = ImGui::CreateContext();
    auto &imgui_io = ImGui::GetIO();
    imgui_io.IniFilename = nullptr;
    imgui_io.LogFilename = nullptr;
    imgui_io.ConfigFlags =
      ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_NavEnableKeyboard;
    imgui_io.DisplaySize = {1280.0F, 800.0F};
    imgui_io.DeltaTime = 1.0F / 60.0F;
    unsigned char *pixels{};
    int atlas_width{};
    int atlas_height{};
    imgui_io.Fonts->GetTexDataAsRGBA32(&pixels, &atlas_width, &atlas_height);

    auto &platform_io = ImGui::GetPlatformIO();
    platform_io.Platform_ClipboardUserData = &displayed_text_;
    platform_io.Platform_SetClipboardTextFn = [](ImGuiContext *,
                                                 char const *text) -> void {
      auto *output = static_cast<std::string *>(
        ImGui::GetPlatformIO().Platform_ClipboardUserData);
      *output = text;
    };

    ImGui::LoadIniSettingsFromMemory(
      "[Window][###GGEMS.Viewport]\nPos=0,30\nSize=600,600\n"
      "Collapsed=0\n\n"
      "[Window][###GGEMS.Camera]\nPos=620,30\nSize=640,700\n"
      "Collapsed=0\n\n");
    workbench_.show_output_panel = false;
    workbench_.show_status_panel = false;
    workbench_.show_scene_panel = false;
    workbench_.show_inspector_panel = false;

    for (int frame = 0; frame < 3; ++frame) {
      Frame();
    }
    imgui_io.AddMousePosEvent(100.0F, 100.0F);
    Frame();
  }

  auto TearDown() -> void override { ImGui::DestroyContext(context_); }

  auto Frame() -> void {
    ImGui::NewFrame();
    displayed_text_.clear();
    ImGui::LogToClipboard();
    workbench_.camera.SetViewportSize(workbench_.viewport.width,
                                      workbench_.viewport.height);
    ggems::ui::GGEMSImGuiLayer::FrameInputs const inputs{
      .scene_texture_id = ImTextureID{1},
      .projection_icons = {ImTextureID{2}, ImTextureID{3}},
      .scene_texture_logical_width = workbench_.viewport.width,
      .scene_texture_logical_height = workbench_.viewport.height,
      .geometry_scene = geometry_scene_,
    };
    layer_.BuildFrame(inputs, ggems::ui::detail::GGEMSDeviceStatusSnapshot{},
                      workbench_);
    ImGui::LogFinish();
    ImGui::Render();
  }

  static auto ImageBounds(ImTextureID texture) -> ImRect {
    auto const *window =
      ImGui::FindWindowByName(ggems::ui::detail::k_viewport_window_name);
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

  auto ClickProjection(ImTextureID texture) -> void {
    ImVec2 const center = ImageBounds(texture).GetCenter();
    auto &io = ImGui::GetIO();
    io.AddMousePosEvent(center.x, center.y);
    Frame();
    io.AddMouseButtonEvent(ImGuiMouseButton_Left, true);
    Frame();
    io.AddMouseButtonEvent(ImGuiMouseButton_Left, false);
    Frame();
    io.AddMousePosEvent(100.0F, 120.0F);
    Frame();
  }

  static auto ExpectProjectionSelected(ImTextureID texture, bool selected)
    -> void {
    ImRect bounds = ImageBounds(texture);
    bounds.Expand(3.0F * ImGui::GetFontSize() /
                  ggems::ui::k_ggems_base_font_size);
    auto const *window =
      ImGui::FindWindowByName(ggems::ui::detail::k_viewport_window_name);
    ASSERT_NE(window, nullptr);
    ImU32 const selected_color = ImGui::ColorConvertFloat4ToU32(
      ggems::ui::GetThemeColor(ggems::ui::GGEMSThemeRole::Selection));
    bool const selected_background = std::ranges::any_of(
      window->DrawList->VtxBuffer, [&](ImDrawVert const &vertex) -> bool {
        return vertex.col == selected_color && bounds.Contains(vertex.pos);
      });
    EXPECT_EQ(selected_background, selected);
  }

  // Compare semantic numeric fields without fixing precision or punctuation.
  auto ExpectValues(std::string_view label,
                    std::initializer_list<float> expected) const -> void {
    auto const position = displayed_text_.find(label);
    ASSERT_NE(position, std::string::npos) << displayed_text_;
    auto const start = displayed_text_.find(':', position) + 1U;
    auto values =
      displayed_text_.substr(start, displayed_text_.find('\n', start) - start);
    for (char &character : values) {
      if (character == '(' || character == ')' || character == ',' ||
          character == 'x') {
        character = ' ';
      }
    }
    std::istringstream stream{values};
    for (float const expected_value : expected) {
      float actual{};
      ASSERT_TRUE(static_cast<bool>(stream >> actual)) << values;
      EXPECT_FLOAT_EQ(actual, expected_value) << label;
    }
  }

  auto PressKey(ImGuiKey key) -> void {
    auto &imgui_io = ImGui::GetIO();
    imgui_io.AddKeyEvent(key, true);
    Frame();
    imgui_io.AddKeyEvent(key, false);
    Frame();
  }

  // Navigate real menu items. No camera setters or synthetic widget return
  // values participate in menu activation.
  auto FocusMenuItem(char const *label) -> void {
    PressKey(ImGuiKey_Home);
    for (int attempt = 0; attempt < 24; ++attempt) {
      auto *window = GImGui->NavWindow;
      if (window != nullptr && GImGui->NavId == window->GetID(label)) {
        return;
      }
      PressKey(ImGuiKey_DownArrow);
    }
    FAIL() << "Menu item was not reachable: " << label;
  }

  auto OpenMenu(char const *submenu) -> void {
    PressKey(ImGuiKey_Escape);
    PressKey(ImGuiKey_Escape);
    auto *menu_bar = ImGui::FindWindowByName("##MainMenuBar");
    ASSERT_NE(menu_bar, nullptr);
    auto &imgui_io = ImGui::GetIO();
    imgui_io.AddMousePosEvent(menu_bar->Pos.x +
                                ImGui::GetStyle().WindowPadding.x +
                                (ImGui::CalcTextSize("View").x * 0.5F),
                              menu_bar->Pos.y + (menu_bar->Size.y * 0.5F));
    Frame();
    imgui_io.AddMouseButtonEvent(ImGuiMouseButton_Left, true);
    Frame();
    imgui_io.AddMouseButtonEvent(ImGuiMouseButton_Left, false);
    Frame();
    if (submenu != nullptr) {
      ASSERT_NO_FATAL_FAILURE(FocusMenuItem(submenu));
      PressKey(ImGuiKey_RightArrow);
    }
  }

  auto Activate(std::initializer_list<char const *> path) -> void {
    auto const *item = path.begin();
    char const *submenu = path.size() == 2U ? *item++ : nullptr;
    ASSERT_NO_FATAL_FAILURE(OpenMenu(submenu));
    ASSERT_NO_FATAL_FAILURE(FocusMenuItem(*item));
    PressKey(ImGuiKey_Enter);
  }

  // Menu checkmarks are geometry, not logged text. Inspect the rendered mark
  // column of the focused item without fixing triangle counts or coordinates.
  auto ExpectMenuChecked(char const *label, bool expected) -> void {
    ASSERT_NO_FATAL_FAILURE(FocusMenuItem(label));
    auto *window = GImGui->NavWindow;
    auto const bounds =
      ImGui::WindowRectRelToAbs(window, window->NavRectRel[ImGuiNavLayer_Main]);
    float const mark_left = bounds.Max.x - (1.5F * ImGui::GetFontSize());
    auto const text_color = ImGui::GetColorU32(ImGuiCol_Text);
    auto const &vertices = window->DrawList->VtxBuffer;
    bool const checked =
      std::ranges::any_of(vertices, [&](ImDrawVert const &vertex) -> bool {
        return vertex.col == text_color && vertex.pos.x >= mark_left &&
               vertex.pos.x <= bounds.Max.x && vertex.pos.y >= bounds.Min.y &&
               vertex.pos.y < bounds.Max.y;
      });
    EXPECT_EQ(checked, expected) << label;
  }

  ggems::ui::detail::GGEMSWorkbenchState workbench_;
  std::string displayed_text_;

  ggems::render::GGEMSGeometryScene const *geometry_scene_{};

private:
  ImGuiContext *context_{};
  ggems::ui::GGEMSImGuiLayer layer_;
};

TEST_F(GGEMSImGuiCameraDiagnostics,
       ShowsCurrentOrbitAndDistinguishesHoverFromAppliedMotion) {
  ExpectValues("Yaw (deg)", {300.0F});
  ExpectValues("Pitch (deg)", {-25.0F});
  ExpectValues("Target (m)", {0.0F, 0.0F, 0.0F});
  ExpectValues("Target scale (/m)", {0.8F});
  ExpectValues("Camera viewport (px)",
               {
                 static_cast<float>(workbench_.viewport.width),
                 static_cast<float>(workbench_.viewport.height),
               });

  auto &imgui_io = ImGui::GetIO();
  imgui_io.AddMouseButtonEvent(ImGuiMouseButton_Left, true);
  Frame();
  for (int step = 1; step <= 2; ++step) {
    imgui_io.AddMousePosEvent(100.0F + (20.0F * static_cast<float>(step)),
                              100.0F);
    Frame();
    ExpectValues("ImGui MouseDelta (px)", {20.0F, 0.0F});
    ExpectValues("Applied orbit (px)", {20.0F, 0.0F});
    ExpectValues("Applied pan (px)", {0.0F, 0.0F});
    ExpectValues("Yaw (deg)", {300.0F - (4.0F * static_cast<float>(step))});
    ExpectValues("Pitch (deg)", {-25.0F});
  }
  EXPECT_TRUE(displayed_text_.contains("Held: left yes / middle no"));

  imgui_io.AddMouseButtonEvent(ImGuiMouseButton_Left, false);
  Frame();
  ExpectValues("Applied orbit (px)", {0.0F, 0.0F});
  imgui_io.AddMousePosEvent(100.0F, 100.0F);
  imgui_io.AddMouseButtonEvent(ImGuiMouseButton_Left, true);
  Frame();
  ExpectValues("ImGui MouseDelta (px)", {-40.0F, 0.0F});
  ExpectValues("Applied orbit (px)", {0.0F, 0.0F});
  ExpectValues("Yaw (deg)", {292.0F});
  imgui_io.AddMousePosEvent(90.0F, 100.0F);
  Frame();
  ExpectValues("Applied orbit (px)", {-10.0F, 0.0F});
  ExpectValues("Yaw (deg)", {294.0F});
  ExpectValues("Pitch (deg)", {-25.0F});
}

TEST_F(GGEMSImGuiCameraDiagnostics, HiddenViewportClearsAppliedPanImmediately) {
  auto &imgui_io = ImGui::GetIO();
  imgui_io.AddMouseButtonEvent(ImGuiMouseButton_Middle, true);
  Frame();
  imgui_io.AddMousePosEvent(115.0F, 95.0F);
  Frame();
  ExpectValues("ImGui MouseDelta (px)", {15.0F, -5.0F});
  ExpectValues("Applied pan (px)", {15.0F, -5.0F});
  ExpectValues("Applied orbit (px)", {0.0F, 0.0F});
  EXPECT_TRUE(displayed_text_.contains("Held: left no / middle yes"));

  auto const panned_matrix = workbench_.camera.BuildWorldToClipMatrix();
  workbench_.show_viewport_panel = false;
  imgui_io.AddMousePosEvent(140.0F, 85.0F);
  Frame();
  ExpectValues("ImGui MouseDelta (px)", {25.0F, -10.0F});
  ExpectValues("Applied pan (px)", {0.0F, 0.0F});
  ExpectValues("Applied orbit (px)", {0.0F, 0.0F});
  EXPECT_EQ(workbench_.camera.BuildWorldToClipMatrix().row_0,
            panned_matrix.row_0);
  EXPECT_EQ(workbench_.camera.BuildWorldToClipMatrix().row_1,
            panned_matrix.row_1);

  imgui_io.AddMouseButtonEvent(ImGuiMouseButton_Middle, false);
  Frame();
  ExpectValues("Applied pan (px)", {0.0F, 0.0F});
  EXPECT_TRUE(displayed_text_.contains("Held: left no / middle no"));
}

// Releasing and repositioning between identical drags must not change their
// signed angular response, including at the reported yaw boundaries.
TEST_F(GGEMSImGuiCameraDiagnostics,
       ContinuousAndRepeatedDragsKeepYawDirectionAcrossBoundaries) {
  auto &imgui_io = ImGui::GetIO();
  std::ostringstream trace;
  trace << std::setprecision(9)
        << "pitch,boundary,direction,path,phase,raw_dx,applied_dx,"
           "yaw_before,yaw_after,pitch_before,pitch_after\n";
  float maximum_yaw_increment_error{};

  for (float const pitch : {-5.0F, 0.0F, 5.0F}) {
    for (float const boundary : {180.0F, 360.0F}) {
      for (float const direction : {-1.0F, 1.0F}) {
        SCOPED_TRACE(::testing::Message()
                     << "pitch=" << pitch << ", boundary=" << boundary
                     << ", direction=" << direction);

        imgui_io.AddMouseButtonEvent(ImGuiMouseButton_Left, false);
        imgui_io.AddMousePosEvent(100.0F, 100.0F);
        Frame();
        float const initial_yaw = boundary + direction;
        workbench_.camera.SetOrbitAngles(initial_yaw, pitch);

        auto const Step = [&](char const *path, char const *phase,
                              float expected_raw_dx,
                              float expected_applied_dx) -> void {
          auto const before = workbench_.camera.GetDiagnostics();
          Frame();
          auto const after = workbench_.camera.GetDiagnostics();
          ExpectValues("Applied orbit (px)", {expected_applied_dx, 0.0F});
          ExpectValues("Applied pan (px)", {0.0F, 0.0F});
          EXPECT_FLOAT_EQ(imgui_io.MouseDelta.x, expected_raw_dx);
          EXPECT_FLOAT_EQ(imgui_io.MouseDelta.y, 0.0F);

          auto const field = displayed_text_.find("Applied orbit (px)");
          ASSERT_NE(field, std::string::npos);
          auto const colon = displayed_text_.find(':', field);
          std::istringstream applied_field{displayed_text_.substr(colon + 1U)};
          char opening_parenthesis{};
          float applied_dx{};
          ASSERT_TRUE(static_cast<bool>(applied_field >> opening_parenthesis >>
                                        applied_dx));
          EXPECT_EQ(opening_parenthesis, '(');

          float const increment = after.yaw_degrees - before.yaw_degrees;
          float const increment_error =
            std::abs(increment + (0.20F * applied_dx));
          maximum_yaw_increment_error =
            std::max(maximum_yaw_increment_error, increment_error);
          EXPECT_NEAR(increment, -0.20F * applied_dx, 1.0e-4F);
          EXPECT_FLOAT_EQ(after.pitch_degrees, before.pitch_degrees);
          if (applied_dx != 0.0F) {
            EXPECT_LT(increment * applied_dx, 0.0F);
          }
          trace << pitch << ',' << boundary << ',' << direction << ',' << path
                << ',' << phase << ',' << imgui_io.MouseDelta.x << ','
                << applied_dx << ',' << before.yaw_degrees << ','
                << after.yaw_degrees << ',' << before.pitch_degrees << ','
                << after.pitch_degrees << '\n';
        };

        imgui_io.AddMouseButtonEvent(ImGuiMouseButton_Left, true);
        Step("continuous", "press", 0.0F, 0.0F);
        for (int step = 1; step <= 4; ++step) {
          imgui_io.AddMousePosEvent(
            100.0F + (5.0F * direction * static_cast<float>(step)), 100.0F);
          Step("continuous", "drag", 5.0F * direction, 5.0F * direction);
        }
        imgui_io.AddMouseButtonEvent(ImGuiMouseButton_Left, false);
        Step("continuous", "release", 0.0F, 0.0F);
        auto const continuous = workbench_.camera.BuildWorldToClipMatrix();
        auto const continuous_diagnostics = workbench_.camera.GetDiagnostics();
        EXPECT_NEAR(continuous_diagnostics.yaw_degrees,
                    initial_yaw - (4.0F * direction), 1.0e-4F);

        imgui_io.AddMousePosEvent(100.0F, 100.0F);
        Step("continuous", "reposition", -20.0F * direction, 0.0F);
        workbench_.camera.SetOrbitAngles(initial_yaw, pitch);

        for (int step = 0; step < 4; ++step) {
          imgui_io.AddMouseButtonEvent(ImGuiMouseButton_Left, true);
          Step("repeated", "press", 0.0F, 0.0F);
          imgui_io.AddMousePosEvent(100.0F + (5.0F * direction), 100.0F);
          Step("repeated", "drag", 5.0F * direction, 5.0F * direction);
          imgui_io.AddMouseButtonEvent(ImGuiMouseButton_Left, false);
          Step("repeated", "release", 0.0F, 0.0F);
          imgui_io.AddMousePosEvent(100.0F, 100.0F);
          Step("repeated", "reposition", -5.0F * direction, 0.0F);
        }

        auto const repeated = workbench_.camera.BuildWorldToClipMatrix();
        EXPECT_EQ(repeated.row_0, continuous.row_0);
        EXPECT_EQ(repeated.row_1, continuous.row_1);
        EXPECT_EQ(repeated.row_2, continuous.row_2);
        EXPECT_EQ(repeated.row_3, continuous.row_3);
        EXPECT_FLOAT_EQ(workbench_.camera.GetDiagnostics().yaw_degrees,
                        continuous_diagnostics.yaw_degrees);
      }
    }
  }

  RecordProperty("event_trace_csv", trace.str());
  RecordProperty("maximum_yaw_increment_error_degrees",
                 std::to_string(maximum_yaw_increment_error));
}

// The actual menu items select exact anatomical views and report the resulting
// camera state in the same frame; reset keeps the selected projection.
TEST_F(GGEMSImGuiCameraDiagnostics, ViewMenuAndResetApplyThroughImGui) {
  Activate({"Camera view", "XY"});
  ExpectValues("Yaw (deg)", {270.0F});
  ExpectValues("Pitch (deg)", {0.0F});
  EXPECT_NEAR(workbench_.camera.GetDiagnostics().forward[2], -1.0F, 1.0e-6F);
  Activate({"Camera view", "XZ"});
  ExpectValues("Pitch (deg)", {90.0F});
  EXPECT_NEAR(workbench_.camera.GetDiagnostics().forward[1], -1.0F, 1.0e-6F);
  Activate({"Camera view", "YZ"});
  ExpectValues("Yaw (deg)", {0.0F});
  ExpectValues("Pitch (deg)", {0.0F});
  EXPECT_NEAR(workbench_.camera.GetDiagnostics().forward[0], -1.0F, 1.0e-6F);
  Activate({"Projection", "Perspective"});
  Activate({"Camera view", "Free view"});
  ExpectValues("Yaw (deg)", {300.0F});
  ExpectValues("Pitch (deg)", {-25.0F});
  EXPECT_EQ(workbench_.camera.GetDiagnostics().projection,
            ggems::ui::detail::GGEMSSceneCamera::Projection::Perspective);
  Activate({"Camera view", "XY"});
  Activate({"Reset Camera"});
  ExpectValues("Yaw (deg)", {300.0F});
  ExpectValues("Pitch (deg)", {-25.0F});
  EXPECT_EQ(workbench_.camera.GetDiagnostics().projection,
            ggems::ui::detail::GGEMSSceneCamera::Projection::Perspective);
}

TEST_F(GGEMSImGuiCameraDiagnostics,
       ProjectionControlsShowOnlyActiveModeParameters) {
  EXPECT_TRUE(displayed_text_.contains("Mode: Orthographic"));
  EXPECT_FALSE(displayed_text_.contains("Vertical FOV"));
  auto const before = workbench_.camera.GetDiagnostics();
  Activate({"Projection", "Perspective"});
  EXPECT_TRUE(displayed_text_.contains("Mode: Perspective"));
  ExpectValues("Vertical FOV (deg)", {45.0F});
  EXPECT_TRUE(displayed_text_.contains("Target distance (m)"));
  EXPECT_TRUE(displayed_text_.contains("Near plane (m)"));
  EXPECT_TRUE(displayed_text_.contains("Far plane (m)"));
  EXPECT_EQ(workbench_.camera.GetDiagnostics().target_m, before.target_m);
  EXPECT_FLOAT_EQ(workbench_.camera.GetDiagnostics().zoom, before.zoom);
  auto const perspective = workbench_.camera.BuildWorldToClipMatrix();
  EXPECT_NE(perspective.row_3[0], 0.0F);
  Activate({"Projection", "Orthographic"});
  EXPECT_TRUE(displayed_text_.contains("Mode: Orthographic"));
  EXPECT_FALSE(displayed_text_.contains("Vertical FOV"));
  EXPECT_FALSE(displayed_text_.contains("Target distance (m)"));
  EXPECT_EQ(workbench_.camera.BuildWorldToClipMatrix().row_3[3], 1.0F);
  EXPECT_FLOAT_EQ(workbench_.camera.GetDiagnostics().zoom, before.zoom);
}

TEST_F(GGEMSImGuiCameraDiagnostics,
       ToolbarAndMenuFollowCameraProjectionWithoutMovingTheView) {
  using Camera = ggems::ui::detail::GGEMSSceneCamera;
  auto expect_selection = [&](char const *stage, bool perspective) -> void {
    SCOPED_TRACE(stage);
    // Leave both mouse and restored keyboard hover before checking the
    // persistent selected background rather than ImGui's hover color.
    ImVec2 const center = ImageBounds(ImTextureID{1}).GetCenter();
    ImGui::GetIO().AddMousePosEvent(center.x, center.y);
    Frame();
    ExpectProjectionSelected(ImTextureID{2}, !perspective);
    ExpectProjectionSelected(ImTextureID{3}, perspective);
  };
  auto const initial = workbench_.camera.GetDiagnostics();
  expect_selection("initial orthographic", false);

  ClickProjection(ImTextureID{3});
  EXPECT_EQ(workbench_.camera.GetDiagnostics().projection,
            Camera::Projection::Perspective);
  EXPECT_EQ(workbench_.camera.GetDiagnostics().target_m, initial.target_m);
  EXPECT_FLOAT_EQ(workbench_.camera.GetDiagnostics().zoom, initial.zoom);
  EXPECT_FLOAT_EQ(workbench_.camera.GetDiagnostics().yaw_degrees,
                  initial.yaw_degrees);
  EXPECT_FLOAT_EQ(workbench_.camera.GetDiagnostics().pitch_degrees,
                  initial.pitch_degrees);
  expect_selection("perspective button", true);
  EXPECT_FALSE(displayed_text_.contains("Orthographic projection"));
  EXPECT_FALSE(displayed_text_.contains("Perspective projection"));

  OpenMenu("Projection");
  ExpectMenuChecked("Perspective", true);
  ExpectMenuChecked("Orthographic", false);
  Activate({"Projection", "Orthographic"});
  Frame();
  expect_selection("orthographic menu", false);

  ClickProjection(ImTextureID{3});
  Activate({"Reset Camera"});
  Frame();
  EXPECT_EQ(workbench_.camera.GetDiagnostics().projection,
            Camera::Projection::Perspective);
  expect_selection("reset preserves perspective", true);

  workbench_.show_viewport_panel = false;
  Frame();
  workbench_.show_viewport_panel = true;
  Frame();
  Frame();
  expect_selection("reopened viewport preserves perspective", true);
  ClickProjection(ImTextureID{2});
  EXPECT_EQ(workbench_.camera.GetDiagnostics().projection,
            Camera::Projection::Orthographic);
}

TEST_F(GGEMSImGuiCameraDiagnostics,
       MenusWorkWithoutCameraControlsInDiagnostics) {
  workbench_.show_camera_panel = false;
  Frame();
  Activate({"Camera view", "XY"});
  EXPECT_NEAR(workbench_.camera.GetDiagnostics().forward[2], -1.0F, 1.0e-6F);
  Activate({"Projection", "Perspective"});
  EXPECT_EQ(workbench_.camera.GetDiagnostics().projection,
            ggems::ui::detail::GGEMSSceneCamera::Projection::Perspective);
  EXPECT_FALSE(workbench_.show_camera_panel);

  Activate({"Camera"});
  EXPECT_TRUE(workbench_.show_camera_panel);
  EXPECT_TRUE(displayed_text_.contains("Mode: Perspective"));
  ExpectValues("Yaw (deg)", {270.0F});
  auto *window =
    ImGui::FindWindowByName(ggems::ui::detail::k_camera_window_name);
  ASSERT_NE(window, nullptr);
  auto &imgui_io = ImGui::GetIO();
  imgui_io.AddMousePosEvent(window->Pos.x + (window->Size.x * 0.5F),
                            window->Pos.y + 10.0F);
  Frame();
  imgui_io.AddMouseButtonEvent(ImGuiMouseButton_Left, true);
  Frame();
  imgui_io.AddMouseButtonEvent(ImGuiMouseButton_Left, false);
  Frame();
  // The retained matrix expander affects display only. No camera command is
  // reachable by tabbing through the diagnostic panel.
  for (int attempt = 0; attempt < 8; ++attempt) {
    PressKey(ImGuiKey_Tab);
    EXPECT_EQ(GImGui->NavWindow, window);
    EXPECT_EQ(GImGui->NavId, window->GetID("WorldToClip (4 x 4)"));
  }
}

TEST_F(GGEMSImGuiCameraDiagnostics, MenuCheckmarksFollowCurrentCameraState) {
  auto &grid = workbench_.cartesian_grid;
  grid.visible = {true, false, true};
  auto const visibility = grid.visible;
  OpenMenu("Camera view");
  ExpectMenuChecked("Free view", true);
  for (char const *view : {"XY", "XZ", "YZ"}) {
    ExpectMenuChecked(view, false);
  }
  for (char const *selected : {"XY", "XZ", "YZ"}) {
    Activate({"Camera view", selected});
    OpenMenu("Camera view");
    ExpectMenuChecked("Free view", false);
    for (char const *view : {"XY", "XZ", "YZ"}) {
      ExpectMenuChecked(view, std::string_view{view} == selected);
    }
  }

  workbench_.camera.Pan(15.0F, -5.0F);
  workbench_.camera.ZoomBy(1.0F);
  workbench_.camera.Orbit(360.0F, 0.0F);
  OpenMenu("Camera view");
  ExpectMenuChecked("YZ", true);
  ExpectMenuChecked("Free view", false);

  PressKey(ImGuiKey_Escape);
  PressKey(ImGuiKey_Escape);
  auto &imgui_io = ImGui::GetIO();
  imgui_io.AddMousePosEvent(100.0F, 100.0F);
  Frame();
  imgui_io.AddMouseButtonEvent(ImGuiMouseButton_Left, true);
  Frame();
  imgui_io.AddMousePosEvent(120.0F, 90.0F);
  Frame();
  imgui_io.AddMouseButtonEvent(ImGuiMouseButton_Left, false);
  Frame();
  OpenMenu("Camera view");
  ExpectMenuChecked("Free view", true);
  for (char const *view : {"XY", "XZ", "YZ"}) {
    ExpectMenuChecked(view, false);
  }

  EXPECT_EQ(workbench_.cartesian_grid.reference_plane,
            ggems::ui::detail::GGEMSSceneCamera::ViewPlane::YZ);
  EXPECT_EQ(grid.visible, visibility);

  for (char const *selected : {"Orthographic", "Perspective"}) {
    Activate({"Projection", selected});
    OpenMenu("Projection");
    for (char const *mode : {"Orthographic", "Perspective"}) {
      ExpectMenuChecked(mode, std::string_view{mode} == selected);
    }
  }
}

TEST_F(GGEMSImGuiCameraDiagnostics, PerspectiveViewportRoutesOrbitPanAndWheel) {
  Activate({"Projection", "Perspective"});
  auto &imgui_io = ImGui::GetIO();
  imgui_io.AddMousePosEvent(100.0F, 100.0F);
  Frame();
  auto const before = workbench_.camera.GetDiagnostics();
  imgui_io.AddMouseButtonEvent(ImGuiMouseButton_Left, true);
  Frame();
  imgui_io.AddMousePosEvent(120.0F, 90.0F);
  Frame();
  ExpectValues("Applied orbit (px)", {20.0F, -10.0F});
  EXPECT_NEAR(workbench_.camera.GetDiagnostics().yaw_degrees,
              before.yaw_degrees - 4.0F, 1.0e-4F);
  EXPECT_NEAR(workbench_.camera.GetDiagnostics().pitch_degrees,
              before.pitch_degrees + 2.0F, 1.0e-4F);
  imgui_io.AddMouseButtonEvent(ImGuiMouseButton_Left, false);
  Frame();
  imgui_io.AddMouseButtonEvent(ImGuiMouseButton_Middle, true);
  Frame();
  imgui_io.AddMousePosEvent(130.0F, 100.0F);
  Frame();
  ExpectValues("Applied pan (px)", {10.0F, 10.0F});
  EXPECT_NE(workbench_.camera.GetDiagnostics().target_m, before.target_m);
  imgui_io.AddMouseButtonEvent(ImGuiMouseButton_Middle, false);
  Frame();
  imgui_io.AddMouseWheelEvent(0.0F, 1.0F);
  Frame();
  EXPECT_GT(workbench_.camera.GetDiagnostics().zoom, before.zoom);
  EXPECT_TRUE(displayed_text_.contains("Mode: Perspective"));
}

TEST_F(GGEMSImGuiCameraDiagnostics,
       GridMenuPreservesCameraAndCursorPlaneAcrossEightMasks) {
  using Camera = ggems::ui::detail::GGEMSSceneCamera;
  auto &grid = workbench_.cartesian_grid;
  EXPECT_EQ(grid.reference_plane, Camera::ViewPlane::XY);
  EXPECT_EQ(grid.visible, (std::array{false, false, false}));
  EXPECT_EQ(grid.polar_visible, (std::array{false, false, false}));
  OpenMenu(nullptr);
  auto *menu = GImGui->NavWindow;
  ASSERT_NE(menu, nullptr);
  for (int item = 0; item < 24; ++item) {
    EXPECT_NE(GImGui->NavId, menu->GetID("Show grid"));
    EXPECT_NE(GImGui->NavId, menu->GetID("Cursor plane"));
    PressKey(ImGuiKey_DownArrow);
  }

  std::array<char const *, 3U> const names{"XY", "XZ", "YZ"};
  Activate({"Grids", "XY##Polar"});
  Activate({"Grids", "YZ##Polar"});
  auto const polar_visibility = grid.polar_visible;
  Activate({"Camera view", "YZ"});
  for (char const *projection : {"Orthographic", "Perspective"}) {
    Activate({"Projection", projection});
    auto const before = workbench_.camera.BuildWorldToClipMatrix();
    for (unsigned int mask = 0U; mask < 8U; ++mask) {
      SCOPED_TRACE(::testing::Message() << projection << ", mask=" << mask);
      for (std::size_t index = 0U; index < names.size(); ++index) {
        bool const expected = (mask & (1U << index)) != 0U;
        if (grid.visible[index] != expected) {
          auto expected_visibility = grid.visible;
          expected_visibility[index] = expected;
          Activate({"Grids", names[index]});
          EXPECT_EQ(grid.visible, expected_visibility);
          EXPECT_EQ(grid.polar_visible, polar_visibility);
        }
      }
      OpenMenu("Grids");
      for (std::size_t index = 0U; index < names.size(); ++index) {
        ExpectMenuChecked(names[index], (mask & (1U << index)) != 0U);
      }
      EXPECT_EQ(grid.reference_plane, Camera::ViewPlane::YZ);
      EXPECT_TRUE(displayed_text_.contains(
        (mask & 4U) != 0U ? "YZ grids: Cartesian on / Polar on"
                          : "YZ grids: Cartesian off / Polar on"));
    }
    auto const after = workbench_.camera.BuildWorldToClipMatrix();
    EXPECT_EQ(after.row_0, before.row_0);
    EXPECT_EQ(after.row_1, before.row_1);
    EXPECT_EQ(after.row_2, before.row_2);
    EXPECT_EQ(after.row_3, before.row_3);
  }
}

TEST_F(GGEMSImGuiCameraDiagnostics,
       PolarMenuRetainsCartesianAndCameraAcrossEightMasks) {
  auto &grid = workbench_.cartesian_grid;
  EXPECT_EQ(grid.polar_visible, (std::array{false, false, false}));
  Activate({"Grids", "XY"});
  Activate({"Grids", "YZ"});
  auto const cartesian_visibility = grid.visible;
  std::array<char const *, 3U> const names{
    "XY##Polar",
    "XZ##Polar",
    "YZ##Polar",
  };
  Activate({"Camera view", "YZ"});
  for (char const *projection : {"Orthographic", "Perspective"}) {
    Activate({"Projection", projection});
    auto const before = workbench_.camera.BuildWorldToClipMatrix();
    for (unsigned int mask = 0U; mask < 8U; ++mask) {
      SCOPED_TRACE(::testing::Message() << projection << ", mask=" << mask);
      for (std::size_t index = 0U; index < names.size(); ++index) {
        bool const expected = (mask & (1U << index)) != 0U;
        if (grid.polar_visible[index] != expected) {
          auto expected_visibility = grid.polar_visible;
          expected_visibility[index] = expected;
          Activate({"Grids", names[index]});
          EXPECT_EQ(grid.polar_visible, expected_visibility);
          EXPECT_EQ(grid.visible, cartesian_visibility);
        }
      }
      OpenMenu("Grids");
      EXPECT_TRUE(displayed_text_.contains("Cartesian"));
      EXPECT_TRUE(displayed_text_.contains("Polar"));
      for (std::size_t index = 0U; index < names.size(); ++index) {
        ExpectMenuChecked(names[index], (mask & (1U << index)) != 0U);
      }
    }
    auto const after = workbench_.camera.BuildWorldToClipMatrix();
    EXPECT_EQ(after.row_0, before.row_0);
    EXPECT_EQ(after.row_1, before.row_1);
    EXPECT_EQ(after.row_2, before.row_2);
    EXPECT_EQ(after.row_3, before.row_3);
  }
  Activate({"Camera"});
  EXPECT_FALSE(workbench_.show_camera_panel);
  Frame();
  Activate({"Camera"});
  EXPECT_TRUE(workbench_.show_camera_panel);
  EXPECT_EQ(grid.visible, cartesian_visibility);
  EXPECT_EQ(grid.polar_visible, (std::array{true, true, true}));
}

TEST_F(GGEMSImGuiCameraDiagnostics,
       CameraViewsChangeCursorPlaneWithoutChangingGridVisibility) {
  using Camera = ggems::ui::detail::GGEMSSceneCamera;
  auto &grid = workbench_.cartesian_grid;
  Activate({"Grids", "XY"});
  Activate({"Grids", "YZ"});
  Activate({"Grids", "XZ##Polar"});
  auto const visibility = grid.visible;
  auto const polar_visibility = grid.polar_visible;
  EXPECT_EQ(visibility, (std::array{true, false, true}));
  std::array<char const *, 3U> const names{"XY", "XZ", "YZ"};
  std::array<char const *, 3U> const labels{
    "XY (Z = 0)",
    "XZ (Y = 0)",
    "YZ (X = 0)",
  };
  for (char const *projection : {"Orthographic", "Perspective"}) {
    Activate({"Projection", projection});
    for (std::size_t selected = 0U; selected < labels.size(); ++selected) {
      Activate({"Camera view", names[selected]});
      EXPECT_EQ(static_cast<std::size_t>(grid.reference_plane), selected);
      EXPECT_EQ(grid.visible, visibility);
      EXPECT_EQ(grid.polar_visible, polar_visibility);
      EXPECT_TRUE(displayed_text_.contains(labels[selected]));
    }
    Activate({"Camera view", "Free view"});
    EXPECT_EQ(grid.reference_plane, Camera::ViewPlane::YZ);
    EXPECT_EQ(grid.visible, visibility);
    EXPECT_EQ(grid.polar_visible, polar_visibility);
    Activate({"Reset Camera"});
    EXPECT_EQ(grid.reference_plane, Camera::ViewPlane::YZ);
    EXPECT_EQ(grid.visible, visibility);
    EXPECT_EQ(grid.polar_visible, polar_visibility);
  }
}

TEST_F(GGEMSImGuiCameraDiagnostics,
       CursorReadoutUsesPlaneAndNeverKeepsOldPoint) {
  using Camera = ggems::ui::detail::GGEMSSceneCamera;
  ggems::render::GGEMSGeometryScene const scene{
    .vertices = {},
    .meshes = {},
    .occurrences = {},
    .world_half_extent_m = {0.6, 0.4, 0.3},
  };
  geometry_scene_ = &scene;
  workbench_.camera.FrameWorld(scene.world_half_extent_m);
  workbench_.camera.SetView(Camera::ViewPlane::XY);
  Frame();
  auto *viewport =
    ImGui::FindWindowByName(ggems::ui::detail::k_viewport_window_name);
  ASSERT_NE(viewport, nullptr);
  // ImGui floors queued pointer positions. Use an even logical image extent
  // so its exact geometric center is a representable pointer position.
  ImGui::SetWindowSize(
    ggems::ui::detail::k_viewport_window_name,
    {viewport->Size.x + static_cast<float>(workbench_.viewport.width % 2U),
     viewport->Size.y + static_cast<float>(workbench_.viewport.height % 2U)});
  Frame();
  Frame();
  ASSERT_EQ(workbench_.viewport.width % 2U, 0U);
  ASSERT_EQ(workbench_.viewport.height % 2U, 0U);
  ImVec2 const origin = ImageBounds(ImTextureID{1}).Min;
  auto &io = ImGui::GetIO();

  for (auto const projection :
       {Camera::Projection::Orthographic, Camera::Projection::Perspective}) {
    workbench_.camera.SetProjection(projection);
    for (char const *view : {"XY", "XZ", "YZ"}) {
      Activate({"Camera view", view});
      io.AddMousePosEvent(
        origin.x + (0.5F * static_cast<float>(workbench_.viewport.width)),
        origin.y + (0.5F * static_cast<float>(workbench_.viewport.height)));
      Frame();
      EXPECT_TRUE(
        displayed_text_.contains(std::string{"Cursor plane: "} + view));
      EXPECT_TRUE(displayed_text_.contains("Ray-plane intersection"));
      EXPECT_TRUE(displayed_text_.contains("Cursor world: (0 mm, 0 mm, 0 mm)"))
        << displayed_text_;
      EXPECT_TRUE(displayed_text_.contains("Cursor radius: 0 mm"));
      EXPECT_TRUE(displayed_text_.contains(
        "Cursor azimuth (deg): N/A (origin / below length resolution)"))
        << displayed_text_;
      EXPECT_TRUE(displayed_text_.contains("minor "));
      EXPECT_TRUE(displayed_text_.contains(" / major "));
    }

    // A free orientation may turn parallel to the retained YZ reference.
    workbench_.camera.SetView(Camera::ViewPlane::XY);
    Frame();
    EXPECT_TRUE(displayed_text_.contains("Cursor world: N/A"));
    EXPECT_TRUE(displayed_text_.contains("Cursor radius: N/A"));
    EXPECT_TRUE(displayed_text_.contains("Cursor azimuth (deg): N/A"));
    workbench_.camera.SetView(Camera::ViewPlane::YZ);
    io.AddMousePosEvent(1200.0F, 750.0F);
    Frame();
    EXPECT_TRUE(displayed_text_.contains("Cursor world: N/A"));
    workbench_.show_viewport_panel = false;
    Frame();
    EXPECT_TRUE(displayed_text_.contains("Cursor world: N/A"));
    workbench_.show_viewport_panel = true;
    Frame();
  }
  geometry_scene_ = nullptr;
  Frame();
  EXPECT_TRUE(displayed_text_.contains("Cursor world: N/A"));
  EXPECT_TRUE(displayed_text_.contains("Cursor radius: N/A"));
  EXPECT_TRUE(displayed_text_.contains("Cursor azimuth (deg): N/A"));
  EXPECT_TRUE(displayed_text_.contains("step N/A"));
}

TEST_F(GGEMSImGuiCameraDiagnostics,
       PolarCursorReadoutUsesPhysicalPlaneAndAllAzimuthQuadrants) {
  ggems::render::GGEMSGeometryScene const scene{
    .vertices = {},
    .meshes = {},
    .occurrences = {},
    .world_half_extent_m = {0.6, 0.4, 0.3},
  };
  geometry_scene_ = &scene;
  workbench_.camera.FrameWorld(scene.world_half_extent_m);
  Frame();
  auto *viewport =
    ImGui::FindWindowByName(ggems::ui::detail::k_viewport_window_name);
  ASSERT_NE(viewport, nullptr);
  ImGui::SetWindowSize(
    ggems::ui::detail::k_viewport_window_name,
    {viewport->Size.x + static_cast<float>(workbench_.viewport.width % 2U),
     viewport->Size.y + static_cast<float>(workbench_.viewport.height % 2U)});
  Frame();
  Frame();
  ASSERT_EQ(workbench_.viewport.width % 2U, 0U);
  ASSERT_EQ(workbench_.viewport.height % 2U, 0U);
  ImVec2 const center{
    ImageBounds(ImTextureID{1}).Min.x +
      (0.5F * static_cast<float>(workbench_.viewport.width)),
    ImageBounds(ImTextureID{1}).Min.y +
      (0.5F * static_cast<float>(workbench_.viewport.height)),
  };

  struct View {
    char const *name;
    ImVec2 positive_u;
    ImVec2 positive_v;
  };
  std::array const views{
    View{.name = "XY", .positive_u = {1, 0}, .positive_v = {0, 1}},
    View{.name = "XZ", .positive_u = {1, 0}, .positive_v = {0, -1}},
    View{.name = "YZ", .positive_u = {0, 1}, .positive_v = {-1, 0}},
  };
  struct Position {
    float u_mm;
    float v_mm;
    float azimuth_degrees;
  };
  std::array const positions{
    Position{.u_mm = 3, .v_mm = 4, .azimuth_degrees = 53.130F},
    Position{.u_mm = -3, .v_mm = 4, .azimuth_degrees = 126.870F},
    Position{.u_mm = -3, .v_mm = -4, .azimuth_degrees = 233.130F},
    Position{.u_mm = 3, .v_mm = -4, .azimuth_degrees = 306.870F},
    Position{.u_mm = 5, .v_mm = 0, .azimuth_degrees = 0},
    Position{.u_mm = 0, .v_mm = 5, .azimuth_degrees = 90},
    Position{.u_mm = -5, .v_mm = 0, .azimuth_degrees = 180},
    Position{.u_mm = 0, .v_mm = -5, .azimuth_degrees = 270},
  };

  for (char const *projection : {"Orthographic", "Perspective"}) {
    Activate({"Projection", projection});
    for (auto const &view : views) {
      Activate({"Camera view", view.name});
      // One logical pixel represents one millimeter at the reference plane.
      workbench_.camera.SetZoom(
        2.0F / (0.001F * static_cast<float>(workbench_.viewport.height)));
      for (auto const &position : positions) {
        SCOPED_TRACE(::testing::Message()
                     << projection << ", " << view.name
                     << ", azimuth=" << position.azimuth_degrees);
        ImGui::GetIO().AddMousePosEvent(
          center.x + (position.u_mm * view.positive_u.x) +
            (position.v_mm * view.positive_v.x),
          center.y + (position.u_mm * view.positive_u.y) +
            (position.v_mm * view.positive_v.y));
        Frame();
        EXPECT_TRUE(displayed_text_.contains("Cursor radius: 5.000 mm"))
          << displayed_text_;
        ExpectValues("Cursor azimuth (deg)", {position.azimuth_degrees});
      }
    }
  }
}

TEST_F(GGEMSImGuiCameraDiagnostics,
       GridStepsBelowUnitResolutionAreNeverLabeledZero) {
  ggems::render::GGEMSGeometryScene const scene{
    .vertices = {},
    .meshes = {},
    .occurrences = {},
    .world_half_extent_m = {1.0e-9, 1.0e-9, 1.0e-9},
  };
  geometry_scene_ = &scene;
  workbench_.camera.FrameWorld(scene.world_half_extent_m);
  workbench_.camera.SetView(ggems::ui::detail::GGEMSSceneCamera::ViewPlane::XY);
  workbench_.camera.SetZoom(1.0e19F);
  Frame();
  EXPECT_TRUE(displayed_text_.contains("minor <1 pm / major <1 pm"))
    << displayed_text_;
  EXPECT_TRUE(displayed_text_.contains("Cursor radius: 0 mm"));
  EXPECT_TRUE(displayed_text_.contains(
    "Cursor azimuth (deg): N/A (origin / below length resolution)"));
}

} // namespace
