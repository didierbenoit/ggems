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
 * \brief Checks complete voxel histories through the production OpenCL worker.
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
#include <filesystem>
#include <format>
#include <limits>
#include <numbers>
#include <string>
#include <span>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMSAnalyticStreamFixture.hh"
#include "GGEMSNavigationTestDevices.hh"
#include "GGEMSOpenCLDeviceInventory.hh"
#include "GGEMSVoxelizedTestFixture.hh"
#include "GGEMS/geometry/GGEMSBox.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSVoxelizedVolume.hh"
#include "GGEMS/geometry/GGEMSWorld.hh"
#include "GGEMS/materials/GGEMSEMMaterialPackage.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/particles/GGEMSParticleState.hh"
#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/random/GGEMSRandom.hh"
#include "GGEMS/sources/GGEMSSourceRecord.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace {
using namespace ggems;
using namespace ggems::core;
using namespace ggems::units;
using Vector = std::array<double, 3>;
using ggems::test::GGEMSVoxelizedTestFixture;
constexpr std::uint32_t k_exterior{0xFFFF'FFFFU};
constexpr double k_mm{1.0e9};

/*! \brief A source ray, with position in millimeters and stored direction. */
struct Ray {
  Vector start;
  Vector direction;
};

/*! \brief Independent physical crossing in millimeters along the test ray. */
struct Crossing {
  double parameter_mm;
  std::uint32_t owner;
  std::uint32_t material;
  std::uint32_t event;
  std::uint32_t faces;
};

/*! \brief Creates a finite World with no material identity ambiguity. */
auto World() -> geometry::GGEMSWorld {
  geometry::GGEMSWorld world;
  world.SetSize(100_mm, 100_mm, 100_mm)
    .SetMaterial(materials::builtins::BuildBuiltInMaterial("Vacuum"));
  return world;
}

/*! \brief Runs the real production stream, capturing accepted transactions. */
auto Trace(geometry::GGEMSWorld const &world, Ray const &ray,
           bool instrument = true, std::uint32_t boundary_limit = 0U)
  -> test::StreamResult {
  auto const &start = ray.start;
  auto const &direction = ray.direction;
  std::vector<materials::GGEMSMaterial> media{world.GetMaterial()};
  for (std::size_t index = 0; index < world.GetVolumeCount(); ++index) {
    if (world.GetVoxelizedVolume(index) == nullptr) {
      media.push_back(*world.GetVolumeMaterial(index));
    }
  }
  auto const palette_begin = media.size();
  for (std::size_t index = 0; index < world.GetVolumeCount(); ++index) {
    if (auto const *grid = world.GetVoxelizedVolume(index)) {
      auto const palette = grid->GetMaterials();
      media.insert(media.end(), palette.begin(), palette.end());
    }
  }
  materials::GGEMSEMMaterialPackage const package{media};
  auto const identities = package.GetMaterialIds();
  auto const analytic = world.BuildAnalyticDeviceRecords(
    identities.subspan(1U, palette_begin - 1U));
  auto const voxel =
    world.BuildVoxelizedDeviceData(identities.subspan(palette_begin));
  auto const auxiliary = world.BuildAnalyticAuxiliaryBuffer();
  auto record = world.BuildRecord();
  if (boundary_limit != 0U) {
    record.max_boundary_events = boundary_limit;
  }
  sources::GGEMSSourceRecord source{};
  source.emitted_particle_type =
    particles::ToKernelParticleType(particles::GGEMSParticleType::Aionino);
  source.position_x_pm = std::llround(start[0] * k_mm);
  source.position_y_pm = std::llround(start[1] * k_mm);
  source.position_z_pm = std::llround(start[2] * k_mm);
  source.axis_z_x = static_cast<float>(direction[0]);
  source.axis_z_y = static_cast<float>(direction[1]);
  source.axis_z_z = static_cast<float>(direction[2]);
  source.energy_micro_eV = 123456789U;
  source.time_start_ps = 17U;
  source.time_stop_ps = 17U;
  auto &context = test::GetSelectedOpenCLContext();
  random::GGEMSRandom random;
  random.SetEngine("philox").SetSeed(91231U);
  std::filesystem::path const root{GGEMS_TEST_KERNEL_ROOT};
  auto const &production = ocl::GGEMSOpenCL::GetInstance().GetOrCreateProgram(
    context, root / "transport", "particle_stream_transport",
    std::format("-I{} {} -DGGEMS_ENABLE_TRANSPORT_OBSERVER=1",
                root.generic_string(), random.GetKernelBuildDefinition()));
  auto const &program =
    instrument ? test::GetAnalyticTraceProgram(test::InstrumentStream(root),
                                               production.GetBuildOptions())
               : production.GetProgramNative();
  return test::RunStream(context, program, random, record, std::span{analytic},
                         source, nullptr, instrument, auxiliary, voxel);
}

/*! \brief Derives the finite World's independent terminal plane intersection.
 */
auto WorldExit(Ray const &ray) -> Crossing {
  double parameter = std::numeric_limits<double>::infinity();
  std::uint32_t faces{};
  for (std::size_t axis = 0; axis < 3U; ++axis) {
    if (ray.direction[axis] == 0.0) {
      continue;
    }
    double const candidate =
      (std::copysign(50.0, ray.direction[axis]) - ray.start[axis]) /
      ray.direction[axis];
    auto const face = 1U << ((2U * axis) + (ray.direction[axis] > 0 ? 1U : 0U));
    if (candidate < parameter) {
      parameter = candidate;
      faces = face;
    } else if (candidate == parameter) {
      faces |= face;
    }
  }
  return {
    .parameter_mm = parameter,
    .owner = k_exterior,
    .material = k_exterior,
    .event = 1U,
    .faces = faces,
  };
}

/*! \brief Decodes the accepted FP32 point recorded by the actual transaction.
 */
auto Point(test::StreamResult const &result, std::size_t offset)
  -> std::array<float, 3> {
  return {
    std::bit_cast<float>(static_cast<std::uint32_t>(result.trace[offset + 3U])),
    std::bit_cast<float>(static_cast<std::uint32_t>(result.trace[offset + 4U])),
    std::bit_cast<float>(static_cast<std::uint32_t>(result.trace[offset + 5U])),
  };
}

/*! \brief Requires ordinary finite escape without random or physical changes.
 */
auto CheckCompletion(test::StreamResult const &result) -> void {
  auto const &counts = result.counts;
  std::array const actual{
    counts.completed_history_count,
    counts.escaped_world_count,
    counts.unresolved_geometry_count,
    counts.outside_world_count,
    counts.overflow_count,
  };
  EXPECT_EQ(actual, (std::array{1U, 1U, 0U, 0U, 0U}));
  EXPECT_TRUE(result.unchanged_rng);
  ASSERT_EQ(result.observer_counts.record_count, 2U);
  auto const &terminal = result.records[1];
  EXPECT_EQ(terminal.status, particles::ToKernelParticleStatus(
                               particles::GGEMSParticleStatus::EscapedWorld));
  std::array<std::uint64_t, 3> const physical{
    terminal.energy_micro_eV,
    terminal.time_ps,
    terminal.deposited_energy_micro_eV,
  };
  EXPECT_EQ(physical, (std::array<std::uint64_t, 3>{123456789U, 17U, 0U}));
  EXPECT_EQ(sizeof(particles::GGEMSParticleState), 80U);
}

/*! \brief Compares exact owner, material, event, and simultaneous face mask. */
auto CheckTopology(test::StreamResult const &result, std::size_t offset,
                   Crossing const &item, std::uint32_t departure) -> void {
  std::array const actual{
    static_cast<std::uint32_t>(result.trace[offset]),
    static_cast<std::uint32_t>(result.trace[offset + 1U]),
    static_cast<std::uint32_t>(result.trace[offset + 2U]),
    static_cast<std::uint32_t>(result.trace[offset + 6U]),
    static_cast<std::uint32_t>(result.trace[offset + 7U]),
  };
  std::array const expected{
    departure, item.owner, item.faces, item.event, item.material,
  };
  EXPECT_EQ(actual, expected);
}

/*! \brief Independent coordinate and physical-motion reference for one axis. */
struct CoordinateCheck {
  float actual;
  float previous;
  double ideal;
  float parameter;
  float direction;
  double budget;
};

/*! \brief Compares a coordinate and excludes internal or World-exit pushes. */
auto CheckCoordinate(CoordinateCheck const &reference, Crossing const &item)
  -> void {
  EXPECT_LE(std::abs(reference.actual - reference.ideal), reference.budget);
  if (item.event == 4U) {
    EXPECT_EQ(reference.actual, static_cast<float>(reference.ideal));
  }
  if (item.event == 1U) {
    EXPECT_FLOAT_EQ(
      reference.actual,
      std::fma(reference.parameter, reference.direction, reference.previous));
  }
}

/*! \brief Independent physical flight and its frozen spatial error budget. */
struct ParameterCheck {
  double expected;
  double norm;
  double budget;
};

/*! \brief Checks the physical parameter independently of accepted position. */
auto CheckParameter(test::StreamResult const &result,
                    ParameterCheck const &reference, std::size_t offset)
  -> float {
  auto const parameter =
    std::bit_cast<float>(static_cast<std::uint32_t>(result.trace[offset + 8U]));
  EXPECT_TRUE(std::isfinite(parameter));
  EXPECT_GT(parameter, 0.0F);
  EXPECT_LE(std::abs((parameter - reference.expected) * reference.norm),
            reference.budget);
  return parameter;
}
/*! \brief Checks all accepted physical intervals without reproducing the
 * kernel. */
auto CheckTrajectory(test::StreamResult const &result,
                     std::uint32_t initial_owner, Ray const &ray,
                     std::span<Crossing const> expected, double budget)
  -> void {
  double const norm =
    std::hypot(ray.direction[0], ray.direction[1], ray.direction[2]);
  double previous{};
  auto departure = initial_owner;
  std::array<float, 3> committed{
    static_cast<float>(ray.start[0] * k_mm),
    static_cast<float>(ray.start[1] * k_mm),
    static_cast<float>(ray.start[2] * k_mm),
  };
  for (std::size_t index = 0; index < expected.size(); ++index) {
    SCOPED_TRACE(index);
    auto const offset = 1U + (test::k_trace_stride * index);
    auto const &item = expected[index];
    CheckTopology(result, offset, item, departure);
    float const parameter =
      CheckParameter(result,
                     {
                       .expected = (item.parameter_mm - previous) * k_mm,
                       .norm = norm,
                       .budget = budget,
                     },
                     offset);
    auto const point = Point(result, offset);
    for (std::size_t axis = 0; axis < 3U; ++axis) {
      double const ideal =
        (ray.start[axis] + (item.parameter_mm * ray.direction[axis])) * k_mm;
      CheckCoordinate(
        {
          .actual = point[axis],
          .previous = committed[axis],
          .ideal = ideal,
          .parameter = parameter,
          .direction = static_cast<float>(ray.direction[axis]),
          .budget = budget,
        },
        item);
    }
    committed = point;
    previous = item.parameter_mm;
    departure = item.owner;
  }
}

/*! \brief Checks a hand-derived trajectory through one valid physical scene. */
auto Check(geometry::GGEMSWorld const &world, Vector const &start,
           Vector const &direction, std::span<Crossing const> crossings,
           std::uint32_t initial_owner = 0U,
           std::uint32_t initial_material = 0U) -> void {
  ASSERT_NO_THROW(world.Validate());
  Ray const ray{.start = start, .direction = direction};
  auto const result = Trace(world, ray);
  CheckCompletion(result);
  auto const initial = test::k_initial_owner_index;
  std::array const actual{
    result.trace[initial],
    result.trace[initial + 1U],
    result.trace[initial + 2U],
  };
  std::array<std::int64_t, 3> const owner{initial_owner, initial_material, 0U};
  EXPECT_EQ(actual, owner);
  std::vector<Crossing> expected{crossings.begin(), crossings.end()};
  expected.push_back(WorldExit(ray));
  ASSERT_EQ(result.trace[0], expected.size());
  double band = world.GetNavigationTolerancePM();
  for (std::size_t index = 0; index < world.GetVolumeCount(); ++index) {
    band = std::max(
      band, static_cast<double>(world.GetVolumeNavigationTolerancePM(index)));
  }
  CheckTrajectory(result, initial_owner, ray, expected, 8 * band);
}
using GGEMSVoxelizedStream = GGEMSNavigationDeviceTest;
} // namespace

TEST_F(GGEMSVoxelizedStream, InternalFacesAdvanceBothSignsOnEveryAxis) {
  std::array<std::uint16_t, 8> const labels{10, 20, 20, 20, 20, 20, 20, 20};
  GGEMSVoxelizedTestFixture const files{
    {2U, 2U, 2U},
    "2 2 2",
    labels,
    "10 10 Water\n20 20 Air\n",
  };
  auto world = World();
  world.AddVolume(files.Import());
  for (std::size_t axis = 0; axis < 3U; ++axis) {
    for (double const sign : {-1.0, 1.0}) {
      SCOPED_TRACE(std::format("axis {} sign {}", axis, sign));
      Vector start{-1, -1, -1};
      start[axis] = -sign;
      Vector direction{};
      direction[axis] = sign;
      auto const faces = 1U << ((2U * axis) + (sign > 0 ? 1U : 0U));
      std::array const crossings{
        Crossing{
          .parameter_mm = 1,
          .owner = 1U,
          .material = sign > 0 ? 2U : 1U,
          .event = 4U,
          .faces = faces,
        },
        Crossing{
          .parameter_mm = 3,
          .owner = 0U,
          .material = 0U,
          .event = 3U,
          .faces = faces,
        },
      };
      Check(world, start, direction, crossings, 1U, sign > 0 ? 1U : 2U);
    }
  }
}

TEST_F(GGEMSVoxelizedStream, InternalEdgeAndCornerAreSingleMultiAxisEvents) {
  std::array<std::uint16_t, 8> const labels{10, 20, 20, 20, 20, 20, 20, 20};
  GGEMSVoxelizedTestFixture const files{
    {2U, 2U, 2U},
    "2 2 2",
    labels,
    "10 10 Water\n20 20 Air\n",
  };
  auto world = World();
  world.AddVolume(files.Import());
  for (std::size_t axes : {2U, 3U}) {
    for (double const sign : {-1.0, 1.0}) {
      SCOPED_TRACE(std::format("axes {} sign {}", axes, sign));
      Vector start{-1, -1, -1};
      Vector direction{};
      std::uint32_t faces{};
      for (std::size_t axis = 0; axis < axes; ++axis) {
        start[axis] = -sign;
        direction[axis] = sign;
        faces |= 1U << ((2U * axis) + (sign > 0 ? 1U : 0U));
      }
      std::array const crossings{
        Crossing{
          .parameter_mm = 1,
          .owner = 1U,
          .material = sign > 0 ? 2U : 1U,
          .event = 4U,
          .faces = faces,
        },
        Crossing{
          .parameter_mm = 3,
          .owner = 0U,
          .material = 0U,
          .event = 3U,
          .faces = faces,
        },
      };
      Check(world, start, direction, crossings, 1U, sign > 0 ? 1U : 2U);
    }
  }
}

TEST_F(GGEMSVoxelizedStream, SameMaterialStillVisitsEveryCellPlane) {
  std::array<std::uint16_t, 4> const labels{10, 10, 10, 10};
  GGEMSVoxelizedTestFixture const files{
    {4U, 1U, 1U},
    "1 2 2",
    labels,
    "10 10 Water\n",
  };
  auto world = World();
  world.AddVolume(files.Import());
  std::array const crossings{
    Crossing{
      .parameter_mm = 1,
      .owner = 1U,
      .material = 1U,
      .event = 2U,
      .faces = 1U,
    },
    Crossing{
      .parameter_mm = 2,
      .owner = 1U,
      .material = 1U,
      .event = 4U,
      .faces = 2U,
    },
    Crossing{
      .parameter_mm = 3,
      .owner = 1U,
      .material = 1U,
      .event = 4U,
      .faces = 2U,
    },
    Crossing{
      .parameter_mm = 4,
      .owner = 1U,
      .material = 1U,
      .event = 4U,
      .faces = 2U,
    },
    Crossing{
      .parameter_mm = 5,
      .owner = 0U,
      .material = 0U,
      .event = 3U,
      .faces = 2U,
    },
  };
  Check(world, {-3, -0.5, -0.5}, {1, 0, 0}, crossings);
  auto const plain =
    Trace(world, {.start = {-3, -0.5, -0.5}, .direction = {1, 0, 0}}, false);
  EXPECT_EQ(plain.counts.escaped_world_count, 1U);
  EXPECT_EQ(plain.counts.unresolved_geometry_count, 0U);
}

TEST_F(GGEMSVoxelizedStream, QuarterTurnAndAnisotropicSpacingUseLocalPlanes) {
  GGEMSVoxelizedTestFixture const files;
  auto unposed = World();
  unposed.AddVolume(files.Import());
  for (std::size_t axis = 0; axis < 3U; ++axis) {
    SCOPED_TRACE(axis);
    auto const half = static_cast<double>(axis + 1U);
    Vector direction{};
    direction[axis] = 1;
    auto const faces = 1U << ((2U * axis) + 1U);
    std::array const crossings{
      Crossing{
        .parameter_mm = 0.5 * half,
        .owner = 1U,
        .material = axis == 0U ? 2U : 1U,
        .event = 4U,
        .faces = faces,
      },
      Crossing{
        .parameter_mm = 1.5 * half,
        .owner = 0U,
        .material = 0U,
        .event = 3U,
        .faces = faces,
      },
    };
    Check(unposed, {-0.5, -1, -1.5}, direction, crossings, 1U, 1U);
  }
  auto grid = files.Import();
  grid.SetPosition(geometry::MakePositionPM(5'000'000'000LL, 3'000'000'000LL,
                                            -2'000'000'000LL));
  grid.SetRotation(0_deg, 0_deg, 90_deg);
  auto world = World();
  world.AddVolume(grid);
  std::array const crossings{
    Crossing{
      .parameter_mm = 1,
      .owner = 1U,
      .material = 1U,
      .event = 2U,
      .faces = 1U,
    },
    Crossing{
      .parameter_mm = 2,
      .owner = 1U,
      .material = 2U,
      .event = 4U,
      .faces = 2U,
    },
    Crossing{
      .parameter_mm = 3,
      .owner = 0U,
      .material = 0U,
      .event = 3U,
      .faces = 2U,
    },
  };
  Check(world, {6, 1, -3.5}, {0, 1, 0}, crossings);
}

TEST_F(GGEMSVoxelizedStream, MultipleGridsCompeteByDistanceNotRegistration) {
  std::array<std::uint16_t, 2> const labels{10, 20};
  GGEMSVoxelizedTestFixture const files{
    {2U, 1U, 1U},
    "1 2 2",
    labels,
    "10 10 Water\n20 20 Air\n",
  };
  auto grid = files.Import();
  auto world = World();
  grid.SetPosition(geometry::MakePositionPM(5'000'000'000LL, 0, 0));
  world.AddVolume(grid);
  grid.SetPosition(geometry::MakePositionPM(-5'000'000'000LL, 0, 0));
  world.AddVolume(grid);
  std::array const crossings{
    Crossing{
      .parameter_mm = 4,
      .owner = 2U,
      .material = 1U,
      .event = 2U,
      .faces = 1U,
    },
    Crossing{
      .parameter_mm = 5,
      .owner = 2U,
      .material = 2U,
      .event = 4U,
      .faces = 2U,
    },
    Crossing{
      .parameter_mm = 6,
      .owner = 0U,
      .material = 0U,
      .event = 3U,
      .faces = 2U,
    },
    Crossing{
      .parameter_mm = 14,
      .owner = 1U,
      .material = 1U,
      .event = 2U,
      .faces = 1U,
    },
    Crossing{
      .parameter_mm = 15,
      .owner = 1U,
      .material = 2U,
      .event = 4U,
      .faces = 2U,
    },
    Crossing{
      .parameter_mm = 16,
      .owner = 0U,
      .material = 0U,
      .event = 3U,
      .faces = 2U,
    },
  };
  Check(world, {-10, -0.5, -0.5}, {1, 0, 0}, crossings);
}

TEST_F(GGEMSVoxelizedStream, TouchingAnalyticAndVoxelFamiliesContinueBothWays) {
  std::array<std::uint16_t, 2> const labels{10, 20};
  GGEMSVoxelizedTestFixture const files{
    {2U, 1U, 1U},
    "1 2 2",
    labels,
    "10 10 Water\n20 20 Air\n",
  };
  auto world = World();
  world.AddVolume(files.Import());
  geometry::GGEMSBox box;
  box.SetSize(2_mm, 2_mm, 2_mm)
    .SetMaterial(materials::builtins::BuildBuiltInMaterial("Water"))
    .SetPosition(geometry::MakePositionPM(2'000'000'000LL, 0, 0));
  world.AddVolume(box);
  box.SetPosition(geometry::MakePositionPM(-2'000'000'000LL, 0, 0));
  world.AddVolume(box);
  std::array const forward{
    Crossing{
      .parameter_mm = 2,
      .owner = 3U,
      .material = 1U,
      .event = 2U,
      .faces = 1U,
    },
    Crossing{
      .parameter_mm = 4,
      .owner = 1U,
      .material = 1U,
      .event = 3U,
      .faces = 2U,
    },
    Crossing{
      .parameter_mm = 5,
      .owner = 1U,
      .material = 2U,
      .event = 4U,
      .faces = 2U,
    },
    Crossing{
      .parameter_mm = 6,
      .owner = 2U,
      .material = 1U,
      .event = 3U,
      .faces = 2U,
    },
    Crossing{
      .parameter_mm = 8,
      .owner = 0U,
      .material = 0U,
      .event = 3U,
      .faces = 2U,
    },
  };
  Check(world, {-5, -0.5, -0.5}, {1, 0, 0}, forward);
  std::array const backward{
    Crossing{
      .parameter_mm = 2,
      .owner = 2U,
      .material = 1U,
      .event = 2U,
      .faces = 2U,
    },
    Crossing{
      .parameter_mm = 4,
      .owner = 1U,
      .material = 2U,
      .event = 3U,
      .faces = 1U,
    },
    Crossing{
      .parameter_mm = 5,
      .owner = 1U,
      .material = 1U,
      .event = 4U,
      .faces = 1U,
    },
    Crossing{
      .parameter_mm = 6,
      .owner = 3U,
      .material = 1U,
      .event = 3U,
      .faces = 1U,
    },
    Crossing{
      .parameter_mm = 8,
      .owner = 0U,
      .material = 0U,
      .event = 3U,
      .faces = 1U,
    },
  };
  Check(world, {5, -0.5, -0.5}, {-1, 0, 0}, backward);
}

TEST_F(GGEMSVoxelizedStream, ExhaustedBoundaryBudgetRemainsUnresolved) {
  std::array<std::uint16_t, 4> const labels{10, 10, 10, 10};
  GGEMSVoxelizedTestFixture const files{
    {4U, 1U, 1U},
    "1 2 2",
    labels,
    "10 10 Water\n",
  };
  auto world = World();
  world.AddVolume(files.Import());
  auto const result =
    Trace(world, {.start = {-3, -0.5, -0.5}, .direction = {1, 0, 0}}, true, 2U);
  EXPECT_EQ(result.trace[0], 2U);
  EXPECT_EQ(result.counts.unresolved_geometry_count, 1U);
  EXPECT_EQ(result.counts.escaped_world_count, 0U);
  EXPECT_EQ(result.counts.completed_history_count, 0U);
}

TEST_F(GGEMSVoxelizedStream, ShallowRotatedEntriesMakeFiniteForwardProgress) {
  std::array<std::uint16_t, 2> const labels{10, 10};
  GGEMSVoxelizedTestFixture const files{
    {2U, 1U, 1U},
    "1 20 2",
    labels,
    "10 10 Water\n",
  };
  std::string evidence{"degrees,incidence,event,parameter_pm,x_pm,y_pm,z_pm\n"};
  for (double const degrees : {15.0, 30.0, 45.0}) {
    double const radians = degrees * std::numbers::pi / 180.0;
    double const cosine = std::cos(radians);
    double const sine = std::sin(radians);
    auto grid = files.Import();
    grid.SetRotation(0_deg, 0_deg, MakeDegrees(degrees));
    auto world = World();
    world.AddVolume(grid);
    ASSERT_NO_THROW(world.Validate());
    for (double const incidence : {0.001, 0.01, 0.05}) {
      SCOPED_TRACE(
        std::format("{} degrees, normal component {}", degrees, incidence));
      double const local_x = -1.0 - (20.0 * incidence);
      Ray const ray{
        .start =
          {
            (cosine * local_x) + (20.0 * sine),
            (sine * local_x) - (20.0 * cosine),
            0.25,
          },
        .direction =
          {
            (cosine * incidence) - sine,
            (sine * incidence) + cosine,
            0,
          },
      };
      auto const result = Trace(world, ray);
      CheckCompletion(result);
      ASSERT_EQ(result.trace[0], 3U);
      std::array<float, 3> previous{
        static_cast<float>(ray.start[0] * k_mm),
        static_cast<float>(ray.start[1] * k_mm),
        static_cast<float>(ray.start[2] * k_mm),
      };
      for (std::size_t index = 0; index < 3U; ++index) {
        auto const offset = 1U + (test::k_trace_stride * index);
        auto const parameter = std::bit_cast<float>(
          static_cast<std::uint32_t>(result.trace[offset + 8U]));
        EXPECT_GT(parameter, 0.0F);
        auto const point = Point(result, offset);
        for (std::size_t axis = 0; axis < 3U; ++axis) {
          EXPECT_FLOAT_EQ(point[axis],
                          std::fma(parameter,
                                   static_cast<float>(ray.direction[axis]),
                                   previous[axis]));
        }
        evidence += std::format("{},{},{},{},{},{},{}\n", degrees, incidence,
                                index, parameter, point[0], point[1], point[2]);
        previous = point;
      }
    }
  }
  RecordProperty("entry_progress_csv", evidence);
}
