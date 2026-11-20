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
 * \brief Defines owned presentation data derived from a real World.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include <cstdint>
#include <string>
#include <vector>
#include <memory>

#include "GGEMS/render/GGEMSColorNames.hh"
#include "GGEMS/render/GGEMSColorTypes.hh"
#include "GGEMS/render/GGEMSGeometryAppearance.hh"
#include "GGEMS/units/GGEMSDensityUnits.hh"

namespace ggems::geometry {
class GGEMSWorld;
struct GGEMSMeshData;
} // namespace ggems::geometry

namespace ggems::render {

/*! \brief Identifies the geometry family in the host and Slang ABI. */
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

  /*! \brief Instanced Box cells of three central regular-grid slices. */
  Voxelized = 8U,

  /*! \brief Indexed OBJ or STL geometry. */
  Mesh = 9U,
};

/*!
 * \brief Packs one immutable procedural shape in the 64-byte Slang ABI.
 *
 * Dimensions are meters and angles are radians. Box, Sphere, and Ellipsoid
 * use parameters[0].xyz for half extents or semiaxes. Tubs and Cons use
 * parameters[0] for outer -Z, outer +Z, inner -Z, inner +Z radii and
 * parameters[1].x for half length. Polyhedra uses parameters[0].x for the
 * first-vertex azimuth. Trd and Trap use parameters[0].xyz and
 * parameters[1].xyz for each end's half height and two half widths,
 * parameters[0].w for half length, and parameters[2] for the two skews,
 * inclination theta, and inclination phi. Voxelized uses header.yzw for grid
 * dimensions and parameters[0].xyz for full cell spacing. Mesh uses header.y
 * for its shared source asset index before Vulkan packs the buffer offsets.
 */
struct GGEMSGeometryShape {
  /*! \brief Family and analytic topology, or family and voxel X/Y/Z counts. */
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

  /*! \brief Number of triangle vertices in a complete draw. */
  std::uint32_t vertex_count;
};

/*! \brief Owns one labeled geometry fact formatted with GGEMS units. */
struct GGEMSGeometryProperty {
  /*! \brief Inspection label, including its local axis when relevant. */
  std::string name;

  /*! \brief Human-readable value with its canonical unit when applicable. */
  std::string value;
};

/*! \brief Owns the original identity and initial appearance of one mapping. */
struct GGEMSGeometryVoxelMapping {
  /*! \brief Inclusive original anatomical-label bound. */
  std::uint32_t first_label;

  /*! \brief Exclusive original bound; 65536 includes label 65535. */
  std::uint32_t end_label;

  /*! \brief Authored Material name, independent of scientific deduplication. */
  std::string material_name;

  /*! \brief Initial imported mode; viewer edits are stored separately. */
  GGEMSGeometryDisplayMode mode;

  /*! \brief Retained mapping or Material default color. */
  ColorKey color;
};

/*! \brief Describes one central-plane draw for one exact mapping row. */
struct GGEMSGeometryVoxelBatch {
  /*! \brief Central plane: zero for XY, one for XZ, two for YZ. */
  std::uint32_t plane;

  /*! \brief First index in the scene's immutable central-cell stream. */
  std::uint32_t first_instance;

  /*! \brief Number of candidate cells, including initially hidden mappings. */
  std::uint32_t instance_count;

  /*! \brief Initial imported mode; None batches are skipped when drawing. */
  GGEMSGeometryDisplayMode mode;

  /*! \brief Resolved mapping override or Material color. */
  ColorKey color;

  /*! \brief Exact index in voxel_slices.mappings, never a MaterialID. */
  std::uint32_t mapping_index{0U};
};

/*! \brief Owns compact regular-grid inspection and central-slice draw data. */
struct GGEMSGeometryVoxelSlices {
  /*! \brief Local X, Y, Z cell counts; each central index is its count / 2. */
  std::array<std::uint32_t, 3> dimensions;

  /*! \brief Full cell sizes in local meters, preserving anisotropy. */
  std::array<double, 3> spacing_m;

  /*! \brief Full physical grid sizes in local meters, before render rounding.
   */
  std::array<double, 3> size_m;

  /*!
   * \brief Full cell counts for XY, XZ, and YZ before hiding mapped None
   * cells.
   */
  std::array<std::uint32_t, 3> instance_counts;

  /*! \brief Number of actual local Material identities, not anatomical labels.
   */
  std::size_t palette_size;

  /*!
   * \brief Immutable batches for all mapped cells of the three central planes.
   */
  std::vector<GGEMSGeometryVoxelBatch> batches;

  /*! \brief Owned mapping rows in original-label order, including None. */
  std::vector<GGEMSGeometryVoxelMapping> mappings{};
};

/*! \brief Describes one nonempty drawable range in the source triangle array.
 */
struct GGEMSGeometryMeshRange {
  /*! \brief First source triangle, independent of occurrence pose. */
  std::uint32_t first_triangle;

  /*! \brief Number of consecutive source triangles in this range. */
  std::uint32_t triangle_count;

  /*! \brief Original OBJ declaration index, absent for anonymous triangles. */
  std::optional<std::uint32_t> object_index;

  /*! \brief Absolute component translation in local meters. */
  std::array<double, 3> translation_m;

  /*! \brief Conservative posed minimum in World meters. */
  std::array<double, 3> minimum_m;

  /*! \brief Conservative posed maximum in World meters. */
  std::array<double, 3> maximum_m;
};

/*! \brief Captures the drawable ranges of one indexed mesh occurrence. */
struct GGEMSGeometryIndexedMesh {
  /*! \brief Index of the immutable import in the scene's mesh assets. */
  std::uint32_t asset_index;

  /*! \brief All drawable triangle ranges in source order. */
  std::vector<GGEMSGeometryMeshRange> ranges;

  /*! \brief True when at least one range belongs to a named OBJ object. */
  bool has_objects{};
};

/*!
 * \brief Retains one physical identity, placement, and owned inspection data.
 */
struct GGEMSGeometryOccurrence {
  /*! \brief Snapshot-local physical VolumeID; zero identifies World. */
  std::uint32_t volume_id;

  /*! \brief Physical parent VolumeID; zero selects World. */
  std::uint32_t parent_volume_id;

  /*! \brief Index of the procedural descriptor and per-instance draw count. */
  std::uint32_t mesh_index;

  /*! \brief Row-major local-to-World matrix, with translations in meters. */
  std::array<float, 16> local_to_world;

  /*! \brief Actual presentation family, or GGEMSWorld for the root. */
  std::string shape_name;

  /*! \brief Canonical shape parameters formatted once for inspection. */
  std::vector<GGEMSGeometryProperty> dimensions;

  /*! \brief Captured model display color; World retains white. */
  ColorKey color{WHITE_White};

  /*! \brief Composed local origin in World meters before render rounding. */
  std::array<double, 3> position_m{};

  /*! \brief Row-major composed local-to-World rotation before render rounding.
   */
  std::array<double, 9> orientation{};

  /*! \brief Actual homogeneous Material name; unused for voxelized grids. */
  std::string material_name{};

  /*! \brief Homogeneous density; unused when voxel_slices is present. */
  units::Density material_density{};

  /*!
   * \brief Conservative physical minimum in World meters, excluding
   * children.
   */
  std::array<double, 3> minimum_m{};

  /*!
   * \brief Conservative physical maximum in World meters, excluding children.
   */
  std::array<double, 3> maximum_m{};

  /*! \brief Direct physical child VolumeIDs in snapshot order. */
  std::vector<std::uint32_t> children{};

  /*!
   * \brief Present only for a voxelized occurrence, which has a Material
   * palette instead of one homogeneous Material or density.
   */
  std::optional<GGEMSGeometryVoxelSlices> voxel_slices{std::nullopt};

  /*! \brief Per-occurrence indexed mesh ranges, present for OBJ and STL. */
  std::optional<GGEMSGeometryIndexedMesh> indexed_mesh{};

  /*!
   * \brief Exact host-authored repeater instance, or zero for an ordinary root.
   *
   * Membership covers repeated roots only, independently of descendants and
   * other instances of a nested repeater. It has no scientific visibility role.
   */
  std::uint32_t repeater_group_id{0U};

  /*!
   * \brief World-local authored repeater identity, or zero for an ordinary
   * root.
   *
   * Shared by roots from the same immutable recipe across physical parents.
   * This host-only provenance controls independent viewer assignments.
   */
  std::uint32_t repeater_recipe_id{0U};

  /*! \brief Authored display name; empty selects the generic physical label. */
  std::string name{};

  /*! \brief Snapshot-local authored identity, independent of name text. */
  std::uint32_t authored_group_id{0U};

  /*! \brief Named definition group, or absent for an unnamed analytic value. */
  std::optional<std::uint32_t> named_group{};

  /*! \brief Reverse group memberships for incremental mixed-state counts. */
  std::vector<std::uint32_t> controlling_groups{};

  /*!
   * \brief Direct children grouped once by exact local repeater identity.
   * Single ordinary children form singleton branches; disclosure has no mode.
   */
  std::vector<std::vector<std::uint32_t>> child_branches{};
};

/*! \brief Owns one named group and its precomputed command membership. */
struct GGEMSGeometryGroup {
  /*! \brief Authored text, or the generic voxel fallback. */
  std::string name;

  /*! \brief True for a named analytic World child controlling its subtree. */
  bool subtree{false};

  /*! \brief All physical root rows have uniform height with no disclosures. */
  bool physical_leaves{false};

  /*! \brief Physical IDs changed by one group command, prepared once. */
  std::vector<std::uint32_t> members;

  /*! \brief Physical occurrences of this exact authored definition. */
  std::vector<std::uint32_t> physical_roots;

  /*! \brief Nested named definitions exposed directly below a subtree group. */
  std::vector<std::uint32_t> child_groups;

  /*! \brief Owned facts from actual members and their captured root recipe. */
  std::vector<GGEMSGeometryProperty> inspection;
};

/*! \brief Preserves top-level presentation order without idle deduplication. */
struct GGEMSGeometryRootEntry {
  /*! \brief Present for a named or generic voxel group header. */
  std::optional<std::uint32_t> group_index;

  /*! \brief Unnamed physical branch, empty when group_index is present. */
  std::vector<std::uint32_t> volumes;
};

/*! \brief Owns display data and physical occurrences, including World. */
struct GGEMSGeometryScene {
  /*! \brief Shared authored polygon profiles; no CPU tessellation is stored. */
  std::vector<GGEMSGeometryProfile> profiles;

  /*! \brief Shared descriptors for exact shape parameters. */
  std::vector<GGEMSGeometryMesh> meshes;

  /*! \brief World followed by every physical occurrence in identity order. */
  std::vector<GGEMSGeometryOccurrence> occurrences;

  /*! \brief All mapped central-plane indices, including None; no 3D field. */
  std::vector<std::uint32_t> voxel_cells{};

  /*! \brief World half extents in meters for camera framing and clipping. */
  std::array<double, 3> world_half_extent_m;

  /*! \brief Named presentation groups, independent of physical identity. */
  std::vector<GGEMSGeometryGroup> groups{};

  /*! \brief Cached named and unnamed roots in physical insertion order. */
  std::vector<GGEMSGeometryRootEntry> root_entries{};

  /*! \brief Retains each source import once across independent placements. */
  std::vector<std::shared_ptr<geometry::GGEMSMeshData const>> mesh_assets{};
};

/*!
 * \brief Captures display data from the current host World.
 *
 * Slang builds procedural triangles and reads indexed mesh triangles.
 * Wireframe and solid modes share that topology. Equal shape parameters share
 * descriptors independently of pose, Material, or color. Inspection metadata
 * and model colors are owned independently of World. Bounds use conservative
 * analytic envelopes under the render transform, without tessellation or a
 * physics query. World uses a Box descriptor and remains restricted to None or
 * Wireframe by appearance.
 * Voxelized occurrences retain one physical identity and a compact descriptor
 * for the three central cell planes, with compact immutable cell indices and
 * exact mapping rows. Initially hidden cells remain available for later viewer
 * edits. No full 3D labels or per-cell meshes are copied.
 * Their full physical outer bounds, rather than slice bounds, drive framing.
 * Indexed meshes retain their immutable source triangles and capture current
 * object translations and posed bounds independently for each occurrence.
 * \param[in] world Current host World, borrowed only during this call.
 * \return Owned presentation data with unchanged physical IDs and parent links.
 * \throws core::GGEMSRecoverable If a voxel or mesh draw exceeds Vulkan's
 * 32-bit draw domain.
 */
[[nodiscard]] auto BuildGeometryScene(geometry::GGEMSWorld const &world)
  -> GGEMSGeometryScene;

} // namespace ggems::render
