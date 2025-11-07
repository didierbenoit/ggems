#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "GGEMS/core/GGEMSLogger.hh"
using namespace ggems::core;

namespace py = pybind11;

void GGEMSInitCore(py::module_ &m) {
  py::class_<GGEMSLogger, std::unique_ptr<GGEMSLogger, py::nodelete>>(m, "GGEMSLogger")
    .def(py::init([]() -> GGEMSLogger* { return &GGEMSLogger::GetInstance(); }),
         py::return_value_policy::reference)

    .def("force_color",
      &GGEMSLogger::SetForceColor,
      py::arg("force"),
      "Force colour output (True/False/None = auto-detect).")

    .def("set_detail_level",
      &GGEMSLogger::SetDetailLevel,
      py::arg("detail"),
      "Set additional detail depth (indentation or sub-verbosity).")

    .def("__repr__", [](const GGEMSLogger&) {
      return "<GGEMSLogger (singleton) — global logging interface>";
    });

    m.def("force_color", [](std::optional<bool> force) {
        GGEMSLogger::GetInstance().SetForceColor(force);
      },
      py::arg("force"), "Force colour display globally (True/False/None).");

    m.def("set_detail_level", [](int d) {
        GGEMSLogger::GetInstance().SetDetailLevel(d);
      },
      py::arg("detail"), "Set detail level globally.");
}
