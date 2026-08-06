#include <cstdint>
#include <filesystem>
#include <format>
#include <memory>
#include <string>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>

#include "detail/GGEMSPythonQuantityConversion.hh"

#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/sources/GGEMSSource.hh"
#include "GGEMS/core/sources/GGEMSSourceTypes.hh"
#include "GGEMS/core/units/GGEMSEnergyUnits.hh"
#include "GGEMS/core/units/GGEMSAngularUnits.hh"
#include "GGEMS/core/units/GGEMSLengthUnits.hh"

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

} // namespace

// =============================================================================
// =============================================================================

auto BindSource(py::module_ &mod) -> void {
  using ggems::core::sources::GGEMSSource;
  using ggems::python::detail::MakeQuantityOrThrow;

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
                MakeQuantityOrThrow<ggems::units::Length>(
                    width, unit,
                    {.quantity_name = "Source rectangle width",
                     .unsupported_unit_subject = "GGEMS length"})
                    .value,
                MakeQuantityOrThrow<ggems::units::Length>(
                    height, unit,
                    {.quantity_name = "Source rectangle height",
                     .unsupported_unit_subject = "GGEMS length"})
                    .value);
          },
          py::arg("width"), py::arg("height"), py::arg("unit") = "mm",
          py::return_value_policy::reference_internal)

      .def(
          "set_emission_ellipse",
          [](GGEMSSource &self, double diameter_x, double diameter_y,
             std::string const &unit) -> GGEMSSource & {
            return self.SetEllipseEmissionPicoMeter(
                MakeQuantityOrThrow<ggems::units::Length>(
                    diameter_x, unit,
                    {.quantity_name = "Source ellipse X diameter",
                     .unsupported_unit_subject = "GGEMS length"})
                    .value,
                MakeQuantityOrThrow<ggems::units::Length>(
                    diameter_y, unit,
                    {.quantity_name = "Source ellipse Y diameter",
                     .unsupported_unit_subject = "GGEMS length"})
                    .value);
          },
          py::arg("diameter_x"), py::arg("diameter_y"), py::arg("unit") = "mm",
          py::return_value_policy::reference_internal)

      .def(
          "set_emission_circle",
          [](GGEMSSource &self, double diameter,
             std::string const &unit) -> GGEMSSource & {
            return self.SetCircleEmissionPicoMeter(
                MakeQuantityOrThrow<ggems::units::Length>(
                    diameter, unit,
                    {.quantity_name = "Source circle diameter",
                     .unsupported_unit_subject = "GGEMS length"})
                    .value);
          },
          py::arg("diameter"), py::arg("unit") = "mm",
          py::return_value_policy::reference_internal)

      .def(
          "set_emission_box",
          [](GGEMSSource &self, double width, double height, double depth,
             std::string const &unit) -> GGEMSSource & {
            return self.SetBoxEmissionPicoMeter(
                MakeQuantityOrThrow<ggems::units::Length>(
                    width, unit,
                    {.quantity_name = "Source box width",
                     .unsupported_unit_subject = "GGEMS length"})
                    .value,
                MakeQuantityOrThrow<ggems::units::Length>(
                    height, unit,
                    {.quantity_name = "Source box height",
                     .unsupported_unit_subject = "GGEMS length"})
                    .value,
                MakeQuantityOrThrow<ggems::units::Length>(
                    depth, unit,
                    {.quantity_name = "Source box depth",
                     .unsupported_unit_subject = "GGEMS length"})
                    .value);
          },
          py::arg("width"), py::arg("height"), py::arg("depth"),
          py::arg("unit") = "mm", py::return_value_policy::reference_internal)

      .def(
          "set_emission_sphere",
          [](GGEMSSource &self, double diameter,
             std::string const &unit) -> GGEMSSource & {
            return self.SetSphereEmissionPicoMeter(
                MakeQuantityOrThrow<ggems::units::Length>(
                    diameter, unit,
                    {.quantity_name = "Source sphere diameter",
                     .unsupported_unit_subject = "GGEMS length"})
                    .value);
          },
          py::arg("diameter"), py::arg("unit") = "mm",
          py::return_value_policy::reference_internal)

      .def(
          "set_emission_cylinder",
          [](GGEMSSource &self, double diameter, double height,
             std::string const &unit) -> GGEMSSource & {
            return self.SetCylinderEmissionPicoMeter(
                MakeQuantityOrThrow<ggems::units::Length>(
                    diameter, unit,
                    {.quantity_name = "Source cylinder diameter",
                     .unsupported_unit_subject = "GGEMS length"})
                    .value,
                MakeQuantityOrThrow<ggems::units::Length>(
                    height, unit,
                    {.quantity_name = "Source cylinder height",
                     .unsupported_unit_subject = "GGEMS length"})
                    .value);
          },
          py::arg("diameter"), py::arg("height"), py::arg("unit") = "mm",
          py::return_value_policy::reference_internal)

      .def("set_angular_fixed", &GGEMSSource::SetFixedAngularDistribution,
           py::return_value_policy::reference_internal)

      .def(
          "set_angular_isotropic",
          [](GGEMSSource &self) -> GGEMSSource & {
            return self.SetIsotropicAngularDistribution();
          },
          py::return_value_policy::reference_internal)

      .def(
          "set_angular_isotropic",
          [](GGEMSSource &self, double theta_min, double theta_max,
             double phi_min, double phi_max,
             std::string const &unit) -> GGEMSSource & {
            return self.SetIsotropicAngularDistribution(
                MakeQuantityOrThrow<ggems::units::Angle>(
                    theta_min, unit,
                    {.quantity_name = "Source angle",
                     .unsupported_unit_subject = "GGEMS angle"}),
                MakeQuantityOrThrow<ggems::units::Angle>(
                    theta_max, unit,
                    {.quantity_name = "Source angle",
                     .unsupported_unit_subject = "GGEMS angle"}),
                MakeQuantityOrThrow<ggems::units::Angle>(
                    phi_min, unit,
                    {.quantity_name = "Source angle",
                     .unsupported_unit_subject = "GGEMS angle"}),
                MakeQuantityOrThrow<ggems::units::Angle>(
                    phi_max, unit,
                    {.quantity_name = "Source angle",
                     .unsupported_unit_subject = "GGEMS angle"}));
          },
          py::arg("theta_min"), py::arg("theta_max"), py::arg("phi_min"),
          py::arg("phi_max"), py::arg("unit") = "deg",
          py::return_value_policy::reference_internal)

      .def(
          "set_angular_focused",
          [](GGEMSSource &self, double focus_x, double focus_y, double focus_z,
             std::string const &unit) -> GGEMSSource & {
            return self.SetFocusedAngularDistributionPicoMeter(
                MakeQuantityOrThrow<ggems::units::PositionCoordinate>(
                    focus_x, unit,
                    {.quantity_name = "Source focus position",
                     .unsupported_unit_subject = "GGEMS length"})
                    .value,
                MakeQuantityOrThrow<ggems::units::PositionCoordinate>(
                    focus_y, unit,
                    {.quantity_name = "Source focus position",
                     .unsupported_unit_subject = "GGEMS length"})
                    .value,
                MakeQuantityOrThrow<ggems::units::PositionCoordinate>(
                    focus_z, unit,
                    {.quantity_name = "Source focus position",
                     .unsupported_unit_subject = "GGEMS length"})
                    .value);
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
                MakeQuantityOrThrow<ggems::units::Energy>(
                    energy, unit,
                    {.quantity_name = "Source energy",
                     .unsupported_unit_subject = "GGEMS energy"})
                    .value);
          },
          py::arg("energy"), py::arg("unit") = "keV",
          py::return_value_policy::reference_internal)

      .def(
          "set_discrete_energy_lines",
          [](GGEMSSource &self, std::vector<double> const &energies,
             std::vector<double> const &weights,
             std::string const &unit) -> GGEMSSource & {
            return self.SetDiscreteEnergyLines(energies, weights, unit);
          },
          py::arg("energies"), py::arg("weights"), py::arg("unit") = "keV",
          py::return_value_policy::reference_internal)

      .def(
          "set_regular_energy_spectrum",
          [](GGEMSSource &self, std::vector<double> const &bin_centers,
             std::vector<double> const &weights,
             std::string const &unit) -> GGEMSSource & {
            return self.SetRegularEnergySpectrum(bin_centers, weights, unit);
          },
          py::arg("bin_centers"), py::arg("weights"), py::arg("unit") = "keV",
          py::return_value_policy::reference_internal)

      .def(
          "load_regular_energy_spectrum",
          [](GGEMSSource &self, std::filesystem::path const &filename,
             std::string const &unit) -> GGEMSSource & {
            return self.LoadRegularEnergySpectrum(filename, unit);
          },
          py::arg("filename"), py::arg("unit") = "keV",
          py::return_value_policy::reference_internal)

      .def(
          "set_position",
          [](GGEMSSource &self, double const pos_x, double const pos_y,
             double const pos_z, std::string const &unit) -> GGEMSSource & {
            return self.SetPositionPicoMeter(
                MakeQuantityOrThrow<ggems::units::PositionCoordinate>(
                    pos_x, unit,
                    {.quantity_name = "Source position",
                     .unsupported_unit_subject = "GGEMS length"})
                    .value,
                MakeQuantityOrThrow<ggems::units::PositionCoordinate>(
                    pos_y, unit,
                    {.quantity_name = "Source position",
                     .unsupported_unit_subject = "GGEMS length"})
                    .value,
                MakeQuantityOrThrow<ggems::units::PositionCoordinate>(
                    pos_z, unit,
                    {.quantity_name = "Source position",
                     .unsupported_unit_subject = "GGEMS length"})
                    .value);
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
        auto const record = source.BuildRecord();
        auto const distribution_type = source.GetEnergyDistribution().GetType();

        auto const geometry_type =
            ggems::core::sources::FromKernelEmissionGeometryType(
                record.emission_geometry_type);
        auto const angular_type =
            ggems::core::sources::FromKernelAngularDistributionType(
                record.angular_distribution_type);

        return std::format(
            "<GGEMSSource type={} particle_type={} emission_geometry={} "
            "angular_distribution={} energy_distribution={}>",
            record.source_type, record.emitted_particle_type,
            ggems::core::sources::ToLongName(geometry_type),
            ggems::core::sources::ToLongName(angular_type),
            ggems::core::sources::ToLongName(distribution_type));
      });
}
