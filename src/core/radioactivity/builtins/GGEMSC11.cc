#include <cstdint>
#include <utility>
#include <vector>

#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/radioactivity/GGEMSBetaSpectrumBuilder.hh"
#include "GGEMS/core/radioactivity/GGEMSBetaTransition.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideDefinition.hh"
#include "GGEMS/core/radioactivity/GGEMSRadionuclideEmission.hh"
#include "GGEMS/core/radioactivity/builtins/GGEMSBuiltInRadionuclides.hh"

namespace ggems::core::radioactivity::builtins {
namespace {

// =============================================================================
// =============================================================================

constexpr long double k_half_life_seconds{1'221.66L};
constexpr long double k_positron_yield{0.99750L};
constexpr std::uint64_t k_beta_endpoint_milli_eV{960'500'000ULL};
constexpr std::uint64_t k_beta_target_width_milli_eV{500'000ULL};

} // namespace

// =============================================================================
// =============================================================================

[[nodiscard]] auto BuildC11Radionuclide() -> GGEMSRadionuclideDefinition {
  GGEMSBetaTransition const transition{GGEMSBetaSign::Plus, 5U, 11U,
                                       k_beta_endpoint_milli_eV,
                                       GGEMSBetaTransitionClass::Allowed};
  GGEMSBetaSpectrumBuildOptions const beta_options{
      .model = GGEMSBetaSpectrumModel::AllowedPointCoulomb,
      .grid = {.target_maximum_bin_width_milli_eV =
                   k_beta_target_width_milli_eV}};
  GGEMSBetaSpectrumBuildResult beta_result =
      BuildBetaSpectrum(transition, beta_options);

  std::vector<GGEMSRadionuclideEmission> emissions;
  emissions.reserve(1U);
  emissions.emplace_back(particles::GGEMSParticleType::Positron,
                         k_positron_yield, std::move(beta_result.distribution));

  return {"C-11",
          {"C11", "11C", "Carbon-11"},
          k_half_life_seconds,
          std::move(emissions)};
}

} // namespace ggems::core::radioactivity::builtins
