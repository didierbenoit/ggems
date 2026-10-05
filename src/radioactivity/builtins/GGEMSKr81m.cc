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
 * \brief Defines the compiled Kr-81m marginal source-emission laws.
 *
 * ENSDF, M. Shamsuzzoha Basunia, September 2024, NDS 199,271 (2025);
 * retained MIRD emission laws. Selected IT/EC source emissions.
 *
 * The 190.46 keV IT photon and its conversion/atomic products belong to
 * this metastable parent. The tiny direct EC branch is included; later
 * ground-state Kr-81 decay is excluded.
 *
 * Explicit MIRD photon, Auger and conversion lines retain absolute yields
 * after the parent-timing audit. No BetaShape generator is attributed to
 * these data.
 *
 * Physical yields remain independent emissions per parent decay. EC creates
 * no primary; neutrinos, recoil and additional radioactive daughter decays
 * are excluded.
 *
 * Selection and exclusions:
 * validation/radioactivity/data/Kr-81m/reference/.
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
#include "GGEMS/sources/GGEMSEnergyDistribution.hh"

namespace ggems::core::radioactivity::builtins {
namespace {

// =============================================================================
// =============================================================================

/*! \brief Evaluated Kr-81m parent half-life in seconds. */
constexpr long double k_half_life_seconds{13.10L};

// =============================================================================
// =============================================================================

/*! \brief Nuclear photon yield per parent decay. */
constexpr long double k_nuclear_gamma_yield{0.675199L};

/*! \brief Selected monoenergy in canonical integer micro-eV. */
constexpr std::uint64_t k_nuclear_gamma_energy_micro_eV{190'460'000'000ULL};

// =============================================================================
// =============================================================================

/*! \brief Atomic X-ray yield per parent decay. */
constexpr long double k_atomic_xray_yield{2.332874047764841L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 53U> k_atomic_xray_energies_micro_eV{
  {
    17'589'400ULL,     19'379'100ULL,     115'520'000ULL,    124'939'000ULL,
    174'000'000ULL,    215'600'000ULL,    228'901'000ULL,    1'302'110'000ULL,
    1'350'610'000ULL,  1'395'230'000ULL,  1'450'130'000ULL,  1'458'290'000ULL,
    1'466'440'000ULL,  1'474'010'000ULL,  1'475'150'000ULL,  1'521'340'000ULL,
    1'522'510'000ULL,  1'529'430'000ULL,  1'577'380'000ULL,  1'577'920'000ULL,
    1'578'050'000ULL,  1'579'420'000ULL,  1'584'420'000ULL,  1'632'950'000ULL,
    1'645'960'000ULL,  1'659'970'000ULL,  1'660'680'000ULL,  1'687'180'000ULL,
    1'695'340'000ULL,  1'700'860'000ULL,  1'715'580'000ULL,  1'757'610'000ULL,
    1'758'170'000ULL,  1'806'950'000ULL,  1'808'320'000ULL,  1'888'870'000ULL,
    1'889'580'000ULL,  11'832'900'000ULL, 11'881'400'000ULL, 12'551'700'000ULL,
    12'606'600'000ULL, 13'243'200'000ULL, 13'250'200'000ULL, 13'355'400'000ULL,
    13'356'500'000ULL, 13'423'400'000ULL, 13'424'000'000ULL, 14'064'900'000ULL,
    14'073'000'000ULL, 14'184'600'000ULL, 14'186'000'000ULL, 14'266'600'000ULL,
    14'267'300'000ULL,
  },
};

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 53U> k_atomic_xray_line_yields{
  {
    0.000179323,   2.15276,       4.6906E-9,     0.0000738279,  9.22643E-7,
    4.83533E-10,   0.00000531464, 8.02868E-8,    3.14755E-8,    0.000992152,
    0.000398933,   0.00000334247, 0.00000319094, 2.76431E-8,    2.70716E-7,
    0.00000277098, 1.35973E-7,    2.83092E-9,    6.16807E-9,    1.42667E-9,
    0.00039758,    0.00386964,    1.103E-8,      0.00206185,    0.0000369654,
    4.35341E-8,    4.12858E-8,    0.0000721641,  0.000128359,   0.0000197451,
    4.31655E-8,    4.38117E-10,   7.78071E-10,   4.24554E-7,    6.31753E-7,
    0.00000676699, 0.000011961,   0.00000397278, 0.00000767041, 0.0508304,
    0.098017,      5.47967E-7,    0.00000106916, 1.44108E-9,    2.06898E-9,
    3.65333E-8,    7.03087E-8,    0.00715157,    0.0139471,     0.0000208502,
    0.0000298735,  0.000628416,   0.0012089,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Atomic X-ray yield per parent decay. */
constexpr long double k_atomic_xray_weak_1_yield{1.0938661191E-9L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 11U> k_atomic_xray_weak_1_energies_micro_eV{
  {
    48'500'000ULL,
    54'900'000ULL,
    167'100'000ULL,
    1'361'780'000ULL,
    1'368'810'000ULL,
    1'417'320'000ULL,
    1'542'010'000ULL,
    1'542'570'000ULL,
    1'591'070'000ULL,
    1'689'610'000ULL,
    1'690'750'000ULL,
  },
};

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 11U> k_atomic_xray_weak_1_line_yields{
  {
    1.35991E-14,
    2.11938E-10,
    8.57946E-11,
    2.5712E-10,
    2.45985E-10,
    2.02542E-10,
    2.37155E-12,
    2.25075E-12,
    2.23132E-12,
    3.36172E-11,
    5.00021E-11,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Auger electron yield per parent decay. */
constexpr long double k_auger_electron_yield{1.8289832671039L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 18U> k_auger_electron_energies_micro_eV{
  {
    43'862'400ULL,
    56'258'500ULL,
    57'733'000ULL,
    65'478'000ULL,
    90'939'100ULL,
    98'252'400ULL,
    1'320'950'000ULL,
    1'407'150'000ULL,
    1'411'990'000ULL,
    1'511'590'000ULL,
    1'554'220'000ULL,
    1'670'360'000ULL,
    10'202'900'000ULL,
    10'798'000'000ULL,
    11'635'400'000ULL,
    12'337'100'000ULL,
    13'070'800'000ULL,
    13'878'600'000ULL,
  },
};

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 18U> k_auger_electron_line_yields{
  {
    0.0000807015,
    0.941951,
    0.0000312246,
    0.36977,
    0.00000563471,
    0.0639987,
    0.0000267465,
    0.331218,
    0.00000158831,
    0.0239096,
    3.63479E-8,
    0.000685276,
    0.00000635567,
    0.072487,
    0.00000194426,
    0.0230065,
    1.49206E-7,
    0.00180281,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Conversion-electron yield per parent decay. */
constexpr long double k_conversion_electron_yield{0.324775717L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U> k_conversion_electron_energies_micro_eV{
  {
    176'180'000'000ULL,
    188'558'000'000ULL,
    188'732'000'000ULL,
    188'787'000'000ULL,
    190'253'000'000ULL,
    190'460'000'000ULL,
  },
};

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_electron_line_yields{
  {
    0.26913,
    0.0255162,
    0.0106526,
    0.0108225,
    0.00781308,
    0.000841337,
  },
};

} // namespace

// =============================================================================
// =============================================================================

[[nodiscard]] auto BuildKr81mRadionuclide() -> GGEMSRadionuclideDefinition {
  std::vector<GGEMSRadionuclideEmission> emissions;
  emissions.reserve(5U);

  emissions.emplace_back(particles::GGEMSParticleType::Gamma,
                         k_nuclear_gamma_yield,
                         sources::GGEMSEnergyDistribution::BuildMono(
                           k_nuclear_gamma_energy_micro_eV));

  emissions.emplace_back(
    particles::GGEMSParticleType::Gamma, k_atomic_xray_yield,
    sources::GGEMSEnergyDistribution::BuildDiscreteLines(
      k_atomic_xray_energies_micro_eV, k_atomic_xray_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Gamma,
                         k_atomic_xray_weak_1_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_atomic_xray_weak_1_energies_micro_eV,
                           k_atomic_xray_weak_1_line_yields));

  emissions.emplace_back(
    particles::GGEMSParticleType::Electron, k_auger_electron_yield,
    sources::GGEMSEnergyDistribution::BuildDiscreteLines(
      k_auger_electron_energies_micro_eV, k_auger_electron_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_electron_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_electron_energies_micro_eV,
                           k_conversion_electron_line_yields));

  return {"Kr-81m", k_half_life_seconds, std::move(emissions)};
}

} // namespace ggems::core::radioactivity::builtins
