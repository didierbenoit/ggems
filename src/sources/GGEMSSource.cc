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

/*#include <pybind11/pybind11.h>

#include "GGEMS/sources/GGEMSSource.hh"

std::string Dog::Go(int const& n_times) {
  std::string result;
  for (int i = 0; i < n_times; ++i) result += "woof! ";
  return result;
}

std::string CallGo(Animal *animal) {
  return animal->Go(3);
}

namespace py = pybind11;

PYBIND11_MODULE(_sources, m, py::mod_gil_not_used(), py::multiple_interpreters::per_interpreter_gil()) {
  py::class_<Animal, PyAnimal, py::smart_holder>(m, "Animal")
    .def(py::init<>())
    .def("go", &Animal::Go);

  py::class_<Dog, Animal, py::smart_holder>(m, "Dog")
    .def(py::init<>());

  m.def("CallGo", &CallGo);
}
*/
