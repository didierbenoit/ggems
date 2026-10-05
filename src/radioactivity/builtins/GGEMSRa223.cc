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
 * \brief Defines the compiled Ra-223 marginal source-emission laws.
 *
 * LNHB retained evaluations select independent emissions per parent decay.
 * Prompt photon and conversion yields are included where separable;
 * unsupported Auger energy laws, neutrinos and recoil are excluded.
 *
 * Direct parent alpha lines use evaluated discrete energies and yields.
 * No radioactive daughter-chain emissions are added.
 *
 * Selection and exclusions: validation/radioactivity/data/Ra-223/reference/.
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

/*! \brief Evaluated parent half-life in seconds. */
constexpr long double k_half_life_seconds{987552.00L};

// =============================================================================
// =============================================================================

/*! \brief Absolute alpha yield per parent decay. */
constexpr long double k_alpha_yield{1.0082057L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 26U> k_alpha_energies_micro_eV{
  5'014'300'000'000ULL, 5'026'100'000'000ULL, 5'035'900'000'000ULL,
  5'056'500'000'000ULL, 5'086'000'000'000ULL, 5'112'500'000'000ULL,
  5'137'100'000'000ULL, 5'151'980'000'000ULL, 5'173'100'000'000ULL,
  5'211'100'000'000ULL, 5'237'120'000'000ULL, 5'259'140'000'000ULL,
  5'283'650'000'000ULL, 5'288'190'000'000ULL, 5'339'370'000'000ULL,
  5'366'370'000'000ULL, 5'432'830'000'000ULL, 5'434'600'000'000ULL,
  5'481'700'000'000ULL, 5'502'120'000'000ULL, 5'539'430'000'000ULL,
  5'606'990'000'000ULL, 5'715'840'000'000ULL, 5'747'140'000'000ULL,
  5'857'520'000'000ULL, 5'871'630'000'000ULL,
};

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 26U> k_alpha_line_yields{
  4.4e-06, 6.3e-06, 4e-06,   2e-06,   3e-06,  6e-06,  1.7e-05, 0.00021, 0.00026,
  5.3e-05, 0.00041, 0.00042, 0.00093, 0.0016, 0.0013, 0.0013,  0.005,   0.016,
  8e-05,   0.0074,  0.106,   0.258,   0.496,  0.1,    0.0032,  0.01,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute nuclear_gamma yield per parent decay. */
constexpr long double k_nuclear_gamma_yield{0.358478714L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 82U> k_nuclear_gamma_energies_micro_eV{
  4'470'000'000ULL,   9'900'000'000ULL,   14'370'000'000ULL,
  31'870'000'000ULL,  69'500'000'000ULL,  70'900'000'000ULL,
  102'200'000'000ULL, 103'200'000'000ULL, 104'040'000'000ULL,
  106'780'000'000ULL, 108'500'000'000ULL, 110'856'000'000ULL,
  114'700'000'000ULL, 122'319'000'000ULL, 131'600'000'000ULL,
  138'300'000'000ULL, 144'270'000'000ULL, 147'200'000'000ULL,
  154'208'000'000ULL, 158'635'000'000ULL, 165'800'000'000ULL,
  175'650'000'000ULL, 177'300'000'000ULL, 179'540'000'000ULL,
  199'300'000'000ULL, 221'320'000'000ULL, 247'200'000'000ULL,
  249'490'000'000ULL, 251'600'000'000ULL, 255'200'000'000ULL,
  255'700'000'000ULL, 260'400'000'000ULL, 269'463'000'000ULL,
  270'300'000'000ULL, 286'000'000'000ULL, 288'180'000'000ULL,
  323'871'000'000ULL, 328'380'000'000ULL, 334'010'000'000ULL,
  338'282'000'000ULL, 342'780'000'000ULL, 355'500'000'000ULL,
  355'700'000'000ULL, 361'890'000'000ULL, 362'900'000'000ULL,
  368'560'000'000ULL, 371'676'000'000ULL, 372'860'000'000ULL,
  376'260'000'000ULL, 383'350'000'000ULL, 387'700'000'000ULL,
  390'100'000'000ULL, 430'600'000'000ULL, 432'450'000'000ULL,
  445'033'000'000ULL, 487'500'000'000ULL, 490'800'000'000ULL,
  500'000'000'000ULL, 510'000'000'000ULL, 523'200'000'000ULL,
  527'611'000'000ULL, 532'900'000'000ULL, 537'600'000'000ULL,
  541'990'000'000ULL, 545'800'000'000ULL, 574'100'000'000ULL,
  579'600'000'000ULL, 584'300'000'000ULL, 594'000'000'000ULL,
  598'721'000'000ULL, 609'310'000'000ULL, 619'100'000'000ULL,
  623'680'000'000ULL, 631'700'000'000ULL, 641'700'000'000ULL,
  646'100'000'000ULL, 696'900'000'000ULL, 711'300'000'000ULL,
  718'400'000'000ULL, 728'400'000'000ULL, 732'800'000'000ULL,
  737'200'000'000ULL,
};

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 82U> k_nuclear_gamma_line_yields{
  6.4e-08,  0.000158, 0.000185, 1.05e-06, 7e-05,   3.6e-05,  8e-06,   6e-05,
  0.000194, 0.000233, 6e-05,    0.00058,  0.0001,  0.01238,  6e-05,   1.7e-05,
  0.0336,   6e-05,    0.0584,   0.00713,  5.4e-05, 0.00017,  0.00047, 0.00154,
  3e-05,    0.00036,  9.7e-05,  0.00038,  0.00055, 0.00048,  5.5e-05, 6.7e-05,
  0.1423,   7e-06,    1.1e-05,  0.00161,  0.0406,  0.00203,  0.001,   0.0285,
  0.00226,  4.3e-05,  2.8e-05,  0.00028,  0.00016, 9e-05,    0.00499, 0.00051,
  0.00013,  7e-05,    0.00016,  4.6e-05,  0.0002,  0.000356, 0.0128,  0.00011,
  1.7e-05,  1.4e-05,  4e-06,    1.4e-05,  0.00073, 1.4e-05,  2.1e-05, 1.4e-05,
  1.1e-05,  1.1e-05,  1.4e-05,  1.4e-05,  1.4e-05, 0.00092,  0.00057, 3.6e-05,
  9e-05,    4e-06,    1.7e-05,  4e-06,    7e-06,   3.7e-05,  1.4e-05, 2.8e-06,
  6e-06,    2.8e-06,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute atomic_xray yield per parent decay. */
constexpr long double k_atomic_xray_yield{0.7268L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 5U> k_atomic_xray_energies_micro_eV{
  13'697'500'000ULL, 81'070'000'000ULL, 83'780'000'000ULL,
  94'854'700'000ULL, 97'896'700'000ULL,
};

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 5U> k_atomic_xray_line_yields{
  0.221, 0.1486, 0.245, 0.085, 0.0272,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_4_47_keV yield per parent decay. */
constexpr long double k_conversion_4_47_keV_yield{0.3264L};

/*! \brief Selected line energy in canonical micro-eV. */
constexpr std::uint64_t k_conversion_4_47_keV_energy_micro_eV{860'000'000ULL};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_9_9_keV yield per parent decay. */
constexpr long double k_conversion_9_9_keV_yield{0.1561L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 2U> k_conversion_9_9_keV_energies_micro_eV{
  6'290'000'000ULL,
  9'277'000'000ULL,
};

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 2U> k_conversion_9_9_keV_line_yields{
  0.119,
  0.0371,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_14_37_keV yield per parent decay. */
constexpr long double k_conversion_14_37_keV_yield{0.0997L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 2U>
  k_conversion_14_37_keV_energies_micro_eV{
    10'760'000'000ULL,
    13'747'000'000ULL,
};

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 2U> k_conversion_14_37_keV_line_yields{
  0.076,
  0.0237,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_31_87_keV yield per parent decay. */
constexpr long double k_conversion_31_87_keV_yield{0.0021114L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 5U>
  k_conversion_31_87_keV_energies_micro_eV{
    13'822'000'000ULL, 14'542'000'000ULL, 17'260'000'000ULL,
    28'260'000'000ULL, 31'247'000'000ULL,
};

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 5U> k_conversion_31_87_keV_line_yields{
  2.14e-05, 0.00076, 0.00078, 0.00042, 0.00013,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_69_5_keV yield per parent decay. */
constexpr long double k_conversion_69_5_keV_yield{0.0005147L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 5U> k_conversion_69_5_keV_energies_micro_eV{
  51'450'000'000ULL, 52'170'000'000ULL, 54'890'000'000ULL,
  65'890'000'000ULL, 68'880'000'000ULL,
};

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 5U> k_conversion_69_5_keV_line_yields{
  0.00035, 3.9e-05, 2.7e-06, 9.3e-05, 3e-05,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_103_2_keV yield per parent decay. */
constexpr long double k_conversion_103_2_keV_yield{0.000581L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_103_2_keV_energies_micro_eV{
    4'800'000'000ULL,  85'150'000'000ULL, 85'870'000'000ULL,
    88'590'000'000ULL, 99'590'000'000ULL, 102'580'000'000ULL,
};

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_103_2_keV_line_yields{
  0.0003, 5e-05, 9e-05, 7e-05, 5.4e-05, 1.7e-05,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_104_04_keV yield per parent decay. */
constexpr long double k_conversion_104_04_keV_yield{0.001864L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_104_04_keV_energies_micro_eV{
    5'643'000'000ULL,  85'992'000'000ULL,  86'712'000'000ULL,
    89'430'000'000ULL, 100'430'000'000ULL, 103'417'000'000ULL,
};

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_104_04_keV_line_yields{
  0.001, 0.00016, 0.00029, 0.00019, 0.00017, 5.4e-05,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_106_78_keV yield per parent decay. */
constexpr long double k_conversion_106_78_keV_yield{0.00253209L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_106_78_keV_energies_micro_eV{
    8'383'000'000ULL,  88'732'000'000ULL,  89'452'000'000ULL,
    92'170'000'000ULL, 103'170'000'000ULL, 106'157'000'000ULL,
};

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_106_78_keV_line_yields{
  0.00204, 0.000335, 3.73e-05, 2.49e-06, 8.9e-05, 2.83e-05,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_110_856_keV yield per parent decay. */
constexpr long double k_conversion_110_856_keV_yield{0.0031136L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_110_856_keV_energies_micro_eV{
    12'459'000'000ULL, 92'808'000'000ULL,  93'528'000'000ULL,
    96'246'000'000ULL, 107'246'000'000ULL, 110'233'000'000ULL,
};

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_110_856_keV_line_yields{
  0.000211, 6.56e-05, 0.00121, 0.00087, 0.000577, 0.00018,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_122_319_keV yield per parent decay. */
constexpr long double k_conversion_122_319_keV_yield{0.090846L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_122_319_keV_energies_micro_eV{
    23'922'000'000ULL,  104'271'000'000ULL, 104'991'000'000ULL,
    107'709'000'000ULL, 118'709'000'000ULL, 121'696'000'000ULL,
};

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_122_319_keV_line_yields{
  0.0728, 0.01184, 0.0016, 0.000285, 0.00328, 0.001041,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_144_27_keV yield per parent decay. */
constexpr long double k_conversion_144_27_keV_yield{0.154198L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_144_27_keV_energies_micro_eV{
    45'873'000'000ULL,  126'222'000'000ULL, 126'942'000'000ULL,
    129'660'000'000ULL, 140'660'000'000ULL, 143'647'000'000ULL,
};

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_144_27_keV_line_yields{
  0.124, 0.0201, 0.00255, 0.000339, 0.00547, 0.001739,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_154_208_keV yield per parent decay. */
constexpr long double k_conversion_154_208_keV_yield{0.223528L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_154_208_keV_energies_micro_eV{
    55'811'000'000ULL,  136'160'000'000ULL, 136'880'000'000ULL,
    139'598'000'000ULL, 150'598'000'000ULL, 153'585'000'000ULL,
};

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_154_208_keV_line_yields{
  0.1805, 0.0293, 0.00328, 0.000208, 0.00777, 0.00247,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_158_635_keV yield per parent decay. */
constexpr long double k_conversion_158_635_keV_yield{0.024704L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_158_635_keV_energies_micro_eV{
    60'238'000'000ULL,  140'587'000'000ULL, 141'307'000'000ULL,
    144'025'000'000ULL, 155'025'000'000ULL, 158'012'000'000ULL,
};

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_158_635_keV_line_yields{
  0.0198, 0.0032, 0.00045, 8e-05, 0.000891, 0.000283,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_179_54_keV yield per parent decay. */
constexpr long double k_conversion_179_54_keV_yield{0.0032559L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_179_54_keV_energies_micro_eV{
    81'140'000'000ULL,  161'490'000'000ULL, 162'210'000'000ULL,
    164'930'000'000ULL, 175'930'000'000ULL, 178'920'000'000ULL,
};

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_179_54_keV_line_yields{
  0.00249, 0.000402, 0.000126, 5.1e-05, 0.000142, 4.49e-05,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_221_32_keV yield per parent decay. */
constexpr long double k_conversion_221_32_keV_yield{0.000024239L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_221_32_keV_energies_micro_eV{
    122'920'000'000ULL, 203'270'000'000ULL, 203'990'000'000ULL,
    206'710'000'000ULL, 217'710'000'000ULL, 220'700'000'000ULL,
};

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_221_32_keV_line_yields{
  1.95e-05, 2.43e-06, 6.6e-07, 5.2e-07, 8.6e-07, 2.69e-07,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_249_49_keV yield per parent decay. */
constexpr long double k_conversion_249_49_keV_yield{0.0002525L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_249_49_keV_energies_micro_eV{
    151'093'000'000ULL, 231'442'000'000ULL, 232'162'000'000ULL,
    234'880'000'000ULL, 245'880'000'000ULL, 248'867'000'000ULL,
};

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_249_49_keV_line_yields{
  0.00019, 2.7e-05, 1.4e-05, 6e-06, 1.18e-05, 3.7e-06,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_251_6_keV yield per parent decay. */
constexpr long double k_conversion_251_6_keV_yield{0.0003088L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_251_6_keV_energies_micro_eV{
    153'200'000'000ULL, 233'550'000'000ULL, 234'270'000'000ULL,
    236'990'000'000ULL, 247'990'000'000ULL, 250'980'000'000ULL,
};

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_251_6_keV_line_yields{
  0.00022, 3.9e-05, 2e-05, 8e-06, 1.65e-05, 5.3e-06,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_269_463_keV yield per parent decay. */
constexpr long double k_conversion_269_463_keV_yield{0.112209L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_269_463_keV_energies_micro_eV{
    171'066'000'000ULL, 251'415'000'000ULL, 252'135'000'000ULL,
    254'853'000'000ULL, 265'853'000'000ULL, 268'840'000'000ULL,
};

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_269_463_keV_line_yields{
  0.0906, 0.01454, 0.00176, 0.000157, 0.00391, 0.001242,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_288_18_keV yield per parent decay. */
constexpr long double k_conversion_288_18_keV_yield{0.000058633L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_288_18_keV_energies_micro_eV{
    189'783'000'000ULL, 270'132'000'000ULL, 270'852'000'000ULL,
    273'570'000'000ULL, 284'570'000'000ULL, 287'557'000'000ULL,
};

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_288_18_keV_line_yields{
  4.75e-05, 6.09e-06, 1.383e-06, 1.019e-06, 2.01e-06, 6.31e-07,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_323_871_keV yield per parent decay. */
constexpr long double k_conversion_323_871_keV_yield{0.019186L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_323_871_keV_energies_micro_eV{
    225'474'000'000ULL, 305'823'000'000ULL, 306'543'000'000ULL,
    309'261'000'000ULL, 320'261'000'000ULL, 323'248'000'000ULL,
};

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_323_871_keV_line_yields{
  0.0155, 0.00248, 0.0003, 2.8e-05, 0.000666, 0.000212,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_328_38_keV yield per parent decay. */
constexpr long double k_conversion_328_38_keV_yield{0.000055015L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_328_38_keV_energies_micro_eV{
    229'983'000'000ULL, 310'332'000'000ULL, 311'052'000'000ULL,
    313'770'000'000ULL, 324'770'000'000ULL, 327'757'000'000ULL,
};

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_328_38_keV_line_yields{
  4.47e-05, 5.81e-06, 1.2e-06, 8.61e-07, 1.86e-06, 5.84e-07,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_334_01_keV yield per parent decay. */
constexpr long double k_conversion_334_01_keV_yield{0.00010058L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_334_01_keV_energies_micro_eV{
    235'610'000'000ULL, 315'960'000'000ULL, 316'680'000'000ULL,
    319'400'000'000ULL, 330'400'000'000ULL, 333'390'000'000ULL,
};

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_334_01_keV_line_yields{
  5.46e-05, 8.7e-06, 1.8e-05, 7.57e-06, 8.9e-06, 2.81e-06,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_338_282_keV yield per parent decay. */
constexpr long double k_conversion_338_282_keV_yield{0.01224667L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_338_282_keV_energies_micro_eV{
    239'885'000'000ULL, 320'234'000'000ULL, 320'954'000'000ULL,
    323'672'000'000ULL, 334'672'000'000ULL, 337'659'000'000ULL,
};

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_338_282_keV_line_yields{
  0.00992, 0.001587, 0.0001761, 1.017e-05, 0.00042, 0.0001334,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_342_78_keV yield per parent decay. */
constexpr long double k_conversion_342_78_keV_yield{0.000055588L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_342_78_keV_energies_micro_eV{
    244'383'000'000ULL, 324'732'000'000ULL, 325'452'000'000ULL,
    328'170'000'000ULL, 339'170'000'000ULL, 342'157'000'000ULL,
};

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_342_78_keV_line_yields{
  4.52e-05, 5.9e-06, 1.19e-06, 8.4e-07, 1.87e-06, 5.88e-07,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_371_676_keV yield per parent decay. */
constexpr long double k_conversion_371_676_keV_yield{0.001662017L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_371_676_keV_energies_micro_eV{
    273'279'000'000ULL, 353'628'000'000ULL, 354'348'000'000ULL,
    357'066'000'000ULL, 368'066'000'000ULL, 371'053'000'000ULL,
};

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_371_676_keV_line_yields{
  0.001347, 0.000215, 2.38e-05, 1.357e-06, 5.68e-05, 1.806e-05,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_372_86_keV yield per parent decay. */
constexpr long double k_conversion_372_86_keV_yield{0.0000104340L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_372_86_keV_energies_micro_eV{
    274'460'000'000ULL, 354'810'000'000ULL, 355'530'000'000ULL,
    358'250'000'000ULL, 369'250'000'000ULL, 372'240'000'000ULL,
};

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_372_86_keV_line_yields{
  8.5e-06, 1.117e-06, 2.122e-07, 1.474e-07, 3.48e-07, 1.094e-07,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_445_033_keV yield per parent decay. */
constexpr long double k_conversion_445_033_keV_yield{0.00262479L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_445_033_keV_energies_micro_eV{
    346'636'000'000ULL, 426'985'000'000ULL, 427'705'000'000ULL,
    430'423'000'000ULL, 441'423'000'000ULL, 444'410'000'000ULL,
};

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_445_033_keV_line_yields{
  0.00213, 0.000338, 3.7e-05, 2.09e-06, 8.93e-05, 2.84e-05,
};

} // namespace

// =============================================================================
// =============================================================================

[[nodiscard]] auto BuildRa223Radionuclide() -> GGEMSRadionuclideDefinition {
  std::vector<GGEMSRadionuclideEmission> emissions;
  emissions.reserve(30U);

  emissions.emplace_back(particles::GGEMSParticleType::Alpha, k_alpha_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_alpha_energies_micro_eV, k_alpha_line_yields));

  emissions.emplace_back(
    particles::GGEMSParticleType::Gamma, k_nuclear_gamma_yield,
    sources::GGEMSEnergyDistribution::BuildDiscreteLines(
      k_nuclear_gamma_energies_micro_eV, k_nuclear_gamma_line_yields));

  emissions.emplace_back(
    particles::GGEMSParticleType::Gamma, k_atomic_xray_yield,
    sources::GGEMSEnergyDistribution::BuildDiscreteLines(
      k_atomic_xray_energies_micro_eV, k_atomic_xray_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_4_47_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildMono(
                           k_conversion_4_47_keV_energy_micro_eV));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_9_9_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_9_9_keV_energies_micro_eV,
                           k_conversion_9_9_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_14_37_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_14_37_keV_energies_micro_eV,
                           k_conversion_14_37_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_31_87_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_31_87_keV_energies_micro_eV,
                           k_conversion_31_87_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_69_5_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_69_5_keV_energies_micro_eV,
                           k_conversion_69_5_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_103_2_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_103_2_keV_energies_micro_eV,
                           k_conversion_103_2_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_104_04_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_104_04_keV_energies_micro_eV,
                           k_conversion_104_04_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_106_78_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_106_78_keV_energies_micro_eV,
                           k_conversion_106_78_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_110_856_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_110_856_keV_energies_micro_eV,
                           k_conversion_110_856_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_122_319_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_122_319_keV_energies_micro_eV,
                           k_conversion_122_319_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_144_27_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_144_27_keV_energies_micro_eV,
                           k_conversion_144_27_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_154_208_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_154_208_keV_energies_micro_eV,
                           k_conversion_154_208_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_158_635_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_158_635_keV_energies_micro_eV,
                           k_conversion_158_635_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_179_54_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_179_54_keV_energies_micro_eV,
                           k_conversion_179_54_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_221_32_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_221_32_keV_energies_micro_eV,
                           k_conversion_221_32_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_249_49_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_249_49_keV_energies_micro_eV,
                           k_conversion_249_49_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_251_6_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_251_6_keV_energies_micro_eV,
                           k_conversion_251_6_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_269_463_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_269_463_keV_energies_micro_eV,
                           k_conversion_269_463_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_288_18_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_288_18_keV_energies_micro_eV,
                           k_conversion_288_18_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_323_871_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_323_871_keV_energies_micro_eV,
                           k_conversion_323_871_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_328_38_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_328_38_keV_energies_micro_eV,
                           k_conversion_328_38_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_334_01_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_334_01_keV_energies_micro_eV,
                           k_conversion_334_01_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_338_282_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_338_282_keV_energies_micro_eV,
                           k_conversion_338_282_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_342_78_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_342_78_keV_energies_micro_eV,
                           k_conversion_342_78_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_371_676_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_371_676_keV_energies_micro_eV,
                           k_conversion_371_676_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_372_86_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_372_86_keV_energies_micro_eV,
                           k_conversion_372_86_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_445_033_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_445_033_keV_energies_micro_eV,
                           k_conversion_445_033_keV_line_yields));

  return {"Ra-223", k_half_life_seconds, std::move(emissions)};
}

} // namespace ggems::core::radioactivity::builtins
