#include <array>
#include <cstddef>
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

// Scientific references for this built-in:
//
// Direct evaluated decay and emission data:
// CEA/LNE-LNHB, Nucleide-LARA, I-123 / Te-123 decay data,
// V. Chiste and M. M. Be evaluation (2001-2003). The LARA direct-emission
// export is the authority for the half-life, Q value, nuclear gamma lines,
// compact Te X-ray groups, and shell-resolved K/L internal-conversion
// electrons.
//
// Companion evaluation:
// CEA/LNE-LNHB, Table de Radionucleides, I-123. It documents the electron
// capture decay scheme, transition probabilities, gamma intensities, internal
// conversion coefficients, and Te atomic relaxation data.
//
// Auger electrons:
// MIRDsoft MIRDspecs, I-123 Summary Spectrum.csv. MIRDspecs identifies ICRP
// Publication 107, "Nuclear Decay Data for Dosimetric Calculations" (2008),
// as the spectra source. GGEMS uses MIRD only for the 13 detailed
// Auger-electron lines because the compact LNHB/LARA tables aggregate Auger
// groups into energy ranges.
//
// BetaShape 2.2 provides the evaluated electron-capture branch and shell
// probabilities used for audit only. I-123 has no beta spectrum in this GGEMS
// definition, and electron capture itself creates no placeholder incident
// particle.
//
// Capture neutrinos, daughter recoil nuclei, and later daughter-chain
// emissions are intentionally excluded. This definition represents prompt
// transportable particles emitted by one I-123 parent decay.
//
// All tabular energies below are converted offline to exact positive integer
// milli-electronvolt values before being embedded in GGEMS.

constexpr std::size_t k_emission_count{4U};
constexpr long double k_half_life_seconds{47'604.24L};

// Current LNHB/LARA prompt nuclear gamma emissions. Intensities published per
// 100 parent disintegrations are stored as yields per parent decay.
constexpr std::array<double, 40U> k_gamma_energies_milli_eV{{
    158'970'000.0, 174'200'000.0, 182'610'000.0,   192'170'000.0,
    197'220'000.0, 198'230'000.0, 206'790'000.0,   207'800'000.0,
    247'960'000.0, 257'510'000.0, 278'360'000.0,   281'030'000.0,
    295'170'000.0, 329'380'000.0, 330'700'000.0,   343'730'000.0,
    346'350'000.0, 405'020'000.0, 437'500'000.0,   440'020'000.0,
    454'760'000.0, 505'330'000.0, 528'960'000.0,   538'540'000.0,
    556'050'000.0, 562'790'000.0, 578'260'000.0,   599'690'000.0,
    610'050'000.0, 624'570'000.0, 628'260'000.0,   687'950'000.0,
    735'780'000.0, 783'590'000.0, 837'100'000.0,   877'520'000.0,
    894'800'000.0, 909'120'000.0, 1'036'630'000.0, 1'068'120'000.0,
}};

constexpr std::array<double, 40U> k_gamma_line_yields{{
    0.8325,     0.0000083, 0.00018,   0.000199,  0.0000033,  0.000035,
    0.000033,   0.0000112, 0.000698,  0.000016,  0.000023,   0.000789,
    0.00001582, 0.000026,  0.0001164, 0.000044,  0.001257,   0.0000298,
    0.000007,   0.004229,  0.0000412, 0.00266,   0.0128,     0.003788,
    0.000029,   0.0000115, 0.0000126, 0.0000266, 0.000011,   0.000798,
    0.0000164,  0.000269,  0.000616,  0.000591,  0.00000582, 0.0000083,
    0.0000101,  0.0000141, 0.0000097, 0.0000142,
}};

// CEA/LNE-LNHB Nucleide-LARA compact Te X-ray emissions. Grouped
// representative energies are preserved without artificial sub-line splitting.
constexpr std::array<double, 5U> k_x_ray_energies_milli_eV{{
    4'078'000.0,
    27'202'000.0,
    27'472'600.0,
    31'104'400.0,
    31'762'300.0,
}};

constexpr std::array<double, 5U> k_x_ray_line_yields{{
    0.09,
    0.2469,
    0.4598,
    0.1316,
    0.0286,
}};

// MIRDspecs/ICRP-107 detailed Auger-electron lines. MIRD energies are rounded
// offline to GGEMS integer milli-electronvolt storage. These are the only
// MIRD-derived runtime emission data in this I-123 definition.
constexpr std::array<double, 13U> k_auger_electron_energies_milli_eV{{
    22'924.0,
    24'926.0,
    121'020.0,
    295'783.0,
    449'494.0,
    541'188.0,
    687'637.0,
    3'084'750.0,
    3'678'860.0,
    4'295'240.0,
    22'665'300.0,
    26'505'600.0,
    30'346'100.0,
}};

constexpr std::array<double, 13U> k_auger_electron_line_yields{{
    7.26733,
    2.42736,
    0.83628,
    0.155577,
    1.86981,
    0.0720601,
    0.000271332,
    0.734409,
    0.207978,
    0.0147089,
    0.0807305,
    0.0355571,
    0.00374455,
}};

// CEA/LNE-LNHB LARA shell-resolved K/L prompt internal-conversion electrons.
// The arrays are sorted by increasing electron energy as required by
// DiscreteLines.
constexpr std::array<double, 36U> k_conversion_electron_energies_milli_eV{{
    127'180'000.0, 142'390'000.0, 150'800'000.0, 154'360'000.0, 160'370'000.0,
    165'410'000.0, 166'440'000.0, 169'570'000.0, 174'980'000.0, 176'010'000.0,
    177'980'000.0, 187'550'000.0, 192'590'000.0, 193'620'000.0, 202'160'000.0,
    203'190'000.0, 216'150'000.0, 225'710'000.0, 243'330'000.0, 246'450'000.0,
    249'220'000.0, 252'890'000.0, 273'630'000.0, 276'400'000.0, 298'900'000.0,
    314'540'000.0, 326'080'000.0, 341'720'000.0, 408'210'000.0, 435'390'000.0,
    473'530'000.0, 500'710'000.0, 506'730'000.0, 533'910'000.0, 592'800'000.0,
    619'980'000.0,
}};

constexpr std::array<double, 36U> k_conversion_electron_line_yields{{
    0.1372,      0.00000132, 0.000025,    0.01798,     0.0000235,   0.00000036,
    0.0000037,   0.00000024, 0.0000031,   0.00000104,  0.0000043,   0.000004,
    0.000000059, 0.00000063, 0.00000053,  0.00000017,  0.0000377,   0.00000084,
    0.0000059,   0.00000089, 0.0000286,   0.000000147, 0.000000133, 0.0000038,
    0.00000276,  0.00002615, 0.000000438, 0.00000329,  0.0000436,   0.00000614,
    0.0000215,   0.00000283, 0.00012,     0.000019,    0.00000335,  0.000000438,
}};

// =============================================================================
// =============================================================================

constexpr long double k_gamma_yield{std::accumulate(
    k_gamma_line_yields.begin(), k_gamma_line_yields.end(), 0.0L)};

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

[[nodiscard]] auto BuildI123Radionuclide() -> GGEMSRadionuclideDefinition {
  std::vector<GGEMSRadionuclideEmission> emissions;
  emissions.reserve(k_emission_count);

  emissions.emplace_back(
      particles::GGEMSParticleType::Gamma, k_gamma_yield,
      sources::GGEMSEnergyDistribution::BuildDiscreteLines(
          k_gamma_energies_milli_eV, k_gamma_line_yields, "meV"));

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

  return {"I-123", k_half_life_seconds, std::move(emissions)};
}

} // namespace ggems::core::radioactivity::builtins
