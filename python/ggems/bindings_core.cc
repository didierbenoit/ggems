#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "GGEMS/core/GGEMSLogger.hh"
#include "GGEMS/core/GGEMSOutputState.hh"
#include "GGEMS/core/GGEMSOutputStateBootstrap.hh"

namespace py = pybind11;

void BindCore(py::module_ &m) {
  /* --------------------------------------------- */
  /* --------------------------------------------- */
  /* --------------------------------------------- */

  py::enum_<ggems::core::RunStatus>(m, "RunStatus")
      .value("Starting", ggems::core::RunStatus::Starting)
      .value("Configuring", ggems::core::RunStatus::Configuring)
      .value("Ready", ggems::core::RunStatus::Ready)
      .value("Running", ggems::core::RunStatus::Running)
      .value("Finished", ggems::core::RunStatus::Finished)
      .value("Failed", ggems::core::RunStatus::Failed);

  /* --------------------------------------------- */
  /* --------------------------------------------- */
  /* --------------------------------------------- */

  py::class_<ggems::core::GGEMSOutputState>(m, "GGEMSOutputState");

  m.def(
      "get_output_state",
      []() -> ggems::core::GGEMSOutputState & {
        return ggems::core::EnsureOutputState();
      },
      py::return_value_policy::reference,
      "Return the global GGEMS output state (read-only observer).");

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

      .def(
          "attach_sink",
          [](ggems::core::GGEMSLogger &self, const std::string &type,
             const std::string &path = "") {
            if (type == "file") {
              if (path.empty())
                throw std::invalid_argument(
                    "FileSink requires a valid file path.");
              self.AttachSink(std::make_unique<ggems::core::FileSink>(path));
            } else {
              throw std::invalid_argument("Unknown sink type: " + type);
            }
          },
          py::arg("type"), py::arg("path") = "",
          "Attach a new log sink ('file').")

      .def("force_color", &ggems::core::GGEMSLogger::SetForceColor,
           py::arg("force"),
           "Force colour output (True/False/None = auto-detect).")

      .def("set_detail_level", &ggems::core::GGEMSLogger::SetDetailLevel,
           py::arg("detail"), "Set depth of verbosity.")

      .def("__repr__", [](ggems::core::GGEMSLogger const &) {
        return "<GGEMSLogger (singleton) — global logging interface>";
      });
}
