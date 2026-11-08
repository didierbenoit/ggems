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
 * \brief Defines owned analytic presentation data derived from a real World.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "GGEMS/units/GGEMSDensityUnits.hh"

namespace ggems::geometry {
class GGEMSWorld;
}

namespace ggems::render {

/*! \brief Identifies the procedural family in the host and Slang ABI. */
enum class GGEMSGeometryKind : std::uint32_t {
  /*! \brief Centered rectangular solid. */
  Box = 0U,

  /*! \brief Complete sphere. */
  Sphere = 1U,

  /*! \brief Complete tube, optionally hollow. */
  Tubs = 2U,

  /*! \brief Complete conical frustum, optionally hollow or pointed. */
  Cons = 3U,

  /*! \brief Polygonal solid described by an ordered apothem profile. */
  Polyhedra = 4U,

  /*! \brief Complete ellipsoid. */
  Ellipsoid = 5U,

  /*! \brief Trapezoid with rectangular end faces. */
  Trd = 6U,

  /*! \brief General planar trapezoid. */
  Trap = 7U,
};

/*!
 * \brief Packs one immutable analytic shape in the 64-byte Slang ABI.
 *
 * Dimensions are meters and angles are radians. Box, Sphere, and Ellipsoid
 * use parameters[0].xyz for half extents or semiaxes. Tubs and Cons use
 * parameters[0] for outer -Z, outer +Z, inner -Z, inner +Z radii and
 * parameters[1].x for half length. Polyhedra uses parameters[0].x for the
 * first-vertex azimuth. Trd and Trap use parameters[0].xyz and
 * parameters[1].xyz for each end's half height and two half widths,
 * parameters[0].w for half length, and parameters[2] for the two skews,
 * inclination theta, and inclination phi. Unused components are zero.
 */
struct GGEMSGeometryShape {
  /*! \brief Family, azimuth side count, first profile, and profile count. */
  std::array<std::uint32_t, 4> header;

  /*! \brief Family-specific scalar parameters; contains no visual vertices. */
  std::array<std::array<float, 4>, 3> parameters;
};

/*! \brief Stores one authored Polyhedra plane in the 16-byte Slang ABI. */
struct GGEMSGeometryProfile {
  /*! \brief Local Z coordinate in meters. */
  float z_m;

  /*! \brief Inner apothem in meters; zero closes the bore. */
  float inner_apothem_m;

  /*! \brief Outer apothem in meters. */
  float outer_apothem_m;

  /*! \brief Cumulative lateral triangles per sector before this plane. */
  std::uint32_t first_triangle;
};

/*! \brief Owns a shared descriptor and its procedural triangle draw count. */
struct GGEMSGeometryMesh {
  /*! \brief Immutable parameters uploaded independently of the draw count. */
  GGEMSGeometryShape shape;

  /*! \brief Number of generated triangle vertices, always divisible by three.*/
  std::uint32_t vertex_count;
};

/*! \brief Owns one labeled analytic parameter formatted with GGEMS units. */
struct GGEMSGeometryProperty {
  /*! \brief Physical parameter name, including its local axis when relevant. */
  std::string name;

  /*! \brief Human-readable canonical parameter value and unit. */
  std::string value;
};

/*!
 * \brief Retains one physical identity, placement, and owned inspection data.
 */
struct GGEMSGeometryOccurrence {
  /*! \brief Snapshot-local physical VolumeID; zero identifies World. */
  std::uint32_t volume_id;

  /*! \brief Physical parent VolumeID; zero selects World. */
  std::uint32_t parent_volume_id;

  /*! \brief Index of the shared analytic descriptor and draw count. */
  std::uint32_t mesh_index;

  /*! \brief Row-major local-to-World matrix, with translations in meters. */
  std::array<float, 16> local_to_world;

  /*! \brief Actual analytic family, or GGEMSWorld for the root. */
  std::string shape_name;

  /*! \brief Canonical shape parameters formatted once for inspection. */
  std::vector<GGEMSGeometryProperty> dimensions;

  /*! \brief Composed local origin in World meters before render rounding. */
  std::array<double, 3> position_m{};

  /*! \brief Row-major composed local-to-World rotation before render rounding.
   */
  std::array<double, 9> orientation{};

  /*! \brief Owned display name of this occurrence's actual Material. */
  std::string material_name{};

  /*! \brief Actual Material density in the canonical GGEMS representation. */
  units::Density material_density{};

  /*!
   * \brief Conservative analytic minimum in World meters, excluding
   * children.
   */
  std::array<double, 3> minimum_m{};

  /*!
   * \brief Conservative analytic maximum in World meters, excluding children.
   */
  std::array<double, 3> maximum_m{};

  /*! \brief Direct physical child VolumeIDs in snapshot order. */
  std::vector<std::uint32_t> children{};
};

/*! \brief Owns analytic display parameters and occurrences, including World. */
struct GGEMSGeometryScene {
  /*! \brief Shared authored polygon profiles; no CPU tessellation is stored. */
  std::vector<GGEMSGeometryProfile> profiles;

  /*! \brief Shared descriptors for exact shape parameters. */
  std::vector<GGEMSGeometryMesh> meshes;

  /*! \brief World followed by every physical occurrence in identity order. */
  std::vector<GGEMSGeometryOccurrence> occurrences;

  /*! \brief World half extents in meters for camera framing and clipping. */
  std::array<double, 3> world_half_extent_m;
};

/*!
 * \brief Packs independent analytic display descriptors from a validated World.
 *
 * Slang generates all visual triangles and normals from these parameters;
 * wireframe and solid modes share that topology. Equal shape parameters share
 * descriptors independently of pose or Material. Inspection metadata is owned
 * independently of World. Bounds use conservative analytic envelopes under
 * the render transform, without tessellation or a physics query. World uses
 * a Box descriptor and remains restricted to None or Wireframe by appearance.
 * \param[in] world Validated Run-owned World, borrowed only during this call.
 * \return Owned presentation data with unchanged physical IDs and parent links.
 */
[[nodiscard]] auto BuildGeometryScene(geometry::GGEMSWorld const &world)
  -> GGEMSGeometryScene;

} // namespace ggems::render
