#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "GGEMS/core/GGEMSLogger.hh"
#include "GGEMS/core/GGEMSOutputMode.hh"

namespace py = pybind11;

void BindCore(py::module_ &m) {
  /* --------------------------------------------- */
  /* --------------------------------------------- */
  /* --------------------------------------------- */

  m.def(
      "set_output_mode",
      [](std::string const &mode) { ggems::core::SetOutputMode(mode); },
      py::arg("mode"), "Select output mode: 'term', 'gui', or 'cluster'.");

  /* --------------------------------------------- */
  /* --------------------------------------------- */
  /* --------------------------------------------- */

  py::class_<ggems::core::GGEMSLogger,
             std::unique_ptr<ggems::core::GGEMSLogger, py::nodelete>>(
      m, "GGEMSLogger")
      .def(py::init([]() -> ggems::core::GGEMSLogger * {
             return &ggems::core::GGEMSLogger::GetInstance();
           }),
           py::return_value_policy::reference)

      .def("set_detail_level", &ggems::core::GGEMSLogger::SetDetailLevel,
           py::arg("detail"), "Set depth of verbosity.")

      .def("__repr__", [](ggems::core::GGEMSLogger const &) {
        return "<GGEMSLogger (singleton) — global logging interface>";
      });
}
