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
#include <limits>
#include <span>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/geometry/GGEMSWorldRecord.hh"
#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/opencl/GGEMSOpenCLContext.hh"
#include "GGEMS/opencl/GGEMSOpenCLKernel.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMBuffer.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMHostAccess.hh"
#include "GGEMS/units/GGEMSBytesUnits.hh"

#include "GGEMSNavigationTestDevices.hh"

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
constexpr std::int64_t k_max_half_pm{0x3FFF'FFFF'FFFF'FFFFLL};
constexpr std::int64_t k_int64_min = std::numeric_limits<std::int64_t>::min();
constexpr std::int64_t k_int64_max = std::numeric_limits<std::int64_t>::max();

/*! \brief Sentinel left in untouched endpoint slots. */
constexpr std::int64_t k_untouched{0x7E7E'7E7E'7E7E'7E7ELL};

/*! \brief Stored binary32 components of normalize(1, 1, 1). */
constexpr float k_third{0.5773502588272095F};

/*! \brief One ray with its independently derived expectation. */
struct RayCase {
  std::array<std::int64_t, 3U> position;
  std::array<float, 3U> direction;
  std::uint32_t status;
  std::array<std::int64_t, 3U> endpoint;
  std::uint32_t faces;
};

/*!
 * \brief Runs the navigation probe kernel for one World on one context.
 *
 * Expected values come from an exact rational oracle (claude_scratch
 * world_exit_oracle.py) or from hand calculation; they never come from the
 * kernel under test.
 */
auto RunCases(ggems::ocl::GGEMSOpenCLContext &context, WorldRecord const &world,
              std::span<RayCase const> cases) -> void {
  auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();

  std::vector<std::int64_t> positions;
  std::vector<float> directions;
  std::vector<std::uint32_t> statuses(cases.size(), 0xFFFF'FFFFU);
  std::vector<std::int64_t> endpoints(3U * cases.size(), k_untouched);
  std::vector<std::uint32_t> faces(cases.size(), 0xFFFF'FFFFU);

  for (RayCase const &ray : cases) {
    positions.insert(positions.end(), ray.position.begin(), ray.position.end());
    directions.insert(directions.end(), ray.direction.begin(),
                      ray.direction.end());
  }

  auto bytes = [](auto const &vector) -> ggems::units::Bytes {
    return ggems::units::Bytes{vector.size() * sizeof(vector[0U])};
  };

  auto world_buffer =
    context.CreateSVMBuffer(ggems::units::Bytes{sizeof(WorldRecord)});
  auto positions_buffer = context.CreateSVMBuffer(bytes(positions));
  auto directions_buffer = context.CreateSVMBuffer(bytes(directions));
  auto statuses_buffer = context.CreateSVMBuffer(bytes(statuses));
  auto endpoints_buffer = context.CreateSVMBuffer(bytes(endpoints));
  auto faces_buffer = context.CreateSVMBuffer(bytes(faces));

  ggems::ocl::WriteSVMFromHost(world_buffer, world);
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
  kernel.SetArgSVMPointer(1U, positions_buffer.GetData());
  kernel.SetArgSVMPointer(2U, directions_buffer.GetData());
  kernel.SetArg(3U, static_cast<cl_uint>(cases.size()));
  kernel.SetArgSVMPointer(4U, statuses_buffer.GetData());
  kernel.SetArgSVMPointer(5U, endpoints_buffer.GetData());
  kernel.SetArgSVMPointer(6U, faces_buffer.GetData());
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

    for (std::size_t axis = 0U; axis < 3U; ++axis) {
      EXPECT_EQ(endpoints[(3U * index) + axis], ray.endpoint[axis]);
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
  WorldRecord const world{.half_extent_x_pm = k_100_mm,
                          .half_extent_y_pm = k_100_mm,
                          .half_extent_z_pm = k_100_mm};

  std::array<RayCase, 10U> const cases{{
    // W01: center to +X, 100 mm of travel to the +X face.
    {.position = {0, 0, 0},
     .direction = {1.0F, 0.0F, 0.0F},
     .status = k_exit,
     .endpoint = {k_100_mm, 0, 0},
     .faces = k_face_plus_x},
    {.position = {0, 0, 0},
     .direction = {-1.0F, 0.0F, 0.0F},
     .status = k_exit,
     .endpoint = {-k_100_mm, 0, 0},
     .faces = k_face_minus_x},
    {.position = {0, 0, 0},
     .direction = {0.0F, 1.0F, 0.0F},
     .status = k_exit,
     .endpoint = {0, k_100_mm, 0},
     .faces = k_face_plus_y},
    {.position = {0, 0, 0},
     .direction = {0.0F, -1.0F, 0.0F},
     .status = k_exit,
     .endpoint = {0, -k_100_mm, 0},
     .faces = k_face_minus_y},
    {.position = {0, 0, 0},
     .direction = {0.0F, 0.0F, 1.0F},
     .status = k_exit,
     .endpoint = {0, 0, k_100_mm},
     .faces = k_face_plus_z},
    {.position = {0, 0, 0},
     .direction = {0.0F, 0.0F, -1.0F},
     .status = k_exit,
     .endpoint = {0, 0, -k_100_mm},
     .faces = k_face_minus_z},
    // Off-center axis-aligned start: 70 mm of travel to the -X face.
    {.position = {-30'000'000'000LL, 12'345LL, -6'789LL},
     .direction = {-1.0F, 0.0F, 0.0F},
     .status = k_exit,
     .endpoint = {-k_100_mm, 12'345LL, -6'789LL},
     .faces = k_face_minus_x},
    // Body diagonal with equal stored components: exact corner exit.
    {.position = {0, 0, 0},
     .direction = {k_third, k_third, k_third},
     .status = k_exit,
     .endpoint = {k_100_mm, k_100_mm, k_100_mm},
     .faces = k_face_plus_x | k_face_plus_y | k_face_plus_z},
    {.position = {0, 0, 0},
     .direction = {-k_third, k_third, -k_third},
     .status = k_exit,
     .endpoint = {-k_100_mm, k_100_mm, -k_100_mm},
     .faces = k_face_minus_x | k_face_plus_y | k_face_minus_z},
    // Off-center diagonal: y limits (80 mm of y travel), x and z follow
    // exactly because the stored components are equal.
    {.position = {-30'000'000'000LL, 20'000'000'000LL, -50'000'000'000LL},
     .direction = {k_third, k_third, k_third},
     .status = k_exit,
     .endpoint = {50'000'000'000LL, k_100_mm, 30'000'000'000LL},
     .faces = k_face_plus_y},
  }};

  ForEachContext(
    [&](auto &context) -> void { RunCases(context, world, cases); });
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSNavigationDeviceTest, CommitsExactlyRoundedLateralCoordinates) {
  WorldRecord const world{.half_extent_x_pm = k_100_mm,
                          .half_extent_y_pm = 2 * k_100_mm,
                          .half_extent_z_pm = k_100_mm / 2};

  // Stored binary32 components of normalize(1, 2, 3) and normalize(0, .6, .8).
  constexpr std::array<float, 3U> k_dir_123{0.267261237F, 0.534522474F,
                                            0.801783741F};
  constexpr std::array<float, 3U> k_dir_068{0.0F, 0.600000024F, 0.800000012F};

  std::array<RayCase, 5U> const cases{{
    // Oracle: z limits at s = 50 mm / 0.801783741; x and y are the exactly
    // rounded rational coordinates of the stored (not ideal) direction.
    {.position = {0, 0, 0},
     .direction = k_dir_123,
     .status = k_exit,
     .endpoint = {16'666'666'047LL, 33'333'332'094LL, 50'000'000'000LL},
     .faces = k_face_plus_z},
    // Oracle: same direction from an off-center start.
    {.position = {-70'000'000'000LL, 30'000'000'000LL, -40'000'000'000LL},
     .direction = k_dir_123,
     .status = k_exit,
     .endpoint = {-40'000'001'115LL, 89'999'997'770LL, 50'000'000'000LL},
     .faces = k_face_plus_z},
    // Body diagonal of the asymmetric World: z limits, x and y follow.
    {.position = {0, 0, 0},
     .direction = {k_third, k_third, k_third},
     .status = k_exit,
     .endpoint = {50'000'000'000LL, 50'000'000'000LL, 50'000'000'000LL},
     .faces = k_face_plus_z},
    // Start on the +X face sliding along it: x stays coincident, z limits.
    {.position = {k_100_mm, 0, 0},
     .direction = k_dir_068,
     .status = k_exit,
     .endpoint = {k_100_mm, 37'500'000'931LL, 50'000'000'000LL},
     .faces = k_face_plus_z},
    // Start on the (+X, -Y) edge moving along z: both faces stay coincident.
    {.position = {k_100_mm, -2 * k_100_mm, 0},
     .direction = {0.0F, 0.0F, 1.0F},
     .status = k_exit,
     .endpoint = {k_100_mm, -2 * k_100_mm, 50'000'000'000LL},
     .faces = k_face_plus_z},
  }};

  ForEachContext(
    [&](auto &context) -> void { RunCases(context, world, cases); });
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSNavigationDeviceTest, RoundsHalfPicometerSumsAwayFromZero) {
  WorldRecord const world{
    .half_extent_x_pm = 10, .half_extent_y_pm = 10, .half_extent_z_pm = 10};

  std::array<RayCase, 8U> const cases{{
    // x travels 11 pm, y moves 5.5 pm: sums 5.5, 2.5, -0.5, -5.5, 0.5.
    {.position = {-1, 0, 0},
     .direction = {1.0F, 0.5F, 0.0F},
     .status = k_exit,
     .endpoint = {10, 6, 0},
     .faces = k_face_plus_x},
    {.position = {-1, -3, 0},
     .direction = {1.0F, 0.5F, 0.0F},
     .status = k_exit,
     .endpoint = {10, 3, 0},
     .faces = k_face_plus_x},
    // -6 + 5.5 = -0.5 rounds away from zero to -1, not to -6 + 6 = 0.
    {.position = {-1, -6, 0},
     .direction = {1.0F, 0.5F, 0.0F},
     .status = k_exit,
     .endpoint = {10, -1, 0},
     .faces = k_face_plus_x},
    {.position = {-1, 0, 0},
     .direction = {1.0F, -0.5F, 0.0F},
     .status = k_exit,
     .endpoint = {10, -6, 0},
     .faces = k_face_plus_x},
    {.position = {-1, 6, 0},
     .direction = {1.0F, -0.5F, 0.0F},
     .status = k_exit,
     .endpoint = {10, 1, 0},
     .faces = k_face_plus_x},
    // 0.05f is slightly above 0.05: 10 * 0.05f rounds up to 1.
    {.position = {0, 0, 0},
     .direction = {1.0F, 0.05F, 0.0F},
     .status = k_exit,
     .endpoint = {10, 1, 0},
     .faces = k_face_plus_x},
    // -9 - 10 * 0.05f = -9.5000000745 rounds to -10: the endpoint lands on
    // the -Y face as well.
    {.position = {0, -9, 0},
     .direction = {1.0F, -0.05F, 0.0F},
     .status = k_exit,
     .endpoint = {10, -10, 0},
     .faces = k_face_plus_x},
    // Exact tie of x and y parameters: both faces are reached.
    {.position = {-10, -10, 0},
     .direction = {1.0F, 1.0F, 0.0F},
     .status = k_exit,
     .endpoint = {10, 10, 0},
     .faces = k_face_plus_x | k_face_plus_y},
  }};

  ForEachContext(
    [&](auto &context) -> void { RunCases(context, world, cases); });
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSNavigationDeviceTest, HandlesBoundaryStartsAndInvalidInputs) {
  WorldRecord const world{.half_extent_x_pm = k_100_mm,
                          .half_extent_y_pm = k_100_mm,
                          .half_extent_z_pm = k_100_mm};
  std::array<RayCase, 9U> const cases{{
    // Start on the -X face pointing inward: full 200 mm crossing.
    {.position = {-k_100_mm, 12'345LL, -6'789LL},
     .direction = {1.0F, 0.0F, 0.0F},
     .status = k_exit,
     .endpoint = {k_100_mm, 12'345LL, -6'789LL},
     .faces = k_face_plus_x},
    // Start on the -X face pointing outward: zero-distance valid exit.
    {.position = {-k_100_mm, 12'345LL, -6'789LL},
     .direction = {-1.0F, 0.0F, 0.0F},
     .status = k_exit,
     .endpoint = {-k_100_mm, 12'345LL, -6'789LL},
     .faces = k_face_minus_x},
    // Start on a corner pointing outward on every axis.
    {.position = {k_100_mm, k_100_mm, k_100_mm},
     .direction = {k_third, k_third, k_third},
     .status = k_exit,
     .endpoint = {k_100_mm, k_100_mm, k_100_mm},
     .faces = k_face_plus_x | k_face_plus_y | k_face_plus_z},
    // Start on a corner pointing inward along the body diagonal.
    {.position = {k_100_mm, k_100_mm, k_100_mm},
     .direction = {-k_third, -k_third, -k_third},
     .status = k_exit,
     .endpoint = {-k_100_mm, -k_100_mm, -k_100_mm},
     .faces = k_face_minus_x | k_face_minus_y | k_face_minus_z},
    // Start on the +X face with a direction parallel to it.
    {.position = {k_100_mm, 0, 0},
     .direction = {0.0F, 0.0F, -1.0F},
     .status = k_exit,
     .endpoint = {k_100_mm, 0, -k_100_mm},
     .faces = k_face_minus_z},
    // Negative zero components are parallel, not invalid.
    {.position = {0, 0, 0},
     .direction = {-0.0F, 1.0F, -0.0F},
     .status = k_exit,
     .endpoint = {0, k_100_mm, 0},
     .faces = k_face_plus_y},
    // One picometer outside: invalid user input, positions untouched.
    {.position = {k_100_mm + 1, 0, 0},
     .direction = {-1.0F, 0.0F, 0.0F},
     .status = k_outside_world,
     .endpoint = {k_100_mm + 1, 0, 0},
     .faces = 0U},
    {.position = {0, 0, -k_100_mm - 1},
     .direction = {0.0F, 0.0F, 1.0F},
     .status = k_outside_world,
     .endpoint = {0, 0, -k_100_mm - 1},
     .faces = 0U},
    // An all-zero direction is rejected before any geometry. Nonfinite
    // components are host-validated; the device bit test for them is not
    // exercised here because finite-math build options may legally elide it.
    {.position = {0, 0, 0},
     .direction = {0.0F, -0.0F, 0.0F},
     .status = k_invalid_direction,
     .endpoint = {0, 0, 0},
     .faces = 0U},
  }};

  ForEachContext(
    [&](auto &context) -> void { RunCases(context, world, cases); });
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSNavigationDeviceTest, HandlesTheLargestWorldAndTinyComponents) {
  WorldRecord const largest{.half_extent_x_pm = k_max_half_pm,
                            .half_extent_y_pm = k_max_half_pm,
                            .half_extent_z_pm = k_max_half_pm};

  std::array<RayCase, 6U> const largest_cases{{
    // Full crossing of 2^63 - 2 pm along x without overflow.
    {.position = {-k_max_half_pm, 0, 0},
     .direction = {1.0F, 0.0F, 0.0F},
     .status = k_exit,
     .endpoint = {k_max_half_pm, 0, 0},
     .faces = k_face_plus_x},
    {.position = {k_max_half_pm, k_max_half_pm, -k_max_half_pm},
     .direction = {0.0F, -1.0F, 0.0F},
     .status = k_exit,
     .endpoint = {k_max_half_pm, -k_max_half_pm, -k_max_half_pm},
     .faces = k_face_minus_y},
    // Body diagonal to the corner beyond binary64 integer precision.
    {.position = {0, 0, 0},
     .direction = {k_third, k_third, k_third},
     .status = k_exit,
     .endpoint = {k_max_half_pm, k_max_half_pm, k_max_half_pm},
     .faces = k_face_plus_x | k_face_plus_y | k_face_plus_z},
    // Tiny transverse component: 2^62 pm * 1e-20 rounds to zero.
    {.position = {0, 0, 0},
     .direction = {1.0F, 1.0e-20F, 0.0F},
     .status = k_exit,
     .endpoint = {k_max_half_pm, 0, 0},
     .faces = k_face_plus_x},
    // Signed extremes are outside the largest admitted World.
    {.position = {k_int64_min, 0, 0},
     .direction = {1.0F, 0.0F, 0.0F},
     .status = k_outside_world,
     .endpoint = {k_int64_min, 0, 0},
     .faces = 0U},
    {.position = {0, k_int64_max, 0},
     .direction = {0.0F, -1.0F, 0.0F},
     .status = k_outside_world,
     .endpoint = {0, k_int64_max, 0},
     .faces = 0U},
  }};

  WorldRecord const cube{.half_extent_x_pm = k_100_mm,
                         .half_extent_y_pm = k_100_mm,
                         .half_extent_z_pm = k_100_mm};
  float const denorm = std::numeric_limits<float>::denorm_min();

  std::array<RayCase, 2U> const tiny_cases{{
    // 100 mm * 1e-30 is far below one picometer.
    {.position = {0, 0, 0},
     .direction = {1.0F, 1.0e-30F, 0.0F},
     .status = k_exit,
     .endpoint = {k_100_mm, 0, 0},
     .faces = k_face_plus_x},
    // A subnormal transverse component never limits the ray; devices that
    // flush subnormals to zero under fast-math options treat it as parallel,
    // which yields the same exit. A direction with only subnormal components
    // is outside the admitted domain for that reason.
    {.position = {0, 0, 0},
     .direction = {-1.0F, 0.0F, denorm},
     .status = k_exit,
     .endpoint = {-k_100_mm, 0, 0},
     .faces = k_face_minus_x},
  }};

  ForEachContext([&](auto &context) -> void {
    RunCases(context, largest, largest_cases);
    RunCases(context, cube, tiny_cases);
  });
}
