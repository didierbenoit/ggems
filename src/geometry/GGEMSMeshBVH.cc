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
 * \brief Builds and packs the static binary tree over imported mesh geometry.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <format>
#include <limits>
#include <numbers>
#include <numeric>
#include <span>
#include <unordered_map>
#include <vector>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSMesh.hh"
#include "GGEMS/geometry/GGEMSMeshBVH.hh"

namespace ggems::geometry {
namespace {

/*! \brief Maximum indexed triangles tested in one device leaf visit. */
constexpr std::size_t k_triangles_per_leaf = 4U;

/*! \brief Tracks the incident triangle and direction of an indexed edge. */
struct EdgeUse {
  /*! \brief First source triangle incident on this undirected edge. */
  std::uint32_t triangle;

  /*! \brief +1 for ascending indices, -1 for descending, zero once paired. */
  int direction;
};

/*!
 * \brief Finds the representative of an edge-connected shell by path halving.
 * \param[in,out] parents Triangle forest, updated with shorter paths.
 * \param[in] triangle Source triangle index.
 * \return Source triangle representing its connected shell.
 */
auto ShellRoot(std::span<std::uint32_t> parents, std::uint32_t triangle)
  -> std::uint32_t {
  while (triangle != parents[triangle]) {
    parents[triangle] = parents[parents[triangle]];
    triangle = parents[triangle];
  }

  return triangle;
}

/*!
 * \brief Pairs oppositely directed indexed edges and connects incident faces.
 * \param[in] data Structurally validated mesh in source triangle order.
 * \return Triangle forest identifying edge-connected shells across OBJ labels.
 * \throws core::GGEMSRecoverable For an open, nonmanifold or misoriented edge.
 */
auto ConnectShells(GGEMSMeshData const &data) -> std::vector<std::uint32_t> {
  std::vector<std::uint32_t> parents(data.triangles.size());
  std::ranges::iota(parents, 0U);
  std::unordered_map<std::uint64_t, EdgeUse> edges;

  for (std::uint32_t index = 0; index < data.triangles.size(); ++index) {
    auto const &triangle = data.triangles[index];
    for (std::size_t corner = 0; corner < triangle.size(); ++corner) {
      auto const start = triangle[corner];
      auto const end = triangle[(corner + 1U) % triangle.size()];
      auto const edge =
        (static_cast<std::uint64_t>(std::min(start, end)) << 32U) |
        std::max(start, end);

      int const direction = start < end ? 1 : -1;

      auto const [entry, inserted] = edges.try_emplace(
        edge, EdgeUse{.triangle = index, .direction = direction});

      if (!inserted) {
        if (entry->second.direction != -direction) {
          throw core::GGEMSRecoverable(std::format(
            "Mesh '{}' has inconsistent winding or nonmanifold edge incidence.",
            data.source_file.string()));
        }
        parents[ShellRoot(parents, index)] =
          ShellRoot(parents, entry->second.triangle);
        entry->second.direction = 0;
      }
    }
  }

  if (std::ranges::any_of(edges, [](auto const &entry) -> bool {
        return entry.second.direction != 0;
      })) {
    throw core::GGEMSRecoverable(std::format(
      "Mesh '{}' has an open indexed edge.", data.source_file.string()));
  }

  return parents;
}

/*! \brief One source triangle, or the already-built root of one component. */
struct PartitionItem {
  /*! \brief Inclusive FP32 local bounds in picometers. */
  GGEMSMeshBounds bounds;

  /*! \brief Source triangle index for a BLAS or a node index for the TLAS. */
  std::uint32_t index;

  /*! \brief Number of internal levels below a prepared component root. */
  std::uint32_t depth{};
};

/*!
 * \brief Merges extrema of already-rounded local device coordinates.
 * \param[in] result First inclusive local bounds in pm.
 * \param[in] other Second inclusive local bounds in pm.
 * \return Bounds enclosing both inputs.
 */
auto UnionBounds(GGEMSMeshBounds result, GGEMSMeshBounds const &other)
  -> GGEMSMeshBounds {
  for (std::size_t axis = 0; axis < result.minimum.size(); ++axis) {
    result.minimum[axis] = std::min(result.minimum[axis], other.minimum[axis]);
    result.maximum[axis] = std::max(result.maximum[axis], other.maximum[axis]);
  }

  return result;
}

/*! \brief Collects the signed volume and bounds of an edge-connected shell. */
struct ShellGeometry {
  /*! \brief Inclusive FP32 extrema of the shell vertices in local pm. */
  GGEMSMeshBounds bounds;

  /*! \brief First source triangle belonging to this shell. */
  std::uint32_t first;

  /*! \brief Last source triangle belonging to this shell, inclusive. */
  std::uint32_t last;

  /*! \brief Six times the oriented volume accumulated in cubic picometers. */
  double six_volume{};
};

/*!
 * \brief Measures the oriented solid angle subtended by one triangle.
 * \param[in] data Imported local mesh geometry in picometers.
 * \param[in] triangle Index of the triangle observed from the witness point.
 * \param[in] witness Vertex of the proposed cavity in local picometers.
 * \return Signed solid angle in steradians, or NaN at a coincident vertex.
 */
auto TriangleSolidAngle(GGEMSMeshData const &data, std::uint32_t triangle,
                        std::array<float, 3> const &witness) -> double {
  std::array<detail::NormalizedVector3D, 3> directions{};
  for (std::size_t i = 0; i < directions.size(); ++i) {
    auto const &vertex = data.vertices_pm[data.triangles[triangle][i]];
    auto const unit =
      detail::TryNormalizeVector3D(static_cast<double>(vertex[0]) - witness[0],
                                   static_cast<double>(vertex[1]) - witness[1],
                                   static_cast<double>(vertex[2]) - witness[2]);
    if (!unit) {
      return std::numeric_limits<double>::quiet_NaN();
    }

    directions[i] = *unit;
  }

  auto const &[a, b, c] = directions;
  auto const cross = detail::Cross3D(b, c);

  double const determinant =
    (a.x * cross[0]) + (a.y * cross[1]) + (a.z * cross[2]);

  return 2.0 *
         std::atan2(determinant, 1.0 + detail::Dot3D(a, b) +
                                   detail::Dot3D(b, c) + detail::Dot3D(c, a));
}

/*!
 * \brief Requires an outward enclosing shell for each inward cavity witness.
 *
 * An outward shell subtends 4 pi steradians at an interior point and zero
 * outside. One witness per possible cavity assumes disjoint shell surfaces.
 * \param[in] data Structurally validated mesh in source triangle order.
 * \param[in,out] parents Edge-connected triangle forest with path compression.
 * \throws core::GGEMSRecoverable For zero volume or an exposed inward shell.
 */
auto CheckShellOrientation(GGEMSMeshData const &data,
                           std::span<std::uint32_t> parents) -> void {
  std::unordered_map<std::uint32_t, ShellGeometry> shells;

  for (std::uint32_t index = 0; index < data.triangles.size(); ++index) {
    auto const root = ShellRoot(parents, index);
    parents[index] = root;
    auto const &origin = data.vertices_pm[data.triangles[root][0]];
    ShellGeometry const initial{
      .bounds = {.minimum = origin, .maximum = origin},
      .first = index,
      .last = index,
    };

    auto &shell = shells.try_emplace(root, initial).first->second;
    shell.last = index;

    auto const &triangle = data.triangles[index];
    for (auto corner : triangle) {
      auto const &point = data.vertices_pm[corner];
      shell.bounds =
        UnionBounds(shell.bounds, {.minimum = point, .maximum = point});
    }

    auto const &a = data.vertices_pm[triangle[0]];
    auto const &b = data.vertices_pm[triangle[1]];
    auto const &c = data.vertices_pm[triangle[2]];
    double const x0 = origin[0];
    // Divergence theorem with F=(x-x0,0,0). The triangle mean of x is exact.
    double const x_sum = (a[0] - x0) + (b[0] - x0) + (c[0] - x0);
    double const ab_y = static_cast<double>(b[1]) - a[1];
    double const ab_z = static_cast<double>(b[2]) - a[2];
    double const ac_y = static_cast<double>(c[1]) - a[1];
    double const ac_z = static_cast<double>(c[2]) - a[2];
    shell.six_volume += x_sum * ((ab_y * ac_z) - (ab_z * ac_y));
  }

  for (auto const &[root, shell] : shells) {
    if (shell.six_volume > 0.0) {
      continue;
    }

    auto const &witness = data.vertices_pm[data.triangles[root][0]];
    auto const enclosing = [&](auto const &entry) -> bool {
      auto const &outer = entry.second;
      auto const bounds = UnionBounds(outer.bounds, shell.bounds);

      if (outer.six_volume <= 0.0 || bounds.minimum != outer.bounds.minimum ||
          bounds.maximum != outer.bounds.maximum) {
        return false;
      }

      double angle = 0.0;
      for (auto index = outer.first; index <= outer.last; ++index) {
        if (parents[index] == entry.first) {
          angle += TriangleSolidAngle(data, index, witness);
        }
      }

      return angle > 2.0 * std::numbers::pi;
    };

    if (shell.six_volume == 0.0 || std::ranges::none_of(shells, enclosing)) {
      throw core::GGEMSRecoverable(std::format(
        "Mesh '{}' has a zero-volume or inward exterior shell at triangle {}.",
        data.source_file.string(), root));
    }
  }
}

/*!
 * \brief Computes twice the bound center without rounding it back to FP32.
 * \param[in] item Triangle or subtree bounds.
 * \param[in] axis Local coordinate index.
 * \return Sum of the minimum and maximum coordinates in pm.
 */
auto CenterSum(PartitionItem const &item, std::size_t axis) -> double {
  return static_cast<double>(item.bounds.minimum[axis]) +
         item.bounds.maximum[axis];
}

/*!
 * \brief Selects the widest extent of triangle or subtree bound centers.
 * \param[in] items Nonempty partition of triangles or component roots.
 * \return Axis index, retaining the first axis on an equal extent.
 */
auto SplitAxis(std::span<PartitionItem const> items) -> std::size_t {
  double widest = -1.0;
  std::size_t axis = 0;

  for (std::size_t candidate = 0; candidate < 3; ++candidate) {
    double lower = CenterSum(items.front(), candidate);
    double upper = lower;
    for (auto const &item : items.subspan(1)) {
      double const center = CenterSum(item, candidate);
      lower = std::min(lower, center);
      upper = std::max(upper, center);
    }

    if (upper - lower > widest) {
      widest = upper - lower;
      axis = candidate;
    }
  }

  return axis;
}

/*!
 * \brief Builds a balanced partition, linking component leaves to BLAS roots.
 * \param[in,out] items Nonempty partition reordered around median centers.
 * \param[in] triangles Whether the items refer to individual triangles.
 * \param[in,out] tree Receives nodes and leaf-ordered source triangle indices.
 * \return Node index, local bounds and depth of the partition root.
 */
auto BuildPartition(std::span<PartitionItem> items, bool triangles,
                    GGEMSMeshBVH &tree) -> PartitionItem {
  if (!triangles && items.size() == 1U) {
    return items.front();
  }

  auto const node = static_cast<std::uint32_t>(tree.nodes.size());
  tree.nodes.emplace_back();
  auto bounds = items.front().bounds;
  std::uint32_t first{};
  std::uint32_t second{};
  std::uint32_t depth{};

  if (triangles && items.size() <= k_triangles_per_leaf) {
    std::ranges::sort(items, {}, &PartitionItem::index);
    first = static_cast<std::uint32_t>(tree.triangle_order.size());
    second = k_mesh_leaf_flag | static_cast<std::uint32_t>(items.size());
    for (auto const &item : items) {
      tree.triangle_order.push_back(item.index);
      bounds = UnionBounds(bounds, item.bounds);
    }
  } else {
    auto const axis = SplitAxis(items);
    auto const middle = items.size() / 2U;
    std::ranges::nth_element(
      items, items.begin() + static_cast<std::ptrdiff_t>(middle),
      [axis](PartitionItem const &left, PartitionItem const &right) -> bool {
        auto const a = CenterSum(left, axis);
        auto const b = CenterSum(right, axis);
        return a == b ? left.index < right.index : a < b;
      });
    auto const left = BuildPartition(items.first(middle), triangles, tree);
    auto const right = BuildPartition(items.subspan(middle), triangles, tree);
    first = left.index;
    second = right.index;
    depth = 1U + std::max(left.depth, right.depth);
    bounds = UnionBounds(left.bounds, right.bounds);
  }

  tree.nodes[node] = {
    .minimum = bounds.minimum,
    .first = first,
    .maximum = bounds.maximum,
    .second = second,
  };

  return {.bounds = bounds, .index = node, .depth = depth};
}

} // namespace

// =============================================================================
// =============================================================================

auto BuildMeshBVH(GGEMSMeshData const &data) -> GGEMSMeshBVH {
  GGEMSMeshBVH tree;
  // Check the original triples before narrowing any source index to uint32_t.
  (void)MeshBVHPackedSize(data, tree);
  auto shells = ConnectShells(data);
  CheckShellOrientation(data, shells);
  std::vector<PartitionItem> components;
  std::vector<PartitionItem> triangles;
  tree.triangle_order.reserve(data.triangles.size());

  auto const append_range = [&](std::size_t first, std::size_t count) -> void {
    triangles.clear();
    triangles.reserve(count);

    for (auto index = first; index < first + count; ++index) {
      auto const &vertices = data.triangles[index];
      auto const &origin = data.vertices_pm[vertices[0]];
      GGEMSMeshBounds bounds{.minimum = origin, .maximum = origin};
      for (auto const corner : {vertices[1], vertices[2]}) {
        auto const &point = data.vertices_pm[corner];
        bounds = UnionBounds(bounds, {.minimum = point, .maximum = point});
      }
      triangles.push_back(
        {.bounds = bounds, .index = static_cast<std::uint32_t>(index)});
    }

    components.push_back(BuildPartition(triangles, true, tree));
  };

  std::size_t cursor = 0;
  for (auto const &component : data.components) {
    if (cursor < component.first_triangle) {
      append_range(cursor, component.first_triangle - cursor);
    }

    auto root = std::numeric_limits<std::uint32_t>::max();

    if (component.triangle_count != 0) {
      append_range(component.first_triangle, component.triangle_count);
      root = components.back().index;
    }

    tree.component_roots.push_back(root);
    cursor = component.first_triangle + component.triangle_count;
  }

  if (cursor < data.triangles.size()) {
    append_range(cursor, data.triangles.size() - cursor);
  }

  auto const root = BuildPartition(components, false, tree);
  tree.root_index = root.index;
  tree.depth = root.depth;

  if (tree.depth > k_mesh_stack_capacity) {
    throw core::GGEMSRecoverable(
      "Mesh tree exceeds device traversal stack capacity.");
  }

  (void)MeshBVHPackedSize(data, tree);
  return tree;
}

// =============================================================================
// =============================================================================

auto MeshBVHPackedSize(GGEMSMeshData const &data, GGEMSMeshBVH const &tree)
  -> std::size_t {
  std::uint64_t const bytes =
    sizeof(GGEMSMeshBVHRecord) +
    (static_cast<std::uint64_t>(tree.nodes.size()) * sizeof(GGEMSMeshBVHNode)) +
    (static_cast<std::uint64_t>(data.triangles.size()) *
     sizeof(data.triangles.front())) +
    (static_cast<std::uint64_t>(data.vertices_pm.size()) *
     sizeof(data.vertices_pm.front()));

  if (bytes > std::numeric_limits<std::uint32_t>::max()) {
    throw core::GGEMSRecoverable(
      "Mesh asset exceeds 32-bit device byte offsets.");
  }

  return static_cast<std::size_t>(bytes);
}

// =============================================================================
// =============================================================================

auto PackMeshBVH(GGEMSMeshData const &data, GGEMSMeshBVH const &tree,
                 std::span<std::byte> destination) -> void {
  if (destination.size() < MeshBVHPackedSize(data, tree)) {
    throw core::GGEMSRecoverable(
      "Insufficient destination space for the packed mesh asset.");
  }

  auto const node_bytes = tree.nodes.size() * sizeof(tree.nodes.front());
  auto const triangle_bytes =
    tree.triangle_order.size() * sizeof(data.triangles.front());

  GGEMSMeshBVHRecord const record{
    .node_offset = sizeof(GGEMSMeshBVHRecord),
    .triangle_offset =
      static_cast<std::uint32_t>(sizeof(GGEMSMeshBVHRecord) + node_bytes),
    .vertex_offset = static_cast<std::uint32_t>(sizeof(GGEMSMeshBVHRecord) +
                                                node_bytes + triangle_bytes),
    .root_index = tree.root_index,
  };

  std::memcpy(destination.data(), &record, sizeof(record));
  std::memcpy(destination.data() + record.node_offset, tree.nodes.data(),
              node_bytes);

  auto *next = destination.data() + record.triangle_offset;

  for (auto index : tree.triangle_order) {
    auto const &triangle = data.triangles[index];
    std::memcpy(next, triangle.data(), sizeof(triangle));
    next += sizeof(triangle);
  }

  std::memcpy(destination.data() + record.vertex_offset,
              data.vertices_pm.data(),
              data.vertices_pm.size() * sizeof(data.vertices_pm.front()));
}

} // namespace ggems::geometry
