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
 * \brief Checks docking admission, migration, and persistence in ImGui frames.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <array>
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <string>

#include <gtest/gtest.h>
#include <imgui.h>
#include <imgui_internal.h>

#include "GGEMS/logging/GGEMSOutputState.hh"
#include "GGEMS/ui/detail/GGEMSDeviceStatus.hh"
#include "GGEMS/ui/detail/GGEMSImGuiLayer.hh"
#include "GGEMS/ui/detail/GGEMSImGuiLayout.hh"
#include "GGEMS/ui/detail/GGEMSWorkbenchState.hh"

namespace {

namespace detail = ggems::ui::detail;

// Real ImGui docking and settings serialization run without files or a native
// backend. A fresh layer and context reproduce application restart admission.
class GGEMSImGuiLayout : public ::testing::Test {
protected:
  auto SetUp() -> void override { Restart({}); }

  auto TearDown() -> void override {
    layer_.reset();
    ImGui::DestroyContext(context_);
  }

  auto Restart(std::string const &settings) -> void {
    layer_.reset();
    if (context_ != nullptr) {
      ImGui::DestroyContext(context_);
    }
    context_ = ImGui::CreateContext();
    auto &io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;
    io.ConfigFlags = ImGuiConfigFlags_DockingEnable;
    io.DisplaySize = {1280.0F, 800.0F};
    io.DeltaTime = 1.0F / 60.0F;
    unsigned char *pixels{};
    int width{};
    int height{};
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    if (!settings.empty()) {
      ImGui::LoadIniSettingsFromMemory(settings.data(), settings.size());
    }
    workbench_ = {};
    layer_ = std::make_unique<ggems::ui::GGEMSImGuiLayer>();
  }

  auto Frame() -> void {
    ImGui::NewFrame();
    ggems::ui::GGEMSImGuiLayer::FrameInputs const inputs{
      .scene_texture_id = ImTextureID{1},
      .toolbar_icons = {ImTextureID{2}, ImTextureID{3}},
      .scene_texture_logical_width = workbench_.viewport.width,
      .scene_texture_logical_height = workbench_.viewport.height,
      .output_state = &output_,
    };
    layer_->BuildFrame(inputs, detail::GGEMSDeviceStatusSnapshot{}, workbench_);
    ImGui::Render();
  }

  auto Settle() -> void {
    for (int frame = 0; frame < 3; ++frame) {
      Frame();
    }
  }

  auto Click(ImVec2 position) -> void {
    auto &io = ImGui::GetIO();
    io.AddMousePosEvent(position.x, position.y);
    Frame();
    io.AddMouseButtonEvent(ImGuiMouseButton_Left, true);
    Frame();
    io.AddMouseButtonEvent(ImGuiMouseButton_Left, false);
    Frame();
  }

  [[nodiscard]] static auto Window(char const *name) -> ImGuiWindow * {
    return ImGui::FindWindowByName(name);
  }

  [[nodiscard]] static auto Save() -> std::string {
    std::size_t size{};
    char const *settings = ImGui::SaveIniSettingsToMemory(&size);
    return {settings, size};
  }

  [[nodiscard]] static auto WithoutWindow(std::string settings,
                                          char const *identity) -> std::string {
    // ImGui may omit the label and ### prefix when writing persistent names.
    auto const position = settings.find(identity);
    EXPECT_NE(position, std::string::npos);
    if (position != std::string::npos) {
      auto const start = settings.rfind("[Window][", position);
      auto const end = settings.find("\n[", position);
      settings.erase(start, end == std::string::npos ? end : end - start);
    }
    return settings;
  }

  auto MakeLegacyLayout() -> void {
    Settle();
    workbench_.show_scene_panel = true;
    workbench_.show_inspector_panel = true;
    Settle();
    auto const left = Window(detail::k_status_window_name)->DockId;
    auto const right = Window(detail::k_volumes_window_name)->DockId;
    ImGui::DockBuilderDockWindow(detail::k_scene_window_name, left);
    ImGui::DockBuilderDockWindow(detail::k_inspector_window_name, right);
    ImGui::DockBuilderDockWindow(detail::k_camera_window_name, right);
    ImGui::DockBuilderFinish(detail::GetMainDockspaceID());
    Settle();
  }

  [[nodiscard]] static auto DockID(char const *name) -> ImGuiID {
    auto const *window = Window(name);
    return window != nullptr ? window->DockId : 0U;
  }

  [[nodiscard]] static auto OtherDocks() -> std::array<ImGuiID, 5> {
    return {
      DockID(detail::k_output_window_name),
      DockID(detail::k_status_window_name),
      DockID(detail::k_scene_window_name),
      DockID(detail::k_viewport_window_name),
      DockID(detail::k_inspector_window_name),
    };
  }

  static auto
  ExpectSharedDock(char const *neighbor = detail::k_status_window_name)
    -> void {
    auto const *camera = Window(detail::k_camera_window_name);
    auto const *other = Window(neighbor);
    ASSERT_NE(camera, nullptr);
    ASSERT_NE(other, nullptr);
    ASSERT_NE(other->DockNode, nullptr);
    EXPECT_EQ(camera->DockNode, other->DockNode);
    EXPECT_NE(camera->DockId, 0U);
  }

  static auto ExpectSelectedTab(char const *name) -> void {
    auto const *selected = Window(name);
    ASSERT_NE(selected, nullptr);
    ASSERT_NE(selected->DockNode, nullptr);
    EXPECT_EQ(selected->DockNode->SelectedTabId, selected->TabId);
    EXPECT_TRUE(selected->DockTabIsVisible);
  }

  detail::GGEMSWorkbenchState workbench_;

private:
  ImGuiContext *context_{};
  std::unique_ptr<ggems::ui::GGEMSImGuiLayer> layer_;
  ggems::core::GGEMSOutputState output_;
};

TEST_F(GGEMSImGuiLayout, FreshLayoutPlacesExplorerRightAndCameraLeft) {
  EXPECT_FALSE(detail::HasSavedLayout());
  Settle();
  ExpectSharedDock();
  ExpectSelectedTab(detail::k_camera_window_name);
  ExpectSelectedTab(detail::k_volumes_window_name);
  EXPECT_EQ(DockID(detail::k_source_window_name),
            DockID(detail::k_volumes_window_name));
  EXPECT_GT(Window(detail::k_volumes_window_name)->Pos.x,
            Window(detail::k_viewport_window_name)->Pos.x);
  EXPECT_LT(Window(detail::k_camera_window_name)->Pos.x,
            Window(detail::k_viewport_window_name)->Pos.x);
  EXPECT_GT(Window(detail::k_output_window_name)->Pos.y,
            Window(detail::k_viewport_window_name)->Pos.y);
  EXPECT_FALSE(workbench_.show_scene_panel);
  EXPECT_FALSE(workbench_.show_inspector_panel);
  EXPECT_EQ(Window(detail::k_scene_window_name), nullptr);
  EXPECT_EQ(Window(detail::k_inspector_window_name), nullptr);

  Restart(Save());
  Settle();
  ExpectSelectedTab(detail::k_camera_window_name);
  ExpectSelectedTab(detail::k_volumes_window_name);
  EXPECT_FALSE(workbench_.show_scene_panel);
  EXPECT_FALSE(workbench_.show_inspector_panel);
}

TEST_F(GGEMSImGuiLayout, ManualTabSelectionSurvivesFramesAndRestart) {
  Settle();
  ExpectSelectedTab(detail::k_camera_window_name);
  auto const other_docks = OtherDocks();

  for (char const *name : {
         detail::k_status_window_name,
         detail::k_camera_window_name,
         detail::k_source_window_name,
         detail::k_volumes_window_name,
       }) {
    auto const *window = Window(name);
    ASSERT_NE(window, nullptr);
    ASSERT_NE(window->DockNode, nullptr);
    auto *bar = window->DockNode->TabBar;
    ASSERT_NE(bar, nullptr);
    auto const *tab = ImGui::TabBarFindTabByID(bar, window->TabId);
    ASSERT_NE(tab, nullptr);
    Click({bar->BarRect.Min.x + tab->Offset + (0.5F * tab->Width),
           0.5F * (bar->BarRect.Min.y + bar->BarRect.Max.y)});
    Settle();
    ExpectSelectedTab(name);
    EXPECT_EQ(OtherDocks(), other_docks);

    Restart(Save());
    Settle();
    ExpectSelectedTab(name);
    EXPECT_EQ(OtherDocks(), other_docks);
  }
}

TEST_F(GGEMSImGuiLayout, EmptyRuntimeDockspaceIsNotASavedPanelLayout) {
  ImGui::NewFrame();
  ImGui::Begin("Transient dock host");
  ImGui::DockSpace(detail::GetMainDockspaceID(), ImVec2{640.0F, 480.0F});
  ImGui::End();
  ImGui::Render();
  EXPECT_FALSE(detail::HasSavedLayout());

  Settle();
  ExpectSelectedTab(detail::k_camera_window_name);
}

TEST_F(GGEMSImGuiLayout, LegacySettingsAddOnlyMissingCameraToInspectorDock) {
  MakeLegacyLayout();
  ImGui::DockBuilderDockWindow(detail::k_output_window_name, 0U);
  ImGui::SetWindowPos(detail::k_output_window_name, ImVec2{30.0F, 350.0F});
  ImGui::SetWindowSize(detail::k_output_window_name, ImVec2{320.0F, 180.0F});
  Settle();
  auto const other_docks = OtherDocks();
  auto const output_position = Window(detail::k_output_window_name)->Pos;

  Restart(WithoutWindow(Save(), "GGEMS.Camera]"));
  ASSERT_EQ(
    ImGui::FindWindowSettingsByID(ImHashStr(detail::k_camera_window_name)),
    nullptr);
  Settle();
  ExpectSharedDock(detail::k_inspector_window_name);
  EXPECT_EQ(OtherDocks(), other_docks);
  EXPECT_FLOAT_EQ(Window(detail::k_output_window_name)->Pos.x,
                  output_position.x);
  EXPECT_FLOAT_EQ(Window(detail::k_output_window_name)->Pos.y,
                  output_position.y);

  Restart(Save());
  Settle();
  ExpectSharedDock(detail::k_inspector_window_name);
  EXPECT_EQ(OtherDocks(), other_docks);
}

TEST_F(GGEMSImGuiLayout, LegacyLayoutAdmitsOnlyMissingExplorerPanels) {
  for (bool const floating_inspector : {false, true}) {
    SCOPED_TRACE(floating_inspector);
    Restart({});
    MakeLegacyLayout();
    if (floating_inspector) {
      ImGui::DockBuilderDockWindow(detail::k_inspector_window_name, 0U);
      ImGui::SetWindowPos(detail::k_inspector_window_name, {40.0F, 80.0F});
      Settle();
    }
    auto const expected_dock =
      DockID(floating_inspector ? detail::k_scene_window_name
                                : detail::k_inspector_window_name);
    auto const other_docks = OtherDocks();
    auto const camera_dock = DockID(detail::k_camera_window_name);
    ImGui::SetWindowFocus(detail::k_camera_window_name);
    Settle();
    Restart(
      WithoutWindow(WithoutWindow(Save(), "GGEMS.Source]"), "GGEMS.Volumes]"));
    Settle();
    EXPECT_TRUE(workbench_.show_scene_panel);
    EXPECT_TRUE(workbench_.show_inspector_panel);
    EXPECT_EQ(OtherDocks(), other_docks);
    EXPECT_EQ(DockID(detail::k_camera_window_name), camera_dock);
    EXPECT_EQ(DockID(detail::k_source_window_name), expected_dock);
    EXPECT_EQ(DockID(detail::k_volumes_window_name), expected_dock);
    ExpectSelectedTab(detail::k_camera_window_name);
    Restart(Save());
    Settle();
    EXPECT_EQ(OtherDocks(), other_docks);
    EXPECT_EQ(DockID(detail::k_source_window_name), expected_dock);
    EXPECT_EQ(DockID(detail::k_volumes_window_name), expected_dock);
  }
}

TEST_F(GGEMSImGuiLayout, UserSourceDockAndFloatingVolumesSurviveRestart) {
  Settle();
  auto const source_dock = DockID(detail::k_status_window_name);
  ImGui::DockBuilderDockWindow(detail::k_source_window_name, source_dock);
  ImGui::DockBuilderDockWindow(detail::k_volumes_window_name, 0U);
  ImGui::DockBuilderFinish(detail::GetMainDockspaceID());
  ImGui::SetWindowPos(detail::k_volumes_window_name, {400.0F, 90.0F});
  ImGui::SetWindowSize(detail::k_volumes_window_name, {360.0F, 540.0F});
  Settle();
  auto const other_docks = OtherDocks();
  auto const position = Window(detail::k_volumes_window_name)->Pos;
  auto const size = Window(detail::k_volumes_window_name)->Size;
  Restart(Save());
  Settle();
  EXPECT_EQ(DockID(detail::k_source_window_name), source_dock);
  EXPECT_EQ(DockID(detail::k_volumes_window_name), 0U);
  EXPECT_EQ(OtherDocks(), other_docks);
  EXPECT_FLOAT_EQ(Window(detail::k_volumes_window_name)->Pos.x, position.x);
  EXPECT_FLOAT_EQ(Window(detail::k_volumes_window_name)->Pos.y, position.y);
  EXPECT_FLOAT_EQ(Window(detail::k_volumes_window_name)->Size.x, size.x);
  EXPECT_FLOAT_EQ(Window(detail::k_volumes_window_name)->Size.y, size.y);
}

TEST_F(GGEMSImGuiLayout, UserFloatingCameraPlacementSurvivesRestart) {
  Settle();
  ImGui::DockBuilderDockWindow(detail::k_camera_window_name, 0U);
  ImGui::SetWindowPos(detail::k_camera_window_name, ImVec2{50.0F, 70.0F});
  ImGui::SetWindowSize(detail::k_camera_window_name, ImVec2{380.0F, 520.0F});
  Settle();
  auto const position = Window(detail::k_camera_window_name)->Pos;
  auto const size = Window(detail::k_camera_window_name)->Size;
  auto const other_docks = OtherDocks();

  Restart(Save());
  Settle();
  auto const *camera = Window(detail::k_camera_window_name);
  EXPECT_EQ(camera->DockId, 0U);
  EXPECT_FLOAT_EQ(camera->Pos.x, position.x);
  EXPECT_FLOAT_EQ(camera->Pos.y, position.y);
  EXPECT_FLOAT_EQ(camera->Size.x, size.x);
  EXPECT_FLOAT_EQ(camera->Size.y, size.y);
  EXPECT_EQ(OtherDocks(), other_docks);
}

TEST_F(GGEMSImGuiLayout, UserCameraDockSurvivesRestart) {
  Settle();
  ImGuiID const custom_dock = Window(detail::k_source_window_name)->DockId;
  ImGui::DockBuilderDockWindow(detail::k_camera_window_name, custom_dock);
  ImGui::DockBuilderFinish(detail::GetMainDockspaceID());
  Settle();
  auto const other_docks = OtherDocks();

  Restart(Save());
  Settle();
  EXPECT_EQ(Window(detail::k_camera_window_name)->DockId, custom_dock);
  EXPECT_NE(Window(detail::k_status_window_name)->DockId, custom_dock);
  EXPECT_EQ(OtherDocks(), other_docks);
}

TEST_F(GGEMSImGuiLayout, LegacyFloatingInspectorDoesNotResetOtherPanels) {
  MakeLegacyLayout();
  ImGui::DockBuilderDockWindow(detail::k_inspector_window_name, 0U);
  ImGui::SetWindowPos(detail::k_inspector_window_name, ImVec2{40.0F, 80.0F});
  Settle();
  auto const other_docks = OtherDocks();

  Restart(WithoutWindow(Save(), "GGEMS.Camera]"));
  Settle();
  EXPECT_EQ(Window(detail::k_camera_window_name)->DockId, 0U);
  EXPECT_EQ(Window(detail::k_inspector_window_name)->DockId, 0U);
  EXPECT_FLOAT_EQ(Window(detail::k_inspector_window_name)->Pos.x, 40.0F);
  EXPECT_FLOAT_EQ(Window(detail::k_inspector_window_name)->Pos.y, 80.0F);
  EXPECT_EQ(OtherDocks(), other_docks);
}

TEST_F(GGEMSImGuiLayout, CameraCloseReopenResizeAndRestartPreserveDock) {
  Settle();
  ImGuiID const camera_dock = Window(detail::k_camera_window_name)->DockId;
  workbench_.show_camera_panel = false;
  Settle();
  auto const closed_settings = Save();
  workbench_.show_camera_panel = true;
  Settle();
  ExpectSharedDock();
  EXPECT_EQ(Window(detail::k_camera_window_name)->DockId, camera_dock);

  ImGui::GetIO().DisplaySize = {1024.0F, 720.0F};
  Settle();
  ExpectSharedDock();
  EXPECT_EQ(Window(detail::k_camera_window_name)->DockId, camera_dock);

  Restart(closed_settings);
  Settle();
  ExpectSharedDock();
  EXPECT_EQ(Window(detail::k_camera_window_name)->DockId, camera_dock);
}

TEST_F(GGEMSImGuiLayout, ViewResetLayoutRebuildsTabsAndPanelVisibility) {
  MakeLegacyLayout();
  ImGui::DockBuilderDockWindow(detail::k_camera_window_name, 0U);
  workbench_.show_camera_panel = false;
  workbench_.show_output_panel = false;
  workbench_.show_scene_panel = false;
  Settle();

  auto const *menu_bar = Window("##MainMenuBar");
  ASSERT_NE(menu_bar, nullptr);
  auto const &style = ImGui::GetStyle();
  Click(ImVec2{menu_bar->Pos.x + (2.0F * style.FramePadding.x) +
                 (0.5F * ImGui::CalcTextSize("View").x),
               menu_bar->Pos.y + (0.5F * menu_bar->Size.y)});
  auto const &popups = ImGui::GetCurrentContext()->OpenPopupStack;
  ASSERT_FALSE(popups.empty());
  auto const *menu = popups.back().Window;
  ASSERT_NE(menu, nullptr);
  // Reset layout is the last View-menu item; use its actual popup geometry.
  Click(ImVec2{menu->Pos.x + (0.5F * menu->Size.x),
               menu->Pos.y + menu->Size.y - style.WindowPadding.y -
                 (0.5F * ImGui::GetFontSize())});
  Settle();

  EXPECT_TRUE(workbench_.show_output_panel);
  EXPECT_TRUE(workbench_.show_status_panel);
  EXPECT_FALSE(workbench_.show_scene_panel);
  EXPECT_TRUE(workbench_.show_viewport_panel);
  EXPECT_FALSE(workbench_.show_inspector_panel);
  EXPECT_TRUE(workbench_.show_camera_panel);
  EXPECT_TRUE(workbench_.show_source_panel);
  EXPECT_TRUE(workbench_.show_volumes_panel);
  ExpectSelectedTab(detail::k_camera_window_name);
  ExpectSelectedTab(detail::k_volumes_window_name);
  Restart(Save());
  Settle();
  EXPECT_FALSE(workbench_.show_scene_panel);
  EXPECT_FALSE(workbench_.show_inspector_panel);
  ExpectSelectedTab(detail::k_camera_window_name);
  ExpectSelectedTab(detail::k_volumes_window_name);
}

} // namespace
