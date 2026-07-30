#include <algorithm>
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
#include "GGEMS/core/radioactivity/GGEMSBetaSpectrumBuilder.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideDefinition.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideEmission.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideLibrary.hh"
#include "GGEMS/core/radioactivity/builtins/GGEMSBuiltInRadionuclides.hh"
#include "GGEMS/core/sources/GGEMSEnergyDistribution.hh"
#include "GGEMS/core/sources/GGEMSSourceTypes.hh"

namespace {

// =============================================================================
// =============================================================================

using ggems::core::particles::GGEMSParticleType;
using ggems::core::radioactivity::GGEMSBetaSpectrumModel;
using ggems::core::radioactivity::GGEMSRadionuclideDefinition;
using ggems::core::radioactivity::GGEMSRadionuclideEmission;
using ggems::core::radioactivity::GGEMSRadionuclideLibrary;
using ggems::core::radioactivity::builtins::BuildF18PositronSpectrum;
using ggems::core::radioactivity::builtins::BuildF18Radionuclide;
using ggems::core::sources::GGEMSEnergyDistribution;
using ggems::core::sources::GGEMSEnergyDistributionType;
using ggems::core::sources::k_energy_ticket_space_size;

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeSyntheticDefinition(std::size_t index)
    -> GGEMSRadionuclideDefinition {
  std::vector<GGEMSRadionuclideEmission> emissions;
  emissions.emplace_back(GGEMSParticleType::Gamma, 1.0L,
                         GGEMSEnergyDistribution::BuildMono(1ULL));
  return {"Synthetic-" + std::to_string(index), {}, 1.0L, std::move(emissions)};
}

// =============================================================================
// =============================================================================

TEST(GGEMSF18Test, BuildsExactIdentity) {
  GGEMSRadionuclideDefinition const definition = BuildF18Radionuclide();

  EXPECT_EQ(definition.GetCanonicalName(), "F-18");
  ASSERT_EQ(definition.GetAliases().size(), 3U);
  EXPECT_EQ(definition.GetAliases()[0U], "F18");
  EXPECT_EQ(definition.GetAliases()[1U], "18F");
  EXPECT_EQ(definition.GetAliases()[2U], "Fluorine-18");
  EXPECT_EQ(definition.GetHalfLifeSeconds(), 6'584.04L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSF18Test, BuildsThreeOrderedFlattenedEmissions) {
  GGEMSRadionuclideDefinition const definition = BuildF18Radionuclide();

  auto const emissions = definition.GetEmissions();

  ASSERT_EQ(emissions.size(), 3U);
  EXPECT_EQ(emissions[0U].GetParticleType(), GGEMSParticleType::Positron);
  EXPECT_EQ(emissions[1U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(emissions[2U].GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_EQ(emissions[0U].GetYieldPerDecay(), 0.9686L);
  EXPECT_EQ(emissions[1U].GetYieldPerDecay(), 0.00229L);
  EXPECT_EQ(emissions[2U].GetYieldPerDecay(), 0.00020L);

  constexpr long double k_expected_total_yield{0.97109L};
  long double const tolerance = std::numeric_limits<long double>::epsilon() *
                                k_expected_total_yield * 8.0L;
  EXPECT_LE(
      std::abs(definition.GetTotalYieldPerDecay() - k_expected_total_yield),
      tolerance);
}

// =============================================================================
// =============================================================================

TEST(GGEMSF18Test, PreservesExactEnergyDistributionsAndBetaDiagnostics) {
  GGEMSRadionuclideDefinition const definition = BuildF18Radionuclide();
  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 3U);

  auto const &positron_energy = emissions[0U].GetEnergyDistribution();

  EXPECT_EQ(positron_energy.GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  EXPECT_EQ(positron_energy.GetTableCount(), 1268U);
  EXPECT_EQ(positron_energy.GetRegularBinWidthMilliElectronVolt(), 499'920ULL);

  auto const centers = positron_energy.GetEnergyValuesMilliElectronVolt();
  auto const tickets = positron_energy.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(centers.size(), 1268U);
  ASSERT_EQ(tickets.size(), centers.size());
  EXPECT_EQ(centers.front() - 249'960ULL, 1'440ULL);
  EXPECT_EQ(centers.back() + 249'960ULL, 633'900'000ULL);

  std::uint64_t previous_ticket{0ULL};
  for (std::uint64_t const upper : tickets) {
    EXPECT_GT(upper, previous_ticket);
    previous_ticket = upper;
  }
  EXPECT_EQ(previous_ticket, k_energy_ticket_space_size);
  auto const &electron_energy = emissions[1U].GetEnergyDistribution();
  EXPECT_EQ(electron_energy.GetType(), GGEMSEnergyDistributionType::Mono);
  EXPECT_EQ(electron_energy.GetMonoEnergyMilliElectronVolt(), 14'300ULL);

  auto const &gamma_energy = emissions[2U].GetEnergyDistribution();
  EXPECT_EQ(gamma_energy.GetType(), GGEMSEnergyDistributionType::Mono);
  EXPECT_EQ(gamma_energy.GetMonoEnergyMilliElectronVolt(), 525'000ULL);

  auto const beta_result = BuildF18PositronSpectrum();
  auto const &diagnostics = beta_result.diagnostics;
  EXPECT_EQ(diagnostics.model, GGEMSBetaSpectrumModel::AllowedPointCoulomb);
  EXPECT_EQ(diagnostics.bin_count, 1268U);
  EXPECT_EQ(diagnostics.bin_width_milli_eV, 499'920ULL);
  EXPECT_EQ(diagnostics.upper_edge_milli_eV, 633'900'000ULL);
  EXPECT_GT(diagnostics.minimum_assigned_ticket_count, 0ULL);

  auto const helper_centers =
      beta_result.distribution.GetEnergyValuesMilliElectronVolt();
  auto const helper_tickets =
      beta_result.distribution.GetCumulativeTicketUpperBounds();
  EXPECT_TRUE(std::ranges::equal(helper_centers, centers));
  EXPECT_TRUE(std::ranges::equal(helper_tickets, tickets));
}

// =============================================================================
// =============================================================================

TEST(GGEMSF18Test, OmitsNonTransportSignaturesAndPlaceholders) {
  GGEMSRadionuclideDefinition const definition = BuildF18Radionuclide();
  auto const emissions = definition.GetEmissions();
  ASSERT_EQ(emissions.size(), 3U);

  std::size_t oxygen_x_ray_count{0U};

  for (GGEMSRadionuclideEmission const &emission : emissions) {
    EXPECT_NE(emission.GetParticleType(), GGEMSParticleType::Aionino);

    auto const &energy = emission.GetEnergyDistribution();

    if (energy.GetType() != GGEMSEnergyDistributionType::Mono) {
      EXPECT_NE(emission.GetParticleType(), GGEMSParticleType::Electron);
      continue;
    }

    std::uint64_t const mono_energy = energy.GetMonoEnergyMilliElectronVolt();
    EXPECT_FALSE(emission.GetParticleType() == GGEMSParticleType::Gamma &&
                 mono_energy == 511'000'000ULL);
    EXPECT_FALSE(emission.GetParticleType() == GGEMSParticleType::Electron &&
                 mono_energy >= 456'000ULL && mono_energy <= 502'000ULL);

    if (emission.GetParticleType() == GGEMSParticleType::Gamma &&
        mono_energy == 525'000ULL) {
      ++oxygen_x_ray_count;
    }
  }
  EXPECT_EQ(oxygen_x_ray_count, 1U);
}

// =============================================================================
// =============================================================================

TEST(GGEMSF18Test, LibraryRetainsStableSimplifiedDefinition) {
  GGEMSRadionuclideLibrary library;
  auto const registered = library.Add(BuildF18Radionuclide());
  ASSERT_NE(registered, nullptr);
  auto const *emissions_address = registered->GetEmissions().data();

  EXPECT_EQ(library.Find("F-18"), registered);
  EXPECT_EQ(library.Find(" f18 "), registered);
  EXPECT_EQ(library.Find("18f"), registered);
  EXPECT_EQ(library.Find("fluorine-18"), registered);
  EXPECT_EQ(library.Find("F18BB"), nullptr);

  for (std::size_t index = 0U; index < 32U; ++index) {
    static_cast<void>(library.Add(MakeSyntheticDefinition(index)));
  }

  auto const after_growth = library.Find("F-18");
  ASSERT_EQ(after_growth, registered);
  EXPECT_EQ(after_growth->GetEmissions().data(), emissions_address);

  std::size_t const count_before_rejection = library.GetCount();
  EXPECT_THROW((void)library.Add(BuildF18Radionuclide()),
               ggems::core::GGEMSExceptionBase);
  EXPECT_EQ(library.GetCount(), count_before_rejection);
  EXPECT_EQ(library.Find("F-18"), registered);
  EXPECT_EQ(library.Find("F-18")->GetEmissions().data(), emissions_address);
}

} // namespace
