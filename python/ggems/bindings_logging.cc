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
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <cstdint>
#include <string>

#include <pybind11/pybind11.h>
/// \endcond

#include "GGEMS/logging/GGEMSLogger.hh"
#include "GGEMS/logging/GGEMSOutputMode.hh"

namespace py = pybind11;

/*!
 * \brief Registers GGEMS logging and output bindings in a Python module.
 *
 * \param[in,out] module Python module receiving the bindings.
 */
void BindLogging(py::module_ &module) {
  module.def(
      "start", []() -> void { ggems::core::StartOutputRuntime(); },
      R"doc(Start GGEMS output.

Terminal output is used by default when no output mode has been selected.
Repeated calls while GGEMS output is already started are ignored.
)doc");

  module.def(
      "start",
      [](std::string const &mode) -> void {
        ggems::core::SetOutputMode(mode);
        ggems::core::StartOutputRuntime();
      },
      py::arg("mode"),
      R"doc(Start GGEMS output using the requested mode.

Accepted values are "term"/"terminal" and "gui"/"imgui".

Parameters:
    mode: Output mode to use.
)doc");

  module.def(
      "stop", []() -> void { ggems::core::StopOutputRuntime(); },
      R"doc(Stop GGEMS output.

The selected output mode and configured sinks are retained and may be reused by
a later call to start().
)doc");

  module.def(
      "is_started",
      []() -> bool { return ggems::core::IsOutputRuntimeStarted(); },
      R"doc(Return whether GGEMS output is currently started.

Returns:
    bool: True when GGEMS output is started.
)doc");

  module.def(
      "set_detail_level",
      [](std::int32_t detail) -> void {
        ggems::core::GGEMSLogger::GetInstance().SetDetailLevel(detail);
      },
      py::arg("detail") = 1,
      R"doc(Set the maximum GGEMS informational detail depth.

Parameters:
    detail: Maximum accepted logging depth.
)doc");

  module.def(
      "set_output_file",
      [](std::string const &path) -> void { ggems::core::SetOutputFile(path); },
      py::arg("path"),
      R"doc(Enable an optional plain-text GGEMS log file.

The output file can only be changed while GGEMS output is stopped.

Parameters:
    path: Destination log-file path.
)doc");

  module.def(
      "clear_output_file", []() -> void { ggems::core::ClearOutputFile(); },
      R"doc(Disable the optional GGEMS log file.

This operation is only allowed while GGEMS output is stopped.
)doc");
}
