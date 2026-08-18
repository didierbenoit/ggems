#include <array>
#include <cstdint>
#include <utility>
#include <vector>

#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/radioactivity/GGEMSRadionuclideDefinition.hh"
#include "GGEMS/radioactivity/GGEMSRadionuclideEmission.hh"
#include "GGEMS/radioactivity/builtins/GGEMSBuiltInRadionuclides.hh"
#include "GGEMS/radioactivity/detail/GGEMSTabulatedSpectrum.hh"
#include "GGEMS/sources/GGEMSEnergyDistribution.hh"

namespace ggems::core::radioactivity::builtins {
namespace {

// =============================================================================
// =============================================================================

// Scientific references for this built-in:
//
// Direct evaluated decay data:
// CEA/LNE-LNHB, Nucleide-LARA and Table de Radionucleides, H-3 / He-3,
// V. P. Chechev, evaluation updated May/June 2006. LNHB is the authority for
// the 12.312 y half-life, the 100 % beta-minus branch to stable He-3, the
// 18.591 keV Q value, and the evaluated 18.564 keV atomic beta endpoint.
//
// Beta spectral shape:
// LNHB BetaShape 2.2 (05/2021), H-3 beta-minus transition. The experimental
// shape-factor dN/dE column is used. BetaShape evaluates that spectrum on an
// 18.591 keV endpoint and reports a 5.69565 keV mean energy. GGEMS preserves
// that supplied BetaShape energy axis rather than rescaling it to the separate
// 18.564 keV atomic endpoint quoted by the 2006 LNHB evaluation.
//
// The differential density is integrated offline onto an exact regular GGEMS
// grid with a maximum target width of 0.5 keV and normalized conditionally.
// The physical beta-minus yield remains one electron per H-3 decay.
//
// MIRDspecs/ICRP-107 was used only as an independent cross-check that H-3 has
// one beta emission channel with unit yield; no MIRD spectral values are
// embedded here.

constexpr long double k_half_life_seconds{388'500'000.0L};
constexpr long double k_beta_minus_yield{1.0L};

constexpr std::uint64_t k_beta_spectrum_lower_edge_milli_eV{32ULL};
constexpr std::uint64_t k_beta_spectrum_bin_width_milli_eV{489236ULL};

// The lower edge is the minimal positive remainder that permits an exact
// 18.591 keV upper edge with an even integer-meV bin width no larger than
// 0.5 keV.
constexpr std::array<double, 38U> k_beta_spectrum_weights{{
    0.041613017273717766, 0.045490541359481704,
    0.04810627469021117, 0.049598511533586367,
    0.050279006500876765, 0.050344654159246612,
    0.049927562748275471, 0.049122065708913122,
    0.048000090661089193, 0.046617923815410175,
    0.045021584239986027, 0.04324950778438616,
    0.041334560477327489, 0.03930530211740571,
    0.037186995687840195, 0.035002207162909665,
    0.032771356637969179, 0.03051311993937026,
    0.028244625979463336, 0.025981808447445583,
    0.023739514903650793, 0.021531683489955305,
    0.019371471970928936, 0.017271320418973508,
    0.015243054723837827, 0.013298020819517333,
    0.011447014233495208, 0.009700459730151165,
    0.0080683620811877865, 0.0065603853293133788,
    0.0051858942489459567, 0.0039539615527811687,
    0.0028733914698240177, 0.0019527665332847363,
    0.0012004395233830927, 0.00062456335031376158,
    0.0002331103043489515, 3.3868391195151469e-05
}};

// =============================================================================
// =============================================================================

[[nodiscard]] auto BuildBetaSpectrum() -> sources::GGEMSEnergyDistribution {
  return detail::BuildTabulatedSpectrum(
      {.lower_edge_milli_eV = k_beta_spectrum_lower_edge_milli_eV,
       .bin_width_milli_eV = k_beta_spectrum_bin_width_milli_eV},
      k_beta_spectrum_weights);
}

} // namespace

// =============================================================================
// =============================================================================

[[nodiscard]] auto BuildH3Radionuclide() -> GGEMSRadionuclideDefinition {
  std::vector<GGEMSRadionuclideEmission> emissions;
  emissions.reserve(1U);
  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_beta_minus_yield, BuildBetaSpectrum());

  return {"H-3", k_half_life_seconds, std::move(emissions)};
}

} // namespace ggems::core::radioactivity::builtins
