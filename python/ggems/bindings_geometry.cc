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
 * \brief Defines Python bindings for GGEMS World and analytic volume authoring.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <string>

#include <pybind11/pybind11.h>

#include "detail/GGEMSPythonQuantityConversion.hh"

#include "GGEMS/geometry/GGEMSBox.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSWorld.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/materials/GGEMSMaterialManager.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace py = pybind11;

namespace {

using ggems::python::detail::MakeQuantityOrThrow;

/*!
 * \brief Resolves a Material by exact name through the Materials authority.
 *
 * Lookup order is registered, built-in, then custom definitions; the Material
 * is registered as referenced by the configuration.
 *
 * \param[in] name Exact, case-sensitive Material name.
 * \return An owned copy of the resolved Material.
 * \throws ggems::core::GGEMSRecoverable If the name is unknown.
 */
[[nodiscard]] auto ResolveMaterial(std::string const &name)
  -> ggems::core::materials::GGEMSMaterial {
  auto &manager = ggems::core::materials::GGEMSMaterialManager::GetInstance();
  return manager.Require(manager.GetOrAdd(name));
}

} // namespace

// =============================================================================
// =============================================================================

/*!
 * \brief Registers GGEMS geometry authoring bindings in a Python module.
 *
 * The Box is registered before the World so that add_volume() exposes the
 * Python type of its argument.
 *
 * \param[in,out] module Python module receiving the bindings.
 */
auto BindGeometry(py::module_ &module) -> void {
  using ggems::geometry::GGEMSBox;
  using ggems::geometry::GGEMSWorld;

  // === === ===
  py::class_<GGEMSBox>(module, "GGEMSBox",
                       R"doc(Configure a finite oriented analytic Box.

The Box is placed by the global position of its geometric center and has its
own explicit material. Each full size must be an even number of picometers
after unit conversion, so local faces lie on the picometer lattice. Rotation
is identity by default.

Configuration methods return the same object, so calls can be chained.

Example:
    box = ggems.geometry.GGEMSBox()
    box.set_size(1.2, 0.5, 0.3, "m")
    box.set_position(0.0, 0.0, 0.0, "m")
    box.set_material("Water")
)doc")
    .def(
      py::init<>(),
      R"doc(Create a Box centered on the origin with no size and no material.)doc")

    .def(
      "set_size",
      [](GGEMSBox &self, double size_x, double size_y, double size_z,
         std::string const &unit) -> GGEMSBox & {
        return self.SetSize(
          MakeQuantityOrThrow<ggems::units::Length>(size_x, unit),
          MakeQuantityOrThrow<ggems::units::Length>(size_y, unit),
          MakeQuantityOrThrow<ggems::units::Length>(size_z, unit));
      },
      R"doc(Set the full Box sizes along the local axes.

Each size must convert to an even number of picometers, at least 2 pm.

Args:
    x: Full extent along the local X axis.
    y: Full extent along the local Y axis.
    z: Full extent along the local Z axis.
    unit: GGEMS length unit.

Returns:
    GGEMSBox: This Box, allowing chained configuration.

Example:
    box.set_size(130.0, 20.0, 30.0, "mm")
)doc",
      py::arg("x"), py::arg("y"), py::arg("z"), py::arg("unit") = "mm",
      py::return_value_policy::reference_internal)

    .def(
      "set_rotation",
      [](GGEMSBox &self, double angle_x, double angle_y, double angle_z,
         std::string const &unit) -> GGEMSBox & {
        return self.SetRotation(
          MakeQuantityOrThrow<ggems::units::Angle>(angle_x, unit),
          MakeQuantityOrThrow<ggems::units::Angle>(angle_y, unit),
          MakeQuantityOrThrow<ggems::units::Angle>(angle_z, unit));
      },
      R"doc(Assign the absolute right-handed active rotation Rz * Ry * Rx.

The Box rotates about its center. Fixed-axis X, Y, then Z rotations replace
any previous rotation; positive angles follow the right-hand rule.

Args:
    rx: Rotation about X.
    ry: Rotation about Y.
    rz: Rotation about Z.
    unit: Angle unit, "deg" or "rad".

Returns:
    GGEMSBox: This Box, allowing chained configuration.
)doc",
      py::arg("rx"), py::arg("ry"), py::arg("rz"), py::arg("unit") = "deg",
      py::return_value_policy::reference_internal)

    .def(
      "set_position",
      [](GGEMSBox &self, double pos_x, double pos_y, double pos_z,
         std::string const &unit) -> GGEMSBox & {
        return self.SetPosition(ggems::geometry::MakePositionPM(
          MakeQuantityOrThrow<ggems::units::PositionCoordinate>(pos_x, unit)
            .value,
          MakeQuantityOrThrow<ggems::units::PositionCoordinate>(pos_y, unit)
            .value,
          MakeQuantityOrThrow<ggems::units::PositionCoordinate>(pos_z, unit)
            .value));
      },
      R"doc(Set the global position of the Box geometric center.

Args:
    x: Global X coordinate of the center.
    y: Global Y coordinate of the center.
    z: Global Z coordinate of the center.
    unit: GGEMS length unit.

Returns:
    GGEMSBox: This Box, allowing chained configuration.

Example:
    box.set_position(10.0, -20.0, 5.0, "mm")
)doc",
      py::arg("x"), py::arg("y"), py::arg("z"), py::arg("unit") = "mm",
      py::return_value_policy::reference_internal)

    .def(
      "set_material",
      [](GGEMSBox &self, std::string const &material_name) -> GGEMSBox & {
        return self.SetMaterial(ResolveMaterial(material_name));
      },
      R"doc(Set the Box medium by exact material name.

The name is resolved among registered, built-in, and custom materials; the
material becomes registered as used by the scene.

Args:
    material_name: Exact, case-sensitive material name.

Returns:
    GGEMSBox: This Box, allowing chained configuration.

Example:
    box.set_material("Water")
)doc",
      py::arg("material_name"), py::return_value_policy::reference_internal);

  // === === ===
  py::class_<GGEMSWorld>(module, "GGEMSWorld",
                         R"doc(Configure the finite World of a GGEMS scene.

The World is an axis-aligned box centered on the global origin. It owns the
physical volumes added with add_volume() and is attached to a run with
GGEMSRun.set_world(), which copies the World and checks the completed scene.
Sizes are converted once to the integer picometer lattice; each full size
must be an even number of picometers.

Configuration methods return the same object, so calls can be chained.

Example:
    world = ggems.geometry.GGEMSWorld()
    world.set_size(2.0, 2.0, 2.0, "m")
    world.set_material("Air")
    world.add_volume(box)
    run.set_world(world)
)doc")
    .def(py::init<>(),
         R"doc(Create a World with no size, no material, and no volume.)doc")

    .def(
      "set_size",
      [](GGEMSWorld &self, double size_x, double size_y, double size_z,
         std::string const &unit) -> GGEMSWorld & {
        return self.SetSize(
          MakeQuantityOrThrow<ggems::units::Length>(size_x, unit),
          MakeQuantityOrThrow<ggems::units::Length>(size_y, unit),
          MakeQuantityOrThrow<ggems::units::Length>(size_z, unit));
      },
      R"doc(Set the full World sizes along the global axes.

Each size must convert to an even number of picometers.

Args:
    x: Full extent along the global X axis.
    y: Full extent along the global Y axis.
    z: Full extent along the global Z axis.
    unit: GGEMS length unit.

Returns:
    GGEMSWorld: This World, allowing chained configuration.

Example:
    world.set_size(400.0, 400.0, 600.0, "mm")
)doc",
      py::arg("x"), py::arg("y"), py::arg("z"), py::arg("unit") = "mm",
      py::return_value_policy::reference_internal)

    .def(
      "set_material",
      [](GGEMSWorld &self, std::string const &material_name) -> GGEMSWorld & {
        return self.SetMaterial(ResolveMaterial(material_name));
      },
      R"doc(Set the World medium by exact material name.

The name is resolved among registered, built-in, and custom materials; the
material becomes registered as used by the scene.

Args:
    material_name: Exact, case-sensitive material name.

Returns:
    GGEMSWorld: This World, allowing chained configuration.

Example:
    world.set_material("Air")
)doc",
      py::arg("material_name"), py::return_value_policy::reference_internal)

    .def("add_volume", &GGEMSWorld::AddVolume,
         R"doc(Add one physical volume owned by this World.

The volume must be complete (size and material set) and is copied into the
World; later edits of the caller's object do not reach the copy. The current
implementation admits at most one GGEMSBox. Strict containment in the World is
checked when the World is attached with GGEMSRun.set_world().

Args:
    volume: Configured GGEMSBox.

Returns:
    GGEMSWorld: This World, allowing chained configuration.

Example:
    world.add_volume(box)
)doc",
         py::arg("volume"), py::return_value_policy::reference_internal);
}
