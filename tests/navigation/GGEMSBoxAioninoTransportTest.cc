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
 * \brief Device tests of Aionino histories crossing one analytic Box through
 * the unmodified production stream Transport kernel and through GGEMSRun.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <span>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/GGEMSRun.hh"
#include "GGEMS/geometry/GGEMSBox.hh"
#include "GGEMS/geometry/GGEMSBoxRecord.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSWorld.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/observer/GGEMSObserverRecord.hh"
#include "GGEMS/observer/GGEMSObserverTypes.hh"
#include "GGEMS/observer/GGEMSTransportObserver.hh"
#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/opencl/GGEMSOpenCLContext.hh"
#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/random/GGEMSRandom.hh"
#include "GGEMS/sources/GGEMSSource.hh"
#include "GGEMS/sources/GGEMSSourceRecord.hh"
#include "GGEMS/sources/GGEMSSourceRunSnapshot.hh"
#include "GGEMS/transport/GGEMSTransportWorkload.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

#include "GGEMSNavigationTestDevices.hh"

namespace {

using namespace ggems::units;
using Box = ggems::geometry::GGEMSBox;
using BoxRecord = ggems::geometry::GGEMSBoxRecord;
using ObserverRecord = ggems::core::observer::GGEMSObserverRecord;
using ObserverRecordKind = ggems::core::observer::GGEMSObserverRecordKind;
using ParticleStatus = ggems::core::particles::GGEMSParticleStatus;
using ParticleType = ggems::core::particles::GGEMSParticleType;
using SourceRecord = ggems::core::sources::GGEMSSourceRecord;
using TransportRunConfig = ggems::core::transport::GGEMSTransportRunConfig;
using TransportRunReport = ggems::core::transport::GGEMSTransportRunReport;
using TransportWorkload = ggems::core::transport::GGEMSTransportWorkload;
using World = ggems::geometry::GGEMSWorld;
using ggems::geometry::MakePositionPM;

constexpr std::int64_t k_100_mm{100'000'000'000LL};
constexpr std::uint32_t k_box{1U};

auto Material(char const *name) -> ggems::core::materials::GGEMSMaterial {
  return ggems::core::materials::builtins::BuildBuiltInMaterial(name);
}

/*! \brief 200 mm Vacuum World. */
auto MakeWorld() -> World {
  return World{200_mm, 200_mm, 200_mm, Material("Vacuum")};
}

/*! \brief Asymmetric Water Box: x in [-20, 40], y in [-30, -10], z in
 * [-10, 20] mm. */
auto MakeBox() -> Box {
  return Box{
    60_mm, 20_mm, 30_mm,
    MakePositionPM(-20'000'000'000LL, -30'000'000'000LL, -10'000'000'000LL),
    Material("Water")};
}

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

auto MakeConfig(std::vector<SourceRecord> records) -> TransportRunConfig {
  TransportRunConfig config{};
  config.total_primary_count = records.size();
  config.source_population_records.resize(records.size());
  for (std::size_t index = 0U; index < records.size(); ++index) {
    config.source_ranges.push_back(
      {.projection_primary_begin = index, .primary_count = 1ULL});
  }
  config.source_records = std::move(records);
  config.observer_config.enabled = 1U;
  config.observer_config.capture_first_primary_count_per_source = 1U;
  return config;
}

/*! \brief Workload with one primary per source, two records per history. */
auto MakeWorkload(ggems::ocl::GGEMSOpenCLContext &context,
                  ggems::core::random::GGEMSRandom const &random,
                  std::size_t source_count, World const *world,
                  std::span<BoxRecord const> boxes)
  -> std::unique_ptr<TransportWorkload> {
  std::vector<std::shared_ptr<ggems::core::sources::GGEMSSource>> sources;
  for (std::size_t index = 0U; index < source_count; ++index) {
    sources.push_back(std::make_shared<ggems::core::sources::GGEMSSource>());
  }
  auto const snapshot =
    ggems::core::sources::BuildSourceConfigurationSnapshot(sources);
  return std::make_unique<TransportWorkload>(
    context, std::filesystem::path{GGEMS_TEST_KERNEL_ROOT}, random, 64U,
    *snapshot, 0ULL, 0U, static_cast<std::uint32_t>(2U * source_count), 0U,
    world, boxes);
}

/*! \brief One history: start, direction and its World exit. */
struct HistoryCase {
  std::array<std::int64_t, 3U> position_pm;
  std::array<double, 3U> direction;
  std::array<std::int64_t, 3U> exit_pm;
};

/*! \brief Records of one history in device publication order. */
auto RecordsOf(TransportRunReport const &report, std::uint64_t primary_id)
  -> std::vector<ObserverRecord> {
  std::vector<ObserverRecord> records;
  for (ObserverRecord const &record : report.observer_records) {
    if (record.global_primary_id == primary_id) {
      records.push_back(record);
    }
  }
  return records;
}

/*! \brief Runs every history in one launch and checks completion and exit. */
auto ExpectHistories(World const &world, std::span<BoxRecord const> boxes,
                     std::span<HistoryCase const> cases) -> void {
  for (auto &context : ggems::ocl::GGEMSOpenCL::GetInstance().GetContext()) {
    SCOPED_TRACE(context.GetDevice().GetName());

    std::vector<SourceRecord> records;
    for (HistoryCase const &history : cases) {
      records.push_back(MakeSource(history.position_pm, history.direction));
    }
    ggems::core::random::GGEMSRandom random{};
    random.SetEngine("philox").SetSeed(41ULL);
    auto workload = MakeWorkload(context, random, cases.size(), &world, boxes);
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
    EXPECT_EQ(report.observer_counters.overflow_count, 0U);

    for (std::size_t index = 0U; index < cases.size(); ++index) {
      SCOPED_TRACE(index);
      HistoryCase const &history = cases[index];
      auto const history_records = RecordsOf(report, index);
      ASSERT_EQ(history_records.size(), 2U);

      ObserverRecord const &source = history_records.front();
      EXPECT_EQ(source.record_kind,
                ggems::core::observer::ToKernelObserverRecordKind(
                  ObserverRecordKind::Source));
      EXPECT_EQ(source.position_x_pm, history.position_pm[0U]);
      EXPECT_EQ(source.position_y_pm, history.position_pm[1U]);
      EXPECT_EQ(source.position_z_pm, history.position_pm[2U]);

      ObserverRecord const &terminal = history_records.back();
      EXPECT_EQ(terminal.record_kind,
                ggems::core::observer::ToKernelObserverRecordKind(
                  ObserverRecordKind::Terminal));
      EXPECT_EQ(terminal.status, ggems::core::particles::ToKernelParticleStatus(
                                   ParticleStatus::EscapedWorld));
      EXPECT_EQ(terminal.position_x_pm, history.exit_pm[0U]);
      EXPECT_EQ(terminal.position_y_pm, history.exit_pm[1U]);
      EXPECT_EQ(terminal.position_z_pm, history.exit_pm[2U]);
      EXPECT_EQ(terminal.energy_micro_eV, records[index].energy_micro_eV);
      EXPECT_EQ(terminal.time_ps, records[index].time_start_ps);
      EXPECT_FLOAT_EQ(terminal.direction_x, records[index].axis_z_x);
      EXPECT_FLOAT_EQ(terminal.direction_y, records[index].axis_z_y);
      EXPECT_FLOAT_EQ(terminal.direction_z, records[index].axis_z_z);
    }
  }
}

} // namespace

// =============================================================================
// =============================================================================

TEST_F(GGEMSNavigationDeviceTest, AioninoCrossesTheBoxThenExitsTheWorld) {
  World const world = MakeWorld();
  std::array<BoxRecord, 1U> const boxes{MakeBox().BuildRecord(k_box, 1U)};

  // The intermediate boundaries of these histories are verified by the
  // traced-stream test; this launch checks the unmodified production stream.
  std::vector<HistoryCase> const cases{
    // Axis-aligned: World -> Box (-X face) -> World (+X face) -> World exit.
    {.position_pm = {-90'000'000'000LL, -20'000'000'000LL, 5'000'000'000LL},
     .direction = {1.0, 0.0, 0.0},
     .exit_pm = {k_100_mm, -20'000'000'000LL, 5'000'000'000LL}},
    // Body diagonal through the corner, exit through +Y, World exit +Z.
    {.position_pm = {-50'000'000'000LL, -60'000'000'000LL, -40'000'000'000LL},
     .direction = {1.0, 1.0, 1.0},
     .exit_pm = {90'000'000'000LL, 80'000'000'000LL, k_100_mm}},
    // Oblique miss (stored normalized (1,2,3)) with oracle-rounded World exit.
    {.position_pm = {-20'000'000'000LL, -90'000'000'000LL, -10'000'000'000LL},
     .direction = {1.0, 2.0, 3.0},
     .exit_pm = {16'666'665'304LL, -16'666'669'392LL, k_100_mm}},
    // Sliding along the +Y Box face plane: no boundary.
    {.position_pm = {-90'000'000'000LL, -10'000'000'000LL, 5'000'000'000LL},
     .direction = {1.0, 0.0, 0.0},
     .exit_pm = {k_100_mm, -10'000'000'000LL, 5'000'000'000LL}},
    // One picometer inside the +Y face plane: true crossing.
    {.position_pm = {-90'000'000'000LL, -10'000'000'001LL, 5'000'000'000LL},
     .direction = {1.0, 0.0, 0.0},
     .exit_pm = {k_100_mm, -10'000'000'001LL, 5'000'000'000LL}},
    // Moving along the +Z face plane diagonally: tangent, no boundary.
    {.position_pm = {-90'000'000'000LL, -40'000'000'000LL, 20'000'000'000LL},
     .direction = {1.0, 1.0, 0.0},
     .exit_pm = {50'000'000'000LL, k_100_mm, 20'000'000'000LL}},
  };

  ExpectHistories(world, boxes, cases);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSNavigationDeviceTest, AioninoBornInsideOrOnTheBoxSurface) {
  World const world = MakeWorld();
  std::array<BoxRecord, 1U> const boxes{MakeBox().BuildRecord(k_box, 1U)};

  std::vector<HistoryCase> const cases{
    // Born strictly inside, -Z: Box exit then World exit.
    {.position_pm = {0LL, -20'000'000'000LL, 0LL},
     .direction = {0.0, 0.0, -1.0},
     .exit_pm = {0LL, -20'000'000'000LL, -k_100_mm}},
    // Born on the -X face pointing inward: Box owner, exits through +X.
    {.position_pm = {-20'000'000'000LL, -20'000'000'000LL, 5'000'000'000LL},
     .direction = {1.0, 0.0, 0.0},
     .exit_pm = {k_100_mm, -20'000'000'000LL, 5'000'000'000LL}},
    // Born on the -X face pointing outward: World owner, no Box event.
    {.position_pm = {-20'000'000'000LL, -20'000'000'000LL, 5'000'000'000LL},
     .direction = {-1.0, 0.0, 0.0},
     .exit_pm = {-k_100_mm, -20'000'000'000LL, 5'000'000'000LL}},
    // Born on the (-X,-Y) edge with both faces left inward: Box owner.
    {.position_pm = {-20'000'000'000LL, -30'000'000'000LL, 0LL},
     .direction = {1.0, 1.0, 0.0},
     .exit_pm = {k_100_mm, 90'000'000'000LL, 0LL}},
    // Born inside, exits through the -Y face then the World -Y face.
    {.position_pm = {0LL, -20'000'000'000LL, 5'000'000'000LL},
     .direction = {0.0, -1.0, 0.0},
     .exit_pm = {0LL, -k_100_mm, 5'000'000'000LL}},
  };

  ExpectHistories(world, boxes, cases);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSNavigationDeviceTest, WorldOnlyAndOtherSpeciesAreUnchangedByBoxes) {
  World const world = MakeWorld();
  std::array<BoxRecord, 1U> const boxes{MakeBox().BuildRecord(k_box, 1U)};

  for (auto &context : ggems::ocl::GGEMSOpenCL::GetInstance().GetContext()) {
    SCOPED_TRACE(context.GetDevice().GetName());
    ggems::core::random::GGEMSRandom random{};
    random.SetEngine("pcg32").SetSeed(7ULL);
    auto workload = MakeWorkload(context, random, 2U, &world, boxes);
    // A Gamma through the Box keeps the one-meter diagnostic projection; a
    // second Aionino shares the launch so multi-source ordering is exercised.
    auto const config = MakeConfig(
      {MakeSource({-90'000'000'000LL, -20'000'000'000LL, 5'000'000'000LL},
                  {1.0, 0.0, 0.0}, ParticleType::Gamma),
       MakeSource({-90'000'000'000LL, -20'000'000'000LL, 5'000'000'000LL},
                  {1.0, 0.0, 0.0})});
    ASSERT_NO_THROW(workload->ValidateRunConfig(config));
    auto const report = workload->Run(config);

    EXPECT_EQ(report.counters.completed_history_count, 2U);
    EXPECT_EQ(report.counters.escaped_world_count, 1U);

    auto const gamma = RecordsOf(report, 0U);
    ASSERT_EQ(gamma.size(), 2U);
    EXPECT_EQ(gamma.back().position_x_pm,
              -90'000'000'000LL + 1'000'000'000'000LL);
    EXPECT_EQ(
      gamma.back().status,
      ggems::core::particles::ToKernelParticleStatus(ParticleStatus::Killed));

    auto const aionino = RecordsOf(report, 1U);
    ASSERT_EQ(aionino.size(), 2U);
    EXPECT_EQ(aionino.back().position_x_pm, k_100_mm);
    EXPECT_EQ(aionino.back().status,
              ggems::core::particles::ToKernelParticleStatus(
                ParticleStatus::EscapedWorld));
  }
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSNavigationDeviceTest, RunOwnsTheBoxAndRejectsInvalidAuthoring) {
  auto source = std::make_shared<ggems::core::sources::GGEMSSource>();
  source->SetAnalytic()
    .SetPrimaryCount(1ULL)
    .SetEmittedParticleType(ParticleType::Aionino)
    .SetPositionPicoMeter(-90'000'000'000LL, -20'000'000'000LL, 5'000'000'000LL)
    .SetDirection(1.0, 0.0, 0.0);
  auto rng = std::make_shared<ggems::core::random::GGEMSRandom>();
  rng->SetSeed(3ULL);
  auto observer =
    std::make_shared<ggems::core::observer::GGEMSTransportObserver>();
  observer->Enable().SetRecordCapacity(2U).CaptureFirstPrimaries(1U);

  {
    // Box without World.
    ggems::core::GGEMSRun run{};
    run.SetRandom(rng);
    run.AddSource(source);
    run.AddBox(MakeBox());
    EXPECT_THROW(run.Initialize(), ggems::core::GGEMSRecoverable);
  }
  {
    // Box touching the World +X face, and a second Box.
    ggems::core::GGEMSRun run{};
    run.SetRandom(rng);
    run.AddSource(source);
    run.SetWorld(MakeWorld());
    run.AddBox(Box{
      40_mm, 20_mm, 30_mm,
      MakePositionPM(60'000'000'000LL, -10'000'000'000LL, -15'000'000'000LL),
      Material("Water")});
    EXPECT_THROW(run.AddBox(MakeBox()), ggems::core::GGEMSRecoverable);
    EXPECT_THROW(run.Initialize(), ggems::core::GGEMSRecoverable);
  }
  {
    // Same Material as the World still gives a distinct physical volume: the
    // history crosses the Box and reaches the same World exit.
    ggems::core::GGEMSRun run{};
    run.SetRandom(rng);
    run.AddSource(source);
    run.SetObserver(observer);
    run.SetWorld(MakeWorld());
    run.AddBox(Box{
      60_mm, 20_mm, 30_mm,
      MakePositionPM(-20'000'000'000LL, -30'000'000'000LL, -10'000'000'000LL),
      Material("Vacuum")});
    ASSERT_NO_THROW(run.Initialize());
    ASSERT_NO_THROW(run.Run());
    ASSERT_EQ(observer->GetRecords().size(), 2U);
    EXPECT_EQ(observer->GetRecords().back().position_x_pm, k_100_mm);
  }

  ggems::core::GGEMSRun run{};
  run.SetRandom(rng);
  run.AddSource(source);
  run.SetObserver(observer);
  run.SetWorld(MakeWorld());
  run.AddBox(MakeBox());
  ASSERT_NO_THROW(run.Initialize());
  EXPECT_THROW(run.AddBox(MakeBox()), ggems::core::GGEMSRecoverable);
  ASSERT_NO_THROW(run.Run());

  auto const records = observer->GetRecords();
  ASSERT_EQ(records.size(), 2U);
  EXPECT_EQ(records[1U].position_x_pm, k_100_mm);
  EXPECT_EQ(records[1U].status, ggems::core::particles::ToKernelParticleStatus(
                                  ParticleStatus::EscapedWorld));
}
