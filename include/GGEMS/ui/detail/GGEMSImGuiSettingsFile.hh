#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace ggems::ui::detail {

struct GGEMSUserDirectories {
  std::optional<std::filesystem::path> local_app_data;
  std::optional<std::filesystem::path> xdg_config_home;
  std::optional<std::filesystem::path> home;
};

struct GGEMSSettingsFileRead {
  enum class Status : std::uint8_t { Missing, Loaded, Failed };

  Status status{Status::Missing};
  std::string text;
  std::string error;
};

[[nodiscard]] auto ReadUserDirectories() -> GGEMSUserDirectories;

[[nodiscard]] auto ToUtf8(std::filesystem::path const &path) -> std::string;

[[nodiscard]] auto
BuildImGuiSettingsPath(GGEMSUserDirectories const &directories)
  -> std::optional<std::filesystem::path>;

[[nodiscard]] auto ReadImGuiSettingsFile(std::filesystem::path const &path)
  -> GGEMSSettingsFileRead;

[[nodiscard]] auto WriteImGuiSettingsFile(std::filesystem::path const &path,
                                          std::string_view text)
  -> std::optional<std::string>;

} // namespace ggems::ui::detail
