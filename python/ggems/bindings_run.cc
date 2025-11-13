#include "GGEMS/frameworks/GGEMSExecutor.hh"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

void GGEMSInitRun(py::module_ &m) {
  using ggems::run::GGEMSExecutor;

  py::class_<GGEMSExecutor>(m, "GGEMSExecutor")
      .def(py::init<>())
      .def("select_devices", &GGEMSExecutor::SelectDevices)
      .def("run", &GGEMSExecutor::Run)
      .def("initialize", &GGEMSExecutor::Initialize);
}
