// *****************************************************************************
// * This file is part of GGEMS.                                               *
// *                                                                           *
// * SPDX-License-Identifier: GPL-3.0-or-later                                 *
// * Copyright (C) 2017-2026 CHRU de Brest, Université de Bretagne Occidentale,*
// * Inserm.                                                                   *
// *                                                                           *
// * GGEMS is free software: you can redistribute it and/or modify             *
// * it under the terms of the GNU General Public License as published by      *
// * the Free Software Foundation, either version 3 of the License, or         *
// * (at your option) any later version.                                       *
// *                                                                           *
// * GGEMS is distributed in the hope that it will be useful,                  *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
// * GNU General Public License for more details.                              *
// *                                                                           *
// * You should have received a copy of the GNU General Public License         *
// * along with GGEMS. If not, see <https://www.gnu.org/licenses/>.            *
// *****************************************************************************

/*!
 * \file
 * \brief Defines Python bindings for GGEMS analytic source configuration.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <filesystem>
#include <format>
#include <memory>
#include <string>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>

#include "detail/GGEMSPythonQuantityConversion.hh"
#include "detail/GGEMSPythonRadionuclideDefinition.hh"

#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/sources/GGEMSSource.hh"
#include "GGEMS/sources/GGEMSSourceTypes.hh"
#include "GGEMS/sources/GGEMSSourcePopulation.hh"
#include "GGEMS/units/GGEMSEnergyUnits.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"
#include "GGEMS/units/GGEMSActivityUnits.hh"
#include "GGEMS/units/GGEMSTimeUnits.hh"
#include "GGEMS/radioactivity/GGEMSRadionuclideDefinition.hh"

namespace py = pybind11;

// =============================================================================
// =============================================================================

/*!
 * \brief Registers GGEMS source bindings in a Python module.
 *
 * \param[in,out] module Python module receiving the bindings.
 */
auto BindSource(py::module_ &module) -> void {
  using ggems::core::sources::GGEMSSource;
  using ggems::python::detail::GGEMSRadionuclideDefinitionHandle;
  using ggems::python::detail::MakeQuantityOrThrow;

  // === === ===
  py::class_<GGEMSSource, std::shared_ptr<GGEMSSource>>(
    module, "GGEMSSource",
    R"doc(Configure an analytic GGEMS particle source.

A new source is CountDriven with 4096 primaries. It emits 511 keV Gamma
particles from a point at the global origin along +Z.

Source-configuration methods return the same object, so calls can be chained.

CountDriven sources use set_primary_count(), set_particle(), and one of the
energy-distribution methods. ActivityDriven sources use set_radionuclide();
particle type, emission energy, and primary population are then provided by the
radionuclide definition and the configured GGEMSRun time window.

Example:
    source = (
        ggems.source.GGEMSSource()
        .set_primary_count(100000)
        .set_particle("Gamma")
        .set_energy(511.0, "keV")
        .set_emission_point()
        .set_position(0.0, 0.0, 0.0, "mm")
        .set_angular_fixed()
    )
)doc")
    .def(py::init<>(),
         R"doc(Create a source with the default CountDriven configuration.

The default source uses 4096 primaries, a 511 keV Gamma, point emission at the
global origin, and a fixed +Z direction.
)doc")

    .def("set_analytic", &GGEMSSource::SetAnalytic,
         R"doc(Select the analytic GGEMS source representation.

Returns:
    GGEMSSource: This source, allowing chained configuration.
)doc",
         py::return_value_policy::reference_internal)

    .def("set_emission_point", &GGEMSSource::SetPointEmission,
         R"doc(Emit all primaries from the configured source position.

Returns:
    GGEMSSource: This source, allowing chained configuration.
)doc",
         py::return_value_policy::reference_internal)

    .def(
      "set_emission_rectangle",
      [](GGEMSSource &self, double width, double height,
         std::string const &unit) -> GGEMSSource & {
        return self.SetRectangleEmissionPicoMeter(
          MakeQuantityOrThrow<ggems::units::Length>(width, unit).value,
          MakeQuantityOrThrow<ggems::units::Length>(height, unit).value);
      },
      R"doc(Sample emission positions uniformly over a rectangle.

The rectangle lies in the source local XY plane and is centered on the
configured source position.

Args:
    width: Rectangle width along the local X axis.
    height: Rectangle height along the local Y axis.
    unit: GGEMS length unit.

Returns:
    GGEMSSource: This source, allowing chained configuration.

Example:
    source.set_emission_rectangle(40.0, 20.0, "mm")
)doc",
      py::arg("width"), py::arg("height"), py::arg("unit") = "mm",
      py::return_value_policy::reference_internal)

    .def(
      "set_emission_ellipse",
      [](GGEMSSource &self, double diameter_x, double diameter_y,
         std::string const &unit) -> GGEMSSource & {
        return self.SetEllipseEmissionPicoMeter(
          MakeQuantityOrThrow<ggems::units::Length>(diameter_x, unit).value,
          MakeQuantityOrThrow<ggems::units::Length>(diameter_y, unit).value);
      },
      R"doc(Sample emission positions uniformly over an ellipse.

The ellipse lies in the source local XY plane and is centered on the configured
source position.

Args:
    diameter_x: Full ellipse diameter along the local X axis.
    diameter_y: Full ellipse diameter along the local Y axis.
    unit: GGEMS length unit.

Returns:
    GGEMSSource: This source, allowing chained configuration.

Example:
    source.set_emission_ellipse(40.0, 20.0, "mm")
)doc",
      py::arg("diameter_x"), py::arg("diameter_y"), py::arg("unit") = "mm",
      py::return_value_policy::reference_internal)

    .def(
      "set_emission_circle",
      [](GGEMSSource &self, double diameter,
         std::string const &unit) -> GGEMSSource & {
        return self.SetCircleEmissionPicoMeter(
          MakeQuantityOrThrow<ggems::units::Length>(diameter, unit).value);
      },
      R"doc(Sample emission positions uniformly over a circle.

The circle lies in the source local XY plane and is centered on the configured
source position.

Args:
    diameter: Circle diameter.
    unit: GGEMS length unit.

Returns:
    GGEMSSource: This source, allowing chained configuration.

Example:
    source.set_emission_circle(10.0, "mm")
)doc",
      py::arg("diameter"), py::arg("unit") = "mm",
      py::return_value_policy::reference_internal)

    .def(
      "set_emission_box",
      [](GGEMSSource &self, double width, double height, double depth,
         std::string const &unit) -> GGEMSSource & {
        return self.SetBoxEmissionPicoMeter(
          MakeQuantityOrThrow<ggems::units::Length>(width, unit).value,
          MakeQuantityOrThrow<ggems::units::Length>(height, unit).value,
          MakeQuantityOrThrow<ggems::units::Length>(depth, unit).value);
      },
      R"doc(Sample emission positions uniformly inside a box.

The box is centered on the configured source position and follows the source
local XYZ frame.

Args:
    width: Box size along the local X axis.
    height: Box size along the local Y axis.
    depth: Box size along the local Z axis.
    unit: GGEMS length unit.

Returns:
    GGEMSSource: This source, allowing chained configuration.

Example:
    source.set_emission_box(20.0, 30.0, 40.0, "mm")
)doc",
      py::arg("width"), py::arg("height"), py::arg("depth"),
      py::arg("unit") = "mm", py::return_value_policy::reference_internal)

    .def(
      "set_emission_sphere",
      [](GGEMSSource &self, double diameter,
         std::string const &unit) -> GGEMSSource & {
        return self.SetSphereEmissionPicoMeter(
          MakeQuantityOrThrow<ggems::units::Length>(diameter, unit).value);
      },
      R"doc(Sample emission positions uniformly inside a sphere.

The sphere is centered on the configured source position.

Args:
    diameter: Sphere diameter.
    unit: GGEMS length unit.

Returns:
    GGEMSSource: This source, allowing chained configuration.

Example:
    source.set_emission_sphere(25.0, "mm")
)doc",
      py::arg("diameter"), py::arg("unit") = "mm",
      py::return_value_policy::reference_internal)

    .def(
      "set_emission_cylinder",
      [](GGEMSSource &self, double diameter, double height,
         std::string const &unit) -> GGEMSSource & {
        return self.SetCylinderEmissionPicoMeter(
          MakeQuantityOrThrow<ggems::units::Length>(diameter, unit).value,
          MakeQuantityOrThrow<ggems::units::Length>(height, unit).value);
      },
      R"doc(Sample emission positions uniformly inside a cylinder.

The cylinder axis follows the source local Z axis and the cylinder is centered
on the configured source position.

Args:
    diameter: Cylinder diameter.
    height: Cylinder height along the local Z axis.
    unit: GGEMS length unit.

Returns:
    GGEMSSource: This source, allowing chained configuration.

Example:
    source.set_emission_cylinder(20.0, 50.0, "mm")
)doc",
      py::arg("diameter"), py::arg("height"), py::arg("unit") = "mm",
      py::return_value_policy::reference_internal)

    .def("set_angular_fixed", &GGEMSSource::SetFixedAngularDistribution,
         R"doc(Emit every primary along the source local +Z axis.

The default source frame makes local +Z equal to global +Z. set_direction() or
set_orientation() can rotate the source frame.

Returns:
    GGEMSSource: This source, allowing chained configuration.
)doc",
         py::return_value_policy::reference_internal)

    .def(
      "set_angular_isotropic",
      [](GGEMSSource &self) -> GGEMSSource & {
        return self.SetIsotropicAngularDistribution();
      },
      R"doc(Sample directions isotropically over the full sphere.

Returns:
    GGEMSSource: This source, allowing chained configuration.
)doc",
      py::return_value_policy::reference_internal)

    .def(
      "set_angular_isotropic",
      [](GGEMSSource &self, double theta_min, double theta_max, double phi_min,
         double phi_max, std::string const &unit) -> GGEMSSource & {
        return self.SetIsotropicAngularDistribution(
          MakeQuantityOrThrow<ggems::units::Angle>(theta_min, unit),
          MakeQuantityOrThrow<ggems::units::Angle>(theta_max, unit),
          MakeQuantityOrThrow<ggems::units::Angle>(phi_min, unit),
          MakeQuantityOrThrow<ggems::units::Angle>(phi_max, unit));
      },
      R"doc(Sample directions uniformly in solid angle inside angular bounds.

Theta is the polar angle from the source local +Z axis and must satisfy
0 <= theta_min < theta_max <= pi. Phi is the azimuth around local +Z and its
interval width must be in (0, 2*pi].

Args:
    theta_min: Minimum polar angle.
    theta_max: Maximum polar angle.
    phi_min: Minimum azimuth.
    phi_max: Maximum azimuth.
    unit: GGEMS angular unit.

Returns:
    GGEMSSource: This source, allowing chained configuration.

Example:
    source.set_angular_isotropic(0.0, 30.0, 0.0, 360.0, "deg")
)doc",
      py::arg("theta_min"), py::arg("theta_max"), py::arg("phi_min"),
      py::arg("phi_max"), py::arg("unit") = "deg",
      py::return_value_policy::reference_internal)

    .def(
      "set_angular_focused",
      [](GGEMSSource &self, double focus_x, double focus_y, double focus_z,
         std::string const &unit) -> GGEMSSource & {
        return self.SetFocusedAngularDistributionPicoMeter(
          MakeQuantityOrThrow<ggems::units::PositionCoordinate>(focus_x, unit)
            .value,
          MakeQuantityOrThrow<ggems::units::PositionCoordinate>(focus_y, unit)
            .value,
          MakeQuantityOrThrow<ggems::units::PositionCoordinate>(focus_z, unit)
            .value);
      },
      R"doc(Direct each primary toward a fixed point in global coordinates.

The direction is computed from each sampled emission position to the focus
point.

Args:
    focus_x: Focus X coordinate.
    focus_y: Focus Y coordinate.
    focus_z: Focus Z coordinate.
    unit: GGEMS length unit.

Returns:
    GGEMSSource: This source, allowing chained configuration.

Example:
    source.set_angular_focused(0.0, 0.0, 1000.0, "mm")
)doc",
      py::arg("focus_x"), py::arg("focus_y"), py::arg("focus_z"),
      py::arg("unit") = "mm", py::return_value_policy::reference_internal)

    .def("set_primary_count", &GGEMSSource::SetPrimaryCount,
         R"doc(Set the number of primaries emitted by a CountDriven source.

Args:
    primary_count: Number of primary histories assigned to this source.

Returns:
    GGEMSSource: This source, allowing chained configuration.

Example:
    source.set_primary_count(1000000)
)doc",
         py::arg("primary_count"), py::return_value_policy::reference_internal)

    .def(
      "set_radionuclide",
      [](GGEMSSource &self,
         GGEMSRadionuclideDefinitionHandle const &radionuclide, double activity,
         std::string const &activity_unit, double reference_time,
         std::string const &time_unit) -> GGEMSSource & {
        auto const converted_activity =
          MakeQuantityOrThrow<ggems::units::Activity>(activity, activity_unit);

        auto const converted_reference_time =
          MakeQuantityOrThrow<ggems::units::TimePoint>(reference_time,
                                                       time_unit);

        return self.SetRadionuclide(radionuclide.GetDefinition(),
                                    converted_activity,
                                    converted_reference_time.value);
      },
      R"doc(Configure an ActivityDriven source from a radionuclide definition.

The radionuclide supplies the emitted particle and energy distributions.
Activity is specified at reference_time. ActivityDriven sources require a
non-empty GGEMSRun time schedule, and reference_time must not be later than the
configured run start time.

Args:
    radionuclide: Definition returned by ggems.radionuclide.create().
    activity: Activity at the reference time.
    activity_unit: GGEMS activity unit.
    reference_time: Time at which activity is specified.
    time_unit: GGEMS time unit.

Returns:
    GGEMSSource: This source, allowing chained configuration.

Example:
    f18 = ggems.radionuclide.create("F-18")
    source.set_radionuclide(f18, 5.0, "MBq", 0.0, "s")
)doc",
      py::arg("radionuclide"), py::arg("activity"),
      py::arg("activity_unit") = "Bq", py::arg("reference_time") = 0.0,
      py::arg("time_unit") = "s", py::return_value_policy::reference_internal)

    .def(
      "set_particle",
      [](GGEMSSource &self, std::string const &particle_name) -> GGEMSSource & {
        return self.SetEmittedParticleType(
          ggems::core::particles::ParseParticleType(particle_name));
      },
      R"doc(Set the emitted particle type of a CountDriven source.

Names are case-insensitive. Supported particle names include Aionino, Gamma,
Electron, Positron, Proton, Neutron, and Alpha, together with their accepted
short aliases.

Args:
    particle: Particle name or supported alias.

Returns:
    GGEMSSource: This source, allowing chained configuration.

Example:
    source.set_particle("Gamma")
)doc",
      py::arg("particle"), py::return_value_policy::reference_internal)

    .def(
      "set_energy",
      [](GGEMSSource &self, double energy,
         std::string const &unit) -> GGEMSSource & {
        return self.SetEnergyMicroElectronVolt(
          MakeQuantityOrThrow<ggems::units::Energy>(energy, unit).value);
      },
      R"doc(Set a monoenergetic distribution for a CountDriven source.

Energy must remain strictly positive after conversion to GGEMS canonical energy
units.

Args:
    energy: Monoenergetic source energy.
    unit: GGEMS energy unit.

Returns:
    GGEMSSource: This source, allowing chained configuration.

Example:
    source.set_energy(511.0, "keV")
)doc",
      py::arg("energy"), py::arg("unit") = "keV",
      py::return_value_policy::reference_internal)

    .def(
      "set_discrete_energy_lines",
      [](GGEMSSource &self, std::vector<double> const &energies,
         std::vector<double> const &weights,
         std::string const &unit) -> GGEMSSource & {
        return self.SetDiscreteEnergyLines(energies, weights, unit);
      },
      R"doc(Set a discrete-line energy distribution for a CountDriven source.

At least two strictly increasing energies are required. energies and weights
must have the same length. Weights are relative, must be finite and
non-negative, and at least one weight must be strictly positive.

Args:
    energies: Discrete line energies in strictly increasing order.
    weights: Relative line weights.
    unit: GGEMS energy unit.

Returns:
    GGEMSSource: This source, allowing chained configuration.

Example:
    source.set_discrete_energy_lines(
        [80.0, 120.0, 140.0],
        [1.0, 2.0, 1.0],
        "keV",
    )
)doc",
      py::arg("energies"), py::arg("weights"), py::arg("unit") = "keV",
      py::return_value_policy::reference_internal)

    .def(
      "set_regular_energy_spectrum",
      [](GGEMSSource &self, std::vector<double> const &bin_centers,
         std::vector<double> const &weights,
         std::string const &unit) -> GGEMSSource & {
        return self.SetRegularEnergySpectrum(bin_centers, weights, unit);
      },
      R"doc(Set a regular binned energy spectrum for a CountDriven source.

At least two strictly increasing bin centers are required. After conversion to
GGEMS canonical energy units, the centers must form an exactly regular grid.
bin_centers and weights must have the same length. Weights are relative,
finite, and non-negative, with at least one strictly positive entry.

The selected bin follows the relative weights and the emitted energy is sampled
across that bin.

Args:
    bin_centers: Regularly spaced energy-bin centers.
    weights: Relative bin weights.
    unit: GGEMS energy unit.

Returns:
    GGEMSSource: This source, allowing chained configuration.

Example:
    source.set_regular_energy_spectrum(
        [20.0, 30.0, 40.0, 50.0],
        [1.0, 2.0, 2.0, 1.0],
        "keV",
    )
)doc",
      py::arg("bin_centers"), py::arg("weights"), py::arg("unit") = "keV",
      py::return_value_policy::reference_internal)

    .def(
      "load_regular_energy_spectrum",
      [](GGEMSSource &self, std::filesystem::path const &filename,
         std::string const &unit) -> GGEMSSource & {
        return self.LoadRegularEnergySpectrum(filename, unit);
      },
      R"doc(Load a regular binned energy spectrum from a text file.

Each data line must contain exactly two numeric fields: energy-bin center and
relative bin weight. Empty lines are ignored and text after '#' is treated as a
comment. The loaded centers must satisfy the same regular-grid requirements as
set_regular_energy_spectrum().

Args:
    filename: Spectrum text-file path.
    unit: GGEMS energy unit applied to the energy centers.

Returns:
    GGEMSSource: This source, allowing chained configuration.

Example:
    source.load_regular_energy_spectrum("spectrum.dat", "keV")
)doc",
      py::arg("filename"), py::arg("unit") = "keV",
      py::return_value_policy::reference_internal)

    .def(
      "set_position",
      [](GGEMSSource &self, double pos_x, double pos_y, double pos_z,
         std::string const &unit) -> GGEMSSource & {
        return self.SetPositionPicoMeter(
          MakeQuantityOrThrow<ggems::units::PositionCoordinate>(pos_x, unit)
            .value,
          MakeQuantityOrThrow<ggems::units::PositionCoordinate>(pos_y, unit)
            .value,
          MakeQuantityOrThrow<ggems::units::PositionCoordinate>(pos_z, unit)
            .value);
      },
      R"doc(Set the source position in global coordinates.

The position is the emission point for point sources and the center of the
configured surface or volume emission geometry.

Args:
    x: Global X coordinate.
    y: Global Y coordinate.
    z: Global Z coordinate.
    unit: GGEMS length unit.

Returns:
    GGEMSSource: This source, allowing chained configuration.

Example:
    source.set_position(0.0, 0.0, -500.0, "mm")
)doc",
      py::arg("x"), py::arg("y"), py::arg("z"), py::arg("unit") = "mm",
      py::return_value_policy::reference_internal)

    .def(
      "set_direction", &GGEMSSource::SetDirection,
      R"doc(Set the source local +Z direction and build the remaining frame automatically.

The input vector is normalized. It must contain finite values and have a
strictly positive norm.

Args:
    x: Direction X component.
    y: Direction Y component.
    z: Direction Z component.

Returns:
    GGEMSSource: This source, allowing chained configuration.

Example:
    source.set_direction(0.0, 0.0, 1.0)
)doc",
      py::arg("x"), py::arg("y"), py::arg("z"),
      py::return_value_policy::reference_internal)

    .def(
      "set_orientation", &GGEMSSource::SetOrientation,
      R"doc(Set the complete source orientation from direction and up-reference vectors.

direction defines the source local +Z axis. up is used as a reference to build
a right-handed orthonormal local frame. Both vectors must be finite and non-zero
and must not be parallel or nearly parallel.

Args:
    direction: Three-component source direction vector.
    up: Three-component up-reference vector.

Returns:
    GGEMSSource: This source, allowing chained configuration.

Example:
    source.set_orientation([0.0, 0.0, 1.0], [0.0, 1.0, 0.0])
)doc",
      py::arg("direction"), py::arg("up"),
      py::return_value_policy::reference_internal)

    .def(
      "verbose", &GGEMSSource::Verbose,
      R"doc(Print the current source configuration through the GGEMS logger.)doc")

    .def(
      "__repr__",
      [](GGEMSSource const &source) -> std::string {
        if (source.GetPopulationMode() ==
            ggems::core::sources::GGEMSSourcePopulationMode::ActivityDriven) {
          auto const record = source.BuildExecutionRecord();

          auto const configuration =
            source.BuildActivityDrivenPopulationConfiguration();

          auto const &radionuclide = *configuration.radionuclide;

          auto const geometry_type =
            ggems::core::sources::FromKernelEmissionGeometryType(
              record.emission_geometry_type);

          auto const angular_type =
            ggems::core::sources::FromKernelAngularDistributionType(
              record.angular_distribution_type);

          return std::format(
            "<GGEMSSource type={} population=ActivityDriven "
            "radionuclide={} activity_Bq={} reference_time_ps={} "
            "emission_count={} emission_geometry={} "
            "angular_distribution={}>",
            record.source_type, radionuclide.GetCanonicalName(),
            configuration.activity_at_reference_time.value,
            configuration.reference_time_ps, radionuclide.GetEmissions().size(),
            ggems::core::sources::ToLongName(geometry_type),
            ggems::core::sources::ToLongName(angular_type));
        }

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
      },
      R"doc(Return a concise representation of the current source configuration.)doc");
}
