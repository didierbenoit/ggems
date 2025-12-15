#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "GGEMS/frameworks/GGEMSOpenCL.hh"

namespace py = pybind11;

void BindOpenCL(py::module_ &m) {
  /* --------------------------------------------- */
  /* --------------------------------------------- */
  /* --------------------------------------------- */

  py::class_<ggems::ocl::GGEMSOpenCL,
             std::unique_ptr<ggems::ocl::GGEMSOpenCL, py::nodelete>>(
      m, "GGEMSOpenCL")
      .def(py::init([]() -> ggems::ocl::GGEMSOpenCL * {
             return &ggems::ocl::GGEMSOpenCL::GetInstance();
           }),
           py::return_value_policy::reference)
      .def("print_platforms", &ggems::ocl::GGEMSOpenCL::PrintPlatforms)
      .def("print_devices", &ggems::ocl::GGEMSOpenCL::PrintDevices)
      .def("print_contexts", &ggems::ocl::GGEMSOpenCL::PrintContexts)
      .def("initialise", &ggems::ocl::GGEMSOpenCL::Initialise)
      .def("select_devices", &ggems::ocl::GGEMSOpenCL::SelectDevices,
           py::arg("devices"))
      .def("clean", &ggems::ocl::GGEMSOpenCL::Clean)
      .def("__repr__", [](ggems::ocl::GGEMSOpenCL const &) {
        return "<GGEMSOpenCL (singleton) — OpenCL 3.0 backend active>";
      });
}
