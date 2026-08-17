#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <numbers>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/GGEMSTimeWindow.hh"
#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/radioactivity/GGEMSRadionuclideDefinition.hh"
#include "GGEMS/radioactivity/GGEMSRadionuclideEmission.hh"
#include "GGEMS/sources/GGEMSSourcePopulationPlan.hh"
#include "GGEMS/radioactivity/builtins/GGEMSBuiltInRadionuclides.hh"
#include "GGEMS/random/GGEMSRandom.hh"
#include "GGEMS/random/GGEMSRandomEngine.hh"
#include "GGEMS/sources/GGEMSEnergyDistribution.hh"
#include "GGEMS/sources/GGEMSSource.hh"
#include "GGEMS/sources/GGEMSSourcePopulation.hh"
#include "GGEMS/sources/GGEMSSourcePopulationRecord.hh"
#include "GGEMS/sources/GGEMSSourceRunSnapshot.hh"
#include "GGEMS/sources/GGEMSSourceTypes.hh"
#include "GGEMS/units/GGEMSActivityUnits.hh"

namespace {

using Definition = ggems::core::radioactivity::GGEMSRadionuclideDefinition;
using Emission = ggems::core::radioactivity::GGEMSRadionuclideEmission;
using Planner = ggems::core::sources::GGEMSSourcePopulationPlanner;
using Random = ggems::core::random::GGEMSRandom;
using Source = ggems::core::sources::GGEMSSource;
using SourcePtr = std::shared_ptr<Source>;

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeRandom() -> Random {
  Random random{};
  random.SetEngine(ggems::core::random::GGEMSRandomEngine::Philox)
      .SetSeed(0xB3'2000ULL);
  return random;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeCountSource(std::uint64_t primary_count) -> SourcePtr {
  auto source = std::make_shared<Source>();
  source->SetPrimaryCount(primary_count);
  return source;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto
MakeActivitySource(std::shared_ptr<Definition const> radionuclide,
                   long double activity_bq) -> SourcePtr {
  auto source = std::make_shared<Source>();
  source->SetRadionuclide(std::move(radionuclide),
                          ggems::units::Activity{activity_bq}, 0ULL);
  return source;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeDiscreteDefinition()
    -> std::shared_ptr<Definition const> {
  constexpr std::array<double, 3U> k_energies_keV{10.0, 20.0, 30.0};
  constexpr std::array<double, 3U> k_weights{1.0, 2.0, 1.0};
  std::vector<Emission> emissions;
  emissions.emplace_back(
      ggems::core::particles::GGEMSParticleType::Gamma, 0.75L,
      ggems::core::sources::GGEMSEnergyDistribution::BuildDiscreteLines(
          k_energies_keV, k_weights, "keV"));
  return std::make_shared<Definition const>("Synthetic-Discrete", 60.0L,
                                            std::move(emissions));
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideSourceSnapshot,
     ImmutablePackingPreservesSourcePrefixAndFlattenedEmissionOrder) {
  auto f18 = std::make_shared<Definition const>(
      ggems::core::radioactivity::builtins::BuildF18Radionuclide());
  auto c11 = std::make_shared<Definition const>(
      ggems::core::radioactivity::builtins::BuildC11Radionuclide());
  auto o15 = std::make_shared<Definition const>(
      ggems::core::radioactivity::builtins::BuildO15Radionuclide());
  auto discrete = MakeDiscreteDefinition();

  auto count_source = MakeCountSource(7ULL);
  count_source->SetEnergyMilliElectronVolt(42'000ULL);
  std::vector<SourcePtr> sources{
      count_source,
      MakeActivitySource(f18, 1.0L),
      MakeActivitySource(c11, 1.0L),
      MakeActivitySource(o15, 1.0L),
      MakeActivitySource(discrete, 1.0L),
      MakeActivitySource(f18, 1.0L),
  };

  auto const configuration =
      ggems::core::sources::BuildSourceConfigurationSnapshot(sources);
  auto const &energy_records = configuration->GetEnergyDistributionRecords();
  auto const &emission_records = configuration->GetEmissionRecords();
  auto const &definitions = configuration->GetRadionuclideDefinitions();

  constexpr std::size_t k_source_count{6U};
  constexpr std::size_t k_emission_count{9U};
  ASSERT_EQ(configuration->GetSourceCount(), k_source_count);
  ASSERT_EQ(configuration->GetEmissionCount(), k_emission_count);
  ASSERT_EQ(energy_records.size(), k_source_count + k_emission_count);
  ASSERT_EQ(emission_records.size(), k_emission_count);
  ASSERT_EQ(definitions.size(), k_source_count);

  EXPECT_EQ(definitions[0U], nullptr);
  EXPECT_EQ(definitions[1U], f18);
  EXPECT_EQ(definitions[2U], c11);
  EXPECT_EQ(definitions[3U], o15);
  EXPECT_EQ(definitions[4U], discrete);
  EXPECT_EQ(definitions[5U], f18);

  EXPECT_EQ(energy_records[0U].distribution_type,
            ggems::core::sources::ToKernelEnergyDistributionType(
                ggems::core::sources::GGEMSEnergyDistributionType::Mono));
  EXPECT_EQ(energy_records[0U].table_count, 0U);
  for (std::size_t index = 1U; index < k_source_count; ++index) {
    EXPECT_EQ(energy_records[index].distribution_type,
              ggems::core::sources::ToKernelEnergyDistributionType(
                  ggems::core::sources::GGEMSEnergyDistributionType::Unknown));
    EXPECT_EQ(energy_records[index].table_count, 0U);
  }

  constexpr std::array<ggems::core::particles::GGEMSParticleType, 9U>
      k_expected_particle_types{
          ggems::core::particles::GGEMSParticleType::Positron,
          ggems::core::particles::GGEMSParticleType::Electron,
          ggems::core::particles::GGEMSParticleType::Gamma,
          ggems::core::particles::GGEMSParticleType::Positron,
          ggems::core::particles::GGEMSParticleType::Positron,
          ggems::core::particles::GGEMSParticleType::Gamma,
          ggems::core::particles::GGEMSParticleType::Positron,
          ggems::core::particles::GGEMSParticleType::Electron,
          ggems::core::particles::GGEMSParticleType::Gamma,
      };
  constexpr std::array<std::uint64_t, 9U> k_expected_mono_energies{
      0ULL, 14'300ULL, 525'000ULL, 0ULL,       0ULL,
      0ULL, 0ULL,      14'300ULL,  525'000ULL,
  };

  for (std::size_t index = 0U; index < emission_records.size(); ++index) {
    auto const &emission = emission_records[index];
    EXPECT_EQ(emission.particle_type,
              ggems::core::particles::ToKernelParticleType(
                  k_expected_particle_types[index]));
    EXPECT_EQ(emission.energy_distribution_record_index,
              k_source_count + index);
    EXPECT_EQ(emission.mono_energy_milli_eV, k_expected_mono_energies[index]);
  }

  EXPECT_EQ(energy_records[6U].table_count, 1'268U);
  EXPECT_EQ(energy_records[7U].table_count, 0U);
  EXPECT_EQ(energy_records[8U].table_count, 0U);
  EXPECT_EQ(energy_records[9U].table_count, 1'921U);
  EXPECT_EQ(energy_records[10U].table_count, 3'465U);
  EXPECT_EQ(energy_records[11U].table_count, 3U);
  EXPECT_EQ(energy_records[12U].table_count, 1'268U);
  EXPECT_EQ(energy_records[13U].table_count, 0U);
  EXPECT_EQ(energy_records[14U].table_count, 0U);

  auto const &values = configuration->GetEnergyValuesMilliElectronVolt();
  auto const &relative_weights = configuration->GetRelativeWeights();
  auto const &tickets = configuration->GetCumulativeTicketUpperBounds();
  ASSERT_EQ(values.size(), 7'925U);
  ASSERT_EQ(relative_weights.size(), values.size());
  ASSERT_EQ(tickets.size(), values.size());

  constexpr std::array<std::uint64_t, 9U> k_expected_table_offsets{
      0ULL, 0ULL, 0ULL, 1'268ULL, 3'189ULL, 6'654ULL, 6'657ULL, 0ULL, 0ULL,
  };
  for (std::size_t index = 0U; index < k_emission_count; ++index) {
    auto const &record = energy_records[k_source_count + index];
    EXPECT_EQ(record.table_offset, k_expected_table_offsets[index]);
    if (record.table_count != 0U) {
      auto const final_ticket_index =
          static_cast<std::size_t>(record.table_offset) +
          static_cast<std::size_t>(record.table_count) - 1U;
      ASSERT_LT(final_ticket_index, tickets.size());
      EXPECT_EQ(tickets[final_ticket_index],
                ggems::core::sources::k_energy_ticket_space_size);
    }
  }

  auto const &discrete_record = energy_records[11U];
  ASSERT_EQ(discrete_record.table_count, 3U);
  auto const discrete_offset =
      static_cast<std::size_t>(discrete_record.table_offset);
  ASSERT_LE(discrete_offset + discrete_record.table_count, values.size());
  EXPECT_EQ(values[discrete_offset + 0U], 10'000'000ULL);
  EXPECT_EQ(values[discrete_offset + 1U], 20'000'000ULL);
  EXPECT_EQ(values[discrete_offset + 2U], 30'000'000ULL);
  EXPECT_DOUBLE_EQ(relative_weights[discrete_offset + 0U], 1.0);
  EXPECT_DOUBLE_EQ(relative_weights[discrete_offset + 1U], 2.0);
  EXPECT_DOUBLE_EQ(relative_weights[discrete_offset + 2U], 1.0);
  EXPECT_EQ(tickets[discrete_offset + 0U], 1'073'741'824ULL);
  EXPECT_EQ(tickets[discrete_offset + 1U], 3'221'225'472ULL);
  EXPECT_EQ(tickets[discrete_offset + 2U],
            ggems::core::sources::k_energy_ticket_space_size);
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideSourceSnapshot,
     RunSnapshotExactlyMirrorsMixedCandidateIncludingEmptyGroups) {
  auto discrete = MakeDiscreteDefinition();
  auto f18 = std::make_shared<Definition const>(
      ggems::core::radioactivity::builtins::BuildF18Radionuclide());
  std::vector<SourcePtr> sources{
      MakeCountSource(3ULL),
      MakeActivitySource(discrete, 25.0L),
      MakeCountSource(0ULL),
      MakeActivitySource(f18, 0.0L),
  };
  constexpr ggems::core::GGEMSTimeWindow k_window{
      .start_ps = 123'000ULL,
      .stop_ps = 1'000'000'123'000ULL,
  };

  auto const configuration =
      ggems::core::sources::BuildSourceConfigurationSnapshot(sources);
  Planner planner{sources, MakeRandom()};
  auto candidate = planner.BuildCandidate(k_window);
  auto const snapshot = ggems::core::sources::BuildSourceRunSnapshot(
      sources, configuration, candidate.GetPlan());

  auto const plan_sources = candidate.GetPlan().GetSources();
  auto const plan_groups = candidate.GetPlan().GetGroups();
  auto const plan_definitions =
      candidate.GetPlan().GetRadionuclideDefinitions();
  auto const &records = snapshot.GetRecords();
  auto const &ranges = snapshot.GetRanges();
  auto const &populations = snapshot.GetPopulationRecords();
  auto const &group_ranges = snapshot.GetGroupRanges();
  auto const &snapshot_definitions = snapshot.GetRadionuclideDefinitions();

  ASSERT_EQ(records.size(), sources.size());
  ASSERT_EQ(ranges.size(), sources.size());
  ASSERT_EQ(populations.size(), sources.size());
  ASSERT_EQ(group_ranges.size(), plan_groups.size());
  ASSERT_EQ(snapshot_definitions.size(), plan_definitions.size());
  EXPECT_TRUE(snapshot.HasActivityDrivenSource());
  EXPECT_EQ(snapshot.GetTotalPrimaryCount(),
            candidate.GetPlan().GetTotalPrimaryCount());
  EXPECT_EQ(&snapshot.GetSourceConfiguration(), configuration.get());

  for (std::size_t index = 0U; index < plan_definitions.size(); ++index) {
    EXPECT_EQ(snapshot_definitions[index], plan_definitions[index]);
  }

  for (std::size_t index = 0U; index < sources.size(); ++index) {
    SCOPED_TRACE(index);
    EXPECT_EQ(ranges[index].projection_primary_begin,
              plan_sources[index].run_primary_begin);
    EXPECT_EQ(ranges[index].primary_count,
              plan_sources[index].run_primary_end -
                  plan_sources[index].run_primary_begin);
    EXPECT_EQ(records[index].time_start_ps, k_window.start_ps);
    EXPECT_EQ(records[index].time_stop_ps, k_window.stop_ps);

    auto const source_record = sources[index]->BuildExecutionRecord();
    EXPECT_EQ(source_record.time_start_ps, 0ULL);
    EXPECT_EQ(source_record.time_stop_ps, 0ULL);
    EXPECT_EQ(populations[index].population_mode,
              ggems::core::sources::ToKernelSourcePopulationMode(
                  plan_sources[index].population_mode));
  }

  EXPECT_EQ(ranges[0U].primary_count, 3ULL);
  EXPECT_EQ(ranges[2U].primary_count, 0ULL);
  EXPECT_EQ(populations[0U].first_emission_index, 0U);
  EXPECT_EQ(populations[0U].emission_count, 0U);
  EXPECT_FLOAT_EQ(populations[0U].scaled_decay, 0.0F);
  EXPECT_EQ(populations[2U].first_emission_index, 0U);
  EXPECT_EQ(populations[2U].emission_count, 0U);
  EXPECT_FLOAT_EQ(populations[2U].scaled_decay, 0.0F);

  EXPECT_EQ(records[1U].emitted_particle_type,
            ggems::core::particles::ToKernelParticleType(
                ggems::core::particles::GGEMSParticleType::Unknown));
  EXPECT_EQ(records[1U].energy_milli_eV, 0ULL);
  EXPECT_EQ(records[3U].emitted_particle_type,
            ggems::core::particles::ToKernelParticleType(
                ggems::core::particles::GGEMSParticleType::Unknown));
  EXPECT_EQ(records[3U].energy_milli_eV, 0ULL);

  EXPECT_EQ(populations[1U].first_emission_index, 0U);
  EXPECT_EQ(populations[1U].emission_count, 1U);
  EXPECT_EQ(populations[3U].first_emission_index, 1U);
  EXPECT_EQ(populations[3U].emission_count, 3U);
  auto const expected_discrete_scaled_decay = static_cast<float>(
      std::numbers::ln2_v<long double> / discrete->GetHalfLifeSeconds());
  auto const expected_f18_scaled_decay = static_cast<float>(
      std::numbers::ln2_v<long double> / f18->GetHalfLifeSeconds());
  EXPECT_FLOAT_EQ(populations[1U].scaled_decay, expected_discrete_scaled_decay);
  EXPECT_FLOAT_EQ(populations[3U].scaled_decay, expected_f18_scaled_decay);

  for (std::size_t index = 0U; index < plan_groups.size(); ++index) {
    SCOPED_TRACE(index);
    EXPECT_EQ(group_ranges[index].source_local_primary_begin,
              plan_groups[index].source_local_primary_begin);
    EXPECT_EQ(group_ranges[index].primary_count,
              plan_groups[index].sampled_primary_count);
  }

  ASSERT_EQ(plan_groups.size(), 4U);
  for (std::size_t index = 1U; index < plan_groups.size(); ++index) {
    EXPECT_EQ(plan_groups[index].source_index, 3U);
    EXPECT_EQ(plan_groups[index].sampled_primary_count, 0ULL);
    EXPECT_EQ(group_ranges[index].primary_count, 0ULL);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideSourceSnapshot,
     RejectsLiveAndPackedDefinitionMismatchesWithoutConsumingCandidate) {
  auto planned_definition = std::make_shared<Definition const>(
      ggems::core::radioactivity::builtins::BuildF18Radionuclide());
  auto replacement_definition = MakeDiscreteDefinition();
  auto source = MakeActivitySource(planned_definition, 0.0L);
  std::vector<SourcePtr> sources{source};
  constexpr ggems::core::GGEMSTimeWindow k_window{
      .start_ps = 0ULL,
      .stop_ps = 1'000'000'000'000ULL,
  };

  auto const configuration =
      ggems::core::sources::BuildSourceConfigurationSnapshot(sources);
  Planner planner{sources, MakeRandom()};
  auto candidate = planner.BuildCandidate(k_window);
  auto const successful_snapshot = ggems::core::sources::BuildSourceRunSnapshot(
      sources, configuration, candidate.GetPlan());

  ASSERT_EQ(successful_snapshot.GetRadionuclideDefinitions().size(), 1U);
  EXPECT_EQ(successful_snapshot.GetRadionuclideDefinitions()[0U],
            planned_definition);
  ASSERT_EQ(successful_snapshot.GetPopulationRecords().size(), 1U);
  EXPECT_EQ(successful_snapshot.GetPopulationRecords()[0U].emission_count, 3U);

  source->SetRadionuclide(replacement_definition, ggems::units::Activity{0.0L},
                          0ULL);
  EXPECT_THROW((void)ggems::core::sources::BuildSourceRunSnapshot(
                   sources, configuration, candidate.GetPlan()),
               ggems::core::GGEMSExceptionBase);

  auto live_configuration =
      source->BuildActivityDrivenPopulationConfiguration();
  EXPECT_EQ(live_configuration.radionuclide, replacement_definition);
  EXPECT_FALSE(candidate.IsCommitted());
  EXPECT_EQ(planner.GetRevision(), 0ULL);
  EXPECT_EQ(successful_snapshot.GetRadionuclideDefinitions()[0U],
            planned_definition);

  source->SetRadionuclide(planned_definition, ggems::units::Activity{0.0L},
                          0ULL);
  std::vector<SourcePtr> mismatched_sources{
      MakeActivitySource(replacement_definition, 0.0L)};
  auto const mismatched_configuration =
      ggems::core::sources::BuildSourceConfigurationSnapshot(
          mismatched_sources);

  EXPECT_THROW((void)ggems::core::sources::BuildSourceRunSnapshot(
                   sources, mismatched_configuration, candidate.GetPlan()),
               ggems::core::GGEMSExceptionBase);

  live_configuration = source->BuildActivityDrivenPopulationConfiguration();
  EXPECT_EQ(live_configuration.radionuclide, planned_definition);
  EXPECT_FALSE(candidate.IsCommitted());
  EXPECT_EQ(planner.GetRevision(), 0ULL);
  EXPECT_EQ(successful_snapshot.GetRadionuclideDefinitions()[0U],
            planned_definition);

  EXPECT_NO_THROW(planner.CommitCandidate(candidate));
  EXPECT_TRUE(candidate.IsCommitted());
  EXPECT_EQ(planner.GetRevision(), 1ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideSourceSnapshot,
     ConfigurationAndRunSnapshotOwnRadionuclideDefinitions) {
  std::weak_ptr<Definition const> weak_definition;
  ggems::core::sources::GGEMSSourceConfigurationSnapshotPtr configuration;
  std::unique_ptr<ggems::core::sources::GGEMSSourceRunSnapshot> snapshot;

  {
    auto definition = MakeDiscreteDefinition();
    weak_definition = definition;
    auto source = MakeActivitySource(definition, 0.0L);
    source->SetBoxEmissionPicoMeter(101ULL, 103ULL, 107ULL)
        .SetPositionPicoMeter(11LL, -22LL, 33LL)
        .SetWeight(0.625F);
    std::vector<SourcePtr> sources{source};
    configuration =
        ggems::core::sources::BuildSourceConfigurationSnapshot(sources);
    Planner planner{sources, MakeRandom()};
    auto candidate = planner.BuildCandidate(
        {.start_ps = 10ULL, .stop_ps = 1'000'000'000'010ULL});
    snapshot = std::make_unique<ggems::core::sources::GGEMSSourceRunSnapshot>(
        ggems::core::sources::BuildSourceRunSnapshot(sources, configuration,
                                                     candidate.GetPlan()));

    definition.reset();
    sources.clear();
  }

  EXPECT_EQ(configuration->GetEnergyValuesMilliElectronVolt(),
            (std::vector<std::uint64_t>{10'000'000ULL, 20'000'000ULL,
                                        30'000'000ULL}));
  EXPECT_EQ(configuration->GetRelativeWeights(),
            (std::vector<double>{1.0, 2.0, 1.0}));
  EXPECT_EQ(configuration->GetCumulativeTicketUpperBounds(),
            (std::vector<std::uint64_t>{1'073'741'824ULL, 3'221'225'472ULL,
                                        4'294'967'296ULL}));

  ASSERT_EQ(snapshot->GetRecords().size(), 1U);
  auto const &record = snapshot->GetRecords().front();
  EXPECT_EQ(record.time_start_ps, 10ULL);
  EXPECT_EQ(record.time_stop_ps, 1'000'000'000'010ULL);
  EXPECT_EQ(record.emitted_particle_type,
            ggems::core::particles::ToKernelParticleType(
                ggems::core::particles::GGEMSParticleType::Unknown));
  EXPECT_EQ(record.energy_milli_eV, 0ULL);
  EXPECT_EQ(record.position_x_pm, 11LL);
  EXPECT_EQ(record.position_y_pm, -22LL);
  EXPECT_EQ(record.position_z_pm, 33LL);
  EXPECT_EQ(record.emission_geometry_type,
            ggems::core::sources::ToKernelEmissionGeometryType(
                ggems::core::sources::GGEMSEmissionGeometryType::Box));
  EXPECT_EQ(record.geometry_size_x_pm, 101ULL);
  EXPECT_EQ(record.geometry_size_y_pm, 103ULL);
  EXPECT_EQ(record.geometry_size_z_pm, 107ULL);
  EXPECT_FLOAT_EQ(record.weight, 0.625F);

  ASSERT_FALSE(weak_definition.expired());
  ASSERT_EQ(configuration->GetRadionuclideDefinitions().size(), 1U);
  ASSERT_NE(configuration->GetRadionuclideDefinitions()[0U], nullptr);
  EXPECT_EQ(configuration->GetRadionuclideDefinitions()[0U]->GetCanonicalName(),
            "Synthetic-Discrete");
  ASSERT_EQ(snapshot->GetRadionuclideDefinitions().size(), 1U);
  EXPECT_EQ(snapshot->GetRadionuclideDefinitions()[0U],
            configuration->GetRadionuclideDefinitions()[0U]);

  ASSERT_EQ(snapshot->GetPopulationRecords().size(), 1U);
  auto const &population = snapshot->GetPopulationRecords()[0U];
  EXPECT_EQ(
      population.population_mode,
      ggems::core::sources::ToKernelSourcePopulationMode(
          ggems::core::sources::GGEMSSourcePopulationMode::ActivityDriven));
  EXPECT_EQ(population.first_emission_index, 0U);
  EXPECT_EQ(population.emission_count, 1U);
  auto const expected_scaled_decay =
      static_cast<float>(std::numbers::ln2_v<long double> / 60.0L);
  EXPECT_FLOAT_EQ(population.scaled_decay, expected_scaled_decay);

  ASSERT_EQ(snapshot->GetRanges().size(), 1U);
  EXPECT_EQ(snapshot->GetRanges()[0U].projection_primary_begin, 0ULL);
  EXPECT_EQ(snapshot->GetRanges()[0U].primary_count, 0ULL);
  EXPECT_EQ(snapshot->GetTotalPrimaryCount(), 0ULL);

  ASSERT_EQ(snapshot->GetGroupRanges().size(), 1U);
  EXPECT_EQ(snapshot->GetGroupRanges()[0U].source_local_primary_begin, 0ULL);
  EXPECT_EQ(snapshot->GetGroupRanges()[0U].primary_count, 0ULL);

  configuration.reset();
  ASSERT_FALSE(weak_definition.expired());
  snapshot.reset();
  EXPECT_TRUE(weak_definition.expired());
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideSourceSnapshot,
     DuplicateActivitySourceObjectKeepsDistinctPackedSlotsAndRanges) {
  auto definition = std::make_shared<Definition const>(
      ggems::core::radioactivity::builtins::BuildF18Radionuclide());
  auto source = MakeActivitySource(definition, 0.0L);
  std::vector<SourcePtr> sources{source, source};

  auto const configuration =
      ggems::core::sources::BuildSourceConfigurationSnapshot(sources);
  ASSERT_EQ(configuration->GetSourceCount(), 2U);
  ASSERT_EQ(configuration->GetEmissionCount(), 6U);
  ASSERT_EQ(configuration->GetRadionuclideDefinitions().size(), 2U);
  EXPECT_EQ(configuration->GetRadionuclideDefinitions()[0U], definition);
  EXPECT_EQ(configuration->GetRadionuclideDefinitions()[1U], definition);

  Planner planner{sources, MakeRandom()};
  auto candidate = planner.BuildCandidate(
      {.start_ps = 0ULL, .stop_ps = 1'000'000'000'000ULL});
  auto const snapshot = ggems::core::sources::BuildSourceRunSnapshot(
      sources, configuration, candidate.GetPlan());

  ASSERT_EQ(snapshot.GetPopulationRecords().size(), 2U);
  ASSERT_EQ(snapshot.GetRadionuclideDefinitions().size(), 2U);
  EXPECT_EQ(snapshot.GetRadionuclideDefinitions()[0U], definition);
  EXPECT_EQ(snapshot.GetRadionuclideDefinitions()[1U], definition);
  ASSERT_EQ(snapshot.GetGroupRanges().size(), 6U);
  for (auto const &group_range : snapshot.GetGroupRanges()) {
    EXPECT_EQ(group_range.source_local_primary_begin, 0ULL);
    EXPECT_EQ(group_range.primary_count, 0ULL);
  }
  EXPECT_EQ(snapshot.GetTotalPrimaryCount(), 0ULL);

  ASSERT_EQ(snapshot.GetRanges().size(), 2U);
  EXPECT_EQ(snapshot.GetPopulationRecords()[0U].first_emission_index, 0U);
  EXPECT_EQ(snapshot.GetPopulationRecords()[1U].first_emission_index, 3U);
  EXPECT_EQ(snapshot.GetPopulationRecords()[0U].emission_count, 3U);
  EXPECT_EQ(snapshot.GetPopulationRecords()[1U].emission_count, 3U);
  EXPECT_EQ(snapshot.GetRanges()[0U].projection_primary_begin, 0ULL);
  EXPECT_EQ(snapshot.GetRanges()[0U].primary_count, 0ULL);
  EXPECT_EQ(snapshot.GetRanges()[1U].projection_primary_begin, 0ULL);
  EXPECT_EQ(snapshot.GetRanges()[1U].primary_count, 0ULL);
}

} // namespace
