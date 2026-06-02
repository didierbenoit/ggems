#ifdef GGEMS_WITH_IMGUI

#include <cstdint>
#include <string>

#include <pybind11/pybind11.h>

#include "GGEMS/ui/GGEMSGuiApplication.hh"

namespace py = pybind11;

void BindGui(py::module_ &m) {
  py::class_<ggems::ui::GGEMSGuiApplication>(m, "GGEMSGuiApplication")
      .def(py::init<std::string, std::int32_t, std::int32_t>(),
           py::arg("title") = "GGEMS GuiMode", py::arg("width") = 1600,
           py::arg("height") = 900)
      .def("initialise", &ggems::ui::GGEMSGuiApplication::Initialise)
      .def("run", &ggems::ui::GGEMSGuiApplication::Run,
           py::call_guard<py::gil_scoped_release>())
      .def("is_initialised", &ggems::ui::GGEMSGuiApplication::IsInitialised);
}

#endif
