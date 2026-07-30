#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideDefinition.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideEmission.hh"
#include "GGEMS/core/sources/GGEMSEnergyDistribution.hh"
#include "GGEMS/core/sources/GGEMSSourceTypes.hh"

namespace {

// =============================================================================
// =============================================================================

using ggems::core::particles::GGEMSParticleType;
using ggems::core::radioactivity::GGEMSRadionuclideDefinition;
using ggems::core::radioactivity::GGEMSRadionuclideEmission;
using ggems::core::sources::GGEMSEnergyDistribution;
using ggems::core::sources::GGEMSEnergyDistributionType;

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeMonoEmission(GGEMSParticleType particle_type,
                                    long double yield_per_decay,
                                    std::uint64_t energy_milli_eV)
    -> GGEMSRadionuclideEmission {
  return {particle_type, yield_per_decay,
          GGEMSEnergyDistribution::BuildMono(energy_milli_eV)};
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeDiscreteEmission(GGEMSParticleType particle_type,
                                        long double yield_per_decay)
    -> GGEMSRadionuclideEmission {
  constexpr std::array<double, 3U> energies{4.0, 5.0, 6.0};
  constexpr std::array<double, 3U> weights{2.0, 3.0, 1.0};
  return {
      particle_type, yield_per_decay,
      GGEMSEnergyDistribution::BuildDiscreteLines(energies, weights, "MeV")};
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeSpectrumEmission(GGEMSParticleType particle_type,
                                        long double yield_per_decay)
    -> GGEMSRadionuclideEmission {
  constexpr std::array<double, 3U> centers{100.0, 102.0, 104.0};
  constexpr std::array<double, 3U> weights{1.0, 4.0, 1.0};
  return {
      particle_type, yield_per_decay,
      GGEMSEnergyDistribution::BuildRegularSpectrum(centers, weights, "keV")};
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideDefinitionTest,
     OwnsChannelsAndNormalizedSelectionWeights) {
  std::string canonical_name{"Synthetic-Mixed"};
  std::vector<std::string> aliases{"SM", "Synthetic mixed display alias"};
  std::vector<GGEMSRadionuclideEmission> emissions;
  emissions.push_back(MakeDiscreteEmission(GGEMSParticleType::Alpha, 1.0L));
  emissions.push_back(
      MakeMonoEmission(GGEMSParticleType::Gamma, 0.3592L, 59'000'000ULL));
  emissions.push_back(
      MakeSpectrumEmission(GGEMSParticleType::Electron, 1.675L));

  GGEMSRadionuclideDefinition const definition{canonical_name, aliases,
                                               4321.25L, emissions};

  canonical_name.clear();
  aliases.clear();
  emissions.clear();

  EXPECT_EQ(definition.GetCanonicalName(), "Synthetic-Mixed");
  ASSERT_EQ(definition.GetAliases().size(), 2U);
  EXPECT_EQ(definition.GetAliases()[0U], "SM");
  EXPECT_EQ(definition.GetAliases()[1U], "Synthetic mixed display alias");
  EXPECT_EQ(definition.GetHalfLifeSeconds(), 4321.25L);

  auto const stored_emissions = definition.GetEmissions();
  auto const selection_weights = definition.GetChannelSelectionWeights();
  ASSERT_EQ(stored_emissions.size(), 3U);
  ASSERT_EQ(selection_weights.size(), stored_emissions.size());

  EXPECT_EQ(stored_emissions[0U].GetParticleType(), GGEMSParticleType::Alpha);
  EXPECT_EQ(stored_emissions[1U].GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_EQ(stored_emissions[2U].GetParticleType(),
            GGEMSParticleType::Electron);
  EXPECT_EQ(stored_emissions[0U].GetYieldPerDecay(), 1.0L);
  EXPECT_EQ(stored_emissions[1U].GetYieldPerDecay(), 0.3592L);
  EXPECT_EQ(stored_emissions[2U].GetYieldPerDecay(), 1.675L);

  long double const expected_total = 1.0L + 0.3592L + 1.675L;
  long double const tolerance =
      std::numeric_limits<long double>::epsilon() * expected_total * 8.0L;
  EXPECT_LE(std::abs(definition.GetTotalYieldPerDecay() - expected_total),
            tolerance);

  for (std::size_t index = 0U; index < stored_emissions.size(); ++index) {
    long double const expected = stored_emissions[index].GetYieldPerDecay() /
                                 definition.GetTotalYieldPerDecay();
    EXPECT_EQ(selection_weights[index], expected);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideDefinitionTest, RejectsInvalidNameHalfLifeAndEmissions) {
  std::vector<GGEMSRadionuclideEmission> valid_emissions;
  valid_emissions.push_back(
      MakeMonoEmission(GGEMSParticleType::Gamma, 1.0L, 1ULL));

  EXPECT_THROW(
      ((void)GGEMSRadionuclideDefinition{"", {}, 1.0L, valid_emissions}),
      ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(
      ((void)GGEMSRadionuclideDefinition{" \t\r\n", {}, 1.0L, valid_emissions}),
      ggems::core::GGEMSExceptionBase);

  for (long double half_life :
       {0.0L, -1.0L, std::numeric_limits<long double>::quiet_NaN(),
        std::numeric_limits<long double>::infinity(),
        -std::numeric_limits<long double>::infinity()}) {
    SCOPED_TRACE(static_cast<double>(half_life));
    EXPECT_THROW(((void)GGEMSRadionuclideDefinition{
                     "Synthetic", {}, half_life, valid_emissions}),
                 ggems::core::GGEMSExceptionBase);
  }

  EXPECT_THROW(((void)GGEMSRadionuclideDefinition{"Synthetic", {}, 1.0L, {}}),
               ggems::core::GGEMSExceptionBase);
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideDefinitionTest,
     RejectsInvalidAndCollidingAliasesDuringDefinitionConstruction) {
  std::vector<GGEMSRadionuclideEmission> emissions;
  emissions.push_back(MakeMonoEmission(GGEMSParticleType::Gamma, 1.0L, 1ULL));

  EXPECT_THROW(((void)GGEMSRadionuclideDefinition{
                   "Synthetic-One", {" \t"}, 1.0L, emissions}),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(((void)GGEMSRadionuclideDefinition{
                   "Synthetic-One", {"Alias", " alias "}, 1.0L, emissions}),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(((void)GGEMSRadionuclideDefinition{
                   "Synthetic-One", {" synthetic-one "}, 1.0L, emissions}),
               ggems::core::GGEMSExceptionBase);
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideDefinitionTest,
     PreservesOrderAndDoesNotMergeMatchingParticleTypes) {
  std::vector<GGEMSRadionuclideEmission> emissions;
  emissions.push_back(MakeSpectrumEmission(GGEMSParticleType::Electron, 0.75L));
  emissions.push_back(MakeDiscreteEmission(GGEMSParticleType::Electron, 1.25L));

  GGEMSRadionuclideDefinition const definition{
      "Synthetic-Electrons", {}, 10.0L, std::move(emissions)};

  auto const stored = definition.GetEmissions();
  ASSERT_EQ(stored.size(), 2U);
  EXPECT_EQ(stored[0U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(stored[1U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(stored[0U].GetYieldPerDecay(), 0.75L);
  EXPECT_EQ(stored[1U].GetYieldPerDecay(), 1.25L);
  EXPECT_EQ(stored[0U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  EXPECT_EQ(stored[1U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideDefinitionTest,
     PreservesTinyPositiveYieldWithoutEnergyTicketQuantization) {
  constexpr long double tiny_yield{1.0e-12L};
  std::vector<GGEMSRadionuclideEmission> emissions;
  emissions.push_back(MakeMonoEmission(GGEMSParticleType::Gamma, 1.0L, 2ULL));
  emissions.push_back(
      MakeMonoEmission(GGEMSParticleType::Electron, tiny_yield, 3ULL));

  GGEMSRadionuclideDefinition const definition{
      "Synthetic-Weak", {}, 100.0L, std::move(emissions)};

  ASSERT_EQ(definition.GetEmissions().size(), 2U);
  ASSERT_EQ(definition.GetChannelSelectionWeights().size(), 2U);
  EXPECT_EQ(definition.GetEmissions()[1U].GetYieldPerDecay(), tiny_yield);
  EXPECT_GT(definition.GetChannelSelectionWeights()[1U], 0.0L);
  EXPECT_EQ(definition.GetChannelSelectionWeights()[1U],
            tiny_yield / definition.GetTotalYieldPerDecay());
  EXPECT_TRUE(definition.GetEmissions()[1U]
                  .GetEnergyDistribution()
                  .GetCumulativeTicketUpperBounds()
                  .empty());
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideDefinitionTest, RejectsNonFiniteTotalYield) {
  long double const maximum = std::numeric_limits<long double>::max();
  std::vector<GGEMSRadionuclideEmission> emissions;
  emissions.push_back(
      MakeMonoEmission(GGEMSParticleType::Gamma, maximum, 1ULL));
  emissions.push_back(
      MakeMonoEmission(GGEMSParticleType::Electron, maximum, 2ULL));

  EXPECT_THROW(((void)GGEMSRadionuclideDefinition{
                   "Synthetic-Overflow", {}, 1.0L, std::move(emissions)}),
               ggems::core::GGEMSExceptionBase);
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideDefinitionTest,
     FailedConstructionLeavesCallerOwnedInputsUnchanged) {
  std::string canonical_name{"Synthetic-Stable"};
  std::vector<std::string> aliases{"Stable alias"};
  std::vector<GGEMSRadionuclideEmission> emissions;
  emissions.push_back(MakeMonoEmission(GGEMSParticleType::Gamma, 1.0L, 1ULL));

  EXPECT_THROW(((void)GGEMSRadionuclideDefinition{canonical_name, aliases, 0.0L,
                                                  emissions}),
               ggems::core::GGEMSExceptionBase);

  EXPECT_EQ(canonical_name, "Synthetic-Stable");
  ASSERT_EQ(aliases.size(), 1U);
  EXPECT_EQ(aliases[0U], "Stable alias");
  ASSERT_EQ(emissions.size(), 1U);
  EXPECT_EQ(emissions[0U].GetYieldPerDecay(), 1.0L);
}

} // namespace
