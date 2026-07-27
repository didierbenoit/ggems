#include <cstdint>
#include <format>
#include <string>
#include <string_view>

#include <pybind11/pybind11.h>

#include "GGEMS/core/observer/GGEMSTransportObserver.hh"
#include "GGEMS/core/random/GGEMSRandom.hh"
#include "GGEMS/core/GGEMSRun.hh"
#include "GGEMSTimeBindingUtilities.hh"

namespace py = pybind11;

namespace {

[[noreturn]] auto
ThrowRunTimeConversionError(ggems::python::detail::TimeConversionError error,
                            std::string_view unit) -> void {
  using ggems::python::detail::TimeConversionError;

  switch (error) {
  case TimeConversionError::NonFinite:
    throw py::value_error("Run time must be finite.");
  case TimeConversionError::Negative:
    throw py::value_error("Run time must be positive or zero.");
  case TimeConversionError::UnsupportedUnit:
    throw py::value_error(std::format("Unsupported Run time unit '{}'.", unit));
  case TimeConversionError::OutOfRange:
    throw py::value_error("Run time is too large.");
  }

  throw py::value_error("Run time conversion failed.");
}

// =============================================================================
// =============================================================================

auto ConvertRunTimeToPicoSecond(double time, std::string_view unit)
    -> std::uint64_t {
  auto const conversion =
      ggems::python::detail::TryConvertTimeToPicoSecond(time, unit);

  if (conversion.has_value()) {
    return *conversion;
  }

  ThrowRunTimeConversionError(conversion.error(), unit);
}

// =============================================================================
// =============================================================================

auto ConvertRunTimeFromPicoSecond(std::uint64_t time_ps, std::string_view unit)
    -> double {
  auto const conversion =
      ggems::python::detail::TryConvertPicoSecondToTime(time_ps, unit);

  if (conversion.has_value()) {
    return *conversion;
  }

  ThrowRunTimeConversionError(conversion.error(), unit);
}

} // namespace

void BindRun(py::module_ &mod) {
  py::class_<ggems::core::GGEMSRun>(mod, "GGEMSRun")
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
            self.SetTimePicoSecond(ConvertRunTimeToPicoSecond(start, unit),
                                   ConvertRunTimeToPicoSecond(stop, unit),
                                   ConvertRunTimeToPicoSecond(step, unit));
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
            return ConvertRunTimeFromPicoSecond(self.GetCurrentTimePicoSecond(),
                                                unit);
          },
          py::arg("unit") = "s")

      .def(
          "get_current_time_window",
          [](ggems::core::GGEMSRun const &self,
             std::string const &unit) -> py::tuple {
            auto const window = self.GetCurrentTimeWindowPicoSecond();
            return py::make_tuple(
                ConvertRunTimeFromPicoSecond(window.start_ps, unit),
                ConvertRunTimeFromPicoSecond(window.stop_ps, unit));
          },
          py::arg("unit") = "s");
}
