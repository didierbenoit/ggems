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
 * \brief Checks real OpenCL mixed-scene location with hand-specified cell
 * owners.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <span>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMSNavigationTestDevices.hh"
#include "GGEMSOpenCLDeviceInventory.hh"
#include "GGEMSVoxelizedTestFixture.hh"
#include "GGEMS/geometry/GGEMSBox.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSVoxelizedVolume.hh"
#include "GGEMS/geometry/GGEMSVoxelizedVolumeRecord.hh"
#include "GGEMS/geometry/GGEMSWorld.hh"
#include "GGEMS/geometry/GGEMSWorldRecord.hh"
#include "GGEMS/materials/GGEMSEMMaterialPackage.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/opencl/GGEMSOpenCLContext.hh"
#include "GGEMS/opencl/GGEMSOpenCLKernel.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMBuffer.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMHostAccess.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/units/GGEMSBytesUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace {
using namespace ggems::geometry;
using namespace ggems::units;
using ggems::core::materials::builtins::BuildBuiltInMaterial;
using ggems::test::GGEMSVoxelizedTestFixture;

/*! \brief One strict interior/exterior point and its independent exact result.
 */
struct LocateCase {
  std::array<float, 3> position;
  std::uint32_t status;
  std::uint32_t volume;
  std::uint32_t material;
};

/*! \brief Creates a finite World, larger than every test fixture. */
auto MakeWorld() -> GGEMSWorld {
  GGEMSWorld world;
  world.SetSize(100_mm, 100_mm, 100_mm)
    .SetMaterial(BuildBuiltInMaterial("Vacuum"));
  return world;
}

/*! \brief Uploads a nonempty span, or one valid unused scalar placeholder. */
template <typename Value>
auto Upload(ggems::ocl::GGEMSOpenCLContext &context,
            std::span<Value const> values) -> ggems::ocl::GGEMSOpenCLSVMBuffer {
  auto buffer = context.CreateSVMBuffer(
    Bytes{values.empty() ? sizeof(Value) : values.size_bytes()});
  if (values.empty()) {
    ggems::ocl::WriteSVMFromHost(buffer, Value{});
  } else {
    ggems::ocl::WriteSVMFromHost(buffer, values);
  }
  return buffer;
}

/*! \brief Compares device results with exact independent owner expectations. */
auto CheckResults(std::span<LocateCase const> cases,
                  std::span<std::uint32_t const> results, bool has_voxels)
  -> void {
  auto const expected_boundary = has_voxels ? 2U : 0U;
  for (std::size_t index = 0U; index < cases.size(); ++index) {
    SCOPED_TRACE(index);
    EXPECT_EQ(results[4U * index], cases[index].status);
    if (cases[index].status != 0U) {
      continue;
    }
    // N3.2 must not pretend it can trace through any voxel-containing scene.
    std::array const actual{
      results[(4U * index) + 1U],
      results[(4U * index) + 2U],
      results[(4U * index) + 3U],
    };
    std::array const expected{
      cases[index].volume,
      cases[index].material,
      expected_boundary,
    };
    EXPECT_EQ(actual, expected);
  }
}

/*! \brief Executes the generic locator using normal host-published assets. */
auto CheckLocate(GGEMSWorld const &world, std::span<LocateCase const> cases)
  -> void {
  auto &context = ggems::test::GetSelectedOpenCLContext();
  using Material = ggems::core::materials::GGEMSMaterial;
  std::vector<Material> materials{world.GetMaterial()};
  for (std::size_t index = 0U; index < world.GetVolumeCount(); ++index) {
    if (world.GetVoxelizedVolume(index) == nullptr) {
      materials.push_back(*world.GetVolumeMaterial(index));
    }
  }
  auto const palette_begin = materials.size();
  for (std::size_t index = 0U; index < world.GetVolumeCount(); ++index) {
    if (auto const *voxel = world.GetVoxelizedVolume(index)) {
      auto const palette = voxel->GetMaterials();
      materials.insert(materials.end(), palette.begin(), palette.end());
    }
  }
  ggems::core::materials::GGEMSEMMaterialPackage const package{materials};
  auto const identities = package.GetMaterialIds();
  auto const analytic = world.BuildAnalyticDeviceRecords(
    identities.subspan(1U, palette_begin - 1U));
  auto const voxel =
    world.BuildVoxelizedDeviceData(identities.subspan(palette_begin));
  auto const record = world.BuildRecord();
  auto const auxiliary = world.BuildAnalyticAuxiliaryBuffer();
  std::vector<float> positions;
  for (auto const &item : cases) {
    positions.insert(positions.end(), item.position.begin(),
                     item.position.end());
  }
  std::vector<std::uint32_t> results(cases.size() * 4U, 0xFFFF'FFFFU);
  auto world_buffer = Upload(context, std::span{&record, 1U});
  auto analytic_buffer = Upload(context, std::span{analytic});
  auto auxiliary_buffer = Upload(context, std::span{auxiliary});
  auto voxel_buffer = Upload(context, std::span{voxel.records});
  auto label_buffer = Upload(context, std::span{voxel.material_labels});
  auto palette_buffer = Upload(context, std::span{voxel.material_ids});
  auto positions_buffer = Upload(context, std::span<float const>{positions});
  auto results_buffer =
    Upload(context, std::span<std::uint32_t const>{results});
  auto const &program =
    ggems::ocl::GGEMSOpenCL::GetInstance().GetOrCreateProgram(
      context, std::filesystem::path{GGEMS_TEST_PROBE_ROOT},
      "voxelized_scene_probe",
      std::format(
        "-I{}",
        std::filesystem::path{GGEMS_TEST_KERNEL_ROOT}.generic_string()));
  ggems::ocl::GGEMSOpenCLKernel kernel{
    context, program.CreateKernel("voxelized_locate_probe"),
    "voxelized_locate_probe"};
  kernel.SetArgSVMPointer(0U, world_buffer.GetData());
  kernel.SetArgSVMPointer(1U, analytic_buffer.GetData());
  kernel.SetArgSVMPointer(2U, auxiliary_buffer.GetData());
  kernel.SetArgSVMPointer(3U, voxel_buffer.GetData());
  kernel.SetArgSVMPointer(4U, label_buffer.GetData());
  kernel.SetArgSVMPointer(5U, palette_buffer.GetData());
  kernel.SetArgSVMPointer(6U, positions_buffer.GetData());
  kernel.SetArgSVMPointer(7U, results_buffer.GetData());
  kernel.Run({cases.size()}, {1U});
  ggems::ocl::ReadSVMToHost(results_buffer, std::span{results});
  CheckResults(cases, results, record.voxelized_count != 0U);
}

using GGEMSVoxelizedNavigation = GGEMSNavigationDeviceTest;
} // namespace

TEST_F(GGEMSVoxelizedNavigation, LocatesEveryAnisotropicCellInXFastOrder) {
  GGEMSVoxelizedTestFixture const files;
  auto world = MakeWorld();
  world.AddVolume(files.Import());
  ASSERT_NO_THROW(world.Validate());
  std::array<LocateCase, 10> const cases{
    {
      {
        .position = {-0.5e9F, -1e9F, -1.5e9F},
        .status = 0U,
        .volume = 1U,
        .material = 1U,
      },
      {
        .position = {0.5e9F, -1e9F, -1.5e9F},
        .status = 0U,
        .volume = 1U,
        .material = 2U,
      },
      {
        .position = {-0.5e9F, 1e9F, -1.5e9F},
        .status = 0U,
        .volume = 1U,
        .material = 1U,
      },
      {
        .position = {0.5e9F, 1e9F, -1.5e9F},
        .status = 0U,
        .volume = 1U,
        .material = 1U,
      },
      {
        .position = {-0.5e9F, -1e9F, 1.5e9F},
        .status = 0U,
        .volume = 1U,
        .material = 1U,
      },
      {
        .position = {0.5e9F, -1e9F, 1.5e9F},
        .status = 0U,
        .volume = 1U,
        .material = 2U,
      },
      {
        .position = {-0.5e9F, 1e9F, 1.5e9F},
        .status = 0U,
        .volume = 1U,
        .material = 2U,
      },
      {
        .position = {0.5e9F, 1e9F, 1.5e9F},
        .status = 0U,
        .volume = 1U,
        .material = 1U,
      },
      {
        .position = {2e9F, 0.0F, 0.0F},
        .status = 0U,
        .volume = 0U,
        .material = 0U,
      },
      {
        .position = {60e9F, 0.0F, 0.0F},
        .status = 1U,
        .volume = 0U,
        .material = 0U,
      },
    },
  };
  CheckLocate(world, cases);
}

TEST_F(GGEMSVoxelizedNavigation,
       TranslatedQuarterTurnUsesGridLocalCoordinates) {
  GGEMSVoxelizedTestFixture const files;
  auto grid = files.Import();
  grid
    .SetPosition(
      MakePositionPM(10'000'000'000LL, -5'000'000'000LL, 2'000'000'000LL))
    .SetRotation(0_deg, 0_deg, 90_deg);
  auto world = MakeWorld();
  world.AddVolume(grid);
  ASSERT_NO_THROW(world.Validate());
  // A +90-degree Z rotation maps (x,y,z) to (10-y,-5+x,2+z) mm.
  std::array<LocateCase, 9> const cases{
    {
      {
        .position = {11e9F, -5.5e9F, 0.5e9F},
        .status = 0U,
        .volume = 1U,
        .material = 1U,
      },
      {
        .position = {11e9F, -4.5e9F, 0.5e9F},
        .status = 0U,
        .volume = 1U,
        .material = 2U,
      },
      {
        .position = {9e9F, -5.5e9F, 0.5e9F},
        .status = 0U,
        .volume = 1U,
        .material = 1U,
      },
      {
        .position = {9e9F, -4.5e9F, 0.5e9F},
        .status = 0U,
        .volume = 1U,
        .material = 1U,
      },
      {
        .position = {11e9F, -5.5e9F, 3.5e9F},
        .status = 0U,
        .volume = 1U,
        .material = 1U,
      },
      {
        .position = {11e9F, -4.5e9F, 3.5e9F},
        .status = 0U,
        .volume = 1U,
        .material = 2U,
      },
      {
        .position = {9e9F, -5.5e9F, 3.5e9F},
        .status = 0U,
        .volume = 1U,
        .material = 2U,
      },
      {
        .position = {9e9F, -4.5e9F, 3.5e9F},
        .status = 0U,
        .volume = 1U,
        .material = 1U,
      },
      {
        .position = {0.0F, 0.0F, 0.0F},
        .status = 0U,
        .volume = 0U,
        .material = 0U,
      },
    },
  };
  CheckLocate(world, cases);
}

TEST_F(GGEMSVoxelizedNavigation,
       PreservesFourLevelHierarchyAndInterleavedGridIDs) {
  GGEMSVoxelizedTestFixture const files;
  auto first = files.Import();
  first.SetPosition(MakePositionPM(-30'000'000'000LL, 0, 0));
  auto second = files.Import();
  second.SetPosition(MakePositionPM(35'000'000'000LL, 0, 0));
  GGEMSBox mother;
  mother.SetSize(20_mm, 20_mm, 20_mm)
    .SetPosition(MakePositionPM(15'000'000'000LL, 0, 0))
    .SetMaterial(BuildBuiltInMaterial("Air"));
  GGEMSBox child;
  child.SetSize(8_mm, 8_mm, 8_mm).SetMaterial(BuildBuiltInMaterial("Water"));
  GGEMSBox grandchild;
  grandchild.SetSize(4_mm, 4_mm, 4_mm).SetMaterial(BuildBuiltInMaterial("Air"));
  GGEMSBox leaf;
  leaf.SetSize(1_mm, 1_mm, 1_mm).SetMaterial(BuildBuiltInMaterial("Aluminum"));
  auto world = MakeWorld();
  world.AddVolume(first)
    .AddVolume(mother)
    .AddVolume(child, 2U)
    .AddVolume(second)
    .AddVolume(grandchild, 3U)
    .AddVolume(leaf, 5U);
  ASSERT_NO_THROW(world.Validate());
  EXPECT_EQ(world.BuildRecord().hierarchy_depth, 4U);
  std::array<LocateCase, 7> const cases{
    {
      {
        .position = {-30.5e9F, -1e9F, -1.5e9F},
        .status = 0U,
        .volume = 1U,
        .material = 2U,
      },
      {
        .position = {35.5e9F, -1e9F, -1.5e9F},
        .status = 0U,
        .volume = 4U,
        .material = 1U,
      },
      {
        .position = {23e9F, 0.0F, 0.0F},
        .status = 0U,
        .volume = 2U,
        .material = 1U,
      },
      {
        .position = {18e9F, 0.0F, 0.0F},
        .status = 0U,
        .volume = 3U,
        .material = 2U,
      },
      {
        .position = {16.5e9F, 0.0F, 0.0F},
        .status = 0U,
        .volume = 5U,
        .material = 1U,
      },
      {
        .position = {15.25e9F, 0.0F, 0.0F},
        .status = 0U,
        .volume = 6U,
        .material = 3U,
      },
      {
        .position = {0.0F, 0.0F, 0.0F},
        .status = 0U,
        .volume = 0U,
        .material = 0U,
      },
    },
  };
  CheckLocate(world, cases);
}

TEST_F(GGEMSVoxelizedNavigation, DeduplicatesWorldAndDifferentLocalPalettes) {
  GGEMSVoxelizedTestFixture const first_files;
  GGEMSVoxelizedTestFixture const second_files{
    {2U, 2U, 2U},
    "1 2 3",
    GGEMSVoxelizedTestFixture::k_labels,
    "10 10 Air\n20 20 Water\n30 30 Air\n",
  };
  auto first = first_files.Import();
  auto second = second_files.Import();
  first.SetPosition(MakePositionPM(-10'000'000'000LL, 0, 0));
  second.SetPosition(MakePositionPM(10'000'000'000LL, 0, 0));
  auto world = MakeWorld();
  world.SetMaterial(BuildBuiltInMaterial("Water"));
  world.AddVolume(first).AddVolume(second);
  ASSERT_NO_THROW(world.Validate());
  std::array<LocateCase, 5> const cases{
    {
      {
        .position = {-10.5e9F, -1e9F, -1.5e9F},
        .status = 0U,
        .volume = 1U,
        .material = 0U,
      },
      {
        .position = {-9.5e9F, -1e9F, -1.5e9F},
        .status = 0U,
        .volume = 1U,
        .material = 1U,
      },
      {
        .position = {9.5e9F, -1e9F, -1.5e9F},
        .status = 0U,
        .volume = 2U,
        .material = 1U,
      },
      {
        .position = {10.5e9F, -1e9F, -1.5e9F},
        .status = 0U,
        .volume = 2U,
        .material = 0U,
      },
      {
        .position = {0.0F, 0.0F, 0.0F},
        .status = 0U,
        .volume = 0U,
        .material = 0U,
      },
    },
  };
  CheckLocate(world, cases);
}

TEST_F(GGEMSVoxelizedNavigation, OverlappingGridsAreUnresolvedInEitherOrder) {
  GGEMSVoxelizedTestFixture const files;
  auto first = files.Import();
  auto second = files.Import();
  second.SetPosition(MakePositionPM(200'000'000LL, 0, 0));
  std::array<LocateCase, 1> const cases{
    {
      {
        .position = {-0.25e9F, -1e9F, -1.5e9F},
        .status = 2U,
        .volume = 0U,
        .material = 0U,
      },
    },
  };
  for (bool const reverse : {false, true}) {
    auto world = MakeWorld();
    world.AddVolume(reverse ? second : first)
      .AddVolume(reverse ? first : second);
    ASSERT_NO_THROW(world.Validate());
    CheckLocate(world, cases);
  }
}

TEST_F(GGEMSVoxelizedNavigation, AnalyticGridOverlapIsUnresolvedInEitherOrder) {
  GGEMSVoxelizedTestFixture const files;
  GGEMSBox box;
  box.SetSize(4_mm, 6_mm, 8_mm).SetMaterial(BuildBuiltInMaterial("Water"));
  std::array<LocateCase, 1> const cases{
    {
      {
        .position = {-0.5e9F, -1e9F, -1.5e9F},
        .status = 2U,
        .volume = 0U,
        .material = 0U,
      },
    },
  };
  for (bool const reverse : {false, true}) {
    auto world = MakeWorld();
    if (reverse) {
      world.AddVolume(box).AddVolume(files.Import());
    } else {
      world.AddVolume(files.Import()).AddVolume(box);
    }
    ASSERT_NO_THROW(world.Validate());
    CheckLocate(world, cases);
  }
}

TEST_F(GGEMSVoxelizedNavigation,
       DeviceRecordOffsetsStrideAndScalarReadsMatchHost) {
  using Record = GGEMSVoxelizedVolumeRecord;
  auto &context = ggems::test::GetSelectedOpenCLContext();
  std::array<Record, 2> records{};
  auto &second = records[1];
  second.dimension_x = 3U;
  second.dimension_y = 5U;
  second.dimension_z = 7U;
  second.volume_id = 23U;
  second.palette_offset = 12U;
  second.material_label_offset = 0x1'0000'0001ULL;
  second.pose.center_x_pm = -7.0F;
  second.spacing_z_pm = 11.0F;
  std::array<std::uint64_t, 25> layout{};
  auto input = Upload(context, std::span<Record const>{records});
  auto output = Upload(context, std::span<std::uint64_t const>{layout});
  auto const &program =
    ggems::ocl::GGEMSOpenCL::GetInstance().GetOrCreateProgram(
      context, std::filesystem::path{GGEMS_TEST_PROBE_ROOT},
      "voxelized_scene_probe",
      std::format(
        "-I{}",
        std::filesystem::path{GGEMS_TEST_KERNEL_ROOT}.generic_string()));
  ggems::ocl::GGEMSOpenCLKernel kernel{
    context, program.CreateKernel("voxelized_abi_probe"),
    "voxelized_abi_probe"};
  kernel.SetArgSVMPointer(0U, input.GetData());
  kernel.SetArgSVMPointer(1U, output.GetData());
  kernel.Run({1U}, {1U});
  ggems::ocl::ReadSVMToHost(output, std::span{layout});
  std::array<std::uint64_t, 25> const expected{
    sizeof(Record),
    alignof(Record),
    sizeof(Record),
    offsetof(Record, pose),
    offsetof(Record, dimension_x),
    offsetof(Record, dimension_y),
    offsetof(Record, dimension_z),
    offsetof(Record, volume_id),
    offsetof(Record, spacing_x_pm),
    offsetof(Record, spacing_y_pm),
    offsetof(Record, spacing_z_pm),
    offsetof(Record, tolerance_pm),
    offsetof(Record, half_extent_x_pm),
    offsetof(Record, half_extent_y_pm),
    offsetof(Record, half_extent_z_pm),
    offsetof(Record, palette_offset),
    offsetof(Record, material_label_offset),
    3U,
    5U,
    7U,
    23U,
    12U,
    0x1'0000'0001ULL,
    std::bit_cast<std::uint32_t>(-7.0F),
    std::bit_cast<std::uint32_t>(11.0F),
  };
  EXPECT_EQ(layout, expected);
}
