#include <array>
#include <cstdint>
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

// Canonical Energy storage is now micro-electronvolts. The original meV
// quantization described below is preserved by exact integer scaling by 1000.

// Scientific references for this built-in:
//
// Direct evaluated decay and emission data:
// CEA/LNE-LNHB, Nucleide-LARA, I-123 / Te-123 decay data,
// V. Chiste and M. M. Be evaluation (2001, PenNuc 16/07/2003, tables updated
// through 5/8/2004). LARA supplies the half-life, Q value, nuclear gamma lines
// and compact Te X-ray groups. The associated LNHB/PenNuc EK/EL records supply
// the selected K/L internal-conversion electrons.
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
// Retained BetaShape 2.4 (06/2024), run with fixint=1, supports the selected
// EC branches and zero beta-plus intensity; calculated shell probabilities
// are audit information only. I-123 has no beta spectrum in this definition,
// and electron capture itself creates no placeholder incident particle.
//
// Capture neutrinos, daughter recoil nuclei, and later daughter-chain
// emissions are intentionally excluded. This definition represents prompt
// transportable particles emitted by one I-123 parent decay.
//
// All tabular energies below were converted offline to exact positive integer
// milli-electronvolt values before being embedded in GGEMS.

constexpr std::size_t k_emission_count{4U};
constexpr long double k_half_life_seconds{47'604.24L};

// Current LNHB/LARA prompt nuclear gamma emissions. Intensities published per
// 100 parent disintegrations are stored as yields per parent decay.
constexpr std::array<std::uint64_t, 40U> k_gamma_energies_micro_eV{{
    158'970'000'000ULL,   174'200'000'000ULL, 182'610'000'000ULL,
    192'170'000'000ULL,   197'220'000'000ULL, 198'230'000'000ULL,
    206'790'000'000ULL,   207'800'000'000ULL, 247'960'000'000ULL,
    257'510'000'000ULL,   278'360'000'000ULL, 281'030'000'000ULL,
    295'170'000'000ULL,   329'380'000'000ULL, 330'700'000'000ULL,
    343'730'000'000ULL,   346'350'000'000ULL, 405'020'000'000ULL,
    437'500'000'000ULL,   440'020'000'000ULL, 454'760'000'000ULL,
    505'330'000'000ULL,   528'960'000'000ULL, 538'540'000'000ULL,
    556'050'000'000ULL,   562'790'000'000ULL, 578'260'000'000ULL,
    599'690'000'000ULL,   610'050'000'000ULL, 624'570'000'000ULL,
    628'260'000'000ULL,   687'950'000'000ULL, 735'780'000'000ULL,
    783'590'000'000ULL,   837'100'000'000ULL, 877'520'000'000ULL,
    894'800'000'000ULL,   909'120'000'000ULL, 1'036'630'000'000ULL,
    1'068'120'000'000ULL,
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
constexpr std::array<std::uint64_t, 5U> k_x_ray_energies_micro_eV{{
    4'078'000'000ULL,
    27'202'000'000ULL,
    27'472'600'000ULL,
    31'104'400'000ULL,
    31'762'300'000ULL,
}};

constexpr std::array<double, 5U> k_x_ray_line_yields{{
    0.09,
    0.2469,
    0.4598,
    0.1316,
    0.0286,
}};

// MIRDspecs/ICRP-107 detailed Auger-electron lines. MIRD energies were rounded
// offline to the original integer milli-electronvolt grid. These are the only
// MIRD-derived runtime emission data in this I-123 definition.
constexpr std::array<std::uint64_t, 13U> k_auger_electron_energies_micro_eV{{
    22'924'000ULL,
    24'926'000ULL,
    121'020'000ULL,
    295'783'000ULL,
    449'494'000ULL,
    541'188'000ULL,
    687'637'000ULL,
    3'084'750'000ULL,
    3'678'860'000ULL,
    4'295'240'000ULL,
    22'665'300'000ULL,
    26'505'600'000ULL,
    30'346'100'000ULL,
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

// CEA/LNE-LNHB PenNuc EK/EL prompt internal-conversion electron records.
// The selected runtime scope is K/L conversion, not all atomic shells.
// The arrays are sorted by increasing electron energy as required by
// DiscreteLines.
constexpr std::array<std::uint64_t, 36U>
    k_conversion_electron_energies_micro_eV{{
        127'180'000'000ULL, 142'390'000'000ULL, 150'800'000'000ULL,
        154'360'000'000ULL, 160'370'000'000ULL, 165'410'000'000ULL,
        166'440'000'000ULL, 169'570'000'000ULL, 174'980'000'000ULL,
        176'010'000'000ULL, 177'980'000'000ULL, 187'550'000'000ULL,
        192'590'000'000ULL, 193'620'000'000ULL, 202'160'000'000ULL,
        203'190'000'000ULL, 216'150'000'000ULL, 225'710'000'000ULL,
        243'330'000'000ULL, 246'450'000'000ULL, 249'220'000'000ULL,
        252'890'000'000ULL, 273'630'000'000ULL, 276'400'000'000ULL,
        298'900'000'000ULL, 314'540'000'000ULL, 326'080'000'000ULL,
        341'720'000'000ULL, 408'210'000'000ULL, 435'390'000'000ULL,
        473'530'000'000ULL, 500'710'000'000ULL, 506'730'000'000ULL,
        533'910'000'000ULL, 592'800'000'000ULL, 619'980'000'000ULL,
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

  emissions.emplace_back(particles::GGEMSParticleType::Gamma, k_gamma_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                             k_gamma_energies_micro_eV, k_gamma_line_yields));

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

  return {"I-123", k_half_life_seconds, std::move(emissions)};
}

} // namespace ggems::core::radioactivity::builtins
