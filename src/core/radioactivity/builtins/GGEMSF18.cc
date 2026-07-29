#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/radioactivity/GGEMSBetaSpectrumBuilder.hh"
#include "GGEMS/core/radioactivity/GGEMSBetaTransition.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideEmission.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideProvenance.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideScientificMetadata.hh"
#include "GGEMS/core/radioactivity/builtins/GGEMSBuiltInRadionuclides.hh"
#include "GGEMS/core/sources/GGEMSEnergyDistribution.hh"

namespace ggems::core::radioactivity::builtins {
namespace {

// =============================================================================
// =============================================================================

constexpr long double k_half_life_seconds{6'584.04L};
constexpr long double k_positron_yield{0.9686L};
constexpr long double k_auger_l_yield{0.00229L};
constexpr long double k_k_alpha_2_yield{0.00007L};
constexpr long double k_k_alpha_1_yield{0.00013L};
constexpr std::uint64_t k_beta_endpoint_milli_eV{633'900'000ULL};
constexpr std::uint64_t k_beta_target_width_milli_eV{500'000ULL};

// =============================================================================
// =============================================================================

[[nodiscard]] auto
MakeYield(std::string label, std::string original_printed_value,
          long double central_value, long double standard_uncertainty)
    -> GGEMSEvaluatedQuantity {
  return {std::move(label),
          std::move(original_printed_value),
          GGEMSEvaluatedQuantityUnit::PerParentDecay,
          GGEMSEvaluatedValueQualifier::CentralValue,
          central_value,
          standard_uncertainty};
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeEvaluationSource() -> GGEMSRadionuclideEvaluationSource {
  GGEMSRadionuclideProvenance provenance{
      "LNHB / KRI", "F-18 radionuclide evaluation", "22/10/2002-29/08/2014"};

  return {std::move(provenance),
          {"V. Chisté", "M. M. Bé", "N. K. Kuzmenko"},
          "LNHB/KRI supplied radionuclide compilation",
          "pages 139-143",
          "SHA-256:"
          "CFE68D16E23AEAB150B147B954D98F303FE7BF6E2C4FF8C70BAB9F8085BEE841",
          std::string{"27/07/2014"}};
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto BuildEvaluatedQuantities()
    -> std::vector<GGEMSEvaluatedQuantity> {
  std::vector<GGEMSEvaluatedQuantity> quantities;
  quantities.reserve(4U);
  quantities.emplace_back(
      "half-life", "1.82890(23) h", GGEMSEvaluatedQuantityUnit::Second,
      GGEMSEvaluatedValueQualifier::CentralValue, k_half_life_seconds, 0.828L);
  quantities.emplace_back("beta-plus endpoint", "633.9(5) keV",
                          GGEMSEvaluatedQuantityUnit::MilliElectronVolt,
                          GGEMSEvaluatedValueQualifier::CentralValue,
                          static_cast<long double>(k_beta_endpoint_milli_eV),
                          500'000.0L);
  quantities.emplace_back("beta-plus mean energy", "249.5(3) keV",
                          GGEMSEvaluatedQuantityUnit::MilliElectronVolt,
                          GGEMSEvaluatedValueQualifier::CentralValue,
                          249'500'000.0L, 300'000.0L);
  quantities.emplace_back("beta-plus log ft", "3.57",
                          GGEMSEvaluatedQuantityUnit::Dimensionless,
                          GGEMSEvaluatedValueQualifier::CentralValue, 3.57L);
  return quantities;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto BuildEvaluatedEmissions()
    -> std::vector<GGEMSEvaluatedEmissionMetadata> {
  std::vector<GGEMSEvaluatedEmissionMetadata> emissions;
  emissions.reserve(7U);

  emissions.emplace_back(
      "positron beta continuum", particles::GGEMSParticleType::Positron,
      MakeYield("beta-plus yield", "96.86(19) per 100 parent decays",
                k_positron_yield, 0.0019L),
      GGEMSEvaluatedEnergy{"beta-plus endpoint", "633.9(5) keV",
                           GGEMSEvaluatedValueQualifier::CentralValue,
                           k_beta_endpoint_milli_eV, 500'000ULL},
      GGEMSEvaluatedEmissionDisposition::Included, 0U);

  emissions.emplace_back(
      "O Auger-L electron", particles::GGEMSParticleType::Electron,
      MakeYield("O Auger-L yield", "0.229(21) per 100 parent decays",
                k_auger_l_yield, 0.00021L),
      GGEMSEvaluatedEnergy{"O Auger-L energy", "0.0143 keV",
                           GGEMSEvaluatedValueQualifier::CentralValue,
                           14'300ULL},
      GGEMSEvaluatedEmissionDisposition::Included, 1U);

  emissions.emplace_back(
      "O Kα2 X ray", particles::GGEMSParticleType::Gamma,
      MakeYield("O Kα2 yield", "0.007(2) per 100 parent decays",
                k_k_alpha_2_yield, 0.00002L),
      GGEMSEvaluatedEnergy{"O Kα2 energy", "0.525 keV",
                           GGEMSEvaluatedValueQualifier::CentralValue,
                           525'000ULL},
      GGEMSEvaluatedEmissionDisposition::Included, 2U);

  emissions.emplace_back(
      "O Kα1 X ray", particles::GGEMSParticleType::Gamma,
      MakeYield("O Kα1 yield", "0.013(4) per 100 parent decays",
                k_k_alpha_1_yield, 0.00004L),
      GGEMSEvaluatedEnergy{"O Kα1 energy", "0.525 keV",
                           GGEMSEvaluatedValueQualifier::CentralValue,
                           525'000ULL},
      GGEMSEvaluatedEmissionDisposition::Included, 3U);

  emissions.emplace_back(
      "O KLL Auger electron group", particles::GGEMSParticleType::Electron,
      MakeYield("O KLL yield", "2.89(18) per 100 parent decays", 0.0289L,
                0.0018L),
      GGEMSEvaluatedEnergy{"O KLL energy range", "0.456-0.502 keV",
                           GGEMSEvaluatedValueQualifier::Range, std::nullopt,
                           std::nullopt, 456'000ULL, 502'000ULL},
      GGEMSEvaluatedEmissionDisposition::Deferred, std::nullopt,
      "Evaluated energy range supplied without a supported conditional "
      "energy distribution.");

  emissions.emplace_back(
      "positron-annihilation photons", particles::GGEMSParticleType::Gamma,
      MakeYield("annihilation-photon yield", "193.72(38) per 100 parent decays",
                1.9372L, 0.0038L),
      GGEMSEvaluatedEnergy{"annihilation-photon energy", "511 keV",
                           GGEMSEvaluatedValueQualifier::CentralValue,
                           511'000'000ULL},
      GGEMSEvaluatedEmissionDisposition::ExcludedGeneratedByTransport,
      std::nullopt,
      "Future positron annihilation transport produces these photons.");

  emissions.emplace_back(
      "electron capture", std::nullopt,
      MakeYield("electron-capture probability",
                "3.14(19) per 100 parent decays", 0.0314L, 0.0019L),
      std::nullopt,
      GGEMSEvaluatedEmissionDisposition::NoDirectTransportedParticle,
      std::nullopt,
      "Electron capture has no direct transported particle in the flattened "
      "model.");

  return emissions;
}

} // namespace

// =============================================================================
// =============================================================================

[[nodiscard]] auto BuildF18Radionuclide() -> GGEMSRadionuclideDefinition {
  GGEMSBetaTransition const transition{GGEMSBetaSign::Plus, 8U, 18U,
                                       k_beta_endpoint_milli_eV,
                                       GGEMSBetaTransitionClass::Allowed};
  GGEMSBetaSpectrumBuildOptions const beta_options{
      .model = GGEMSBetaSpectrumModel::AllowedPointCoulomb,
      .grid = {.target_maximum_bin_width_milli_eV =
                   k_beta_target_width_milli_eV}};
  GGEMSBetaSpectrumBuildResult beta_result =
      BuildBetaSpectrum(transition, beta_options);

  std::vector<GGEMSRadionuclideEmission> channels;
  channels.reserve(4U);
  channels.emplace_back(particles::GGEMSParticleType::Positron,
                        k_positron_yield, std::move(beta_result.distribution));
  channels.emplace_back(particles::GGEMSParticleType::Electron, k_auger_l_yield,
                        sources::GGEMSEnergyDistribution::BuildMono(14'300ULL));
  channels.emplace_back(
      particles::GGEMSParticleType::Gamma, k_k_alpha_2_yield,
      sources::GGEMSEnergyDistribution::BuildMono(525'000ULL));
  channels.emplace_back(
      particles::GGEMSParticleType::Gamma, k_k_alpha_1_yield,
      sources::GGEMSEnergyDistribution::BuildMono(525'000ULL));

  std::vector<GGEMSBetaApproximationMetadata> beta_approximations;
  beta_approximations.emplace_back(
      0U, transition, beta_options, beta_result.diagnostics,
      "Fermi point-Coulomb beta-spectrum treatment",
      "CODATA 2022 recommended values of the fundamental physical constants",
      "GGEMS AllowedPointCoulomb approximation; not an exact LNHB, BetaShape, "
      "or Geant4 spectrum.");

  GGEMSRadionuclideScientificMetadata metadata{
      GGEMSRadionuclideModelKind::PhysicalEvaluated,
      GGEMSRadionuclideCompleteness::EvaluatedSubset,
      GGEMSNuclideIdentity{9U, 18U, "F-18", false},
      GGEMSNuclideIdentity{8U, 18U, "O-18", true},
      MakeEvaluationSource(),
      BuildEvaluatedQuantities(),
      BuildEvaluatedEmissions(),
      std::move(beta_approximations)};

  return {"F-18",
          {"F18", "18F", "Fluorine-18"},
          k_half_life_seconds,
          std::move(metadata),
          std::move(channels)};
}

} // namespace ggems::core::radioactivity::builtins
