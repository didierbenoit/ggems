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
 * \brief Implements process-wide GGEMS output configuration and runtime
 * control.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <atomic>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <cctype>

/// \endcond
#include "GGEMS/logging/GGEMSLogger.hh"
#include "GGEMS/logging/GGEMSOutputMode.hh"
#include "GGEMS/GGEMSException.hh"

#include "GGEMS/logging/GGEMSOutputStateSink.hh"
#include "GGEMS/logging/GGEMSOutputState.hh"
#include "GGEMS/render/GGEMSBanner.hh"
#include "GGEMS/render/GGEMSVisualLine.hh"
#include "GGEMS/utf/GGEMSUTF.hh"
#include "GGEMS/render/GGEMSColor.hh"

#if defined(_WIN32)
/// \cond
#include <windows.h>
/// \endcond
#endif

namespace ggems::core {

namespace {

// =============================================================================
// =============================================================================

/*!
 * \brief Process-wide selected output mode.
 */
OutputMode g_mode{OutputMode::Term};
/*!
 * \brief Indicates whether logger sinks have been configured for an output
 * mode.
 */
bool g_configured{false};

/*!
 * \brief Optional plain-text output file requested by the user.
 */
std::optional<std::string> g_output_file_path{};

/*!
 * \brief Lazily created process-wide GUI output state.
 */
std::unique_ptr<GGEMSOutputState> g_state{};
/*!
 * \brief Process-wide output-runtime started flag.
 */
std::atomic<bool> g_output_running{false};

// =============================================================================
// =============================================================================

/*!
 * \brief Normalizes a textual output selector to lowercase.
 *
 * \param[in] mode User-provided output selector.
 * \return Lowercase selector.
 */
[[nodiscard]] auto NormalizeOutputMode(std::string_view mode) -> std::string {
  std::string normalized{mode};

  for (char &character : normalized) {
    character =
        static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
  }

  return normalized;
}

// =============================================================================
// =============================================================================

#if defined(_WIN32)
/*!
 * \brief Enables virtual-terminal processing for one Windows standard handle.
 *
 * Failure is intentionally ignored because terminal preparation is best effort.
 *
 * \param[in] standard_handle Windows standard-handle identifier.
 */
void EnableWindowsVirtualTerminal(DWORD standard_handle) noexcept {
  HANDLE handle = GetStdHandle(standard_handle);

  if (handle == nullptr || handle == INVALID_HANDLE_VALUE) {
    return;
  }

  DWORD console_mode{0};
  if (GetConsoleMode(handle, &console_mode) == FALSE) {
    return;
  }

  console_mode |= ENABLE_PROCESSED_OUTPUT;
  console_mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

  static_cast<void>(SetConsoleMode(handle, console_mode));
}

// =============================================================================
// =============================================================================

/*!
 * \brief Prepares the Windows console for UTF-8 and ANSI terminal output.
 */
void PrepareWindowsTerminal() noexcept {
  static_cast<void>(SetConsoleCP(CP_UTF8));
  static_cast<void>(SetConsoleOutputCP(CP_UTF8));

  EnableWindowsVirtualTerminal(STD_OUTPUT_HANDLE);
  EnableWindowsVirtualTerminal(STD_ERROR_HANDLE);
}
#endif

// =============================================================================
// =============================================================================

/*!
 * \brief Parses a normalized user-facing output-mode selector.
 *
 * \param[in] mode Selector to parse.
 * \return Parsed output mode.
 * \throws GGEMSFatal If the selector is unknown.
 */
auto Parse(std::string_view mode) -> OutputMode {
  std::string const value = NormalizeOutputMode(mode);

  if (value == "term" || value == "terminal") {
    return OutputMode::Term;
  }

  if (value == "gui" || value == "imgui") {
    return OutputMode::Gui;
  }

  throw ggems::core::GGEMSFatal(
      "Unknown output mode. Expected: 'term' or 'gui'.");
}

// =============================================================================
// =============================================================================

/*!
 * \brief Adds the configured file sink when an output path is present.
 *
 * \param[in,out] logger Logger receiving the optional sink.
 * \throws GGEMSFatal If the configured file cannot be opened.
 */
auto AddOptionalFileSink(GGEMSLogger &logger) -> void {
  if (g_output_file_path.has_value()) {
    logger.AddSink(std::make_unique<FileSink>(*g_output_file_path));
  }
}

// =============================================================================
// =============================================================================

/*!
 * \brief Rebuilds logger sinks for the selected output mode.
 *
 * Terminal mode installs a standard-output sink; GUI mode installs an output-
 * state sink. The optional file sink is appended in either mode.
 *
 * \param[in] mode Output mode whose sinks are installed.
 */
auto ConfigureLoggerForMode(OutputMode mode) -> void {
  GGEMSLogger &logger = GGEMSLogger::GetInstance();

  logger.ClearSinks();

  switch (mode) {
  case OutputMode::Term:
    logger.AddSink(std::make_unique<StdoutSink>());
    AddOptionalFileSink(logger);
    logger.SetForceColor(true);
    logger.SetForceEncoding(Encoding::Unicode);
    break;

  case OutputMode::Gui:
    logger.AddSink(std::make_unique<GGEMSOutputStateSink>(GetOutputState()));
    AddOptionalFileSink(logger);
    logger.SetForceColor(true);
    logger.SetForceEncoding(Encoding::Unicode);
    break;
  }

  g_configured = true;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Converts one visual banner line to terminal UTF-8 and ANSI text.
 *
 * \param[in] line Visual line to encode.
 * \return Terminal-ready text.
 */
auto ToTerminalText(render::WrappedLine const &line) -> std::string {
  std::string out;

  bool const use_color = GGEMSLogger::GetInstance().UseColor();
  bool wrote_color{false};

  for (render::VisualSegment const &segment : line.segments) {
    if (use_color) {
      render::AppendAnsiColor(out, segment.color);
      wrote_color = true;
    }

    out += utf::UTF32ToUTF8(segment.text);
  }

  if (wrote_color) {
    render::AppendAnsiControl(out, render::AnsiControl::ResetColor);
  }

  return out;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Writes the GGEMS banner directly to the terminal stream.
 */
auto EmitTerminalBanner() -> void {
  auto const lines = render::BuildBannerLines();

  for (render::WrappedLine const &line : lines) {
    std::cout << ToTerminalText(line) << '\n';
  }

  std::cout << '\n';
}
} // namespace

// =============================================================================
// =============================================================================

auto GetOutputMode() noexcept -> OutputMode { return g_mode; }

// =============================================================================
// =============================================================================

auto IsOutputConfigured() noexcept -> bool { return g_configured; }

// =============================================================================
// =============================================================================

auto IsOutputRuntimeStarted() noexcept -> bool {
  return g_output_running.load(std::memory_order_relaxed);
}

// =============================================================================
// =============================================================================

auto GetOutputState() -> GGEMSOutputState & {
  if (!g_state) {
    g_state = std::make_unique<GGEMSOutputState>();
  }

  return *g_state;
}

// =============================================================================
// =============================================================================

auto SetOutputMode(OutputMode mode) -> void {
  if (mode == g_mode && g_configured) {
    return;
  }

  if (g_configured) {
    throw ggems::core::GGEMSFatal(
        "Output mode already configured; it must be set exactly once before "
        "starting GGEMS output runtime.");
  }

  g_mode = mode;
  ConfigureLoggerForMode(g_mode);
}

// =============================================================================
// =============================================================================

auto SetOutputMode(std::string_view mode) -> void {
  SetOutputMode(Parse(mode));
}

// =============================================================================
// =============================================================================

auto SetOutputFile(std::string_view path) -> void {
  if (path.empty()) {
    throw ggems::core::GGEMSFatal("Output file path must not be empty.");
  }

  if (g_output_running.load(std::memory_order_relaxed)) {
    throw ggems::core::GGEMSFatal(
        "Output file cannot be changed while output runtime is started.");
  }

  g_output_file_path = std::string(path);

  if (g_configured) {
    ConfigureLoggerForMode(g_mode);
  }
}

// =============================================================================
// =============================================================================

auto ClearOutputFile() -> void {
  if (g_output_running.load(std::memory_order_relaxed)) {
    throw ggems::core::GGEMSFatal(
        "Output file cannot be cleared while output runtime is started.");
  }

  g_output_file_path.reset();

  if (g_configured) {
    ConfigureLoggerForMode(g_mode);
  }
}

// =============================================================================
// =============================================================================

auto StartOutputRuntime() -> void {
  if (g_output_running.load(std::memory_order_relaxed)) {
    return;
  }

  if (!(g_configured)) {
    ConfigureLoggerForMode(g_mode);
  }

  if (g_mode == OutputMode::Term) {
#if defined(_WIN32)
    PrepareWindowsTerminal();
#endif
  }

  g_output_running.store(true, std::memory_order_relaxed);

  if (g_mode == OutputMode::Term) {
    EmitTerminalBanner();
  }
}

// =============================================================================
// =============================================================================

auto StopOutputRuntime() noexcept -> void {
  g_output_running.store(false, std::memory_order_relaxed);
}
} // namespace ggems::core
