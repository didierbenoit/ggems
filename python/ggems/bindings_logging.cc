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
 * \brief Defines Python bindings for GGEMS logging and output controls.
 *
 * Provides runtime Python help for output-mode setup, optional file logging, runtime lifecycle control, and logger verbosity.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

/// \endcond
#include "GGEMS/logging/GGEMSLogger.hh"
#include "GGEMS/logging/GGEMSOutputMode.hh"

namespace py = pybind11;

/*!
 * \brief Registers GGEMS logging and output bindings in a Python module.
 *
 * \param[in,out] m Python logging submodule receiving the bindings.
 */
void BindLogging(py::module_ &m) {
  m.doc() = R"doc(GGEMS logging and output controls.

Typical terminal setup:
    import ggems

    ggems.logging.set_output_mode("term")
    ggems.logging.start_output_runtime()

    # Configure and run GGEMS...

    ggems.logging.stop_output_runtime()

Output configuration is process-wide. Select the output mode before starting
runtime output. An optional log file may be configured while the runtime is
stopped.
)doc";
  /* --------------------------------------------- */
  /* --------------------------------------------- */
  /* --------------------------------------------- */

  m.def(
      "set_output_mode",
      [](std::string const &mode) { ggems::core::SetOutputMode(mode); },
      py::arg("mode"),
      R"doc(Configure the process-wide GGEMS output mode.

Accepted values are "term"/"terminal" and "gui"/"imgui". Matching is
case-insensitive.

The mode configures the logger sinks immediately. Repeating the same mode is
allowed, but switching to a different mode after configuration is rejected.
Configure the mode before start_output_runtime().

Parameters:
    mode: Output-mode selector.

Example:
    ggems.logging.set_output_mode("term")
)doc");

  m.def(
      "set_output_file",
      [](std::string const &path) { ggems::core::SetOutputFile(path); },
      py::arg("path"),
      R"doc(Enable an optional plain-text GGEMS log file.

The path must be non-empty and can only be changed while the output runtime is
stopped. If the output mode is already configured, the logger sinks are rebuilt
immediately. The file sink opens its destination in truncate mode and reports
an error if the file cannot be opened.

Parameters:
    path: Destination log-file path.

Example:
    ggems.logging.set_output_file("ggems.log")
)doc");

  m.def(
      "clear_output_file", []() { ggems::core::ClearOutputFile(); },
      R"doc(Disable the optional GGEMS log file.

This operation is only allowed while the output runtime is stopped. If an
output mode is already configured, the remaining sinks are rebuilt immediately.
)doc");

  m.def(
      "start_output_runtime", []() { ggems::core::StartOutputRuntime(); },
      R"doc(Start the configured GGEMS output runtime.

set_output_mode() must be called first. Repeated calls while the runtime is
already started are ignored. In terminal mode GGEMS prepares UTF-8/ANSI output
and emits the GGEMS banner.
)doc");

  m.def(
      "stop_output_runtime", []() { ggems::core::StopOutputRuntime(); },
      R"doc(Stop the GGEMS output runtime.

The selected output mode and configured sinks are retained, so the runtime can
be started again later.
)doc");

  m.def(
      "is_output_runtime_started",
      []() { return ggems::core::IsOutputRuntimeStarted(); },
      R"doc(Return whether the GGEMS output runtime is currently started.

Returns:
    bool: True when runtime output is started.
)doc");

  /* --------------------------------------------- */
  /* --------------------------------------------- */
  /* --------------------------------------------- */

  py::class_<ggems::core::GGEMSLogger,
             std::unique_ptr<ggems::core::GGEMSLogger, py::nodelete>>(
      m, "GGEMSLogger",
      R"doc(Access the process-wide GGEMS logger.

GGEMSLogger is a singleton. Python exposes the verbosity/detail control while
output destination configuration is handled by the module-level
set_output_mode(), set_output_file(), and clear_output_file() functions.

Example:
    logger = ggems.logging.GGEMSLogger()
    logger.set_detail_level(2)
)doc")
      .def(py::init([]() -> ggems::core::GGEMSLogger * {
             return &ggems::core::GGEMSLogger::GetInstance();
           }),
           py::return_value_policy::reference,
           R"doc(Return the process-wide GGEMS logger singleton.)doc")

      .def("set_detail_level", &ggems::core::GGEMSLogger::SetDetailLevel,
           py::arg("detail") = 1,
           R"doc(Set the maximum GGEMS informational detail depth.

Formatted log messages whose depth is greater than this value are discarded
before formatting. Standard informational messages use depth 0; GGEMS_INFOEX
messages use the explicit depth supplied by the caller.

Parameters:
    detail: Maximum accepted logging depth. The default is 1.

Example:
    ggems.logging.GGEMSLogger().set_detail_level(2)
)doc")

      .def("__repr__", [](ggems::core::GGEMSLogger const &) {
        return "<GGEMSLogger (singleton) — global logging interface>";
      });
}
