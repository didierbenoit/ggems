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
 * \brief Checks mesh import, source metadata, and detached authoring behavior.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <array>
#include <bit>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
#include <limits>
#include <memory>
#include <random>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSMesh.hh"
#include "GGEMS/geometry/GGEMSMeshBVH.hh"
#include "GGEMS/geometry/GGEMSMeshOccurrenceRecord.hh"
#include "GGEMS/geometry/GGEMSWorld.hh"
#include "GGEMS/logging/GGEMSLogger.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/render/GGEMSColorNames.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"
#include "GGEMS/units/GGEMSUnitFormatting.hh"

namespace {

using ggems::core::GGEMSRecoverable;
using ggems::geometry::GGEMSMesh;
using ggems::geometry::GGEMSMeshFormat;
using namespace ggems::units;

constexpr std::string_view k_triangle_obj =
  "v 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 3\n";

class GGEMSMeshTest : public ::testing::Test {
protected:
  auto SetUp() -> void override {
    root_ =
      std::filesystem::path{::testing::TempDir()} /
      std::format("ggems_mesh_{}_{}",
                  std::chrono::steady_clock::now().time_since_epoch().count(),
                  std::random_device{}());
    std::filesystem::create_directory(root_);
  }

  auto TearDown() -> void override {
    std::error_code ignored;
    std::filesystem::remove_all(root_, ignored);
  }

  [[nodiscard]] auto
  Write(std::string_view contents,
        std::filesystem::path const &filename = "mesh.obj") const
    -> std::filesystem::path {
    auto const path = root_ / filename;
    std::ofstream output;
    output.exceptions(std::ios::failbit | std::ios::badbit);
    output.open(path, std::ios::binary);
    output.write(contents.data(),
                 static_cast<std::streamsize>(contents.size()));
    output.close();
    return path;
  }

private:
  std::filesystem::path root_;
};

/*! \brief Encodes explicit little-endian test words independently of host
 * order. */
auto AppendWord(std::string &bytes, std::uint32_t value) -> void {
  for (unsigned shift = 0; shift < 32U; shift += 8U) {
    bytes.push_back(static_cast<char>((value >> shift) & 0xffU));
  }
}

/*! \brief Creates binary facets with a deceptive solid-prefixed header. */
auto BinarySTL(std::vector<std::array<std::array<float, 3>, 3>> const &facets)
  -> std::string {
  std::string bytes(80, '\0');
  bytes.replace(0, 5, "solid");
  AppendWord(bytes, static_cast<std::uint32_t>(facets.size()));
  for (auto const &facet : facets) {
    // Normals are advisory, including nonfinite source normals.
    AppendWord(bytes, std::bit_cast<std::uint32_t>(
                        std::numeric_limits<float>::quiet_NaN()));
    AppendWord(bytes, 0U);
    AppendWord(bytes, 0U);
    for (auto const &vertex : facet) {
      for (auto const coordinate : vertex) {
        AppendWord(bytes, std::bit_cast<std::uint32_t>(coordinate));
      }
    }
    bytes.append(2, '\0');
  }
  return bytes;
}

class MeshLogSink final : public ggems::core::LogSink {
public:
  explicit MeshLogSink(std::vector<ggems::core::RenderedLogLine> &lines)
      : lines_{lines} {}

  auto Write(ggems::core::RenderedLogLine &&line) -> void override {
    lines_.push_back(std::move(line));
  }

private:
  std::vector<ggems::core::RenderedLogLine> &lines_;
};

struct ClearLogSinks {
  ~ClearLogSinks() { ggems::core::GGEMSLogger::GetInstance().ClearSinks(); }
};

} // namespace

TEST(GGEMSMeshLayout, RecordsMatchPackedDeviceFields) {
  using ggems::geometry::GGEMSMeshBVHNode;
  using ggems::geometry::GGEMSMeshBVHRecord;
  using ggems::geometry::GGEMSMeshOccurrenceRecord;
  EXPECT_EQ(sizeof(GGEMSMeshBVHRecord), 16U);
  EXPECT_EQ(alignof(GGEMSMeshBVHRecord), 4U);
  EXPECT_EQ(offsetof(GGEMSMeshBVHRecord, node_offset), 0U);
  EXPECT_EQ(offsetof(GGEMSMeshBVHRecord, triangle_offset), 4U);
  EXPECT_EQ(offsetof(GGEMSMeshBVHRecord, vertex_offset), 8U);
  EXPECT_EQ(offsetof(GGEMSMeshBVHRecord, root_index), 12U);

  EXPECT_EQ(sizeof(GGEMSMeshBVHNode), 32U);
  EXPECT_EQ(alignof(GGEMSMeshBVHNode), 4U);
  EXPECT_EQ(offsetof(GGEMSMeshBVHNode, minimum), 0U);
  EXPECT_EQ(offsetof(GGEMSMeshBVHNode, first), 12U);
  EXPECT_EQ(offsetof(GGEMSMeshBVHNode, maximum), 16U);
  EXPECT_EQ(offsetof(GGEMSMeshBVHNode, second), 28U);
  EXPECT_EQ(sizeof(std::array<float, 3>), 12U);
  EXPECT_EQ(sizeof(std::array<std::uint32_t, 3>), 12U);

  EXPECT_EQ(sizeof(GGEMSMeshOccurrenceRecord), 60U);
  EXPECT_EQ(alignof(GGEMSMeshOccurrenceRecord), 4U);
  EXPECT_EQ(offsetof(GGEMSMeshOccurrenceRecord, pose), 0U);
  EXPECT_EQ(offsetof(GGEMSMeshOccurrenceRecord, tolerance_pm), 48U);
  EXPECT_EQ(offsetof(GGEMSMeshOccurrenceRecord, material_id), 52U);
  EXPECT_EQ(offsetof(GGEMSMeshOccurrenceRecord, asset_offset), 56U);
}

TEST_F(GGEMSMeshTest,
       OBJPreservesLiteralObjectsGlobalIndicesAndReferencedBounds) {
  auto const path =
    Write("v -1 0 0\nv 2 0 0\nv 0 3 0\nv 0 0 4\nv 999 999 999\n"
          "o A01##label%s\nf 1 2 3\n"
          "o repeated\nf 1/7 3/8 4/9\ng ignored group\nf -5//1 -4//1 -2//1\n"
          "o repeated\nf 1/1/1 2/2/2 3/3/3\no empty\n");
  GGEMSMesh const mesh{"leaves", path, "mm"};
  auto const &data = mesh.GetData();
  EXPECT_EQ(mesh.GetName(), "leaves");
  EXPECT_EQ(data.source_file, path);
  EXPECT_EQ(data.source_unit, "mm");
  EXPECT_EQ(data.format, GGEMSMeshFormat::Obj);
  EXPECT_EQ(data.source_vertex_count, 5U);
  ASSERT_EQ(data.vertices_pm.size(), 5U);
  ASSERT_EQ(data.triangles.size(), 4U);
  EXPECT_EQ(data.triangles[2], (std::array<std::uint32_t, 3>{0, 1, 3}));
  EXPECT_EQ(mesh.GetComponentNames(),
            (std::vector<std::string>{"A01##label%s", "repeated", "repeated",
                                      "empty"}));
  ASSERT_EQ(data.components.size(), 4U);
  EXPECT_EQ(data.components[0].first_triangle, 0U);
  EXPECT_EQ(data.components[1].first_triangle, 1U);
  EXPECT_EQ(data.components[1].triangle_count, 2U);
  EXPECT_EQ(data.components[2].first_triangle, 3U);
  EXPECT_EQ(data.components[3].first_triangle, 4U);
  EXPECT_EQ(data.components[3].triangle_count, 0U);
  EXPECT_FALSE(data.components[3].bounds_pm);
  ASSERT_TRUE(data.components[0].bounds_pm);
  EXPECT_EQ(data.components[0].bounds_pm->maximum[2], 0.0F);
  EXPECT_EQ(data.bounds_pm.minimum, (std::array<float, 3>{-1e9F, 0.0F, 0.0F}));
  EXPECT_EQ(data.bounds_pm.maximum, (std::array<float, 3>{2e9F, 3e9F, 4e9F}));
}

TEST_F(GGEMSMeshTest,
       UnnamedOBJUsesExplicitUnitsWithoutIntegerPositionRounding) {
  auto const path = Write("v 0.25 0 0\nv 1.25 0 0\nv 0.25 1 0\nf +1 +2 +3\n");
  GGEMSMesh const unnamed{path};
  EXPECT_EQ(unnamed.GetData().source_unit, "mm");
  constexpr auto const &literal =
    GGEMS_TEST_PROBE_ROOT "/../navigation/meshes/three_slabs.obj";
  EXPECT_EQ((GGEMSMesh{literal}).GetData().source_unit, "mm");
  EXPECT_EQ((GGEMSMesh{literal, "cm"}).GetData().source_unit, "cm");
  EXPECT_EQ((GGEMSMesh{"leaf", literal, "mm"}).GetName(), "leaf");
  GGEMSMesh const picometers{path, "pm"};
  GGEMSMesh const centimeters{path, "cm"};
  EXPECT_TRUE(picometers.GetName().empty());
  EXPECT_TRUE(picometers.GetComponentNames().empty());
  EXPECT_EQ(picometers.GetData().vertices_pm[0][0], 0.25F);
  EXPECT_EQ(centimeters.GetData().vertices_pm[0][0], 2.5e9F);
  EXPECT_EQ(centimeters.GetData().bounds_pm.maximum[1], 1e10F);
}

TEST_F(GGEMSMeshTest,
       BinarySTLUsesStructuralDetectionAndExactDeterministicIndexing) {
  std::vector<std::array<std::array<float, 3>, 3>> const facets{
    {{{0, 0, 0}, {1, 0, 0}, {0, 1, 0}}},
    {{{0, 1, 0}, {1, 0, 0}, {1, 1, 0}}},
    {{{8, 0, 0}, {9, 0, 0}, {8, 1, 0}}},
  };
  auto const path = Write(BinarySTL(facets), "disconnected.STL");
  GGEMSMesh const first{path, "mm"};
  GGEMSMesh const second{path, "mm"};
  EXPECT_EQ(first.GetData().format, GGEMSMeshFormat::BinarySTL);
  EXPECT_EQ(first.GetData().source_vertex_count, 9U);
  EXPECT_EQ(first.GetData().vertices_pm.size(), 7U);
  ASSERT_EQ(first.GetData().triangles.size(), 3U);
  EXPECT_EQ(first.GetData().triangles[1],
            (std::array<std::uint32_t, 3>{2, 1, 3}));
  EXPECT_EQ(first.GetData().triangles[2],
            (std::array<std::uint32_t, 3>{4, 5, 6}));
  EXPECT_EQ(first.GetData().vertices_pm, second.GetData().vertices_pm);
  EXPECT_EQ(first.GetData().triangles, second.GetData().triangles);
  EXPECT_TRUE(first.GetComponentNames().empty());
  EXPECT_EQ(first.GetData().bounds_pm.maximum[0], 9e9F);
}

TEST_F(GGEMSMeshTest,
       AsciiSTLImportsAllSolidBlocksAndDeduplicatesExactCoordinates) {
  auto const path =
    Write("solid first\nfacet normal 9 9 9\nouter loop\nvertex 0 0 0\n"
          "vertex 1 0 0\nvertex 0 1 0\nendloop\nendfacet\nendsolid first\n"
          "solid second\nfacet normal 0 0 0\nouter loop\nvertex -0 1 0\n"
          "vertex 1 0 0\nvertex 1 1 0\nendloop\nendfacet\nendsolid second\n",
          "mesh.stl");
  GGEMSMesh const mesh{path, "nm"};
  EXPECT_EQ(mesh.GetData().format, GGEMSMeshFormat::AsciiSTL);
  EXPECT_EQ(mesh.GetData().source_vertex_count, 6U);
  EXPECT_EQ(mesh.GetData().vertices_pm.size(), 4U);
  ASSERT_EQ(mesh.GetData().triangles.size(), 2U);
  EXPECT_EQ(mesh.GetData().triangles[1],
            (std::array<std::uint32_t, 3>{2, 1, 3}));
  EXPECT_TRUE(mesh.GetComponentNames().empty());
  EXPECT_EQ(mesh.GetData().bounds_pm.maximum[0], 1000.0F);
}

TEST_F(GGEMSMeshTest,
       STLDoesNotWeldDistinctSourceCoordinatesAfterFP32Conversion) {
  auto const path =
    Write("solid\nfacet normal 0 0 1\nouter loop\nvertex 0 0 1\n"
          "vertex 1 0 1\nvertex 0 1 1\nendloop\nendfacet\n"
          "facet normal 0 0 1\nouter loop\nvertex 0 0 1.00000001\n"
          "vertex 1 0 1.00000001\nvertex 0 1 "
          "1.00000001\nendloop\nendfacet\nendsolid\n",
          "mesh.stl");
  GGEMSMesh const mesh{path, "pm"};
  ASSERT_EQ(mesh.GetData().vertices_pm.size(), 6U);
  EXPECT_EQ(mesh.GetData().vertices_pm[0], mesh.GetData().vertices_pm[3]);
  EXPECT_EQ(mesh.GetData().triangles[1],
            (std::array<std::uint32_t, 3>{3, 4, 5}));
}

TEST_F(GGEMSMeshTest,
       CopiesShareGeometryAndKeepIndependentPoseMaterialAndColor) {
  GGEMSMesh original{"source", Write(k_triangle_obj), "mm"};
  EXPECT_THROW(static_cast<void>(original.GetMaterial()), GGEMSRecoverable);
  EXPECT_FALSE(original.GetColorOverride());
  auto material =
    ggems::core::materials::builtins::BuildBuiltInMaterial("Water");
  material.SetColor(ggems::render::RED_Shade5);
  EXPECT_EQ(&original.SetMaterial(material), &original);
  EXPECT_EQ(original.GetColor(), material.GetColor());
  GGEMSMesh copy = original;
  auto const original_vertices = original.GetData().vertices_pm;
  EXPECT_EQ(&original.GetData(), &copy.GetData());
  EXPECT_EQ(&copy.SetPosition({10, -20, 30})
               .SetRotation(0_deg, 0_deg, 90_deg)
               .SetColor(ggems::render::WHITE_White),
            &copy);
  copy.SetMaterial(
    ggems::core::materials::builtins::BuildBuiltInMaterial("Air"));
  EXPECT_EQ(copy.GetName(), "source");
  EXPECT_EQ(copy.GetPositionPM(), (ggems::geometry::Position3PM{10, -20, 30}));
  EXPECT_EQ(copy.GetRotation().axis_x.y, 1.0);
  EXPECT_TRUE(original.GetRotation().IsIdentity());
  EXPECT_EQ(original.GetPositionPM(), ggems::geometry::Position3PM{});
  EXPECT_EQ(original.GetMaterial().GetName(), "Water");
  EXPECT_EQ(copy.GetMaterial().GetName(), "Air");
  EXPECT_EQ(copy.GetColor(), ggems::render::WHITE_White);
  EXPECT_TRUE(copy.GetColorOverride());
  EXPECT_FALSE(original.GetColorOverride());
  copy.SetRotation(90_deg, 0_deg, 0_deg).SetRotation(0_deg, 0_deg, 90_deg);
  EXPECT_EQ(copy.GetRotation().axis_x.y, 1.0);
  EXPECT_EQ(copy.GetRotation().axis_z.z, 1.0);
  EXPECT_EQ(copy.GetData().vertices_pm, original_vertices);
}

TEST_F(GGEMSMeshTest, VerboseEmitsOneSummaryWithLiteralNamesAndReturnsMesh) {
  std::vector<ggems::core::RenderedLogLine> lines;
  auto &logger = ggems::core::GGEMSLogger::GetInstance();
  logger.AddSink(std::make_unique<MeshLogSink>(lines));
  ClearLogSinks const cleanup;
  GGEMSMesh mesh{
    "name##%s",
    Write(std::string{"o leaf##%s\n"} + std::string{k_triangle_obj}), "mm"};
  EXPECT_EQ(&mesh.Verbose(), &mesh);
  ASSERT_EQ(lines.size(), 1U);
  EXPECT_EQ(lines[0].module, "Mesh");
  EXPECT_NE(lines[0].msg.find("name##%s"), std::string::npos);
  EXPECT_NE(lines[0].msg.find("leaf##%s"), std::string::npos);
  EXPECT_NE(lines[0].msg.find("<not configured>"), std::string::npos);
  EXPECT_NE(lines[0].msg.find("triangles: 1; named objects: 1"),
            std::string::npos);
  EXPECT_NE(lines[0].msg.find("Local bounds:"), std::string::npos);
  EXPECT_NE(lines[0].msg.find("source unit: mm"), std::string::npos);
  EXPECT_NE(lines[0].msg.find("size=(" + HumanReadable(1_mm)),
            std::string::npos);
}

TEST_F(GGEMSMeshTest, RejectsMalformedOBJAtImportBoundary) {
  std::array<std::string_view, 11> const invalid{
    "v 0 0 0\n",
    "v nan 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 3\n",
    "v 1e100 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 3\n",
    "v 0 0 0\nv 1 0 0\nv 0 1 0\nf 0 2 3\n",
    "v 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 4\n",
    "v 0 0 0\nv 1 0 0\nv 0 1 0\nf -4 -2 -1\n",
    "v 0 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 2\n",
    "v 0 0 0\nv 1 0 0\nv 2 0 0\nf 1 2 3\n",
    "v 0 0 0\nv 1 0 0\nv 0 1 0\nf 1///2 2 3\n",
    "v 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 3\n",
    "v +-1 0 0\nv 1 0 0\nv 0 1 0\nf 1 2 3\n",
  };
  for (auto const text : invalid) {
    SCOPED_TRACE(text);
    EXPECT_THROW((GGEMSMesh{Write(text), "mm"}), GGEMSRecoverable);
  }
}

TEST_F(GGEMSMeshTest, PolygonRejectionRequestsSourceTriangulation) {
  auto const path = Write("v 0 0 0\nv 1 0 0\nv 1 1 0\nv 0 1 0\nf 1 2 3 4\n");
  try {
    GGEMSMesh const mesh{path, "mm"};
    FAIL() << "Expected polygon rejection";
  } catch (GGEMSRecoverable const &error) {
    EXPECT_NE(std::string_view{error.what()}.find("triangulate"),
              std::string_view::npos);
    EXPECT_NE(std::string_view{error.what()}.find("line 5"),
              std::string_view::npos);
  }
}

TEST_F(GGEMSMeshTest, PhysicalTreeRetainsAnonymousFacesAndNamedObjectIndices) {
  // One oriented tetrahedron distributed across named and anonymous ranges.
  auto const path = Write("v 0 0 0\nv 1 0 0\nv 0 1 0\nv 0 0 1\n"
                          "f 1 3 2\no group\nf 1 2 4\no\nf 1 4 3\n"
                          "o empty\no group\nf 2 3 4\n");
  GGEMSMesh const mesh{path, "mm"};
  auto const tree = BuildMeshBVH(mesh.GetData());
  ASSERT_EQ(tree.component_roots.size(), 3U);
  EXPECT_NE(tree.component_roots[0], tree.component_roots[2]);
  EXPECT_EQ(tree.component_roots[1], std::numeric_limits<std::uint32_t>::max());
  EXPECT_EQ(tree.triangle_order, (std::vector<std::uint32_t>{0, 1, 2, 3}));
  ASSERT_LT(tree.root_index, tree.nodes.size());
  EXPECT_EQ(tree.nodes[tree.root_index].minimum,
            mesh.GetData().bounds_pm.minimum);
  EXPECT_EQ(tree.nodes[tree.root_index].maximum,
            mesh.GetData().bounds_pm.maximum);
  EXPECT_EQ(tree.depth, 2U);
  GGEMSMesh const open{Write(k_triangle_obj), "mm"};
  EXPECT_THROW((void)BuildMeshBVH(open.GetData()), GGEMSRecoverable);
}

TEST_F(GGEMSMeshTest, AdmitsInwardCavityAndRejectsExposedInwardShell) {
  std::string const outer = "v 0 0 0\nv 12 0 0\nv 0 12 0\nv 0 0 12\n"
                            "f 1 3 2\nf 1 2 4\nf 1 4 3\nf 2 3 4\n";
  std::string const inward_faces = "f 5 6 7\nf 5 8 6\nf 5 7 8\nf 6 8 7\n";
  auto const cavity =
    Write(outer + "v 1 1 1\nv 2 1 1\nv 1 2 1\nv 1 1 2\n" + inward_faces);
  EXPECT_NO_THROW((void)BuildMeshBVH(GGEMSMesh{cavity}.GetData()));
  auto const exposed =
    Write(outer + "v 21 1 1\nv 22 1 1\nv 21 2 1\nv 21 1 2\n" + inward_faces);
  EXPECT_THROW((void)BuildMeshBVH(GGEMSMesh{exposed}.GetData()),
               GGEMSRecoverable);
  auto const empty_corner =
    Write(outer + "v 9 9 9\nv 10 9 9\nv 9 10 9\nv 9 9 10\n" + inward_faces);
  EXPECT_THROW((void)BuildMeshBVH(GGEMSMesh{empty_corner}.GetData()),
               GGEMSRecoverable);
}

TEST_F(GGEMSMeshTest, RejectsMalformedSTLAndInvalidExternalInputs) {
  std::vector<std::array<std::array<float, 3>, 3>> facets{
    {{{0, 0, 0}, {1, 0, 0}, {0, 1, 0}}},
  };
  auto truncated = BinarySTL(facets);
  truncated.pop_back();
  EXPECT_THROW((GGEMSMesh{Write(truncated, "truncated.stl"), "mm"}),
               GGEMSRecoverable);
  facets[0][0][0] = std::numeric_limits<float>::infinity();
  EXPECT_THROW((GGEMSMesh{Write(BinarySTL(facets), "nonfinite.stl"), "mm"}),
               GGEMSRecoverable);
  EXPECT_THROW(
    (GGEMSMesh{Write("solid empty\nendsolid empty\n", "empty.stl"), "mm"}),
    GGEMSRecoverable);
  EXPECT_THROW(
    (GGEMSMesh{Write("solid broken\nfacet normal 0 0 0\n", "broken.stl"),
               "mm"}),
    GGEMSRecoverable);
  EXPECT_THROW((GGEMSMesh{Write(k_triangle_obj), "inch"}), GGEMSRecoverable);
  EXPECT_THROW((GGEMSMesh{Write(k_triangle_obj), ""}), GGEMSRecoverable);
  EXPECT_THROW(
    (GGEMSMesh{Write(k_triangle_obj).parent_path() / "missing.obj", "mm"}),
    GGEMSRecoverable);
  EXPECT_THROW((GGEMSMesh{Write(k_triangle_obj, "mesh.off"), "mm"}),
               GGEMSRecoverable);
}

TEST(GGEMSMeshLayout, CopiedWorldSharesOneAssetAcrossTwoMeshPlacements) {
  using ggems::core::materials::builtins::BuildBuiltInMaterial;
  using ggems::geometry::GGEMSMeshOccurrenceRecord;
  auto const fixture =
    std::filesystem::path{GGEMS_TEST_PROBE_ROOT}.parent_path() / "navigation" /
    "meshes" / "three_slabs.obj";
  GGEMSMesh mesh{fixture};
  mesh.SetMaterial(BuildBuiltInMaterial("Tungsten"));
  ggems::geometry::GGEMSWorld world;
  world.SetSize(100_mm, 100_mm, 100_mm)
    .SetMaterial(BuildBuiltInMaterial("Air"))
    .AddVolume(mesh);

  auto snapshot = world;
  mesh.SetPosition({.y = (10_mm).value})
    .SetMaterial(BuildBuiltInMaterial("Water"));
  snapshot.AddVolume(mesh);
  EXPECT_EQ(world.GetVolumeCount(), 1U);
  EXPECT_EQ(snapshot.GetVolumeMaterial(0)->GetName(), "Tungsten");
  EXPECT_EQ(snapshot.GetVolumeMaterial(1)->GetName(), "Water");

  auto const descriptors = snapshot.BuildOccurrenceRecords();
  ASSERT_EQ(descriptors.size(), 2U);
  std::array const material_ids{1U, 2U};
  auto const bytes = snapshot.BuildAnalyticAuxiliaryBuffer(material_ids);
  std::array<GGEMSMeshOccurrenceRecord, 2> records{};
  for (std::size_t index = 0; index < records.size(); ++index) {
    auto const offset = descriptors[index].auxiliary_offset;
    ASSERT_LE(offset + sizeof(GGEMSMeshOccurrenceRecord), bytes.size());
    std::memcpy(&records[index], bytes.data() + offset,
                sizeof(GGEMSMeshOccurrenceRecord));
    EXPECT_EQ(records[index].material_id, material_ids[index]);
  }
  EXPECT_EQ(records[0].pose.center_y_pm, 0.0F);
  EXPECT_EQ(records[1].pose.center_y_pm, static_cast<float>((10_mm).value));
  EXPECT_EQ(records[0].asset_offset, records[1].asset_offset);
}
