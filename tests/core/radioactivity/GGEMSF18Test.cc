#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/radioactivity/GGEMSBetaSpectrumBuilder.hh"
#include "GGEMS/core/radioactivity/GGEMSBetaTransition.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideDefinition.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideEmission.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideLibrary.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideProvenance.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideScientificMetadata.hh"
#include "GGEMS/core/radioactivity/builtins/GGEMSBuiltInRadionuclides.hh"
#include "GGEMS/core/sources/GGEMSEnergyDistribution.hh"
#include "GGEMS/core/sources/GGEMSSourceTypes.hh"

namespace {

// =============================================================================
// =============================================================================

using ggems::core::particles::GGEMSParticleType;
using ggems::core::radioactivity::GGEMSBetaSign;
using ggems::core::radioactivity::GGEMSBetaSpectrumModel;
using ggems::core::radioactivity::GGEMSBetaTransitionClass;
using ggems::core::radioactivity::GGEMSEvaluatedEmissionDisposition;
using ggems::core::radioactivity::GGEMSEvaluatedEmissionMetadata;
using ggems::core::radioactivity::GGEMSEvaluatedQuantity;
using ggems::core::radioactivity::GGEMSEvaluatedValueQualifier;
using ggems::core::radioactivity::GGEMSRadionuclideCompleteness;
using ggems::core::radioactivity::GGEMSRadionuclideDefinition;
using ggems::core::radioactivity::GGEMSRadionuclideEmission;
using ggems::core::radioactivity::GGEMSRadionuclideLibrary;
using ggems::core::radioactivity::GGEMSRadionuclideModelKind;
using ggems::core::radioactivity::GGEMSRadionuclideProvenance;
using ggems::core::radioactivity::GGEMSRadionuclideScientificMetadata;
using ggems::core::radioactivity::builtins::BuildF18Radionuclide;
using ggems::core::sources::GGEMSEnergyDistribution;
using ggems::core::sources::GGEMSEnergyDistributionType;
using ggems::core::sources::k_energy_ticket_space_size;

// =============================================================================
// =============================================================================

[[nodiscard]] auto FindQuantity(GGEMSRadionuclideScientificMetadata const &data,
                                std::string_view label)
    -> GGEMSEvaluatedQuantity const * {
  for (GGEMSEvaluatedQuantity const &quantity : data.GetEvaluatedQuantities()) {
    if (quantity.GetLabel() == label) {
      return &quantity;
    }
  }
  return nullptr;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto FindEmission(GGEMSRadionuclideScientificMetadata const &data,
                                std::string_view label)
    -> GGEMSEvaluatedEmissionMetadata const * {
  for (GGEMSEvaluatedEmissionMetadata const &emission :
       data.GetEvaluatedEmissions()) {
    if (emission.GetLabel() == label) {
      return &emission;
    }
  }
  return nullptr;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeSyntheticDefinition(std::size_t index)
    -> GGEMSRadionuclideDefinition {
  std::vector<GGEMSRadionuclideEmission> emissions;
  emissions.emplace_back(GGEMSParticleType::Gamma, 1.0L,
                         GGEMSEnergyDistribution::BuildMono(1ULL));
  return {"Synthetic-" + std::to_string(index),
          {},
          1.0L,
          GGEMSRadionuclideProvenance{"Synthetic authority",
                                      "Synthetic citation",
                                      "Synthetic evaluation"},
          std::move(emissions)};
}

// =============================================================================
// =============================================================================

TEST(GGEMSF18Test, BuildsExactIdentityEvaluationAndPrintedQuantities) {
  GGEMSRadionuclideDefinition const definition = BuildF18Radionuclide();

  EXPECT_EQ(definition.GetCanonicalName(), "F-18");
  ASSERT_EQ(definition.GetAliases().size(), 3U);
  EXPECT_EQ(definition.GetAliases()[0U], "F18");
  EXPECT_EQ(definition.GetAliases()[1U], "18F");
  EXPECT_EQ(definition.GetAliases()[2U], "Fluorine-18");
  EXPECT_EQ(definition.GetHalfLifeSeconds(), 6'584.04L);

  ASSERT_TRUE(definition.GetScientificMetadata().has_value());
  auto const &metadata = *definition.GetScientificMetadata();
  EXPECT_EQ(metadata.GetModelKind(),
            GGEMSRadionuclideModelKind::PhysicalEvaluated);
  EXPECT_EQ(metadata.GetCompleteness(),
            GGEMSRadionuclideCompleteness::EvaluatedSubset);
  EXPECT_EQ(metadata.GetParent().GetAtomicNumber(), 9U);
  EXPECT_EQ(metadata.GetParent().GetMassNumber(), 18U);
  EXPECT_EQ(metadata.GetParent().GetDisplayName(), "F-18");
  EXPECT_FALSE(metadata.GetParent().IsStable());
  EXPECT_EQ(metadata.GetDaughter().GetAtomicNumber(), 8U);
  EXPECT_EQ(metadata.GetDaughter().GetMassNumber(), 18U);
  EXPECT_EQ(metadata.GetDaughter().GetDisplayName(), "O-18");
  EXPECT_TRUE(metadata.GetDaughter().IsStable());

  auto const &source = metadata.GetEvaluationSource();
  EXPECT_EQ(source.GetProvenance().GetAuthority(), "LNHB / KRI");
  EXPECT_EQ(source.GetProvenance().GetCitation(),
            "F-18 radionuclide evaluation");
  EXPECT_EQ(source.GetProvenance().GetEvaluationDateOrVersion(),
            "22/10/2002-29/08/2014");
  EXPECT_FALSE(source.GetProvenance().GetReferenceIdentifier().has_value());
  ASSERT_EQ(source.GetEvaluators().size(), 3U);
  EXPECT_EQ(source.GetEvaluators()[0U], "V. Chisté");
  EXPECT_EQ(source.GetEvaluators()[1U], "M. M. Bé");
  EXPECT_EQ(source.GetEvaluators()[2U], "N. K. Kuzmenko");
  EXPECT_EQ(source.GetSourceDocumentIdentifier(),
            "LNHB/KRI supplied radionuclide compilation");
  EXPECT_EQ(source.GetSourceLocation(), "pages 139-143");
  EXPECT_EQ(source.GetSourceSnapshotIdentifier(),
            "SHA-256:"
            "CFE68D16E23AEAB150B147B954D98F303FE7BF6E2C4FF8C70BAB9F8085BEE841");
  ASSERT_TRUE(source.GetSchemeDate().has_value());
  EXPECT_EQ(*source.GetSchemeDate(), "27/07/2014");

  auto const *half_life = FindQuantity(metadata, "half-life");
  ASSERT_NE(half_life, nullptr);
  EXPECT_EQ(half_life->GetOriginalPrintedValue(), "1.82890(23) h");
  ASSERT_TRUE(half_life->GetCentralValue().has_value());
  ASSERT_TRUE(half_life->GetStandardUncertainty().has_value());
  EXPECT_EQ(*half_life->GetCentralValue(), 6'584.04L);
  EXPECT_EQ(*half_life->GetStandardUncertainty(), 0.828L);

  auto const *endpoint = FindQuantity(metadata, "beta-plus endpoint");
  ASSERT_NE(endpoint, nullptr);
  EXPECT_EQ(endpoint->GetOriginalPrintedValue(), "633.9(5) keV");
  EXPECT_EQ(*endpoint->GetCentralValue(), 633'900'000.0L);
  EXPECT_EQ(*endpoint->GetStandardUncertainty(), 500'000.0L);

  auto const *mean = FindQuantity(metadata, "beta-plus mean energy");
  ASSERT_NE(mean, nullptr);
  ASSERT_TRUE(mean->GetCentralValue().has_value());
  ASSERT_TRUE(mean->GetStandardUncertainty().has_value());
  EXPECT_EQ(mean->GetOriginalPrintedValue(), "249.5(3) keV");
  EXPECT_EQ(*mean->GetCentralValue(), 249'500'000.0L);
  EXPECT_EQ(*mean->GetStandardUncertainty(), 300'000.0L);

  auto const *log_ft = FindQuantity(metadata, "beta-plus log ft");
  ASSERT_NE(log_ft, nullptr);
  ASSERT_TRUE(log_ft->GetCentralValue().has_value());
  EXPECT_EQ(log_ft->GetOriginalPrintedValue(), "3.57");
  EXPECT_EQ(*log_ft->GetCentralValue(), 3.57L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSF18Test, BuildsFourOrderedChannelsAndAttachedBetaDiagnostics) {
  GGEMSRadionuclideDefinition const definition = BuildF18Radionuclide();
  auto const channels = definition.GetEmissions();

  ASSERT_EQ(channels.size(), 4U);
  EXPECT_EQ(channels[0U].GetParticleType(), GGEMSParticleType::Positron);
  EXPECT_EQ(channels[1U].GetParticleType(), GGEMSParticleType::Electron);
  EXPECT_EQ(channels[2U].GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_EQ(channels[3U].GetParticleType(), GGEMSParticleType::Gamma);
  EXPECT_EQ(channels[0U].GetYieldPerDecay(), 0.9686L);
  EXPECT_EQ(channels[1U].GetYieldPerDecay(), 0.00229L);
  EXPECT_EQ(channels[2U].GetYieldPerDecay(), 0.00007L);
  EXPECT_EQ(channels[3U].GetYieldPerDecay(), 0.00013L);

  constexpr long double expected_total{0.97109L};
  long double const tolerance =
      std::numeric_limits<long double>::epsilon() * expected_total * 8.0L;
  EXPECT_LE(std::abs(definition.GetTotalYieldPerDecay() - expected_total),
            tolerance);

  auto const &positron_energy = channels[0U].GetEnergyDistribution();
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

  EXPECT_EQ(channels[1U].GetEnergyDistribution().GetType(),
            GGEMSEnergyDistributionType::Mono);
  EXPECT_EQ(
      channels[1U].GetEnergyDistribution().GetMonoEnergyMilliElectronVolt(),
      14'300ULL);
  EXPECT_EQ(
      channels[2U].GetEnergyDistribution().GetMonoEnergyMilliElectronVolt(),
      525'000ULL);
  EXPECT_EQ(
      channels[3U].GetEnergyDistribution().GetMonoEnergyMilliElectronVolt(),
      525'000ULL);

  ASSERT_TRUE(definition.GetScientificMetadata().has_value());
  auto const &metadata = *definition.GetScientificMetadata();
  ASSERT_EQ(metadata.GetBetaApproximations().size(), 1U);
  auto const &approximation = metadata.GetBetaApproximations()[0U];
  EXPECT_EQ(approximation.GetIncludedChannelIndex(), 0U);
  EXPECT_EQ(approximation.GetTransition().GetSign(), GGEMSBetaSign::Plus);
  EXPECT_EQ(approximation.GetTransition().GetDaughterAtomicNumber(), 8U);
  EXPECT_EQ(approximation.GetTransition().GetDaughterMassNumber(), 18U);
  EXPECT_EQ(
      approximation.GetTransition().GetEndpointKineticEnergyMilliElectronVolt(),
      633'900'000ULL);
  EXPECT_EQ(approximation.GetTransition().GetTransitionClass(),
            GGEMSBetaTransitionClass::Allowed);
  EXPECT_EQ(approximation.GetOptions().model,
            GGEMSBetaSpectrumModel::AllowedPointCoulomb);
  EXPECT_EQ(approximation.GetOptions().grid.target_maximum_bin_width_milli_eV,
            500'000ULL);
  EXPECT_EQ(approximation.GetDiagnostics().bin_count, 1268U);
  EXPECT_EQ(approximation.GetDiagnostics().bin_width_milli_eV, 499'920ULL);
  EXPECT_EQ(approximation.GetDiagnostics().upper_edge_milli_eV, 633'900'000ULL);
  EXPECT_EQ(approximation.GetDiagnostics().model,
            GGEMSBetaSpectrumModel::AllowedPointCoulomb);
  EXPECT_GT(approximation.GetDiagnostics().minimum_assigned_ticket_count, 0ULL);
  EXPECT_NEAR(
      static_cast<double>(
          approximation.GetDiagnostics().continuous_mean_energy_milli_eV /
          1.0e6L),
      250.53206, 0.001);
  EXPECT_NE(approximation.GetDiagnostics().continuous_mean_energy_milli_eV,
            249'500'000.0L);
  EXPECT_EQ(approximation.GetReferenceCitation(),
            "Fermi point-Coulomb beta-spectrum treatment");
  EXPECT_EQ(
      approximation.GetConstantsReferenceCitation(),
      "CODATA 2022 recommended values of the fundamental physical constants");
  EXPECT_EQ(
      approximation.GetNonExactStatement(),
      "GGEMS AllowedPointCoulomb approximation; not an exact LNHB, BetaShape, "
      "or Geant4 spectrum.");
}

// =============================================================================
// =============================================================================

TEST(GGEMSF18Test, PreservesIncludedDeferredExcludedAndNoDirectRecords) {
  GGEMSRadionuclideDefinition const definition = BuildF18Radionuclide();
  ASSERT_TRUE(definition.GetScientificMetadata().has_value());
  auto const &metadata = *definition.GetScientificMetadata();
  ASSERT_EQ(metadata.GetEvaluatedEmissions().size(), 7U);

  auto const *positron = FindEmission(metadata, "positron beta continuum");
  ASSERT_NE(positron, nullptr);
  EXPECT_EQ(positron->GetDisposition(),
            GGEMSEvaluatedEmissionDisposition::Included);
  EXPECT_EQ(positron->GetIncludedChannelIndex(), 0U);
  EXPECT_EQ(positron->GetYield().GetCentralValue(), 0.9686L);
  EXPECT_EQ(positron->GetYield().GetStandardUncertainty(), 0.0019L);

  auto const *auger_l = FindEmission(metadata, "O Auger-L electron");
  ASSERT_NE(auger_l, nullptr);
  EXPECT_EQ(auger_l->GetIncludedChannelIndex(), 1U);
  ASSERT_TRUE(auger_l->GetEnergy().has_value());
  EXPECT_EQ(auger_l->GetEnergy()->GetCentralMilliElectronVolt(), 14'300ULL);
  EXPECT_EQ(auger_l->GetYield().GetCentralValue(), 0.00229L);
  EXPECT_EQ(auger_l->GetYield().GetStandardUncertainty(), 0.00021L);

  auto const *k_alpha_2 = FindEmission(metadata, "O Kα2 X ray");
  auto const *k_alpha_1 = FindEmission(metadata, "O Kα1 X ray");
  ASSERT_NE(k_alpha_2, nullptr);
  ASSERT_NE(k_alpha_1, nullptr);
  ASSERT_TRUE(k_alpha_2->GetEnergy().has_value());
  ASSERT_TRUE(k_alpha_1->GetEnergy().has_value());
  EXPECT_NE(k_alpha_2, k_alpha_1);
  EXPECT_EQ(k_alpha_2->GetIncludedChannelIndex(), 2U);
  EXPECT_EQ(k_alpha_1->GetIncludedChannelIndex(), 3U);
  EXPECT_EQ(k_alpha_2->GetEnergy()->GetCentralMilliElectronVolt(), 525'000ULL);
  EXPECT_EQ(k_alpha_1->GetEnergy()->GetCentralMilliElectronVolt(), 525'000ULL);
  EXPECT_EQ(k_alpha_2->GetYield().GetCentralValue(), 0.00007L);
  EXPECT_EQ(k_alpha_2->GetYield().GetStandardUncertainty(), 0.00002L);
  EXPECT_EQ(k_alpha_1->GetYield().GetCentralValue(), 0.00013L);
  EXPECT_EQ(k_alpha_1->GetYield().GetStandardUncertainty(), 0.00004L);

  auto const *kll = FindEmission(metadata, "O KLL Auger electron group");
  ASSERT_NE(kll, nullptr);
  EXPECT_EQ(kll->GetDisposition(), GGEMSEvaluatedEmissionDisposition::Deferred);
  EXPECT_FALSE(kll->GetIncludedChannelIndex().has_value());
  ASSERT_TRUE(kll->GetEnergy().has_value());
  EXPECT_EQ(kll->GetEnergy()->GetQualifier(),
            GGEMSEvaluatedValueQualifier::Range);
  EXPECT_EQ(kll->GetEnergy()->GetRangeLowerMilliElectronVolt(), 456'000ULL);
  EXPECT_EQ(kll->GetEnergy()->GetRangeUpperMilliElectronVolt(), 502'000ULL);
  EXPECT_EQ(kll->GetYield().GetCentralValue(), 0.0289L);
  EXPECT_EQ(kll->GetYield().GetStandardUncertainty(), 0.0018L);
  EXPECT_FALSE(kll->GetReason().empty());

  auto const *annihilation =
      FindEmission(metadata, "positron-annihilation photons");
  ASSERT_NE(annihilation, nullptr);
  ASSERT_TRUE(annihilation->GetEnergy().has_value());
  EXPECT_EQ(annihilation->GetDisposition(),
            GGEMSEvaluatedEmissionDisposition::ExcludedGeneratedByTransport);
  EXPECT_FALSE(annihilation->GetIncludedChannelIndex().has_value());
  EXPECT_EQ(annihilation->GetEnergy()->GetCentralMilliElectronVolt(),
            511'000'000ULL);
  EXPECT_EQ(annihilation->GetYield().GetCentralValue(), 1.9372L);
  EXPECT_EQ(annihilation->GetYield().GetStandardUncertainty(), 0.0038L);
  EXPECT_FALSE(annihilation->GetReason().empty());

  auto const *electron_capture = FindEmission(metadata, "electron capture");
  ASSERT_NE(electron_capture, nullptr);
  EXPECT_EQ(electron_capture->GetDisposition(),
            GGEMSEvaluatedEmissionDisposition::NoDirectTransportedParticle);
  EXPECT_FALSE(electron_capture->GetParticleType().has_value());
  EXPECT_FALSE(electron_capture->GetEnergy().has_value());
  EXPECT_FALSE(electron_capture->GetIncludedChannelIndex().has_value());
  EXPECT_EQ(electron_capture->GetYield().GetCentralValue(), 0.0314L);
  EXPECT_EQ(electron_capture->GetYield().GetStandardUncertainty(), 0.0019L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSF18Test,
     PhysicalChannelsExcludeKllAnnihilationAndCapturePlaceholders) {
  GGEMSRadionuclideDefinition const definition = BuildF18Radionuclide();
  auto const channels = definition.GetEmissions();
  ASSERT_EQ(channels.size(), 4U);

  for (GGEMSRadionuclideEmission const &channel : channels) {
    auto const &energy = channel.GetEnergyDistribution();
    EXPECT_FALSE(channel.GetParticleType() == GGEMSParticleType::Gamma &&
                 energy.GetType() == GGEMSEnergyDistributionType::Mono &&
                 energy.GetMonoEnergyMilliElectronVolt() == 511'000'000ULL);
    EXPECT_FALSE(channel.GetParticleType() == GGEMSParticleType::Electron &&
                 energy.GetType() ==
                     GGEMSEnergyDistributionType::RegularSpectrum);
    EXPECT_NE(channel.GetParticleType(), GGEMSParticleType::Aionino);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSF18Test, LibraryRetainsStableImmutableMetadataAndStrongAddGuarantee) {
  GGEMSRadionuclideLibrary library;
  auto const registered = library.Add(BuildF18Radionuclide());
  ASSERT_NE(registered, nullptr);
  ASSERT_TRUE(registered->GetScientificMetadata().has_value());
  auto const *metadata_address = &*registered->GetScientificMetadata();

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
  ASSERT_TRUE(after_growth->GetScientificMetadata().has_value());
  EXPECT_EQ(&*after_growth->GetScientificMetadata(), metadata_address);

  std::size_t const count_before_rejection = library.GetCount();
  EXPECT_THROW((void)library.Add(BuildF18Radionuclide()),
               ggems::core::GGEMSExceptionBase);
  EXPECT_EQ(library.GetCount(), count_before_rejection);
  EXPECT_EQ(library.Find("F-18"), registered);
  EXPECT_EQ(&*library.Find("F-18")->GetScientificMetadata(), metadata_address);
}

} // namespace
