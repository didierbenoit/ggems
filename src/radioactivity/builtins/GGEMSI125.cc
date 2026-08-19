#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/radioactivity/GGEMSRadionuclideDefinition.hh"
#include "GGEMS/radioactivity/GGEMSRadionuclideEmission.hh"
#include "GGEMS/radioactivity/builtins/GGEMSBuiltInRadionuclides.hh"
#include "GGEMS/sources/GGEMSEnergyDistribution.hh"

namespace ggems::core::radioactivity::builtins {
namespace {

// =============================================================================
// =============================================================================

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
// All tabular energies below are converted offline to exact positive integer
// milli-electronvolt values before being embedded in GGEMS.

constexpr std::size_t k_emission_count{4U};
constexpr long double k_half_life_seconds{5'131'123.2L};

constexpr std::uint64_t k_gamma_energy_milli_eV{35'492'200ULL};
constexpr long double k_gamma_yield{0.0663L};

constexpr std::array<double, 5U> k_x_ray_energies_milli_eV{{
    4'078'800.0,
    27'202'000.0,
    27'472'600.0,
    31'058'900.0,
    31'762'300.0,
}};

constexpr std::array<double, 5U> k_x_ray_line_yields{{
    0.147,
    0.393,
    0.732,
    0.209,
    0.0454,
}};

constexpr std::array<double, 13U> k_auger_electron_energies_milli_eV{{
    22'927.0,
    24'922.0,
    120'899.0,
    299'843.0,
    449'679.0,
    541'735.0,
    690'006.0,
    3'088'170.0,
    3'682'760.0,
    4'299'940.0,
    22'665'300.0,
    26'505'600.0,
    30'346'100.0,
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

constexpr std::array<double, 6U> k_conversion_electron_energies_milli_eV{{
    3'678'400.0,
    30'553'000.0,
    30'880'200.0,
    31'150'800.0,
    34'722'400.0,
    35'398'500.0,
}};

constexpr std::array<double, 6U> k_conversion_electron_line_yields{{
    0.776,
    0.0936,
    0.0172,
    0.0159,
    0.0256,
    0.00548,
}};

template <std::size_t Size>
[[nodiscard]] constexpr auto
SumLineYields(std::array<double, Size> const &line_yields) noexcept
    -> long double {
  long double total{0.0L};
  for (double const line_yield : line_yields) {
    total += static_cast<long double>(line_yield);
  }
  return total;
}

constexpr long double k_x_ray_yield{SumLineYields(k_x_ray_line_yields)};
constexpr long double k_auger_electron_yield{
    SumLineYields(k_auger_electron_line_yields)};
constexpr long double k_conversion_electron_yield{
    SumLineYields(k_conversion_electron_line_yields)};

} // namespace

// =============================================================================
// =============================================================================

[[nodiscard]] auto BuildI125Radionuclide() -> GGEMSRadionuclideDefinition {
  std::vector<GGEMSRadionuclideEmission> emissions;
  emissions.reserve(k_emission_count);

  emissions.emplace_back(
      particles::GGEMSParticleType::Gamma, k_gamma_yield,
      sources::GGEMSEnergyDistribution::BuildMono(k_gamma_energy_milli_eV));

  emissions.emplace_back(
      particles::GGEMSParticleType::Gamma, k_x_ray_yield,
      sources::GGEMSEnergyDistribution::BuildDiscreteLines(
          k_x_ray_energies_milli_eV, k_x_ray_line_yields, "meV"));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_auger_electron_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                             k_auger_electron_energies_milli_eV,
                             k_auger_electron_line_yields, "meV"));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_electron_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                             k_conversion_electron_energies_milli_eV,
                             k_conversion_electron_line_yields, "meV"));

  return {"I-125", k_half_life_seconds, std::move(emissions)};
}

} // namespace ggems::core::radioactivity::builtins
