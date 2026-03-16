#pragma once

/// \cond
#include <string_view>
/// \endcond

#ifdef _WIN32
#include "GGEMS/platform/windows/GGEMSWindowsCore.hh"
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
    Space
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
  [[nodiscard]] TerminalKey PollKey() noexcept;

private:
#ifdef _WIN32
  bool EnableVTUtf8WinConsole();
  bool RestoreWinConsole();
#endif

private:
  bool started_{false};
  bool use_alt_buffer_{false};
#ifdef _WIN32
  DWORD original_mode_;
  UINT original_cp_out_;
  UINT original_cp_;
#endif
};
} // namespace ggems::render
