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
 * \brief Defines owned wireframe presentation data derived from a real World.
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

/*! \brief Stores one local wireframe position in meters. */
struct GGEMSGeometryVertex {
  /*! \brief Local X coordinate in meters. */
  float x;

  /*! \brief Local Y coordinate in meters. */
  float y;

  /*! \brief Local Z coordinate in meters. */
  float z;
};

/*! \brief Selects a shared line-list range in the scene vertex array. */
struct GGEMSGeometryMesh {
  /*! \brief Index of the first line endpoint. */
  std::uint32_t first_vertex;

  /*! \brief Number of line endpoints, always even. */
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

  /*! \brief Index of the shared line mesh. */
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

  /*! \brief Minimum World-meter coordinates of this occurrence's wire mesh. */
  std::array<double, 3> minimum_m{};

  /*! \brief Maximum World-meter coordinates of this occurrence's wire mesh. */
  std::array<double, 3> maximum_m{};

  /*! \brief Direct physical child VolumeIDs in snapshot order. */
  std::vector<std::uint32_t> children{};
};

/*! \brief Owns display-only meshes and occurrences, including World first. */
struct GGEMSGeometryScene {
  /*! \brief Packed shared local line endpoints in meters. */
  std::vector<GGEMSGeometryVertex> vertices;

  /*! \brief Shared line-list ranges for exact shape parameters. */
  std::vector<GGEMSGeometryMesh> meshes;

  /*! \brief World followed by every physical occurrence in identity order. */
  std::vector<GGEMSGeometryOccurrence> occurrences;

  /*! \brief World half extents in meters for camera framing and clipping. */
  std::array<double, 3> world_half_extent_m;
};

/*!
 * \brief Builds independent wire meshes from a validated World.
 *
 * Curved surfaces use fixed visual tessellation; physical geometry is
 * unchanged. Equal shape parameters share line topology, independently of pose
 * or Material. Inspection metadata is owned independently of World. Bounds
 * enclose each displayed wire mesh only, including its composed placement;
 * curved bounds follow the existing visual tessellation, not a physics query.
 * \param[in] world Validated Run-owned World, borrowed only during this call.
 * \return Owned presentation data with unchanged physical IDs and parent links.
 */
[[nodiscard]] auto BuildGeometryScene(geometry::GGEMSWorld const &world)
  -> GGEMSGeometryScene;

} // namespace ggems::render
