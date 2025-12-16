/// \cond
#include <cstdio>
/// \endcond

#include "GGEMS/render/GGEMSTerminalPresenter.hh"

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

GGEMSTerminalPresenter::~GGEMSTerminalPresenter() { End(); }

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSTerminalPresenter::Begin(bool use_alt_buffer) noexcept {
  if (started_)
    return;

  use_alt_buffer_ = use_alt_buffer;

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
  if (started_)
    return;

  WriteRaw("\033[0m");   // Reset and go back default terminal
  WriteRaw("\033[?25h"); // Show the cursor
  if (use_alt_buffer_) {
    WriteRaw("\033[?1049l"); // Disable alternative screen buffer
  }
  std::fflush(stdout);

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
