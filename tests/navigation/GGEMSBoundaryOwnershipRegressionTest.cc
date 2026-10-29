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
 * \brief Protects physical ownership across a narrow FP32 child Box crossing.
 *
 * The fixed ray is the retained Sphere-entry state of history 71346066. This
 * geometric fixture does not reconstruct its source RNG or scheduling. A second
 * scene moves only the child beyond the ray to distinguish a crossing from a
 * clear near-edge miss. Both use the existing production-worker trace fixture.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <array>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <span>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/geometry/GGEMSBox.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSSphere.hh"
#include "GGEMS/geometry/GGEMSWorld.hh"
#include "GGEMS/materials/GGEMSEMMaterialPackage.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/opencl/GGEMSOpenCLContext.hh"
#include "GGEMS/opencl/GGEMSOpenCLDevice.hh"
#include "GGEMS/opencl/GGEMSOpenCLExternal.hh"
#include "GGEMS/opencl/GGEMSOpenCLProgram.hh"
#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/random/GGEMSRandom.hh"
#include "GGEMS/sources/GGEMSSourceDeviceRecord.hh"
#include "GGEMS/sources/GGEMSSourceRecord.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

#include "GGEMSAnalyticStreamFixture.hh"
#include "GGEMSNavigationTestDevices.hh"

namespace {
using namespace ggems;
using namespace ggems::core;
using namespace ggems::units;

/*! \brief Exterior VolumeID and MaterialID in the accepted World exit. */
constexpr std::uint32_t k_exterior{0xFFFF'FFFFU};

/*! \brief Negative Y displacement in pm of the independent miss fixture. */
constexpr std::int64_t k_miss_displacement_pm{-8'388'608LL};

/*!
 * \brief Builds the admitted three-level geometry of the captured ray.
 * \param[in] miss Moves the inner Box toward negative Y to remove its chord.
 * \return Air World, Water Box, Aluminum Sphere, and Air Box hierarchy.
 */
auto BoundaryWorld(bool miss) -> geometry::GGEMSWorld {
  geometry::GGEMSWorld world;
  world.SetSize(3_m, 3_m, 3_m)
    .SetMaterial(materials::builtins::BuildBuiltInMaterial("Air"));
  geometry::GGEMSBox mother;
  mother.SetSize(250_mm, 250_mm, 250_mm)
    .SetMaterial(materials::builtins::BuildBuiltInMaterial("Water"));
  geometry::GGEMSSphere sphere;
  sphere.SetRadius(100_mm).SetMaterial(
    materials::builtins::BuildBuiltInMaterial("Aluminum"));
  geometry::GGEMSBox child;
  child.SetSize(25_mm, 25_mm, 25_mm)
    .SetMaterial(materials::builtins::BuildBuiltInMaterial("Air"));
  if (miss) {
    child.SetPosition(geometry::MakePositionPM(0, k_miss_displacement_pm, 0));
  }
  world.AddVolume(mother).AddVolume(sphere, 1U).AddVolume(child, 2U);
  return world;
}

/*!
 * \brief Reuses the exact captured anchor and direction without random draws.
 * \return Fixed point-source record with position in pm, energy in micro-eV,
 * and time in ps; its position converts exactly to the retained FP32 bits.
 */
auto BoundarySource() -> sources::GGEMSSourceRecord {
  sources::GGEMSSourceRecord source;
  source.emitted_particle_type =
    particles::ToKernelParticleType(particles::GGEMSParticleType::Aionino);
  source.position_x_pm = -99'450'732'544LL;
  source.position_y_pm = 10'269'609'984LL;
  source.position_z_pm = 2'021'630'080LL;
  source.axis_z_x = std::bit_cast<float>(0x3F7FE9A3U);
  source.axis_z_y = std::bit_cast<float>(0x3CD1F626U);
  source.axis_z_z = std::bit_cast<float>(0x3BA55412U);
  source.energy_micro_eV = 125'000'000'000ULL;
  return source;
}

/*!
 * \brief Requires one real World escape with unchanged source invariants.
 * \param[in] result Evidence from one production worker dispatch.
 * \param[in] source Fixed source used for the dispatch.
 */
auto CheckCompletion(test::StreamResult const &result,
                     sources::GGEMSSourceRecord const &source) -> void {
  EXPECT_TRUE(result.unchanged_rng);
  EXPECT_EQ(result.counts.completed_history_count, 1U);
  EXPECT_EQ(result.counts.escaped_world_count, 1U);
  EXPECT_EQ(result.counts.unresolved_geometry_count, 0U);
  EXPECT_EQ(result.counts.outside_world_count, 0U);
  EXPECT_EQ(result.counts.overflow_count, 0U);
  EXPECT_EQ(result.counts.total_fake_step_count, 0U);
  EXPECT_EQ(result.counts.created_secondary_count, 0U);
  ASSERT_EQ(result.observer_counts.record_count, 2U);
  EXPECT_EQ(result.observer_counts.overflow_count, 0U);
  auto const &terminal = result.records[1];
  EXPECT_EQ(terminal.status, particles::ToKernelParticleStatus(
                               particles::GGEMSParticleStatus::EscapedWorld));
  EXPECT_EQ(terminal.energy_micro_eV, source.energy_micro_eV);
  EXPECT_EQ(terminal.time_ps, source.time_start_ps);
  EXPECT_EQ(terminal.deposited_energy_micro_eV, 0U);
  EXPECT_EQ(std::bit_cast<std::uint32_t>(terminal.direction_x),
            std::bit_cast<std::uint32_t>(source.axis_z_x));
  EXPECT_EQ(std::bit_cast<std::uint32_t>(terminal.direction_y),
            std::bit_cast<std::uint32_t>(source.axis_z_y));
  EXPECT_EQ(std::bit_cast<std::uint32_t>(terminal.direction_z),
            std::bit_cast<std::uint32_t>(source.axis_z_z));
}
} // namespace

TEST_F(GGEMSNavigationDeviceTest,
       CapturedNearEdgeChildEntryPreservesPhysicalIntervals) {
  auto const source = BoundarySource();
  auto const uploaded = sources::BuildSourceDeviceRecord(source);
  ASSERT_EQ(std::bit_cast<std::uint32_t>(uploaded.position_x_pm), 0xD1B93DCEU);
  ASSERT_EQ(std::bit_cast<std::uint32_t>(uploaded.position_y_pm), 0x50190774U);
  ASSERT_EQ(std::bit_cast<std::uint32_t>(uploaded.position_z_pm), 0x4EF0FF41U);
  std::filesystem::path const root{GGEMS_TEST_KERNEL_ROOT};
  auto const instrumented = test::InstrumentStream(root);
  for (auto &context : ocl::GGEMSOpenCL::GetInstance().GetContext()) {
    SCOPED_TRACE(context.GetDevice().GetName());
    random::GGEMSRandom random;
    random.SetEngine("Philox").SetSeed(1U);
    auto const &production = ocl::GGEMSOpenCL::GetInstance().GetOrCreateProgram(
      context, root / "transport", "particle_stream_transport",
      std::format("-I{} {} -DGGEMS_ENABLE_TRANSPORT_OBSERVER=1",
                  root.generic_string(), random.GetKernelBuildDefinition()));
    cl::Program program{context.GetContextNative(), instrumented};
    auto const device = context.GetDevice().GetDeviceNative();
    ASSERT_EQ(program.build({device},
                            std::string{production.GetBuildOptions()}.c_str()),
              CL_SUCCESS)
      << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);

    for (bool miss : {false, true}) {
      SCOPED_TRACE(miss);
      auto const world = BoundaryWorld(miss);
      ASSERT_NO_THROW(world.Validate());
      std::vector<materials::GGEMSMaterial> media{world.GetMaterial()};
      for (std::size_t index = 0; index < world.GetVolumeCount(); ++index) {
        media.push_back(*world.GetVolumeMaterial(index));
      }
      materials::GGEMSEMMaterialPackage const package{media};
      auto const ids = package.GetMaterialIds();
      auto const volumes = world.BuildAnalyticDeviceRecords(ids.subspan(1U));
      auto const auxiliary = world.BuildAnalyticAuxiliaryBuffer();
      if (miss) {
        // Independent nominal slab ordering: +Y is left before -X is reached.
        auto const &box = volumes[2].parameters.box;
        double const x_entry = (-static_cast<double>(box.half_extent_x_pm) -
                                uploaded.position_x_pm) /
                               uploaded.axis_z_x;
        double const y_exit = (static_cast<double>(box.half_extent_y_pm) +
                               static_cast<double>(k_miss_displacement_pm) -
                               uploaded.position_y_pm) /
                              uploaded.axis_z_y;
        ASSERT_LT(y_exit, x_entry);
      }
      auto const result =
        test::RunStream(context, program, random, world.BuildRecord(),
                        std::span{volumes}, source, nullptr, true, auxiliary);
      CheckCompletion(result, source);
      EXPECT_EQ(result.trace[test::k_initial_owner_index], 2U);
      EXPECT_EQ(result.trace[test::k_initial_owner_index + 1U], ids[2]);
      EXPECT_EQ(result.trace[test::k_initial_owner_index + 2U], 0U);
      std::array const crossing_owners{3U, 2U, 1U, 0U, k_exterior};
      auto const expected = std::span{crossing_owners}.subspan(miss ? 2U : 0U);
      ASSERT_EQ(result.trace[0], expected.size());
      std::uint32_t departure{2U};
      for (std::size_t index = 0; index < expected.size(); ++index) {
        SCOPED_TRACE(index);
        auto const offset = 1U + (test::k_trace_stride * index);
        auto const arrival = expected[index];
        EXPECT_EQ(result.trace[offset], departure);
        EXPECT_EQ(result.trace[offset + 1U], arrival);
        std::uint32_t event{3U};
        if (arrival == k_exterior) {
          event = 1U;
        } else if (arrival == 3U) {
          event = 2U;
        }
        EXPECT_EQ(result.trace[offset + 6U], event);
        EXPECT_EQ(result.trace[offset + 7U],
                  arrival == k_exterior ? k_exterior : ids[arrival]);
        auto const parameter = std::bit_cast<float>(
          static_cast<std::uint32_t>(result.trace[offset + 8U]));
        ASSERT_TRUE(std::isfinite(parameter));
        EXPECT_GT(parameter, 0.0F);
        departure = arrival;
      }
      CheckCompletion(test::RunStream(context, production.GetProgramNative(),
                                      random, world.BuildRecord(),
                                      std::span{volumes}, source, nullptr,
                                      false, auxiliary),
                      source);
    }
  }
}
