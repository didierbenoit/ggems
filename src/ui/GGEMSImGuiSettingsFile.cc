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

#ifdef _WIN32
[[nodiscard]] auto ReadAbsolutePathVariable(wchar_t const *name)
  -> std::optional<std::filesystem::path> {
  wchar_t const *value = _wgetenv(name);

  if (value == nullptr || *value == L'\0') {
    return std::nullopt;
  }

  std::filesystem::path path{value};
#else
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

[[nodiscard]] auto ReadFailure(std::string error)
  -> ggems::ui::detail::GGEMSSettingsFileRead {
  return ggems::ui::detail::GGEMSSettingsFileRead{
    .status = ggems::ui::detail::GGEMSSettingsFileRead::Status::Failed,
    .text = {},
    .error = std::move(error),
  };
}

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

  // A writer-private sibling: concurrent GGEMS processes never share or
  // truncate each other's staging file (noreplace = exclusive creation).
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
