#include <cmath>
#include <cstdint>
#include <format>
#include <limits>
#include <string>

#include <pybind11/pybind11.h>

#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/sources/GGEMSSource.hh"

namespace py = pybind11;

namespace {

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

std::uint64_t ConvertEnergyToMilliElectronVolt(double energy,
                                               std::string const &unit) {
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

  if (energy_milli_eV >=
      static_cast<long double>(std::numeric_limits<std::uint64_t>::max())) {
    throw py::value_error("Source energy is too large.");
  }

  return static_cast<std::uint64_t>(energy_milli_eV + 0.5L);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

std::uint64_t ConvertTimeToPicosecond(double const time,
                                      std::string const &unit) {
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

  if (time_ps >=
      static_cast<long double>(std::numeric_limits<std::uint64_t>::max())) {
    throw py::value_error("Source time is too large.");
  }

  return static_cast<std::uint64_t>(time_ps + 0.5L);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

std::int64_t ConvertDistanceToPicometre(double const distance,
                                        std::string const &unit) {
  if (!std::isfinite(distance)) {
    throw py::value_error("Source position must contain finite values.");
  }

  long double factor_to_pm{0.0L};

  if (unit == "pm") {
    factor_to_pm = 1.0L;
  } else if (unit == "nm") {
    factor_to_pm = 1.0e3L;
  } else if (unit == "um") {
    factor_to_pm = 1.0e6L;
  } else if (unit == "mm") {
    factor_to_pm = 1.0e9L;
  } else if (unit == "cm") {
    factor_to_pm = 1.0e10L;
  } else if (unit == "m") {
    factor_to_pm = 1.0e12L;
  } else {
    throw py::value_error(
        std::format("Unsupported GGEMS distance unit '{}'.", unit));
  }

  long double const distance_pm =
      static_cast<long double>(distance) * factor_to_pm;

  if (distance_pm >
          static_cast<long double>(std::numeric_limits<std::int64_t>::max()) ||
      distance_pm <
          static_cast<long double>(std::numeric_limits<std::int64_t>::min())) {
    throw py::value_error("Source position is too large.");
  }

  if (distance_pm >= 0.0L) {
    return static_cast<std::int64_t>(distance_pm + 0.5L);
  }

  return static_cast<std::int64_t>(distance_pm - 0.5L);
}

} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void BindSource(py::module_ &m) {
  using ggems::core::sources::GGEMSSource;

  py::class_<GGEMSSource, std::shared_ptr<GGEMSSource>>(m, "GGEMSSource")
      .def(py::init<>())

      .def("set_analytic", &GGEMSSource::SetAnalytic,
           py::return_value_policy::reference_internal)

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
          [](GGEMSSource &self, double const x, double const y, double const z,
             std::string const &unit) -> GGEMSSource & {
            return self.SetPositionPicoMeter(
                ConvertDistanceToPicometre(x, unit),
                ConvertDistanceToPicometre(y, unit),
                ConvertDistanceToPicometre(z, unit));
          },
          py::arg("x"), py::arg("y"), py::arg("z"), py::arg("unit") = "mm",
          py::return_value_policy::reference_internal)

      .def("set_direction", &GGEMSSource::SetDirection, py::arg("x"),
           py::arg("y"), py::arg("z"),
           py::return_value_policy::reference_internal)

      .def("set_weight", &GGEMSSource::SetWeight, py::arg("weight"),
           py::return_value_policy::reference_internal)

      .def("verbose", &GGEMSSource::Verbose)

      .def("__repr__", [](GGEMSSource const &source) {
        auto const &record = source.GetRecord();

        return std::format(
            "<GGEMSSource type={} particle_type={} energy_milli_ev={}>",
            record.source_type, record.emitted_particle_type,
            record.energy_milli_eV);
      });
}
