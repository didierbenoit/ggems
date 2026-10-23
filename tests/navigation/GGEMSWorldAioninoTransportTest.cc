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
#include <memory>
#include <span>
#include <string>
#include <vector>
#include <utility>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/geometry/GGEMSWorld.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/observer/GGEMSObserverRecord.hh"
#include "GGEMS/observer/GGEMSObserverTypes.hh"
#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/opencl/GGEMSOpenCLContext.hh"
#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/random/GGEMSRandom.hh"
#include "GGEMS/sources/GGEMSSource.hh"
#include "GGEMS/sources/GGEMSSourceRecord.hh"
#include "GGEMS/sources/GGEMSSourceDeviceRecord.hh"
#include "GGEMS/sources/GGEMSSourceRunRange.hh"
#include "GGEMS/sources/GGEMSSourceRunSnapshot.hh"
#include "GGEMS/transport/GGEMSTransportWorkload.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

#include "GGEMSNavigationTestDevices.hh"
#include "GGEMSFP32TestOracle.hh"
#include "GGEMSWorldTestOracle.hh"

namespace {

using namespace ggems::units;
using ObserverRecord = ggems::core::observer::GGEMSObserverRecord;
using ObserverRecordKind = ggems::core::observer::GGEMSObserverRecordKind;
using ParticleStatus = ggems::core::particles::GGEMSParticleStatus;
using ParticleType = ggems::core::particles::GGEMSParticleType;
using SourceRecord = ggems::core::sources::GGEMSSourceRecord;
using SourceRunRange = ggems::core::sources::GGEMSSourceRunRange;
using TransportRunConfig = ggems::core::transport::GGEMSTransportRunConfig;
using TransportRunReport = ggems::core::transport::GGEMSTransportRunReport;
using TransportWorkload = ggems::core::transport::GGEMSTransportWorkload;
using World = ggems::geometry::GGEMSWorld;

constexpr std::int64_t k_100_mm{100'000'000'000LL};
constexpr std::int64_t k_one_meter_pm{1'000'000'000'000LL};

/*! \brief Vacuum World of the given full sizes centered on the origin. */
auto MakeVacuumWorld(Length size_x, Length size_y, Length size_z) -> World {
  World world{};
  world.SetSize(size_x, size_y, size_z)
    .SetMaterial(
      ggems::core::materials::builtins::BuildBuiltInMaterial("Vacuum"));
  return world;
}

/*! \brief The W01 World: 200 mm cube of Vacuum centered on the origin. */
auto MakeCubicWorld() -> World {
  return MakeVacuumWorld(200_mm, 200_mm, 200_mm);
}

/*! \brief Builds a count-driven analytic point source along a stored axis. */
auto MakeSource(std::array<std::int64_t, 3U> const &position_pm,
                std::array<double, 3U> const &direction,
                ParticleType particle_type = ParticleType::Aionino)
  -> SourceRecord {
  ggems::core::sources::GGEMSSource source{};
  source.SetAnalytic()
    .SetEmittedParticleType(particle_type)
    .SetEnergyMicroElectronVolt(1'000'000'000ULL)
    .SetPositionPicoMeter(position_pm[0U], position_pm[1U], position_pm[2U])
    .SetDirection(direction[0U], direction[1U], direction[2U]);
  return source.BuildRecord();
}

/*! \brief Builds a run config with one primary per source slot. */
auto MakeConfig(std::vector<SourceRecord> records) -> TransportRunConfig {
  TransportRunConfig config{};
  config.total_primary_count = records.size();
  config.source_population_records.resize(records.size());

  std::uint64_t begin{0ULL};
  for (std::size_t index = 0U; index < records.size(); ++index) {
    config.source_ranges.push_back(
      {.projection_primary_begin = begin, .primary_count = 1ULL});
    ++begin;
  }

  config.source_records = std::move(records);
  config.observer_config.enabled = 1U;
  config.observer_config.capture_first_primary_count_per_source = 1U;
  return config;
}

/*! \brief Builds a workload with one primary per source on one context. */
auto MakeWorkload(ggems::ocl::GGEMSOpenCLContext &context,
                  ggems::core::random::GGEMSRandom const &random,
                  std::size_t source_count, World const *world)
  -> std::unique_ptr<TransportWorkload> {
  std::vector<std::shared_ptr<ggems::core::sources::GGEMSSource>> sources;
  sources.reserve(source_count);
  for (std::size_t index = 0U; index < source_count; ++index) {
    sources.push_back(std::make_shared<ggems::core::sources::GGEMSSource>());
  }
  auto const snapshot =
    ggems::core::sources::BuildSourceConfigurationSnapshot(sources);

  return std::make_unique<TransportWorkload>(
    context, std::filesystem::path{GGEMS_TEST_KERNEL_ROOT}, random, 64U,
    *snapshot, 0ULL, 0U, static_cast<std::uint32_t>(2U * source_count), 0U,
    world);
}

/*! \brief Returns the terminal record of one history. */
auto FindTerminalRecord(TransportRunReport const &report,
                        std::uint64_t global_primary_id) -> ObserverRecord {
  for (ObserverRecord const &record : report.observer_records) {
    if (record.global_primary_id == global_primary_id &&
        record.record_kind == ggems::core::observer::ToKernelObserverRecordKind(
                                ObserverRecordKind::Terminal)) {
      return record;
    }
  }
  ADD_FAILURE() << "No terminal record for history " << global_primary_id;
  return {};
}

/*! \brief One authored Aionino ray; references use its actual upload. */
struct AioninoCase {
  std::array<std::int64_t, 3U> position_pm;
  std::array<double, 3U> direction;
};

// Geometry and projection use actual uploaded binary32 values; only the
// terminal observer adds the canonical integer conversion allowance (0.5 pm).
auto ExpectEndpoint(ObserverRecord const &terminal, SourceRecord const &source,
                    World const *world) -> void {
  namespace fp = ggems::test::fp32;
  auto const packed = ggems::core::sources::BuildSourceDeviceRecord(source);
  std::array<float, 3> const position{
    packed.position_x_pm,
    packed.position_y_pm,
    packed.position_z_pm,
  };
  std::array<float, 3> const direction{
    packed.axis_z_x,
    packed.axis_z_y,
    packed.axis_z_z,
  };
  std::array<fp::Scalar, 3> expected{};
  if (world != nullptr) {
    auto const reference =
      ggems::test::WorldReference(world->BuildRecord(), position, direction);
    ASSERT_EQ(reference.status, 0U);
    expected = reference.point;
  } else {
    // One FP32 distance multiply and one add (or their contraction).
    auto const distance = fp::FromFloat(static_cast<float>(k_one_meter_pm));
    for (std::size_t axis = 0; axis < 3; ++axis) {
      expected[axis] =
        fp::Add(fp::FromFloat(position[axis]),
                fp::Multiply(distance, fp::FromFloat(direction[axis])));
    }
  }
  std::array<std::int64_t, 3> const actual{
    terminal.position_x_pm,
    terminal.position_y_pm,
    terminal.position_z_pm,
  };
  EXPECT_EQ(terminal.position_valid, 1U);
  for (std::size_t axis = 0; axis < 3; ++axis) {
    fp::ExpectWithin(static_cast<double>(actual[axis]), expected[axis], 0.5);
  }
}

auto ExpectAioninoExits(World const &world, std::span<AioninoCase const> cases)
  -> void {
  for (auto &context : ggems::ocl::GGEMSOpenCL::GetInstance().GetContext()) {
    SCOPED_TRACE(context.GetDevice().GetName());

    std::vector<SourceRecord> records;
    for (AioninoCase const &aionino : cases) {
      records.push_back(MakeSource(aionino.position_pm, aionino.direction));
    }

    ggems::core::random::GGEMSRandom random{};
    random.SetEngine("philox").SetSeed(41ULL);
    auto workload = MakeWorkload(context, random, cases.size(), &world);
    auto const config = MakeConfig(records);

    ASSERT_NO_THROW(workload->ValidateRunConfig(config));
    auto const report = workload->Run(config);

    EXPECT_EQ(report.counters.consumed_primary_count, cases.size());
    EXPECT_EQ(report.counters.completed_history_count, cases.size());
    EXPECT_EQ(report.counters.terminal_particle_count, cases.size());
    EXPECT_EQ(report.counters.escaped_world_count, cases.size());
    EXPECT_EQ(report.counters.outside_world_count, 0U);
    EXPECT_EQ(report.counters.unresolved_geometry_count, 0U);
    EXPECT_EQ(report.counters.overflow_count, 0U);
    EXPECT_EQ(report.counters.created_secondary_count, 0U);
    EXPECT_EQ(report.counters.total_fake_step_count, 0U);
    EXPECT_EQ(report.observer_counters.record_count, 2U * cases.size());

    for (std::size_t index = 0U; index < cases.size(); ++index) {
      SCOPED_TRACE(index);
      ObserverRecord const terminal = FindTerminalRecord(report, index);

      EXPECT_EQ(terminal.status, ggems::core::particles::ToKernelParticleStatus(
                                   ParticleStatus::EscapedWorld));
      ExpectEndpoint(terminal, records[index], &world);
      EXPECT_EQ(terminal.source_index, index);
      EXPECT_EQ(terminal.source_local_primary_id, 0U);
      EXPECT_EQ(
        terminal.particle_type,
        ggems::core::particles::ToKernelParticleType(ParticleType::Aionino));
      EXPECT_EQ(terminal.energy_micro_eV, records[index].energy_micro_eV);
      EXPECT_EQ(terminal.time_ps, records[index].time_start_ps);
      EXPECT_EQ(terminal.deposited_energy_micro_eV, 0ULL);
      EXPECT_FLOAT_EQ(terminal.direction_x, records[index].axis_z_x);
      EXPECT_FLOAT_EQ(terminal.direction_y, records[index].axis_z_y);
      EXPECT_FLOAT_EQ(terminal.direction_z, records[index].axis_z_z);
    }
  }
}

} // namespace

// =============================================================================
// =============================================================================

TEST_F(GGEMSNavigationDeviceTest,
       W01CenterToPlusXExitsAtOneHundredMillimeters) {
  World const world = MakeCubicWorld();

  // geometric travel = 100 mm, final position = (100 mm, 0, 0), +X face.
  std::array<AioninoCase, 1U> const cases{
    {
      {.position_pm = {0, 0, 0}, .direction = {1.0, 0.0, 0.0}},
    },
  };

  ExpectAioninoExits(world, cases);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSNavigationDeviceTest, ExitsCubicWorldOnEveryFaceAndDiagonal) {
  World const world = MakeCubicWorld();

  std::array<AioninoCase, 9U> const cases{
    {
      {.position_pm = {0, 0, 0}, .direction = {-1.0, 0.0, 0.0}},
      {.position_pm = {0, 0, 0}, .direction = {0.0, 1.0, 0.0}},
      {.position_pm = {0, 0, 0}, .direction = {0.0, -1.0, 0.0}},
      {.position_pm = {0, 0, 0}, .direction = {0.0, 0.0, 1.0}},
      {.position_pm = {0, 0, 0}, .direction = {0.0, 0.0, -1.0}},
      // Off-center axis-aligned: 130 mm of travel to the +Z face.
      {
        .position_pm = {12'000'000'000LL, -7'000'000'000LL, -30'000'000'000LL},
        .direction = {0.0, 0.0, 1.0},
      },
      // Body diagonal: equal stored components tie at the corner.
      {.position_pm = {0, 0, 0}, .direction = {1.0, 1.0, 1.0}},
      // Off-center diagonal: y limits; equal components determine x and z
      // geometrically.
      {
        .position_pm = {-30'000'000'000LL, 20'000'000'000LL, -50'000'000'000LL},
        .direction = {1.0, 1.0, 1.0},
      },
      // Stored normalize(-2, 1, 0.5): x limits; represented-geometry lateral
      // exit.
      {
        .position_pm = {50'000'000'000LL, -20'000'000'000LL, 10'000'000'000LL},
        .direction = {-2.0, 1.0, 0.5},
      },
    },
  };

  ExpectAioninoExits(world, cases);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSNavigationDeviceTest,
       ExitsAsymmetricWorldWithStoredFP32Directions) {
  World const world = MakeVacuumWorld(200_mm, 400_mm, 100_mm);

  std::array<AioninoCase, 3U> const cases{
    {
      // Stored normalize(1, 2, 3) from the center: z limits at 50 mm; the
      // lateral coordinates follow the actual uploaded binary32 direction.
      {.position_pm = {0, 0, 0}, .direction = {1.0, 2.0, 3.0}},
      {
        .position_pm = {-70'000'000'000LL, 30'000'000'000LL, -40'000'000'000LL},
        .direction = {1.0, 2.0, 3.0},
      },
      // Body diagonal of the asymmetric World exits on the +Z face only.
      {.position_pm = {0, 0, 0}, .direction = {1.0, 1.0, 1.0}},
    },
  };

  ExpectAioninoExits(world, cases);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSNavigationDeviceTest, HandlesBoundaryStartsAndCoincidentMovement) {
  World const world = MakeCubicWorld();

  std::array<AioninoCase, 6U> const cases{
    {
      // On the -X face pointing inward: full 200 mm crossing.
      {
        .position_pm = {-k_100_mm, 12'000'000'000LL, -7'000'000'000LL},
        .direction = {1.0, 0.0, 0.0},
      },
      // On the -X face pointing outward: valid zero-distance exit.
      {
        .position_pm = {-k_100_mm, 12'000'000'000LL, -7'000'000'000LL},
        .direction = {-1.0, 0.0, 0.0},
      },
      // On the +X face moving parallel to it along -Z.
      {.position_pm = {k_100_mm, 0, 0}, .direction = {0.0, 0.0, -1.0}},
      // On the (+X, -Y) edge moving along +Z.
      {.position_pm = {k_100_mm, -k_100_mm, 0}, .direction = {0.0, 0.0, 1.0}},
      // On the (+X, +Y, +Z) corner pointing outward and inward.
      {
        .position_pm = {k_100_mm, k_100_mm, k_100_mm},
        .direction = {1.0, 1.0, 1.0},
      },
      {
        .position_pm = {k_100_mm, k_100_mm, k_100_mm},
        .direction = {-1.0, -1.0, -1.0},
      },
    },
  };

  ExpectAioninoExits(world, cases);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSNavigationDeviceTest, ExitsThreeMeterWorld) {
  World const world = MakeVacuumWorld(3000_mm, 3000_mm, 3000_mm);
  constexpr std::int64_t k_half{1'500'000'000'000LL};

  std::array<AioninoCase, 2U> const cases{
    {
      // Three-meter crossing with a resolvable medical-scale World.
      {.position_pm = {-k_half, 0, 0}, .direction = {1.0, 0.0, 0.0}},
      // Representable symmetric body diagonal; keep the corner trajectory.
      {.position_pm = {0, 0, 0}, .direction = {1.0, 1.0, 1.0}},
    },
  };

  ExpectAioninoExits(world, cases);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSNavigationDeviceTest,
       RejectsAioninoSourcesOutsideTheWorldOrWithoutAWorld) {
  World const world = MakeCubicWorld();
  auto &context = ggems::ocl::GGEMSOpenCL::GetInstance().GetContext().front();
  ggems::core::random::GGEMSRandom random{};
  random.SetEngine("philox").SetSeed(42ULL);

  // One micrometer outside the +Y face is invalid user input.
  auto workload = MakeWorkload(context, random, 1U, &world);
  EXPECT_THROW(workload->ValidateRunConfig(MakeConfig(
                 {MakeSource({0, k_100_mm + 1'000'000, 0}, {0.0, -1.0, 0.0})})),
               ggems::core::GGEMSRecoverable);
  // On the face is valid.
  EXPECT_NO_THROW(workload->ValidateRunConfig(
    MakeConfig({MakeSource({0, k_100_mm, 0}, {0.0, -1.0, 0.0})})));

  // Aionino without an attached World is rejected; Gamma is not.
  auto no_world = MakeWorkload(context, random, 1U, nullptr);
  EXPECT_THROW(no_world->ValidateRunConfig(
                 MakeConfig({MakeSource({0, 0, 0}, {1.0, 0.0, 0.0})})),
               ggems::core::GGEMSRecoverable);
  EXPECT_NO_THROW(no_world->ValidateRunConfig(
    MakeConfig({MakeSource({0, 0, 0}, {1.0, 0.0, 0.0}, ParticleType::Gamma)})));
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSNavigationDeviceTest,
       KeepsTheDiagnosticProjectionForOtherParticlesWithAWorld) {
  World const world = MakeCubicWorld();

  for (auto &context : ggems::ocl::GGEMSOpenCL::GetInstance().GetContext()) {
    SCOPED_TRACE(context.GetDevice().GetName());

    ggems::core::random::GGEMSRandom random{};
    random.SetEngine("philox").SetSeed(43ULL);
    auto workload = MakeWorkload(context, random, 1U, &world);
    auto const config =
      MakeConfig({MakeSource({0, 0, 0}, {1.0, 0.0, 0.0}, ParticleType::Gamma)});

    ASSERT_NO_THROW(workload->ValidateRunConfig(config));
    auto const report = workload->Run(config);

    EXPECT_EQ(report.counters.escaped_world_count, 0U);
    EXPECT_EQ(report.counters.overflow_count, 0U);
    EXPECT_EQ(report.counters.terminal_particle_count, 1U);

    ObserverRecord const terminal = FindTerminalRecord(report, 0ULL);
    EXPECT_EQ(terminal.status, ggems::core::particles::ToKernelParticleStatus(
                                 ParticleStatus::Killed));
    ExpectEndpoint(terminal, config.source_records[0], nullptr);
    EXPECT_EQ(report.counters.completed_history_count, 1U);
    EXPECT_EQ(report.observer_counters.record_count, 2U);
  }
}

// =============================================================================
// =============================================================================

// Multi-source gate for the diagnostic species: nine Gamma histories from nine
// source slots must each keep their own one-meter projection. This exposed an
// Intel CPU Release code-generation defect when the Aionino step was inlined
// into the stream kernel (histories received each other's positions).
TEST_F(GGEMSNavigationDeviceTest,
       GammaMultiSourceHistoriesKeepTheirOwnProjection) {
  World const world = MakeCubicWorld();
  std::array<std::array<double, 3U>, 9U> const directions{
    {
      {-1.0, 0.0, 0.0},
      {0.0, 1.0, 0.0},
      {0.0, -1.0, 0.0},
      {0.0, 0.0, 1.0},
      {0.0, 0.0, -1.0},
      {1.0, 0.0, 0.0},
      {0.0, 1.0, 0.0},
      {-1.0, 0.0, 0.0},
      {0.0, 0.0, 1.0},
    },
  };

  for (auto &context : ggems::ocl::GGEMSOpenCL::GetInstance().GetContext()) {
    SCOPED_TRACE(context.GetDevice().GetName());

    std::vector<SourceRecord> records;
    for (std::size_t index = 0U; index < directions.size(); ++index) {
      std::int64_t const offset_pm =
        static_cast<std::int64_t>(index) * 1'000'000'000LL;
      records.push_back(
        MakeSource({offset_pm, 0, 0}, directions[index], ParticleType::Gamma));
    }

    ggems::core::random::GGEMSRandom random{};
    random.SetEngine("philox").SetSeed(41ULL);
    auto workload = MakeWorkload(context, random, directions.size(), &world);
    auto const config = MakeConfig(records);

    ASSERT_NO_THROW(workload->ValidateRunConfig(config));
    auto const report = workload->Run(config);

    EXPECT_EQ(report.counters.completed_history_count, directions.size());
    EXPECT_EQ(report.counters.terminal_particle_count, directions.size());
    EXPECT_EQ(report.observer_counters.record_count, 2U * directions.size());
    EXPECT_EQ(report.counters.escaped_world_count, 0U);
    EXPECT_EQ(report.counters.overflow_count, 0U);

    for (std::size_t index = 0U; index < directions.size(); ++index) {
      SCOPED_TRACE(index);
      ObserverRecord const terminal = FindTerminalRecord(report, index);

      EXPECT_EQ(terminal.status, ggems::core::particles::ToKernelParticleStatus(
                                   ParticleStatus::Killed));
      ExpectEndpoint(terminal, records[index], nullptr);
      EXPECT_EQ(terminal.source_index, index);
      EXPECT_EQ(terminal.source_local_primary_id, 0U);
      EXPECT_EQ(terminal.global_primary_id, index);
      EXPECT_EQ(terminal.energy_micro_eV, records[index].energy_micro_eV);
    }
  }
}
