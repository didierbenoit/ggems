#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "GGEMS/frameworks/GGEMSOpenCL.hh"

namespace py = pybind11;

void GGEMSInitOpenCL(py::module_ &m) {
  using ggems::ocl::GGEMSOpenCL;

  py::class_<GGEMSOpenCL, std::unique_ptr<GGEMSOpenCL, py::nodelete>>(
      m, "GGEMSOpenCL")
      .def(py::init(
               []() -> GGEMSOpenCL * { return &GGEMSOpenCL::GetInstance(); }),
           py::return_value_policy::reference)
      .def("print_platforms", &GGEMSOpenCL::PrintPlatforms)
      .def("print_devices", &GGEMSOpenCL::PrintDevices)
      .def("print_contexts", &GGEMSOpenCL::PrintContexts)
      .def("initialise", &GGEMSOpenCL::Initialise)
      .def("select_devices", &GGEMSOpenCL::SelectDevices, py::arg("devices"))
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
      "print_contexts", []() { GGEMSOpenCL::GetInstance().PrintContexts(); },
      "Print infos about all created OpenCL contexts");

  m.def(
      "clean", []() { GGEMSOpenCL::GetInstance().Clean(); },
      "Release the internal compilers of the platform");

  m.def(
      "initialise", []() { GGEMSOpenCL::GetInstance().Initialise(); },
      "OpenCL initialisation, creation of contexts");

  m.def(
      "select_devices",
      [](std::vector<std::string> const &devices) {
        GGEMSOpenCL::GetInstance().SelectDevices(devices);
      },
      "Select OpenCL device(s)");
}
