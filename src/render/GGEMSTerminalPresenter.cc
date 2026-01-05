/// \cond
#include <cstdio>
/// \endcond

#include "GGEMS/render/GGEMSTerminalPresenter.hh"
#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/core/GGEMSException.hh"

namespace ggems::render {
/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

static void WriteRaw(std::string_view s) noexcept {
  std::fwrite(s.data(), 1, s.size(), stdout);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

#ifdef _WIN32
bool GGEMSTerminalPresenter::EnableVTUtf8WinConsole() {
  bool ok{true};

  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  if (hOut == INVALID_HANDLE_VALUE)
    return false;

  original_cp_ = GetConsoleCP();
  original_cp_out_ = GetConsoleOutputCP();
  if (!GetConsoleMode(hOut, &original_mode_))
    return false;

  ok &= (SetConsoleOutputCP(CP_UTF8) != 0);
  ok &= (SetConsoleCP(CP_UTF8) != 0);

  DWORD mode = original_mode_;

  mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
  ok &= (SetConsoleMode(hOut, mode) != 0);

  DWORD newMode = 0;
  if (!GetConsoleMode(hOut, &newMode))
    return false;

  bool vtEnabled = (newMode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) != 0;

  return ok && vtEnabled;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

bool GGEMSTerminalPresenter::RestoreWinConsole() {
  bool ok{true};

  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  if (hOut == INVALID_HANDLE_VALUE)
    return false;

  ok &= (SetConsoleMode(hOut, original_mode_) != 0);
  ok &= (SetConsoleOutputCP(original_cp_out_) != 0);
  ok &= (SetConsoleCP(original_cp_) != 0);
  return ok;
}
#endif

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSTerminalPresenter::Begin(bool use_alt_buffer) noexcept {
  if (started_)
    return;

  use_alt_buffer_ = use_alt_buffer;

#ifdef _WIN32
  GGEMS_CHECK(EnableVTUtf8WinConsole(),
              "Impossible to activate Virtual Terminal and UTF-8 Windows "
              "console mode.");
#endif

  if (use_alt_buffer_) {
    WriteRaw("\033[?1049h"); // Enable alternative screen buffer
  }
  WriteRaw("\033[?25l");     // Hide the cursor
  WriteRaw("\033[2J\033[H"); // Clear screen, and move cursor top left corner
  std::fflush(stdout);

  started_ = true;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSTerminalPresenter::End() noexcept {
  if (!started_)
    return;

  WriteRaw("\033[0m");   // Reset and go back default terminal
  WriteRaw("\033[?25h"); // Show the cursor
  if (use_alt_buffer_) {
    WriteRaw("\033[?1049l"); // Disable alternative screen buffer
  }
  std::fflush(stdout);

#ifdef _WIN32
  GGEMS_CHECK(RestoreWinConsole(),
              "Impossible to restore Windows console mode.");
#endif

  started_ = false;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSTerminalPresenter::Present(std::string_view frame_utf8) noexcept {
  if (!started_) {
    Begin(use_alt_buffer_);
  }
  WriteRaw("\033[H"); // Move cursor to top left corner
  WriteRaw(frame_utf8);
  std::fflush(stdout);
}
} // namespace ggems::render
