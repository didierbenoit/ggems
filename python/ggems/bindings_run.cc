#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "GGEMS/frameworks/GGEMSExecutor.hh"

namespace py = pybind11;

void GGEMSInitRun(py::module_ &m) {
  using ggems::run::GGEMSExecutor;

  py::class_<GGEMSExecutor>(m, "GGEMSExecutor")
      .def(py::init<>())
      .def("selectDevices", &GGEMSExecutor::SelectDevices)
      .def("run", &GGEMSExecutor::Run)
      .def("initialize", &GGEMSExecutor::Initialize);
}
