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
#include "GGEMS/core/radioactivity/GGEMSBetaSpectrumBuilder.hh"
#include "GGEMS/core/radioactivity/GGEMSBetaTransition.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideDefinition.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideEmission.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideProvenance.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideScientificMetadata.hh"
#include "GGEMS/core/sources/GGEMSEnergyDistribution.hh"

namespace {

using ggems::core::GGEMSExceptionBase;
using ggems::core::particles::GGEMSParticleType;
using ggems::core::radioactivity::GGEMSBetaApproximationMetadata;
using ggems::core::radioactivity::GGEMSBetaSign;
using ggems::core::radioactivity::GGEMSBetaSpectrumBuildOptions;
using ggems::core::radioactivity::GGEMSBetaSpectrumDiagnostics;
using ggems::core::radioactivity::GGEMSBetaSpectrumModel;
using ggems::core::radioactivity::GGEMSBetaTransition;
using ggems::core::radioactivity::GGEMSBetaTransitionClass;
using ggems::core::radioactivity::GGEMSEvaluatedEmissionDisposition;
using ggems::core::radioactivity::GGEMSEvaluatedEmissionMetadata;
using ggems::core::radioactivity::GGEMSEvaluatedEnergy;
using ggems::core::radioactivity::GGEMSEvaluatedQuantity;
using ggems::core::radioactivity::GGEMSEvaluatedQuantityUnit;
using ggems::core::radioactivity::GGEMSEvaluatedValueQualifier;
using ggems::core::radioactivity::GGEMSNuclideIdentity;
using ggems::core::radioactivity::GGEMSRadionuclideCompleteness;
using ggems::core::radioactivity::GGEMSRadionuclideDefinition;
using ggems::core::radioactivity::GGEMSRadionuclideEmission;
using ggems::core::radioactivity::GGEMSRadionuclideEvaluationSource;
using ggems::core::radioactivity::GGEMSRadionuclideModelKind;
using ggems::core::radioactivity::GGEMSRadionuclideProvenance;
using ggems::core::radioactivity::GGEMSRadionuclideScientificMetadata;
using ggems::core::sources::GGEMSEnergyDistribution;

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeProvenance() -> GGEMSRadionuclideProvenance {
  return {"LNHB / KRI", "Synthetic evaluation citation",
          "22/10/2002-29/08/2014", std::string{"synthetic-reference"}};
}

// -----------------------------------------------------------------------------

[[nodiscard]] auto MakeEvaluationSource() -> GGEMSRadionuclideEvaluationSource {
  return {MakeProvenance(),
          {"V. Chisté", "M. M. Bé", "N. K. Kuzmenko"},
          "Synthetic radionuclide evaluation",
          "pages 139-143",
          "sha256:synthetic-snapshot",
          std::string{"27/07/2014"}};
}

// -----------------------------------------------------------------------------

[[nodiscard]] auto MakeYield(long double value = 1.0L)
    -> GGEMSEvaluatedQuantity {
  return {"global yield",
          "1.0 per parent decay",
          GGEMSEvaluatedQuantityUnit::PerParentDecay,
          GGEMSEvaluatedValueQualifier::CentralValue,
          value,
          0.01L};
}

// -----------------------------------------------------------------------------

[[nodiscard]] auto MakeCentralEnergy(std::uint64_t energy_milli_eV = 525'000ULL)
    -> GGEMSEvaluatedEnergy {
  return {"emission energy", "0.525 keV",
          GGEMSEvaluatedValueQualifier::CentralValue, energy_milli_eV,
          1'000ULL};
}

// -----------------------------------------------------------------------------

[[nodiscard]] auto MakeIncludedEmissionMetadata(std::size_t channel_index)
    -> GGEMSEvaluatedEmissionMetadata {
  return {"included gamma",
          GGEMSParticleType::Gamma,
          MakeYield(),
          MakeCentralEnergy(),
          GGEMSEvaluatedEmissionDisposition::Included,
          channel_index};
}

// -----------------------------------------------------------------------------

[[nodiscard]] auto
MakeRuntimeEmission(std::uint64_t energy_milli_eV = 525'000ULL)
    -> GGEMSRadionuclideEmission {
  return {GGEMSParticleType::Gamma, 1.0L,
          GGEMSEnergyDistribution::BuildMono(energy_milli_eV)};
}

// -----------------------------------------------------------------------------

[[nodiscard]] auto MakeRuntimeEmissions(std::size_t count = 1U)
    -> std::vector<GGEMSRadionuclideEmission> {
  std::vector<GGEMSRadionuclideEmission> emissions;
  emissions.reserve(count);

  for (std::size_t index = 0U; index < count; ++index) {
    emissions.push_back(
        MakeRuntimeEmission(525'000ULL + static_cast<std::uint64_t>(index)));
  }

  return emissions;
}

// -----------------------------------------------------------------------------

[[nodiscard]] auto MakeBetaApproximation(std::size_t channel_index)
    -> GGEMSBetaApproximationMetadata {
  GGEMSBetaSpectrumBuildOptions const options{
      .model = GGEMSBetaSpectrumModel::AllowedPointCoulomb,
      .grid = {.target_maximum_bin_width_milli_eV = 2ULL}};
  GGEMSBetaSpectrumDiagnostics const diagnostics{
      .model = GGEMSBetaSpectrumModel::AllowedPointCoulomb,
      .lower_edge_milli_eV = 2ULL,
      .upper_edge_milli_eV = 10ULL,
      .bin_width_milli_eV = 2ULL,
      .bin_count = 4U,
      .full_unnormalized_integral = 1.0L,
      .represented_unnormalized_integral = 0.99L,
      .excluded_probability = 0.01L,
      .continuous_mean_energy_milli_eV = 5.0L,
      .represented_mean_energy_milli_eV = 5.0L,
      .normalization_residual = 0.0L,
      .minimum_positive_bin_probability = 0.1L,
      .minimum_assigned_ticket_count = 1ULL};

  return {channel_index,
          GGEMSBetaTransition{GGEMSBetaSign::Plus, 1U, 1U, 10ULL,
                              GGEMSBetaTransitionClass::Allowed},
          options,
          diagnostics,
          "Fermi point-Coulomb reference",
          "CODATA 2022 constants reference",
          "This approximation is not an exact evaluated spectrum."};
}

// -----------------------------------------------------------------------------

[[nodiscard]] auto MakeScientificMetadata(
    std::vector<GGEMSEvaluatedEmissionMetadata> evaluated_emissions,
    std::vector<GGEMSBetaApproximationMetadata> beta_approximations = {})
    -> GGEMSRadionuclideScientificMetadata {
  std::vector<GGEMSEvaluatedQuantity> quantities;
  quantities.emplace_back(
      "half-life", "1.0(1) s", GGEMSEvaluatedQuantityUnit::Second,
      GGEMSEvaluatedValueQualifier::CentralValue, 1.0L, 0.1L);

  return {GGEMSRadionuclideModelKind::PhysicalEvaluated,
          GGEMSRadionuclideCompleteness::EvaluatedSubset,
          GGEMSNuclideIdentity{9U, 18U, "F-18", false},
          GGEMSNuclideIdentity{8U, 18U, "stable O-18", true},
          MakeEvaluationSource(),
          std::move(quantities),
          std::move(evaluated_emissions),
          std::move(beta_approximations)};
}

// -----------------------------------------------------------------------------

[[nodiscard]] auto MakeValidScientificMetadata()
    -> GGEMSRadionuclideScientificMetadata {
  std::vector<GGEMSEvaluatedEmissionMetadata> evaluated_emissions;
  evaluated_emissions.push_back(MakeIncludedEmissionMetadata(0U));
  return MakeScientificMetadata(std::move(evaluated_emissions));
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideScientificMetadataTest,
     OwnsCallerTextVectorsAndPreservesUtf8) {
  std::vector<std::string> evaluators{"V. Chisté", "M. M. Bé",
                                      "N. K. Kuzmenko"};
  std::string document{"Évaluation LNHB F-18"};
  std::string location{"pages 139-143"};
  std::string snapshot{"sha256:owned-snapshot"};
  std::optional<std::string> scheme_date{"27/07/2014"};

  GGEMSRadionuclideEvaluationSource const source{
      MakeProvenance(), evaluators, document, location, snapshot, scheme_date};

  evaluators.clear();
  document.clear();
  location.clear();
  snapshot.clear();
  scheme_date->clear();

  ASSERT_EQ(source.GetEvaluators().size(), 3U);
  EXPECT_EQ(source.GetEvaluators()[0U], "V. Chisté");
  EXPECT_EQ(source.GetEvaluators()[1U], "M. M. Bé");
  EXPECT_EQ(source.GetSourceDocumentIdentifier(), "Évaluation LNHB F-18");
  EXPECT_EQ(source.GetSourceLocation(), "pages 139-143");
  EXPECT_EQ(source.GetSourceSnapshotIdentifier(), "sha256:owned-snapshot");
  ASSERT_TRUE(source.GetSchemeDate().has_value());
  EXPECT_EQ(*source.GetSchemeDate(), "27/07/2014");

  std::vector<GGEMSEvaluatedQuantity> quantities;
  quantities.emplace_back(
      "half-life", "1.82890(23) h", GGEMSEvaluatedQuantityUnit::Second,
      GGEMSEvaluatedValueQualifier::CentralValue, 6584.04L, 0.828L);
  std::vector<GGEMSEvaluatedEmissionMetadata> evaluated_emissions;
  evaluated_emissions.push_back(MakeIncludedEmissionMetadata(0U));

  GGEMSRadionuclideScientificMetadata const metadata{
      GGEMSRadionuclideModelKind::PhysicalEvaluated,
      GGEMSRadionuclideCompleteness::EvaluatedSubset,
      GGEMSNuclideIdentity{9U, 18U, "F-18", false},
      GGEMSNuclideIdentity{8U, 18U, "stable O-18", true},
      source,
      quantities,
      evaluated_emissions,
      {}};

  quantities.clear();
  evaluated_emissions.clear();

  ASSERT_EQ(metadata.GetEvaluatedQuantities().size(), 1U);
  ASSERT_EQ(metadata.GetEvaluatedEmissions().size(), 1U);
  EXPECT_EQ(metadata.GetEvaluatedQuantities()[0U].GetOriginalPrintedValue(),
            "1.82890(23) h");
  EXPECT_EQ(metadata.GetEvaluatedEmissions()[0U].GetLabel(), "included gamma");
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideScientificMetadataTest,
     RejectsInvalidNuclideIdentityAndRequiredText) {
  EXPECT_THROW((void)GGEMSNuclideIdentity(0U, 18U, "F-18", false),
               GGEMSExceptionBase);
  EXPECT_THROW((void)GGEMSNuclideIdentity(9U, 0U, "F-18", false),
               GGEMSExceptionBase);
  EXPECT_THROW((void)GGEMSNuclideIdentity(19U, 18U, "F-18", false),
               GGEMSExceptionBase);
  EXPECT_THROW((void)GGEMSNuclideIdentity(9U, 18U, " \t\r\n", false),
               GGEMSExceptionBase);

  EXPECT_THROW(((void)GGEMSEvaluatedQuantity{
                   "", "1.0", GGEMSEvaluatedQuantityUnit::Dimensionless,
                   GGEMSEvaluatedValueQualifier::CentralValue, 1.0L}),
               GGEMSExceptionBase);
  EXPECT_THROW(((void)GGEMSEvaluatedQuantity{
                   "quantity", " \t", GGEMSEvaluatedQuantityUnit::Dimensionless,
                   GGEMSEvaluatedValueQualifier::CentralValue, 1.0L}),
               GGEMSExceptionBase);
  EXPECT_THROW(
      ((void)GGEMSEvaluatedEnergy{
          " ", "1 keV", GGEMSEvaluatedValueQualifier::CentralValue, 1ULL}),
      GGEMSExceptionBase);

  EXPECT_THROW(
      ((void)GGEMSRadionuclideEvaluationSource{
          MakeProvenance(), {}, "document", "pages", "snapshot", std::nullopt}),
      GGEMSExceptionBase);
  EXPECT_THROW(((void)GGEMSRadionuclideEvaluationSource{MakeProvenance(),
                                                        {" \t"},
                                                        "document",
                                                        "pages",
                                                        "snapshot",
                                                        std::nullopt}),
               GGEMSExceptionBase);
  EXPECT_THROW(((void)GGEMSRadionuclideEvaluationSource{MakeProvenance(),
                                                        {"Evaluator"},
                                                        " ",
                                                        "pages",
                                                        "snapshot",
                                                        std::nullopt}),
               GGEMSExceptionBase);
  EXPECT_THROW(((void)GGEMSRadionuclideEvaluationSource{MakeProvenance(),
                                                        {"Evaluator"},
                                                        "document",
                                                        "",
                                                        "snapshot",
                                                        std::nullopt}),
               GGEMSExceptionBase);
  EXPECT_THROW(((void)GGEMSRadionuclideEvaluationSource{MakeProvenance(),
                                                        {"Evaluator"},
                                                        "document",
                                                        "pages",
                                                        "\n",
                                                        std::nullopt}),
               GGEMSExceptionBase);
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideScientificMetadataTest, RejectsUnknownEnumValues) {
  auto const unknown_qualifier =
      static_cast<GGEMSEvaluatedValueQualifier>(255U);
  auto const unknown_unit = static_cast<GGEMSEvaluatedQuantityUnit>(255U);
  auto const unknown_disposition =
      static_cast<GGEMSEvaluatedEmissionDisposition>(255U);

  EXPECT_THROW(((void)GGEMSEvaluatedQuantity{
                   "quantity", "1.0", GGEMSEvaluatedQuantityUnit::Dimensionless,
                   unknown_qualifier, 1.0L}),
               GGEMSExceptionBase);
  EXPECT_THROW(((void)GGEMSEvaluatedQuantity{
                   "quantity", "1.0", unknown_unit,
                   GGEMSEvaluatedValueQualifier::CentralValue, 1.0L}),
               GGEMSExceptionBase);
  EXPECT_THROW(
      ((void)GGEMSEvaluatedEnergy{"energy", "1 meV", unknown_qualifier, 1ULL}),
      GGEMSExceptionBase);
  EXPECT_THROW(
      ((void)GGEMSEvaluatedEmissionMetadata{
          "emission", GGEMSParticleType::Gamma, MakeYield(),
          MakeCentralEnergy(), unknown_disposition, std::nullopt, "reason"}),
      GGEMSExceptionBase);

  std::vector<GGEMSEvaluatedEmissionMetadata> evaluated_emissions;
  evaluated_emissions.push_back(MakeIncludedEmissionMetadata(0U));

  EXPECT_THROW(((void)GGEMSRadionuclideScientificMetadata{
                   static_cast<GGEMSRadionuclideModelKind>(255U),
                   GGEMSRadionuclideCompleteness::EvaluatedSubset,
                   GGEMSNuclideIdentity{9U, 18U, "F-18", false},
                   GGEMSNuclideIdentity{8U, 18U, "O-18", true},
                   MakeEvaluationSource(),
                   {},
                   evaluated_emissions,
                   {}}),
               GGEMSExceptionBase);
  EXPECT_THROW(((void)GGEMSRadionuclideScientificMetadata{
                   GGEMSRadionuclideModelKind::PhysicalEvaluated,
                   static_cast<GGEMSRadionuclideCompleteness>(255U),
                   GGEMSNuclideIdentity{9U, 18U, "F-18", false},
                   GGEMSNuclideIdentity{8U, 18U, "O-18", true},
                   MakeEvaluationSource(),
                   {},
                   evaluated_emissions,
                   {}}),
               GGEMSExceptionBase);

  GGEMSBetaSpectrumBuildOptions invalid_options;
  invalid_options.model = static_cast<GGEMSBetaSpectrumModel>(255U);
  EXPECT_THROW(((void)GGEMSBetaApproximationMetadata{
                   0U,
                   GGEMSBetaTransition{GGEMSBetaSign::Plus, 1U, 1U, 10ULL,
                                       GGEMSBetaTransitionClass::Allowed},
                   invalid_options, MakeBetaApproximation(0U).GetDiagnostics(),
                   "reference", "constants", "not exact"}),
               GGEMSExceptionBase);
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideScientificMetadataTest,
     PreservesUncertaintiesAndExactEnergyRanges) {
  GGEMSEvaluatedQuantity const half_life{
      "half-life",
      "1.82890(23) h",
      GGEMSEvaluatedQuantityUnit::Second,
      GGEMSEvaluatedValueQualifier::CentralValue,
      6584.04L,
      0.828L};
  ASSERT_TRUE(half_life.GetCentralValue().has_value());
  ASSERT_TRUE(half_life.GetStandardUncertainty().has_value());
  EXPECT_EQ(*half_life.GetCentralValue(), 6584.04L);
  EXPECT_EQ(*half_life.GetStandardUncertainty(), 0.828L);

  GGEMSEvaluatedEnergy const endpoint{
      "beta-plus endpoint", "633.9(5) keV",
      GGEMSEvaluatedValueQualifier::CentralValue, 633'900'000ULL, 500'000ULL};
  ASSERT_TRUE(endpoint.GetCentralMilliElectronVolt().has_value());
  ASSERT_TRUE(endpoint.GetStandardUncertaintyMilliElectronVolt().has_value());
  EXPECT_EQ(*endpoint.GetCentralMilliElectronVolt(), 633'900'000ULL);
  EXPECT_EQ(*endpoint.GetStandardUncertaintyMilliElectronVolt(), 500'000ULL);

  GGEMSEvaluatedEnergy const range{"O KLL Auger electron group",
                                   "0.456-0.502 keV",
                                   GGEMSEvaluatedValueQualifier::Range,
                                   std::nullopt,
                                   std::nullopt,
                                   456'000ULL,
                                   502'000ULL};
  ASSERT_TRUE(range.GetRangeLowerMilliElectronVolt().has_value());
  ASSERT_TRUE(range.GetRangeUpperMilliElectronVolt().has_value());
  EXPECT_EQ(*range.GetRangeLowerMilliElectronVolt(), 456'000ULL);
  EXPECT_EQ(*range.GetRangeUpperMilliElectronVolt(), 502'000ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideScientificMetadataTest,
     RejectsInvalidUncertaintiesAndRanges) {
  EXPECT_THROW(((void)GGEMSEvaluatedQuantity{
                   "quantity", "1.0", GGEMSEvaluatedQuantityUnit::Dimensionless,
                   GGEMSEvaluatedValueQualifier::CentralValue, 1.0L, -0.1L}),
               GGEMSExceptionBase);
  EXPECT_THROW(((void)GGEMSEvaluatedQuantity{
                   "quantity", "1.0", GGEMSEvaluatedQuantityUnit::Dimensionless,
                   GGEMSEvaluatedValueQualifier::CentralValue,
                   std::numeric_limits<long double>::quiet_NaN()}),
               GGEMSExceptionBase);
  EXPECT_THROW(((void)GGEMSEvaluatedQuantity{
                   "range", "1-2", GGEMSEvaluatedQuantityUnit::Dimensionless,
                   GGEMSEvaluatedValueQualifier::Range, std::nullopt,
                   std::nullopt, 2.0L, 1.0L}),
               GGEMSExceptionBase);
  EXPECT_THROW(
      ((void)GGEMSEvaluatedQuantity{
          "range", "1-2", GGEMSEvaluatedQuantityUnit::Dimensionless,
          GGEMSEvaluatedValueQualifier::Range, 1.5L, std::nullopt, 1.0L, 2.0L}),
      GGEMSExceptionBase);
  EXPECT_THROW(((void)GGEMSEvaluatedEnergy{
                   "range", "1-2 meV", GGEMSEvaluatedValueQualifier::Range,
                   std::nullopt, std::nullopt, 2ULL, 1ULL}),
               GGEMSExceptionBase);
  EXPECT_THROW(((void)GGEMSEvaluatedEnergy{"range", "1-2 meV",
                                           GGEMSEvaluatedValueQualifier::Range,
                                           1ULL, std::nullopt, 1ULL, 2ULL}),
               GGEMSExceptionBase);
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideScientificMetadataTest,
     EnforcesDispositionParticleReasonAndChannelContracts) {
  EXPECT_THROW(((void)GGEMSEvaluatedEmissionMetadata{
                   "included", GGEMSParticleType::Gamma, MakeYield(),
                   MakeCentralEnergy(),
                   GGEMSEvaluatedEmissionDisposition::Included, std::nullopt}),
               GGEMSExceptionBase);
  EXPECT_THROW(
      ((void)GGEMSEvaluatedEmissionMetadata{
          "deferred", GGEMSParticleType::Electron, MakeYield(),
          MakeCentralEnergy(), GGEMSEvaluatedEmissionDisposition::Deferred,
          std::nullopt, " \t"}),
      GGEMSExceptionBase);
  EXPECT_THROW(
      ((void)GGEMSEvaluatedEmissionMetadata{
          "excluded", GGEMSParticleType::Gamma, MakeYield(),
          MakeCentralEnergy(),
          GGEMSEvaluatedEmissionDisposition::ExcludedGeneratedByTransport,
          std::nullopt, ""}),
      GGEMSExceptionBase);
  EXPECT_THROW(
      ((void)GGEMSEvaluatedEmissionMetadata{
          "electron capture", std::nullopt, MakeYield(), std::nullopt,
          GGEMSEvaluatedEmissionDisposition::NoDirectTransportedParticle,
          std::nullopt, ""}),
      GGEMSExceptionBase);
  EXPECT_THROW(((void)GGEMSEvaluatedEmissionMetadata{
                   "invalid particle", GGEMSParticleType::Aionino, MakeYield(),
                   MakeCentralEnergy(),
                   GGEMSEvaluatedEmissionDisposition::Included, 0U}),
               GGEMSExceptionBase);
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideScientificMetadataTest,
     DefinitionValidatesAllMetadataChannelLinks) {
  EXPECT_NO_THROW(
      ((void)GGEMSRadionuclideDefinition{"Valid",
                                         {},
                                         1.0L,
                                         MakeValidScientificMetadata(),
                                         MakeRuntimeEmissions()}));

  std::vector<GGEMSEvaluatedEmissionMetadata> out_of_bounds_records;
  out_of_bounds_records.push_back(MakeIncludedEmissionMetadata(1U));
  EXPECT_THROW(((void)GGEMSRadionuclideDefinition{
                   "Out-Of-Bounds",
                   {},
                   1.0L,
                   MakeScientificMetadata(std::move(out_of_bounds_records)),
                   MakeRuntimeEmissions()}),
               GGEMSExceptionBase);

  std::vector<GGEMSEvaluatedEmissionMetadata> duplicate_records;
  duplicate_records.push_back(MakeIncludedEmissionMetadata(0U));
  duplicate_records.push_back(MakeIncludedEmissionMetadata(0U));
  EXPECT_THROW(((void)GGEMSRadionuclideDefinition{
                   "Duplicate",
                   {},
                   1.0L,
                   MakeScientificMetadata(std::move(duplicate_records)),
                   MakeRuntimeEmissions()}),
               GGEMSExceptionBase);

  std::vector<GGEMSEvaluatedEmissionMetadata> missing_record;
  missing_record.push_back(MakeIncludedEmissionMetadata(0U));
  EXPECT_THROW(((void)GGEMSRadionuclideDefinition{
                   "Missing",
                   {},
                   1.0L,
                   MakeScientificMetadata(std::move(missing_record)),
                   MakeRuntimeEmissions(2U)}),
               GGEMSExceptionBase);

  std::vector<GGEMSEvaluatedEmissionMetadata> approximation_records;
  approximation_records.push_back(MakeIncludedEmissionMetadata(0U));
  std::vector<GGEMSBetaApproximationMetadata> approximations;
  approximations.push_back(MakeBetaApproximation(1U));
  EXPECT_THROW(((void)GGEMSRadionuclideDefinition{
                   "Approximation-Out-Of-Bounds",
                   {},
                   1.0L,
                   MakeScientificMetadata(std::move(approximation_records),
                                          std::move(approximations)),
                   MakeRuntimeEmissions()}),
               GGEMSExceptionBase);
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideScientificMetadataTest,
     DefinitionDerivesLegacyProvenanceAndOwnsMetadata) {
  GGEMSRadionuclideScientificMetadata metadata = MakeValidScientificMetadata();
  GGEMSRadionuclideDefinition const definition{
      "F-18", {"F18"}, 6584.04L, metadata, MakeRuntimeEmissions()};

  ASSERT_TRUE(definition.GetScientificMetadata().has_value());
  EXPECT_EQ(definition.GetProvenance().GetAuthority(), "LNHB / KRI");
  EXPECT_EQ(definition.GetProvenance().GetCitation(),
            "Synthetic evaluation citation");
  EXPECT_EQ(definition.GetScientificMetadata()
                ->GetEvaluationSource()
                .GetEvaluators()[1U],
            "M. M. Bé");

  metadata = MakeValidScientificMetadata();
  EXPECT_EQ(definition.GetScientificMetadata()
                ->GetEvaluationSource()
                .GetSourceLocation(),
            "pages 139-143");
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideScientificMetadataTest,
     LegacyConstructorKeepsScientificMetadataAbsent) {
  GGEMSRadionuclideDefinition const definition{"Legacy",
                                               {"Legacy alias"},
                                               1.0L,
                                               MakeProvenance(),
                                               MakeRuntimeEmissions()};

  EXPECT_FALSE(definition.GetScientificMetadata().has_value());
  EXPECT_EQ(definition.GetProvenance().GetAuthority(), "LNHB / KRI");
}

// =============================================================================
// =============================================================================

TEST(GGEMSRadionuclideScientificMetadataTest,
     FailedDefinitionConstructionLeavesCallerDataUnchanged) {
  std::string canonical_name{"Caller-Owned"};
  std::vector<std::string> aliases{"Caller alias"};
  std::vector<GGEMSEvaluatedEmissionMetadata> evaluated_emissions;
  evaluated_emissions.push_back(MakeIncludedEmissionMetadata(1U));
  GGEMSRadionuclideScientificMetadata metadata =
      MakeScientificMetadata(std::move(evaluated_emissions));
  std::vector<GGEMSRadionuclideEmission> runtime_emissions =
      MakeRuntimeEmissions();

  EXPECT_THROW(((void)GGEMSRadionuclideDefinition{canonical_name, aliases, 1.0L,
                                                  metadata, runtime_emissions}),
               GGEMSExceptionBase);

  EXPECT_EQ(canonical_name, "Caller-Owned");
  ASSERT_EQ(aliases.size(), 1U);
  EXPECT_EQ(aliases[0U], "Caller alias");
  ASSERT_EQ(metadata.GetEvaluatedEmissions().size(), 1U);
  ASSERT_TRUE(metadata.GetEvaluatedEmissions()[0U]
                  .GetIncludedChannelIndex()
                  .has_value());
  EXPECT_EQ(*metadata.GetEvaluatedEmissions()[0U].GetIncludedChannelIndex(),
            1U);
  ASSERT_EQ(runtime_emissions.size(), 1U);
  EXPECT_EQ(runtime_emissions[0U].GetYieldPerDecay(), 1.0L);
}

} // namespace
