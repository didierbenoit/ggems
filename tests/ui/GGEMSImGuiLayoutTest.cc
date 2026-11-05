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
      .scene_texture_logical_width = workbench_.viewport.width,
      .scene_texture_logical_height = workbench_.viewport.height,
      .output_state = &output_};
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

  [[nodiscard]] static auto WithoutCamera(std::string settings) -> std::string {
    // ImGui may omit the label and ### prefix when writing persistent names.
    auto const identity = settings.find("GGEMS.Camera]");
    EXPECT_NE(identity, std::string::npos);
    if (identity != std::string::npos) {
      auto const start = settings.rfind("[Window][", identity);
      auto const end = settings.find("\n[", identity);
      settings.erase(start, end == std::string::npos ? end : end - start);
    }
    return settings;
  }

  [[nodiscard]] static auto OtherDocks() -> std::array<ImGuiID, 5> {
    return {
      Window(detail::k_output_window_name)->DockId,
      Window(detail::k_status_window_name)->DockId,
      Window(detail::k_scene_window_name)->DockId,
      Window(detail::k_viewport_window_name)->DockId,
      Window(detail::k_inspector_window_name)->DockId,
    };
  }

  static auto ExpectSharedInspectorDock(bool inspector_selected) -> void {
    auto const *camera = Window(detail::k_camera_window_name);
    auto const *inspector = Window(detail::k_inspector_window_name);
    ASSERT_NE(camera, nullptr);
    ASSERT_NE(inspector, nullptr);
    ASSERT_NE(inspector->DockNode, nullptr);
    EXPECT_EQ(camera->DockNode, inspector->DockNode);
    EXPECT_NE(camera->DockId, 0U);
    if (inspector_selected) {
      EXPECT_EQ(inspector->DockNode->SelectedTabId, inspector->TabId);
      EXPECT_TRUE(inspector->DockTabIsVisible);
      EXPECT_FALSE(camera->DockTabIsVisible);
    }
  }

  detail::GGEMSWorkbenchState workbench_;

private:
  ImGuiContext *context_{};
  std::unique_ptr<ggems::ui::GGEMSImGuiLayer> layer_;
  ggems::core::GGEMSOutputState output_;
};

TEST_F(GGEMSImGuiLayout, FreshLayoutSelectsInspectorBesideCamera) {
  EXPECT_FALSE(detail::HasSavedLayout());
  Settle();
  ExpectSharedInspectorDock(true);
  EXPECT_GT(Window(detail::k_inspector_window_name)->Pos.x,
            Window(detail::k_viewport_window_name)->Pos.x);

  Restart(Save());
  Settle();
  ExpectSharedInspectorDock(true);
}

TEST_F(GGEMSImGuiLayout, EmptyRuntimeDockspaceIsNotASavedPanelLayout) {
  ImGui::NewFrame();
  ImGui::Begin("Transient dock host");
  ImGui::DockSpace(detail::GetMainDockspaceID(), ImVec2{640.0F, 480.0F});
  ImGui::End();
  ImGui::Render();
  EXPECT_FALSE(detail::HasSavedLayout());

  Settle();
  ExpectSharedInspectorDock(true);
}

TEST_F(GGEMSImGuiLayout, LegacySettingsAddOnlyMissingCameraToInspectorDock) {
  Settle();
  ImGui::DockBuilderDockWindow(detail::k_output_window_name, 0U);
  ImGui::SetWindowPos(detail::k_output_window_name, ImVec2{30.0F, 350.0F});
  ImGui::SetWindowSize(detail::k_output_window_name, ImVec2{320.0F, 180.0F});
  Settle();
  auto const other_docks = OtherDocks();
  auto const output_position = Window(detail::k_output_window_name)->Pos;

  Restart(WithoutCamera(Save()));
  ASSERT_EQ(
    ImGui::FindWindowSettingsByID(ImHashStr(detail::k_camera_window_name)),
    nullptr);
  Settle();
  ExpectSharedInspectorDock(false);
  EXPECT_EQ(OtherDocks(), other_docks);
  EXPECT_FLOAT_EQ(Window(detail::k_output_window_name)->Pos.x,
                  output_position.x);
  EXPECT_FLOAT_EQ(Window(detail::k_output_window_name)->Pos.y,
                  output_position.y);

  Restart(Save());
  Settle();
  ExpectSharedInspectorDock(false);
  EXPECT_EQ(OtherDocks(), other_docks);
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
  ImGuiID const custom_dock = Window(detail::k_scene_window_name)->DockId;
  ImGui::DockBuilderDockWindow(detail::k_camera_window_name, custom_dock);
  ImGui::DockBuilderFinish(detail::GetMainDockspaceID());
  Settle();
  auto const other_docks = OtherDocks();

  Restart(Save());
  Settle();
  EXPECT_EQ(Window(detail::k_camera_window_name)->DockId, custom_dock);
  EXPECT_NE(Window(detail::k_inspector_window_name)->DockId, custom_dock);
  EXPECT_EQ(OtherDocks(), other_docks);
}

TEST_F(GGEMSImGuiLayout, LegacyFloatingInspectorDoesNotResetOtherPanels) {
  Settle();
  ImGui::DockBuilderDockWindow(detail::k_inspector_window_name, 0U);
  ImGui::SetWindowPos(detail::k_inspector_window_name, ImVec2{40.0F, 80.0F});
  Settle();
  auto const other_docks = OtherDocks();

  Restart(WithoutCamera(Save()));
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
  ExpectSharedInspectorDock(false);
  EXPECT_EQ(Window(detail::k_camera_window_name)->DockId, camera_dock);

  ImGui::GetIO().DisplaySize = {1024.0F, 720.0F};
  Settle();
  ExpectSharedInspectorDock(false);
  EXPECT_EQ(Window(detail::k_camera_window_name)->DockId, camera_dock);

  Restart(closed_settings);
  Settle();
  ExpectSharedInspectorDock(false);
  EXPECT_EQ(Window(detail::k_camera_window_name)->DockId, camera_dock);
}

TEST_F(GGEMSImGuiLayout, ViewResetLayoutRebuildsTabsAndPanelVisibility) {
  Settle();
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
  EXPECT_TRUE(workbench_.show_scene_panel);
  EXPECT_TRUE(workbench_.show_viewport_panel);
  EXPECT_TRUE(workbench_.show_inspector_panel);
  EXPECT_TRUE(workbench_.show_camera_panel);
  ExpectSharedInspectorDock(true);
}

} // namespace
