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

#include "GGEMS/tools/GGEMSPrint.hh"

Pet::Pet(std::string const& name)
: name_(name) {
  ;
}

void Pet::SetName(std::string const& name) {
  name_ = name;
}

std::string const& Pet::GetName(void) const {
  return name_;
}

namespace py = pybind11;

PYBIND11_MODULE(_tools, m, py::mod_gil_not_used(), py::multiple_interpreters::per_interpreter_gil()) {
  m.doc() = "Class example for pet animal";

  py::class_<Pet>(m, "Pet")
    .def(py::init<std::string const&>());
}

