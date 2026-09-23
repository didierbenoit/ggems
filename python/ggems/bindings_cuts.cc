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
 * \brief Defines Python bindings for GGEMS Production Cuts.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <optional>
#include <string>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "detail/GGEMSPythonQuantityConversion.hh"

#include "GGEMS/processes/GGEMSProductionCutDescription.hh"
#include "GGEMS/processes/GGEMSProductionCutPolicy.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace py = pybind11;

namespace {

namespace processes = ggems::core::processes;

// =============================================================================
// =============================================================================

auto MakeLength(std::optional<double> value, std::string const &unit)
  -> std::optional<ggems::units::Length> {
  if (!value.has_value()) {
    return std::nullopt;
  }

  return ggems::python::detail::MakeQuantityOrThrow<ggems::units::Length>(
    *value, unit);
}

} // namespace

// =============================================================================
// =============================================================================

auto BindCuts(py::module_ &module) -> void {
  // === === ===
  module.def(
    "set_cut",
    [](std::optional<double> gamma, std::optional<double> electron,
       std::optional<double> positron, std::optional<double> proton,
       std::string const &unit) -> void {
      processes::SetProductionCuts({
        .gamma = MakeLength(gamma, unit),
        .electron = MakeLength(electron, unit),
        .positron = MakeLength(positron, unit),
        .proton = MakeLength(proton, unit),
      });
    },
    py::kw_only(), py::arg("gamma") = py::none(),
    py::arg("electron") = py::none(), py::arg("positron") = py::none(),
    py::arg("proton") = py::none(), py::arg("unit") = "mm",
    R"doc(
Set one or more global Production Cuts.

GGEMS uses 1 mm by default for Gamma, Electron, Positron, and Proton.
Only the channels supplied by the caller are changed.

Args:
    gamma: Gamma Production-Cut length.
    electron: Electron Production-Cut length.
    positron: Positron Production-Cut length.
    proton: Proton Production-Cut length.
    unit: Length unit shared by the supplied values. Defaults to "mm".

Examples:
    Change only the Gamma Cut::

        ggems.cuts.set_cut(gamma=5.0, unit="mm")

    Set all four Cuts::

        ggems.cuts.set_cut(
            gamma=1.0,
            electron=1.0,
            positron=1.0,
            proton=1.0,
            unit="mm",
        )
)doc");

  // === === ===
  module.def("verbose", &processes::VerboseProductionCuts,
             R"doc(
Print the current global Production Cuts.

GGEMS uses 1 mm by default for every Production-Cut channel.
)doc");
}
