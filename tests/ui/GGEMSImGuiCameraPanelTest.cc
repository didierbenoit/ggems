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
#include <cmath>
#include <iomanip>
#include <initializer_list>
#include <sstream>
#include <string>
#include <string_view>

#include <gtest/gtest.h>
#include <imgui.h>

#include "GGEMS/ui/detail/GGEMSDeviceStatus.hh"
#include "GGEMS/ui/detail/GGEMSImGuiLayer.hh"
#include "GGEMS/ui/detail/GGEMSSceneCamera.hh"
#include "GGEMS/ui/detail/GGEMSWorkbenchState.hh"

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
    imgui_io.ConfigFlags = ImGuiConfigFlags_DockingEnable;
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
      .scene_texture_logical_width = workbench_.viewport.width,
      .scene_texture_logical_height = workbench_.viewport.height,
    };
    layer_.BuildFrame(inputs, ggems::ui::detail::GGEMSDeviceStatusSnapshot{},
                      workbench_);
    ImGui::LogFinish();
    ImGui::Render();
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

  ggems::ui::detail::GGEMSWorkbenchState workbench_;
  std::string displayed_text_;

private:
  ImGuiContext *context_{};
  ggems::ui::GGEMSImGuiLayer layer_;
};

TEST_F(GGEMSImGuiCameraDiagnostics,
       ShowsCurrentOrbitAndDistinguishesHoverFromAppliedMotion) {
  ExpectValues("Yaw (deg)", {300.0F});
  ExpectValues("Pitch (deg)", {-25.0F});
  ExpectValues("Target (m)", {0.0F, 0.0F, 0.0F});
  ExpectValues("Zoom", {0.8F});
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

} // namespace
