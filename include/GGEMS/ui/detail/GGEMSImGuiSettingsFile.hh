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
 * \brief Reads and replaces per-user Dear ImGui layout settings.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace ggems::ui::detail {

/*! \brief Stores available platform configuration directories. */
struct GGEMSUserDirectories {
  /*! \brief Absolute Windows local application data directory. */
  std::optional<std::filesystem::path> local_app_data;

  /*! \brief Absolute XDG configuration directory when available. */
  std::optional<std::filesystem::path> xdg_config_home;

  /*! \brief Absolute home directory for platform fallback paths. */
  std::optional<std::filesystem::path> home;
};

/*! \brief Distinguishes missing settings from successful or failed reads. */
struct GGEMSSettingsFileRead {
  /*! \brief Classifies a settings-file read outcome. */
  enum class Status : std::uint8_t { Missing, Loaded, Failed };

  /*!
   * \var ggems::ui::detail::GGEMSSettingsFileRead::Status::Missing
   * \brief The settings file does not exist.
   */

  /*!
   * \var ggems::ui::detail::GGEMSSettingsFileRead::Status::Loaded
   * \brief The file was read successfully.
   */

  /*!
   * \var ggems::ui::detail::GGEMSSettingsFileRead::Status::Failed
   * \brief The file could not be read.
   */

  /*! \brief Read outcome, including an absent file. */
  Status status{Status::Missing};

  /*! \brief Loaded file contents, including a valid empty file. */
  std::string text;

  /*! \brief Diagnostic text when the read failed. */
  std::string error;
};

/*!
 * \brief Reads absolute configuration paths from the environment.
 *
 * \return Platform-relevant directories; empty or relative values are omitted.
 */
[[nodiscard]] auto ReadUserDirectories() -> GGEMSUserDirectories;

/*!
 * \brief Encodes a filesystem path for UTF-8 diagnostics.
 *
 * \param[in] path Native filesystem path.
 * \return UTF-8 path text.
 */
[[nodiscard]] auto ToUtf8(std::filesystem::path const &path) -> std::string;

/*!
 * \brief Selects the platform-specific GGEMS layout settings path.
 *
 * Uses local application data on Windows, Application Support on macOS, and XDG
 *   configuration or ~/.config on other platforms.
 *
 * \param[in] directories Available absolute user directories.
 * \return Path to GGEMS/imgui.ini, or no path when the required directory is
 *   absent.
 */
[[nodiscard]] auto
BuildImGuiSettingsPath(GGEMSUserDirectories const &directories)
  -> std::optional<std::filesystem::path>;

/*!
 * \brief Reads a layout file without treating absence as an error.
 *
 * \param[in] path Settings file to read.
 * \return Read status, loaded text, and failure diagnostic.
 */
[[nodiscard]] auto ReadImGuiSettingsFile(std::filesystem::path const &path)
  -> GGEMSSettingsFileRead;

/*!
 * \brief Replaces a settings file through a temporary sibling file.
 *
 * \param[in] path Destination file; missing parent directories are created.
 * \param[in] text Complete serialized Dear ImGui settings.
 * \return No value on success, or a filesystem/write diagnostic on failure.
 */
[[nodiscard]] auto WriteImGuiSettingsFile(std::filesystem::path const &path,
                                          std::string_view text)
  -> std::optional<std::string>;

} // namespace ggems::ui::detail
