#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "GGEMS/core/GGEMSLogger.hh"
using namespace ggems::core;

namespace py = pybind11;

void GGEMSInitCore(py::module_ &m) {
  py::class_<GGEMSLogger, std::unique_ptr<GGEMSLogger, py::nodelete>>(
      m, "GGEMSLogger")
      // --- Constructor: returns always the singleton instance ---
      .def(py::init(
               []() -> GGEMSLogger * { return &GGEMSLogger::GetInstance(); }),
           py::return_value_policy::reference)

      // --- Methods available from Python instance ---
      .def(
          "attach_sink",
          [](GGEMSLogger &self, const std::string &type,
             const std::string &path = "") {
            if (type == "file") {
              if (path.empty())
                throw std::invalid_argument(
                    "FileSink requires a valid file path.");
              self.AttachSink(std::make_unique<FileSink>(path));
            } else {
              throw std::invalid_argument("Unknown sink type: " + type);
            }
          },
          py::arg("type"), py::arg("path") = "",
          "Attach a new log sink ('file').")

      .def("force_color", &GGEMSLogger::SetForceColor, py::arg("force"),
           "Force colour output (True/False/None = auto-detect).")

      .def("set_detail_level", &GGEMSLogger::SetDetailLevel, py::arg("detail"),
           "Set additional detail depth (indentation or sub-verbosity).")

      .def("__repr__", [](const GGEMSLogger &) {
        return "<GGEMSLogger (singleton) — global logging interface>";
      });

  // --- Module-level aliases for convenience --------------------------------
  m.def(
      "attach_sink",
      [](const std::string &type, const std::string &path = "") {
        auto &log = GGEMSLogger::GetInstance();
        if (type == "file") {
          if (path.empty())
            throw std::invalid_argument("FileSink requires a valid file path.");
          log.AttachSink(std::make_unique<FileSink>(path));
        } else {
          throw std::invalid_argument("Unknown sink type: " + type);
        }
      },
      py::arg("type"), py::arg("path") = "", "Attach a sink directly (file).");

  m.def(
      "force_color",
      [](std::optional<bool> force) {
        GGEMSLogger::GetInstance().SetForceColor(force);
      },
      py::arg("force"), "Force colour display globally (True/False/None).");

  m.def(
      "set_detail_level",
      [](int d) { GGEMSLogger::GetInstance().SetDetailLevel(d); },
      py::arg("detail"), "Set detail level globally.");
}
