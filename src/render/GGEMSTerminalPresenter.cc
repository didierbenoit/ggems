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
  HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);

  if (hOut == INVALID_HANDLE_VALUE || hIn == INVALID_HANDLE_VALUE)
    return false;

  original_cp_ = GetConsoleCP();
  original_cp_out_ = GetConsoleOutputCP();

  if (!GetConsoleMode(hOut, &original_mode_out_)) {
    return false;
  }

  if (!GetConsoleMode(hIn, &original_mode_in_)) {
    return false;
  }

  ok &= (SetConsoleOutputCP(CP_UTF8) != 0);
  ok &= (SetConsoleCP(CP_UTF8) != 0);

  DWORD out_mode = original_mode_out_;
  out_mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
  ok &= (SetConsoleMode(hOut, out_mode) != 0);

  DWORD in_mode = original_mode_in_;
  in_mode |= static_cast<DWORD>(ENABLE_WINDOW_INPUT);
  in_mode |= static_cast<DWORD>(ENABLE_MOUSE_INPUT);
  in_mode |= static_cast<DWORD>(ENABLE_EXTENDED_FLAGS);
  in_mode &= ~static_cast<DWORD>(ENABLE_QUICK_EDIT_MODE);
  ok &= (SetConsoleMode(hIn, in_mode) != 0);

  DWORD new_out_mode = 0;
  if (!GetConsoleMode(hOut, &new_out_mode)) {
    return false;
  }

  bool vt_enabled = (new_out_mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) != 0;

  return ok && vt_enabled;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

bool GGEMSTerminalPresenter::RestoreWinConsole() {
  bool ok{true};

  HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
  HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);

  if (hOut == INVALID_HANDLE_VALUE || hIn == INVALID_HANDLE_VALUE) {
    return false;
  }

  ok &= (SetConsoleMode(hOut, original_mode_out_) != 0);
  ok &= (SetConsoleMode(hIn, original_mode_in_) != 0);
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
  GGEMS_CHECK_FATAL(EnableVTUtf8WinConsole(),
                    "Impossible to activate Virtual Terminal and UTF-8 Windows "
                    "console mode.");
#else
  GGEMS_CHECK_FATAL(EnablePosixTerminal(),
                    "Impossible to activate POSIX terminal raw mode.");
#endif

  if (use_alt_buffer_) {
    WriteRaw("\033[?1049h"); // Enable alternative screen buffer
  }

#ifndef _WIN32
  WriteRaw("\033[?1000h"); // Enable mouse click reporting
  WriteRaw("\033[?1006h"); // Enable SGR extended mouse mode
  WriteRaw("\033[?1015l"); // Ensure urxvt mode is disabled
#endif

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

#ifndef _WIN32
  WriteRaw("\033[?1000l"); // Disable mouse click
  WriteRaw("\033[?1006l"); // Disable SGR extended mouse mode
#endif

  if (use_alt_buffer_) {
    WriteRaw("\033[?1049l"); // Disable alternative screen buffer
  }
  std::fflush(stdout);

#ifdef _WIN32
  GGEMS_CHECK_FATAL(RestoreWinConsole(),
                    "Impossible to restore Windows console mode.");
#else
  GGEMS_CHECK_FATAL(RestorePosixTerminal(),
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
  HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);

  if (hIn == INVALID_HANDLE_VALUE) {
    return TerminalKey::None;
  }

  DWORD available = 0;
  if (!GetNumberOfConsoleInputEvents(hIn, &available) || available == 0) {
    return TerminalKey::None;
  }

  INPUT_RECORD record{};
  DWORD read{0};

  while (available > 0) {
    if (!ReadConsoleInputW(hIn, &record, 1, &read) || read == 0) {
      return TerminalKey::None;
    }

    if (record.EventType == KEY_EVENT) {
      KEY_EVENT_RECORD &key = record.Event.KeyEvent;

      if (!key.bKeyDown) {
        --available;
        continue;
      }

      switch (key.wVirtualKeyCode) {
      case VK_RETURN:
        return TerminalKey::Enter;
      case VK_SPACE:
        return TerminalKey::Space;
      case VK_UP:
        return TerminalKey::Up;
      case VK_DOWN:
        return TerminalKey::Down;
      case VK_PRIOR:
        return TerminalKey::PageUp;
      case VK_NEXT:
        return TerminalKey::PageDown;
      case VK_HOME:
        return TerminalKey::Home;
      default:
        break;
      }
    } else if (record.EventType == MOUSE_EVENT) {
      MOUSE_EVENT_RECORD &mouse = record.Event.MouseEvent;

      if (mouse.dwEventFlags == MOUSE_WHEELED) {
        SHORT delta = static_cast<SHORT>(HIWORD(mouse.dwButtonState));
        if (delta > 0) {
          return TerminalKey::WheelUp;
        }

        if (delta < 0) {
          return TerminalKey::WheelDown;
        }
      }
    }

    --available;
  }

  return TerminalKey::None;
#else
  unsigned char seq[32] = {};
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

  // --- CSI sequences
  if (seq[0] == 0x1B && seq[1] == '[') {
    // Arrow keys
    if (count >= 3) {
      switch (seq[2]) {
      case 'A':
        return TerminalKey::Up;
      case 'B':
        return TerminalKey::Down;
      case 'H':
        return TerminalKey::Home;
      default:
        break;
      }
    }

    // Tilde-terminated keys: Home / PageUp / PageDown
    if (count >= 4) {
      if (seq[2] == '1' && seq[3] == '~') {
        return TerminalKey::Home;
      }
      if (seq[2] == '5' && seq[3] == '~') {
        return TerminalKey::PageUp;
      }
      if (seq[2] == '6' && seq[3] == '~') {
        return TerminalKey::PageDown;
      }
    }

    // SGR mouse mode: ESC [ < button ; x ; y M/m
    if (count >= 6 && seq[2] == '<') {
      int button = 0;
      int x = 0;
      int y = 0;
      char suffix = '\0';

      if (std::sscanf(reinterpret_cast<char const *>(seq), "\x1b[<%d;%d;%d%c",
                      &button, &x, &y, &suffix) == 4) {
        (void)x;
        (void)y;
        (void)suffix;

        // Wheel up/down in SGR mode
        if (button == 64) {
          return TerminalKey::WheelUp;
        }
        if (button == 65) {
          return TerminalKey::WheelDown;
        }
      }
    }
  }

  // --- SS3 sequences used by some terminals for Home
  if (seq[0] == 0x1B && seq[1] == 'O') {
    if (count >= 3 && seq[2] == 'H') {
      return TerminalKey::Home;
    }
  }

  return TerminalKey::None;
#endif
}
} // namespace ggems::render
