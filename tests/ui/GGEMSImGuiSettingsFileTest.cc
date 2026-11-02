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
 * \brief Checks settings persistence and atomic publication failures.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <atomic>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <functional>
#include <ios>
#include <optional>
#include <string>
#include <system_error>
#include <thread>

#include <gtest/gtest.h>

#include "GGEMS/ui/detail/GGEMSImGuiSettingsFile.hh"

namespace {

using ggems::ui::detail::BuildImGuiSettingsPath;
using ggems::ui::detail::GGEMSSettingsFileRead;
using ggems::ui::detail::GGEMSUserDirectories;
using ggems::ui::detail::ReadImGuiSettingsFile;
using ggems::ui::detail::ToUtf8;
using ggems::ui::detail::WriteImGuiSettingsFile;

// =============================================================================
// =============================================================================

class GGEMSImGuiSettingsFileTest : public ::testing::Test {
protected:
  auto SetUp() -> void override {
    root_ = std::filesystem::temp_directory_path() /
            ("ggems_imgui_settings_test_" +
             std::to_string(
               std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::create_directories(root_);
  }

  auto TearDown() -> void override {
    std::error_code ignored{};
    std::filesystem::remove_all(root_, ignored);
  }

  std::filesystem::path root_;
};

// =============================================================================
// =============================================================================

// Settings paths must obey the platform's user-directory policy and reject
// missing roots.
TEST(GGEMSImGuiSettingsPath, UsesThePerUserConfigurationDirectory) {
  std::filesystem::path const base =
    std::filesystem::temp_directory_path() / "user";

#ifdef _WIN32
  EXPECT_EQ(BuildImGuiSettingsPath(GGEMSUserDirectories{
              .local_app_data = base, .xdg_config_home = {}, .home = {}}),
            base / "GGEMS" / "imgui.ini");
  EXPECT_FALSE(BuildImGuiSettingsPath(GGEMSUserDirectories{
                                        .local_app_data = {},
                                        .xdg_config_home = base,
                                        .home = base,
                                      })
                 .has_value());
#elifdef __APPLE__
  EXPECT_EQ(BuildImGuiSettingsPath(GGEMSUserDirectories{
              .local_app_data = {}, .xdg_config_home = {}, .home = base}),
            base / "Library" / "Application Support" / "GGEMS" / "imgui.ini");
  EXPECT_FALSE(BuildImGuiSettingsPath(GGEMSUserDirectories{}).has_value());
#else
  std::filesystem::path const config = base / "config";
  EXPECT_EQ(BuildImGuiSettingsPath(GGEMSUserDirectories{
              .local_app_data = {}, .xdg_config_home = config, .home = base}),
            config / "GGEMS" / "imgui.ini");
  EXPECT_EQ(BuildImGuiSettingsPath(GGEMSUserDirectories{
              .local_app_data = {}, .xdg_config_home = {}, .home = base}),
            base / ".config" / "GGEMS" / "imgui.ini");
  EXPECT_FALSE(BuildImGuiSettingsPath(GGEMSUserDirectories{}).has_value());
#endif
}

// =============================================================================
// =============================================================================

// First launch without saved layout must report missing settings without
// inventing content.
TEST_F(GGEMSImGuiSettingsFileTest, MissingFileIsNotAnError) {
  GGEMSSettingsFileRead const settings =
    ReadImGuiSettingsFile(root_ / "GGEMS" / "imgui.ini");

  EXPECT_EQ(settings.status, GGEMSSettingsFileRead::Status::Missing);
  EXPECT_TRUE(settings.text.empty());
}

// =============================================================================
// =============================================================================

// Saving a layout must create parent directories and preserve the exact
// payload.
TEST_F(GGEMSImGuiSettingsFileTest, WriteCreatesDirectoriesAndRoundTrips) {
  std::filesystem::path const path = root_ / "GGEMS" / "imgui.ini";
  std::string const text = "[Window][###GGEMS.Viewport]\nPos=0,0\n";

  EXPECT_FALSE(WriteImGuiSettingsFile(path, text).has_value());

  GGEMSSettingsFileRead const settings = ReadImGuiSettingsFile(path);

  EXPECT_EQ(settings.status, GGEMSSettingsFileRead::Status::Loaded);
  EXPECT_EQ(settings.text, text);
  EXPECT_FALSE(std::filesystem::exists(root_ / "GGEMS" / "imgui.ini.tmp"));
}

// =============================================================================
// =============================================================================

// A second save must replace the previous layout completely.
TEST_F(GGEMSImGuiSettingsFileTest, WriteReplacesTheExistingFile) {
  std::filesystem::path const path = root_ / "imgui.ini";

  EXPECT_FALSE(WriteImGuiSettingsFile(path, "first\n").has_value());
  EXPECT_FALSE(WriteImGuiSettingsFile(path, "second\n").has_value());

  EXPECT_EQ(ReadImGuiSettingsFile(path).text, "second\n");
}

// =============================================================================
// =============================================================================

// A directory at the settings filename must remain intact and cause a clear
// read/write failure.
TEST_F(GGEMSImGuiSettingsFileTest, NonRegularSettingsPathFailsBothWays) {
  std::filesystem::path const path = root_ / "imgui.ini";
  std::filesystem::create_directories(path);

  GGEMSSettingsFileRead const settings = ReadImGuiSettingsFile(path);

  EXPECT_EQ(settings.status, GGEMSSettingsFileRead::Status::Failed);
  EXPECT_FALSE(settings.error.empty());

  EXPECT_TRUE(WriteImGuiSettingsFile(path, "layout\n").has_value());
  EXPECT_TRUE(std::filesystem::is_directory(path));
  EXPECT_FALSE(std::filesystem::exists(root_ / "imgui.ini.tmp"));
}

// =============================================================================
// =============================================================================

// A file blocking the parent directory must fail without replacing that file.
TEST_F(GGEMSImGuiSettingsFileTest, UnwritableDestinationLeavesNothingBehind) {
  std::filesystem::path const blocking_file = root_ / "GGEMS";

  std::ofstream{blocking_file, std::ios::binary} << "not a directory";

  std::optional<std::string> const error =
    WriteImGuiSettingsFile(blocking_file / "imgui.ini", "layout\n");

  ASSERT_TRUE(error.has_value());
  EXPECT_FALSE(error->empty());
  EXPECT_TRUE(std::filesystem::is_regular_file(blocking_file));
}

// =============================================================================
// =============================================================================

// Unicode user paths must support layout persistence and readable failure
// paths.
TEST_F(GGEMSImGuiSettingsFileTest, UnicodeDirectoriesNeverThrow) {
  std::filesystem::path const user =
    root_ / std::filesystem::path{u8"\u6d4b\u8bd5_\U0001F52C_\u00e9"};
  std::filesystem::path const path = user / "GGEMS" / "imgui.ini";

  EXPECT_FALSE(WriteImGuiSettingsFile(path, "layout\n").has_value());
  EXPECT_EQ(ReadImGuiSettingsFile(path).text, "layout\n");
  EXPECT_EQ(ToUtf8(path).find("imgui.ini"), ToUtf8(path).size() - 9U);

  std::ofstream{user / "blocked", std::ios::binary} << "file";
  std::optional<std::string> const error =
    WriteImGuiSettingsFile(user / "blocked" / "imgui.ini", "layout\n");

  ASSERT_TRUE(error.has_value());
  EXPECT_NE(error->find(ToUtf8(user / "blocked")), std::string::npos);
}

// =============================================================================
// =============================================================================

// An unrelated stale temporary file must not prevent publication of a valid
// layout.
TEST_F(GGEMSImGuiSettingsFileTest, StaleTemporaryFileDoesNotBlockSaving) {
  std::filesystem::path const path = root_ / "imgui.ini";

  std::ofstream{root_ / "imgui.ini.tmp", std::ios::binary} << "stale";

  EXPECT_FALSE(WriteImGuiSettingsFile(path, "layout\n").has_value());
  EXPECT_EQ(ReadImGuiSettingsFile(path).text, "layout\n");
}

// =============================================================================
// =============================================================================

// Concurrent saves must expose only complete payloads and leave no temporary
// files behind.
TEST_F(GGEMSImGuiSettingsFileTest,
       ConcurrentWritersPublishOnlyCompletePayloads) {
  std::filesystem::path const path = root_ / "imgui.ini";
  std::string const payload_a(16384U, 'A');
  std::string const payload_b(49152U, 'B');
  std::atomic<bool> mixed{false};

  auto const Writer = [&path](std::string const &payload) -> void {
    for (int iteration = 0; iteration < 150; ++iteration) {
      (void)WriteImGuiSettingsFile(path, payload);
    }
  };

  auto const Reader = [&] -> void {
    for (int iteration = 0; iteration < 300; ++iteration) {
      GGEMSSettingsFileRead const settings = ReadImGuiSettingsFile(path);

      if (settings.status == GGEMSSettingsFileRead::Status::Loaded &&
          settings.text != payload_a && settings.text != payload_b) {
        mixed = true;
      }
    }
  };

  std::thread writer_a{Writer, std::cref(payload_a)};
  std::thread writer_b{Writer, std::cref(payload_b)};
  std::thread reader{Reader};
  writer_a.join();
  writer_b.join();
  reader.join();

  EXPECT_FALSE(mixed.load());
  std::string const final_text = ReadImGuiSettingsFile(path).text;
  EXPECT_TRUE(final_text == payload_a || final_text == payload_b);

  std::size_t leftovers{0U};
  for (auto const &entry : std::filesystem::directory_iterator{root_}) {
    leftovers += entry.path().extension() == ".tmp" ? 1U : 0U;
  }
  EXPECT_EQ(leftovers, 0U);
}

} // namespace
