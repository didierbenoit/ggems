#include <memory>
#include <string>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "GGEMS/frameworks/GGEMSOpenCL.hh"

namespace py = pybind11;

void BindOpenCL(py::module_ &module) {
  py::class_<ggems::ocl::GGEMSOpenCL,
             std::unique_ptr<ggems::ocl::GGEMSOpenCL, py::nodelete>>(
      module, "GGEMSOpenCL")

      .def(py::init([]() -> ggems::ocl::GGEMSOpenCL * {
             return &ggems::ocl::GGEMSOpenCL::GetInstance();
           }),
           py::return_value_policy::reference)

      .def("print_platforms", &ggems::ocl::GGEMSOpenCL::PrintPlatforms)

      .def("print_devices", &ggems::ocl::GGEMSOpenCL::PrintDevices)

      .def("print_contexts", &ggems::ocl::GGEMSOpenCL::PrintContexts)

      .def("initialize", &ggems::ocl::GGEMSOpenCL::Initialize)

      .def(
          "select_devices",
          [](ggems::ocl::GGEMSOpenCL &opencl, std::string const &devices)
              -> void { opencl.SelectDevices({devices}); },
          py::arg("devices"))

      .def("select_devices", &ggems::ocl::GGEMSOpenCL::SelectDevices,
           py::arg("devices"))

      .def("__repr__", [](ggems::ocl::GGEMSOpenCL const &) -> std::string {
        return "<GGEMSOpenCL (singleton) — OpenCL 3.0 backend active>";
      });
}
