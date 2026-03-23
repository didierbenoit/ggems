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

#ifdef _WIN32
/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

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
#else

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */
bool GGEMSTerminalPresenter::EnablePosixTerminal() {
  if (posix_terminal_enabled_) {
    return true;
  }

  if (::isatty(STDIN_FILENO) == 0) {
    return false;
  }

  if (::tcgetattr(STDIN_FILENO, &original_termios_) != 0) {
    return false;
  }

  if (!EnablePosixRawInput()) {
    return false;
  }

  posix_terminal_enabled_ = true;
  return true;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

bool GGEMSTerminalPresenter::EnablePosixRawInput() {
  if (::isatty(STDIN_FILENO) == 0) {
    return false;
  }

  termios raw = original_termios_;

  raw.c_lflag &= static_cast<tcflag_t>(~(ICANON | ECHO));
  raw.c_iflag &= static_cast<tcflag_t>(~(IXON | ICRNL));
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 0;

  if (::tcsetattr(STDIN_FILENO, TCSANOW, &raw) != 0) {
    return false;
  }

  ::tcflush(STDIN_FILENO, TCIFLUSH);
  return true;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

bool GGEMSTerminalPresenter::EnablePosixCanonicalInput() {
  if (::isatty(STDIN_FILENO) == 0) {
    return false;
  }

  termios cooked = original_termios_;

  cooked.c_lflag |= static_cast<tcflag_t>(ICANON | ECHO);
  cooked.c_iflag |= static_cast<tcflag_t>(ICRNL);
  cooked.c_cc[VMIN] = 1;
  cooked.c_cc[VTIME] = 0;

  if (::tcsetattr(STDIN_FILENO, TCSANOW, &cooked) != 0) {
    return false;
  }

  ::tcflush(STDIN_FILENO, TCIFLUSH);
  return true;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

bool GGEMSTerminalPresenter::RestorePosixTerminal() {
  if (!posix_terminal_enabled_) {
    return true;
  }

  bool ok = (::tcsetattr(STDIN_FILENO, TCSANOW, &original_termios_) == 0);

  posix_terminal_enabled_ = false;
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
#else
  GGEMS_CHECK(EnablePosixTerminal(),
              "Impossible to activate POSIX terminal raw mode.");
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
#else
  GGEMS_CHECK(RestorePosixTerminal(),
              "Impossible to restore POSIX terminal mode.");
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

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSTerminalPresenter::Write(std::string_view bytes) noexcept {
  if (!started_) {
    Begin(use_alt_buffer_);
  }

  WriteRaw(bytes);
  std::fflush(stdout);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSTerminalPresenter::TerminalKey GGEMSTerminalPresenter::PollKey() noexcept {
#ifdef _WIN32
  if (!_kbhit()) {
    return TerminalKey::None;
  }

  int ch = _getch();

  if (ch == 13) {
    return TerminalKey::Enter;
  }

  if (ch == ' ') {
    return TerminalKey::Space;
  }

  // Arrow / Page keys
  if (ch == 0 || ch == 224) {
    int ext = _getch();
    switch (ext) {
    case 72:
      return TerminalKey::Up; // Arrow Up
    case 80:
      return TerminalKey::Down; // Arrow Down
    case 73:
      return TerminalKey::PageUp; // Page Up
    case 81:
      return TerminalKey::PageDown; // Page Down
    default:
      return TerminalKey::None;
    }
  }

  return TerminalKey::None;
#else
  unsigned char seq[4] = {};
  ssize_t count = ::read(STDIN_FILENO, seq, sizeof(seq));

  if (count <= 0) {
    return TerminalKey::None;
  }

  if (count == 1) {
    if (seq[0] == ' ') {
      return TerminalKey::Space;
    }

    if (seq[0] == '\n' || seq[0] == '\r') {
      return TerminalKey::Enter;
    }
    return TerminalKey::None;
  }

  if (seq[0] == 0x1B && seq[1] == '[') {
    switch (seq[2]) {
    case 'A':
      return TerminalKey::Up;
    case 'B':
      return TerminalKey::Down;
    case '5':
      if (count >= 4 && seq[3] == '~') {
        return TerminalKey::PageUp;
      }
    case '6':
      if (count >= 4 && seq[3] == '~') {
        return TerminalKey::PageDown;
      }
    default:
      break;
    }
  }

  return TerminalKey::None;
#endif
}
} // namespace ggems::render
