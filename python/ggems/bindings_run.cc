#include <pybind11/pybind11.h>

#include "GGEMS/core/GGEMSRun.hh"

namespace py = pybind11;

void BindRun(py::module_ &m) {
  py::class_<ggems::core::GGEMSRun>(m, "GGEMSRun")
      .def(py::init<>())
      .def("run", &ggems::core::GGEMSRun::Run,
           py::call_guard<py::gil_scoped_release>())
      .def("initialise", &ggems::core::GGEMSRun::Initialise);
}
