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
 * \brief Defines static mesh acceleration and its local device byte layout.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace ggems::geometry {

struct GGEMSMeshData;

/*! \brief Locates arrays in one packed mesh asset relative to this record. */
struct GGEMSMeshBVHRecord {
  /*! \brief Byte offset of the first local BVH node. */
  std::uint32_t node_offset;

  /*! \brief Byte offset of the first leaf-ordered vertex index triple. */
  std::uint32_t triangle_offset;

  /*! \brief Byte offset of the first FP32 local vertex triple in pm. */
  std::uint32_t vertex_offset;

  /*! \brief Index of the entry node in this asset's node array. */
  std::uint32_t root_index;
};

/*!
 * \brief Stores two child indices, or a start index and tagged triangle count.
 *
 * All node and triangle indices are local to the asset. Bounds enclose
 * the exact uploaded FP32 coordinates in the mesh frame.
 */
struct GGEMSMeshBVHNode {
  /*! \brief Inclusive lower bounds in FP32 local picometers. */
  std::array<float, 3> minimum;

  /*! \brief Left child index or first triangle index in packed leaf order. */
  std::uint32_t first;

  /*! \brief Inclusive upper bounds in FP32 local picometers. */
  std::array<float, 3> maximum;

  /*! \brief Right child index or leaf triangle count with the high bit set. */
  std::uint32_t second;
};

/*! \brief Distinguishes a count (leaf) from a right child index. */
inline constexpr std::uint32_t k_mesh_leaf_flag = 1U << 31U;

/*! \brief Maximum deferred far children supported by the device traversal. */
inline constexpr std::uint32_t k_mesh_stack_capacity = 32U;

/*! \brief Owns pose-independent partitions of one imported mesh. */
struct GGEMSMeshBVH {
  /*! \brief Nodes of the component and triangle partitions in asset order. */
  std::vector<GGEMSMeshBVHNode> nodes;

  /*! \brief Source indices in triangle leaf order. */
  std::vector<std::uint32_t> triangle_order;

  /*! \brief Root by named OBJ object index, UINT32_MAX for empty objects. */
  std::vector<std::uint32_t> component_roots;

  /*! \brief Edge-connected shell root per triangle for posed shell admission.
   */
  std::vector<std::uint32_t> shell_roots;

  /*! \brief Entry node of the entire mesh. */
  std::uint32_t root_index{};

  /*! \brief Maximum number of internal nodes on any root-to-leaf path. */
  std::uint32_t depth{};
};

/*!
 * \brief Checks shell admission and partitions triangles for navigation.
 * \param[in] data Structurally validated geometry from GGEMSMesh::GetData().
 * \return Owned tree with indices into the supplied geometry.
 * \throws core::GGEMSRecoverable For open or inconsistently oriented edges,
 * impossible shell orientation, or exceeded device capacity.
 */
[[nodiscard]] auto BuildMeshBVH(GGEMSMeshData const &data) -> GGEMSMeshBVH;

/*!
 * \brief Requires an outward enclosing shell for each inward cavity witness.
 *
 * An outward shell subtends 4 pi steradians at an interior point and zero
 * outside. One witness per possible cavity assumes disjoint shell surfaces.
 * \param[in] data Structurally validated mesh in source triangle order.
 * \param[in] roots Edge-connected shell root by source triangle.
 * \param[in] vertices Admitted or translated local coordinates in FP32 pm.
 * \throws core::GGEMSRecoverable For zero volume or an exposed inward shell.
 */
auto ValidateMeshShells(GGEMSMeshData const &data,
                        std::span<std::uint32_t const> roots,
                        std::span<std::array<float, 3> const> vertices) -> void;

/*!
 * \brief Computes the complete device asset size with checked 32-bit offsets.
 * \param[in] data Imported geometry supplying the triangle and vertex arrays.
 * \param[in] tree Partition built for this geometry.
 * \return Byte count including the record and all three arrays.
 * \throws core::GGEMSRecoverable If the asset exceeds the device offset range.
 */
[[nodiscard]] auto MeshBVHPackedSize(GGEMSMeshData const &data,
                                     GGEMSMeshBVH const &tree) -> std::size_t;

/*!
 * \brief Packs the local record, nodes, leaf-ordered triangles and vertices.
 * \param[in] data Imported geometry supplying the triangle and vertex arrays.
 * \param[in] tree Partition built for this geometry.
 * \param[out] destination Writable storage for one complete asset.
 * \param[in] translated Posed FP32 vertices, empty for the source geometry.
 * \throws core::GGEMSRecoverable If the asset or its destination cannot fit.
 */
auto PackMeshBVH(GGEMSMeshData const &data, GGEMSMeshBVH const &tree,
                 std::span<std::byte> destination,
                 std::span<std::array<float, 3> const> translated = {}) -> void;
} // namespace ggems::geometry
