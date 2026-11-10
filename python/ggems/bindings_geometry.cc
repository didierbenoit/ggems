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
 * \brief Defines Python bindings for GGEMS World and physical volume authoring.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <memory>
#include <vector>

// Keep Python C API inclusion behind pybind11's Windows Debug boundary.
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>
#include <pybind11/pybind11.h>
#include <pybind11/cast.h>
#include <pybind11/pytypes.h>

#include "detail/GGEMSPythonQuantityConversion.hh"
#include "detail/GGEMSGeometryAttachments.hh"

#include "GGEMS/geometry/GGEMSVoxelizedVolume.hh"
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
#include "GGEMS/geometry/GGEMSVolumeAuthoring.hh"
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
 * \brief Converts a parent-local position for a physical volume.
 * \tparam Shape Physical volume with a parent-local pose.
 * \param[in,out] shape Shape receiving the absolute position.
 * \param[in] x Parent-local X coordinate.
 * \param[in] y Parent-local Y coordinate.
 * \param[in] z Parent-local Z coordinate.
 * \param[in] unit GGEMS length unit.
 * \return The same shape for chained configuration.
 */
template <class Shape>
auto SetPosition(Shape &shape, double x, double y, double z,
                 std::string const &unit) -> Shape & {
  return shape.SetPosition(ggems::geometry::MakePositionPM(
    MakeQuantityOrThrow<ggems::units::PositionCoordinate>(x, unit).value,
    MakeQuantityOrThrow<ggems::units::PositionCoordinate>(y, unit).value,
    MakeQuantityOrThrow<ggems::units::PositionCoordinate>(z, unit).value));
}

/*!
 * \brief Converts absolute parent-axis angles for a physical volume.
 * \tparam Shape Physical volume with a parent-local pose.
 * \param[in,out] shape Shape receiving the absolute rotation.
 * \param[in] x Rotation about the parent X axis.
 * \param[in] y Rotation about the parent Y axis.
 * \param[in] z Rotation about the parent Z axis.
 * \param[in] unit GGEMS angular unit.
 * \return The same shape for chained configuration.
 */
template <class Shape>
auto SetRotation(Shape &shape, double x, double y, double z,
                 std::string const &unit) -> Shape & {
  return shape.SetRotation(MakeQuantityOrThrow<ggems::units::Angle>(x, unit),
                           MakeQuantityOrThrow<ggems::units::Angle>(y, unit),
                           MakeQuantityOrThrow<ggems::units::Angle>(z, unit));
}

/*!
 * \brief Resolves and assigns a medium to World or an analytical shape.
 * \tparam Volume World or analytical shape type.
 * \param[in,out] volume Authoring value receiving the material.
 * \param[in] name Exact registered, built-in, or custom material name.
 * \return The same value for chained configuration.
 */
template <class Volume>
auto SetMaterial(Volume &volume, std::string const &name) -> Volume & {
  return volume.SetMaterial(ResolveMaterial(name));
}

/*!
 * \brief Converts an actual positive Python integer to the occurrence domain.
 * \param[in] value Python integer; Boolean values are not admitted counts.
 * \return Count in the existing unsigned 32-bit occurrence domain.
 * \throws py::type_error If value is not an integer or is a Boolean.
 * \throws py::value_error If value is zero, negative, or exceeds uint32.
 */
[[nodiscard]] auto ParseRepeatCount(py::handle value) -> std::uint32_t {
  if (!py::isinstance<py::int_>(value) || py::isinstance<py::bool_>(value)) {
    throw py::type_error("count must be a positive integer, not a Boolean.");
  }
  std::uint32_t count{};
  try {
    count = py::cast<std::uint32_t>(value);
  } catch (py::cast_error const &) {
    throw py::value_error("count must be in [1, 4294967295].");
  }
  if (count == 0U) {
    throw py::value_error("count must be in [1, 4294967295].");
  }
  return count;
}

/*!
 * \brief Borrows an exact three-element tuple for Cartesian repetition.
 * \param[in] value Unconverted Python count or pitch argument.
 * \param[in] name Argument name used in diagnostics.
 * \return Borrowed tuple after type and length validation.
 * \throws py::type_error If value is not a built-in tuple.
 * \throws py::value_error If the tuple does not contain three entries.
 */
[[nodiscard]] auto ParseRepeatTuple(py::handle value, std::string_view name)
  -> py::tuple {
  if (!PyTuple_CheckExact(value.ptr())) {
    throw py::type_error(std::string{name} + " must be a built-in tuple.");
  }

  auto values = py::reinterpret_borrow<py::tuple>(value);

  if (values.size() != 3U) {
    throw py::value_error(
      std::string{name} +
      " must contain exactly three entries ordered X, Y, Z.");
  }

  return values;
}

/*!
 * \brief Converts three exact Cartesian count entries to physical counts.
 * \param[in] value Built-in count tuple ordered X, Y, and Z.
 * \return Positive unsigned 32-bit counts in the same order.
 * \throws py::type_error If tuple or entry types are invalid, including bool.
 * \throws py::value_error If length, positivity, or numeric range is invalid.
 */
[[nodiscard]] auto ParseRepeatCounts(py::handle value)
  -> std::array<std::uint32_t, 3> {
  auto const values = ParseRepeatTuple(value, "count");

  std::array<std::uint32_t, 3> counts{};

  for (std::size_t index = 0U; index < counts.size(); ++index) {
    counts[index] = ParseRepeatCount(values[index]);
  }

  return counts;
}

/*!
 * \brief Converts three Cartesian pitches through the central unit policy.
 * \param[in] value Built-in pitch tuple ordered X, Y, and Z.
 * \param[in] unit Explicit GGEMS length unit shared by the three entries.
 * \return Finite nonnegative canonical spacings, including singleton axes.
 * \throws py::type_error If tuple or numeric entry types are invalid.
 * \throws py::value_error If length, unit, sign, finiteness, or range is
 * invalid.
 */
[[nodiscard]] auto ParseRepeatPitches(py::handle value, std::string_view unit)
  -> std::array<ggems::units::Length, 3> {
  auto const values = ParseRepeatTuple(value, "pitch");

  std::array<ggems::units::Length, 3> pitches{};

  for (std::size_t index = 0U; index < pitches.size(); ++index) {
    double spacing{};

    try {
      spacing = py::cast<double>(values[index]);
    } catch (py::cast_error const &) {
      throw py::type_error("pitch entries must be real numbers.");
    }

    pitches[index] = MakeQuantityOrThrow<ggems::units::Length>(spacing, unit);
  }

  return pitches;
}

/*!
 * \brief Resolves one exact positive mother-local axis token.
 * \param[in] axis Case-sensitive X, Y, or Z token.
 * \return Corresponding positive Cartesian repetition axis.
 * \throws py::value_error If the token is unsupported.
 */
[[nodiscard]] auto ParseRepeatAxis(std::string_view axis)
  -> ggems::geometry::GGEMSRepeatAxis {
  using ggems::geometry::GGEMSRepeatAxis;
  if (axis == "X") {
    return GGEMSRepeatAxis::X;
  }
  if (axis == "Y") {
    return GGEMSRepeatAxis::Y;
  }
  if (axis == "Z") {
    return GGEMSRepeatAxis::Z;
  }
  throw py::value_error(
    "axis must be exactly X, Y, or Z; reversed axes are unsupported.");
}

/*!
 * \brief Binds value attachment and two repeaters for all analytical children.
 * \tparam Parent Bound World or analytical mother type.
 * \param[in,out] binding Existing class receiving eight typed child overloads.
 * \param[in] attachments Module-local weak metadata for unique top-down
 * attachments; scientific descriptions remain owned by C++ authoring values.
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
      R"doc(Capture a configured volume and its current subtree by value.

Detached mothers own the captured description. Later changes to the source child
never update that copy. A detached mother may be translated or rotated before
or after adding children, until its own value is captured by its parent.
Position and rotation are parent-local; physical depth is at most four below
World. Physical IDs are assigned internally.

World attachment copies the complete subtree independently. An explicitly
attached root remains a handle for top-down additions to its original live
World; its later setters do not change the stored pose, shape, material, or
color. Descendant sources captured bottom-up have no such handle. Adding
children to those sources changes only their own authoring values. Direct
self-use and reuse of a source already registered in a live World are rejected.

Every World or registered-mother attachment requires configured World size and
material and validates the complete batch before publication, including a
single leaf. A rejected attachment leaves the World and registrations unchanged.
run.set_world(world) copies the completed World; later attachments do not change
the Run.

Args:
    volume: Complete analytical shape and its currently authored descendants.

Returns:
    This parent for chaining.
)doc",
      py::arg("volume"), py::return_value_policy::reference_internal);

    binding.def(
      "linear_repeater",
      [attachments](std::shared_ptr<Parent> const &parent,
                    std::shared_ptr<Child> const &child, py::handle count,
                    double pitch, std::string const &unit,
                    std::string const &axis) -> Parent & {
        auto const copies = ParseRepeatCount(count);
        auto const repeat_axis = ParseRepeatAxis(axis);
        auto const spacing =
          MakeQuantityOrThrow<ggems::units::Length>(pitch, unit);
        attachments->LinearRepeater(parent, child, copies, spacing,
                                    repeat_axis);
        return *parent;
      },
      R"doc(Capture a centered line of subtree copies along a mother-local axis.

The source position p is the center of the whole line. Copy i has nominal
position p + (i - (count - 1)/2)*pitch*axis and the unchanged source orientation.
This centered convention intentionally replaces the earlier first-copy
positioning. Fractional-picometer offsets truncate toward zero before addition
to the source position. No manual centering position is needed.

Pitch is the center-to-center spacing, with an explicit GGEMS length unit. It
must be finite, nonnegative, and representable even for count=1, where spacing
does not change placement. Multiple copies require a strictly positive converted
pitch. Reversed axes are unsupported. The tuple overload takes exact count and
pitch triples ordered X, Y, Z and accepts no axis argument.

The complete child subtree is captured now. Later source edits do not alter the
recipe. The detached mother's final pose composes with every occurrence when
attached to World. A uniquely registered mother instead extends its original
World using the already captured parent pose. Repeated source objects never
become live handles for individual occurrences. Direct self-use and reuse of a
source registered in a live World are rejected.

World expansion requires configured size and material. Existing depth, capacity,
FP32, containment, and overlap admission apply atomically to the expanded batch.
Bounded overlap checks do not prove arbitrary geometry overlap-free. Initial
expansion uses ordinary occurrences and linear device candidate scans; this is
not a scalable accelerator for large repeated scenes.

Args:
    volume: Complete analytical reference volume and captured descendants.
    count: Positive integer total count, excluding Booleans.
    pitch: Finite nonnegative center-to-center spacing.
    unit: Explicit GGEMS length unit.
    axis: Exactly "X", "Y", or "Z" in the mother's local frame.

Returns:
    This parent for chaining.
)doc",
      py::arg("volume"), py::arg("count"), py::arg("pitch"), py::arg("unit"),
      py::arg("axis"), py::return_value_policy::reference_internal);

    binding.def(
      "linear_repeater",
      [attachments](std::shared_ptr<Parent> const &parent,
                    std::shared_ptr<Child> const &child, py::handle count,
                    py::handle pitch, std::string const &unit) -> Parent & {
        auto const counts = ParseRepeatCounts(count);
        auto const pitches = ParseRepeatPitches(pitch, unit);
        attachments->LinearRepeater(parent, child, counts, pitches);
        return *parent;
      },
      R"doc(Capture a centered Cartesian arrangement of subtree copies.

Count and pitch must be built-in tuples of exactly three entries ordered X, Y,
Z in the mother's local frame. No axis argument is accepted. Counts (nx, ny, nz)
produce nx*ny*nz direct sibling roots without Row, Plane, or other intermediate
volumes. X varies fastest: flat = x + nx*(y + ny*z). Each root is followed by its
captured descendants before the next root. Every physical copy has its own ID.

The source position is the center of the entire arrangement. For each axis,
copy i has nominal coordinate p + (i - (n - 1)/2)*pitch; all copies retain the
source orientation. Fractional-picometer offsets truncate toward zero before
addition to the source position. A default source position centers
the arrangement at the mother's origin. Singleton axes preserve their source
coordinate; their pitches must still be finite, nonnegative, and representable.
Axes with multiple copies require a strictly positive converted pitch.

The complete child subtree is captured now and remains independent of later
source edits. The detached mother's pose may be set before or after capturing
its children, until that mother is itself captured or attached. A registered
mother extends its original World using its stored pose. Repeated sources do
not become live occurrence handles. Self-use and reuse of a source registered
in a live World are rejected.

World expansion requires configured size and material and admits the complete
batch atomically under existing depth, capacity, FP32, containment, and overlap
rules. Bounded overlap sampling is not a complete geometric proof. Expansion
materializes ordinary occurrences with linear device candidate scans; it does
not provide large-scene spatial acceleration.

Args:
    volume: Complete analytical reference volume and captured descendants.
    count: Tuple of three positive integers, excluding Booleans.
    pitch: Tuple of three finite nonnegative center-to-center spacings.
    unit: Explicit GGEMS length unit shared by all pitch entries.

Returns:
    This parent for chaining.
)doc",
      py::arg("volume"), py::arg("count"), py::arg("pitch"), py::arg("unit"),
      py::return_value_policy::reference_internal);

    binding.def(
      "circular_repeater",
      [attachments](std::shared_ptr<Parent> const &parent,
                    std::shared_ptr<Child> const &child, py::handle count,
                    std::string const &axis) -> Parent & {
        auto const copies = ParseRepeatCount(count);
        auto const repeat_axis = ParseRepeatAxis(axis);
        attachments->CircularRepeater(parent, child, copies, repeat_axis);
        return *parent;
      },
      R"doc(Capture rigid subtree copies around a mother-local axis through its origin.

Count includes occurrence zero at the source pose. Occurrence i rotates both
source position and orientation by the positive right-handed angle 2*pi*i/count.
No final 360-degree duplicate is added. The source pose determines radius and
phase; a zero-radius placement with multiple copies is rejected.

The complete child subtree is captured now. Later source edits do not alter the
recipe. The detached mother's final pose composes with every occurrence when
attached to World. A uniquely registered mother instead extends its original
World using the already captured parent pose. Repeated sources are not handles
for individual occurrences. Direct self-use and reuse of a source registered in
a live World are rejected.

World expansion requires configured size and material. Existing depth, capacity,
FP32, containment, and overlap admission apply atomically to the expanded batch.
Bounded overlap checks are not an exhaustive geometric proof. Expansion uses
ordinary physical VolumeIDs, including separate IDs for equal materials, and
linear device candidate scans. Large-scene acceleration is not implemented.

Args:
    volume: Complete analytical reference volume and captured descendants.
    count: Positive integer total count, excluding Booleans.
    axis: Exactly "X", "Y", or "Z" in the mother's local frame.

Returns:
    This parent for chaining.
)doc",
      py::arg("volume"), py::arg("count"), py::arg("axis"),
      py::return_value_policy::reference_internal);
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
 * Volume classes are registered before World so that add_volume() exposes the
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
  using ggems::geometry::GGEMSVoxelizedVolume;
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

      .def("set_rotation", &SetRotation<GGEMSBox>,
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

      .def("set_position", &SetPosition<GGEMSBox>,
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

      .def("set_material", &SetMaterial<GGEMSBox>,
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
           py::arg("material_name"),
           py::return_value_policy::reference_internal);

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
      .def("set_position", &SetPosition<GGEMSSphere>,
           R"doc(Set the geometric center in the selected parent frame using a

GGEMS length unit.

Returns:
    GGEMSSphere: This Sphere, allowing chained configuration.
)doc",
           py::arg("x"), py::arg("y"), py::arg("z"), py::arg("unit") = "mm",
           py::return_value_policy::reference_internal)

      .def(
        "set_rotation", &SetRotation<GGEMSSphere>,
        R"doc(Set the local-to-parent daughter frame with active Rz * Ry * Rx rotation.

The spherical boundary is rotation invariant. Angles use a GGEMS angular unit;
this orientation determines how positions and rotations of daughters compose.
Return this Sphere for chaining.
)doc",
        py::arg("x"), py::arg("y"), py::arg("z"), py::arg("unit") = "deg",
        py::return_value_policy::reference_internal)

      .def(
        "set_material", &SetMaterial<GGEMSSphere>,
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

      .def("set_position", &SetPosition<GGEMSTubs>,
           "Set the parent-frame geometric center and return this tube.",
           py::arg("x"), py::arg("y"), py::arg("z"), py::arg("unit") = "mm",
           py::return_value_policy::reference_internal)

      .def(
        "set_rotation", &SetRotation<GGEMSTubs>,
        R"doc(Assign the absolute active rotation Rz * Ry * Rx about the center.

Positive angles follow the right-hand rule about fixed X, Y, then Z axes.
Axes belong to the parent selected by add_volume(), or World by default.
Calls replace the previous rotation and return this tube.
)doc",
        py::arg("rx"), py::arg("ry"), py::arg("rz"), py::arg("unit") = "deg",
        py::return_value_policy::reference_internal)

      .def(
        "set_material", &SetMaterial<GGEMSTubs>,
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

      .def("set_position", &SetPosition<GGEMSEllipsoid>,
           "Set the parent-frame geometric center and return this ellipsoid.",
           py::arg("x"), py::arg("y"), py::arg("z"), py::arg("unit") = "mm",
           py::return_value_policy::reference_internal)

      .def(
        "set_rotation", &SetRotation<GGEMSEllipsoid>,
        R"doc(Assign the absolute active rotation Rz * Ry * Rx about the center.

Positive angles follow the right-hand rule about fixed X, Y, then Z axes.
Axes belong to the parent selected by add_volume(), or World by default.
Calls replace the previous rotation and return this ellipsoid.
)doc",
        py::arg("rx"), py::arg("ry"), py::arg("rz"), py::arg("unit") = "deg",
        py::return_value_policy::reference_internal)

      .def(
        "set_material", &SetMaterial<GGEMSEllipsoid>,
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

      .def("set_position", &SetPosition<GGEMSTrd>,
           "Set the parent-frame position of the local frame origin and return "
           "this solid.",
           py::arg("x"), py::arg("y"), py::arg("z"), py::arg("unit") = "mm",
           py::return_value_policy::reference_internal)

      .def(
        "set_rotation", &SetRotation<GGEMSTrd>,
        R"doc(Replace the absolute right-handed fixed-axis rotation Rz * Ry * Rx.

Return this solid. Rotation acts about the local frame origin using parent axes
selected by add_volume(), or World axes by default.
)doc",
        py::arg("rx"), py::arg("ry"), py::arg("rz"), py::arg("unit") = "deg",
        py::return_value_policy::reference_internal)

      .def(
        "set_material", &SetMaterial<GGEMSTrd>,
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

      .def("set_position", &SetPosition<GGEMSTrap>,
           "Set the parent-frame position of the local frame origin and return "
           "this solid.",
           py::arg("x"), py::arg("y"), py::arg("z"), py::arg("unit") = "mm",
           py::return_value_policy::reference_internal)

      .def(
        "set_rotation", &SetRotation<GGEMSTrap>,
        R"doc(Replace the absolute right-handed fixed-axis rotation Rz * Ry * Rx.

Return this solid. Rotation acts about the local frame origin using parent axes
selected by add_volume(), or World axes by default.
)doc",
        py::arg("rx"), py::arg("ry"), py::arg("rz"), py::arg("unit") = "deg",
        py::return_value_policy::reference_internal)

      .def(
        "set_material", &SetMaterial<GGEMSTrap>,
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

      .def("set_position", &SetPosition<GGEMSCons>,
           "Set the parent-frame geometric center and return this frustum.",
           py::arg("x"), py::arg("y"), py::arg("z"), py::arg("unit") = "mm",
           py::return_value_policy::reference_internal)

      .def(
        "set_rotation", &SetRotation<GGEMSCons>,
        R"doc(Assign the absolute active rotation Rz * Ry * Rx about the center.

Positive angles follow the right-hand rule about fixed X, Y, then Z axes.
Axes belong to the parent selected by add_volume(), or World by default.
Calls replace the previous rotation and return this frustum.
)doc",
        py::arg("rx"), py::arg("ry"), py::arg("rz"), py::arg("unit") = "deg",
        py::return_value_policy::reference_internal)

      .def(
        "set_material", &SetMaterial<GGEMSCons>,
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

      .def("set_position", &SetPosition<GGEMSPolyhedra>,
           "Set the parent-frame position of the local frame origin and return "
           "this solid.",
           py::arg("x"), py::arg("y"), py::arg("z"), py::arg("unit") = "mm",
           py::return_value_policy::reference_internal)

      .def(
        "set_rotation", &SetRotation<GGEMSPolyhedra>,
        R"doc(Assign the absolute active rotation Rz * Ry * Rx about the local frame origin.

Positive angles follow the right-hand rule about fixed X, Y, then Z axes.
Axes belong to the parent selected by add_volume(), or World by default.
Calls replace the previous rotation and return this solid.
)doc",
        py::arg("rx"), py::arg("ry"), py::arg("rz"), py::arg("unit") = "deg",
        py::return_value_policy::reference_internal)

      .def(
        "set_material", &SetMaterial<GGEMSPolyhedra>,
        R"doc(Set the medium by exact registered, built-in, or custom material name.

The material becomes registered as used by the scene. Return this solid.
)doc",
        py::arg("name"), py::return_value_policy::reference_internal);

  // === === ===
  py::class_<GGEMSVoxelizedVolume, std::shared_ptr<GGEMSVoxelizedVolume>>(
    module, "GGEMSVoxelizedVolume",
    R"doc(Import one host voxelized volume from MetaImage and material files.

The external RAW payload is little-endian MET_USHORT with X varying fastest.
Its path is relative to the MHD file. The material file maps original labels
to existing GGEMS materials. The grid is centered on its local origin; its
full size is DimSize * ElementSpacing, with spacing interpreted in millimeters.

Only direct World attachment is supported. This volume has no analytical
children, repeaters, or single global material. Its display color defaults to
ggems.colors.white_white. OpenCL Navigation and voxel rendering are not yet
implemented.
)doc")
    .def(py::init<std::filesystem::path const &,
                  std::filesystem::path const &>(),
         py::arg("mhd"), py::arg("materials"))

    .def("set_position", &SetPosition<GGEMSVoxelizedVolume>,
         R"doc(Set the World-frame position of the grid center.

Args:
    x: World X coordinate of the center.
    y: World Y coordinate of the center.
    z: World Z coordinate of the center.
    unit: GGEMS length unit.

Returns:
    This voxelized volume for chained configuration.
)doc",
         py::arg("x"), py::arg("y"), py::arg("z"), py::arg("unit") = "mm",
         py::return_value_policy::reference_internal)

    .def("set_rotation", &SetRotation<GGEMSVoxelizedVolume>,
         R"doc(Assign the absolute right-handed active rotation Rz * Ry * Rx.

The grid rotates about its center. Fixed World-axis X, Y, then Z rotations
replace the previous rotation; positive angles follow the right-hand rule.

Args:
    rx: Rotation about World X.
    ry: Rotation about World Y.
    rz: Rotation about World Z.
    unit: Angle unit, "deg" or "rad".

Returns:
    This voxelized volume for chained configuration.
)doc",
         py::arg("rx"), py::arg("ry"), py::arg("rz"), py::arg("unit") = "deg",
         py::return_value_policy::reference_internal);

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

Physical occurrences may use any of the eight supported analytical families.
A GGEMSVoxelizedVolume can also be captured directly under World as one host-only
physical occurrence; voxel Navigation and visualization remain unsupported.
Detached analytical mothers capture ordinary children and repeater recipes
by value. Configure the mother pose before or after its children, then attach
its complete subtree with world.add_volume(mother). Later source edits never
change captured copies. Position and rotation remain parent-local. Physical
depth is limited to four below World, and Sphere rotation defines its daughter
frame without changing the spherical surface. Scalar and Cartesian linear
repeaters center their copies about the child position and add no hierarchy
level. Circular repeaters retain their occurrence-zero angular convention.

Explicit ordinary attachment also retains top-down assembly: subsequent
mother.add_volume(child) extends the original live World using the stored mother
pose. Descendant sources captured bottom-up and repeater sources have no live
occurrence handle. run.set_world(world) copies the completed scene independently.

All World attachments and repeater batches require configured size and material;
every attachment is atomic, including a single leaf. Invalid geometry is
rejected without publishing a partial batch. Children must fit parent and World
support; sibling interiors must be disjoint. Gaps and surface contact are
allowed subject to existing FP32 admission. Bounded geometric samples reject
detected violations but are not an exhaustive overlap proof. Sub-band gaps may
behave as contact. Expanded repeaters use the existing ordinary OpenCL path;
they do not provide large-scene spatial acceleration.

Configuration methods return the same object, so calls can be chained.

Example:
    world = ggems.geometry.GGEMSWorld()
    world.set_size(2.0, 2.0, 2.0, "m")
    world.set_material("Air")
    mother.add_volume(daughter)
    mother.set_position(10.0, 0.0, 0.0, "mm")
    world.add_volume(mother)
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

      .def("set_material", &SetMaterial<GGEMSWorld>,
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
           py::arg("material_name"),
           py::return_value_policy::reference_internal);

  // === === ===
  auto const bind_color =
    []<typename Shape>(
      py::class_<Shape, std::shared_ptr<Shape>> &binding) -> void {
    binding.def(
      "set_color", &ggems::geometry::GGEMSVolumeAuthoring::SetColor<Shape>,
      R"doc(Set the volume's visualization color from the GGEMS normal palette.

The default is ggems.colors.white_white. Color is independent of the material
and is used in Wireframe and Solid modes. Child captures and repeaters retain
the model's color at attachment; later setters do not recolor captured copies.
Selection temporarily uses alpha 0.5 while preserving this color.

Args:
    color: A typed ggems.colors.ColorKey, such as ggems.colors.magenta_blueviolet.

Returns:
    This analytical volume for chaining.
)doc",
      py::arg("color"), py::return_value_policy::reference_internal);
  };

  bind_color(box_binding);
  bind_color(sphere_binding);
  bind_color(tubs_binding);
  bind_color(ellipsoid_binding);
  bind_color(trd_binding);
  bind_color(trap_binding);
  bind_color(cons_binding);
  bind_color(polyhedra_binding);

  // === === ===
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

  world_binding.def(
    "add_volume",
    [attachments](std::shared_ptr<GGEMSWorld> const &world,
                  std::shared_ptr<GGEMSVoxelizedVolume> const &volume)
      -> GGEMSWorld & {
      attachments->AddVolume(world, volume);
      return *world;
    },
    R"doc(Capture one voxelized volume directly under World by value.

Configure World size and material before attachment. The imported labels,
material palette, and current pose become an independent World snapshot.
Later source setters do not change that occurrence. Reusing an object already
registered in a live World is rejected. A failed attachment leaves World and
registrations unchanged.

Args:
    volume: Imported GGEMSVoxelizedVolume to capture as one physical occurrence.

Returns:
    This World for chaining.
)doc",
    py::arg("volume"), py::return_value_policy::reference_internal);
}
