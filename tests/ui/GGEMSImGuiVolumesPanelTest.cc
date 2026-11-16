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

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>

#include <gtest/gtest.h>
#include <imgui.h>
#include <imgui_internal.h>

#include "GGEMSVoxelizedTestFixture.hh"

#include "GGEMS/geometry/GGEMSBox.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSVolumeAuthoring.hh"
#include "GGEMS/geometry/GGEMSWorld.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/render/GGEMSGeometryAppearance.hh"
#include "GGEMS/render/GGEMSGeometryScene.hh"
#include "GGEMS/ui/detail/GGEMSDeviceStatus.hh"
#include "GGEMS/ui/detail/GGEMSImGuiLayer.hh"
#include "GGEMS/ui/detail/GGEMSImGuiLayout.hh"
#include "GGEMS/ui/detail/GGEMSImGuiTheme.hh"
#include "GGEMS/ui/detail/GGEMSSceneCamera.hh"
#include "GGEMS/ui/detail/GGEMSWorkbenchState.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace {

using namespace ggems::geometry;
using namespace ggems::units;
using ggems::core::materials::builtins::BuildBuiltInMaterial;
using Mode = ggems::render::GGEMSGeometryDisplayMode;
namespace detail = ggems::ui::detail;
namespace render = ggems::render;

auto MakeWorld(std::string name = {}) -> GGEMSWorld {
  GGEMSWorld world{std::move(name)};
  world.SetSize(2_m, 2_m, 2_m).SetMaterial(BuildBuiltInMaterial("Vacuum"));
  return world;
}

auto MakeNestedScene() -> render::GGEMSGeometryScene {
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
  return render::BuildGeometryScene(world);
}

auto MakeNamedScene(bool large = false) -> render::GGEMSGeometryScene {
  GGEMSBox crystal{"Crystal"};
  crystal.SetSize(100_um, 100_um, 100_um)
    .SetMaterial(BuildBuiltInMaterial("Water"));
  GGEMSBox block{"Block"};
  block.SetSize(2_mm, 2_mm, 2_mm).SetMaterial(BuildBuiltInMaterial("Air"));
  if (large) {
    block.LinearRepeater(crystal, {4U, 4U, 4U}, {200_um, 200_um, 200_um});
  } else {
    block.LinearRepeater(crystal, 2U, 200_um, GGEMSRepeatAxis::Z);
  }
  GGEMSBox module{"Module"};
  module.SetSize(20_mm, 20_mm, 20_mm).SetMaterial(BuildBuiltInMaterial("Air"));
  module.LinearRepeater(block, large ? 8U : 2U, 2300_um, GGEMSRepeatAxis::Y);
  GGEMSBox pet{"PET"};
  pet.SetSize(1800_mm, 100_mm, 100_mm).SetMaterial(BuildBuiltInMaterial("Air"));
  pet.LinearRepeater(module, large ? 56U : 2U, 30_mm, GGEMSRepeatAxis::X);
  auto world = MakeWorld();
  world.AddVolume(pet);
  return render::BuildGeometryScene(world);
}

class GGEMSImGuiVolumes : public ::testing::Test {
protected:
  auto SetUp() -> void override {
    context_ = ImGui::CreateContext();
    ImGui::GetStyle() = ggems::ui::BuildGGEMSStyle(1.0F, 1.0F);
    auto &io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    io.ConfigFlags =
      ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_NavEnableKeyboard;
    io.DisplaySize = {1400.0F, 900.0F};
    io.DeltaTime = 1.0F / 60.0F;
    auto &platform = ImGui::GetPlatformIO();
    platform.Platform_ClipboardUserData = &text_;
    platform.Platform_SetClipboardTextFn = [](ImGuiContext *,
                                              char const *text) -> void {
      *static_cast<std::string *>(
        ImGui::GetPlatformIO().Platform_ClipboardUserData) = text;
    };
    ImGui::LoadIniSettingsFromMemory(
      "[Window][###GGEMS.Viewport]\nPos=0,30\nSize=600,820\nCollapsed=0\n\n"
      "[Window][###GGEMS.Volumes]\nPos=620,30\nSize=560,820\nCollapsed=0\n\n");
    workbench_.show_output_panel = false;
    workbench_.show_status_panel = false;
    workbench_.show_camera_panel = false;
    workbench_.show_source_panel = false;
    workbench_.camera.SetViewportSize(600U, 650U);
    Load(MakeNestedScene());
  }

  auto TearDown() -> void override { ImGui::DestroyContext(context_); }

  auto Frame(bool log = false) -> void {
    // Refresh the legacy headless atlas after newly requested font
    // sizes/glyphs.
    unsigned char *pixels{};
    int width{};
    int height{};
    ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
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
    detail::GGEMSDeviceStatusSnapshot const status{};
    layer_.BuildFrame(inputs, status, workbench_);
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

  auto Load(render::GGEMSGeometryScene scene) -> void {
    scene_ = std::move(scene);
    loaded_ = true;
    workbench_.selected_volume_id.reset();
    workbench_.selected_geometry_group.reset();
    workbench_.geometry_appearance.Reset(scene_);
    ++workbench_.geometry_presentation_revision;
    workbench_.camera.FrameWorld(scene_.world_half_extent_m);
    Settle();
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

  [[nodiscard]] auto Table() const -> ImGuiTable * {
    auto const *tree = Child("##volume_tree");
    auto const revision =
      static_cast<int>(workbench_.geometry_presentation_revision);
    auto const seed = ImHashData(&revision, sizeof(revision), tree->IDStack[0]);
    return ImGui::TableFindByID(ImHashStr("##volume_rows", 0U, seed));
  }

  [[nodiscard]] auto Row(int row, int depth, bool arrow = false) const
    -> ImVec2 {
    auto const *table = Table();
    float const height =
      ImGui::GetFrameHeight() + (2.0F * ImGui::GetStyle().CellPadding.y);
    return {table->Columns[0].WorkMinX +
              (static_cast<float>(depth) * ImGui::GetStyle().IndentSpacing) +
              (arrow ? 0.5F * ImGui::GetFontSize()
                     : ImGui::GetTreeNodeToLabelSpacing() +
                         ImGui::GetStyle().ItemSpacing.x +
                         (0.5F * ImGui::GetFontSize())),
            table->OuterRect.Min.y + ImGui::GetStyle().CellPadding.y +
              (static_cast<float>(row) * height) +
              (0.5F * ImGui::GetFrameHeight())};
  }

  [[nodiscard]] auto ButtonBounds(int row, Mode mode) const -> ImRect {
    std::array const names{"None", "Wireframe", "Solid"};
    auto const index = static_cast<std::size_t>(mode);
    float left = Table()->Columns[1].WorkMinX;
    for (std::size_t previous = 0U; previous < index; ++previous) {
      left += ImGui::CalcTextSize(names[previous]).x +
              (2.0F * ImGui::GetStyle().FramePadding.x) +
              ImGui::GetStyle().ItemSpacing.x;
    }
    float const width = ImGui::CalcTextSize(names[index]).x +
                        (2.0F * ImGui::GetStyle().FramePadding.x);
    float const top = Row(row, 0).y - (0.5F * ImGui::GetFrameHeight());
    return {{left, top}, {left + width, top + ImGui::GetFrameHeight()}};
  }

  auto ChangeMode(int row, Mode mode) -> void {
    Click(ButtonBounds(row, mode).GetCenter());
    Settle();
    ASSERT_TRUE(ImGui::GetCurrentContext()->OpenPopupStack.empty());
  }

  [[nodiscard]] auto HasButtonColor(int row, Mode mode, ImU32 color) const
    -> bool {
    auto const bounds = ButtonBounds(row, mode);
    auto const *draw = Child("##volume_tree")->DrawList;
    for (int index = 0; index + 2 < draw->IdxBuffer.Size; index += 3) {
      auto const &first = draw->VtxBuffer[draw->IdxBuffer[index]];
      auto const &second = draw->VtxBuffer[draw->IdxBuffer[index + 1]];
      auto const &third = draw->VtxBuffer[draw->IdxBuffer[index + 2]];
      ImVec2 const center{(first.pos.x + second.pos.x + third.pos.x) / 3.0F,
                          (first.pos.y + second.pos.y + third.pos.y) / 3.0F};
      if (first.col == color && second.col == color && third.col == color &&
          bounds.Contains(center)) {
        return true;
      }
    }
    return false;
  }

  [[nodiscard]] auto HasSelectedFill(int row, Mode mode) const -> bool {
    return HasButtonColor(row, mode,
                          ImGui::GetColorU32(ggems::ui::GetThemeColor(
                            ggems::ui::GGEMSThemeRole::Accent)));
  }

  auto LeaveControls() -> void {
    ImGui::GetIO().AddMousePosEvent(-1000.0F, -1000.0F);
    Settle();
  }

  [[nodiscard]] auto Occurrences() const
    -> std::span<render::GGEMSGeometryAppearance const> {
    return workbench_.geometry_appearance.GetOccurrences();
  }

  auto OpenMother() -> void {
    Click(Row(1, 1, true));
    PauseClicks();
  }

  render::GGEMSGeometryScene scene_;
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
    auto const actual = workbench_.camera.BuildWorldToClipMatrix();
    auto const reference = expected.BuildWorldToClipMatrix();
    EXPECT_EQ(actual.row_0, reference.row_0);
    EXPECT_EQ(actual.row_1, reference.row_1);
    EXPECT_EQ(actual.row_2, reference.row_2);
    EXPECT_EQ(actual.row_3, reference.row_3);
  }
}

TEST_F(GGEMSImGuiVolumes, EmptyAndWorldOnlyScenesRemainInspectable) {
  loaded_ = false;
  Frame(true);
  EXPECT_NE(text_.find("No World loaded yet"), std::string::npos);
  EXPECT_FALSE(workbench_.selected_volume_id);
  Load(render::BuildGeometryScene(MakeWorld()));
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
  Load(render::BuildGeometryScene(world));
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
  io.AddMousePosEvent(tree->Pos.x + 100.0F, tree->Pos.y + height - 1.0F);
  Settle();
  io.AddMouseButtonEvent(ImGuiMouseButton_Left, true);
  Frame();
  io.AddMousePosEvent(tree->Pos.x + 100.0F, tree->Pos.y + height + 34.0F);
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
       LocalButtonsStayAlignedAndPreserveIndependentChildren) {
  OpenMother();
  Click(Row(2, 2));
  auto const before = workbench_.camera.BuildWorldToClipMatrix();
  auto const height = Child("##volume_tree")->ContentSize.y;
  ChangeMode(1, Mode::None);
  EXPECT_EQ(Occurrences()[1].mode_override, Mode::None);
  EXPECT_FALSE(Occurrences()[2].mode_override);
  EXPECT_FALSE(Occurrences()[3].mode_override);
  EXPECT_EQ(workbench_.selected_volume_id, 2U);
  EXPECT_FLOAT_EQ(Child("##volume_tree")->ContentSize.y, height);
  ChangeMode(1, Mode::Solid);
  ChangeMode(2, Mode::Wireframe);
  LeaveControls();
  EXPECT_TRUE(HasSelectedFill(1, Mode::Solid));
  EXPECT_TRUE(HasSelectedFill(2, Mode::Wireframe));
  EXPECT_TRUE(HasSelectedFill(3, Mode::Solid));
  EXPECT_FALSE(HasSelectedFill(1, Mode::None));
  EXPECT_LE(ButtonBounds(3, Mode::Solid).Max.x,
            Child("##volume_tree")->InnerClipRect.Max.x);
  auto const after = workbench_.camera.BuildWorldToClipMatrix();
  EXPECT_EQ(after.row_0, before.row_0);
  EXPECT_EQ(after.row_1, before.row_1);
  EXPECT_EQ(after.row_2, before.row_2);
  EXPECT_EQ(after.row_3, before.row_3);
}

TEST_F(GGEMSImGuiVolumes, ActualThemeSeparatesSelectionHoverPressAndText) {
  using ggems::ui::GetThemeColor;
  using ggems::ui::GGEMSThemeRole;
  auto const accent = GetThemeColor(GGEMSThemeRole::Accent);
  auto const dark = GetThemeColor(GGEMSThemeRole::RecessedBackground);
  auto const luminance = [](ImVec4 color) -> float {
    return (0.2126F * color.x) + (0.7152F * color.y) + (0.0722F * color.z);
  };
  EXPECT_GT((luminance(accent) + 0.05F) / (luminance(dark) + 0.05F), 4.5F);
  EXPECT_NE(ImGui::GetColorU32(accent), ImGui::GetColorU32(ImGuiCol_Button));
  LeaveControls();
  EXPECT_TRUE(HasSelectedFill(0, Mode::Wireframe));
  EXPECT_FALSE(HasSelectedFill(0, Mode::None));
  EXPECT_TRUE(HasButtonColor(0, Mode::Wireframe, ImGui::GetColorU32(dark)));
  auto &io = ImGui::GetIO();
  auto const point = ButtonBounds(0, Mode::Wireframe).GetCenter();
  io.AddMousePosEvent(point.x, point.y);
  Settle();
  EXPECT_TRUE(HasButtonColor(
    0, Mode::Wireframe,
    ImGui::GetColorU32(GetThemeColor(GGEMSThemeRole::AccentStrong))));
  io.AddMouseButtonEvent(ImGuiMouseButton_Left, true);
  Frame();
  EXPECT_TRUE(HasButtonColor(
    0, Mode::Wireframe,
    ImGui::GetColorU32(GetThemeColor(GGEMSThemeRole::Attention))));
  io.AddMouseButtonEvent(ImGuiMouseButton_Left, false);
  Frame();
  auto const inactive = ButtonBounds(0, Mode::None).GetCenter();
  io.AddMousePosEvent(inactive.x, inactive.y);
  Settle();
  EXPECT_FALSE(HasSelectedFill(0, Mode::None));
  EXPECT_TRUE(
    HasButtonColor(0, Mode::None, ImGui::GetColorU32(ImGuiCol_ButtonHovered)));
  EXPECT_FALSE(workbench_.selected_volume_id);
  EXPECT_FALSE(workbench_.selected_geometry_group);
}

TEST_F(GGEMSImGuiVolumes,
       NamedWorldHasOneEnvelopeControlAndNoSolidAtScaledMetrics) {
  Load(render::BuildGeometryScene(MakeWorld("PET / XCAT")));
  ImGui::SetWindowSize(detail::k_volumes_window_name, {340.0F, 820.0F});
  ImGui::GetStyle().ScaleAllSizes(1.5F);
  ImGui::GetStyle().FontScaleMain = 1.5F;
  Settle();
  Frame(true);
  EXPECT_NE(text_.find("PET / XCAT"), std::string::npos);
  EXPECT_NE(text_.find("GGEMSWorld"), std::string::npos);
  EXPECT_EQ(text_.find("Solid"), std::string::npos);
  EXPECT_EQ(text_.find("Project"), std::string::npos);
  Settle();
  ChangeMode(0, Mode::None);
  EXPECT_EQ(Occurrences()[0].mode_override, Mode::None);
  EXPECT_FALSE(workbench_.selected_volume_id);
  ChangeMode(0, Mode::Wireframe);
  EXPECT_EQ(Occurrences()[0].mode_override, Mode::Wireframe);
  EXPECT_LE(ButtonBounds(0, Mode::Wireframe).Max.x,
            Child("##volume_tree")->InnerClipRect.Max.x);
  ImGui::SetWindowSize(detail::k_volumes_window_name, {560.0F, 820.0F});
  Settle();
  Click(Row(1, 1));
  EXPECT_EQ(workbench_.selected_volume_id, 0U);
}

TEST_F(GGEMSImGuiVolumes, LocalModesPersistDuringCameraNavigationAndFraming) {
  OpenMother();
  ChangeMode(1, Mode::None);
  ChangeMode(2, Mode::Wireframe);
  ChangeMode(3, Mode::None);
  auto const *viewport =
    ImGui::FindWindowByName(detail::k_viewport_window_name);
  ImGui::GetIO().AddMousePosEvent(viewport->Pos.x + 200.0F,
                                  viewport->Pos.y + 200.0F);
  Settle();
  auto const zoom = workbench_.camera.GetDiagnostics().zoom;
  ImGui::GetIO().AddMouseWheelEvent(0.0F, 1.0F);
  Settle();
  EXPECT_NE(workbench_.camera.GetDiagnostics().zoom, zoom);
  PauseClicks();
  Click(Row(2, 2));
  Click(Row(2, 2));
  EXPECT_EQ(workbench_.selected_volume_id, 2U);
  EXPECT_EQ(Occurrences()[1].mode_override, Mode::None);
  EXPECT_EQ(Occurrences()[2].mode_override, Mode::Wireframe);
  EXPECT_EQ(Occurrences()[3].mode_override, Mode::None);
  Frame(true);
  EXPECT_EQ(text_.find("Automatic"), std::string::npos);
  EXPECT_EQ(text_.find("Appearance"), std::string::npos);
  EXPECT_NE(text_.find("Production Cuts: not available in this scene snapshot"),
            std::string::npos);
}

TEST_F(GGEMSImGuiVolumes,
       NamedGroupsApplyLastCommandWinsAndShowActualMixedState) {
  Load(MakeNamedScene());
  ASSERT_EQ(scene_.groups.size(), 4U);
  ASSERT_EQ(scene_.occurrences.size(), 16U);
  ASSERT_EQ(scene_.groups[0].name, "PET");
  ASSERT_EQ(scene_.groups[1].name, "Module");
  ASSERT_EQ(scene_.groups[2].name, "Block");
  ASSERT_EQ(scene_.groups[3].name, "Crystal");
  EXPECT_FALSE(workbench_.geometry_appearance.GetGroupMode(0U));
  ChangeMode(1, Mode::None);
  for (std::size_t id = 1U; id < Occurrences().size(); ++id) {
    EXPECT_EQ(Occurrences()[id].mode_override, Mode::None);
  }
  // Individual copies are reached only through the physical hierarchy.
  for (int row = 5; row <= 10; ++row) {
    Click(Row(row, row - 3, true));
    PauseClicks();
  }
  auto *tree = Child("##volume_tree");
  ImGui::SetScrollY(tree, tree->ScrollMax.y);
  Settle();
  ChangeMode(11, Mode::Solid);
  EXPECT_EQ(Occurrences()[4].mode_override, Mode::Solid);
  for (std::size_t id = 1U; id < Occurrences().size(); ++id) {
    if (id != 4U) {
      EXPECT_EQ(Occurrences()[id].mode_override, Mode::None);
    }
  }
  EXPECT_FALSE(workbench_.geometry_appearance.GetGroupMode(0U));
  ImGui::SetScrollY(tree, 0.0F);
  Settle();
  LeaveControls();
  for (auto const mode : {Mode::None, Mode::Wireframe, Mode::Solid}) {
    EXPECT_FALSE(HasSelectedFill(1, mode));
  }
  ChangeMode(3, Mode::Wireframe);
  for (std::size_t id = 1U; id < Occurrences().size(); ++id) {
    auto expected = id == 4U ? Mode::Solid : Mode::None;
    if (scene_.occurrences[id].name == "Block") {
      expected = Mode::Wireframe;
    }
    EXPECT_EQ(Occurrences()[id].mode_override, expected);
  }
  ChangeMode(1, Mode::Solid);
  for (std::size_t id = 1U; id < Occurrences().size(); ++id) {
    EXPECT_EQ(Occurrences()[id].mode_override, Mode::Solid);
  }
  EXPECT_EQ(workbench_.geometry_appearance.GetGroupMode(0U), Mode::Solid);
  EXPECT_FALSE(Occurrences()[0].mode_override);
  EXPECT_FALSE(workbench_.selected_volume_id);
}

TEST_F(GGEMSImGuiVolumes, EqualDisplayNamesAndMarkupStayIndependent) {
  auto world = MakeWorld();
  GGEMSBox first{"Detector##%s"};
  first.SetSize(10_mm, 10_mm, 10_mm)
    .SetPosition(MakePositionPM(-20'000'000'000LL, 0, 0))
    .SetMaterial(BuildBuiltInMaterial("Water"));
  GGEMSBox second{"Detector##%s"};
  second.SetSize(10_mm, 10_mm, 10_mm)
    .SetPosition(MakePositionPM(20'000'000'000LL, 0, 0))
    .SetMaterial(BuildBuiltInMaterial("Water"));
  world.AddVolume(first);
  world.AddVolume(second);
  Load(render::BuildGeometryScene(world));
  ASSERT_EQ(scene_.groups.size(), 2U);
  ChangeMode(1, Mode::None);
  EXPECT_EQ(Occurrences()[1].mode_override, Mode::None);
  Click(Row(1, 1));
  EXPECT_EQ(workbench_.selected_geometry_group, 0U);
  Click(Row(3, 1));
  EXPECT_EQ(workbench_.selected_geometry_group, 1U);
  EXPECT_FALSE(Occurrences()[2].mode_override);
  ChangeMode(3, Mode::Wireframe);
  EXPECT_EQ(Occurrences()[2].mode_override, Mode::Wireframe);
  EXPECT_EQ(Occurrences()[1].mode_override, Mode::None);
  Frame(true);
  auto const first_name = text_.find("Detector##%s");
  ASSERT_NE(first_name, std::string::npos);
  EXPECT_NE(text_.find("Detector##%s", first_name + 1U), std::string::npos);
  EXPECT_FALSE(workbench_.selected_volume_id);
}

TEST_F(GGEMSImGuiVolumes, SummarySelectionInspectsTrueGroupWithoutPhysicalID) {
  Load(MakeNamedScene());
  auto const before = workbench_.camera.BuildWorldToClipMatrix();
  auto const height = Child("##volume_tree")->ContentSize.y;
  Click(Row(4, 2));
  ASSERT_EQ(workbench_.selected_geometry_group, 3U);
  EXPECT_FALSE(workbench_.selected_volume_id);
  Frame(true);
  EXPECT_NE(text_.find("Controlled occurrences: 8"), std::string::npos);
  EXPECT_NE(text_.find("Authored roots (global): 8"), std::string::npos);
  EXPECT_NE(text_.find("Physical parents: 4"), std::string::npos);
  EXPECT_NE(text_.find("Root placement: Linear"), std::string::npos);
  EXPECT_NE(text_.find("Roots per parent: 2"), std::string::npos);
  EXPECT_NE(text_.find("Linear counts (X, Y, Z): (1, 1, 2)"),
            std::string::npos);
  EXPECT_NE(text_.find("Material: Water; density"), std::string::npos);
  EXPECT_NE(text_.find("Dimensions (Box)"), std::string::npos);
  EXPECT_EQ(text_.find("VolumeID:"), std::string::npos);
  Settle();
  EXPECT_FLOAT_EQ(Child("##volume_tree")->ContentSize.y, height);
  ChangeMode(2, Mode::None);
  EXPECT_EQ(workbench_.selected_geometry_group, 3U);
  Click(Row(1, 1, true));
  PauseClicks();
  EXPECT_EQ(workbench_.selected_geometry_group, 3U);
  EXPECT_FALSE(workbench_.selected_volume_id);
  Click(Row(1, 1, true));
  PauseClicks();
  Click(Row(1, 1));
  EXPECT_EQ(workbench_.selected_geometry_group, 0U);
  Frame(true);
  EXPECT_NE(text_.find("Controlled occurrences: 15"), std::string::npos);
  EXPECT_NE(text_.find("Material: Air; density"), std::string::npos);
  EXPECT_NE(text_.find("Material: Water; density"), std::string::npos);
  EXPECT_NE(text_.find("Production Cuts: not available in this scene snapshot"),
            std::string::npos);
  EXPECT_EQ(text_.find("VolumeID:"), std::string::npos);
  Settle();
  Click(Row(5, 2));
  EXPECT_EQ(workbench_.selected_volume_id, 1U);
  EXPECT_FALSE(workbench_.selected_geometry_group);
  auto const after = workbench_.camera.BuildWorldToClipMatrix();
  EXPECT_EQ(after.row_0, before.row_0);
  EXPECT_EQ(after.row_1, before.row_1);
  EXPECT_EQ(after.row_2, before.row_2);
  EXPECT_EQ(after.row_3, before.row_3);
}

TEST_F(GGEMSImGuiVolumes, LargeNamedSceneKeepsPhysicalTreeCollapsed) {
  Load(MakeNamedScene(true));
  ASSERT_EQ(scene_.occurrences.size(), 29'178U);
  ASSERT_EQ(scene_.groups.size(), 4U);
  EXPECT_EQ(scene_.groups[1].members.size(), 56U);
  EXPECT_EQ(scene_.groups[2].members.size(), 448U);
  EXPECT_EQ(scene_.groups[3].members.size(), 28'672U);
  auto const *membership = scene_.groups[3].members.data();
  auto const height = Child("##volume_tree")->ContentSize.y;
  EXPECT_LT(height, 8.0F * (ImGui::GetFrameHeight() +
                            (2.0F * ImGui::GetStyle().CellPadding.y)));
  for (int frame = 0; frame < 6; ++frame) {
    Frame();
    EXPECT_EQ(scene_.groups[3].members.data(), membership);
    EXPECT_FLOAT_EQ(Child("##volume_tree")->ContentSize.y, height);
  }
  ChangeMode(4, Mode::None);
  EXPECT_EQ(workbench_.geometry_appearance.GetGroupMode(3U), Mode::None);
  EXPECT_FALSE(workbench_.geometry_appearance.GetGroupMode(0U));
  EXPECT_FALSE(workbench_.selected_volume_id);
  EXPECT_FLOAT_EQ(Child("##volume_tree")->ContentSize.y, height);
  Click(Row(4, 2, true));
  Settle();
  EXPECT_FLOAT_EQ(Child("##volume_tree")->ContentSize.y, height);
  EXPECT_FALSE(workbench_.selected_geometry_group);
  Click(Row(4, 2));
  EXPECT_EQ(workbench_.selected_geometry_group, 3U);
  EXPECT_FALSE(workbench_.selected_volume_id);
  EXPECT_FLOAT_EQ(Child("##volume_tree")->ContentSize.y, height);
}

TEST_F(GGEMSImGuiVolumes, RightClickNeverOpensNamedOrPhysicalContextMenus) {
  Load(MakeNamedScene());
  auto const before = workbench_.camera.BuildWorldToClipMatrix();
  for (auto const point : {Row(0, 0), Row(1, 1), Row(5, 2)}) {
    Click(point, ImGuiMouseButton_Right);
    EXPECT_TRUE(ImGui::GetCurrentContext()->OpenPopupStack.empty());
    EXPECT_FALSE(workbench_.selected_volume_id);
  }
  for (auto const &appearance : Occurrences()) {
    EXPECT_FALSE(appearance.mode_override);
  }
  auto const after = workbench_.camera.BuildWorldToClipMatrix();
  EXPECT_EQ(after.row_0, before.row_0);
  EXPECT_EQ(after.row_1, before.row_1);
  EXPECT_EQ(after.row_2, before.row_2);
  EXPECT_EQ(after.row_3, before.row_3);
}

TEST_F(GGEMSImGuiVolumes, VoxelMappingsRemainIndependentUnderGlobalControls) {
  std::array<std::uint16_t, 8> const labels{0U, 1U, 2U, 3U, 0U, 1U, 2U, 3U};
  ggems::test::GGEMSVoxelizedTestFixture const fixture{
    {2U, 2U, 2U},
    "1 2 3",
    labels,
    "0 0 Air\n1 1 Water Solid blue_babyblue\n"
    "2 2 Water Wireframe red_ruby\n3 3 Air None white_white\n"};
  auto world = MakeWorld();
  world.AddVolume(fixture.Import("XCAT"));
  Load(render::BuildGeometryScene(world));
  ASSERT_EQ(scene_.groups.size(), 1U);
  // XCAT header, then one control-free selectable physical voxel row.
  float const collapsed = Child("##volume_tree")->ContentSize.y;
  EXPECT_LT(collapsed, 4.0F * (ImGui::GetFrameHeight() +
                               (2.0F * ImGui::GetStyle().CellPadding.y)));
  Click(Row(2, 2, true));
  PauseClicks();
  EXPECT_GT(Child("##volume_tree")->ContentSize.y, collapsed);
  Click(Row(2, 2));
  ASSERT_EQ(workbench_.selected_volume_id, 1U);
  auto expected = workbench_.camera;
  expected.FrameBounds(scene_.occurrences[1].minimum_m,
                       scene_.occurrences[1].maximum_m);
  Click(Row(2, 2));
  EXPECT_EQ(workbench_.camera.GetDiagnostics().target_m,
            expected.GetDiagnostics().target_m);
  Frame(true);
  EXPECT_NE(text_.find("Air [0]"), std::string::npos);
  EXPECT_NE(text_.find("Air [3]"), std::string::npos);
  EXPECT_NE(text_.find("Material palette: 2 entries"), std::string::npos);
  Settle();
  LeaveControls();
  for (auto const mode : {Mode::None, Mode::Wireframe, Mode::Solid}) {
    EXPECT_FALSE(HasSelectedFill(2, mode));
  }
  ChangeMode(3, Mode::None);
  EXPECT_EQ(workbench_.geometry_appearance.GetVoxelMappingMode(1U, 0U),
            Mode::None);
  EXPECT_EQ(workbench_.geometry_appearance.GetVoxelMappingMode(1U, 3U),
            Mode::None);
  ChangeMode(6, Mode::Solid);
  EXPECT_EQ(workbench_.geometry_appearance.GetVoxelMappingMode(1U, 3U),
            Mode::Solid);
  EXPECT_EQ(workbench_.geometry_appearance.GetVoxelMappingMode(1U, 0U),
            Mode::None);
  for (auto const mode : {Mode::Wireframe, Mode::None, Mode::Solid}) {
    ChangeMode(1, mode);
    EXPECT_EQ(workbench_.geometry_appearance.GetOccurrenceMode(scene_, 1U),
              mode);
    EXPECT_EQ(workbench_.geometry_appearance.GetVoxelMappingMode(1U, 0U),
              Mode::None);
    EXPECT_EQ(workbench_.geometry_appearance.GetVoxelMappingMode(1U, 1U),
              Mode::Solid);
    EXPECT_EQ(workbench_.geometry_appearance.GetVoxelMappingMode(1U, 2U),
              Mode::Wireframe);
    EXPECT_EQ(workbench_.geometry_appearance.GetVoxelMappingMode(1U, 3U),
              Mode::Solid);
  }
  EXPECT_EQ(workbench_.selected_volume_id, 1U);
  EXPECT_EQ(scene_.occurrences[1].voxel_slices->palette_size, 2U);
  EXPECT_EQ(scene_.occurrences[1].voxel_slices->mappings[0].mode, Mode::Solid);
  Click(Row(2, 2, true));
  PauseClicks();
  EXPECT_FLOAT_EQ(Child("##volume_tree")->ContentSize.y, collapsed);
  Click(Row(1, 1, true));
  PauseClicks();
  EXPECT_LT(Child("##volume_tree")->ContentSize.y, collapsed);
  Click(Row(1, 1, true));
  PauseClicks();
  EXPECT_FLOAT_EQ(Child("##volume_tree")->ContentSize.y, collapsed);
  Load(render::BuildGeometryScene(world));
  EXPECT_FLOAT_EQ(Child("##volume_tree")->ContentSize.y, collapsed);
}

TEST_F(GGEMSImGuiVolumes, ReplacementResetsModesSelectionAndExpansionScope) {
  Load(MakeNamedScene());
  ChangeMode(1, Mode::None);
  Click(Row(4, 2));
  ASSERT_EQ(workbench_.selected_geometry_group, 3U);
  EXPECT_FALSE(workbench_.selected_volume_id);
  auto const old_revision = workbench_.geometry_presentation_revision;
  Load(MakeNamedScene());
  EXPECT_NE(workbench_.geometry_presentation_revision, old_revision);
  EXPECT_FALSE(workbench_.selected_volume_id);
  EXPECT_FALSE(workbench_.selected_geometry_group);
  for (auto const &appearance : Occurrences()) {
    EXPECT_FALSE(appearance.mode_override);
  }
  EXPECT_FALSE(workbench_.geometry_appearance.GetGroupMode(0U));
  EXPECT_LT(Child("##volume_tree")->ContentSize.y,
            8.0F * (ImGui::GetFrameHeight() +
                    (2.0F * ImGui::GetStyle().CellPadding.y)));
}

} // namespace
