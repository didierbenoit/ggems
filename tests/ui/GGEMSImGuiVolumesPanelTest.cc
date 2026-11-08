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
 * \brief Checks physical tree selection, double-click framing, and split input.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <cstdint>
#include <initializer_list>
#include <string>
#include <string_view>

#include <gtest/gtest.h>
#include <imgui.h>
#include <imgui_internal.h>

#include "GGEMS/geometry/GGEMSBox.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSWorld.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/render/GGEMSGeometryScene.hh"
#include "GGEMS/render/GGEMSGeometryAppearance.hh"
#include "GGEMS/ui/detail/GGEMSDeviceStatus.hh"
#include "GGEMS/ui/detail/GGEMSImGuiLayer.hh"
#include "GGEMS/ui/detail/GGEMSImGuiLayout.hh"
#include "GGEMS/ui/detail/GGEMSSceneCamera.hh"
#include "GGEMS/ui/detail/GGEMSWorkbenchState.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace {

using namespace ggems::geometry;
using namespace ggems::units;
using ggems::core::materials::builtins::BuildBuiltInMaterial;
namespace detail = ggems::ui::detail;

auto MakeWorld() -> GGEMSWorld {
  GGEMSWorld world;
  world.SetSize(1_m, 1_m, 1_m).SetMaterial(BuildBuiltInMaterial("Vacuum"));
  return world;
}

auto MakeNestedScene() -> ggems::render::GGEMSGeometryScene {
  auto world = MakeWorld();
  GGEMSBox mother;
  mother.SetSize(200_mm, 200_mm, 200_mm)
    .SetPosition(MakePositionPM(100'000'000'000LL, 0, 0))
    .SetRotation(0_deg, 0_deg, 90_deg)
    .SetMaterial(BuildBuiltInMaterial("Water"));
  world.AddVolume(mother);
  GGEMSBox child;
  child.SetSize(10_mm, 20_mm, 30_mm)
    .SetPosition(MakePositionPM(20'000'000'000LL, 0, 0))
    .SetMaterial(BuildBuiltInMaterial("Aluminum"));
  world.AddVolume(child, 1U);
  child.SetPosition(MakePositionPM(-20'000'000'000LL, 0, 0));
  world.AddVolume(child, 1U);
  world.Validate();
  return ggems::render::BuildGeometryScene(world);
}

class GGEMSImGuiVolumes : public ::testing::Test {
protected:
  auto SetUp() -> void override {
    context_ = ImGui::CreateContext();
    auto &io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    io.ConfigFlags =
      ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_NavEnableKeyboard;
    io.DisplaySize = {1200.0F, 800.0F};
    io.DeltaTime = 1.0F / 60.0F;
    unsigned char *pixels{};
    int width{};
    int height{};
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    auto &platform = ImGui::GetPlatformIO();
    platform.Platform_ClipboardUserData = &text_;
    platform.Platform_SetClipboardTextFn = [](ImGuiContext *,
                                              char const *text) -> void {
      *static_cast<std::string *>(
        ImGui::GetPlatformIO().Platform_ClipboardUserData) = text;
    };
    ImGui::LoadIniSettingsFromMemory(
      "[Window][###GGEMS.Viewport]\nPos=0,30\nSize=600,740\nCollapsed=0\n\n"
      "[Window][###GGEMS.Volumes]\nPos=620,30\nSize=340,740\nCollapsed=0\n\n");
    workbench_.show_output_panel = false;
    workbench_.show_status_panel = false;
    workbench_.show_camera_panel = false;
    workbench_.show_source_panel = false;
    scene_ = MakeNestedScene();
    workbench_.geometry_appearance.resize(scene_.occurrences.size());
    workbench_.camera.SetViewportSize(600U, 650U);
    workbench_.camera.FrameWorld(scene_.world_half_extent_m);
    Settle();
  }

  auto TearDown() -> void override { ImGui::DestroyContext(context_); }

  auto Frame(bool log = false) -> void {
    ImGui::NewFrame();
    if (log) {
      text_.clear();
      ImGui::LogToClipboard();
    }
    ggems::ui::GGEMSImGuiLayer::FrameInputs const inputs{
      .scene_texture_id = ImTextureID{1},
      .scene_texture_logical_width = workbench_.viewport.width,
      .scene_texture_logical_height = workbench_.viewport.height,
      .geometry_scene = loaded_ ? &scene_ : nullptr,
    };
    layer_.BuildFrame(inputs, detail::GGEMSDeviceStatusSnapshot{}, workbench_);
    if (log) {
      ImGui::LogFinish();
    }
    ImGui::Render();
  }

  auto Settle() -> void {
    for (int frame = 0; frame < 3; ++frame) {
      Frame();
    }
  }

  auto Click(ImVec2 point) -> void {
    auto &io = ImGui::GetIO();
    io.AddMousePosEvent(point.x, point.y);
    Frame();
    io.AddMouseButtonEvent(ImGuiMouseButton_Left, true);
    Frame();
    io.AddMouseButtonEvent(ImGuiMouseButton_Left, false);
    Frame();
  }

  auto PauseClicks() -> void {
    for (int frame = 0; frame < 25; ++frame) {
      Frame();
    }
  }

  [[nodiscard]] static auto Child(std::string_view label) -> ImGuiWindow * {
    auto const *panel = ImGui::FindWindowByName(detail::k_volumes_window_name);
    for (auto *child : panel->DC.ChildWindows) {
      if (std::string_view{child->Name}.contains(label)) {
        return child;
      }
    }
    return nullptr;
  }

  [[nodiscard]] static auto Row(int row, int depth, bool arrow = false)
    -> ImVec2 {
    auto const *tree = Child("##volume_tree");
    return {tree->DC.CursorStartPos.x +
              (static_cast<float>(depth) * ImGui::GetStyle().IndentSpacing) +
              (arrow ? 0.5F * ImGui::GetFontSize()
                     : (2.0F * (ImGui::GetFontSize() +
                                (2.0F * ImGui::GetStyle().FramePadding.x))) +
                         (2.0F * ImGui::GetStyle().ItemSpacing.x) +
                         (0.5F * ImGui::GetFontSize())),
            tree->DC.CursorStartPos.y +
              (static_cast<float>(row) * ImGui::GetFrameHeightWithSpacing()) +
              (0.5F * ImGui::GetFrameHeight())};
  }

  [[nodiscard]] static auto ModeControl(int row, int depth) -> ImVec2 {
    auto point = Row(row, depth, true);
    float const cell =
      ImGui::GetFontSize() + (2.0F * ImGui::GetStyle().FramePadding.x);
    point.x += (1.5F * cell) + ImGui::GetStyle().ItemSpacing.x -
               (0.5F * ImGui::GetFontSize());
    return point;
  }

  auto PressKey(ImGuiKey key) -> void {
    ImGui::GetIO().AddKeyEvent(key, true);
    Frame();
    ImGui::GetIO().AddKeyEvent(key, false);
    Frame();
  }

  auto SelectPopup(char const *label) -> void {
    auto &context = *ImGui::GetCurrentContext();
    ASSERT_FALSE(context.OpenPopupStack.empty());
    auto *popup = context.OpenPopupStack.back().Window;
    ASSERT_NE(popup, nullptr);
    auto const target = popup->GetID(label);
    PressKey(ImGuiKey_Home);
    for (int step = 0; step < 16 && context.NavId != target; ++step) {
      PressKey(ImGuiKey_DownArrow);
    }
    ASSERT_EQ(context.NavId, target);
    PressKey(ImGuiKey_Enter);
    Settle();
  }

  auto ChangeMode(int row, int depth, char const *label) -> void {
    Click(ModeControl(row, depth));
    SelectPopup(label);
  }

  auto OpenMother() -> void {
    Click(Row(1, 1, true));
    PauseClicks();
  }

  ggems::render::GGEMSGeometryScene scene_;
  detail::GGEMSWorkbenchState workbench_;
  bool loaded_{true};
  std::string text_;

private:
  ImGuiContext *context_{};
  ggems::ui::GGEMSImGuiLayer layer_;
};

TEST_F(GGEMSImGuiVolumes,
       SingleClickSelectsOnlyOnePhysicalOccurrenceWithoutMovingCamera) {
  OpenMother();
  auto const before = workbench_.camera.BuildWorldToClipMatrix();
  Click(Row(2, 2));
  EXPECT_EQ(workbench_.selected_volume_id, 2U);
  auto const after = workbench_.camera.BuildWorldToClipMatrix();
  EXPECT_EQ(after.row_0, before.row_0);
  EXPECT_EQ(after.row_1, before.row_1);
  EXPECT_EQ(after.row_2, before.row_2);
  EXPECT_EQ(after.row_3, before.row_3);
  PauseClicks();
  Click(Row(3, 2));
  EXPECT_EQ(workbench_.selected_volume_id, 3U);
  Frame(true);
  EXPECT_NE(text_.find("VolumeID: 3"), std::string::npos);
  EXPECT_NE(text_.find("Parent VolumeID: 1"), std::string::npos);
  EXPECT_NE(text_.find("Aluminum"), std::string::npos);
  EXPECT_NE(text_.find("World position"), std::string::npos);
  EXPECT_NE(text_.find("World orientation"), std::string::npos);
}

TEST_F(GGEMSImGuiVolumes,
       DoubleClickFramesOnlySelectedNestedOccurrenceInBothProjections) {
  OpenMother();
  for (auto const projection : {
         detail::GGEMSSceneCamera::Projection::Orthographic,
         detail::GGEMSSceneCamera::Projection::Perspective,
       }) {
    workbench_.camera.Reset();
    workbench_.camera.SetProjection(projection);
    PauseClicks();
    auto expected = workbench_.camera;
    expected.FrameBounds(scene_.occurrences[2].minimum_m,
                         scene_.occurrences[2].maximum_m);
    Click(Row(2, 2));
    Click(Row(2, 2));
    EXPECT_EQ(workbench_.selected_volume_id, 2U);
    auto const actual_matrix = workbench_.camera.BuildWorldToClipMatrix();
    auto const expected_matrix = expected.BuildWorldToClipMatrix();
    EXPECT_EQ(actual_matrix.row_0, expected_matrix.row_0);
    EXPECT_EQ(actual_matrix.row_1, expected_matrix.row_1);
    EXPECT_EQ(actual_matrix.row_2, expected_matrix.row_2);
    EXPECT_EQ(actual_matrix.row_3, expected_matrix.row_3);
  }
}

TEST_F(GGEMSImGuiVolumes, EmptyAndWorldOnlyScenesRemainInspectable) {
  loaded_ = false;
  Frame(true);
  EXPECT_NE(text_.find("No World loaded yet"), std::string::npos);
  EXPECT_FALSE(workbench_.selected_volume_id);
  scene_ = ggems::render::BuildGeometryScene(MakeWorld());
  workbench_.geometry_appearance.assign(scene_.occurrences.size(), {});
  loaded_ = true;
  Settle();
  Click(Row(0, 0));
  EXPECT_EQ(workbench_.selected_volume_id, 0U);
  Frame(true);
  EXPECT_NE(text_.find("Parent: none (World root)"), std::string::npos);
  EXPECT_NE(text_.find("Vacuum"), std::string::npos);
  EXPECT_NE(text_.find("Dimensions"), std::string::npos);
}

TEST_F(GGEMSImGuiVolumes,
       TreeScrollAndSplitResizeLeaveCameraAndSelectionUntouched) {
  auto world = MakeWorld();
  GGEMSBox box;
  box.SetSize(1_mm, 1_mm, 1_mm).SetMaterial(BuildBuiltInMaterial("Water"));
  for (std::int64_t index = 0; index < 100; ++index) {
    box.SetPosition(MakePositionPM(0, (index - 50) * 5'000'000'000LL, 0));
    world.AddVolume(box);
  }
  world.Validate();
  scene_ = ggems::render::BuildGeometryScene(world);
  workbench_.geometry_appearance.assign(scene_.occurrences.size(), {});
  workbench_.selected_volume_id.reset();
  Settle();
  auto *tree = Child("##volume_tree");
  ASSERT_NE(tree, nullptr);
  EXPECT_GT(tree->ScrollMax.y, 0.0F);
  auto const before = workbench_.camera.BuildWorldToClipMatrix();
  auto &io = ImGui::GetIO();
  io.AddMousePosEvent(tree->Pos.x + 100.0F, tree->Pos.y + 80.0F);
  Settle();
  io.AddMouseWheelEvent(0.0F, -3.0F);
  Settle();
  EXPECT_GT(tree->Scroll.y, 0.0F);
  EXPECT_FLOAT_EQ(Child("##volume_metadata")->Scroll.y, 0.0F);
  float const height = tree->Size.y;
  float const border = tree->Pos.y + height - 1.0F;
  io.AddMousePosEvent(tree->Pos.x + 100.0F, border);
  Settle();
  io.AddMouseButtonEvent(ImGuiMouseButton_Left, true);
  Frame();
  io.AddMousePosEvent(tree->Pos.x + 100.0F, border + 35.0F);
  Frame();
  io.AddMouseButtonEvent(ImGuiMouseButton_Left, false);
  Settle();
  EXPECT_GT(tree->Size.y, height + 20.0F);
  EXPECT_FALSE(workbench_.selected_volume_id);
  auto const after = workbench_.camera.BuildWorldToClipMatrix();
  EXPECT_EQ(after.row_0, before.row_0);
  EXPECT_EQ(after.row_1, before.row_1);
  EXPECT_EQ(after.row_2, before.row_2);
  EXPECT_EQ(after.row_3, before.row_3);
}

TEST_F(GGEMSImGuiVolumes,
       RowModeControlsNeverSelectExpandOrFocusAndKeepChildrenIndependent) {
  using Mode = ggems::render::GGEMSGeometryDisplayMode;
  OpenMother();
  Click(Row(2, 2));
  ASSERT_EQ(workbench_.selected_volume_id, 2U);
  for (auto const projection : {
         detail::GGEMSSceneCamera::Projection::Orthographic,
         detail::GGEMSSceneCamera::Projection::Perspective,
       }) {
    workbench_.camera.SetProjection(projection);
    auto const before = workbench_.camera.BuildWorldToClipMatrix();
    float const tree_height = Child("##volume_tree")->ContentSize.y;
    ChangeMode(1, 1, "None");
    EXPECT_EQ(workbench_.geometry_appearance[1].mode_override, Mode::None);
    EXPECT_FALSE(workbench_.geometry_appearance[2].mode_override);
    EXPECT_FALSE(workbench_.geometry_appearance[3].mode_override);
    EXPECT_EQ(workbench_.selected_volume_id, 2U);
    EXPECT_FLOAT_EQ(Child("##volume_tree")->ContentSize.y, tree_height);
    auto const after = workbench_.camera.BuildWorldToClipMatrix();
    EXPECT_EQ(after.row_0, before.row_0);
    EXPECT_EQ(after.row_1, before.row_1);
    EXPECT_EQ(after.row_2, before.row_2);
    EXPECT_EQ(after.row_3, before.row_3);
    ChangeMode(1, 1, "Solid");
    EXPECT_EQ(workbench_.geometry_appearance[1].mode_override, Mode::Solid);
    ChangeMode(1, 1, "Wireframe");
    EXPECT_EQ(workbench_.geometry_appearance[1].mode_override, Mode::Wireframe);
  }
  ChangeMode(2, 2, "None");
  PauseClicks();
  Click(Row(3, 2));
  Click(Row(2, 2));
  EXPECT_EQ(workbench_.selected_volume_id, 2U);
  auto expected = workbench_.camera;
  expected.FrameBounds(scene_.occurrences[2].minimum_m,
                       scene_.occurrences[2].maximum_m);
  Click(Row(2, 2));
  EXPECT_EQ(workbench_.camera.GetDiagnostics().target_m,
            expected.GetDiagnostics().target_m);
  EXPECT_EQ(workbench_.geometry_appearance[2].mode_override, Mode::None);
  EXPECT_EQ(workbench_.geometry_appearance[1].mode_override, Mode::Wireframe);
}

TEST_F(GGEMSImGuiVolumes, WorldModePopupHasNoSolidAndWorksAtScaledMetrics) {
  using Mode = ggems::render::GGEMSGeometryDisplayMode;
  ImGui::GetStyle().ScaleAllSizes(1.5F);
  ImGui::GetStyle().FontScaleMain = 1.5F;
  Settle();
  auto const before = workbench_.camera.BuildWorldToClipMatrix();
  Click(ModeControl(0, 0));
  Frame(true);
  EXPECT_NE(text_.find("Wireframe"), std::string::npos);
  EXPECT_EQ(text_.find("Solid"), std::string::npos);
  EXPECT_EQ(text_.find("Automatic"), std::string::npos);
  EXPECT_FALSE(workbench_.selected_volume_id);
  SelectPopup("None");
  EXPECT_EQ(workbench_.geometry_appearance[0].mode_override, Mode::None);
  ChangeMode(0, 0, "Wireframe");
  EXPECT_EQ(workbench_.geometry_appearance[0].mode_override, Mode::Wireframe);
  auto const after = workbench_.camera.BuildWorldToClipMatrix();
  EXPECT_EQ(after.row_0, before.row_0);
  EXPECT_EQ(after.row_1, before.row_1);
  EXPECT_EQ(after.row_2, before.row_2);
  EXPECT_EQ(after.row_3, before.row_3);
}

TEST_F(GGEMSImGuiVolumes,
       LeafModesPersistDuringNavigationSelectionAndFramingWithoutAppearance) {
  using Mode = ggems::render::GGEMSGeometryDisplayMode;
  OpenMother();
  Click(Row(2, 2));
  ASSERT_EQ(workbench_.selected_volume_id, 2U);
  Click(ModeControl(2, 2));
  Frame(true);
  EXPECT_NE(text_.find("None"), std::string::npos);
  EXPECT_NE(text_.find("Wireframe"), std::string::npos);
  EXPECT_NE(text_.find("Solid"), std::string::npos);
  EXPECT_EQ(text_.find("Automatic"), std::string::npos);
  EXPECT_EQ(text_.find("Appearance"), std::string::npos);
  EXPECT_EQ(text_.find("Color"), std::string::npos);
  EXPECT_NE(text_.find("Production Cuts: not available in this scene snapshot"),
            std::string::npos);
  SelectPopup("None");
  EXPECT_EQ(workbench_.geometry_appearance[2].mode_override, Mode::None);
  ChangeMode(2, 2, "Wireframe");
  EXPECT_EQ(workbench_.geometry_appearance[2].mode_override, Mode::Wireframe);
  ChangeMode(2, 2, "Solid");
  EXPECT_EQ(workbench_.geometry_appearance[2].mode_override, Mode::Solid);
  ChangeMode(1, 1, "None");
  ChangeMode(3, 2, "None");

  auto const *viewport =
    ImGui::FindWindowByName(detail::k_viewport_window_name);
  ASSERT_NE(viewport, nullptr);
  auto &io = ImGui::GetIO();
  io.AddMousePosEvent(viewport->Pos.x + (0.5F * viewport->Size.x),
                      viewport->Pos.y + (0.5F * viewport->Size.y));
  Settle();
  auto const zoom = workbench_.camera.GetDiagnostics().zoom;
  io.AddMouseWheelEvent(0.0F, 1.0F);
  Settle();
  EXPECT_NE(workbench_.camera.GetDiagnostics().zoom, zoom);
  EXPECT_EQ(workbench_.geometry_appearance[2].mode_override, Mode::Solid);

  PauseClicks();
  Click(Row(3, 2));
  EXPECT_EQ(workbench_.selected_volume_id, 3U);
  PauseClicks();
  Click(Row(2, 2));
  Click(Row(2, 2));
  EXPECT_EQ(workbench_.selected_volume_id, 2U);
  EXPECT_EQ(workbench_.geometry_appearance[1].mode_override, Mode::None);
  EXPECT_EQ(workbench_.geometry_appearance[2].mode_override, Mode::Solid);
  EXPECT_EQ(workbench_.geometry_appearance[3].mode_override, Mode::None);
}

} // namespace
