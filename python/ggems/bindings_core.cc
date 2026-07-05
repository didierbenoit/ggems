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
      py::arg("mode"), "Select output mode: 'term', or 'gui'.");

  m.def(
      "set_output_file",
      [](std::string const &path) { ggems::core::SetOutputFile(path); },
      py::arg("path"), "Set optional GGEMS output log file path.");

  m.def(
      "clear_output_file", []() { ggems::core::ClearOutputFile(); },
      "Disable optional GGEMS output log file.");

  m.def(
      "start_output_runtime", []() { ggems::core::StartOutputRuntime(); },
      "Start GGEMS output runtime.");

  m.def(
      "stop_output_runtime", []() { ggems::core::StopOutputRuntime(); },
      "Stop GGEMS output runtime.");

  m.def(
      "is_output_runtime_started",
      []() { return ggems::core::IsOutputRuntimeStarted(); },
      "Return whether GGEMS output runtime is started.");

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
           py::arg("detail") = 1, "Set depth of verbosity.")

      .def("__repr__", [](ggems::core::GGEMSLogger const &) {
        return "<GGEMSLogger (singleton) — global logging interface>";
      });
}
