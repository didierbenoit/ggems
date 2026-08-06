#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <span>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSTimeWindow.hh"
#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/radioactivity/GGEMSDecayStatistics.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideDefinition.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideEmission.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideEmissionPlan.hh"
#include "GGEMS/core/radioactivity/builtins/GGEMSBuiltInRadionuclides.hh"
#include "GGEMS/core/random/GGEMSHostRandomStream.hh"
#include "GGEMS/core/random/GGEMSPoissonSampler.hh"
#include "GGEMS/core/random/GGEMSRandom.hh"
#include "GGEMS/core/random/GGEMSRandomEngine.hh"
#include "GGEMS/core/sources/GGEMSEnergyDistribution.hh"
#include "GGEMS/core/sources/GGEMSSource.hh"
#include "GGEMS/core/sources/GGEMSSourcePopulation.hh"
#include "GGEMS/core/units/GGEMSActivityUnits.hh"

namespace {

// =============================================================================
// =============================================================================

using Definition = ggems::core::radioactivity::GGEMSRadionuclideDefinition;
using Plan = ggems::core::radioactivity::GGEMSRadionuclideEmissionPlan;
using PlanGroup =
    ggems::core::radioactivity::GGEMSRadionuclideEmissionPlanGroup;
using PlanSource =
    ggems::core::radioactivity::GGEMSRadionuclideEmissionPlanSource;
using Planner = ggems::core::radioactivity::GGEMSRadionuclideEmissionPlanner;
using Random = ggems::core::random::GGEMSRandom;
using RandomEngine = ggems::core::random::GGEMSRandomEngine;
using Source = ggems::core::sources::GGEMSSource;
using SourcePtr = std::shared_ptr<Source>;

constexpr std::array<RandomEngine, 3U> k_engines{
    RandomEngine::JKISS, RandomEngine::PCG32, RandomEngine::Philox};
constexpr ggems::core::GGEMSTimeWindow k_one_second_window{
    .start_ps = 0ULL, .stop_ps = 1'000'000'000'000ULL};

template <typename T>
concept HasEmissionTimesPicoSecond =
    requires(T const &value) { value.GetEmissionTimesPicoSecond(); };

template <typename T>
concept HasTimeOffsets = requires(T const &value) {
  value.time_begin;
  value.time_end;
};

static_assert(!HasEmissionTimesPicoSecond<Plan>);
static_assert(!HasTimeOffsets<PlanSource>);
static_assert(!HasTimeOffsets<PlanGroup>);

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeRandom(RandomEngine engine = RandomEngine::PCG32,
                              std::uint64_t seed = 77'777ULL) -> Random {
  Random random{};
  random.SetEngine(engine).SetSeed(seed);
  return random;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeSyntheticDefinition(
    std::span<long double const> yields, long double half_life_seconds = 10.0L,
    std::string name = "Synthetic") -> std::shared_ptr<Definition const> {
  std::vector<ggems::core::radioactivity::GGEMSRadionuclideEmission> emissions;
  emissions.reserve(yields.size());

  for (std::size_t index = 0U; index < yields.size(); ++index) {
    emissions.emplace_back(
        ggems::core::particles::GGEMSParticleType::Gamma, yields[index],
        ggems::core::sources::GGEMSEnergyDistribution::BuildMono(
            1'000ULL + static_cast<std::uint64_t>(index)));
  }

  return std::make_shared<Definition const>(
      std::move(name), std::vector<std::string>{}, half_life_seconds,
      std::move(emissions));
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto
MakeActivitySource(std::shared_ptr<Definition const> const &definition,
                   long double activity, std::uint64_t reference_time_ps = 0ULL)
    -> SourcePtr {
  auto source = std::make_shared<Source>();
  source->SetActivityDrivenRadionuclide(
      definition, ggems::units::Activity{activity}, reference_time_ps);
  return source;
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

auto ExpectPlansEqual(Plan const &actual, Plan const &expected) -> void {
  EXPECT_EQ(actual.GetTimeWindow(), expected.GetTimeWindow());
  EXPECT_EQ(actual.GetTotalPrimaryCount(), expected.GetTotalPrimaryCount());

  auto const actual_sources = actual.GetSources();
  auto const expected_sources = expected.GetSources();
  ASSERT_EQ(actual_sources.size(), expected_sources.size());
  for (std::size_t index = 0U; index < actual_sources.size(); ++index) {
    auto const &lhs = actual_sources[index];
    auto const &rhs = expected_sources[index];
    EXPECT_EQ(lhs.source_index, rhs.source_index);
    EXPECT_EQ(lhs.population_mode, rhs.population_mode);
    EXPECT_EQ(lhs.expected_parent_decay_count, rhs.expected_parent_decay_count);
    EXPECT_EQ(lhs.emission_begin, rhs.emission_begin);
    EXPECT_EQ(lhs.emission_count, rhs.emission_count);
    EXPECT_EQ(lhs.run_primary_begin, rhs.run_primary_begin);
    EXPECT_EQ(lhs.run_primary_end, rhs.run_primary_end);
  }

  auto const actual_groups = actual.GetGroups();
  auto const expected_groups = expected.GetGroups();
  ASSERT_EQ(actual_groups.size(), expected_groups.size());
  for (std::size_t index = 0U; index < actual_groups.size(); ++index) {
    auto const &lhs = actual_groups[index];
    auto const &rhs = expected_groups[index];
    EXPECT_EQ(lhs.source_index, rhs.source_index);
    EXPECT_EQ(lhs.emission_index, rhs.emission_index);
    EXPECT_EQ(lhs.host_stream_id, rhs.host_stream_id);
    EXPECT_EQ(lhs.yield_per_decay, rhs.yield_per_decay);
    EXPECT_EQ(lhs.expected_emission_count, rhs.expected_emission_count);
    EXPECT_EQ(lhs.sampled_primary_count, rhs.sampled_primary_count);
    EXPECT_EQ(lhs.source_local_primary_begin, rhs.source_local_primary_begin);
    EXPECT_EQ(lhs.source_local_primary_end, rhs.source_local_primary_end);
    EXPECT_EQ(lhs.run_primary_begin, rhs.run_primary_begin);
    EXPECT_EQ(lhs.run_primary_end, rhs.run_primary_end);
  }
}

// =============================================================================
// =============================================================================

auto ExpectGroupCountMatchesReferenceStream(PlanGroup const &group,
                                            Random const &simulation_random)
    -> void {
  Random host_random = simulation_random;
  host_random.SetSeed(
      simulation_random.GetSeed() ^
      ggems::core::radioactivity::k_radionuclide_host_random_seed_domain_tag);
  ggems::core::random::GGEMSHostRandomStream reference{host_random,
                                                       group.host_stream_id};

  std::uint64_t const expected_count = ggems::core::random::SamplePoisson(
      group.expected_emission_count, reference);
  EXPECT_EQ(group.sampled_primary_count, expected_count);
}

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideEmissionPlanTest,
     BuildsF18GroupsFromOriginalYieldsAndIndependentStreams) {
  auto definition = std::make_shared<Definition const>(
      ggems::core::radioactivity::builtins::BuildF18Radionuclide());
  std::vector<SourcePtr> sources{MakeActivitySource(definition, 100.0L)};
  Random const random = MakeRandom();
  Planner planner{sources, random};

  auto candidate = planner.BuildCandidate(k_one_second_window);
  Plan const &plan = candidate.GetPlan();
  auto const source_entries = plan.GetSources();
  auto const groups = plan.GetGroups();

  ASSERT_EQ(source_entries.size(), 1U);
  ASSERT_EQ(groups.size(), 3U);
  EXPECT_EQ(source_entries[0U].source_index, 0U);
  EXPECT_EQ(source_entries[0U].population_mode,
            ggems::core::sources::GGEMSSourcePopulationMode::ActivityDriven);
  EXPECT_EQ(source_entries[0U].emission_begin, 0ULL);
  EXPECT_EQ(source_entries[0U].emission_count, 3ULL);

  long double const expected_parent =
      ggems::core::radioactivity::ComputeExpectedDecayEventCount(
          ggems::units::Activity{100.0L}, definition->GetHalfLifeSeconds(),
          0ULL, k_one_second_window);
  EXPECT_EQ(source_entries[0U].expected_parent_decay_count, expected_parent);

  constexpr std::array<long double, 3U> k_expected_yields{0.9686L, 0.00229L,
                                                          0.00020L};
  std::uint64_t source_local_begin{0ULL};

  for (std::size_t index = 0U; index < groups.size(); ++index) {
    auto const &group = groups[index];
    EXPECT_EQ(group.source_index, 0U);
    EXPECT_EQ(group.emission_index, static_cast<std::uint32_t>(index));
    EXPECT_EQ(group.host_stream_id, static_cast<std::uint64_t>(index));
    EXPECT_EQ(group.yield_per_decay, k_expected_yields[index]);
    EXPECT_EQ(group.expected_emission_count,
              expected_parent * k_expected_yields[index]);
    EXPECT_EQ(group.source_local_primary_begin, source_local_begin);
    EXPECT_EQ(group.source_local_primary_end,
              group.source_local_primary_begin + group.sampled_primary_count);
    EXPECT_EQ(group.run_primary_begin, group.source_local_primary_begin);
    EXPECT_EQ(group.run_primary_end, group.source_local_primary_end);
    source_local_begin = group.source_local_primary_end;

    ExpectGroupCountMatchesReferenceStream(group, random);
  }

  EXPECT_EQ(source_entries[0U].run_primary_end, source_local_begin);
  EXPECT_EQ(plan.GetTotalPrimaryCount(), source_local_begin);
  ASSERT_EQ(plan.GetRadionuclideDefinitions().size(), 1U);
  EXPECT_EQ(plan.GetRadionuclideDefinitions()[0U], definition);

  auto const emissions = definition->GetEmissions();
  ASSERT_EQ(emissions.size(), 3U);
  EXPECT_EQ(emissions[0U].GetParticleType(),
            ggems::core::particles::GGEMSParticleType::Positron);
  EXPECT_EQ(emissions[1U].GetParticleType(),
            ggems::core::particles::GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[2U].GetParticleType(),
            ggems::core::particles::GGEMSParticleType::Gamma);
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideEmissionPlanTest,
     PreservesC11AndO15SourceAndSingleGroupOrder) {
  auto c11 = std::make_shared<Definition const>(
      ggems::core::radioactivity::builtins::BuildC11Radionuclide());
  auto o15 = std::make_shared<Definition const>(
      ggems::core::radioactivity::builtins::BuildO15Radionuclide());
  std::vector<SourcePtr> sources{MakeActivitySource(c11, 50.0L),
                                 MakeActivitySource(o15, 75.0L)};
  Planner planner{sources, MakeRandom(RandomEngine::Philox)};

  auto candidate = planner.BuildCandidate(k_one_second_window);
  Plan const &plan = candidate.GetPlan();
  auto const source_entries = plan.GetSources();
  auto const groups = plan.GetGroups();

  ASSERT_EQ(source_entries.size(), 2U);
  ASSERT_EQ(groups.size(), 2U);
  EXPECT_EQ(groups[0U].source_index, 0U);
  EXPECT_EQ(groups[0U].emission_index, 0U);
  EXPECT_EQ(groups[0U].host_stream_id, 0ULL);
  EXPECT_EQ(groups[0U].yield_per_decay, 0.99750L);
  EXPECT_EQ(groups[1U].source_index, 1U);
  EXPECT_EQ(groups[1U].emission_index, 0U);
  EXPECT_EQ(groups[1U].host_stream_id, 1ULL);
  EXPECT_EQ(groups[1U].yield_per_decay, 0.999001L);
  EXPECT_EQ(source_entries[0U].emission_begin, 0ULL);
  EXPECT_EQ(source_entries[1U].emission_begin, 1ULL);
  EXPECT_EQ(source_entries[0U].run_primary_end,
            source_entries[1U].run_primary_begin);
  EXPECT_EQ(plan.GetRadionuclideDefinitions()[0U], c11);
  EXPECT_EQ(plan.GetRadionuclideDefinitions()[1U], o15);
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideEmissionPlanTest,
     PreservesSubUnitUnitAndSuperUnitYieldsWithoutNormalization) {
  constexpr std::array<long double, 3U> k_yields{0.25L, 1.0L, 1.5L};
  auto definition = MakeSyntheticDefinition(k_yields);
  std::vector<SourcePtr> sources{MakeActivitySource(definition, 20.0L)};
  Planner planner{sources, MakeRandom()};

  auto candidate = planner.BuildCandidate(k_one_second_window);
  auto const groups = candidate.GetPlan().GetGroups();
  ASSERT_EQ(groups.size(), k_yields.size());
  long double const expected_parent =
      candidate.GetPlan().GetSources()[0U].expected_parent_decay_count;

  for (std::size_t index = 0U; index < groups.size(); ++index) {
    EXPECT_EQ(groups[index].yield_per_decay, k_yields[index]);
    EXPECT_EQ(groups[index].expected_emission_count,
              expected_parent * k_yields[index]);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideEmissionPlanTest,
     UsesDomainSeparatedIndependentReferenceStreamForEveryEngine) {
  constexpr std::array<long double, 1U> k_yields{0.75L};
  auto definition = MakeSyntheticDefinition(k_yields);

  for (RandomEngine engine : k_engines) {
    SCOPED_TRACE(static_cast<std::uint32_t>(engine));

    std::vector<SourcePtr> sources{MakeActivitySource(definition, 50.0L)};
    Random const random = MakeRandom(engine, 91'337ULL);
    Planner planner{sources, random};
    auto candidate = planner.BuildCandidate(k_one_second_window);
    auto const groups = candidate.GetPlan().GetGroups();

    ASSERT_EQ(groups.size(), 1U);
    ExpectGroupCountMatchesReferenceStream(groups[0U], random);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideEmissionPlanTest,
     BuildsMixedSourcesWithContiguousRanges) {
  constexpr std::array<long double, 2U> k_yields{1.0L, 0.5L};
  auto definition = MakeSyntheticDefinition(k_yields);
  std::vector<SourcePtr> sources{MakeCountSource(3ULL),
                                 MakeActivitySource(definition, 40.0L),
                                 MakeCountSource(2ULL)};
  constexpr ggems::core::GGEMSTimeWindow k_window{
      .start_ps = 123ULL, .stop_ps = 1'000'000'000'123ULL};
  Planner planner{sources, MakeRandom()};

  auto candidate = planner.BuildCandidate(k_window);
  Plan const &plan = candidate.GetPlan();
  auto const source_entries = plan.GetSources();
  auto const groups = plan.GetGroups();

  ASSERT_EQ(source_entries.size(), 3U);
  ASSERT_EQ(groups.size(), 2U);
  EXPECT_EQ(source_entries[0U].population_mode,
            ggems::core::sources::GGEMSSourcePopulationMode::CountDriven);
  EXPECT_EQ(source_entries[0U].run_primary_begin, 0ULL);
  EXPECT_EQ(source_entries[0U].run_primary_end, 3ULL);
  EXPECT_EQ(source_entries[0U].emission_count, 0ULL);
  EXPECT_EQ(source_entries[1U].run_primary_begin, 3ULL);
  EXPECT_EQ(source_entries[1U].run_primary_end,
            source_entries[2U].run_primary_begin);
  EXPECT_EQ(source_entries[2U].run_primary_end, plan.GetTotalPrimaryCount());
  EXPECT_EQ(source_entries[2U].run_primary_end -
                source_entries[2U].run_primary_begin,
            2ULL);
  EXPECT_EQ(groups[0U].source_local_primary_begin, 0ULL);
  EXPECT_EQ(groups[0U].source_local_primary_end,
            groups[1U].source_local_primary_begin);
  EXPECT_EQ(groups[0U].run_primary_begin, 3ULL);
  EXPECT_EQ(groups[0U].run_primary_end, groups[1U].run_primary_begin);
  EXPECT_EQ(sources[0U]->GetPrimaryCount(), 3ULL);
  EXPECT_EQ(sources[2U]->GetPrimaryCount(), 2ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideEmissionPlanTest,
     ZeroActivityPreservesEmptyGroupsAndConsumesNoHostState) {
  constexpr std::array<long double, 2U> k_yields{1.0L, 2.0L};
  constexpr long double k_live_activity_bq{25.0L};
  constexpr std::uint64_t k_reference_time_ps{500'000'000'000ULL};
  constexpr ggems::core::GGEMSTimeWindow k_later_window{
      .start_ps = 1'000'000'000'000ULL, .stop_ps = 2'000'000'000'000ULL};

  for (RandomEngine engine : k_engines) {
    SCOPED_TRACE(static_cast<std::uint32_t>(engine));

    auto definition = MakeSyntheticDefinition(k_yields);
    auto source = MakeActivitySource(definition, 0.0L);
    std::vector<SourcePtr> sources{source};
    Random const random = MakeRandom(engine);
    Planner planner{sources, random};

    auto zero_candidate = planner.BuildCandidate(k_one_second_window);
    ASSERT_EQ(zero_candidate.GetPlan().GetGroups().size(), 2U);
    EXPECT_EQ(zero_candidate.GetPlan().GetTotalPrimaryCount(), 0ULL);
    for (auto const &group : zero_candidate.GetPlan().GetGroups()) {
      EXPECT_EQ(group.expected_emission_count, 0.0L);
      EXPECT_EQ(group.sampled_primary_count, 0ULL);
      EXPECT_EQ(group.source_local_primary_begin,
                group.source_local_primary_end);
      EXPECT_EQ(group.run_primary_begin, group.run_primary_end);
    }
    auto const source_entries = zero_candidate.GetPlan().GetSources();
    ASSERT_EQ(source_entries.size(), 1U);
    EXPECT_EQ(source_entries[0U].run_primary_begin,
              source_entries[0U].run_primary_end);
    planner.CommitCandidate(zero_candidate);

    auto replacement_definition =
        MakeSyntheticDefinition(k_yields, 10.0L, "Replacement");
    source->SetActivityDrivenRadionuclide(
        replacement_definition, ggems::units::Activity{k_live_activity_bq},
        k_reference_time_ps);
    EXPECT_THROW((void)planner.BuildCandidate(k_later_window),
                 ggems::core::GGEMSExceptionBase);
    EXPECT_EQ(planner.GetRevision(), 1ULL);

    source->SetActivityDrivenRadionuclide(
        definition, ggems::units::Activity{k_live_activity_bq},
        k_reference_time_ps);
    auto continued = planner.BuildCandidate(k_later_window);
    auto const continued_sources = continued.GetPlan().GetSources();
    ASSERT_EQ(continued_sources.size(), 1U);
    EXPECT_EQ(continued_sources[0U].expected_parent_decay_count,
              ggems::core::radioactivity::ComputeExpectedDecayEventCount(
                  ggems::units::Activity{k_live_activity_bq},
                  definition->GetHalfLifeSeconds(), k_reference_time_ps,
                  k_later_window));

    auto reference_source =
        MakeActivitySource(definition, k_live_activity_bq, k_reference_time_ps);
    std::vector<SourcePtr> reference_sources{reference_source};
    Planner reference{reference_sources, random};
    auto expected = reference.BuildCandidate(k_later_window);
    ExpectPlansEqual(continued.GetPlan(), expected.GetPlan());
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideEmissionPlanTest,
     PositiveMeanZeroCountConsumesOnlyPoissonState) {
  constexpr std::array<long double, 1U> k_yields{1.0L};
  auto definition = MakeSyntheticDefinition(k_yields);

  for (RandomEngine engine : k_engines) {
    SCOPED_TRACE(static_cast<std::uint32_t>(engine));

    auto source = MakeActivitySource(definition, 1.0e-30L);
    std::vector<SourcePtr> sources{source};
    Random const simulation_random = MakeRandom(engine, 27'182ULL);
    Planner planner{sources, simulation_random};

    auto zero_candidate = planner.BuildCandidate(k_one_second_window);
    auto const zero_groups = zero_candidate.GetPlan().GetGroups();
    ASSERT_EQ(zero_groups.size(), 1U);
    EXPECT_GT(zero_groups[0U].expected_emission_count, 0.0L);
    EXPECT_EQ(zero_groups[0U].sampled_primary_count, 0ULL);

    Random host_random = simulation_random;
    host_random.SetSeed(
        simulation_random.GetSeed() ^
        ggems::core::radioactivity::k_radionuclide_host_random_seed_domain_tag);
    ggems::core::random::GGEMSHostRandomStream reference{host_random, 0ULL};
    EXPECT_EQ(ggems::core::random::SamplePoisson(
                  zero_groups[0U].expected_emission_count, reference),
              0ULL);

    planner.CommitCandidate(zero_candidate);
    source->SetActivityDrivenRadionuclide(definition,
                                          ggems::units::Activity{50.0L}, 0ULL);
    auto continuation = planner.BuildCandidate(k_one_second_window);
    auto const continuation_groups = continuation.GetPlan().GetGroups();
    ASSERT_EQ(continuation_groups.size(), 1U);

    std::uint64_t const expected_count = ggems::core::random::SamplePoisson(
        continuation_groups[0U].expected_emission_count, reference);
    EXPECT_EQ(continuation_groups[0U].sampled_primary_count, expected_count);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideEmissionPlanTest,
     ContinuesDeterministicallyForEveryRandomEngine) {
  constexpr std::array<long double, 2U> k_yields{1.0L, 0.75L};

  for (RandomEngine engine : k_engines) {
    SCOPED_TRACE(static_cast<std::uint32_t>(engine));

    auto definition = MakeSyntheticDefinition(k_yields);
    auto source = MakeActivitySource(definition, 30.0L);
    std::vector<SourcePtr> sources{source};
    Random const random = MakeRandom(engine, 123'456ULL);
    Planner first{sources, random};
    Planner second{sources, random};

    auto first_candidate = first.BuildCandidate(k_one_second_window);
    auto second_candidate = second.BuildCandidate(k_one_second_window);
    ExpectPlansEqual(first_candidate.GetPlan(), second_candidate.GetPlan());
    first.CommitCandidate(first_candidate);
    second.CommitCandidate(second_candidate);

    auto first_continuation = first.BuildCandidate(k_one_second_window);
    auto second_continuation = second.BuildCandidate(k_one_second_window);
    ExpectPlansEqual(first_continuation.GetPlan(),
                     second_continuation.GetPlan());
    EXPECT_EQ(first.GetRevision(), 1ULL);
    EXPECT_EQ(second.GetRevision(), 1ULL);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideEmissionPlanTest,
     CommitRejectsDoubleStaleAndForeignCandidates) {
  constexpr std::array<long double, 1U> k_yields{1.0L};
  auto definition = MakeSyntheticDefinition(k_yields);
  std::vector<SourcePtr> sources{MakeActivitySource(definition, 20.0L)};
  Random const random = MakeRandom();
  Planner first{sources, random};
  Planner second{sources, random};

  auto ready = first.BuildCandidate(k_one_second_window);
  auto stale = first.BuildCandidate(k_one_second_window);
  auto foreign = second.BuildCandidate(k_one_second_window);
  EXPECT_FALSE(ready.IsCommitted());
  EXPECT_EQ(ready.GetBaseRevision(), 0ULL);

  EXPECT_NO_THROW(first.CommitCandidate(ready));
  EXPECT_TRUE(ready.IsCommitted());
  EXPECT_EQ(first.GetRevision(), 1ULL);
  EXPECT_THROW(first.CommitCandidate(ready), ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(first.CommitCandidate(stale), ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(first.CommitCandidate(foreign), ggems::core::GGEMSExceptionBase);
  EXPECT_EQ(first.GetRevision(), 1ULL);

  auto first_continuation = first.BuildCandidate(k_one_second_window);
  second.CommitCandidate(foreign);
  auto expected_continuation = second.BuildCandidate(k_one_second_window);
  ExpectPlansEqual(first_continuation.GetPlan(),
                   expected_continuation.GetPlan());
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideEmissionPlanTest,
     CommitInstallsCandidateStreamsExactlyOnceForEveryEngine) {
  constexpr std::array<long double, 1U> k_yields{1.0L};
  auto definition = MakeSyntheticDefinition(k_yields);

  for (RandomEngine engine : k_engines) {
    SCOPED_TRACE(static_cast<std::uint32_t>(engine));

    std::vector<SourcePtr> sources{MakeActivitySource(definition, 50.0L)};
    Random const simulation_random = MakeRandom(engine, 81'337ULL);
    Planner planner{sources, simulation_random};

    auto first = planner.BuildCandidate(k_one_second_window);
    auto repeated_without_commit = planner.BuildCandidate(k_one_second_window);
    ExpectPlansEqual(first.GetPlan(), repeated_without_commit.GetPlan());

    Random host_random = simulation_random;
    host_random.SetSeed(
        simulation_random.GetSeed() ^
        ggems::core::radioactivity::k_radionuclide_host_random_seed_domain_tag);
    ggems::core::random::GGEMSHostRandomStream reference{host_random, 0ULL};

    auto const first_groups = first.GetPlan().GetGroups();
    ASSERT_EQ(first_groups.size(), 1U);
    EXPECT_EQ(first_groups[0U].sampled_primary_count,
              ggems::core::random::SamplePoisson(
                  first_groups[0U].expected_emission_count, reference));

    planner.CommitCandidate(first);
    EXPECT_EQ(planner.GetRevision(), 1ULL);

    for (std::uint64_t cycle = 1ULL; cycle < 8ULL; ++cycle) {
      auto continuation = planner.BuildCandidate(k_one_second_window);
      auto const continuation_groups = continuation.GetPlan().GetGroups();
      ASSERT_EQ(continuation_groups.size(), 1U);
      EXPECT_EQ(
          continuation_groups[0U].sampled_primary_count,
          ggems::core::random::SamplePoisson(
              continuation_groups[0U].expected_emission_count, reference));

      planner.CommitCandidate(continuation);
      EXPECT_EQ(planner.GetRevision(), cycle + 1ULL);
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideEmissionPlanTest,
     FailedCandidateAfterRandomDrawDoesNotAdvancePersistentStreams) {
  constexpr std::array<long double, 1U> k_yields{1.0L};

  for (RandomEngine engine : k_engines) {
    SCOPED_TRACE(static_cast<std::uint32_t>(engine));

    auto definition = MakeSyntheticDefinition(k_yields);
    auto activity = MakeActivitySource(definition, 25.0L);
    auto overflowing =
        MakeCountSource(std::numeric_limits<std::uint64_t>::max());
    auto forcing_overflow = MakeCountSource(1ULL);
    std::vector<SourcePtr> sources{activity, overflowing, forcing_overflow};
    Random const random = MakeRandom(engine);
    Planner planner{sources, random};

    EXPECT_THROW((void)planner.BuildCandidate(k_one_second_window),
                 ggems::core::GGEMSExceptionBase);
    EXPECT_EQ(planner.GetRevision(), 0ULL);

    overflowing->SetPrimaryCount(1ULL);
    auto recovered = planner.BuildCandidate(k_one_second_window);

    std::vector<SourcePtr> reference_sources{
        MakeActivitySource(definition, 25.0L), MakeCountSource(1ULL),
        MakeCountSource(1ULL)};
    Planner reference{reference_sources, random};
    auto expected = reference.BuildCandidate(k_one_second_window);
    ExpectPlansEqual(recovered.GetPlan(), expected.GetPlan());
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideEmissionPlanTest,
     AppendingSourceDoesNotPerturbExistingStableOrdinals) {
  constexpr std::array<long double, 2U> k_first_yields{1.0L, 0.5L};
  constexpr std::array<long double, 1U> k_appended_yields{2.0L};
  auto first_definition = MakeSyntheticDefinition(k_first_yields);
  auto appended_definition =
      MakeSyntheticDefinition(k_appended_yields, 12.0L, "Appended");
  auto first_source = MakeActivitySource(first_definition, 40.0L);
  auto appended_source = MakeActivitySource(appended_definition, 30.0L);
  Random const random = MakeRandom(RandomEngine::Philox);

  std::vector<SourcePtr> original_sources{first_source};
  std::vector<SourcePtr> extended_sources{first_source, appended_source};
  Planner original{original_sources, random};
  Planner extended{extended_sources, random};

  auto original_candidate = original.BuildCandidate(k_one_second_window);
  auto extended_candidate = extended.BuildCandidate(k_one_second_window);
  auto const original_groups = original_candidate.GetPlan().GetGroups();
  auto const extended_groups = extended_candidate.GetPlan().GetGroups();

  ASSERT_EQ(original_groups.size(), 2U);
  ASSERT_EQ(extended_groups.size(), 3U);
  for (std::size_t index = 0U; index < original_groups.size(); ++index) {
    EXPECT_EQ(original_groups[index].host_stream_id,
              extended_groups[index].host_stream_id);
    EXPECT_EQ(original_groups[index].sampled_primary_count,
              extended_groups[index].sampled_primary_count);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideEmissionPlanTest,
     DuplicateSourceObjectsRemainDistinctSlotsAndStreams) {
  constexpr std::array<long double, 2U> k_yields{1.0L, 0.5L};
  auto definition = MakeSyntheticDefinition(k_yields);
  auto source = MakeActivitySource(definition, 30.0L);
  std::vector<SourcePtr> sources{source, source};
  Planner planner{sources, MakeRandom()};

  auto candidate = planner.BuildCandidate(k_one_second_window);
  auto const source_entries = candidate.GetPlan().GetSources();
  auto const groups = candidate.GetPlan().GetGroups();

  ASSERT_EQ(source_entries.size(), 2U);
  ASSERT_EQ(groups.size(), 4U);
  EXPECT_EQ(source_entries[0U].expected_parent_decay_count,
            source_entries[1U].expected_parent_decay_count);
  EXPECT_EQ(source_entries[0U].emission_begin, 0ULL);
  EXPECT_EQ(source_entries[1U].emission_begin, 2ULL);
  EXPECT_EQ(groups[0U].host_stream_id, 0ULL);
  EXPECT_EQ(groups[1U].host_stream_id, 1ULL);
  EXPECT_EQ(groups[2U].host_stream_id, 2ULL);
  EXPECT_EQ(groups[3U].host_stream_id, 3ULL);
  EXPECT_EQ(groups[0U].source_index, 0U);
  EXPECT_EQ(groups[2U].source_index, 1U);
  EXPECT_EQ(groups[0U].source_local_primary_begin, 0ULL);
  EXPECT_EQ(groups[2U].source_local_primary_begin, 0ULL);
  EXPECT_EQ(source_entries[0U].run_primary_end,
            source_entries[1U].run_primary_begin);
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideEmissionPlanTest,
     ResultIsIndependentOfSimulatedDeviceCount) {
  constexpr std::array<long double, 2U> k_yields{1.0L, 0.5L};
  auto definition = MakeSyntheticDefinition(k_yields);
  auto source = MakeActivitySource(definition, 30.0L);
  std::vector<SourcePtr> sources{source};
  Random const random = MakeRandom();

  Planner reference_planner{sources, random};
  auto reference = reference_planner.BuildCandidate(k_one_second_window);

  for (std::uint32_t simulated_device_count : {1U, 2U, 7U, 64U}) {
    SCOPED_TRACE(simulated_device_count);
    Planner planner{sources, random};
    auto candidate = planner.BuildCandidate(k_one_second_window);
    ExpectPlansEqual(candidate.GetPlan(), reference.GetPlan());
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideEmissionPlanTest,
     ActivityRequiresNonEmptyWindowWhileCountDrivenKeepsStaticMode) {
  constexpr std::array<long double, 1U> k_yields{1.0L};
  auto definition = MakeSyntheticDefinition(k_yields);
  std::vector<SourcePtr> activity_sources{MakeActivitySource(definition, 1.0L)};
  Planner activity_planner{activity_sources, MakeRandom()};
  EXPECT_THROW((void)activity_planner.BuildCandidate({}),
               ggems::core::GGEMSExceptionBase);

  std::vector<SourcePtr> count_sources{MakeCountSource(3ULL)};
  Planner count_planner{count_sources, MakeRandom()};
  auto count_candidate = count_planner.BuildCandidate({});
  Plan const &count_plan = count_candidate.GetPlan();
  ASSERT_EQ(count_plan.GetSources().size(), 1U);
  EXPECT_TRUE(count_plan.GetGroups().empty());
  EXPECT_EQ(count_plan.GetSources()[0U].run_primary_begin, 0ULL);
  EXPECT_EQ(count_plan.GetSources()[0U].run_primary_end, 3ULL);
  EXPECT_EQ(count_plan.GetTotalPrimaryCount(), 3ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideEmissionPlanTest,
     CountDrivenCountAboveUint32BuildsCompactPlan) {
  constexpr std::uint64_t k_primary_count =
      static_cast<std::uint64_t>(std::numeric_limits<std::uint32_t>::max()) +
      17ULL;
  std::vector<SourcePtr> sources{MakeCountSource(k_primary_count)};
  Planner planner{sources, MakeRandom()};

  auto candidate = planner.BuildCandidate({});
  Plan const &plan = candidate.GetPlan();
  auto const source_entries = plan.GetSources();

  ASSERT_EQ(source_entries.size(), 1U);
  EXPECT_TRUE(plan.GetGroups().empty());
  ASSERT_EQ(plan.GetRadionuclideDefinitions().size(), 1U);
  EXPECT_EQ(plan.GetRadionuclideDefinitions()[0U], nullptr);
  EXPECT_EQ(source_entries[0U].source_index, 0U);
  EXPECT_EQ(source_entries[0U].population_mode,
            ggems::core::sources::GGEMSSourcePopulationMode::CountDriven);
  EXPECT_EQ(source_entries[0U].emission_begin, 0ULL);
  EXPECT_EQ(source_entries[0U].emission_count, 0ULL);
  EXPECT_EQ(source_entries[0U].run_primary_begin, 0ULL);
  EXPECT_EQ(source_entries[0U].run_primary_end, k_primary_count);
  EXPECT_EQ(plan.GetTotalPrimaryCount(), k_primary_count);
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideEmissionPlanTest,
     ClinicallyLargeF18BuildsThreeGroupCompactPlan) {
  constexpr long double k_activity_bq{1'000'000'000.0L};
  constexpr ggems::core::GGEMSTimeWindow k_ten_second_window{
      .start_ps = 0ULL, .stop_ps = 10'000'000'000'000ULL};
  constexpr std::array<long double, 3U> k_expected_yields{0.9686L, 0.00229L,
                                                          0.00020L};

  auto definition = std::make_shared<Definition const>(
      ggems::core::radioactivity::builtins::BuildF18Radionuclide());
  std::vector<SourcePtr> sources{MakeActivitySource(definition, k_activity_bq)};
  Random const random = MakeRandom(RandomEngine::PCG32, 91'337ULL);
  Planner planner{sources, random};

  auto candidate = planner.BuildCandidate(k_ten_second_window);
  Plan const &plan = candidate.GetPlan();
  auto const source_entries = plan.GetSources();
  auto const groups = plan.GetGroups();

  ASSERT_EQ(source_entries.size(), 1U);
  ASSERT_EQ(groups.size(), 3U);
  ASSERT_EQ(plan.GetRadionuclideDefinitions().size(), 1U);
  EXPECT_EQ(plan.GetRadionuclideDefinitions()[0U], definition);
  EXPECT_EQ(source_entries[0U].emission_begin, 0ULL);
  EXPECT_EQ(source_entries[0U].emission_count, 3ULL);
  EXPECT_EQ(source_entries[0U].run_primary_begin, 0ULL);

  long double const expected_parent =
      ggems::core::radioactivity::ComputeExpectedDecayEventCount(
          ggems::units::Activity{k_activity_bq},
          definition->GetHalfLifeSeconds(), 0ULL, k_ten_second_window);
  EXPECT_EQ(source_entries[0U].expected_parent_decay_count, expected_parent);

  std::uint64_t next_source_local_primary{0ULL};
  std::uint64_t next_run_primary{0ULL};

  for (std::size_t index = 0U; index < groups.size(); ++index) {
    PlanGroup const &group = groups[index];
    EXPECT_EQ(group.source_index, 0U);
    EXPECT_EQ(group.emission_index, static_cast<std::uint32_t>(index));
    EXPECT_EQ(group.host_stream_id, static_cast<std::uint64_t>(index));
    EXPECT_EQ(group.yield_per_decay, k_expected_yields[index]);
    EXPECT_EQ(group.expected_emission_count,
              expected_parent * k_expected_yields[index]);
    EXPECT_EQ(group.source_local_primary_begin, next_source_local_primary);
    EXPECT_EQ(group.run_primary_begin, next_run_primary);
    EXPECT_EQ(group.source_local_primary_end - group.source_local_primary_begin,
              group.sampled_primary_count);
    EXPECT_EQ(group.run_primary_end - group.run_primary_begin,
              group.sampled_primary_count);

    long double const deviation =
        std::abs(static_cast<long double>(group.sampled_primary_count) -
                 group.expected_emission_count);
    long double const conservative_bound =
        (20.0L * std::sqrt(group.expected_emission_count)) + 1.0L;
    EXPECT_LE(deviation, conservative_bound);
    ExpectGroupCountMatchesReferenceStream(group, random);

    next_source_local_primary = group.source_local_primary_end;
    next_run_primary = group.run_primary_end;
  }

  EXPECT_EQ(source_entries[0U].run_primary_end, next_run_primary);
  EXPECT_EQ(plan.GetTotalPrimaryCount(), next_run_primary);
  EXPECT_GT(
      plan.GetTotalPrimaryCount(),
      static_cast<std::uint64_t>(std::numeric_limits<std::uint32_t>::max()));
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideEmissionPlanTest,
     RejectsExpectedCountAndCheckedTotalOverflowWithoutCommit) {
  constexpr std::array<long double, 1U> k_huge_yield{
      std::numeric_limits<long double>::max()};
  auto huge_definition = MakeSyntheticDefinition(k_huge_yield, 1.0e20L, "Huge");
  std::vector<SourcePtr> huge_sources{
      MakeActivitySource(huge_definition, 1.0e300L)};
  Planner huge_planner{huge_sources, MakeRandom()};
  EXPECT_THROW((void)huge_planner.BuildCandidate(ggems::core::GGEMSTimeWindow{
                   .start_ps = 0ULL,
                   .stop_ps = std::numeric_limits<std::uint64_t>::max()}),
               ggems::core::GGEMSExceptionBase);
  EXPECT_EQ(huge_planner.GetRevision(), 0ULL);

  std::vector<SourcePtr> count_sources{
      MakeCountSource(std::numeric_limits<std::uint64_t>::max()),
      MakeCountSource(1ULL)};
  Planner count_planner{count_sources, MakeRandom()};
  EXPECT_THROW((void)count_planner.BuildCandidate({}),
               ggems::core::GGEMSExceptionBase);
  EXPECT_EQ(count_planner.GetRevision(), 0ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideEmissionPlanTest,
     RetainsDefinitionOwnershipBeyondCallerAndPlannerLifetime) {
  constexpr std::array<long double, 1U> k_yields{1.0L};
  auto definition = MakeSyntheticDefinition(k_yields);
  std::weak_ptr<Definition const> retained = definition;
  std::unique_ptr<
      ggems::core::radioactivity::GGEMSRadionuclideEmissionPlanCandidate>
      candidate;

  {
    auto source = MakeActivitySource(definition, 10.0L);
    std::vector<SourcePtr> sources{source};
    Planner planner{sources, MakeRandom()};
    auto local = planner.BuildCandidate(k_one_second_window);
    candidate = std::make_unique<
        ggems::core::radioactivity::GGEMSRadionuclideEmissionPlanCandidate>(
        std::move(local));
    definition.reset();
    source.reset();
  }

  ASSERT_FALSE(retained.expired());
  ASSERT_EQ(candidate->GetPlan().GetRadionuclideDefinitions().size(), 1U);
  EXPECT_EQ(
      candidate->GetPlan().GetRadionuclideDefinitions()[0U]->GetCanonicalName(),
      "Synthetic");
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideEmissionPlanTest,
     ReusingAnEarlierWindowDoesNotResetPersistentHostStreams) {
  constexpr std::array<long double, 1U> k_yields{1.0L};
  auto definition = MakeSyntheticDefinition(k_yields);
  auto source = MakeActivitySource(definition, 20.0L);
  std::vector<SourcePtr> sources{source};
  Random const random = MakeRandom();
  Planner planner{sources, random};
  Planner reference{sources, random};

  auto first = planner.BuildCandidate(k_one_second_window);
  auto reference_first = reference.BuildCandidate(k_one_second_window);
  planner.CommitCandidate(first);
  reference.CommitCandidate(reference_first);

  auto after_rewind = planner.BuildCandidate(k_one_second_window);
  auto expected = reference.BuildCandidate(k_one_second_window);
  ExpectPlansEqual(after_rewind.GetPlan(), expected.GetPlan());
}
