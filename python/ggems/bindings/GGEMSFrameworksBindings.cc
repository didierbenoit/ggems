// ************************************************************************
// * This file is part of GGEMS.                                          *
// *                                                                      *
// * GGEMS is free software: you can redistribute it and/or modify        *
// * it under the terms of the GNU General Public License as published by *
// * the Free Software Foundation, either version 3 of the License, or    *
// * (at your option) any later version.                                  *
// *                                                                      *
// * GGEMS is distributed in the hope that it will be useful,             *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
// * GNU General Public License for more details.                         *
// *                                                                      *
// * You should have received a copy of the GNU General Public License    *
// * along with GGEMS.  If not, see <https://www.gnu.org/licenses/>.      *
// *                                                                      *
// ************************************************************************

#include <pybind11/pybind11.h>
#include "GGEMS/frameworks/GGEMSOpenCL.hh"

namespace py = pybind11;

void GGEMSInitFrameworks(py::module_& m) {
  py::class_<GGEMSOpenCL>(m, "GGEMSOpenCL", "GGEMSOpenCL singleton class managing OpenCL ressources", py::module_local())
    .def(py::init([]() -> GGEMSOpenCL* {return &GGEMSOpenCL::GetInstance();}), py::return_value_policy::reference, "Return GGEMSOpenCL singleton instance")
    .def("__del__", [](GGEMSOpenCL&) {}, "Do nothing on deletion (C++ singleton)")
    .def("print_platforms", &GGEMSOpenCL::PrintPlatforms, "Print infos about all found OpenCL platforms")
    .def("clean", &GGEMSOpenCL::Clean, "Release the internal compilers of the platform");
}
