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
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <span>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/geometry/GGEMSAnalyticVolumeRecord.hh"
#include "GGEMS/geometry/GGEMSWorldRecord.hh"
#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/opencl/GGEMSOpenCLContext.hh"
#include "GGEMS/opencl/GGEMSOpenCLExternal.hh"
#include "GGEMS/opencl/GGEMSOpenCLKernel.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMBuffer.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMHostAccess.hh"
#include "GGEMS/units/GGEMSBytesUnits.hh"

#include "GGEMSNavigationTestDevices.hh"
#include "GGEMSWorldTestOracle.hh"
#include "GGEMSFP32TestOracle.hh"

namespace {

using WorldRecord = ggems::geometry::GGEMSWorldRecord;

// Navigation status codes and face bits of kernels/navigation.
constexpr std::uint32_t k_exit{0U};
constexpr std::uint32_t k_outside_world{1U};
constexpr std::uint32_t k_invalid_direction{2U};

constexpr std::uint32_t k_face_minus_x{1U};
constexpr std::uint32_t k_face_plus_x{2U};
constexpr std::uint32_t k_face_minus_y{4U};
constexpr std::uint32_t k_face_plus_y{8U};
constexpr std::uint32_t k_face_minus_z{16U};
constexpr std::uint32_t k_face_plus_z{32U};

constexpr std::int64_t k_100_mm{100'000'000'000LL};
constexpr std::int64_t k_mm{1'000'000'000LL};

/*! \brief Sentinel left in untouched endpoint slots. */
constexpr std::int64_t k_untouched{0x7E7E'7E7E'7E7E'7E7ELL};

/*! \brief Stored binary32 components of normalize(1, 1, 1). */
constexpr float k_third{0x1.279a74p-1F};

/*! \brief One ray with its independently derived expectation. */
struct RayCase {
  std::array<std::int64_t, 3U> position;
  std::array<float, 3U> direction;
  std::uint32_t status;
  std::uint32_t faces;
};

/*!
 * \brief Runs the navigation probe kernel for one World on one context.
 *
 * Endpoints use independent binary64 represented-plane intersections and
 * operand-specific FP32 error propagation. Discrete expectations remain exact.
 */
auto RunCases(ggems::ocl::GGEMSOpenCLContext &context, WorldRecord const &world,
              std::span<RayCase const> cases) -> void {
  auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();

  std::vector<float> positions;
  std::vector<float> directions;
  std::vector<std::uint32_t> statuses(cases.size(), 0xFFFF'FFFFU);
  std::vector<float> endpoints(3U * cases.size(),
                               static_cast<float>(k_untouched));
  std::vector<std::uint32_t> faces(cases.size(), 0xFFFF'FFFFU);

  for (RayCase const &ray : cases) {
    for (auto coordinate : ray.position) {
      positions.push_back(static_cast<float>(coordinate));
    }
    directions.insert(directions.end(), ray.direction.begin(),
                      ray.direction.end());
  }

  auto bytes = [](auto const &vector) -> ggems::units::Bytes {
    return ggems::units::Bytes{vector.size() * sizeof(vector[0U])};
  };

  auto world_buffer =
    context.CreateSVMBuffer(ggems::units::Bytes{sizeof(WorldRecord)});
  // The World-only probe uses the common single-record ABI.
  ggems::geometry::GGEMSAnalyticVolumeRecord const no_box{};
  auto box_buffer =
    context.CreateSVMBuffer(ggems::units::Bytes{sizeof(no_box)});
  auto positions_buffer = context.CreateSVMBuffer(bytes(positions));
  auto directions_buffer = context.CreateSVMBuffer(bytes(directions));
  auto statuses_buffer = context.CreateSVMBuffer(bytes(statuses));
  auto endpoints_buffer = context.CreateSVMBuffer(bytes(endpoints));
  auto faces_buffer = context.CreateSVMBuffer(bytes(faces));

  ggems::ocl::WriteSVMFromHost(world_buffer, world);
  ggems::ocl::WriteSVMFromHost(box_buffer, no_box);
  ggems::ocl::WriteSVMFromHost(positions_buffer, std::span{positions});
  ggems::ocl::WriteSVMFromHost(directions_buffer, std::span{directions});
  ggems::ocl::WriteSVMFromHost(statuses_buffer, std::span{statuses});
  ggems::ocl::WriteSVMFromHost(endpoints_buffer, std::span{endpoints});
  ggems::ocl::WriteSVMFromHost(faces_buffer, std::span{faces});

  std::filesystem::path const kernel_root{GGEMS_TEST_KERNEL_ROOT};
  auto const &program = opencl.GetOrCreateProgram(
    context, kernel_root / "tests", "world_navigation_probe",
    std::format("-I{}", kernel_root.generic_string()));
  cl::Kernel raw_kernel = program.CreateKernel("world_navigation_probe");
  ggems::ocl::GGEMSOpenCLKernel kernel{context, std::move(raw_kernel),
                                       "world_navigation_probe"};

  kernel.SetArgSVMPointer(0U, world_buffer.GetData());
  kernel.SetArgSVMPointer(1U, box_buffer.GetData());
  kernel.SetArgSVMPointer(2U, positions_buffer.GetData());
  kernel.SetArgSVMPointer(3U, directions_buffer.GetData());
  kernel.SetArg(4U, static_cast<cl_uint>(cases.size()));
  kernel.SetArgSVMPointer(5U, statuses_buffer.GetData());
  kernel.SetArgSVMPointer(6U, endpoints_buffer.GetData());
  kernel.SetArgSVMPointer(7U, faces_buffer.GetData());
  kernel.Run({cases.size()}, {1U});

  ggems::ocl::ReadSVMToHost(statuses_buffer, std::span{statuses});
  ggems::ocl::ReadSVMToHost(endpoints_buffer, std::span{endpoints});
  ggems::ocl::ReadSVMToHost(faces_buffer, std::span{faces});

  for (std::size_t index = 0U; index < cases.size(); ++index) {
    RayCase const &ray = cases[index];
    SCOPED_TRACE(std::format("case {} position ({}, {}, {}) direction ({}, "
                             "{}, {})",
                             index, ray.position[0U], ray.position[1U],
                             ray.position[2U], ray.direction[0U],
                             ray.direction[1U], ray.direction[2U]));

    EXPECT_EQ(statuses[index], ray.status);
    EXPECT_EQ(faces[index], ray.faces);

    std::array<float, 3> const start{
      positions[3U * index],
      positions[(3U * index) + 1U],
      positions[(3U * index) + 2U],
    };
    auto const reference =
      ggems::test::WorldReference(world, start, ray.direction);
    EXPECT_EQ(reference.status, ray.status);
    EXPECT_EQ(reference.faces, ray.faces);
    for (std::size_t axis = 0U; axis < 3U; ++axis) {
      ggems::test::fp32::ExpectWithin(endpoints[(3U * index) + axis],
                                      reference.point[axis]);
    }
  }
}

auto ForEachContext(auto &&body) -> void {
  for (auto &context : ggems::ocl::GGEMSOpenCL::GetInstance().GetContext()) {
    SCOPED_TRACE(context.GetDevice().GetName());
    body(context);
  }
}

} // namespace

// =============================================================================
// =============================================================================

TEST_F(GGEMSNavigationDeviceTest, ExitsTheCubicWorldAlongAxesAndDiagonals) {
  WorldRecord const world{
    .half_extent_x_pm = static_cast<float>(k_100_mm),
    .half_extent_y_pm = static_cast<float>(k_100_mm),
    .half_extent_z_pm = static_cast<float>(k_100_mm),
  };

  std::array<RayCase, 10U> const cases{
    {
      // W01: center to +X, 100 mm of travel to the +X face.
      {
        .position = {0, 0, 0},
        .direction = {1.0F, 0.0F, 0.0F},
        .status = k_exit,
        .faces = k_face_plus_x,
      },
      {
        .position = {0, 0, 0},
        .direction = {-1.0F, 0.0F, 0.0F},
        .status = k_exit,
        .faces = k_face_minus_x,
      },
      {
        .position = {0, 0, 0},
        .direction = {0.0F, 1.0F, 0.0F},
        .status = k_exit,
        .faces = k_face_plus_y,
      },
      {
        .position = {0, 0, 0},
        .direction = {0.0F, -1.0F, 0.0F},
        .status = k_exit,
        .faces = k_face_minus_y,
      },
      {
        .position = {0, 0, 0},
        .direction = {0.0F, 0.0F, 1.0F},
        .status = k_exit,
        .faces = k_face_plus_z,
      },
      {
        .position = {0, 0, 0},
        .direction = {0.0F, 0.0F, -1.0F},
        .status = k_exit,
        .faces = k_face_minus_z,
      },
      // Off-center axis-aligned start: 70 mm of travel to the -X face.
      {
        .position = {-30'000'000'000LL, 12'000'000'000LL, -7'000'000'000LL},
        .direction = {-1.0F, 0.0F, 0.0F},
        .status = k_exit,
        .faces = k_face_minus_x,
      },
      // Body diagonal with equal stored components: exact corner exit.
      {
        .position = {0, 0, 0},
        .direction = {k_third, k_third, k_third},
        .status = k_exit,
        .faces = k_face_plus_x | k_face_plus_y | k_face_plus_z,
      },
      {
        .position = {0, 0, 0},
        .direction = {-k_third, k_third, -k_third},
        .status = k_exit,
        .faces = k_face_minus_x | k_face_plus_y | k_face_minus_z,
      },
      // Off-center diagonal: y limits (80 mm of y travel), x and z follow
      // exactly because the stored components are equal.
      {
        .position = {-30'000'000'000LL, 20'000'000'000LL, -50'000'000'000LL},
        .direction = {k_third, k_third, k_third},
        .status = k_exit,
        .faces = k_face_plus_y,
      },
    },
  };

  ForEachContext(
    [&](auto &context) -> void { RunCases(context, world, cases); });
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSNavigationDeviceTest,
       ExitsAsymmetricWorldAlongStoredFP32Directions) {
  WorldRecord const world{
    .half_extent_x_pm = static_cast<float>(k_100_mm),
    .half_extent_y_pm = static_cast<float>(2 * k_100_mm),
    .half_extent_z_pm = static_cast<float>(50'000'000'000LL),
  };

  // Stored binary32 components of normalize(1, 2, 3) and normalize(0, .6, .8).
  constexpr std::array<float, 3U> k_dir_123{
    0.267261237F,
    0.534522474F,
    0.801783741F,
  };
  constexpr std::array<float, 3U> k_dir_068{0.0F, 0.600000024F, 0.800000012F};

  std::array<RayCase, 5U> const cases{
    {
      // The represented +Z plane limits; use the stored (not ideal) direction.
      {
        .position = {0, 0, 0},
        .direction = k_dir_123,
        .status = k_exit,
        .faces = k_face_plus_z,
      },
      // Oracle: same direction from an off-center start.
      {
        .position = {-70'000'000'000LL, 30'000'000'000LL, -40'000'000'000LL},
        .direction = k_dir_123,
        .status = k_exit,
        .faces = k_face_plus_z,
      },
      // Body diagonal of the asymmetric World: z limits, x and y follow.
      {
        .position = {0, 0, 0},
        .direction = {k_third, k_third, k_third},
        .status = k_exit,
        .faces = k_face_plus_z,
      },
      // Start on the +X face sliding along it: x stays coincident, z limits.
      {
        .position = {k_100_mm, 0, 0},
        .direction = k_dir_068,
        .status = k_exit,
        .faces = k_face_plus_z,
      },
      // Start on the (+X, -Y) edge moving along z: both faces stay coincident.
      {
        .position = {k_100_mm, -2 * k_100_mm, 0},
        .direction = {0.0F, 0.0F, 1.0F},
        .status = k_exit,
        .faces = k_face_plus_z,
      },
    },
  };

  ForEachContext(
    [&](auto &context) -> void { RunCases(context, world, cases); });
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSNavigationDeviceTest,
       MovesAlongOrdinaryObliqueRaysWithoutIntegerRounding) {
  WorldRecord const world{
    .half_extent_x_pm = static_cast<float>(10 * k_mm),
    .half_extent_y_pm = static_cast<float>(10 * k_mm),
    .half_extent_z_pm = static_cast<float>(10 * k_mm),
  };
  // The legacy 20 pm integer-half-rounding scene is now a 20 mm physical
  // scene. Device geometry preserves fractional coordinates; observer rounding
  // is tested separately by the transport tests.
  std::array<RayCase, 8U> const cases{
    {
      {.position = {-k_mm, 0, 0},
       .direction = {1, 0.5F, 0},
       .status = k_exit,
       .faces = k_face_plus_x},
      {.position = {-k_mm, -3 * k_mm, 0},
       .direction = {1, 0.5F, 0},
       .status = k_exit,
       .faces = k_face_plus_x},
      {.position = {-k_mm, -6 * k_mm, 0},
       .direction = {1, 0.5F, 0},
       .status = k_exit,
       .faces = k_face_plus_x},
      {.position = {-k_mm, 0, 0},
       .direction = {1, -0.5F, 0},
       .status = k_exit,
       .faces = k_face_plus_x},
      {.position = {-k_mm, 6 * k_mm, 0},
       .direction = {1, -0.5F, 0},
       .status = k_exit,
       .faces = k_face_plus_x},
      {.position = {0, 0, 0},
       .direction = {1, 0.05F, 0},
       .status = k_exit,
       .faces = k_face_plus_x},
      {.position = {0, -9 * k_mm, 0},
       .direction = {1, -0.05F, 0},
       .status = k_exit,
       .faces = k_face_plus_x},
      {
        .position = {-10 * k_mm, -10 * k_mm, 0},
        .direction = {1, 1, 0},
        .status = k_exit,
        .faces = k_face_plus_x | k_face_plus_y,
      },
    },
  };
  ForEachContext(
    [&](auto &context) -> void { RunCases(context, world, cases); });
}

TEST_F(GGEMSNavigationDeviceTest, HandlesBoundaryStartsAndInvalidInputs) {
  WorldRecord const world{
    .half_extent_x_pm = static_cast<float>(k_100_mm),
    .half_extent_y_pm = static_cast<float>(k_100_mm),
    .half_extent_z_pm = static_cast<float>(k_100_mm),
  };
  std::array<RayCase, 9U> const cases{
    {
      // Start on the -X face pointing inward: full 200 mm crossing.
      {
        .position = {-k_100_mm, 12'000'000'000LL, -7'000'000'000LL},
        .direction = {1.0F, 0.0F, 0.0F},
        .status = k_exit,
        .faces = k_face_plus_x,
      },
      // Start on the -X face pointing outward: zero-distance valid exit.
      {
        .position = {-k_100_mm, 12'000'000'000LL, -7'000'000'000LL},
        .direction = {-1.0F, 0.0F, 0.0F},
        .status = k_exit,
        .faces = k_face_minus_x,
      },
      // Start on a corner pointing outward on every axis.
      {
        .position = {k_100_mm, k_100_mm, k_100_mm},
        .direction = {k_third, k_third, k_third},
        .status = k_exit,
        .faces = k_face_plus_x | k_face_plus_y | k_face_plus_z,
      },
      // Start on a corner pointing inward along the body diagonal.
      {
        .position = {k_100_mm, k_100_mm, k_100_mm},
        .direction = {-k_third, -k_third, -k_third},
        .status = k_exit,
        .faces = k_face_minus_x | k_face_minus_y | k_face_minus_z,
      },
      // Start on the +X face with a direction parallel to it.
      {
        .position = {k_100_mm, 0, 0},
        .direction = {0.0F, 0.0F, -1.0F},
        .status = k_exit,
        .faces = k_face_minus_z,
      },
      // Negative zero components are parallel, not invalid.
      {
        .position = {0, 0, 0},
        .direction = {-0.0F, 1.0F, -0.0F},
        .status = k_exit,
        .faces = k_face_plus_y,
      },
      // One micrometer outside: represented outside gap, positions untouched.
      {
        .position = {k_100_mm + 1'000'000, 0, 0},
        .direction = {-1.0F, 0.0F, 0.0F},
        .status = k_outside_world,
        .faces = 0U,
      },
      {
        .position = {0, 0, -k_100_mm - 1'000'000},
        .direction = {0.0F, 0.0F, 1.0F},
        .status = k_outside_world,
        .faces = 0U,
      },
      // An all-zero direction is rejected before any geometry.
      {
        .position = {0, 0, 0},
        .direction = {0.0F, -0.0F, 0.0F},
        .status = k_invalid_direction,
        .faces = 0U,
      },
    },
  };

  ForEachContext(
    [&](auto &context) -> void { RunCases(context, world, cases); });
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSNavigationDeviceTest,
       HandlesThreeMeterWorldAndSmallDirectionComponents) {
  constexpr std::int64_t half{1500 * k_mm};
  WorldRecord const world{
    .half_extent_x_pm = static_cast<float>(half),
    .half_extent_y_pm = static_cast<float>(half),
    .half_extent_z_pm = static_cast<float>(half),
  };
  std::array<RayCase, 6U> const cases{
    {
      {.position = {-half, 0, 0},
       .direction = {1, 0, 0},
       .status = k_exit,
       .faces = k_face_plus_x},
      {.position = {half, half, -half},
       .direction = {0, -1, 0},
       .status = k_exit,
       .faces = k_face_minus_y},
      {
        .position = {0, 0, 0},
        .direction = {k_third, k_third, k_third},
        .status = k_exit,
        .faces = k_face_plus_x | k_face_plus_y | k_face_plus_z,
      },
      {.position = {0, 0, 0},
       .direction = {1, 1.0e-4F, 0},
       .status = k_exit,
       .faces = k_face_plus_x},
      {.position = {-half - k_mm, 0, 0},
       .direction = {1, 0, 0},
       .status = k_outside_world,
       .faces = 0},
      {.position = {0, half + k_mm, 0},
       .direction = {0, -1, 0},
       .status = k_outside_world,
       .faces = 0},
    },
  };
  WorldRecord const cube{
    .half_extent_x_pm = static_cast<float>(k_100_mm),
    .half_extent_y_pm = static_cast<float>(k_100_mm),
    .half_extent_z_pm = static_cast<float>(k_100_mm),
  };
  std::array<RayCase, 2U> const small_component_cases{
    {
      {.position = {0, 0, 0},
       .direction = {1, 1.0e-3F, 0},
       .status = k_exit,
       .faces = k_face_plus_x},
      {.position = {0, 0, 0},
       .direction = {-1, 0, 1.0e-4F},
       .status = k_exit,
       .faces = k_face_minus_x},
    },
  };
  // Extreme integer Worlds and subnormal directions are retired here.
  // Normal small components still require the expected nonzero transverse
  // motion, exact exit status and exact limiting face on every context.
  ForEachContext([&](auto &context) -> void {
    RunCases(context, world, cases);
    RunCases(context, cube, small_component_cases);
  });
}
