#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideDefinition.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideEmission.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideProvenance.hh"
#include "GGEMS/core/sources/GGEMSEnergyDistribution.hh"
#include "GGEMS/core/sources/GGEMSSourceTypes.hh"

namespace {

// =============================================================================
// =============================================================================

using ggems::core::particles::GGEMSParticleType;
using ggems::core::radioactivity::GGEMSRadionuclideDefinition;
using ggems::core::radioactivity::GGEMSRadionuclideEmission;
using ggems::core::radioactivity::GGEMSRadionuclideProvenance;
using ggems::core::sources::GGEMSEnergyDistribution;
using ggems::core::sources::GGEMSEnergyDistributionType;

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeProvenance() -> GGEMSRadionuclideProvenance {
  return {"Synthetic authority", "Synthetic citation, preserved verbatim.",
          "Evaluation 1", std::string{"synthetic-reference"}};
}

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

TEST(GGEMSRadionuclideDefinitionTest, ProvenanceOwnsAndPreservesOriginalText) {
  std::string authority{"Synthetic authority"};
  std::string citation{" Citation punctuation: A/B; C. "};
  std::string evaluation{"2026-07 synthetic evaluation"};
  std::optional<std::string> reference{"doi:synthetic/value"};

  GGEMSRadionuclideProvenance const provenance{authority, citation, evaluation,
                                               reference};

  authority.clear();
  citation.clear();
  evaluation.clear();
  reference->clear();

  EXPECT_EQ(provenance.GetAuthority(), "Synthetic authority");
  EXPECT_EQ(provenance.GetCitation(), " Citation punctuation: A/B; C. ");
  EXPECT_EQ(provenance.GetEvaluationDateOrVersion(),
            "2026-07 synthetic evaluation");
  ASSERT_TRUE(provenance.GetReferenceIdentifier().has_value());
  EXPECT_EQ(*provenance.GetReferenceIdentifier(), "doi:synthetic/value");
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideDefinitionTest, ProvenanceValidatesRequiredText) {
  EXPECT_NO_THROW(((void)GGEMSRadionuclideProvenance{"Authority", "Citation",
                                                     "Version", std::nullopt}));

  EXPECT_THROW(((void)GGEMSRadionuclideProvenance{" \t\r\n", "Citation",
                                                  "Version", std::nullopt}),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(((void)GGEMSRadionuclideProvenance{"Authority", " \f\v",
                                                  "Version", std::nullopt}),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(((void)GGEMSRadionuclideProvenance{"Authority", "Citation", "",
                                                  std::nullopt}),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(((void)GGEMSRadionuclideProvenance{
                   "Authority", "Citation", "Version", std::string{" \t"}}),
               ggems::core::GGEMSExceptionBase);
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideDefinitionTest,
     OwnsMetadataChannelsAndNormalizedSelectionWeights) {
  std::string canonical_name{"Synthetic-Mixed"};
  std::vector<std::string> aliases{"SM", "Synthetic mixed display alias"};
  std::vector<GGEMSRadionuclideEmission> emissions;
  emissions.push_back(MakeDiscreteEmission(GGEMSParticleType::Alpha, 1.0L));
  emissions.push_back(
      MakeMonoEmission(GGEMSParticleType::Gamma, 0.3592L, 59'000'000ULL));
  emissions.push_back(
      MakeSpectrumEmission(GGEMSParticleType::Electron, 1.675L));

  GGEMSRadionuclideDefinition const definition{
      canonical_name, aliases, 4321.25L, MakeProvenance(), emissions};

  canonical_name.clear();
  aliases.clear();
  emissions.clear();

  EXPECT_EQ(definition.GetCanonicalName(), "Synthetic-Mixed");
  ASSERT_EQ(definition.GetAliases().size(), 2U);
  EXPECT_EQ(definition.GetAliases()[0U], "SM");
  EXPECT_EQ(definition.GetAliases()[1U], "Synthetic mixed display alias");
  EXPECT_EQ(definition.GetHalfLifeSeconds(), 4321.25L);
  EXPECT_EQ(definition.GetProvenance().GetAuthority(), "Synthetic authority");

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

  EXPECT_THROW(((void)GGEMSRadionuclideDefinition{
                   "", {}, 1.0L, MakeProvenance(), valid_emissions}),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(((void)GGEMSRadionuclideDefinition{
                   " \t\r\n", {}, 1.0L, MakeProvenance(), valid_emissions}),
               ggems::core::GGEMSExceptionBase);

  for (long double half_life :
       {0.0L, -1.0L, std::numeric_limits<long double>::quiet_NaN(),
        std::numeric_limits<long double>::infinity(),
        -std::numeric_limits<long double>::infinity()}) {
    SCOPED_TRACE(static_cast<double>(half_life));
    EXPECT_THROW(
        ((void)GGEMSRadionuclideDefinition{
            "Synthetic", {}, half_life, MakeProvenance(), valid_emissions}),
        ggems::core::GGEMSExceptionBase);
  }

  EXPECT_THROW(((void)GGEMSRadionuclideDefinition{
                   "Synthetic", {}, 1.0L, MakeProvenance(), {}}),
               ggems::core::GGEMSExceptionBase);
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideDefinitionTest,
     RejectsInvalidAndCollidingAliasesDuringDefinitionConstruction) {
  std::vector<GGEMSRadionuclideEmission> emissions;
  emissions.push_back(MakeMonoEmission(GGEMSParticleType::Gamma, 1.0L, 1ULL));

  EXPECT_THROW(
      ((void)GGEMSRadionuclideDefinition{
          "Synthetic-One", {" \t"}, 1.0L, MakeProvenance(), emissions}),
      ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(((void)GGEMSRadionuclideDefinition{"Synthetic-One",
                                                  {"Alias", " alias "},
                                                  1.0L,
                                                  MakeProvenance(),
                                                  emissions}),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(((void)GGEMSRadionuclideDefinition{"Synthetic-One",
                                                  {" synthetic-one "},
                                                  1.0L,
                                                  MakeProvenance(),
                                                  emissions}),
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
      "Synthetic-Electrons", {}, 10.0L, MakeProvenance(), std::move(emissions)};

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
      "Synthetic-Weak", {}, 100.0L, MakeProvenance(), std::move(emissions)};

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

  EXPECT_THROW(((void)GGEMSRadionuclideDefinition{"Synthetic-Overflow",
                                                  {},
                                                  1.0L,
                                                  MakeProvenance(),
                                                  std::move(emissions)}),
               ggems::core::GGEMSExceptionBase);
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideDefinitionTest,
     FailedConstructionLeavesCallerOwnedInputsUnchanged) {
  std::string canonical_name{"Synthetic-Stable"};
  std::vector<std::string> aliases{"Stable alias"};
  GGEMSRadionuclideProvenance provenance = MakeProvenance();
  std::vector<GGEMSRadionuclideEmission> emissions;
  emissions.push_back(MakeMonoEmission(GGEMSParticleType::Gamma, 1.0L, 1ULL));

  EXPECT_THROW(((void)GGEMSRadionuclideDefinition{canonical_name, aliases, 0.0L,
                                                  provenance, emissions}),
               ggems::core::GGEMSExceptionBase);

  EXPECT_EQ(canonical_name, "Synthetic-Stable");
  ASSERT_EQ(aliases.size(), 1U);
  EXPECT_EQ(aliases[0U], "Stable alias");
  EXPECT_EQ(provenance.GetAuthority(), "Synthetic authority");
  ASSERT_EQ(emissions.size(), 1U);
  EXPECT_EQ(emissions[0U].GetYieldPerDecay(), 1.0L);
}

} // namespace
