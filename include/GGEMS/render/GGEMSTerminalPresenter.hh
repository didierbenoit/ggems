#pragma once

/// \cond
#include <string_view>
#include <cstdint>
/// \endcond

#ifdef _WIN32
#include "GGEMS/platform/windows/GGEMSWindowsCore.hh"
#else
#include "GGEMS/platform/posix/GGEMSPosixCore.hh"
#endif

namespace ggems::render {
class GGEMSTerminalPresenter {
public:
  enum class TerminalKey : std::uint8_t {
    None = 0,
    Up,
    Down,
    PageUp,
    PageDown,
    Space,
    Enter
  };

public:
  GGEMSTerminalPresenter() = default;
  ~GGEMSTerminalPresenter() = default;

  GGEMSTerminalPresenter(GGEMSTerminalPresenter const &) = delete;
  GGEMSTerminalPresenter(GGEMSTerminalPresenter &&) = delete;
  GGEMSTerminalPresenter &operator=(GGEMSTerminalPresenter const &) = delete;
  GGEMSTerminalPresenter &operator=(GGEMSTerminalPresenter &&) = delete;

public:
  void Begin(bool use_alt_buffer = true) noexcept;
  void End() noexcept;
  void Present(std::string_view frame_utf8) noexcept;
  void Write(std::string_view bytes) noexcept;
  [[nodiscard]] TerminalKey PollKey() noexcept;

#ifndef _WIN32
  bool EnablePosixRawInput();
  bool EnablePosixCanonicalInput();
#endif

private:
#ifdef _WIN32
  bool EnableVTUtf8WinConsole();
  bool RestoreWinConsole();
#else
  bool EnablePosixTerminal();
  bool RestorePosixTerminal();
#endif

private:
  bool started_{false};
  bool use_alt_buffer_{false};
#ifdef _WIN32
  DWORD original_mode_;
  UINT original_cp_out_;
  UINT original_cp_;
#else
  termios original_termios_{};
  std::int32_t original_stdin_flags_{0};
  bool posix_terminal_enabled_{false};
#endif
};
} // namespace ggems::render
