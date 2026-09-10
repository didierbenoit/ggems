#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>
#include <numeric>

#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/radioactivity/GGEMSRadionuclideDefinition.hh"
#include "GGEMS/radioactivity/GGEMSRadionuclideEmission.hh"
#include "GGEMS/radioactivity/builtins/GGEMSBuiltInRadionuclides.hh"
#include "GGEMS/sources/GGEMSEnergyDistribution.hh"

namespace ggems::core::radioactivity::builtins {
namespace {

// =============================================================================
// =============================================================================

// Canonical Energy storage is now micro-electronvolts. The original meV
// quantization described below is preserved by exact integer scaling by 1000.

// Scientific references for this built-in:
//
// Direct evaluated decay and emission data:
// CEA/LNE-LNHB, DDEP I-125 / Te-125 evaluation, V. Chiste, E. Schonfeld and
// M. M. Be, completed in 2010. The evaluation is the authority for the
// half-life, Q value, 100% electron-capture decay, the 35.4922 keV gamma
// emission, compact Te X-ray groups, and shell-resolved conversion electrons.
//
// Auger electrons:
// MIRDsoft MIRDspecs, I-125 Summary Spectrum.csv. MIRDspecs identifies ICRP
// Publication 107, "Nuclear Decay Data for Dosimetric Calculations" (2008),
// as the spectra source. GGEMS uses MIRD only for the 13 detailed Auger lines
// because the DDEP tables publish the Auger emissions as aggregate groups.
//
// BetaShape provides the evaluated electron-capture branch and shell
// probabilities for audit only. I-125 has no beta spectrum in this GGEMS
// definition, and electron capture itself creates no placeholder incident
// particle. Capture neutrinos and daughter recoil nuclei are intentionally
// excluded. The 1.48 ns Te-125 de-excitation and associated atomic relaxation
// emissions are flattened at parent decay time.
//
// All tabular energies below were converted offline to exact positive integer
// milli-electronvolt values before being embedded in GGEMS.

constexpr std::size_t k_emission_count{4U};
constexpr long double k_half_life_seconds{5'131'123.2L};

constexpr std::uint64_t k_gamma_energy_micro_eV{35'492'200'000ULL};
constexpr long double k_gamma_yield{0.0663L};

constexpr std::array<std::uint64_t, 5U> k_x_ray_energies_micro_eV{{
    4'078'800'000ULL,
    27'202'000'000ULL,
    27'472'600'000ULL,
    31'058'900'000ULL,
    31'762'300'000ULL,
}};

constexpr std::array<double, 5U> k_x_ray_line_yields{{
    0.147,
    0.393,
    0.732,
    0.209,
    0.0454,
}};

constexpr std::array<std::uint64_t, 13U> k_auger_electron_energies_micro_eV{{
    22'927'000ULL,
    24'922'000ULL,
    120'899'000ULL,
    299'843'000ULL,
    449'679'000ULL,
    541'735'000ULL,
    690'006'000ULL,
    3'088'170'000ULL,
    3'682'760'000ULL,
    4'299'940'000ULL,
    22'665'300'000ULL,
    26'505'600'000ULL,
    30'346'100'000ULL,
}};

constexpr std::array<double, 13U> k_auger_electron_line_yields{{
    12.198,
    4.08498,
    1.41044,
    0.275991,
    3.12454,
    0.120544,
    0.000457128,
    1.2264,
    0.347461,
    0.0245935,
    0.130975,
    0.057687,
    0.00607509,
}};

constexpr std::array<std::uint64_t, 6U> k_conversion_electron_energies_micro_eV{
    {
        3'678'400'000ULL,
        30'553'000'000ULL,
        30'880'200'000ULL,
        31'150'800'000ULL,
        34'722'400'000ULL,
        35'398'500'000ULL,
    }};

constexpr std::array<double, 6U> k_conversion_electron_line_yields{{
    0.776,
    0.0936,
    0.0172,
    0.0159,
    0.0256,
    0.00548,
}};

// =============================================================================
// =============================================================================

constexpr long double k_x_ray_yield{std::accumulate(
    k_x_ray_line_yields.begin(), k_x_ray_line_yields.end(), 0.0L)};

constexpr long double k_auger_electron_yield{
    std::accumulate(k_auger_electron_line_yields.begin(),
                    k_auger_electron_line_yields.end(), 0.0L)};

constexpr long double k_conversion_electron_yield{
    std::accumulate(k_conversion_electron_line_yields.begin(),
                    k_conversion_electron_line_yields.end(), 0.0L)};

} // namespace

// =============================================================================
// =============================================================================

[[nodiscard]] auto BuildI125Radionuclide() -> GGEMSRadionuclideDefinition {
  std::vector<GGEMSRadionuclideEmission> emissions;
  emissions.reserve(k_emission_count);

  emissions.emplace_back(
      particles::GGEMSParticleType::Gamma, k_gamma_yield,
      sources::GGEMSEnergyDistribution::BuildMono(k_gamma_energy_micro_eV));

  emissions.emplace_back(particles::GGEMSParticleType::Gamma, k_x_ray_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                             k_x_ray_energies_micro_eV, k_x_ray_line_yields));

  emissions.emplace_back(
      particles::GGEMSParticleType::Electron, k_auger_electron_yield,
      sources::GGEMSEnergyDistribution::BuildDiscreteLines(
          k_auger_electron_energies_micro_eV, k_auger_electron_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_electron_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                             k_conversion_electron_energies_micro_eV,
                             k_conversion_electron_line_yields));

  return {"I-125", k_half_life_seconds, std::move(emissions)};
}

} // namespace ggems::core::radioactivity::builtins
