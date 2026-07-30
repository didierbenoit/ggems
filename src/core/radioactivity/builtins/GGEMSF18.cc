#include <cstdint>
#include <utility>
#include <vector>

#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/radioactivity/GGEMSBetaSpectrumBuilder.hh"
#include "GGEMS/core/radioactivity/GGEMSBetaTransition.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideEmission.hh"
#include "GGEMS/core/radioactivity/builtins/GGEMSBuiltInRadionuclides.hh"
#include "GGEMS/core/sources/GGEMSEnergyDistribution.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideDefinition.hh"

namespace ggems::core::radioactivity::builtins {
namespace {

// =============================================================================
// =============================================================================

constexpr long double k_half_life_seconds{6'584.04L};
constexpr long double k_positron_yield{0.9686L};
constexpr long double k_auger_l_yield{0.00229L};
constexpr long double k_oxygen_x_ray_yield{0.00020L};
constexpr std::uint64_t k_beta_endpoint_milli_eV{633'900'000ULL};
constexpr std::uint64_t k_beta_target_width_milli_eV{500'000ULL};

} // namespace

// =============================================================================
// =============================================================================

[[nodiscard]] auto BuildF18PositronSpectrum() -> GGEMSBetaSpectrumBuildResult {
  GGEMSBetaTransition const transition{GGEMSBetaSign::Plus, 8U, 18U,
                                       k_beta_endpoint_milli_eV,
                                       GGEMSBetaTransitionClass::Allowed};
  GGEMSBetaSpectrumBuildOptions const beta_options{
      .model = GGEMSBetaSpectrumModel::AllowedPointCoulomb,
      .grid = {.target_maximum_bin_width_milli_eV =
                   k_beta_target_width_milli_eV}};
  return BuildBetaSpectrum(transition, beta_options);
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto BuildF18Radionuclide() -> GGEMSRadionuclideDefinition {
  GGEMSBetaSpectrumBuildResult beta_result = BuildF18PositronSpectrum();

  std::vector<GGEMSRadionuclideEmission> channels;
  channels.reserve(3U);
  channels.emplace_back(particles::GGEMSParticleType::Positron,
                        k_positron_yield, std::move(beta_result.distribution));
  channels.emplace_back(particles::GGEMSParticleType::Electron, k_auger_l_yield,
                        sources::GGEMSEnergyDistribution::BuildMono(14'300ULL));

  // The equal-energy X-ray yields 0.00007 and 0.00013 share one signature.
  channels.emplace_back(
      particles::GGEMSParticleType::Gamma, k_oxygen_x_ray_yield,
      sources::GGEMSEnergyDistribution::BuildMono(525'000ULL));

  return {"F-18",
          {"F18", "18F", "Fluorine-18"},
          k_half_life_seconds,
          std::move(channels)};
}

} // namespace ggems::core::radioactivity::builtins
