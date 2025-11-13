#include <pybind11/pybind11.h>

#include "GGEMS/core/GGEMSRun.hh"

namespace py = pybind11;

void GGEMSInitRun(py::module_ &m) {
  using ggems::core::GGEMSRun;

  py::class_<GGEMSRun>(m, "GGEMSRun")
      .def(py::init<>())
      .def("run", &GGEMSRun::Run)
      .def("initialise", &GGEMSRun::Initialise);
}
