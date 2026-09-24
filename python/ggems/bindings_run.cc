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
 * \brief Defines Python bindings for GGEMS run configuration and execution.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <string>

#include <pybind11/pybind11.h>

#include "detail/GGEMSPythonQuantityConversion.hh"

#include "GGEMS/random/GGEMSRandom.hh"
#include "GGEMS/observer/GGEMSTransportObserver.hh"
#include "GGEMS/GGEMSRun.hh"
#include "GGEMS/units/GGEMSTimeUnits.hh"

namespace py = pybind11;

// =============================================================================
// =============================================================================

void BindRun(py::module_ &module) {
  // === === ===
  py::class_<ggems::core::GGEMSRun>(module, "GGEMSRun")
    .def(py::init<>())

    .def("run", &ggems::core::GGEMSRun::Run,
         py::call_guard<py::gil_scoped_release>())

    .def("initialize", &ggems::core::GGEMSRun::Initialize)

    .def("set_random", &ggems::core::GGEMSRun::SetRandom, py::arg("random"))

    .def("add_source", &ggems::core::GGEMSRun::AddSource, py::arg("source"))

    .def("set_observer", &ggems::core::GGEMSRun::SetObserver,
         py::arg("observer"))

    .def(
      "set_time",
      [](ggems::core::GGEMSRun &self, double start, double stop, double step,
         std::string const &unit) -> void {
        self.SetTimePicoSecond(
          ggems::python::detail::MakeQuantityOrThrow<ggems::units::TimePoint>(
            start, unit)
            .value,
          ggems::python::detail::MakeQuantityOrThrow<ggems::units::TimePoint>(
            stop, unit)
            .value,
          ggems::python::detail::MakeQuantityOrThrow<ggems::units::Duration>(
            step, unit)
            .value);
      },
      py::arg("start"), py::arg("stop"), py::arg("step"), py::arg("unit") = "s")

    .def("reset_time", &ggems::core::GGEMSRun::ResetTime)

    .def("has_time_configuration", &ggems::core::GGEMSRun::HasTimeConfiguration)

    .def("has_next_time_step", &ggems::core::GGEMSRun::HasNextTimeStep)

    .def(
      "get_current_time",
      [](ggems::core::GGEMSRun const &self, std::string const &unit) -> double {
        return ggems::python::detail::ConvertQuantityToDoubleOrThrow(
          ggems::units::TimePoint{.value = self.GetCurrentTimePicoSecond()},
          unit);
      },
      py::arg("unit") = "s")

    .def(
      "get_current_time_window",
      [](ggems::core::GGEMSRun const &self,
         std::string const &unit) -> py::tuple {
        auto const window = self.GetCurrentTimeWindowPicoSecond();
        return py::make_tuple(
          ggems::python::detail::ConvertQuantityToDoubleOrThrow(
            ggems::units::TimePoint{.value = window.start_ps}, unit),
          ggems::python::detail::ConvertQuantityToDoubleOrThrow(
            ggems::units::TimePoint{.value = window.stop_ps}, unit));
      },
      py::arg("unit") = "s");
}
