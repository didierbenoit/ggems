#include <array>
#include <limits>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/radioactivity/GGEMSRadionuclideEmission.hh"
#include "GGEMS/sources/GGEMSEnergyDistribution.hh"
#include "GGEMS/sources/GGEMSSourceTypes.hh"

namespace {

// =============================================================================
// =============================================================================

using ggems::core::particles::GGEMSParticleType;
using ggems::core::radioactivity::GGEMSRadionuclideEmission;
using ggems::core::sources::GGEMSEnergyDistribution;
using ggems::core::sources::GGEMSEnergyDistributionType;

// =============================================================================
// =============================================================================

[[nodiscard]] auto BuildDiscreteLines() -> GGEMSEnergyDistribution {
  constexpr std::array<double, 3U> energies{10.0, 20.0, 30.0};
  constexpr std::array<double, 3U> weights{1.0, 2.0, 1.0};
  return GGEMSEnergyDistribution::BuildDiscreteLines(energies, weights, "keV");
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto BuildRegularSpectrum() -> GGEMSEnergyDistribution {
  constexpr std::array<double, 3U> centers{20.0, 22.0, 24.0};
  constexpr std::array<double, 3U> weights{1.0, 2.0, 1.0};
  return GGEMSEnergyDistribution::BuildRegularSpectrum(centers, weights, "keV");
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideEmissionTest, AcceptsEveryCurrentPhysicalParticleType) {
  constexpr std::array<GGEMSParticleType, 6U> particle_types{
      GGEMSParticleType::Gamma,    GGEMSParticleType::Electron,
      GGEMSParticleType::Positron, GGEMSParticleType::Proton,
      GGEMSParticleType::Neutron,  GGEMSParticleType::Alpha};

  for (GGEMSParticleType particle_type : particle_types) {
    SCOPED_TRACE(static_cast<unsigned int>(particle_type));
    GGEMSRadionuclideEmission const emission{
        particle_type, 1.0L, GGEMSEnergyDistribution::BuildMono(1'000ULL)};
    EXPECT_EQ(emission.GetParticleType(), particle_type);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideEmissionTest, RejectsNonPhysicalParticleTypes) {
  for (GGEMSParticleType particle_type :
       {GGEMSParticleType::Unknown, GGEMSParticleType::Aionino,
        static_cast<GGEMSParticleType>(999U)}) {
    SCOPED_TRACE(static_cast<unsigned int>(particle_type));
    EXPECT_THROW(
        ((void)GGEMSRadionuclideEmission{
            particle_type, 1.0L, GGEMSEnergyDistribution::BuildMono(1'000ULL)}),
        ggems::core::GGEMSExceptionBase);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideEmissionTest, ValidatesYieldWithoutProbabilityCeiling) {
  for (long double yield :
       {0.0L, -1.0L, std::numeric_limits<long double>::quiet_NaN(),
        std::numeric_limits<long double>::infinity(),
        -std::numeric_limits<long double>::infinity()}) {
    SCOPED_TRACE(static_cast<double>(yield));
    EXPECT_THROW(((void)GGEMSRadionuclideEmission{
                     GGEMSParticleType::Gamma, yield,
                     GGEMSEnergyDistribution::BuildMono(1'000ULL)}),
                 ggems::core::GGEMSExceptionBase);
  }

  GGEMSRadionuclideEmission const emission{
      GGEMSParticleType::Electron, 3.5L,
      GGEMSEnergyDistribution::BuildMono(1'000ULL)};
  EXPECT_EQ(emission.GetYieldPerDecay(), 3.5L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideEmissionTest, OwnsEverySupportedEnergyDistribution) {
  GGEMSRadionuclideEmission const mono{
      GGEMSParticleType::Gamma, 1.0L,
      GGEMSEnergyDistribution::BuildMono(10'000'001'000ULL)};
  GGEMSRadionuclideEmission const lines{GGEMSParticleType::Electron, 0.5L,
                                        BuildDiscreteLines()};
  GGEMSRadionuclideEmission const spectrum{GGEMSParticleType::Positron, 0.25L,
                                           BuildRegularSpectrum()};

  EXPECT_EQ(mono.GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::Mono);
  EXPECT_EQ(mono.GetEnergyDistribution().GetMonoEnergyMicroElectronVolt(),
            10'000'001'000ULL);
  EXPECT_EQ(lines.GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::DiscreteLines);
  EXPECT_EQ(lines.GetEnergyDistribution().GetTableCount(), 3U);
  EXPECT_EQ(spectrum.GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  EXPECT_EQ(spectrum.GetEnergyDistribution().GetTableCount(), 3U);
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideEmissionTest, OwnsCallerProvidedEnergyData) {
  std::vector<double> energies{10.0, 20.0, 30.0};
  std::vector<double> weights{1.0, 2.0, 1.0};

  GGEMSRadionuclideEmission const emission{
      GGEMSParticleType::Alpha, 1.25L,
      GGEMSEnergyDistribution::BuildDiscreteLines(energies, weights, "keV")};

  energies.assign(3U, 999.0);
  weights.clear();

  auto const stored_energies =
      emission.GetEnergyDistribution().GetEnergyValuesMicroElectronVolt();
  auto const stored_weights =
      emission.GetEnergyDistribution().GetRelativeWeights();

  ASSERT_EQ(stored_energies.size(), 3U);
  EXPECT_EQ(stored_energies[0U], 10'000'000'000ULL);
  EXPECT_EQ(stored_energies[1U], 20'000'000'000ULL);
  EXPECT_EQ(stored_energies[2U], 30'000'000'000ULL);
  ASSERT_EQ(stored_weights.size(), 3U);
  EXPECT_EQ(stored_weights[0U], 1.0);
  EXPECT_EQ(stored_weights[1U], 2.0);
  EXPECT_EQ(stored_weights[2U], 1.0);
}

} // namespace
