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
 * \brief Defines Python bindings for built-in GGEMS radionuclide definitions.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <cstddef>
#include <memory>
#include <string>
#include <utility>

#include <pybind11/pybind11.h>

#include "detail/GGEMSPythonRadionuclideDefinition.hh"

#include "GGEMS/radioactivity/GGEMSRadionuclideDefinition.hh"
#include "GGEMS/radioactivity/builtins/GGEMSBuiltInRadionuclides.hh"

namespace py = pybind11;

// =============================================================================
// =============================================================================

auto BindRadionuclide(py::module_ &module) -> void {
  using ggems::core::radioactivity::GGEMSRadionuclideDefinition;
  using ggems::python::detail::GGEMSRadionuclideDefinitionHandle;

  // === === ===
  py::class_<GGEMSRadionuclideDefinitionHandle>(module,
                                                "RadionuclideDefinition",
                                                R"pbdoc(
Read-only handle to a built-in GGEMS radionuclide definition.

Radionuclide definitions are provided by GGEMS and are intended to be attached
to activity-driven sources. Use ``ggems.radionuclide.load()`` to obtain a
definition.

Examples:
    >>> f18 = ggems.radionuclide.load("F-18")
    >>> print(f18)
    F-18
    >>> f18.verbose()
)pbdoc")
    .def(
      "__str__",
      [](GGEMSRadionuclideDefinitionHandle const &handle) -> std::string {
        return std::string{handle.GetDefinition()->GetCanonicalName()};
      },
      R"pbdoc(
Return the canonical GGEMS radionuclide identifier.
)pbdoc")

    .def(
      "verbose",
      [](GGEMSRadionuclideDefinitionHandle const &handle) -> void {
        ggems::core::radioactivity::builtins::VerboseBuiltInRadionuclide(
          *handle.GetDefinition());
      },
      R"pbdoc(
Print detailed information about the radionuclide through GGEMS output.

The report includes the half-life, emission channels, emission yields, and
energy-distribution summaries.
)pbdoc");

  // === === ===
  module.def(
    "available",
    [] -> py::tuple {
      auto const names =
        ggems::core::radioactivity::builtins::GetAvailableRadionuclideNames();

      py::tuple result{names.size()};
      for (std::size_t index = 0U; index < names.size(); ++index) {
        result[index] = py::str{names[index]};
      }

      return result;
    },
    R"pbdoc(
Return the canonical names of all built-in GGEMS radionuclides.

Returns:
    tuple[str, ...]: Canonical radionuclide identifiers accepted by
    ``ggems.radionuclide.load()``.

Examples:
    >>> "F-18" in ggems.radionuclide.available()
    True
)pbdoc");

  // === === ===
  module.def(
    "load",
    [](std::string const &canonical_name) -> GGEMSRadionuclideDefinitionHandle {
      auto definition =
        ggems::core::radioactivity::builtins::BuildBuiltInRadionuclide(
          canonical_name);

      return GGEMSRadionuclideDefinitionHandle{
        std::make_shared<GGEMSRadionuclideDefinition const>(
          std::move(*definition))};
    },
    py::arg("name"),
    R"pbdoc(
Load a built-in GGEMS radionuclide definition.

Args:
    name: Canonical built-in radionuclide identifier, such as ``"F-18"`` or
        ``"I-131"``.

Returns:
    RadionuclideDefinition: Read-only handle to the GGEMS radionuclide
    definition.

Examples:
    >>> f18 = ggems.radionuclide.load("F-18")
    >>> source = ggems.source.GGEMSSource()
    >>> source.set_radionuclide(f18, 10.0, "MBq")
)pbdoc");
}
