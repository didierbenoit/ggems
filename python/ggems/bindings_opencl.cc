#include <pybind11/pybind11.h>

#include "GGEMS/frameworks/GGEMSOpenCL.hh"

namespace py = pybind11;

void GGEMSInitOpenCL(py::module_ &m) {
  py::class_<GGEMSOpenCL, std::unique_ptr<GGEMSOpenCL, py::nodelete>>(
      m, "GGEMSOpenCL")
      .def(py::init(
               []() -> GGEMSOpenCL * { return &GGEMSOpenCL::GetInstance(); }),
           py::return_value_policy::reference)

      .def("print_platforms", &GGEMSOpenCL::PrintPlatforms)

      .def("print_devices", &GGEMSOpenCL::PrintDevices)

      .def("clean", &GGEMSOpenCL::Clean)

      .def("__repr__", [](const GGEMSOpenCL &) {
        return "<GGEMSOpenCL (singleton) — OpenCL 3.0 backend active>";
      });

  m.def(
      "print_platforms", []() { GGEMSOpenCL::GetInstance().PrintPlatforms(); },
      "Print infos about all found OpenCL platforms");

  m.def(
      "print_devices", []() { GGEMSOpenCL::GetInstance().PrintDevices(); },
      "Print infos about all found OpenCL devices");

  m.def(
      "clean", []() { GGEMSOpenCL::GetInstance().Clean(); },
      "Release the internal compilers of the platform");
}
