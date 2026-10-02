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
 * \brief Resolves user configuration paths and reads or replaces layout files.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
#include <iterator>
#include <optional>
#include <random>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

#include "GGEMS/ui/detail/GGEMSImGuiSettingsFile.hh"

namespace {

// =============================================================================
// =============================================================================

#ifdef _WIN32
/*!
 * \brief Reads a nonempty absolute path from an environment variable.
 *
 * \param[in] name Platform-native environment-variable name.
 * \return Absolute path, or no value for absent, empty, or relative values.
 */
[[nodiscard]] auto ReadAbsolutePathVariable(wchar_t const *name)
  -> std::optional<std::filesystem::path> {
  wchar_t const *value = _wgetenv(name);

  if (value == nullptr || *value == L'\0') {
    return std::nullopt;
  }

  std::filesystem::path path{value};
#else
/*!
 * \brief Reads a nonempty absolute path from an environment variable.
 *
 * \param[in] name Platform-native environment-variable name.
 * \return Absolute path, or no value for absent, empty, or relative values.
 */
[[nodiscard]] auto ReadAbsolutePathVariable(char const *name)
  -> std::optional<std::filesystem::path> {
  char const *value = std::getenv(name);

  if (value == nullptr || *value == '\0') {
    return std::nullopt;
  }

  std::filesystem::path path{value};
#endif

  if (!path.is_absolute()) {
    return std::nullopt;
  }

  return path;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Builds a failed settings-read result with its diagnostic.
 *
 * \param[in] error Read failure diagnostic to retain.
 * \return Failed result with empty settings text.
 */
[[nodiscard]] auto ReadFailure(std::string error)
  -> ggems::ui::detail::GGEMSSettingsFileRead {
  return ggems::ui::detail::GGEMSSettingsFileRead{
    .status = ggems::ui::detail::GGEMSSettingsFileRead::Status::Failed,
    .text = {},
    .error = std::move(error),
  };
}

/*! \brief Maximum attempts to create a unique temporary settings file. */
constexpr int k_temporary_name_attempts{8};

} // namespace

namespace ggems::ui::detail {

// =============================================================================
// =============================================================================

auto ToUtf8(std::filesystem::path const &path) -> std::string {
  std::u8string const text = path.u8string();

  return std::string{text.begin(), text.end()};
}

// =============================================================================
// =============================================================================

auto ReadUserDirectories() -> GGEMSUserDirectories {
#ifdef _WIN32
  return GGEMSUserDirectories{
    .local_app_data = ReadAbsolutePathVariable(L"LOCALAPPDATA"),
    .xdg_config_home = std::nullopt,
    .home = std::nullopt,
  };
#else
  return GGEMSUserDirectories{
    .local_app_data = std::nullopt,
    .xdg_config_home = ReadAbsolutePathVariable("XDG_CONFIG_HOME"),
    .home = ReadAbsolutePathVariable("HOME"),
  };
#endif
}

// -----------------------------------------------------------------------------

auto BuildImGuiSettingsPath(GGEMSUserDirectories const &directories)
  -> std::optional<std::filesystem::path> {
#ifdef _WIN32
  if (!directories.local_app_data.has_value()) {
    return std::nullopt;
  }

  return *directories.local_app_data / "GGEMS" / "imgui.ini";
#elifdef __APPLE__
  if (!directories.home.has_value()) {
    return std::nullopt;
  }

  return *directories.home / "Library" / "Application Support" / "GGEMS" /
         "imgui.ini";
#else
  if (directories.xdg_config_home.has_value()) {
    return *directories.xdg_config_home / "GGEMS" / "imgui.ini";
  }

  if (!directories.home.has_value()) {
    return std::nullopt;
  }

  return *directories.home / ".config" / "GGEMS" / "imgui.ini";
#endif
}

// -----------------------------------------------------------------------------

auto ReadImGuiSettingsFile(std::filesystem::path const &path)
  -> GGEMSSettingsFileRead {
  std::error_code error{};

  bool const exists = std::filesystem::exists(path, error);

  if (error) {
    return ReadFailure(error.message());
  }

  if (!exists) {
    return GGEMSSettingsFileRead{};
  }

  if (!std::filesystem::is_regular_file(path, error)) {
    return ReadFailure(error ? error.message() : "not a regular file");
  }

  std::ifstream stream{path, std::ios::binary};

  if (!stream) {
    return ReadFailure("cannot open the file for reading");
  }

  std::string text{std::istreambuf_iterator<char>{stream},
                   std::istreambuf_iterator<char>{}};

  if (stream.bad()) {
    return ReadFailure("read error");
  }

  return GGEMSSettingsFileRead{
    .status = GGEMSSettingsFileRead::Status::Loaded,
    .text = std::move(text),
    .error = {},
  };
}

// -----------------------------------------------------------------------------

auto WriteImGuiSettingsFile(std::filesystem::path const &path,
                            std::string_view text)
  -> std::optional<std::string> {
  std::error_code error{};

  std::filesystem::create_directories(path.parent_path(), error);

  if (error) {
    return std::format("cannot create '{}': {}", ToUtf8(path.parent_path()),
                       error.message());
  }

  std::random_device entropy{};
  std::filesystem::path temporary_path{};
  std::ofstream stream{};

  for (int attempt = 0;
       attempt < k_temporary_name_attempts && !stream.is_open(); ++attempt) {
    std::uint64_t const suffix =
      (static_cast<std::uint64_t>(entropy()) << 32U) | entropy();

    temporary_path = path;
    temporary_path += std::format(".{:016x}.tmp", suffix);
    stream.open(temporary_path,
                std::ios::binary | std::ios::out | std::ios::noreplace);
  }

  {
    if (!stream.is_open()) {
      return std::format("cannot create a temporary file next to '{}'",
                         ToUtf8(path));
    }

    stream.write(text.data(), static_cast<std::streamsize>(text.size()));
    stream.close();

    if (!stream) {
      std::error_code ignored{};
      std::filesystem::remove(temporary_path, ignored);

      return std::format("cannot write '{}'", ToUtf8(temporary_path));
    }
  }

  std::filesystem::rename(temporary_path, path, error);

  if (error) {
    std::error_code ignored{};
    std::filesystem::remove(temporary_path, ignored);

    return std::format("cannot replace '{}': {}", ToUtf8(path),
                       error.message());
  }

  return std::nullopt;
}

} // namespace ggems::ui::detail
