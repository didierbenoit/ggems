#include <memory>

#include <pybind11/pybind11.h>

#include "GGEMS/core/observer/GGEMSTransportObserver.hh"

namespace py = pybind11;

void BindObserver(py::module_ &mod) {
  using ggems::core::observer::GGEMSTransportObserver;

  py::class_<GGEMSTransportObserver, std::shared_ptr<GGEMSTransportObserver>>(
      mod, "GGEMSTransportObserver")
      .def(py::init<>())

      .def("enable", &GGEMSTransportObserver::Enable, py::arg("enabled") = true,
           py::return_value_policy::reference_internal)

      .def("disable", &GGEMSTransportObserver::Disable,
           py::return_value_policy::reference_internal)

      .def("set_capacity", &GGEMSTransportObserver::SetRecordCapacity,
           py::arg("record_capacity"),
           py::return_value_policy::reference_internal)

      .def("set_max_stored_record_count",
           &GGEMSTransportObserver::SetMaxStoredRecordCount,
           py::arg("max_stored_record_count"),
           py::return_value_policy::reference_internal)

      .def("capture_first_primaries",
           &GGEMSTransportObserver::CaptureFirstPrimaries,
           py::arg("primary_count"),
           py::return_value_policy::reference_internal)

      .def("capture_primary", &GGEMSTransportObserver::CapturePrimary,
           py::arg("source_index"), py::arg("primary_index"),
           py::return_value_policy::reference_internal)

      .def("clear_capture_primary",
           &GGEMSTransportObserver::ClearCapturedPrimary,
           py::return_value_policy::reference_internal)

      .def_property_readonly("record_count",
                             &GGEMSTransportObserver::GetRecordCount)

      .def_property_readonly("overflow_count",
                             &GGEMSTransportObserver::GetOverflowCount)

      .def_property_readonly("captured_primary_count",
                             &GGEMSTransportObserver::GetCapturedPrimaryCount);
}
