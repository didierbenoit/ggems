#include <string>

#include <pybind11/pybind11.h>

#include "detail/GGEMSPythonQuantityConversion.hh"

#include "GGEMS/core/random/GGEMSRandom.hh"
#include "GGEMS/core/observer/GGEMSTransportObserver.hh"
#include "GGEMS/core/GGEMSRun.hh"
#include "GGEMS/core/units/GGEMSTimeUnits.hh"

namespace py = pybind11;

namespace {

constexpr auto k_run_time_conversion_context =
    ggems::python::detail::QuantityConversionContext{
        .quantity_name = "Run time", .unsupported_unit_subject = "Run time"};

} // namespace

// =============================================================================
// =============================================================================

void BindRun(py::module_ &module) {
  py::class_<ggems::core::GGEMSRun>(module, "GGEMSRun")
      .def(py::init<>())

      .def("run", &ggems::core::GGEMSRun::Run,
           py::call_guard<py::gil_scoped_release>())

      .def("initialise", &ggems::core::GGEMSRun::Initialise)

      .def("set_random", &ggems::core::GGEMSRun::SetRandom, py::arg("random"))

      .def("set_source", &ggems::core::GGEMSRun::SetSource, py::arg("source"))

      .def("add_source", &ggems::core::GGEMSRun::AddSource, py::arg("source"))

      .def("set_observer", &ggems::core::GGEMSRun::SetObserver,
           py::arg("observer"))

      .def("set_primary_count", &ggems::core::GGEMSRun::SetPrimaryCount,
           py::arg("primary_count"))

      .def("set_worker_count", &ggems::core::GGEMSRun::SetWorkerCount,
           py::arg("worker_count"))

      .def(
          "set_time",
          [](ggems::core::GGEMSRun &self, double start, double stop,
             double step, std::string const &unit) -> void {
            self.SetTimePicoSecond(
                ggems::python::detail::MakeQuantityOrThrow<
                    ggems::units::TimePoint>(start, unit,
                                             k_run_time_conversion_context)
                    .value,
                ggems::python::detail::MakeQuantityOrThrow<
                    ggems::units::TimePoint>(stop, unit,
                                             k_run_time_conversion_context)
                    .value,
                ggems::python::detail::MakeQuantityOrThrow<
                    ggems::units::Duration>(step, unit,
                                            k_run_time_conversion_context)
                    .value);
          },
          py::arg("start"), py::arg("stop"), py::arg("step"),
          py::arg("unit") = "s")

      .def("reset_time", &ggems::core::GGEMSRun::ResetTime)

      .def("has_time_configuration",
           &ggems::core::GGEMSRun::HasTimeConfiguration)

      .def("has_next_time_step", &ggems::core::GGEMSRun::HasNextTimeStep)

      .def(
          "get_current_time",
          [](ggems::core::GGEMSRun const &self,
             std::string const &unit) -> double {
            return ggems::python::detail::ConvertQuantityToDoubleOrThrow(
                ggems::units::TimePoint{.value =
                                            self.GetCurrentTimePicoSecond()},
                unit, k_run_time_conversion_context);
          },
          py::arg("unit") = "s")

      .def(
          "get_current_time_window",
          [](ggems::core::GGEMSRun const &self,
             std::string const &unit) -> py::tuple {
            auto const window = self.GetCurrentTimeWindowPicoSecond();
            return py::make_tuple(
                ggems::python::detail::ConvertQuantityToDoubleOrThrow(
                    ggems::units::TimePoint{.value = window.start_ps}, unit,
                    k_run_time_conversion_context),
                ggems::python::detail::ConvertQuantityToDoubleOrThrow(
                    ggems::units::TimePoint{.value = window.stop_ps}, unit,
                    k_run_time_conversion_context));
          },
          py::arg("unit") = "s");
}
