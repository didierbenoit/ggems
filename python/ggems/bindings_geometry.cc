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
#include <memory>
#include <vector>

#include <pybind11/stl.h>
#include <pybind11/pybind11.h>

#include "detail/GGEMSPythonQuantityConversion.hh"
#include "detail/GGEMSGeometryAttachments.hh"

#include "GGEMS/geometry/GGEMSBox.hh"
#include "GGEMS/geometry/GGEMSSphere.hh"
#include "GGEMS/geometry/GGEMSTubs.hh"
#include "GGEMS/geometry/GGEMSCons.hh"
#include "GGEMS/geometry/GGEMSPolyhedra.hh"
#include "GGEMS/geometry/GGEMSEllipsoid.hh"
#include "GGEMS/geometry/GGEMSTrd.hh"
#include "GGEMS/geometry/GGEMSTrap.hh"
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

/*!
 * \brief Binds object-based attachment for all existing daughter types.
 * \tparam Parent Bound World or analytical mother type.
 * \param[in,out] binding Existing class receiving eight typed overloads.
 * \param[in] attachments Module-local C++ registration metadata, retained by
 * the callables; it holds no strong World or shape references.
 */
template <typename Parent>
auto BindAttachments(
  py::class_<Parent, std::shared_ptr<Parent>> &binding,
  std::shared_ptr<ggems::python::detail::GGEMSGeometryAttachments> const
    &attachments) -> void {
  auto const bind = [&]<typename Child> -> void {
    binding.def(
      "add_volume",
      [attachments](std::shared_ptr<Parent> const &parent,
                    std::shared_ptr<Child> const &child) -> Parent & {
        attachments->AddVolume(parent, child);
        return *parent;
      },
      R"doc(Copy a configured volume under this World or registered mother.

Attach the mother to World before adding daughters. Every shape must have one
registration in a live World; duplicate or multiple attachment is rejected.
Position and rotation are relative to this parent. Three physical levels below
World are supported. Physical IDs are assigned internally.

Shape dimensions, pose, and material are copied now; later setters do not change
the occurrence. Adding a daughter extends the original authoring World, which
must remain alive. run.set_world(world) copies the completed World; later
attachments do not change that Run. Bottom-up assembly is not supported.

Args:
    volume: Complete analytical shape, not already attached to a live World.

Returns:
    This parent for chaining.
)doc",
      py::arg("volume"), py::return_value_policy::reference_internal);
  };
  bind.template operator()<ggems::geometry::GGEMSBox>();
  bind.template operator()<ggems::geometry::GGEMSSphere>();
  bind.template operator()<ggems::geometry::GGEMSTubs>();
  bind.template operator()<ggems::geometry::GGEMSEllipsoid>();
  bind.template operator()<ggems::geometry::GGEMSTrd>();
  bind.template operator()<ggems::geometry::GGEMSTrap>();
  bind.template operator()<ggems::geometry::GGEMSCons>();
  bind.template operator()<ggems::geometry::GGEMSPolyhedra>();
}

} // namespace

// =============================================================================
// =============================================================================

/*!
 * \brief Registers GGEMS geometry authoring bindings in a Python module.
 *
 * Analytic volumes are registered before World so that add_volume() exposes the
 * Python type of its argument.
 *
 * \param[in,out] module Python module receiving the bindings.
 */
auto BindGeometry(py::module_ &module) -> void {
  using ggems::geometry::GGEMSBox;
  using ggems::geometry::GGEMSCons;
  using ggems::geometry::GGEMSEllipsoid;
  using ggems::geometry::GGEMSPolyhedra;
  using ggems::geometry::GGEMSSphere;
  using ggems::geometry::GGEMSTrap;
  using ggems::geometry::GGEMSTrd;
  using ggems::geometry::GGEMSTubs;
  using ggems::geometry::GGEMSWorld;

  // === === ===
  auto box_binding =
    py::class_<GGEMSBox, std::shared_ptr<GGEMSBox>>(
      module, "GGEMSBox",
      R"doc(Configure a finite oriented analytic Box.

The Box is placed by the parent-frame position of its geometric center and has
its own explicit material. Each full size must be an even number of picometers
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

The Box rotates about its center. Fixed parent-axis X, Y, then Z rotations
replace any previous rotation; positive angles follow the right-hand rule.

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
        R"doc(Set the parent-frame position of the Box geometric center.

Args:
    x: Parent-frame X coordinate of the center.
    y: Parent-frame Y coordinate of the center.
    z: Parent-frame Z coordinate of the center.
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
  auto sphere_binding =
    py::class_<GGEMSSphere, std::shared_ptr<GGEMSSphere>>(
      module, "GGEMSSphere",
      R"doc(Configure a full solid Sphere by center, radius, and material.

The radius must be strictly positive after conversion to integer picometers.
Odd picometer radii are allowed. The default center is the parent-frame origin.
Rotation changes the frame inherited by daughters, not the spherical surface.
Configuration methods return this Sphere for chaining.
)doc")
      .def(py::init<>(), "Create a Sphere without radius or material.")

      .def(
        "set_radius",
        [](GGEMSSphere &self, double radius,
           std::string const &unit) -> GGEMSSphere & {
          return self.SetRadius(
            MakeQuantityOrThrow<ggems::units::Length>(radius, unit));
        },
        R"doc(Set the strictly positive radius using a GGEMS length unit.

Returns:
    GGEMSSphere: This Sphere, allowing chained configuration.
)doc",
        py::arg("radius"), py::arg("unit") = "mm",
        py::return_value_policy::reference_internal)
      .def(
        "set_position",
        [](GGEMSSphere &self, double x, double y, double z,
           std::string const &unit) -> GGEMSSphere & {
          return self.SetPosition(ggems::geometry::MakePositionPM(
            MakeQuantityOrThrow<ggems::units::PositionCoordinate>(x, unit)
              .value,
            MakeQuantityOrThrow<ggems::units::PositionCoordinate>(y, unit)
              .value,
            MakeQuantityOrThrow<ggems::units::PositionCoordinate>(z, unit)
              .value));
        },
        R"doc(Set the geometric center in the selected parent frame using a

GGEMS length unit.

Returns:
    GGEMSSphere: This Sphere, allowing chained configuration.
)doc",
        py::arg("x"), py::arg("y"), py::arg("z"), py::arg("unit") = "mm",
        py::return_value_policy::reference_internal)

      .def(
        "set_rotation",
        [](GGEMSSphere &self, double x, double y, double z,
           std::string const &unit) -> GGEMSSphere & {
          return self.SetRotation(
            MakeQuantityOrThrow<ggems::units::Angle>(x, unit),
            MakeQuantityOrThrow<ggems::units::Angle>(y, unit),
            MakeQuantityOrThrow<ggems::units::Angle>(z, unit));
        },
        R"doc(Set the local-to-parent daughter frame with active Rz * Ry * Rx rotation.

The spherical boundary is rotation invariant. Angles use a GGEMS angular unit;
this orientation determines how positions and rotations of daughters compose.
Return this Sphere for chaining.
)doc",
        py::arg("x"), py::arg("y"), py::arg("z"), py::arg("unit") = "deg",
        py::return_value_policy::reference_internal)

      .def(
        "set_material",
        [](GGEMSSphere &self, std::string const &name) -> GGEMSSphere & {
          return self.SetMaterial(ResolveMaterial(name));
        },
        R"doc(Set the medium by exact registered, built-in, or custom material name.

The material becomes registered as used by the scene.

Returns:
    GGEMSSphere: This Sphere, allowing chained configuration.
)doc",
        py::arg("name"), py::return_value_policy::reference_internal);

  // === === ===
  auto tubs_binding =
    py::class_<GGEMSTubs, std::shared_ptr<GGEMSTubs>>(
      module, "GGEMSTubs",
      R"doc(Configure a finite full-azimuth circular cylinder or hollow tube along local +Z.

The default pose is centered at the parent-frame origin with identity rotation.
Configuration methods return the same object for chaining. World owns a copy.
)doc")
      .def(py::init<>(), "Create a tube without dimensions or material.")

      .def(
        "set_radii",
        [](GGEMSTubs &self, double outer_radius, double inner_radius,
           std::string const &unit) -> GGEMSTubs & {
          return self.SetRadii(
            MakeQuantityOrThrow<ggems::units::Length>(outer_radius, unit),
            MakeQuantityOrThrow<ggems::units::Length>(inner_radius, unit));
        },
        R"doc(Set radii, not diameters, satisfying 0 <= inner_radius < outer_radius.

Zero inner radius gives a filled cylinder with no inner wall. Return this tube.
)doc",
        py::arg("outer_radius"), py::arg("inner_radius") = 0.0,
        py::arg("unit") = "mm", py::return_value_policy::reference_internal)

      .def(
        "set_length",
        [](GGEMSTubs &self, double length,
           std::string const &unit) -> GGEMSTubs & {
          return self.SetLength(
            MakeQuantityOrThrow<ggems::units::Length>(length, unit));
        },
        R"doc(Set the full cap-to-cap length along local Z and return this tube.

The length must convert to a positive even number of picometers. Caps are at
+/- half length. The azimuth is always the full 360 degrees.
)doc",
        py::arg("length"), py::arg("unit") = "mm",
        py::return_value_policy::reference_internal)

      .def(
        "set_position",
        [](GGEMSTubs &self, double x, double y, double z,
           std::string const &unit) -> GGEMSTubs & {
          return self.SetPosition(ggems::geometry::MakePositionPM(
            MakeQuantityOrThrow<ggems::units::PositionCoordinate>(x, unit)
              .value,
            MakeQuantityOrThrow<ggems::units::PositionCoordinate>(y, unit)
              .value,
            MakeQuantityOrThrow<ggems::units::PositionCoordinate>(z, unit)
              .value));
        },
        "Set the parent-frame geometric center and return this tube.",
        py::arg("x"), py::arg("y"), py::arg("z"), py::arg("unit") = "mm",
        py::return_value_policy::reference_internal)

      .def(
        "set_rotation",
        [](GGEMSTubs &self, double rx, double ry, double rz,
           std::string const &unit) -> GGEMSTubs & {
          return self.SetRotation(
            MakeQuantityOrThrow<ggems::units::Angle>(rx, unit),
            MakeQuantityOrThrow<ggems::units::Angle>(ry, unit),
            MakeQuantityOrThrow<ggems::units::Angle>(rz, unit));
        },
        R"doc(Assign the absolute active rotation Rz * Ry * Rx about the center.

Positive angles follow the right-hand rule about fixed X, Y, then Z axes.
Axes belong to the parent selected by add_volume(), or World by default.
Calls replace the previous rotation and return this tube.
)doc",
        py::arg("rx"), py::arg("ry"), py::arg("rz"), py::arg("unit") = "deg",
        py::return_value_policy::reference_internal)

      .def(
        "set_material",
        [](GGEMSTubs &self, std::string const &name) -> GGEMSTubs & {
          return self.SetMaterial(ResolveMaterial(name));
        },
        R"doc(Set the medium by exact registered, built-in, or custom material name.

The material becomes registered as used by the scene. Return this tube.
)doc",
        py::arg("name"), py::return_value_policy::reference_internal);

  // === === ===
  auto ellipsoid_binding =
    py::class_<GGEMSEllipsoid, std::shared_ptr<GGEMSEllipsoid>>(
      module, "GGEMSEllipsoid",
      R"doc(Configure a complete solid ellipsoid with local X/Y/Z semi-axes and a rigid pose.

The default pose is centered at the parent-frame origin with identity rotation.
Configuration methods return the same object for chaining. World owns a copy.
)doc")
      .def(py::init<>(), "Create an ellipsoid without dimensions or material.")

      .def(
        "set_semi_axes",
        [](GGEMSEllipsoid &self, double x, double y, double z,
           std::string const &unit) -> GGEMSEllipsoid & {
          return self.SetSemiAxes(
            MakeQuantityOrThrow<ggems::units::Length>(x, unit),
            MakeQuantityOrThrow<ggems::units::Length>(y, unit),
            MakeQuantityOrThrow<ggems::units::Length>(z, unit));
        },
        R"doc(Set the strictly positive local X/Y/Z semi-axes and return this ellipsoid.

These are radii, not full dimensions. Odd picometer values are allowed. The
complete local surface is (x/a)^2 + (y/b)^2 + (z/c)^2 = 1, without cut planes.
)doc",
        py::arg("x"), py::arg("y"), py::arg("z"), py::arg("unit") = "mm",
        py::return_value_policy::reference_internal)

      .def(
        "set_position",
        [](GGEMSEllipsoid &self, double x, double y, double z,
           std::string const &unit) -> GGEMSEllipsoid & {
          return self.SetPosition(ggems::geometry::MakePositionPM(
            MakeQuantityOrThrow<ggems::units::PositionCoordinate>(x, unit)
              .value,
            MakeQuantityOrThrow<ggems::units::PositionCoordinate>(y, unit)
              .value,
            MakeQuantityOrThrow<ggems::units::PositionCoordinate>(z, unit)
              .value));
        },
        "Set the parent-frame geometric center and return this ellipsoid.",
        py::arg("x"), py::arg("y"), py::arg("z"), py::arg("unit") = "mm",
        py::return_value_policy::reference_internal)

      .def(
        "set_rotation",
        [](GGEMSEllipsoid &self, double rx, double ry, double rz,
           std::string const &unit) -> GGEMSEllipsoid & {
          return self.SetRotation(
            MakeQuantityOrThrow<ggems::units::Angle>(rx, unit),
            MakeQuantityOrThrow<ggems::units::Angle>(ry, unit),
            MakeQuantityOrThrow<ggems::units::Angle>(rz, unit));
        },
        R"doc(Assign the absolute active rotation Rz * Ry * Rx about the center.

Positive angles follow the right-hand rule about fixed X, Y, then Z axes.
Axes belong to the parent selected by add_volume(), or World by default.
Calls replace the previous rotation and return this ellipsoid.
)doc",
        py::arg("rx"), py::arg("ry"), py::arg("rz"), py::arg("unit") = "deg",
        py::return_value_policy::reference_internal)

      .def(
        "set_material",
        [](GGEMSEllipsoid &self, std::string const &name) -> GGEMSEllipsoid & {
          return self.SetMaterial(ResolveMaterial(name));
        },
        R"doc(Set the medium by exact registered, built-in, or custom material name.

The material becomes registered as used by the scene. Return this ellipsoid.
)doc",
        py::arg("name"), py::return_value_policy::reference_internal);

  // === === ===
  auto trd_binding =
    py::class_<GGEMSTrd, std::shared_ptr<GGEMSTrd>>(
      module, "GGEMSTrd",
      R"doc(Configure a solid with independently sized rectangular Z end faces.

All dimensions are full sizes, positive and even in pm after Units conversion.
Sides connect corresponding end edges; Trd has no end skew or inclination.
Rigid XYZ rotation is separate. World owns a copy; setters return this object.
)doc")
      .def(py::init<>(), "Create a Trd without dimensions or material.")

      .def(
        "set_negative_z_face",
        [](GGEMSTrd &self, double size_x, double size_y,
           std::string const &unit) -> GGEMSTrd & {
          return self.SetNegativeZFace(
            MakeQuantityOrThrow<ggems::units::Length>(size_x, unit),
            MakeQuantityOrThrow<ggems::units::Length>(size_y, unit));
        },
        "Set full X/Y sizes at the negative-Z end and return this solid.",
        py::arg("size_x"), py::arg("size_y"), py::arg("unit") = "mm",
        py::return_value_policy::reference_internal)

      .def(
        "set_positive_z_face",
        [](GGEMSTrd &self, double size_x, double size_y,
           std::string const &unit) -> GGEMSTrd & {
          return self.SetPositiveZFace(
            MakeQuantityOrThrow<ggems::units::Length>(size_x, unit),
            MakeQuantityOrThrow<ggems::units::Length>(size_y, unit));
        },
        "Set full X/Y sizes at the positive-Z end and return this solid.",
        py::arg("size_x"), py::arg("size_y"), py::arg("unit") = "mm",
        py::return_value_policy::reference_internal)

      .def(
        "set_length",
        [](GGEMSTrd &self, double length,
           std::string const &unit) -> GGEMSTrd & {
          return self.SetLength(
            MakeQuantityOrThrow<ggems::units::Length>(length, unit));
        },
        "Set the full Z cap-to-cap length and return this solid.",
        py::arg("length"), py::arg("unit") = "mm",
        py::return_value_policy::reference_internal)

      .def(
        "set_position",
        [](GGEMSTrd &self, double x, double y, double z,
           std::string const &unit) -> GGEMSTrd & {
          return self.SetPosition(ggems::geometry::MakePositionPM(
            MakeQuantityOrThrow<ggems::units::PositionCoordinate>(x, unit)
              .value,
            MakeQuantityOrThrow<ggems::units::PositionCoordinate>(y, unit)
              .value,
            MakeQuantityOrThrow<ggems::units::PositionCoordinate>(z, unit)
              .value));
        },
        "Set the parent-frame position of the local frame origin and return "
        "this solid.",
        py::arg("x"), py::arg("y"), py::arg("z"), py::arg("unit") = "mm",
        py::return_value_policy::reference_internal)

      .def(
        "set_rotation",
        [](GGEMSTrd &self, double rx, double ry, double rz,
           std::string const &unit) -> GGEMSTrd & {
          return self.SetRotation(
            MakeQuantityOrThrow<ggems::units::Angle>(rx, unit),
            MakeQuantityOrThrow<ggems::units::Angle>(ry, unit),
            MakeQuantityOrThrow<ggems::units::Angle>(rz, unit));
        },
        R"doc(Replace the absolute right-handed fixed-axis rotation Rz * Ry * Rx.

Return this solid. Rotation acts about the local frame origin using parent axes
selected by add_volume(), or World axes by default.
)doc",
        py::arg("rx"), py::arg("ry"), py::arg("rz"), py::arg("unit") = "deg",
        py::return_value_policy::reference_internal)

      .def(
        "set_material",
        [](GGEMSTrd &self, std::string const &name) -> GGEMSTrd & {
          return self.SetMaterial(ResolveMaterial(name));
        },
        R"doc(Set an owned medium by exact registered, built-in or custom name.

The Material becomes registered as used by the scene. Return this solid.
)doc",
        py::arg("name"), py::return_value_policy::reference_internal);

  // === === ===
  auto trap_binding =
    py::class_<GGEMSTrap, std::shared_ptr<GGEMSTrap>>(
      module, "GGEMSTrap",
      R"doc(Configure a general planar-faced trapezoid.

All dimensions are full sizes, positive and even in pm after Units conversion.
End-face height and two X widths follow the G4Trap geometric convention.
Inclination shifts the end centers; skew shifts X within each end face.
The side faces must be planar within the automatic local FP32 surface band:
incompatible end skews or width gradients fail when added to World.
Rigid XYZ rotation is separate. World owns a copy; setters return this object.
)doc")
      .def(py::init<>(), "Create a Trap without dimensions or material.")

      .def(
        "set_negative_z_face",
        [](GGEMSTrap &self, double height_y, double width_at_negative_y,
           double width_at_positive_y, double skew, std::string const &unit,
           std::string const &angle_unit) -> GGEMSTrap & {
          return self.SetNegativeZFace(
            MakeQuantityOrThrow<ggems::units::Length>(height_y, unit),
            MakeQuantityOrThrow<ggems::units::Length>(width_at_negative_y,
                                                      unit),
            MakeQuantityOrThrow<ggems::units::Length>(width_at_positive_y,
                                                      unit),
            MakeQuantityOrThrow<ggems::units::Angle>(skew, angle_unit));
        },
        R"doc(Set full Y height, negative-Y X width, positive-Y X width and end skew.

The face lies at -half the Z length. Its X center at local Y offset v is
v * tan(skew). Skew must lie strictly between -90 and +90 degrees.
eturn this solid; side planarity is checked when copied into World.
)doc",
        py::arg("height_y"), py::arg("width_at_negative_y"),
        py::arg("width_at_positive_y"), py::arg("skew") = 0.0,
        py::arg("unit") = "mm", py::arg("angle_unit") = "deg",
        py::return_value_policy::reference_internal)

      .def(
        "set_positive_z_face",
        [](GGEMSTrap &self, double height_y, double width_at_negative_y,
           double width_at_positive_y, double skew, std::string const &unit,
           std::string const &angle_unit) -> GGEMSTrap & {
          return self.SetPositiveZFace(
            MakeQuantityOrThrow<ggems::units::Length>(height_y, unit),
            MakeQuantityOrThrow<ggems::units::Length>(width_at_negative_y,
                                                      unit),
            MakeQuantityOrThrow<ggems::units::Length>(width_at_positive_y,
                                                      unit),
            MakeQuantityOrThrow<ggems::units::Angle>(skew, angle_unit));
        },
        R"doc(Set full Y height, negative-Y X width, positive-Y X width and end skew.

The face lies at +half the Z length. Its X center at local Y offset v is
v * tan(skew). Skew must lie strictly between -90 and +90 degrees.
Return this solid; side planarity is checked when copied into World.
)doc",
        py::arg("height_y"), py::arg("width_at_negative_y"),
        py::arg("width_at_positive_y"), py::arg("skew") = 0.0,
        py::arg("unit") = "mm", py::arg("angle_unit") = "deg",
        py::return_value_policy::reference_internal)

      .def(
        "set_length",
        [](GGEMSTrap &self, double length,
           std::string const &unit) -> GGEMSTrap & {
          return self.SetLength(
            MakeQuantityOrThrow<ggems::units::Length>(length, unit));
        },
        "Set the full Z cap-to-cap length and return this solid.",
        py::arg("length"), py::arg("unit") = "mm",
        py::return_value_policy::reference_internal)

      .def(
        "set_inclination",
        [](GGEMSTrap &self, double theta, double phi,
           std::string const &unit) -> GGEMSTrap & {
          return self.SetInclination(
            MakeQuantityOrThrow<ggems::units::Angle>(theta, unit),
            MakeQuantityOrThrow<ggems::units::Angle>(phi, unit));
        },
        R"doc(Set the center-line inclination separately from rigid rotation.

The face center at local z is z * tan(theta) * (cos(phi), sin(phi)).
Theta must lie strictly between -90 and +90 degrees; phi is azimuth from +X
toward +Y. Return this solid.
)doc",
        py::arg("theta"), py::arg("phi"), py::arg("unit") = "deg",
        py::return_value_policy::reference_internal)

      .def(
        "set_position",
        [](GGEMSTrap &self, double x, double y, double z,
           std::string const &unit) -> GGEMSTrap & {
          return self.SetPosition(ggems::geometry::MakePositionPM(
            MakeQuantityOrThrow<ggems::units::PositionCoordinate>(x, unit)
              .value,
            MakeQuantityOrThrow<ggems::units::PositionCoordinate>(y, unit)
              .value,
            MakeQuantityOrThrow<ggems::units::PositionCoordinate>(z, unit)
              .value));
        },
        "Set the parent-frame position of the local frame origin and return "
        "this solid.",
        py::arg("x"), py::arg("y"), py::arg("z"), py::arg("unit") = "mm",
        py::return_value_policy::reference_internal)

      .def(
        "set_rotation",
        [](GGEMSTrap &self, double rx, double ry, double rz,
           std::string const &unit) -> GGEMSTrap & {
          return self.SetRotation(
            MakeQuantityOrThrow<ggems::units::Angle>(rx, unit),
            MakeQuantityOrThrow<ggems::units::Angle>(ry, unit),
            MakeQuantityOrThrow<ggems::units::Angle>(rz, unit));
        },
        R"doc(Replace the absolute right-handed fixed-axis rotation Rz * Ry * Rx.

Return this solid. Rotation acts about the local frame origin using parent axes
selected by add_volume(), or World axes by default.
)doc",
        py::arg("rx"), py::arg("ry"), py::arg("rz"), py::arg("unit") = "deg",
        py::return_value_policy::reference_internal)

      .def(
        "set_material",
        [](GGEMSTrap &self, std::string const &name) -> GGEMSTrap & {
          return self.SetMaterial(ResolveMaterial(name));
        },
        R"doc(Set an owned medium by exact registered, built-in or custom name.

The Material becomes registered as used by the scene. Return this solid.
)doc",
        py::arg("name"), py::return_value_policy::reference_internal);

  // === === ===
  auto cons_binding =
    py::class_<GGEMSCons, std::shared_ptr<GGEMSCons>>(
      module, "GGEMSCons",
      R"doc(Configure a full-azimuth solid or hollow conical frustum along local +Z.

Radii interpolate linearly between caps at +/- half length. A solid cone tip
is admitted; hollow outer tips are rejected. The default pose is centered at the
parent-frame origin with identity rotation. Configuration methods return the
same object for chaining. World owns a copy.
)doc")
      .def(py::init<>(), "Create a frustum without dimensions or material.")

      .def(
        "set_negative_z_radii",
        [](GGEMSCons &self, double inner_radius, double outer_radius,
           std::string const &unit) -> GGEMSCons & {
          return self.SetNegativeZRadii(
            MakeQuantityOrThrow<ggems::units::Length>(inner_radius, unit),
            MakeQuantityOrThrow<ggems::units::Length>(outer_radius, unit));
        },
        "Set inner and outer radii at the negative-Z cap and return this "
        "frustum.",
        py::arg("inner_radius"), py::arg("outer_radius"),
        py::arg("unit") = "mm", py::return_value_policy::reference_internal)

      .def(
        "set_positive_z_radii",
        [](GGEMSCons &self, double inner_radius, double outer_radius,
           std::string const &unit) -> GGEMSCons & {
          return self.SetPositiveZRadii(
            MakeQuantityOrThrow<ggems::units::Length>(inner_radius, unit),
            MakeQuantityOrThrow<ggems::units::Length>(outer_radius, unit));
        },
        "Set inner and outer radii at the positive-Z cap and return this "
        "frustum.",
        py::arg("inner_radius"), py::arg("outer_radius"),
        py::arg("unit") = "mm", py::return_value_policy::reference_internal)

      .def(
        "set_length",
        [](GGEMSCons &self, double length,
           std::string const &unit) -> GGEMSCons & {
          return self.SetLength(
            MakeQuantityOrThrow<ggems::units::Length>(length, unit));
        },
        R"doc(Set the full cap-to-cap length along local Z and return this frustum.

The length must convert to a positive even number of picometers. Caps are at
+/- half length. The azimuth is always the full 360 degrees.
)doc",
        py::arg("length"), py::arg("unit") = "mm",
        py::return_value_policy::reference_internal)

      .def(
        "set_position",
        [](GGEMSCons &self, double x, double y, double z,
           std::string const &unit) -> GGEMSCons & {
          return self.SetPosition(ggems::geometry::MakePositionPM(
            MakeQuantityOrThrow<ggems::units::PositionCoordinate>(x, unit)
              .value,
            MakeQuantityOrThrow<ggems::units::PositionCoordinate>(y, unit)
              .value,
            MakeQuantityOrThrow<ggems::units::PositionCoordinate>(z, unit)
              .value));
        },
        "Set the parent-frame geometric center and return this frustum.",
        py::arg("x"), py::arg("y"), py::arg("z"), py::arg("unit") = "mm",
        py::return_value_policy::reference_internal)

      .def(
        "set_rotation",
        [](GGEMSCons &self, double rx, double ry, double rz,
           std::string const &unit) -> GGEMSCons & {
          return self.SetRotation(
            MakeQuantityOrThrow<ggems::units::Angle>(rx, unit),
            MakeQuantityOrThrow<ggems::units::Angle>(ry, unit),
            MakeQuantityOrThrow<ggems::units::Angle>(rz, unit));
        },
        R"doc(Assign the absolute active rotation Rz * Ry * Rx about the center.

Positive angles follow the right-hand rule about fixed X, Y, then Z axes.
Axes belong to the parent selected by add_volume(), or World by default.
Calls replace the previous rotation and return this frustum.
)doc",
        py::arg("rx"), py::arg("ry"), py::arg("rz"), py::arg("unit") = "deg",
        py::return_value_policy::reference_internal)

      .def(
        "set_material",
        [](GGEMSCons &self, std::string const &name) -> GGEMSCons & {
          return self.SetMaterial(ResolveMaterial(name));
        },
        R"doc(Set the medium by exact registered, built-in, or custom material name.

The material becomes registered as used by the scene. Return this frustum.
)doc",
        py::arg("name"), py::return_value_policy::reference_internal);

  // === === ===
  auto polyhedra_binding =
    py::class_<GGEMSPolyhedra, std::shared_ptr<GGEMSPolyhedra>>(
      module, "GGEMSPolyhedra",
      R"doc(Configure a full-azimuth regular polygon with continuous axial profiles.

Authoring radii are apothems (distances to sides). The first vertex is at
start_angle. Signed Z coordinates locate planes in the local frame; position
places that frame's origin, and rotation acts about it. Convex and concave
profiles, solid sections, and tapered bores share one material and VolumeID.
Repeated Z steps and zero outer radii are rejected. Setters return this object.
)doc")
      .def(py::init<>(),
           "Create a Polyhedra without side count, profiles or material.")

      .def("set_side_count", &GGEMSPolyhedra::SetSideCount,
           "Set the regular polygon side count (at least three). Return this "
           "solid.",
           py::arg("side_count"), py::return_value_policy::reference_internal)

      .def(
        "set_start_angle",
        [](GGEMSPolyhedra &self, double angle,
           std::string const &unit) -> GGEMSPolyhedra & {
          return self.SetStartAngle(
            MakeQuantityOrThrow<ggems::units::Angle>(angle, unit));
        },
        "Set the first vertex angle, from local +X toward +Y. Return this "
        "solid.",
        py::arg("angle"), py::arg("unit") = "deg",
        py::return_value_policy::reference_internal)

      .def(
        "set_z_profile",
        [](GGEMSPolyhedra &self, std::vector<double> const &z,
           std::vector<double> const &inner, std::vector<double> const &outer,
           std::string const &unit) -> GGEMSPolyhedra & {
          std::vector<ggems::units::PositionCoordinate> planes;
          std::vector<ggems::units::Length> inner_radii;
          std::vector<ggems::units::Length> outer_radii;
          planes.reserve(z.size());
          inner_radii.reserve(inner.size());
          outer_radii.reserve(outer.size());
          for (auto value : z) {
            planes.push_back(
              MakeQuantityOrThrow<ggems::units::PositionCoordinate>(value,
                                                                    unit));
          }
          for (auto value : inner) {
            inner_radii.push_back(
              MakeQuantityOrThrow<ggems::units::Length>(value, unit));
          }
          for (auto value : outer) {
            outer_radii.push_back(
              MakeQuantityOrThrow<ggems::units::Length>(value, unit));
          }
          return self.SetZProfile(planes, inner_radii, outer_radii);
        },
        R"doc(Set equally sized Z, inner-apothem and outer-apothem sequences.

Z coordinates are absolute local coordinates and must strictly increase.
Radii are distances to polygon sides, linearly interpolated between planes.
Inner radius zero closes the bore; outer radius must remain strictly positive.
Repeated Z planes and steps are rejected. Return this solid.
)doc",
        py::arg("z"), py::arg("inner"), py::arg("outer"),
        py::arg("unit") = "mm", py::return_value_policy::reference_internal)

      .def(
        "set_position",
        [](GGEMSPolyhedra &self, double x, double y, double z,
           std::string const &unit) -> GGEMSPolyhedra & {
          return self.SetPosition(ggems::geometry::MakePositionPM(
            MakeQuantityOrThrow<ggems::units::PositionCoordinate>(x, unit)
              .value,
            MakeQuantityOrThrow<ggems::units::PositionCoordinate>(y, unit)
              .value,
            MakeQuantityOrThrow<ggems::units::PositionCoordinate>(z, unit)
              .value));
        },
        "Set the parent-frame position of the local frame origin and return "
        "this solid.",
        py::arg("x"), py::arg("y"), py::arg("z"), py::arg("unit") = "mm",
        py::return_value_policy::reference_internal)

      .def(
        "set_rotation",
        [](GGEMSPolyhedra &self, double rx, double ry, double rz,
           std::string const &unit) -> GGEMSPolyhedra & {
          return self.SetRotation(
            MakeQuantityOrThrow<ggems::units::Angle>(rx, unit),
            MakeQuantityOrThrow<ggems::units::Angle>(ry, unit),
            MakeQuantityOrThrow<ggems::units::Angle>(rz, unit));
        },
        R"doc(Assign the absolute active rotation Rz * Ry * Rx about the local frame origin.

Positive angles follow the right-hand rule about fixed X, Y, then Z axes.
Axes belong to the parent selected by add_volume(), or World by default.
Calls replace the previous rotation and return this solid.
)doc",
        py::arg("rx"), py::arg("ry"), py::arg("rz"), py::arg("unit") = "deg",
        py::return_value_policy::reference_internal)

      .def(
        "set_material",
        [](GGEMSPolyhedra &self, std::string const &name) -> GGEMSPolyhedra & {
          return self.SetMaterial(ResolveMaterial(name));
        },
        R"doc(Set the medium by exact registered, built-in, or custom material name.

The material becomes registered as used by the scene. Return this solid.
)doc",
        py::arg("name"), py::return_value_policy::reference_internal);

  // === === ===
  auto world_binding =
    py::class_<GGEMSWorld, std::shared_ptr<GGEMSWorld>>(
      module, "GGEMSWorld",
      R"doc(Configure the finite World of a GGEMS scene.

The World is an axis-aligned box centered on the global origin. It owns the
physical volumes added with add_volume() and is attached to a run with
GGEMSRun.set_world(), which copies the World and checks the completed scene.
Sizes are converted once to the integer picometer lattice; each full size
must be an even number of picometers.

Physical occurrences may use any supported analytical shape. Add a configured
mother with world.add_volume(mother), then daughters with
mother.add_volume(child). Each call copies the shape value; subsequent setters
do not change that copy. The authoring World must remain alive while daughters
are added. Physical IDs are internal. Position and rotation use the parent's
local frame. Physical depth is limited to three below World. Sphere rotation
defines its daughter frame while leaving its spherical surface unchanged.
Bottom-up assembly is rejected.

A daughter replaces its parent's medium only inside all ancestor solids.
Nominal daughters may protrude beyond ancestors or World. Sibling effective
interiors must be disjoint; surface contacts are allowed. General effective
support overlap detection is not provided. Clipped intervals and intended gaps
must remain resolvable outside adjacent FP32 surface bands; smaller gaps may
behave as contact. Direct World children retain strict World clearance.

Configuration methods return the same object, so calls can be chained.

Example:
    world = ggems.geometry.GGEMSWorld()
    world.set_size(2.0, 2.0, 2.0, "m")
    world.set_material("Air")
    world.add_volume(mother)
    mother.add_volume(daughter)
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
        py::arg("material_name"), py::return_value_policy::reference_internal);

  auto const attachments =
    std::make_shared<ggems::python::detail::GGEMSGeometryAttachments>();
  BindAttachments(box_binding, attachments);
  BindAttachments(sphere_binding, attachments);
  BindAttachments(tubs_binding, attachments);
  BindAttachments(ellipsoid_binding, attachments);
  BindAttachments(trd_binding, attachments);
  BindAttachments(trap_binding, attachments);
  BindAttachments(cons_binding, attachments);
  BindAttachments(polyhedra_binding, attachments);
  BindAttachments(world_binding, attachments);
}
