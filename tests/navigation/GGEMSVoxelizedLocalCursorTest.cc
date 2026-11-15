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
 * \brief Verifies long local voxel continuation and failed transaction
 * rollback.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <iterator>
#include <numbers>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMSAnalyticStreamFixture.hh"
#include "GGEMSNavigationTestDevices.hh"
#include "GGEMSOpenCLDeviceInventory.hh"
#include "GGEMSVoxelizedTestFixture.hh"
#include "GGEMS/geometry/GGEMSAnalyticVolumeRecord.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSWorld.hh"
#include "GGEMS/geometry/GGEMSWorldRecord.hh"
#include "GGEMS/materials/GGEMSEMMaterialPackage.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/opencl/GGEMSOpenCLContext.hh"
#include "GGEMS/opencl/GGEMSOpenCLExternal.hh"
#include "GGEMS/opencl/GGEMSOpenCLKernel.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMBuffer.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMHostAccess.hh"
#include "GGEMS/particles/GGEMSParticleState.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/units/GGEMSBytesUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace {
using namespace ggems;
using namespace ggems::core;
using namespace ggems::units;
using Vec = std::array<double, 3>;

/*! \brief Reads an existing production header for a test-only instrumented
 * copy. */
auto Read(std::filesystem::path const &path) -> std::string {
  std::ifstream input{path};
  if (!input) {
    throw std::runtime_error("Cannot read probe input " + path.string());
  }
  return {std::istreambuf_iterator<char>{input}, {}};
}

/*!
 * \brief Counts real helper calls using trailers on the probe's single records.
 *
 * Only the compiled test copy gains atomics. No production hook, cursor field,
 * geometry query, or boundary arithmetic is replaced. Matching the helper name
 * and balanced function braces avoids dependence on its statement formatting.
 */
auto InstrumentFunction(std::string &source, std::string_view name,
                        bool count_transforms, std::string_view counter)
  -> void {
  auto const declaration = source.find(std::string{name} + "(");
  auto const begin = source.find('{', declaration);
  if (declaration == std::string::npos || begin == std::string::npos) {
    throw std::runtime_error("Missing probe helper " + std::string{name});
  }
  std::size_t end = begin + 1U;
  std::size_t depth = 1U;
  while (depth != 0U && end < source.size()) {
    depth += source[end] == '{' ? 1U : 0U;
    depth -= source[end] == '}' ? 1U : 0U;
    ++end;
  }
  if (depth != 0U) {
    throw std::runtime_error("Unbalanced probe helper " + std::string{name});
  }
  auto body = source.substr(begin + 1U, end - begin - 2U);
  if (count_transforms) {
    std::string const call{"GGEMS_BoxLocalRay("};
    std::string const replacement{
      "atomic_inc((volatile __global uint *)(volume + 1) + 1);\n"
      "  GGEMS_BoxLocalRay("};
    std::size_t position{};
    while ((position = body.find(call, position)) != std::string::npos) {
      body.replace(position, call.size(), replacement);
      position += replacement.size();
    }
  }
  source.replace(begin + 1U, end - begin - 2U,
                 "\n" + std::string{counter} + body);
}

/*! \brief Builds a normal common-Transport probe with call-count
 * instrumentation. */
auto ProbeSource() -> std::string {
  std::filesystem::path const root{GGEMS_TEST_KERNEL_ROOT};
  auto voxel = Read(root / "navigation/GGEMSVoxelizedNavigation.clh");
  auto scene = Read(root / "navigation/GGEMSOccurrenceNavigation.clh");
  InstrumentFunction(voxel, "GGEMS_VoxelizedLocate", true,
                     "  atomic_inc((volatile __global uint *)(volume + 1));\n");
  InstrumentFunction(voxel, "GGEMS_VoxelizedFindBoundary", true, "");
  InstrumentFunction(scene, "GGEMS_SceneLocateExcluding", false,
                     "  atomic_inc((volatile __global uint *)(world + 1));\n");
  return voxel + "\n" + scene + R"cl(
#include "transport/GGEMSTransport.clh"

__kernel void local_cursor_probe(
  __global GGEMSWorldRecord const *world,
  __global GGEMSAnalyticVolumeRecord const *analytic,
  __global void const *auxiliary,
  __global GGEMSVoxelizedVolumeRecord const *voxel,
  __global ushort const *labels, __global uint const *palette,
  __global float const *ray, __global uint *output, uint reject_move) {
  GGEMSParticleState particle = {0};
  particle.position_x_pm = ray[0];
  particle.position_y_pm = ray[1];
  particle.position_z_pm = ray[2];
  particle.direction_x = ray[3];
  particle.direction_y = ray[4];
  particle.direction_z = ray[5];
  particle.status = GGEMS_PARTICLE_STATUS_ALIVE;
  GGEMSVoxelCursor cursor = {0};
  ulong owner;
  uint status = GGEMS_SceneLocate(world, analytic, auxiliary, voxel, labels,
    palette, vload3(0, ray), vload3(1, ray), &owner, &cursor);
  output[0] = 0U;
  output[1] = status;
  if (status != GGEMS_NAVIGATION_RESOLVED) { return; }
  particle.current_volume_id = (uint)(owner >> 32U);
  particle.material_id = (uint)owner;
  uint event = GGEMS_NAVIGATION_EVENT_NONE;
  uint faces = 0U;
  uint previous = GGEMS_VOLUME_ID_WORLD;
  if (reject_move != 0U) {
    particle.position_x_pm = INFINITY;
    GGEMSVoxelCursor proposal = cursor;
    GGEMSBoundaryWitness witness;
    output[2] = GGEMS_SceneFindBoundary(world, analytic, auxiliary, voxel,
      labels, palette, (float3)(INFINITY, ray[1], ray[2]), vload3(1, ray),
      particle.current_volume_id, event, previous, &witness, &proposal);
    output[3] = as_uint(witness.parameter);
    uchar cursor_before[sizeof(GGEMSVoxelCursor)];
    uchar particle_before[sizeof(GGEMSParticleState)];
    for (uint index = 0U; index < sizeof(cursor); ++index) {
      cursor_before[index] = ((uchar *)&cursor)[index];
    }
    for (uint index = 0U; index < sizeof(particle); ++index) {
      particle_before[index] = ((uchar *)&particle)[index];
    }
    output[4] = GGEMS_TransportToNextBoundary(world, analytic, auxiliary, voxel,
      labels, palette, &particle, &event, &faces, &previous, &cursor);
    uint changed = 0U;
    for (uint index = 0U; index < sizeof(cursor); ++index) {
      changed |= cursor_before[index] != ((uchar *)&cursor)[index];
    }
    output[5] = changed;
    changed = 0U;
    for (uint index = 0U; index < sizeof(particle); ++index) {
      changed |= particle_before[index] != ((uchar *)&particle)[index];
    }
    output[6] = changed;
    output[7] = event;
    output[8] = faces;
    output[9] = previous;
    return;
  }
  for (uint index = 0U; index < world->max_boundary_events; ++index) {
    uint offset = 2U + 12U * index;
    status = GGEMS_TransportToNextBoundary(world, analytic, auxiliary, voxel,
      labels, palette, &particle, &event, &faces, &previous, &cursor);
    output[0] = index + 1U;
    output[offset] = status;
    output[offset + 1U] = event;
    output[offset + 2U] = faces;
    output[offset + 3U] = particle.current_volume_id;
    output[offset + 4U] = particle.material_id;
    output[offset + 5U] = as_uint(particle.position_x_pm);
    output[offset + 6U] = as_uint(particle.position_y_pm);
    output[offset + 7U] = as_uint(particle.position_z_pm);
    output[offset + 8U] = *((volatile __global uint *)(world + 1));
    output[offset + 9U] = *((volatile __global uint *)(voxel + 1));
    output[offset + 10U] = *((volatile __global uint *)(voxel + 1) + 1);
    output[offset + 11U] = particle.status;
    if (status != GGEMS_NAVIGATION_RESOLVED ||
        event == GGEMS_NAVIGATION_EVENT_WORLD_EXIT) { break; }
  }
}
)cl";
}

/*! \brief Uploads a nonempty asset, or one valid unused record. */
template <typename Value>
auto Upload(ocl::GGEMSOpenCLContext &context, std::span<Value const> values)
  -> ocl::GGEMSOpenCLSVMBuffer {
  auto result = context.CreateSVMBuffer(
    Bytes{std::max(values.size_bytes(), sizeof(Value))});
  if (values.empty()) {
    ocl::WriteSVMFromHost(result, Value{});
  } else {
    ocl::WriteSVMFromHost(result, values);
  }
  return result;
}

/*! \brief Appends two zero counters to one immutable test record allocation. */
template <typename Record>
auto Counted(ocl::GGEMSOpenCLContext &context, Record const &record)
  -> ocl::GGEMSOpenCLSVMBuffer {
  std::array<std::byte, sizeof(Record) + (2U * sizeof(std::uint32_t))> bytes{};
  std::memcpy(bytes.data(), &record, sizeof(record));
  return Upload(context, std::span<std::byte const>{bytes});
}

/*! \brief Executes the actual transaction with one tiny host-published grid. */
auto RunCursorProbe(geometry::GGEMSWorld const &world,
                    std::array<float, 6> const &ray, bool reject_move = false)
  -> std::vector<std::uint32_t> {
  auto &context = test::GetSelectedOpenCLContext();
  auto const *grid = world.GetVoxelizedVolume(0U);
  std::vector<materials::GGEMSMaterial> media{world.GetMaterial()};
  auto const palette = grid->GetMaterials();
  media.insert(media.end(), palette.begin(), palette.end());
  materials::GGEMSEMMaterialPackage const package{media};
  auto const voxel =
    world.BuildVoxelizedDeviceData(package.GetMaterialIds().subspan(1U));
  auto const record = world.BuildRecord();
  auto const auxiliary = world.BuildAnalyticAuxiliaryBuffer();
  auto world_buffer = Counted(context, record);
  auto voxel_buffer = Counted(context, voxel.records[0]);
  auto analytic_buffer =
    Upload(context, std::span<geometry::GGEMSAnalyticVolumeRecord const>{});
  auto auxiliary_buffer = Upload(context, std::span{auxiliary});
  auto labels_buffer = Upload(context, std::span{voxel.material_labels});
  auto palette_buffer = Upload(context, std::span{voxel.material_ids});
  auto ray_buffer = Upload(context, std::span<float const>{ray});
  std::vector<std::uint32_t> result(
    2U + (12U * static_cast<std::size_t>(record.max_boundary_events)), 0U);
  auto result_buffer = Upload(context, std::span<std::uint32_t const>{result});
  auto const &program = test::GetAnalyticTraceProgram(
    ProbeSource(),
    std::format(
      "-I{}", std::filesystem::path{GGEMS_TEST_KERNEL_ROOT}.generic_string()));
  ocl::GGEMSOpenCLKernel kernel{
    context, cl::Kernel(program, "local_cursor_probe"), "local_cursor_probe"};
  kernel.SetArgSVMPointer(0U, world_buffer.GetData());
  kernel.SetArgSVMPointer(1U, analytic_buffer.GetData());
  kernel.SetArgSVMPointer(2U, auxiliary_buffer.GetData());
  kernel.SetArgSVMPointer(3U, voxel_buffer.GetData());
  kernel.SetArgSVMPointer(4U, labels_buffer.GetData());
  kernel.SetArgSVMPointer(5U, palette_buffer.GetData());
  kernel.SetArgSVMPointer(6U, ray_buffer.GetData());
  kernel.SetArgSVMPointer(7U, result_buffer.GetData());
  kernel.SetArg(8U, static_cast<cl_uint>(reject_move));
  kernel.Run({1U}, {1U});
  ocl::ReadSVMToHost(result_buffer, std::span{result});
  return result;
}

/*! \brief Returns a 100 mm cubic Vacuum World. */
auto World() -> geometry::GGEMSWorld {
  geometry::GGEMSWorld world;
  world.SetSize(100_mm, 100_mm, 100_mm)
    .SetMaterial(materials::builtins::BuildBuiltInMaterial("Vacuum"));
  return world;
}

/*! \brief Independent 30 degree Y rotation followed by fixed translation. */
auto Transform(Vec const &local, bool position) -> Vec {
  double const cosine = std::numbers::sqrt3 / 2;
  return {
    (cosine * local[0]) + (0.5 * local[2]) + (position ? 5.0 : 0.0),
    local[1] + (position ? -3.0 : 0.0),
    (-0.5 * local[0]) + (cosine * local[2]) + (position ? 4.0 : 0.0),
  };
}

/*! \brief Creates alternating material cells in X-fastest order. */
auto Labels(std::array<std::size_t, 3> const &dimensions)
  -> std::vector<std::uint16_t> {
  std::vector<std::uint16_t> labels;
  for (std::size_t z = 0; z < dimensions[2]; ++z) {
    for (std::size_t y = 0; y < dimensions[1]; ++y) {
      for (std::size_t x = 0; x < dimensions[0]; ++x) {
        labels.push_back(((x + y + z) % 2U) == 0U ? 10U : 20U);
      }
    }
  }
  return labels;
}

using GGEMSVoxelizedLocalCursor = GGEMSNavigationDeviceTest;
} // namespace

TEST_F(GGEMSVoxelizedLocalCursor, ThousandsOfPosedCellsDecodeAndTransformOnce) {
  double maximum_error{};
  for (std::size_t axis = 0; axis < 3U; ++axis) {
    std::array<std::size_t, 3> dimensions{2U, 3U, 2U};
    dimensions[axis] = 4096U;
    Vec spacing{1.0, 2.0, 1.0};
    spacing[axis] = 0.01;
    auto const labels = Labels(dimensions);
    test::GGEMSVoxelizedTestFixture const files{
      dimensions, std::format("{} {} {}", spacing[0], spacing[1], spacing[2]),
      labels, "10 10 Water\n20 20 Air\n"};
    auto grid = files.Import();
    grid.SetRotation(0_deg, 30_deg, 0_deg);
    grid.SetPosition(geometry::MakePositionPM(5'000'000'000LL, -3'000'000'000LL,
                                              4'000'000'000LL));
    auto world = World();
    world.AddVolume(grid);
    ASSERT_NO_THROW(world.Validate());
    double const budget =
      8 * std::max(world.GetNavigationTolerancePM(),
                   world.GetVolumeNavigationTolerancePM(0U));
    auto const packed =
      world.BuildVoxelizedDeviceData(std::array{1U, 2U}).records.front();
    auto const &pose = packed.pose;
    std::array<Vec, 3> const axes{
      Vec{pose.axis_x_x, pose.axis_x_y, pose.axis_x_z},
      Vec{pose.axis_y_x, pose.axis_y_y, pose.axis_y_z},
      Vec{pose.axis_z_x, pose.axis_z_y, pose.axis_z_z},
    };
    Vec const center{pose.center_x_pm, pose.center_y_pm, pose.center_z_pm};
    Vec const half_extent{
      packed.half_extent_x_pm,
      packed.half_extent_y_pm,
      packed.half_extent_z_pm,
    };
    for (double const sign : {-1.0, 1.0}) {
      SCOPED_TRACE(std::format("axis {}, sign {}", axis, sign));
      Vec local{};
      std::array<std::size_t, 3> cell{};
      for (std::size_t component = 0; component < 3U; ++component) {
        cell[component] = dimensions[component] / 2U;
        local[component] =
          (static_cast<double>(cell[component]) + 0.5 -
           (0.5 * static_cast<double>(dimensions[component]))) *
          spacing[component];
      }
      cell[axis] = sign > 0 ? 0U : dimensions[axis] - 1U;
      local[axis] = -sign * 20.475;
      auto const start = Transform(local, true);
      Vec local_direction{};
      local_direction[axis] = sign;
      auto const direction = Transform(local_direction, false);
      std::array<float, 6> const ray{
        static_cast<float>(start[0] * 1.0e9),
        static_cast<float>(start[1] * 1.0e9),
        static_cast<float>(start[2] * 1.0e9),
        static_cast<float>(direction[0]),
        static_cast<float>(direction[1]),
        static_cast<float>(direction[2]),
      };
      auto const result = RunCursorProbe(world, ray);
      ASSERT_EQ(result[1], 0U);
      ASSERT_EQ(result[0], 4097U);
      auto const faces = 1U << ((2U * axis) + (sign > 0 ? 1U : 0U));
      double case_error{};
      Vec exit_error{};
      double exit_normal_error{};
      double exit_plane_residual{-half_extent[axis]};
      std::array<std::size_t, 2> worst{};
      for (std::size_t event = 0; event < 4096U; ++event) {
        auto const offset = 2U + (12U * event);
        bool const internal = event < 4095U;
        if (internal) {
          if (sign > 0) {
            ++cell[axis];
          } else {
            --cell[axis];
          }
        }
        auto const material =
          static_cast<std::uint32_t>(1U + ((cell[0] + cell[1] + cell[2]) % 2U));
        std::array const actual{
          result[offset],      result[offset + 1U],  result[offset + 2U],
          result[offset + 3U], result[offset + 4U],  result[offset + 8U],
          result[offset + 9U], result[offset + 10U],
        };
        std::array const expected{
          0U,
          internal ? 4U : 3U,
          faces,
          internal ? 1U : 0U,
          internal ? material : 0U,
          internal ? 1U : 2U,
          1U,
          1U,
        };
        ASSERT_EQ(actual, expected) << "event " << event;
        double const distance = 0.005 + (0.01 * static_cast<double>(event));
        for (std::size_t component = 0; component < 3U; ++component) {
          auto const point =
            std::bit_cast<float>(result[offset + 5U + component]);
          double const ideal =
            (start[component] + (distance * direction[component])) * 1.0e9;
          double const error = std::abs(point - ideal);
          if (!internal) {
            exit_error[component] = point - ideal;
            exit_normal_error +=
              (sign * axes[axis][component]) * exit_error[component];
            exit_plane_residual +=
              (sign * axes[axis][component]) * (point - center[component]);
          }
          if (error > case_error) {
            case_error = error;
            worst = {event, component};
          }
        }
      }
      maximum_error = std::max(maximum_error, case_error);
      auto const case_name =
        std::format("axis{}_{}", axis, sign > 0 ? "positive" : "negative");
      RecordProperty(case_name + "_max_error_pm", case_error);
      for (std::size_t component = 0; component < 3U; ++component) {
        RecordProperty(std::format("{}_exit_error_{}_pm", case_name, component),
                       exit_error[component]);
      }
      RecordProperty(case_name + "_exit_normal_error_pm", exit_normal_error);
      RecordProperty(case_name + "_exit_plane_residual_pm",
                     exit_plane_residual);
      EXPECT_LE(case_error, budget)
        << "event " << worst[0] << " coordinate " << worst[1];
      auto const terminal = 2U + (12U * 4096U);
      EXPECT_EQ(result[terminal], 0U);
      EXPECT_EQ(result[terminal + 1U], 1U);
      EXPECT_EQ(result[terminal + 3U], 0xFFFF'FFFFU);
      EXPECT_EQ(sizeof(particles::GGEMSParticleState), 80U);
    }
  }
  RecordProperty("maximum_global_endpoint_error_pm", maximum_error);
}

TEST_F(GGEMSVoxelizedLocalCursor,
       RejectedGlobalMovePreservesParticleAndCursor) {
  test::GGEMSVoxelizedTestFixture const files;
  auto world = World();
  world.AddVolume(files.Import());
  std::array<float, 6> const ray{-0.5e9F, -1.0e9F, -1.5e9F, 1, 0, 0};
  auto const result = RunCursorProbe(world, ray, true);
  ASSERT_EQ(result[1], 0U);
  EXPECT_EQ(result[2], 0U);
  EXPECT_GT(std::bit_cast<float>(result[3]), 0.0F);
  EXPECT_EQ(result[4], 2U);
  std::array const untouched{
    result[5], result[6], result[7], result[8], result[9],
  };
  EXPECT_EQ(untouched, (std::array{0U, 0U, 0U, 0U, 0U}));
}

TEST_F(GGEMSVoxelizedLocalCursor,
       NearDiagonalRetainsAccumulatedPlaneSeparation) {
  std::array<std::size_t, 3> const dimensions{64U, 64U, 1U};
  auto const labels = Labels(dimensions);
  test::GGEMSVoxelizedTestFixture const files{
    dimensions,
    "1 1 1",
    labels,
    "10 10 Water\n20 20 Air\n",
  };
  auto world = World();
  world.AddVolume(files.Import());
  ASSERT_NO_THROW(world.Validate());
  float const tolerance = world.GetVolumeNavigationTolerancePM(0U);
  float const slope = 1.0F + (0.5F * tolerance / 1.0e9F);
  std::array<float, 6> const ray{-31.5e9F, -31.5e9F, 0.0F, 1.0F, slope, 0.0F};
  auto const result = RunCursorProbe(world, ray);
  ASSERT_EQ(result[1], 0U);
  std::uint32_t single_x{};
  std::uint32_t single_y{};
  std::uint32_t air_intervals{};
  for (std::size_t index = 0; index < result[0]; ++index) {
    auto const offset = 2U + (12U * index);
    ASSERT_EQ(result[offset], 0U);
    if (result[offset + 1U] != 4U) {
      continue;
    }
    // At these central planes, the two ray parameters differ by many surface
    // bands. They must no longer be merged as simultaneous corner events.
    auto const x = std::bit_cast<float>(result[offset + 5U]);
    if (std::abs(x) < 10.0e9F) {
      single_x += result[offset + 2U] == 2U ? 1U : 0U;
      single_y += result[offset + 2U] == 8U ? 1U : 0U;
      air_intervals += result[offset + 4U] == 2U ? 1U : 0U;
    }
  }
  EXPECT_GT(single_x, 0U);
  EXPECT_GT(single_y, 0U);
  EXPECT_GT(air_intervals, 0U);
  auto const terminal = 2U + (12U * (static_cast<std::size_t>(result[0]) - 1U));
  EXPECT_EQ(result[terminal + 1U], 1U);
  RecordProperty("single_x_events", single_x);
  RecordProperty("single_y_events", single_y);
  RecordProperty("air_intervals", air_intervals);
}
