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
 * \brief XXX
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <memory>

#include <pybind11/pybind11.h>

#include "GGEMS/observer/GGEMSTransportObserver.hh"

namespace py = pybind11;

// =============================================================================
// =============================================================================

void BindObserver(py::module_ &mod) {
  using ggems::core::observer::GGEMSTransportObserver;

  // === === ===
  py::class_<GGEMSTransportObserver, std::shared_ptr<GGEMSTransportObserver>>(
    mod, "GGEMSTransportObserver")
    .def(py::init<>())

    .def("enable", &GGEMSTransportObserver::Enable, py::arg("enabled") = true,
         py::return_value_policy::reference_internal)

    .def("disable", &GGEMSTransportObserver::Disable,
         py::return_value_policy::reference_internal)

    .def("set_capacity", &GGEMSTransportObserver::SetRecordCapacity,
         py::arg("record_capacity"),
         py::return_value_policy::reference_internal)

    .def("set_max_stored_record_count",
         &GGEMSTransportObserver::SetMaxStoredRecordCount,
         py::arg("max_stored_record_count"),
         py::return_value_policy::reference_internal)

    .def("capture_first_primaries",
         &GGEMSTransportObserver::CaptureFirstPrimaries,
         py::arg("primary_count"), py::return_value_policy::reference_internal)

    .def("capture_primary", &GGEMSTransportObserver::CapturePrimary,
         py::arg("source_index"), py::arg("primary_index"),
         py::return_value_policy::reference_internal)

    .def("clear_capture_primary", &GGEMSTransportObserver::ClearCapturedPrimary,
         py::return_value_policy::reference_internal)

    .def_property_readonly("record_count",
                           &GGEMSTransportObserver::GetRecordCount)

    .def_property_readonly("overflow_count",
                           &GGEMSTransportObserver::GetOverflowCount)

    .def_property_readonly("captured_primary_count",
                           &GGEMSTransportObserver::GetCapturedPrimaryCount);
}
