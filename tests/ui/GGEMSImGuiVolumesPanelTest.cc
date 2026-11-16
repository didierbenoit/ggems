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

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <string>
#include <string_view>

#include <gtest/gtest.h>
#include <imgui.h>
#include <imgui_internal.h>

#include "GGEMSVoxelizedTestFixture.hh"

#include "GGEMS/geometry/GGEMSBox.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSVolumeAuthoring.hh"
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

  auto Click(ImVec2 point, ImGuiMouseButton button = ImGuiMouseButton_Left)
    -> void {
    auto &io = ImGui::GetIO();
    io.AddMousePosEvent(point.x, point.y);
    Frame();
    io.AddMouseButtonEvent(button, true);
    Frame();
    io.AddMouseButtonEvent(button, false);
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

  [[nodiscard]] static auto GroupLabel(int row, int depth) -> ImVec2 {
    auto point = Row(row, depth, true);
    point.x += ImGui::GetFontSize() + (2.0F * ImGui::GetStyle().FramePadding.x);
    return point;
  }

  auto ChangeGroupMode(int row, int depth, std::string_view label,
                       std::size_t mode) -> void {
    std::array const letters{"N", "W", "S"};
    auto point = GroupLabel(row, depth);
    point.x +=
      ImGui::CalcTextSize(label.data(), label.data() + label.size()).x +
      ImGui::GetStyle().ItemSpacing.x - (0.5F * ImGui::GetFontSize());
    for (std::size_t choice = 0U; choice < mode; ++choice) {
      point.x += ImGui::CalcTextSize(letters[choice]).x +
                 (2.0F * ImGui::GetStyle().FramePadding.x) +
                 ImGui::GetStyle().ItemSpacing.x;
    }
    point.x += 0.5F * (ImGui::CalcTextSize(letters[mode]).x +
                       (2.0F * ImGui::GetStyle().FramePadding.x));
    ASSERT_LE(point.x + (0.5F * (ImGui::CalcTextSize(letters[mode]).x +
                                 (2.0F * ImGui::GetStyle().FramePadding.x))),
              Child("##volume_tree")->InnerClipRect.Max.x);
    Click(point);
    Settle();
    ASSERT_TRUE(ImGui::GetCurrentContext()->OpenPopupStack.empty());
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
  EXPECT_EQ(text_.find("Repeated copies:"), std::string::npos);
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

TEST_F(GGEMSImGuiVolumes,
       AuthoredGroupsControl56ModulesAnd448CrystalsWithoutInheritance) {
  using Mode = ggems::render::GGEMSGeometryDisplayMode;
  auto world = MakeWorld();
  GGEMSBox child;
  child.SetSize(2_mm, 2_mm, 2_mm).SetMaterial(BuildBuiltInMaterial("Water"));
  child.SetPosition(MakePositionPM(0, 300'000'000'000LL, 0));
  world.AddVolume(child);
  GGEMSBox leaf;
  leaf.SetSize(250_um, 250_um, 250_um)
    .SetMaterial(BuildBuiltInMaterial("Water"));
  GGEMSBox inner;
  inner.SetSize(500_um, 500_um, 500_um)
    .SetMaterial(BuildBuiltInMaterial("Air"));
  inner.AddVolume(leaf);
  GGEMSBox outer;
  outer.SetSize(12_mm, 12_mm, 12_mm).SetMaterial(BuildBuiltInMaterial("Air"));
  outer.LinearRepeater(inner, 8U, 1_mm, GGEMSRepeatAxis::Y);
  world.LinearRepeater(outer, 56U, 16_mm, GGEMSRepeatAxis::X);
  scene_ = ggems::render::BuildGeometryScene(world);
  workbench_.geometry_appearance.assign(scene_.occurrences.size(), {});
  for (auto const &occurrence : scene_.occurrences) {
    if (occurrence.parent_volume_id != 0U) {
      workbench_.geometry_appearance[occurrence.volume_id].mode_override =
        static_cast<Mode>(occurrence.volume_id % 3U);
    }
  }
  auto const initial_appearance = workbench_.geometry_appearance;
  Settle();
  ASSERT_EQ(scene_.occurrences.size(), 954U);
  auto const outer_recipe = scene_.occurrences[2].repeater_recipe_id;
  auto const inner_recipe = scene_.occurrences[3].repeater_recipe_id;
  ASSERT_NE(outer_recipe, 0U);
  ASSERT_NE(inner_recipe, 0U);
  ASSERT_NE(inner_recipe, outer_recipe);
  ASSERT_EQ(scene_.occurrences.front().children.size(), 57U);
  std::array<std::uint32_t, 56U> local_groups{};
  std::size_t group_count{};
  for (auto const root : scene_.occurrences.front().children) {
    if (root == 1U) {
      continue;
    }
    ASSERT_EQ(scene_.occurrences[root].children.size(), 8U);
    auto const local_group =
      scene_.occurrences[scene_.occurrences[root].children.front()]
        .repeater_group_id;
    EXPECT_NE(local_group, 0U);
    for (std::size_t previous = 0U; previous < group_count; ++previous) {
      EXPECT_NE(local_groups[previous], local_group);
    }
    local_groups[group_count++] = local_group;
    for (auto const member : scene_.occurrences[root].children) {
      ASSERT_EQ(scene_.occurrences[member].children.size(), 1U);
      EXPECT_EQ(scene_.occurrences[member].repeater_recipe_id, inner_recipe);
    }
  }
  EXPECT_EQ(group_count, 56U);
  auto const before = workbench_.camera.BuildWorldToClipMatrix();
  auto const collapsed_height = Child("##volume_tree")->ContentSize.y;
  Frame(true);
  std::string const label{"Box repeated \xC3\x97"
                          "56"};
  auto const label_position = text_.find(label);
  ASSERT_NE(label_position, std::string::npos);
  EXPECT_EQ(text_.find(label, label_position + label.size()),
            std::string::npos);
  EXPECT_FALSE(workbench_.selected_volume_id);
  EXPECT_EQ(text_.find("Repeated copies:"), std::string::npos);
  Settle();
  for (auto const mode :
       {Mode::Solid, Mode::None, Mode::Solid, Mode::Wireframe}) {
    ChangeGroupMode(2, 1, label, static_cast<std::size_t>(mode));
    std::size_t members{};
    for (std::uint32_t index = 0U; index < scene_.occurrences.size(); ++index) {
      auto const &appearance = workbench_.geometry_appearance[index];
      if (scene_.occurrences[index].repeater_recipe_id == outer_recipe) {
        EXPECT_EQ(appearance.mode_override, mode);
        ++members;
      } else {
        EXPECT_EQ(appearance.mode_override,
                  initial_appearance[index].mode_override);
      }
      EXPECT_EQ(scene_.occurrences[index].volume_id, index);
    }
    EXPECT_EQ(members, 56U);
    EXPECT_FALSE(workbench_.selected_volume_id);
    EXPECT_FLOAT_EQ(Child("##volume_tree")->ContentSize.y, collapsed_height);
  }
  auto const after = workbench_.camera.BuildWorldToClipMatrix();
  EXPECT_EQ(after.row_0, before.row_0);
  EXPECT_EQ(after.row_1, before.row_1);
  EXPECT_EQ(after.row_2, before.row_2);
  EXPECT_EQ(after.row_3, before.row_3);

  Click(Row(2, 1, true));
  Settle();
  EXPECT_GT(Child("##volume_tree")->ContentSize.y, collapsed_height);
  Click(Row(3, 2, true));
  PauseClicks();
  auto const before_inner = workbench_.geometry_appearance;
  std::string const inner_label{"Box \xC3\x97"
                                "8 \xC2\xB7 448 total"};
  Frame(true);
  EXPECT_NE(text_.find(inner_label), std::string::npos);
  Settle();
  ASSERT_NO_FATAL_FAILURE(ChangeGroupMode(4, 3, inner_label, 2U));
  std::size_t inner_members{};
  for (std::size_t index = 0U; index < scene_.occurrences.size(); ++index) {
    if (scene_.occurrences[index].repeater_recipe_id == inner_recipe) {
      EXPECT_EQ(workbench_.geometry_appearance[index].mode_override,
                Mode::Solid);
      ++inner_members;
    } else {
      EXPECT_EQ(workbench_.geometry_appearance[index].mode_override,
                before_inner[index].mode_override);
    }
  }
  EXPECT_EQ(inner_members, 448U);
  Click(Row(4, 3, true));
  PauseClicks();
  auto const before_individual = workbench_.geometry_appearance;
  ChangeMode(5, 4, "Wireframe");
  std::size_t solid_roots{};
  std::size_t wireframe_roots{};
  for (std::size_t index = 0U; index < scene_.occurrences.size(); ++index) {
    auto const &actual = workbench_.geometry_appearance[index].mode_override;
    if (index == 3U) {
      EXPECT_EQ(actual, Mode::Wireframe);
    } else {
      EXPECT_EQ(actual, before_individual[index].mode_override);
    }
    if (scene_.occurrences[index].repeater_recipe_id == inner_recipe) {
      solid_roots += actual == Mode::Solid ? 1U : 0U;
      wireframe_roots += actual == Mode::Wireframe ? 1U : 0U;
    }
  }
  EXPECT_EQ(solid_roots, 447U);
  EXPECT_EQ(wireframe_roots, 1U);
  Click(Row(5, 4, true));
  PauseClicks();
  EXPECT_FALSE(workbench_.selected_volume_id);
  PauseClicks();
  Click(Row(3, 2));
  EXPECT_EQ(workbench_.selected_volume_id, 2U);
  Frame(true);
  EXPECT_NE(text_.find("VolumeID: 2"), std::string::npos);
  EXPECT_EQ(text_.find("Repeated copies:"), std::string::npos);
  PauseClicks();
  auto expected = workbench_.camera;
  expected.FrameBounds(scene_.occurrences[4].minimum_m,
                       scene_.occurrences[4].maximum_m);
  Click(Row(6, 5));
  Click(Row(6, 5));
  EXPECT_EQ(workbench_.selected_volume_id, 4U);
  auto const framed = workbench_.camera.BuildWorldToClipMatrix();
  auto const reference = expected.BuildWorldToClipMatrix();
  EXPECT_EQ(framed.row_0, reference.row_0);
  EXPECT_EQ(framed.row_1, reference.row_1);
  EXPECT_EQ(framed.row_2, reference.row_2);
  EXPECT_EQ(framed.row_3, reference.row_3);
  EXPECT_EQ(scene_.occurrences.size(), 954U);
}

TEST_F(GGEMSImGuiVolumes,
       VisibleGroupsExcludeManualAndIndependentSameShapeRepeaters) {
  using Mode = ggems::render::GGEMSGeometryDisplayMode;
  auto world = MakeWorld();
  GGEMSBox child;
  child.SetSize(2_mm, 2_mm, 2_mm)
    .SetMaterial(BuildBuiltInMaterial("Water"))
    .SetPosition(MakePositionPM(-100'000'000'000LL, 0, 0));
  world.AddVolume(child);
  child.SetPosition(MakePositionPM(-60'000'000'000LL, 0, 0));
  world.LinearRepeater(child, 3U, 8_mm, GGEMSRepeatAxis::X);
  child.SetPosition(MakePositionPM(40'000'000'000LL, 0, 0));
  world.CircularRepeater(child, 4U, GGEMSRepeatAxis::Z);
  child.SetPosition(MakePositionPM(0, 80'000'000'000LL, 0));
  world.LinearRepeater(child, 2U, 8_mm, GGEMSRepeatAxis::X);
  scene_ = ggems::render::BuildGeometryScene(world);
  workbench_.geometry_appearance.assign(scene_.occurrences.size(), {});
  ASSERT_EQ(scene_.occurrences.size(), 11U);
  Settle();
  std::array const groups{
    std::array{2U, 3U, 4U, 0U},
    std::array{5U, 6U, 7U, 8U},
    std::array{9U, 10U, 0U, 0U},
  };
  std::array const labels{
    "Box repeated \xC3\x97"
    "3",
    "Box repeated \xC3\x97"
    "4",
    "Box repeated \xC3\x97"
    "2",
  };
  Frame(true);
  for (auto const *label : labels) {
    EXPECT_NE(text_.find(label), std::string::npos);
  }
  Settle();
  for (std::size_t group_index = 0U; group_index < groups.size();
       ++group_index) {
    workbench_.geometry_appearance.assign(scene_.occurrences.size(), {});
    auto const &group = groups[group_index];
    ChangeGroupMode(static_cast<int>(group_index) + 2, 1, labels[group_index],
                    2U);
    for (std::uint32_t index = 0U; index < scene_.occurrences.size(); ++index) {
      auto const &mode = workbench_.geometry_appearance[index].mode_override;
      if (index != 0U && std::ranges::find(group, index) != group.end()) {
        EXPECT_EQ(mode, Mode::Solid);
      } else {
        EXPECT_FALSE(mode);
      }
      EXPECT_EQ(scene_.occurrences[index].volume_id, index);
      EXPECT_EQ(scene_.occurrences[index].parent_volume_id, 0U);
    }
    EXPECT_FALSE(workbench_.selected_volume_id);
  }
}

TEST_F(GGEMSImGuiVolumes,
       AuthoredRecipesSpanFourParentsButIdenticalRecipesStayIndependent) {
  using Mode = ggems::render::GGEMSGeometryDisplayMode;
  GGEMSBox child;
  child.SetSize(2_mm, 2_mm, 2_mm).SetMaterial(BuildBuiltInMaterial("Water"));
  GGEMSBox leaf;
  leaf.SetSize(500_um, 500_um, 500_um)
    .SetMaterial(BuildBuiltInMaterial("Water"));
  child.AddVolume(leaf);
  GGEMSBox mother;
  mother.SetSize(50_mm, 50_mm, 50_mm).SetMaterial(BuildBuiltInMaterial("Air"));
  child.SetPosition(MakePositionPM(-10'000'000'000LL, 0, 0));
  mother.LinearRepeater(child, 3U, 5_mm, GGEMSRepeatAxis::Y);
  child.SetPosition(MakePositionPM(10'000'000'000LL, 0, 0));
  mother.LinearRepeater(child, 3U, 5_mm, GGEMSRepeatAxis::Y);
  child.SetPosition(MakePositionPM(0, 15'000'000'000LL, 0));
  mother.AddVolume(child);
  auto world = MakeWorld();
  world.LinearRepeater(mother, 4U, 100_mm, GGEMSRepeatAxis::X);
  scene_ = ggems::render::BuildGeometryScene(world);
  workbench_.geometry_appearance.assign(scene_.occurrences.size(), {});
  for (auto const &occurrence : scene_.occurrences) {
    if (occurrence.parent_volume_id != 0U) {
      workbench_.geometry_appearance[occurrence.volume_id].mode_override =
        static_cast<Mode>(occurrence.volume_id % 3U);
    }
  }
  auto const initial_appearance = workbench_.geometry_appearance;
  Settle();
  ASSERT_EQ(scene_.occurrences.size(), 61U);
  auto const outer_recipe = scene_.occurrences[1].repeater_recipe_id;
  auto const first_recipe = scene_.occurrences[2].repeater_recipe_id;
  auto const second_recipe = scene_.occurrences[8].repeater_recipe_id;
  ASSERT_NE(outer_recipe, 0U);
  ASSERT_NE(first_recipe, 0U);
  ASSERT_NE(second_recipe, 0U);
  ASSERT_NE(first_recipe, second_recipe);
  ASSERT_NE(first_recipe, outer_recipe);
  ASSERT_NE(second_recipe, outer_recipe);
  EXPECT_EQ(scene_.occurrences[17].repeater_recipe_id, first_recipe);
  EXPECT_NE(scene_.occurrences[17].repeater_group_id,
            scene_.occurrences[2].repeater_group_id);
  std::string const outer_label{"Box repeated \xC3\x97"
                                "4"};
  std::string const inner_label{"Box \xC3\x97"
                                "3 \xC2\xB7 12 total"};
  Frame(true);
  EXPECT_NE(text_.find(outer_label), std::string::npos);
  Settle();
  for (auto const mode : {Mode::None, Mode::Solid, Mode::Wireframe}) {
    ChangeGroupMode(1, 1, outer_label, static_cast<std::size_t>(mode));
    std::size_t outer_members{};
    for (std::size_t index = 0U; index < scene_.occurrences.size(); ++index) {
      if (scene_.occurrences[index].repeater_recipe_id == outer_recipe) {
        EXPECT_EQ(workbench_.geometry_appearance[index].mode_override, mode);
        ++outer_members;
      } else {
        EXPECT_EQ(workbench_.geometry_appearance[index].mode_override,
                  initial_appearance[index].mode_override);
      }
    }
    EXPECT_EQ(outer_members, 4U);
  }
  Click(Row(1, 1, true));
  PauseClicks();
  Click(Row(2, 2, true));
  PauseClicks();
  Frame(true);
  EXPECT_NE(text_.find(inner_label), std::string::npos);
  Settle();
  auto const before_inner = workbench_.geometry_appearance;
  ASSERT_NO_FATAL_FAILURE(ChangeGroupMode(3, 3, inner_label, 2U));
  std::size_t first_members{};
  for (std::size_t index = 0U; index < scene_.occurrences.size(); ++index) {
    if (scene_.occurrences[index].repeater_recipe_id == first_recipe) {
      EXPECT_EQ(workbench_.geometry_appearance[index].mode_override,
                Mode::Solid);
      ++first_members;
    } else {
      EXPECT_EQ(workbench_.geometry_appearance[index].mode_override,
                before_inner[index].mode_override);
    }
  }
  EXPECT_EQ(first_members, 12U);
  auto const before_other_recipe = workbench_.geometry_appearance;
  ASSERT_NO_FATAL_FAILURE(ChangeGroupMode(4, 3, inner_label, 0U));
  std::size_t second_members{};
  for (std::size_t index = 0U; index < scene_.occurrences.size(); ++index) {
    if (scene_.occurrences[index].repeater_recipe_id == second_recipe) {
      EXPECT_EQ(workbench_.geometry_appearance[index].mode_override,
                Mode::None);
      ++second_members;
    } else {
      EXPECT_EQ(workbench_.geometry_appearance[index].mode_override,
                before_other_recipe[index].mode_override);
    }
  }
  EXPECT_EQ(second_members, 12U);
  Click(Row(6, 2, true));
  PauseClicks();
  auto const before_other_parent = workbench_.geometry_appearance;
  ASSERT_NO_FATAL_FAILURE(ChangeGroupMode(7, 3, inner_label, 1U));
  for (std::size_t index = 0U; index < scene_.occurrences.size(); ++index) {
    if (scene_.occurrences[index].repeater_recipe_id == first_recipe) {
      EXPECT_EQ(workbench_.geometry_appearance[index].mode_override,
                Mode::Wireframe);
    } else {
      EXPECT_EQ(workbench_.geometry_appearance[index].mode_override,
                before_other_parent[index].mode_override);
    }
  }
  EXPECT_FALSE(workbench_.geometry_appearance[0].mode_override);
  EXPECT_EQ(scene_.occurrences[2].parent_volume_id, 1U);
  EXPECT_EQ(scene_.occurrences[17].parent_volume_id, 16U);
  EXPECT_FALSE(workbench_.selected_volume_id);
  auto const appearances = workbench_.geometry_appearance;
  auto const camera = workbench_.camera.BuildWorldToClipMatrix();
  auto const expanded_height = Child("##volume_tree")->ContentSize.y;
  Click(Row(1, 1, true));
  Settle();
  EXPECT_LT(Child("##volume_tree")->ContentSize.y, expanded_height);
  for (std::size_t index = 0U; index < appearances.size(); ++index) {
    EXPECT_EQ(workbench_.geometry_appearance[index].mode_override,
              appearances[index].mode_override);
    EXPECT_EQ(scene_.occurrences[index].volume_id, index);
  }
  auto const after = workbench_.camera.BuildWorldToClipMatrix();
  EXPECT_EQ(after.row_0, camera.row_0);
  EXPECT_EQ(after.row_1, camera.row_1);
  EXPECT_EQ(after.row_2, camera.row_2);
  EXPECT_EQ(after.row_3, camera.row_3);
  EXPECT_FALSE(workbench_.selected_volume_id);
}

TEST_F(GGEMSImGuiVolumes, RightClickNeverOpensPhysicalOrRepeaterContextMenus) {
  auto world = MakeWorld();
  GGEMSBox child;
  child.SetSize(2_mm, 2_mm, 2_mm).SetMaterial(BuildBuiltInMaterial("Water"));
  world.LinearRepeater(child, 2U, 10_mm, GGEMSRepeatAxis::X);
  scene_ = ggems::render::BuildGeometryScene(world);
  workbench_.geometry_appearance.assign(scene_.occurrences.size(), {});
  Settle();
  auto const camera = workbench_.camera.BuildWorldToClipMatrix();
  // The World is physical; the next visible row is presentation only.
  std::array const points{Row(0, 0), GroupLabel(1, 1)};
  for (auto const point : points) {
    Click(point, ImGuiMouseButton_Right);
    Settle();
    EXPECT_TRUE(ImGui::GetCurrentContext()->OpenPopupStack.empty());
    Frame(true);
    for (auto const *label : {
           "This occurrence",
           "All siblings",
           "This subtree",
           "This repeater",
           "Repeated copies:",
         }) {
      EXPECT_EQ(text_.find(label), std::string::npos);
    }
    Settle();
    EXPECT_FALSE(workbench_.selected_volume_id);
  }
  Click(Row(1, 1, true));
  Settle();
  Click(Row(2, 2), ImGuiMouseButton_Right);
  Settle();
  EXPECT_TRUE(ImGui::GetCurrentContext()->OpenPopupStack.empty());
  Frame(true);
  for (auto const *label :
       {"This occurrence", "All siblings", "This subtree", "This repeater"}) {
    EXPECT_EQ(text_.find(label), std::string::npos);
  }
  for (auto const &appearance : workbench_.geometry_appearance) {
    EXPECT_FALSE(appearance.mode_override);
  }
  auto const after = workbench_.camera.BuildWorldToClipMatrix();
  EXPECT_EQ(after.row_0, camera.row_0);
  EXPECT_EQ(after.row_1, camera.row_1);
  EXPECT_EQ(after.row_2, camera.row_2);
  EXPECT_EQ(after.row_3, camera.row_3);
}

TEST_F(GGEMSImGuiVolumes, VoxelMetadataShowsPaletteAndFramesTheWholeGrid) {
  ggems::test::GGEMSVoxelizedTestFixture const fixture;
  auto world = MakeWorld();
  world.AddVolume(fixture.Import());
  scene_ = ggems::render::BuildGeometryScene(world);
  workbench_.geometry_appearance.assign(scene_.occurrences.size(), {});
  Settle();
  Click(Row(1, 1));
  Frame(true);
  EXPECT_EQ(workbench_.selected_volume_id, 1U);
  EXPECT_NE(text_.find("VoxelizedVolume"), std::string::npos);
  EXPECT_NE(text_.find("Material palette: 2 entries"), std::string::npos);
  EXPECT_EQ(text_.find("Density:"), std::string::npos);
  EXPECT_EQ(text_.find("Name: Water"), std::string::npos);
  auto expected = workbench_.camera;
  expected.FrameBounds(scene_.occurrences[1].minimum_m,
                       scene_.occurrences[1].maximum_m);
  Click(Row(1, 1));
  EXPECT_EQ(workbench_.camera.GetDiagnostics().target_m,
            expected.GetDiagnostics().target_m);
  EXPECT_EQ(workbench_.camera.BuildWorldToClipMatrix().row_0,
            expected.BuildWorldToClipMatrix().row_0);
}

} // namespace
