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
 * \brief Python bindings for the GGEMS OpenCL backend.
 *
 * Exposes the GGEMS OpenCL singleton, platform/device/context reporting,
 * backend initialization, and device-selection facilities to Python.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <memory>
#include <string>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "GGEMS/opencl/GGEMSOpenCL.hh"

namespace py = pybind11;

// =============================================================================
// =============================================================================

void BindOpenCL(py::module_ &module) {
  // === === ===
  py::class_<ggems::ocl::GGEMSOpenCL,
             std::unique_ptr<ggems::ocl::GGEMSOpenCL, py::nodelete>>(
    module, "GGEMSOpenCL",
    R"doc(Access the process-wide GGEMS OpenCL backend.

GGEMSOpenCL is a singleton: every GGEMSOpenCL() call returns the same backend
instance and therefore shares device selection and context state.

Platform and device discovery occurs when the singleton is first accessed.
A typical setup is:

    opencl = ggems.opencl.GGEMSOpenCL()
    opencl.print_devices()
    opencl.select_devices("gpu")
    opencl.initialize()
    opencl.print_contexts()

Text selectors are usually the most portable choice. Numeric selectors refer
to the flattened discovery order across all platforms.
)doc")

    .def(py::init([] -> ggems::ocl::GGEMSOpenCL * {
           return &ggems::ocl::GGEMSOpenCL::GetInstance();
         }),
         py::return_value_policy::reference,
         R"doc(Return the process-wide GGEMS OpenCL singleton.

Repeated construction does not create independent OpenCL backend instances.
)doc")

    .def("print_platforms", &ggems::ocl::GGEMSOpenCL::PrintPlatforms,
         R"doc(Print detailed information about all discovered OpenCL platforms.

Output is written through the GGEMS logger.
)doc")

    .def("print_devices", &ggems::ocl::GGEMSOpenCL::PrintDevices,
         R"doc(Print detailed information about all discovered OpenCL devices.

Output is written through the GGEMS logger. Each device report includes its
platform/device pair and its OpenCL capabilities.
)doc")

    .def("print_contexts", &ggems::ocl::GGEMSOpenCL::PrintContexts,
         R"doc(Print the currently active OpenCL contexts and command queues.

Call initialize() after selecting devices before using this report to inspect
the active compute configuration.
)doc")

    .def("initialize", &ggems::ocl::GGEMSOpenCL::Initialize,
         R"doc(Create OpenCL contexts for the currently selected devices.

Call select_devices() first, then initialize() before running GGEMS workloads
that require OpenCL contexts.

The active contexts are created once and retained for the process lifetime
because live SVM buffers, kernels, and transport workloads keep references to
them. Creating the contexts also freezes the device selection, so
select_devices() can no longer change it; running on another device selection
requires a new process.

Calling initialize() again once the backend is initialized keeps the active
contexts, and the compiled-program cache, unchanged.

Calling initialize() before select_devices() creates no context and leaves the
backend uninitialized, so a later call can still create the contexts.

Device selection and initialization are not synchronized; the caller
serializes them.

Note:
    Context-creation failures are treated as fatal backend initialization
    errors by GGEMS and terminate the process.
)doc")

    .def(
      "select_devices",
      [](ggems::ocl::GGEMSOpenCL &opencl, std::string const &devices) -> void {
        opencl.SelectDevices({devices});
      },
      R"doc(Select OpenCL devices using one selector expression.

The expression may contain semicolon-separated tokens.

Text selectors:
    "cpu" or "gpu"
    "intel", "nvidia", or "amd"

A device type and vendor can be combined and are applied together, for example
"gpu;nvidia". Text matching is case-insensitive and ignores surrounding
whitespace.

Numeric selectors:
    "0"       select one device
    "0-2"     select an inclusive device-index range

Numeric indices refer to the flattened discovery order across all OpenCL
platforms. Numeric and textual selectors cannot be mixed.

Special selector:
    "all"     select every discovered device; it must be used alone

The device selection is frozen once the backend is initialized. After that, a
selection that designates exactly the active devices, in the same order, is
accepted and changes nothing, whichever selector expression is used. Any
selection that would really change the active devices raises RuntimeError
without modifying the current selection; running on another device selection
requires a new process.

Device selection and initialization are not synchronized; the caller
serializes them.

Examples:
    opencl.select_devices("gpu")
    opencl.select_devices("gpu;amd")
    opencl.select_devices("0-1")
    opencl.select_devices("all")
)doc",
      py::arg("devices"))

    .def("select_devices", &ggems::ocl::GGEMSOpenCL::SelectDevices,
         R"doc(Select OpenCL devices from a sequence of selector expressions.

Each sequence entry follows the same rules as the string overload and may
itself contain semicolon-separated tokens.

Text selectors such as "gpu" and "nvidia" can be combined. Numeric selectors
such as "0" and "2-3" can also be combined, but numeric and textual selectors
cannot be mixed. Duplicate numeric device indices are ignored.

Passing an empty sequence selects the first discovered GPU, or the first
available OpenCL device if no GPU is present.

The device selection is frozen once the backend is initialized. After that, a
selection that designates exactly the active devices, in the same order, is
accepted and changes nothing, whichever selector expression is used. Any
selection that would really change the active devices raises RuntimeError
without modifying the current selection; running on another device selection
requires a new process.

Device selection and initialization are not synchronized; the caller
serializes them.

Examples:
    opencl.select_devices(["gpu", "nvidia"])
    opencl.select_devices(["0", "2-3"])
    opencl.select_devices([])
)doc",
         py::arg("devices"))

    .def("__repr__", [](ggems::ocl::GGEMSOpenCL const &) -> std::string {
      return "<GGEMSOpenCL (singleton) — OpenCL 3.0 backend>";
    });
}
