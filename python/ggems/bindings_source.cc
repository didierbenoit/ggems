#include <cmath>
#include <cstdint>
#include <format>
#include <limits>
#include <string>
#include <memory>
#include <string_view>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/sources/GGEMSSource.hh"
#include "GGEMSSourceBindingUtilities.hh"

namespace py = pybind11;

namespace {

// =============================================================================
// =============================================================================

auto ConvertPrimaryCount(py::handle primary_count) -> std::uint64_t {
  if (!PyLong_Check(primary_count.ptr())) {
    throw py::type_error("Source primary count must be a Python integer.");
  }

  unsigned long long converted = PyLong_AsUnsignedLongLong(primary_count.ptr());

  if (PyErr_Occurred() != nullptr) {
    PyErr_Clear();
    throw py::value_error(
        "Source primary count must be in the range [0, UINT64_MAX].");
  }

  return static_cast<std::uint64_t>(converted);
}

// =============================================================================
// =============================================================================

auto ConvertEnergyToMilliElectronVolt(double energy, std::string const &unit)
    -> std::uint64_t {
  if (!std::isfinite(energy) || energy <= 0.0) {
    throw py::value_error("Source energy must be finite and positive.");
  }

  long double factor_to_milli_eV{0.0L};

  if (unit == "milli_eV" || unit == "meV") {
    factor_to_milli_eV = 1.0L;
  } else if (unit == "eV") {
    factor_to_milli_eV = 1.0e3L;
  } else if (unit == "keV") {
    factor_to_milli_eV = 1.0e6L;
  } else if (unit == "MeV") {
    factor_to_milli_eV = 1.0e9L;
  } else if (unit == "GeV") {
    factor_to_milli_eV = 1.0e12L;
  } else {
    throw py::value_error(
        std::format("Unsupported GGEMS energy unit '{}'.", unit));
  }

  long double energy_milli_eV{static_cast<long double>(energy) *
                              factor_to_milli_eV};

  long double const rounded_energy_milli_eV{std::round(energy_milli_eV)};

  if (rounded_energy_milli_eV >=
      static_cast<long double>(std::numeric_limits<std::uint64_t>::max())) {
    throw py::value_error("Source energy is too large.");
  }

  return static_cast<std::uint64_t>(rounded_energy_milli_eV);
}

// =============================================================================
// =============================================================================

auto ConvertTimeToPicosecond(double const time, std::string const &unit)
    -> std::uint64_t {
  if (!std::isfinite(time) || time < 0.0) {
    throw py::value_error("Source time must be finite and positive or zero.");
  }

  long double factor_to_ps{0.0L};

  if (unit == "ps") {
    factor_to_ps = 1.0L;
  } else if (unit == "ns") {
    factor_to_ps = 1.0e3L;
  } else if (unit == "us") {
    factor_to_ps = 1.0e6L;
  } else if (unit == "ms") {
    factor_to_ps = 1.0e9L;
  } else if (unit == "s") {
    factor_to_ps = 1.0e12L;
  } else if (unit == "min") {
    factor_to_ps = 60.0e12L;
  } else if (unit == "h") {
    factor_to_ps = 3600.0e12L;
  } else {
    throw py::value_error(
        std::format("Unsupported GGEMS time unit '{}'.", unit));
  }

  long double const time_ps = static_cast<long double>(time) * factor_to_ps;

  long double const rounded_time_ps{std::round(time_ps)};

  constexpr int uint64_digits{std::numeric_limits<std::uint64_t>::digits};

  long double const uint64_upper_bound{std::ldexp(1.0L, uint64_digits)};

  if (rounded_time_ps >= uint64_upper_bound) {
    throw py::value_error("Source time is too large.");
  }

  return static_cast<std::uint64_t>(rounded_time_ps);
}

// =============================================================================
// =============================================================================

auto ConvertDistanceToPicometre(double distance, std::string const &unit,
                                std::string_view quantity) -> std::int64_t {
  auto const conversion =
      ggems::python::detail::TryConvertDistanceToPicometre(distance, unit);

  if (conversion.has_value()) {
    return *conversion;
  }

  using ggems::python::detail::DistanceToPicometreError;

  if (conversion.error() == DistanceToPicometreError::NonFinite) {
    throw py::value_error(std::format("{} must be finite.", quantity));
  }

  if (conversion.error() == DistanceToPicometreError::UnsupportedUnit) {
    throw py::value_error(
        std::format("Unsupported GGEMS distance unit '{}'.", unit));
  }

  throw py::value_error(std::format("{} is too large.", quantity));
}

// =============================================================================
// =============================================================================

auto ConvertPositiveDistanceToPicometre(double distance,
                                        std::string const &unit,
                                        std::string_view quantity)
    -> std::uint64_t {
  auto const conversion =
      ggems::python::detail::TryConvertPositiveDistanceToPicometre(distance,
                                                                   unit);

  if (conversion.has_value()) {
    return *conversion;
  }

  using ggems::python::detail::DistanceToPicometreError;

  if (conversion.error() == DistanceToPicometreError::NonFinite) {
    throw py::value_error(std::format("{} must be finite.", quantity));
  }

  if (conversion.error() == DistanceToPicometreError::NonPositive) {
    throw py::value_error(
        std::format("{} must be strictly positive.", quantity));
  }

  if (conversion.error() == DistanceToPicometreError::UnsupportedUnit) {
    throw py::value_error(
        std::format("Unsupported GGEMS distance unit '{}'.", unit));
  }

  throw py::value_error(std::format("{} is too large.", quantity));
}
} // namespace

// =============================================================================
// =============================================================================

auto BindSource(py::module_ &mod) -> void {
  using ggems::core::sources::GGEMSSource;

  py::class_<GGEMSSource, std::shared_ptr<GGEMSSource>>(mod, "GGEMSSource")
      .def(py::init<>())

      .def("set_analytic", &GGEMSSource::SetAnalytic,
           py::return_value_policy::reference_internal)

      .def("set_emission_point", &GGEMSSource::SetPointEmission,
           py::return_value_policy::reference_internal)

      .def(
          "set_emission_rectangle",
          [](GGEMSSource &self, double width, double height,
             std::string const &unit) -> GGEMSSource & {
            return self.SetRectangleEmissionPicoMeter(
                ConvertPositiveDistanceToPicometre(width, unit,
                                                   "Source rectangle width"),
                ConvertPositiveDistanceToPicometre(height, unit,
                                                   "Source rectangle height"));
          },
          py::arg("width"), py::arg("height"), py::arg("unit") = "mm",
          py::return_value_policy::reference_internal)

      .def(
          "set_emission_ellipse",
          [](GGEMSSource &self, double diameter_x, double diameter_y,
             std::string const &unit) -> GGEMSSource & {
            return self.SetEllipseEmissionPicoMeter(
                ConvertPositiveDistanceToPicometre(diameter_x, unit,
                                                   "Source ellipse X diameter"),
                ConvertPositiveDistanceToPicometre(
                    diameter_y, unit, "Source ellipse Y diameter"));
          },
          py::arg("diameter_x"), py::arg("diameter_y"), py::arg("unit") = "mm",
          py::return_value_policy::reference_internal)

      .def(
          "set_emission_circle",
          [](GGEMSSource &self, double diameter,
             std::string const &unit) -> GGEMSSource & {
            return self.SetCircleEmissionPicoMeter(
                ConvertPositiveDistanceToPicometre(diameter, unit,
                                                   "Source circle diameter"));
          },
          py::arg("diameter"), py::arg("unit") = "mm",
          py::return_value_policy::reference_internal)

      .def("set_angular_fixed", &GGEMSSource::SetFixedAngularDistribution,
           py::return_value_policy::reference_internal)

      .def("set_angular_isotropic",
           &GGEMSSource::SetIsotropicAngularDistribution,
           py::return_value_policy::reference_internal)

      .def(
          "set_angular_focused",
          [](GGEMSSource &self, double focus_x, double focus_y, double focus_z,
             std::string const &unit) -> GGEMSSource & {
            return self.SetFocusedAngularDistributionPicoMeter(
                ConvertDistanceToPicometre(focus_x, unit,
                                           "Source focus position"),
                ConvertDistanceToPicometre(focus_y, unit,
                                           "Source focus position"),
                ConvertDistanceToPicometre(focus_z, unit,
                                           "Source focus position"));
          },
          py::arg("focus_x"), py::arg("focus_y"), py::arg("focus_z"),
          py::arg("unit") = "mm", py::return_value_policy::reference_internal)

      .def(
          "set_primary_count",
          [](GGEMSSource &self, py::handle primary_count) -> GGEMSSource & {
            return self.SetPrimaryCount(ConvertPrimaryCount(primary_count));
          },
          py::arg("primary_count"), py::return_value_policy::reference_internal)

      .def(
          "set_particle",
          [](GGEMSSource &self,
             std::string const &particle_name) -> GGEMSSource & {
            return self.SetEmittedParticleType(
                ggems::core::particles::ParseParticleType(particle_name));
          },
          py::arg("particle"), py::return_value_policy::reference_internal)

      .def(
          "set_energy",
          [](GGEMSSource &self, double const energy,
             std::string const &unit) -> GGEMSSource & {
            return self.SetEnergyMilliElectronVolt(
                ConvertEnergyToMilliElectronVolt(energy, unit));
          },
          py::arg("energy"), py::arg("unit") = "keV",
          py::return_value_policy::reference_internal)

      .def(
          "set_time_window",
          [](GGEMSSource &self, double const time_start, double const time_stop,
             std::string const &unit) -> GGEMSSource & {
            return self.SetTimeWindowPicoSecond(
                ConvertTimeToPicosecond(time_start, unit),
                ConvertTimeToPicosecond(time_stop, unit));
          },
          py::arg("time_start"), py::arg("time_stop"), py::arg("unit") = "s",
          py::return_value_policy::reference_internal)

      .def(
          "set_position",
          [](GGEMSSource &self, double const pos_x, double const pos_y,
             double const pos_z, std::string const &unit) -> GGEMSSource & {
            return self.SetPositionPicoMeter(
                ConvertDistanceToPicometre(pos_x, unit, "Source position"),
                ConvertDistanceToPicometre(pos_y, unit, "Source position"),
                ConvertDistanceToPicometre(pos_z, unit, "Source position"));
          },
          py::arg("x"), py::arg("y"), py::arg("z"), py::arg("unit") = "mm",
          py::return_value_policy::reference_internal)

      .def("set_direction", &GGEMSSource::SetDirection, py::arg("x"),
           py::arg("y"), py::arg("z"),
           py::return_value_policy::reference_internal)

      .def("set_orientation", &GGEMSSource::SetOrientation,
           py::arg("direction"), py::arg("up"),
           py::return_value_policy::reference_internal)

      .def("set_weight", &GGEMSSource::SetWeight, py::arg("weight"),
           py::return_value_policy::reference_internal)

      .def("verbose", &GGEMSSource::Verbose)

      .def("__repr__", [](GGEMSSource const &source) -> std::string {
        auto const &record = source.GetRecord();

        return std::format(
            "<GGEMSSource type={} particle_type={} energy_milli_ev={}>",
            record.source_type, record.emitted_particle_type,
            record.energy_milli_eV);
      });
}
