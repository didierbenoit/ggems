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
 * \brief Device tests of the Box record ABI and of one production Transport
 * transaction over World and Box: owner location, first boundary, crossing
 * mask and canonical commit.
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

#include "GGEMS/geometry/GGEMSBoxRecord.hh"
#include "GGEMS/geometry/GGEMSBoxPoseRecord.hh"
#include "GGEMS/geometry/GGEMSAnalyticVolumeRecord.hh"
#include "GGEMS/geometry/GGEMSWorldRecord.hh"
#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/opencl/GGEMSOpenCLContext.hh"
#include "GGEMS/opencl/GGEMSOpenCLExternal.hh"
#include "GGEMS/opencl/GGEMSOpenCLKernel.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMHostAccess.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMBuffer.hh"
#include "GGEMS/units/GGEMSBytesUnits.hh"

#include "GGEMSNavigationTestDevices.hh"
#include "GGEMSBoxFP32Oracle.hh"
#include "GGEMSFP32TestOracle.hh"

namespace {

using ggems::geometry::GGEMSBoxRecord;
using ggems::geometry::GGEMSWorldRecord;

constexpr std::uint32_t k_resolved{0U};
constexpr std::uint32_t k_unresolved{2U};

constexpr std::uint32_t k_event_world_exit{1U};
constexpr std::uint32_t k_event_entry{2U};
constexpr std::uint32_t k_event_exit{3U};

constexpr std::uint32_t k_world{0U};
constexpr std::uint32_t k_box{1U};
constexpr std::uint32_t k_exterior{0xFFFFFFFFU};

constexpr std::uint32_t k_face_minus_x{1U};
constexpr std::uint32_t k_face_plus_x{2U};
constexpr std::uint32_t k_face_minus_y{4U};
constexpr std::uint32_t k_face_plus_y{8U};
constexpr std::uint32_t k_face_minus_z{16U};
constexpr std::uint32_t k_face_plus_z{32U};

// A 2^30-pm grid unit is 1.073741824 mm. Integer grid coordinates below
// preserve the authored face/edge/corner ties through every FP32 subtraction.
constexpr std::int64_t k_unit{1'073'741'824LL};
constexpr std::int64_t k_world_half{(100 * k_unit)};

/*! \brief Asymmetric, off-center Box: x in [-20, 40], y in [-30, -10],
 * z in [-10, 20] grid units. */
constexpr GGEMSBoxRecord k_box_record{
  .half_extent_x_pm = static_cast<float>((30 * k_unit)),
  .half_extent_y_pm = static_cast<float>((10 * k_unit)),
  .half_extent_z_pm = static_cast<float>((15 * k_unit)),
  .volume_id = k_box,
  .material_id = 1U,
};
constexpr ggems::geometry::GGEMSBoxPoseRecord k_box_pose{
  .center_x_pm = static_cast<float>((10 * k_unit)),
  .center_y_pm = static_cast<float>(-(20 * k_unit)),
  .center_z_pm = static_cast<float>((5 * k_unit)),
  .axis_x_x = 1.0F,
  .axis_y_y = 1.0F,
  .axis_z_z = 1.0F,
};

/*! \brief One transaction: start, direction and the expected device answer. */
struct RayCase {
  std::array<std::int64_t, 3U> position;
  std::array<float, 3U> direction;
  std::uint32_t start_owner;
  std::uint32_t status;
  std::uint32_t event;
  std::uint32_t faces;
  std::uint32_t arrival;
};

/*! \brief Runs the probe for a batch of rays sharing one World and Box set. */
auto RunRays(ggems::ocl::GGEMSOpenCLContext &context,
             GGEMSWorldRecord const &world,
             std::span<GGEMSBoxRecord const> boxes,
             std::span<RayCase const> cases) -> void {
  auto const ray_count = static_cast<std::uint32_t>(cases.size());
  std::vector<float> positions;
  std::vector<float> directions;
  for (RayCase const &ray : cases) {
    for (auto coordinate : ray.position) {
      positions.push_back(static_cast<float>(coordinate));
    }
    directions.insert(directions.end(), ray.direction.begin(),
                      ray.direction.end());
  }

  auto make =
    [&context](auto const &values) -> ggems::ocl::GGEMSOpenCLSVMBuffer {
    auto buffer = context.CreateSVMBuffer(ggems::units::Bytes{
      static_cast<std::uint64_t>(values.size() * sizeof(values[0])),
    });
    ggems::ocl::WriteSVMFromHost(buffer, std::span{values});
    return buffer;
  };

  auto world_buffer =
    context.CreateSVMBuffer(ggems::units::Bytes{sizeof(world)});
  ggems::ocl::WriteSVMFromHost(world_buffer, world);
  ggems::geometry::GGEMSAnalyticVolumeRecord volume{};
  if (!boxes.empty()) {
    volume.kind = ggems::geometry::GGEMSAnalyticKind::Box;
    volume.parameters.box = boxes.front();
  }
  auto box_buffer =
    context.CreateSVMBuffer(ggems::units::Bytes{sizeof(volume)});
  ggems::ocl::WriteSVMFromHost(box_buffer, volume);
  auto pose_buffer =
    context.CreateSVMBuffer(ggems::units::Bytes{sizeof(k_box_pose)});
  ggems::ocl::WriteSVMFromHost(pose_buffer, k_box_pose);
  auto position_buffer = make(positions);
  auto direction_buffer = make(directions);
  std::vector<std::uint32_t> const zeros(ray_count, 0U);
  std::vector<float> const zeros64(3ULL * ray_count, 0);
  auto start_owners = make(zeros);
  auto statuses = make(zeros);
  auto events = make(zeros);
  auto faces = make(zeros);
  auto endpoints = make(zeros64);
  auto arrivals = make(zeros);

  std::filesystem::path const root{GGEMS_TEST_KERNEL_ROOT};
  auto const &program =
    ggems::ocl::GGEMSOpenCL::GetInstance().GetOrCreateProgram(
      context, root / "tests", "box_navigation_probe",
      std::format("-I{}", root.generic_string()));
  ggems::ocl::GGEMSOpenCLKernel kernel{
    context, program.CreateKernel("box_navigation_probe"),
    "box_navigation_probe"};
  kernel.SetArgSVMPointer(0U, world_buffer.GetData());
  kernel.SetArgSVMPointer(1U, box_buffer.GetData());
  kernel.SetArgSVMPointer(2U, pose_buffer.GetData());
  kernel.SetArgSVMPointer(3U, position_buffer.GetData());
  kernel.SetArgSVMPointer(4U, direction_buffer.GetData());
  kernel.SetArg(5U, static_cast<cl_uint>(ray_count));
  kernel.SetArgSVMPointer(6U, start_owners.GetData());
  kernel.SetArgSVMPointer(7U, statuses.GetData());
  kernel.SetArgSVMPointer(8U, events.GetData());
  kernel.SetArgSVMPointer(9U, faces.GetData());
  kernel.SetArgSVMPointer(10U, endpoints.GetData());
  kernel.SetArgSVMPointer(11U, arrivals.GetData());
  kernel.Run({static_cast<std::size_t>((ray_count + 63U) / 64U) * 64U}, {64U});

  std::vector<std::uint32_t> got_owner(ray_count);
  std::vector<std::uint32_t> got_status(ray_count);
  std::vector<std::uint32_t> got_event(ray_count);
  std::vector<std::uint32_t> got_faces(ray_count);
  std::vector<float> got_end(3ULL * ray_count);
  std::vector<std::uint32_t> got_arrival(ray_count);
  ggems::ocl::ReadSVMToHost(start_owners, std::span{got_owner});
  ggems::ocl::ReadSVMToHost(statuses, std::span{got_status});
  ggems::ocl::ReadSVMToHost(events, std::span{got_event});
  ggems::ocl::ReadSVMToHost(faces, std::span{got_faces});
  ggems::ocl::ReadSVMToHost(endpoints, std::span{got_end});
  ggems::ocl::ReadSVMToHost(arrivals, std::span{got_arrival});

  for (std::size_t index = 0U; index < cases.size(); ++index) {
    RayCase const &ray = cases[index];
    SCOPED_TRACE(
      std::format("case {} position ({}, {}, {}) direction ({}, {}, {})", index,
                  ray.position[0U], ray.position[1U], ray.position[2U],
                  ray.direction[0U], ray.direction[1U], ray.direction[2U]));
    EXPECT_EQ(got_status[index], ray.status);
    auto const start = ggems::test::box_oracle::PackedPoint(ray.position);
    if (ray.status == k_resolved) {
      auto const parameter = ggems::test::box_oracle::Parameter(
        world, boxes.front(), k_box_pose, start, ray.direction, ray.faces,
        ray.event == k_event_world_exit);
      auto const expected =
        ggems::test::box_oracle::Advance(start, ray.direction, parameter);
      for (std::size_t axis = 0; axis < 3; ++axis) {
        ggems::test::fp32::ExpectWithin(got_end[(3U * index) + axis],
                                        expected[axis]);
      }
    } else {
      // An unresolved transaction must not commit a position change.
      for (std::size_t axis = 0; axis < 3; ++axis) {
        EXPECT_EQ(got_end[(3U * index) + axis],
                  static_cast<float>(ray.position[axis]));
      }
    }
    if (got_owner[index] == k_exterior && got_status[index] != k_resolved) {
      // AnalyticLocate does not define ownership when location fails.
      continue;
    }
    EXPECT_EQ(got_owner[index], ray.start_owner);
    EXPECT_EQ(got_event[index], ray.event);
    EXPECT_EQ(got_faces[index], ray.faces);
    EXPECT_EQ(got_arrival[index], ray.arrival);
  }
}

auto ExpectRays(std::span<RayCase const> cases) -> void {
  GGEMSWorldRecord const world{
    .half_extent_x_pm = static_cast<float>(k_world_half),
    .half_extent_y_pm = static_cast<float>(k_world_half),
    .half_extent_z_pm = static_cast<float>(k_world_half),
  };
  std::array<GGEMSBoxRecord, 1U> const boxes{k_box_record};
  for (auto &context : ggems::ocl::GGEMSOpenCL::GetInstance().GetContext()) {
    SCOPED_TRACE(context.GetDevice().GetName());
    RunRays(context, world, boxes, cases);
  }
}

} // namespace

// =============================================================================
// =============================================================================

TEST_F(GGEMSNavigationDeviceTest, BoxRecordDeviceLayoutMatchesHost) {
  for (auto &context : ggems::ocl::GGEMSOpenCL::GetInstance().GetContext()) {
    SCOPED_TRACE(context.GetDevice().GetName());
    GGEMSBoxRecord const populated{
      .half_extent_x_pm = static_cast<float>(1'000'000'000'001ULL),
      .half_extent_y_pm = static_cast<float>(2ULL),
      .half_extent_z_pm = static_cast<float>(3'000'000'000'003ULL),
      .volume_id = 7U,
      .material_id = 8U,
    };
    std::array<GGEMSBoxRecord, 2U> boxes{{GGEMSBoxRecord{}, populated}};
    auto box_buffer =
      context.CreateSVMBuffer(ggems::units::Bytes{sizeof(boxes)});
    ggems::ocl::WriteSVMFromHost(box_buffer,
                                 std::span<GGEMSBoxRecord const>{boxes});
    std::array<std::uint64_t, 13U> layout{};
    auto layout_buffer =
      context.CreateSVMBuffer(ggems::units::Bytes{sizeof(layout)});

    std::filesystem::path const root{GGEMS_TEST_KERNEL_ROOT};
    auto const &program =
      ggems::ocl::GGEMSOpenCL::GetInstance().GetOrCreateProgram(
        context, root / "tests", "box_navigation_probe",
        std::format("-I{}", root.generic_string()));
    ggems::ocl::GGEMSOpenCLKernel kernel{
      context, program.CreateKernel("box_record_abi_probe"),
      "box_record_abi_probe"};
    kernel.SetArgSVMPointer(0U, box_buffer.GetData());
    kernel.SetArgSVMPointer(1U, layout_buffer.GetData());
    kernel.Run({1U}, {1U});
    ggems::ocl::ReadSVMToHost(layout_buffer, std::span<std::uint64_t>{layout});

    std::array<std::uint64_t, 13U> const expected{
      sizeof(GGEMSBoxRecord),
      offsetof(GGEMSBoxRecord, half_extent_x_pm),
      offsetof(GGEMSBoxRecord, half_extent_y_pm),
      offsetof(GGEMSBoxRecord, half_extent_z_pm),
      offsetof(GGEMSBoxRecord, volume_id),
      offsetof(GGEMSBoxRecord, material_id),
      alignof(GGEMSBoxRecord),
      sizeof(GGEMSBoxRecord),
      std::bit_cast<std::uint32_t>(populated.half_extent_x_pm),
      std::bit_cast<std::uint32_t>(populated.half_extent_y_pm),
      std::bit_cast<std::uint32_t>(populated.half_extent_z_pm),
      populated.volume_id,
      populated.material_id,
    };
    EXPECT_EQ(layout, expected);

    ggems::ocl::ReadSVMToHost(box_buffer, std::span<GGEMSBoxRecord>{boxes});
    EXPECT_EQ(boxes[0U].half_extent_x_pm, populated.half_extent_x_pm + 1U);
    EXPECT_EQ(boxes[0U].half_extent_y_pm, populated.half_extent_y_pm + 2U);
    EXPECT_EQ(boxes[0U].half_extent_z_pm, populated.half_extent_z_pm + 3U);
    EXPECT_EQ(boxes[0U].volume_id, populated.volume_id + 100U);
    EXPECT_EQ(boxes[0U].material_id, populated.material_id + 34U);
    EXPECT_EQ(boxes[1U].half_extent_x_pm, populated.half_extent_x_pm);
    EXPECT_EQ(boxes[1U].material_id, populated.material_id);
  }
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSNavigationDeviceTest, LocatesOwnersAndFindsAxisAlignedBoundaries) {
  std::array<RayCase, 9U> const cases{
    {
      // World start, +X through the Box: first boundary is the -X Box entry.
      {
        .position = {-(90 * k_unit), -(20 * k_unit), (5 * k_unit)},
        .direction = {1.0F, 0.0F, 0.0F},
        .start_owner = k_world,
        .status = k_resolved,
        .event = k_event_entry,
        .faces = k_face_minus_x,
        .arrival = k_box,
      },
      // Box start (strictly inside), +X: exit through the +X Box face.
      {
        .position = {0LL, -(20 * k_unit), (5 * k_unit)},
        .direction = {1.0F, 0.0F, 0.0F},
        .start_owner = k_box,
        .status = k_resolved,
        .event = k_event_exit,
        .faces = k_face_plus_x,
        .arrival = k_world,
      },
      // World start after the Box, +X: no crossing, World exit, exterior owner.
      {
        .position = {(40 * k_unit), -(20 * k_unit), (5 * k_unit)},
        .direction = {1.0F, 0.0F, 0.0F},
        .start_owner = k_world,
        .status = k_resolved,
        .event = k_event_world_exit,
        .faces = k_face_plus_x,
        .arrival = k_exterior,
      },
      // On the -X Box face pointing inward: owned by the Box, exits at +X.
      {
        .position = {-(20 * k_unit), -(20 * k_unit), (5 * k_unit)},
        .direction = {1.0F, 0.0F, 0.0F},
        .start_owner = k_box,
        .status = k_resolved,
        .event = k_event_exit,
        .faces = k_face_plus_x,
        .arrival = k_world,
      },
      // On the -X Box face pointing outward: World owner, straight to -X World.
      {
        .position = {-(20 * k_unit), -(20 * k_unit), (5 * k_unit)},
        .direction = {-1.0F, 0.0F, 0.0F},
        .start_owner = k_world,
        .status = k_resolved,
        .event = k_event_world_exit,
        .faces = k_face_minus_x,
        .arrival = k_exterior,
      },
      // Sliding along the +Y Box face plane (y = -10 grid units): no entry.
      {
        .position = {-(90 * k_unit), -(10 * k_unit), (5 * k_unit)},
        .direction = {1.0F, 0.0F, 0.0F},
        .start_owner = k_world,
        .status = k_resolved,
        .event = k_event_world_exit,
        .faces = k_face_plus_x,
        .arrival = k_exterior,
      },
      // One 1.048576-micrometer step inside the +Y face plane: true entry
      // through
      // -X.
      {
        .position =
          {
            -(90 * k_unit),
            -((10 * k_unit) + (k_unit / 1024)),
            (5 * k_unit),
          },
        .direction = {1.0F, 0.0F, 0.0F},
        .start_owner = k_world,
        .status = k_resolved,
        .event = k_event_entry,
        .faces = k_face_minus_x,
        .arrival = k_box,
      },
      // Parallel ray outside the slab: no entry.
      {
        .position =
          {
            -(90 * k_unit),
            -((10 * k_unit) - (k_unit / 1024)),
            (5 * k_unit),
          },
        .direction = {1.0F, 0.0F, 0.0F},
        .start_owner = k_world,
        .status = k_resolved,
        .event = k_event_world_exit,
        .faces = k_face_plus_x,
        .arrival = k_exterior,
      },
      // Box behind the start (moving away): no entry.
      {
        .position = {(50 * k_unit), -(20 * k_unit), (5 * k_unit)},
        .direction = {1.0F, 0.0F, 0.0F},
        .start_owner = k_world,
        .status = k_resolved,
        .event = k_event_world_exit,
        .faces = k_face_plus_x,
        .arrival = k_exterior,
      },
    },
  };

  ExpectRays(cases);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSNavigationDeviceTest, HandlesEdgesCornersTangentsAndDiagonals) {
  std::array<RayCase, 8U> const cases{
    {
      // Body diagonal entering exactly through the (-X,-Y,-Z) corner.
      {
        .position = {-(50 * k_unit), -(60 * k_unit), -(40 * k_unit)},
        .direction = {1.0F, 1.0F, 1.0F},
        .start_owner = k_world,
        .status = k_resolved,
        .event = k_event_entry,
        .faces = k_face_minus_x | k_face_minus_y | k_face_minus_z,
        .arrival = k_box,
      },
      // From that corner, inside: the +Y face limits first.
      {
        .position = {-(20 * k_unit), -(30 * k_unit), -(10 * k_unit)},
        .direction = {1.0F, 1.0F, 1.0F},
        .start_owner = k_box,
        .status = k_resolved,
        .event = k_event_exit,
        .faces = k_face_plus_y,
        .arrival = k_world,
      },
      // Entering exactly through the (-X,-Y) edge.
      {
        .position = {-(30 * k_unit), -(40 * k_unit), 0LL},
        .direction = {1.0F, 1.0F, 0.0F},
        .start_owner = k_world,
        .status = k_resolved,
        .event = k_event_entry,
        .faces = k_face_minus_x | k_face_minus_y,
        .arrival = k_box,
      },
      // Touching the (-X,-Y) edge and leaving: tangent, no entry.
      {
        .position = {-(30 * k_unit), -(20 * k_unit), 0LL},
        .direction = {1.0F, -1.0F, 0.0F},
        .start_owner = k_world,
        .status = k_resolved,
        .event = k_event_world_exit,
        .faces = k_face_minus_y,
        .arrival = k_exterior,
      },
      // Moving along the +Z face plane (z = 20 grid units) diagonally: no
      // entry.
      {
        .position = {-(90 * k_unit), -(40 * k_unit), (20 * k_unit)},
        .direction = {1.0F, 1.0F, 0.0F},
        .start_owner = k_world,
        .status = k_resolved,
        .event = k_event_world_exit,
        .faces = k_face_plus_y,
        .arrival = k_exterior,
      },
      // Oblique miss (1,2,3): World exit through +Z with an independent plane
      // intersection.
      {
        .position = {-(20 * k_unit), -(90 * k_unit), -(10 * k_unit)},
        .direction = {1.0F, 2.0F, 3.0F},
        .start_owner = k_world,
        .status = k_resolved,
        .event = k_event_world_exit,
        .faces = k_face_plus_z,
        .arrival = k_exterior,
      },
      // Edge start with both coincident faces left inward: Box owner.
      {
        .position = {-(20 * k_unit), -(30 * k_unit), 0LL},
        .direction = {1.0F, 1.0F, 0.0F},
        .start_owner = k_box,
        .status = k_resolved,
        .event = k_event_exit,
        .faces = k_face_plus_y,
        .arrival = k_world,
      },
      // Edge start with one coincident face left outward: World owner.
      {
        .position = {-(20 * k_unit), -(30 * k_unit), 0LL},
        .direction = {1.0F, -1.0F, 0.0F},
        .start_owner = k_world,
        .status = k_resolved,
        .event = k_event_world_exit,
        .faces = k_face_minus_y,
        .arrival = k_exterior,
      },
    },
  };

  ExpectRays(cases);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSNavigationDeviceTest, ReportsUnresolvedAndExactTies) {
  std::array<RayCase, 2U> const cases{
    {
      // Zero direction inside the Box: unresolved, position and owner
      // unchanged.
      {
        .position = {0LL, -(20 * k_unit), (5 * k_unit)},
        .direction = {0.0F, 0.0F, 0.0F},
        .start_owner = k_box,
        .status = k_unresolved,
        .event = 0U,
        .faces = 0U,
        .arrival = k_box,
      },
      // Inside the Box, exact simultaneous exit through the (+X,+Y) edge:
      // 30 grid units / 1 along X equals 15 grid units / 0.5 along Y.
      {
        .position = {(10 * k_unit), -(25 * k_unit), (5 * k_unit)},
        .direction = {1.0F, 0.5F, 0.0F},
        .start_owner = k_box,
        .status = k_resolved,
        .event = k_event_exit,
        .faces = k_face_plus_x | k_face_plus_y,
        .arrival = k_world,
      },
    },
  };

  ExpectRays(cases);
}
