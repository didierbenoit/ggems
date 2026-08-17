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
 * \brief Declares process-wide GGEMS output-mode and runtime controls.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <string>
#include <string_view>
#include <cstdint>

/// \endcond
#include "GGEMS/logging/GGEMSOutputState.hh"

namespace ggems::core {

/*!
 * \brief Identifies the active GGEMS output destination mode.
 */
enum class OutputMode : std::uint8_t { Term = 0, Gui };
/*!
 * \var ggems::core::OutputMode ggems::core::OutputMode::Term
 * \brief Routes GGEMS output to the terminal logger sink.
 */
/*!
 * \var ggems::core::OutputMode ggems::core::OutputMode::Gui
 * \brief Routes GGEMS output to the in-memory state used by the GUI.
 */

/*!
 * \brief Returns the currently selected output mode.
 *
 * \return Current output mode.
 */
auto GetOutputMode() noexcept -> OutputMode;

/*!
 * \brief Returns whether the output mode has configured the logger sinks.
 *
 * \return True after output configuration has been applied.
 */
auto IsOutputConfigured() noexcept -> bool;
/*!
 * \brief Returns whether the GGEMS output runtime is marked as started.
 *
 * \return True while the output runtime is started.
 */
auto IsOutputRuntimeStarted() noexcept -> bool;

/*!
 * \brief Configures GGEMS output for an explicit mode.
 *
 * Repeating the already configured mode is accepted. Switching to another mode
 * after configuration is rejected.
 *
 * \param[in] mode Output mode to configure.
 * \throws GGEMSFatal If a different mode has already been configured or a configured log file cannot be opened.
 */
auto SetOutputMode(OutputMode mode) -> void;
/*!
 * \brief Configures GGEMS output from a textual mode selector.
 *
 * Accepted selectors are ``term``/``terminal`` and ``gui``/``imgui``.
 * Matching is case-insensitive.
 *
 * \param[in] mode Textual output-mode selector.
 * \throws GGEMSFatal If the selector is unknown, a different mode has already been configured, or a configured log file cannot be opened.
 */
auto SetOutputMode(std::string_view mode) -> void;

/*!
 * \brief Configures an optional plain-text log file.
 *
 * \param[in] path Non-empty destination path.
 * \throws GGEMSFatal If the path is empty, the output runtime is started, or the file cannot be opened after output configuration.
 */
auto SetOutputFile(std::string_view path) -> void;
/*!
 * \brief Disables the optional output log file.
 *
 * \throws GGEMSFatal If the output runtime is started.
 */
auto ClearOutputFile() -> void;

/*!
 * \brief Starts the configured GGEMS output runtime.
 *
 * Repeated calls while already started are ignored. Terminal mode prepares the
 * terminal and emits the GGEMS banner on first start after a stopped state.
 *
 * \throws GGEMSFatal If no output mode has been configured.
 */
auto StartOutputRuntime() -> void;
/*!
 * \brief Marks the GGEMS output runtime as stopped.
 */
auto StopOutputRuntime() noexcept -> void;

/*!
 * \brief Returns the process-wide in-memory output state.
 *
 * The state is created lazily on first access.
 *
 * \return Reference to the process-wide output state.
 */
auto GetOutputState() -> GGEMSOutputState &;

/*!
 * \brief Returns the canonical string representation of an output mode.
 *
 * \param[in] mode Output mode to convert.
 * \return ``term`` for terminal mode or ``gui`` for GUI mode.
 */
[[nodiscard]] inline auto ToString(OutputMode mode) -> std::string {
  switch (mode) {
  case OutputMode::Term:
    return "term";
  case OutputMode::Gui:
    return "gui";
  }
  return "term";
}
} // namespace ggems::core
