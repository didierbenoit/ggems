// *****************************************************************************
// * This file is part of GGEMS.                                               *
// *                                                                           *
// * SPDX-License-Identifier: GPL-3.0-or-later                                 *
// * Copyright (C) 2017-2026 CHRU de Brest, Université de Bretagne Occidentale,*
// * Inserm.                                                                   *
// *                                                                           *
// * GGEMS is free software: you can redistribute it and/or modify             *
// * it under the terms of the GNU General Public License as published by      *
// * the Free Software Foundation, either version 3 of the License, or         *
// * (at your option) any later version.                                       *
// *                                                                           *
// * GGEMS is distributed in the hope that it will be useful,                  *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
// * GNU General Public License for more details.                              *
// *                                                                           *
// * You should have received a copy of the GNU General Public License         *
// * along with GGEMS. If not, see <https://www.gnu.org/licenses/>.            *
// *****************************************************************************

/*!
 * \file
 * \brief Defines the compiled Y-88 marginal source-emission laws.
 *
 * V. P. Chechev and N. K. Kuzmenko, KRI/LNHB, June 2015. Selected
 * EC/beta-plus source emissions.
 *
 * BetaShape 2.4 (06/2024): experimental first-forbidden unique positron
 * law; fixint=1 preserves the evaluated EC/beta-plus split. Conditional bin
 * masses integrate the selected piecewise-linear densities. Full-support
 * references and finite grids remain distinct.
 *
 * Bins up to 4 keV preserve positive-tail ticket reachability. Source
 * annihilation photons and unsupported internal-pair energy laws are
 * excluded.
 *
 * Compact LNHB photon and PenNuc conversion lines retain absolute yields;
 * unsupported grouped Auger energy laws are excluded.
 *
 * Physical yields remain independent emissions per parent decay. EC creates
 * no primary; neutrinos, recoil and additional radioactive daughter decays
 * are excluded.
 *
 * Selection and exclusions: validation/radioactivity/data/Y-88/reference/.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

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

/*! \brief Evaluated Y-88 parent half-life in seconds. */
constexpr long double k_half_life_seconds{9212832.00L};

// =============================================================================
// =============================================================================

/*! \brief Positron yield per parent decay for the 764.5 keV endpoint. */
constexpr long double k_beta_plus_764_5_keV_yield{0.0021L};

/*! \brief First bin lower edge in micro-eV; full reference support starts at
 * zero. */
constexpr std::uint64_t k_beta_plus_764_5_keV_lower_edge_micro_eV{160'000ULL};

/*! \brief Regular bin width in micro-eV for the 764.5 keV endpoint. */
constexpr std::uint64_t k_beta_plus_764_5_keV_bin_width_micro_eV{
  3'981'770'000ULL};

/*! \brief Conditional integrated bin masses; physical branch yield is stored
 * separately. */
constexpr std::array<double, 192U> k_beta_plus_764_5_keV_weights{
  {
    1.780928071918656e-08,  1.464274991278736e-06,  1.2559880988882128e-05,
    4.4525858255565641e-05, 0.00010288174999906453, 0.00018768606928435308,
    0.00029628773499146456, 0.0004251144986099104,  0.00057054591411704767,
    0.000729267637957003,   0.00089839147547554296, 0.0010754643823951524,
    0.0012584282748065406,  0.0014455718849463056,  0.0016354742382979164,
    0.0018269643257835201,  0.0020190774943484452,  0.0022110125162700996,
    0.0024021065046604469,  0.0025918358367392896,  0.0027797607584964215,
    0.0029655035671549679,  0.0031487887100959508,  0.0033293699388815411,
    0.0035070645316734881,  0.0036817071214826466,  0.0038531965825273533,
    0.0040214409307226728,  0.0041863667122615982,  0.0043479388314423455,
    0.0045061224973726704,  0.0046608985195447433,  0.0048122624222883123,
    0.0049602204433876421,  0.0051047849033353156,  0.0052459709906585838,
    0.0053838002333154106,  0.0055183192355941395,  0.0056495369889813319,
    0.0057774977602900622,  0.0059022408232847698,  0.0060238074485490286,
    0.0061422234685688448,  0.0062575468359636274,  0.0063697965487056647,
    0.0064790292402583114,  0.0065852856472953109,  0.0066885952917822983,
    0.0067890025565158774,  0.0068865456838504892,  0.0069812705744165806,
    0.0070731986314351188,  0.0071623737788501091,  0.0072488331826644527,
    0.0073325977565061014,  0.0074137038364520931,  0.0074921805892242703,
    0.0075680703486338203,  0.007641379752265141,   0.0077121582559096678,
    0.0077804035358387128,  0.0078461561133344772,  0.0079094294883918469,
    0.0079702488460089045,  0.0080286343582996519,  0.0080845969247612184,
    0.0081381536806875779,  0.0081893226426478921,  0.0082381131224199006,
    0.0082845287105487581,  0.0083286021657901031,  0.0083703187675378734,
    0.0084096957228678385,  0.0084467398572361605,  0.0084814579452325515,
    0.0085138586471666545,  0.0085439271646603424,  0.0085716873802716073,
    0.0085971314802929175,  0.0086202461708097776,  0.0086410474854798323,
    0.0086595372228772047,  0.0086756908385163152,  0.0086895238927614787,
    0.0087010271880927741,  0.0087101939036527114,  0.0087170186029645567,
    0.0087214911510376091,  0.0087236186180981532,  0.0087233875833936311,
    0.0087207768126369815,  0.0087157985820085909,  0.0087084374301582609,
    0.0086986840266908821,  0.0086865257107962852,  0.0086719565297122599,
    0.0086549694509688996,  0.0086355565153564927,  0.0086137161437270168,
    0.0085894219744136815,  0.0085626814868540544,  0.008533478189133005,
    0.0085018096415309854,  0.0084676650631537766,  0.0084310404188359996,
    0.0083919306239107257,  0.0083503269707879864,  0.0083062239134197215,
    0.0082596175581031838,  0.0082105028274315987,  0.0081588846089773442,
    0.0081047628421771455,  0.0080481186995123525,  0.0079889803464306495,
    0.0079273293127121464,  0.0078631786088625873,  0.0077965300275049743,
    0.0077273814513399743,  0.0076557527936215106,  0.0075816479811219072,
    0.0075050818651101175,  0.0074260639368529937,  0.0073446052617854342,
    0.0072607285276705836,  0.0071744445807145731,  0.0070857873133889802,
    0.0069947660902221851,  0.006901421625889013,   0.00680576004825995,
    0.0067078279406335194,  0.0066076442893730656,  0.006505271285134129,
    0.0064007090679791302,  0.0062940287961137046,  0.0061852619570172537,
    0.0060744615148152579,  0.0059616650894725167,  0.0058469363708621965,
    0.0057303308270386516,  0.0056118957421951174,  0.0054917167430815822,
    0.0053698392953015059,  0.0052463512754094303,  0.0051213140166398189,
    0.0049948090379352073,  0.0048669291610755104,  0.0047377521442012469,
    0.0046073641082143985,  0.0044758704023639467,  0.0043433559937291677,
    0.0042099455980521422,  0.0040757421277998343,  0.0039408431964131777,
    0.0038053767480091742,  0.0036694676704029194,  0.0035332530186917609,
    0.0033968559317537955,  0.0032604103122121013,  0.0031240714125650311,
    0.0029879894462327803,  0.0028523077822293521,  0.0027172026503093404,
    0.0025828404976455262,  0.0024493831822102862,  0.002317016418058399,
    0.0021859273321755668,  0.0020563123394051106,  0.0019283590095818634,
    0.0018022821850278354,  0.0016782904647054456,  0.0015566033131771443,
    0.0014374480895339933,  0.001321056990757873,   0.0012076701779333815,
    0.0010975366659670771,  0.0009909114127792096,  0.00088805645412528341,
    0.00078924467071830114, 0.00069475394912584703, 0.00060487214083626475,
    0.00051989209728284572, 0.00044012018303916624, 0.0003658652130237886,
    0.00029745000457444021, 0.00023520266187923927, 0.00017946143803974169,
    0.00013057363511960766, 8.8895328001849838e-05, 5.4792125272384276e-05,
    2.8638896758386166e-05, 1.0820277493573274e-05, 1.7165961916345057e-06,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Nuclear photon yield per parent decay. */
constexpr long double k_nuclear_gamma_yield{1.937260L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 7U> k_nuclear_gamma_energies_micro_eV{
  {
    484'352'000'000ULL,
    850'643'000'000ULL,
    898'042'000'000ULL,
    1'382'387'000'000ULL,
    1'836'070'000'000ULL,
    2'734'092'000'000ULL,
    3'218'426'000'000ULL,
  },
};

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 7U> k_nuclear_gamma_line_yields{
  {
    0.000009,
    0.00048,
    0.937,
    0.00016,
    0.99346,
    0.00608,
    0.000071,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Atomic X-ray yield per parent decay. */
constexpr long double k_atomic_xray_yield{0.6342L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 5U> k_atomic_xray_energies_micro_eV{
  {
    1'890'200'000ULL,
    14'098'000'000ULL,
    14'165'200'000ULL,
    15'876'700'000ULL,
    16'094'300'000ULL,
  },
};

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 5U> k_atomic_xray_line_yields{
  {
    0.0276,
    0.1755,
    0.3371,
    0.0832,
    0.0108,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Conversion-electron yield per parent decay. */
constexpr long double k_conversion_484_352_keV_yield{1.1253E-8L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_484_352_keV_energies_micro_eV{
    {
      468'247'000'000ULL,
      482'136'000'000ULL,
      482'345'000'000ULL,
      482'412'000'000ULL,
      484'117'000'000ULL,
      484'332'000'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_484_352_keV_line_yields{
  {
    1E-8,
    1E-9,
    1.9E-11,
    3.1E-11,
    1.8E-10,
    2.3E-11,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Conversion-electron yield per parent decay. */
constexpr long double k_conversion_850_643_keV_yield{4.0773E-7L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_850_643_keV_energies_micro_eV{
    {
      834'542'000'000ULL,
      848'431'000'000ULL,
      848'640'000'000ULL,
      848'707'000'000ULL,
      850'412'000'000ULL,
      850'627'000'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_850_643_keV_line_yields{
  {
    3.6E-7,
    3.8E-8,
    1.12E-9,
    1.02E-9,
    6.7E-9,
    8.9E-10,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Conversion-electron yield per parent decay. */
constexpr long double k_conversion_898_042_keV_yield{0.0002882888L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_898_042_keV_energies_micro_eV{
    {
      881'942'000'000ULL,
      895'831'000'000ULL,
      896'040'000'000ULL,
      896'107'000'000ULL,
      897'812'000'000ULL,
      898'027'000'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_898_042_keV_line_yields{
  {
    0.0002558,
    0.00002642,
    3.008E-7,
    5.75E-7,
    0.00000458,
    6.13E-7,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Conversion-electron yield per parent decay. */
constexpr long double k_conversion_1382_387_keV_yield{4.6211E-8L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_1382_387_keV_energies_micro_eV{
    {
      1'366'294'000'000ULL,
      1'380'183'000'000ULL,
      1'380'392'000'000ULL,
      1'380'459'000'000ULL,
      1'382'164'000'000ULL,
      1'382'379'000'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_1382_387_keV_line_yields{
  {
    4.1E-8,
    4.3E-9,
    4.9E-11,
    3.4E-11,
    7.3E-10,
    9.8E-11,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Conversion-electron yield per parent decay. */
constexpr long double k_conversion_1836_07_keV_yield{0.0001623225L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_1836_07_keV_energies_micro_eV{
    {
      1'819'985'000'000ULL,
      1'833'874'000'000ULL,
      1'834'083'000'000ULL,
      1'834'150'000'000ULL,
      1'835'855'000'000ULL,
      1'836'070'000'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_1836_07_keV_line_yields{
  {
    0.000144,
    0.00001502,
    1.778E-7,
    1.957E-7,
    0.000002583,
    3.46E-7,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Conversion-electron yield per parent decay. */
constexpr long double k_conversion_2734_092_keV_yield{7.53135E-7L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_2734_092_keV_energies_micro_eV{
    {
      2'718'032'000'000ULL,
      2'731'921'000'000ULL,
      2'732'130'000'000ULL,
      2'732'197'000'000ULL,
      2'733'902'000'000ULL,
      2'734'117'000'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_2734_092_keV_line_yields{
  {
    6.68E-7,
    7E-8,
    8.88E-10,
    6.37E-10,
    1.2E-8,
    1.61E-9,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Conversion-electron yield per parent decay. */
constexpr long double k_conversion_3218_426_keV_yield{4.3848E-9L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_3218_426_keV_energies_micro_eV{
    {
      3'202'384'000'000ULL,
      3'216'273'000'000ULL,
      3'216'482'000'000ULL,
      3'216'549'000'000ULL,
      3'218'254'000'000ULL,
      3'218'469'000'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_3218_426_keV_line_yields{
  {
    3.9E-9,
    4E-10,
    2.6E-12,
    4E-12,
    6.9E-11,
    9.2E-12,
  },
};

} // namespace

// =============================================================================
// =============================================================================

[[nodiscard]] auto BuildY88Radionuclide() -> GGEMSRadionuclideDefinition {
  std::vector<GGEMSRadionuclideEmission> emissions;
  emissions.reserve(10U);

  emissions.emplace_back(
    particles::GGEMSParticleType::Positron, k_beta_plus_764_5_keV_yield,
    detail::BuildTabulatedSpectrum(
      {
        .lower_edge_micro_eV = k_beta_plus_764_5_keV_lower_edge_micro_eV,
        .bin_width_micro_eV = k_beta_plus_764_5_keV_bin_width_micro_eV,
      },
      k_beta_plus_764_5_keV_weights));

  emissions.emplace_back(
    particles::GGEMSParticleType::Gamma, k_nuclear_gamma_yield,
    sources::GGEMSEnergyDistribution::BuildDiscreteLines(
      k_nuclear_gamma_energies_micro_eV, k_nuclear_gamma_line_yields));

  emissions.emplace_back(
    particles::GGEMSParticleType::Gamma, k_atomic_xray_yield,
    sources::GGEMSEnergyDistribution::BuildDiscreteLines(
      k_atomic_xray_energies_micro_eV, k_atomic_xray_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_484_352_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_484_352_keV_energies_micro_eV,
                           k_conversion_484_352_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_850_643_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_850_643_keV_energies_micro_eV,
                           k_conversion_850_643_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_898_042_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_898_042_keV_energies_micro_eV,
                           k_conversion_898_042_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_1382_387_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_1382_387_keV_energies_micro_eV,
                           k_conversion_1382_387_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_1836_07_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_1836_07_keV_energies_micro_eV,
                           k_conversion_1836_07_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_2734_092_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_2734_092_keV_energies_micro_eV,
                           k_conversion_2734_092_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_3218_426_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_3218_426_keV_energies_micro_eV,
                           k_conversion_3218_426_keV_line_yields));

  return {"Y-88", k_half_life_seconds, std::move(emissions)};
}

} // namespace ggems::core::radioactivity::builtins
