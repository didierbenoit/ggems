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
 * \brief Defines the compiled Ac-225 marginal source-emission laws.
 *
 * LNHB retained evaluations select independent emissions per parent decay.
 * Prompt photon and conversion yields are included where separable;
 * unsupported Auger energy laws, neutrinos and recoil are excluded.
 *
 * Direct parent alpha lines use evaluated discrete energies and yields.
 * No radioactive daughter-chain emissions are added.
 *
 * Selection and exclusions: validation/radioactivity/data/Ac-225/reference/.
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
constexpr long double k_half_life_seconds{856846.0800L};

// =============================================================================
// =============================================================================

/*! \brief Absolute alpha yield per parent decay. */
constexpr long double k_alpha_yield{0.99956647L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 49U> k_alpha_energies_micro_eV{
  4'903'600'000'000ULL, 4'992'700'000'000ULL, 5'019'300'000'000ULL,
  5'025'500'000'000ULL, 5'035'500'000'000ULL, 5'064'100'000'000ULL,
  5'076'800'000'000ULL, 5'094'100'000'000ULL, 5'129'000'000'000ULL,
  5'162'100'000'000ULL, 5'195'100'000'000ULL, 5'203'300'000'000ULL,
  5'210'200'000'000ULL, 5'239'300'000'000ULL, 5'269'100'000'000ULL,
  5'287'600'000'000ULL, 5'321'200'000'000ULL, 5'341'900'000'000ULL,
  5'356'200'000'000ULL, 5'379'000'000'000ULL, 5'391'200'000'000ULL,
  5'414'500'000'000ULL, 5'428'300'000'000ULL, 5'430'100'000'000ULL,
  5'435'800'000'000ULL, 5'443'300'000'000ULL, 5'468'400'000'000ULL,
  5'487'400'000'000ULL, 5'497'400'000'000ULL, 5'515'200'000'000ULL,
  5'523'700'000'000ULL, 5'540'100'000'000ULL, 5'546'500'000'000ULL,
  5'555'300'000'000ULL, 5'563'300'000'000ULL, 5'580'500'000'000ULL,
  5'599'300'000'000ULL, 5'609'000'000'000ULL, 5'637'300'000'000ULL,
  5'682'200'000'000ULL, 5'686'400'000'000ULL, 5'723'100'000'000ULL,
  5'730'500'000'000ULL, 5'731'600'000'000ULL, 5'731'900'000'000ULL,
  5'791'700'000'000ULL, 5'793'100'000'000ULL, 5'804'200'000'000ULL,
  5'829'600'000'000ULL,
};

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 49U> k_alpha_line_yields{
  1.1e-05, 1.3e-05, 1.5e-06, 8.3e-06, 2.1e-05,  1.14e-05, 3.8e-05,
  0.00015, 5.8e-05, 6.6e-06, 1.5e-06, 0.000101, 0.00022,  2.6e-05,
  0.00048, 0.00214, 7e-05,   2.7e-05, 9.7e-07,  2e-05,    6e-06,
  3e-05,   2.3e-05, 2.8e-05, 8.3e-05, 0.00098,  5.2e-06,  2e-05,
  2.2e-05, 5.2e-05, 0.00013, 7.2e-05, 0.00055,  0.00084,  0.00017,
  0.0095,  0.00114, 0.0109,  0.0416,  0.0131,   0.00021,  0.0203,
  0.016,   0.0124,  0.09,    0.062,   0.189,    0.003,    0.524,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute nuclear_gamma yield per parent decay. */
constexpr long double k_nuclear_gamma_yield{0.07457414L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 140U> k_nuclear_gamma_energies_micro_eV{
  10'790'000'000ULL,  25'856'000'000ULL,  36'646'000'000ULL,
  38'546'000'000ULL,  46'159'000'000ULL,  49'166'000'000ULL,
  50'311'000'000ULL,  53'036'000'000ULL,  57'762'000'000ULL,
  62'351'000'000ULL,  62'950'000'000ULL,  63'106'000'000ULL,
  64'251'000'000ULL,  69'858'000'000ULL,  71'758'000'000ULL,
  73'740'000'000ULL,  73'896'000'000ULL,  75'041'000'000ULL,
  78'812'000'000ULL,  87'385'000'000ULL,  94'892'000'000ULL,
  96'037'000'000ULL,  99'596'000'000ULL,  99'752'000'000ULL,
  100'897'000'000ULL, 103'488'000'000ULL, 108'404'000'000ULL,
  111'517'000'000ULL, 112'780'000'000ULL, 114'091'000'000ULL,
  119'899'000'000ULL, 121'080'000'000ULL, 123'670'000'000ULL,
  124'815'000'000ULL, 126'066'000'000ULL, 129'146'000'000ULL,
  133'573'000'000ULL, 134'874'000'000ULL, 137'400'000'000ULL,
  139'749'000'000ULL, 144'627'000'000ULL, 145'147'000'000ULL,
  150'063'000'000ULL, 152'654'000'000ULL, 153'955'000'000ULL,
  157'243'000'000ULL, 161'350'000'000ULL, 168'733'000'000ULL,
  169'933'000'000ULL, 170'805'000'000ULL, 178'312'000'000ULL,
  179'756'000'000ULL, 186'021'000'000ULL, 186'286'000'000ULL,
  187'263'000'000ULL, 187'921'000'000ULL, 195'789'000'000ULL,
  197'511'000'000ULL, 197'824'000'000ULL, 198'711'000'000ULL,
  205'190'000'000ULL, 216'905'000'000ULL, 220'430'000'000ULL,
  224'567'000'000ULL, 227'695'000'000ULL, 231'196'000'000ULL,
  234'490'000'000ULL, 238'640'000'000ULL, 240'663'000'000ULL,
  243'237'000'000ULL, 249'614'000'000ULL, 253'551'000'000ULL,
  256'144'000'000ULL, 279'209'000'000ULL, 282'201'000'000ULL,
  284'896'000'000ULL, 298'330'000'000ULL, 317'119'000'000ULL,
  321'753'000'000ULL, 348'350'000'000ULL, 354'754'000'000ULL,
  356'023'000'000ULL, 362'394'000'000ULL, 367'740'000'000ULL,
  374'881'000'000ULL, 388'100'000'000ULL, 403'130'000'000ULL,
  406'057'000'000ULL, 417'882'000'000ULL, 429'430'000'000ULL,
  434'762'000'000ULL, 442'100'000'000ULL, 443'408'000'000ULL,
  443'430'000'000ULL, 446'310'000'000ULL, 450'915'000'000ULL,
  452'216'000'000ULL, 458'740'000'000ULL, 462'266'000'000ULL,
  469'773'000'000ULL, 480'988'000'000ULL, 491'778'000'000ULL,
  496'500'000'000ULL, 498'600'000'000ULL, 513'265'000'000ULL,
  515'165'000'000ULL, 517'633'000'000ULL, 522'146'000'000ULL,
  525'955'000'000ULL, 529'653'000'000ULL, 530'954'000'000ULL,
  532'123'000'000ULL, 538'003'000'000ULL, 544'850'000'000ULL,
  551'811'000'000ULL, 564'293'000'000ULL, 567'480'000'000ULL,
  570'669'000'000ULL, 592'004'000'000ULL, 593'904'000'000ULL,
  600'939'000'000ULL, 600'953'000'000ULL, 603'074'000'000ULL,
  629'260'000'000ULL, 637'599'000'000ULL, 645'940'000'000ULL,
  649'077'000'000ULL, 656'290'000'000ULL, 658'030'000'000ULL,
  666'840'000'000ULL, 674'940'000'000ULL, 679'530'000'000ULL,
  702'020'000'000ULL, 747'000'000'000ULL, 752'480'000'000ULL,
  753'460'000'000ULL, 766'440'000'000ULL, 779'320'000'000ULL,
  808'480'000'000ULL, 825'000'000'000ULL,
};

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 140U> k_nuclear_gamma_line_yields{
  0.00015,  1.59e-05, 0.000181, 0.000107, 4.9e-05,  8e-05,    6.2e-06,
  4e-05,    5.1e-05,  5.3e-05,  0.0049,   0.00021,  0.00047,  4.7e-05,
  0.000132, 0.00019,  0.00309,  0.00015,  0.000123, 0.00271,  0.00105,
  0.00033,  0.0076,   0.0108,   0.00096,  3e-05,    0.00255,  0.00313,
  2.1e-05,  8.7e-06,  0.0008,   0.00017,  0.00087,  0.000292, 7.9e-05,
  2.7e-05,  0.000196, 0.00032,  2.3e-05,  1.39e-05, 4.6e-06,  0.00146,
  0.00693,  0.000197, 0.00205,  0.0036,   3.6e-05,  0.00012,  0.000139,
  0.00013,  0.000161, 0.000108, 0.000127, 4.2e-05,  0.000103, 0.0053,
  0.00148,  0.00026,  0.00038,  0.000188, 1.5e-05,  0.0032,   6e-05,
  0.00112,  4.6e-05,  5e-05,    1.7e-05,  1e-05,    0.000117, 3.1e-05,
  0.000135, 0.00132,  3.7e-06,  0.000305, 5.5e-06,  7.4e-05,  2e-05,
  4.2e-06,  3.3e-05,  3e-05,    2e-05,    2.6e-06,  5.4e-05,  5.2e-06,
  1.9e-05,  1.25e-05, 1.9e-06,  7.8e-05,  5.6e-05,  3.8e-06,  2.9e-05,
  4.5e-05,  1.4e-05,  1e-06,    6e-06,    3e-05,    0.00107,  5.3e-06,
  4.4e-06,  2.8e-05,  0.00034,  3.5e-06,  1.5e-05,  8.3e-06,  5.5e-06,
  0.000214, 0.000159, 2.08e-05, 0.000353, 7.6e-05,  4.7e-05,  7.6e-06,
  3.8e-05,  5.3e-06,  5.2e-05,  2.2e-06,  1.2e-05,  4e-05,    8.3e-06,
  2.9e-05,  2.4e-05,  6e-05,    1.73e-05, 3.2e-06,  1.2e-06,  1.5e-06,
  1.7e-05,  4.9e-06,  1.4e-05,  2.1e-05,  1e-06,    6.6e-06,  1.6e-06,
  1.1e-05,  2.6e-06,  2.3e-06,  3e-06,    5.5e-07,  2.1e-05,  4.9e-07,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute atomic_xray yield per parent decay. */
constexpr long double k_atomic_xray_yield{0.22586L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 5U> k_atomic_xray_energies_micro_eV{
  14'089'550'000ULL, 83'230'000'000ULL,  86'100'000'000ULL,
  97'452'700'000ULL, 100'560'000'000ULL,
};

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 5U> k_atomic_xray_line_yields{
  0.19, 0.0106, 0.0173, 0.006, 0.00196,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_10_79_keV yield per parent decay. */
constexpr long double k_conversion_10_79_keV_yield{0.0715L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 2U>
  k_conversion_10_79_keV_energies_micro_eV{
    7'042'000'000ULL,
    10'125'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 2U> k_conversion_10_79_keV_line_yields{
  0.054,
  0.0175,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_25_856_keV yield per parent decay. */
constexpr long double k_conversion_25_856_keV_yield{0.0971L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 5U>
  k_conversion_25_856_keV_energies_micro_eV{
    7'222'000'000ULL,  7'957'000'000ULL,  10'831'000'000ULL,
    22'108'000'000ULL, 25'191'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 5U> k_conversion_25_856_keV_line_yields{
  0.001, 0.0345, 0.0363, 0.0192, 0.0061,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_36_646_keV yield per parent decay. */
constexpr long double k_conversion_36_646_keV_yield{0.1990L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 5U>
  k_conversion_36_646_keV_energies_micro_eV{
    18'012'000'000ULL, 18'747'000'000ULL, 21'621'000'000ULL,
    32'898'000'000ULL, 35'981'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 5U> k_conversion_36_646_keV_line_yields{
  0.0021, 0.073, 0.072, 0.0395, 0.0124,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_38_546_keV yield per parent decay. */
constexpr long double k_conversion_38_546_keV_yield{0.09167L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 5U>
  k_conversion_38_546_keV_energies_micro_eV{
    19'912'000'000ULL, 20'647'000'000ULL, 23'521'000'000ULL,
    34'798'000'000ULL, 37'881'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 5U> k_conversion_38_546_keV_line_yields{
  0.00097, 0.0338, 0.033, 0.0182, 0.0057,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_46_159_keV yield per parent decay. */
constexpr long double k_conversion_46_159_keV_yield{0.00004136L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 5U>
  k_conversion_46_159_keV_energies_micro_eV{
    27'525'000'000ULL, 28'260'000'000ULL, 31'134'000'000ULL,
    42'411'000'000ULL, 45'494'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 5U> k_conversion_46_159_keV_line_yields{
  1.05e-05, 9.7e-06, 1.11e-05, 7.7e-06, 2.36e-06,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_49_166_keV yield per parent decay. */
constexpr long double k_conversion_49_166_keV_yield{0.00005695L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 5U>
  k_conversion_49_166_keV_energies_micro_eV{
    30'532'000'000ULL, 31'267'000'000ULL, 34'141'000'000ULL,
    45'418'000'000ULL, 48'501'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 5U> k_conversion_49_166_keV_line_yields{
  1.52e-05, 1.32e-05, 1.48e-05, 1.05e-05, 3.25e-06,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_50_311_keV yield per parent decay. */
constexpr long double k_conversion_50_311_keV_yield{0.00144774L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 5U>
  k_conversion_50_311_keV_energies_micro_eV{
    31'677'000'000ULL, 32'412'000'000ULL, 35'286'000'000ULL,
    46'563'000'000ULL, 49'646'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 5U> k_conversion_50_311_keV_line_yields{
  1.544e-05, 0.00055, 0.000503, 0.0002883, 9.1e-05,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_53_036_keV yield per parent decay. */
constexpr long double k_conversion_53_036_keV_yield{0.00071513L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 5U>
  k_conversion_53_036_keV_energies_micro_eV{
    34'402'000'000ULL, 35'137'000'000ULL, 38'011'000'000ULL,
    49'288'000'000ULL, 52'371'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 5U> k_conversion_53_036_keV_line_yields{
  0.000486, 5.42e-05, 3.73e-06, 0.0001296, 4.16e-05,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_57_762_keV yield per parent decay. */
constexpr long double k_conversion_57_762_keV_yield{0.00002355L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 5U>
  k_conversion_57_762_keV_energies_micro_eV{
    39'128'000'000ULL, 39'863'000'000ULL, 42'737'000'000ULL,
    54'014'000'000ULL, 57'097'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 5U> k_conversion_57_762_keV_line_yields{
  6.9e-06, 5.3e-06, 5.7e-06, 4.3e-06, 1.35e-06,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_62_351_keV yield per parent decay. */
constexpr long double k_conversion_62_351_keV_yield{0.00438L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 5U>
  k_conversion_62_351_keV_energies_micro_eV{
    43'717'000'000ULL, 44'452'000'000ULL, 47'326'000'000ULL,
    58'603'000'000ULL, 61'686'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 5U> k_conversion_62_351_keV_line_yields{
  5e-05, 0.00171, 0.00147, 0.00087, 0.00028,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_62_95_keV yield per parent decay. */
constexpr long double k_conversion_62_95_keV_yield{0.052992L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 5U>
  k_conversion_62_95_keV_energies_micro_eV{
    44'316'000'000ULL, 45'051'000'000ULL, 47'925'000'000ULL,
    59'202'000'000ULL, 62'285'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 5U> k_conversion_62_95_keV_line_yields{
  0.036, 0.00403, 0.000272, 0.0096, 0.00309,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_63_106_keV yield per parent decay. */
constexpr long double k_conversion_63_106_keV_yield{0.0000768L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 5U>
  k_conversion_63_106_keV_energies_micro_eV{
    44'472'000'000ULL, 45'207'000'000ULL, 48'081'000'000ULL,
    59'358'000'000ULL, 62'441'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 5U> k_conversion_63_106_keV_line_yields{
  2.38e-05, 1.68e-05, 1.77e-05, 1.41e-05, 4.4e-06,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_64_251_keV yield per parent decay. */
constexpr long double k_conversion_64_251_keV_yield{0.01081L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 5U>
  k_conversion_64_251_keV_energies_micro_eV{
    45'617'000'000ULL, 46'352'000'000ULL, 49'226'000'000ULL,
    60'503'000'000ULL, 63'586'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 5U> k_conversion_64_251_keV_line_yields{
  0.00268, 0.003, 0.0024, 0.00207, 0.00066,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_69_858_keV yield per parent decay. */
constexpr long double k_conversion_69_858_keV_yield{0.002249L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 5U>
  k_conversion_69_858_keV_energies_micro_eV{
    51'224'000'000ULL, 51'959'000'000ULL, 54'833'000'000ULL,
    66'110'000'000ULL, 69'193'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 5U> k_conversion_69_858_keV_line_yields{
  2.7e-05, 0.00089, 0.00074, 0.00045, 0.000142,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_71_758_keV yield per parent decay. */
constexpr long double k_conversion_71_758_keV_yield{0.005559L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 5U>
  k_conversion_71_758_keV_energies_micro_eV{
    53'124'000'000ULL, 53'859'000'000ULL, 56'733'000'000ULL,
    68'010'000'000ULL, 71'093'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 5U> k_conversion_71_758_keV_line_yields{
  6.9e-05, 0.00221, 0.00182, 0.00111, 0.00035,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_73_74_keV yield per parent decay. */
constexpr long double k_conversion_73_74_keV_yield{0.007029L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 5U>
  k_conversion_73_74_keV_energies_micro_eV{
    55'106'000'000ULL, 55'841'000'000ULL, 58'715'000'000ULL,
    69'992'000'000ULL, 73'075'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 5U> k_conversion_73_74_keV_line_yields{
  8.9e-05, 0.0028, 0.0023, 0.0014, 0.00044,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_73_896_keV yield per parent decay. */
constexpr long double k_conversion_73_896_keV_yield{0.0007392L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 5U>
  k_conversion_73_896_keV_energies_micro_eV{
    55'262'000'000ULL, 55'997'000'000ULL, 58'871'000'000ULL,
    70'148'000'000ULL, 73'231'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 5U> k_conversion_73_896_keV_line_yields{
  0.000249, 0.000155, 0.000157, 0.000136, 4.22e-05,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_75_041_keV yield per parent decay. */
constexpr long double k_conversion_75_041_keV_yield{0.00180L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 5U>
  k_conversion_75_041_keV_energies_micro_eV{
    56'407'000'000ULL, 57'142'000'000ULL, 60'016'000'000ULL,
    71'293'000'000ULL, 74'376'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 5U> k_conversion_75_041_keV_line_yields{
  0.00054, 0.00047, 0.00034, 0.00034, 0.00011,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_78_812_keV yield per parent decay. */
constexpr long double k_conversion_78_812_keV_yield{0.0006915L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 5U>
  k_conversion_78_812_keV_energies_micro_eV{
    60'178'000'000ULL, 60'913'000'000ULL, 63'787'000'000ULL,
    75'064'000'000ULL, 78'147'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 5U> k_conversion_78_812_keV_line_yields{
  0.00047, 5.3e-05, 3.5e-06, 0.000125, 4e-05,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_87_385_keV yield per parent decay. */
constexpr long double k_conversion_87_385_keV_yield{0.0113118L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 5U>
  k_conversion_87_385_keV_energies_micro_eV{
    68'751'000'000ULL, 69'486'000'000ULL, 72'360'000'000ULL,
    83'637'000'000ULL, 86'720'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 5U> k_conversion_87_385_keV_line_yields{
  0.0077, 0.00086, 5.58e-05, 0.00204, 0.000656,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_94_892_keV yield per parent decay. */
constexpr long double k_conversion_94_892_keV_yield{0.0034309L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 5U>
  k_conversion_94_892_keV_energies_micro_eV{
    76'258'000'000ULL, 76'993'000'000ULL, 79'867'000'000ULL,
    91'144'000'000ULL, 94'227'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 5U> k_conversion_94_892_keV_line_yields{
  0.00233, 0.000264, 1.69e-05, 0.00062, 0.0002,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_96_037_keV yield per parent decay. */
constexpr long double k_conversion_96_037_keV_yield{0.002004L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 5U>
  k_conversion_96_037_keV_energies_micro_eV{
    77'403'000'000ULL, 78'138'000'000ULL, 81'012'000'000ULL,
    92'289'000'000ULL, 95'372'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 5U> k_conversion_96_037_keV_line_yields{
  0.00046, 0.00059, 0.00043, 0.0004, 0.000124,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_99_596_keV yield per parent decay. */
constexpr long double k_conversion_99_596_keV_yield{0.02324L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 5U>
  k_conversion_99_596_keV_energies_micro_eV{
    80'962'000'000ULL, 81'697'000'000ULL, 84'571'000'000ULL,
    95'848'000'000ULL, 98'931'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 5U> k_conversion_99_596_keV_line_yields{
  0.0142, 0.00258, 0.00084, 0.00426, 0.00136,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_99_752_keV yield per parent decay. */
constexpr long double k_conversion_99_752_keV_yield{0.001161L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 5U>
  k_conversion_99_752_keV_energies_micro_eV{
    81'118'000'000ULL, 81'853'000'000ULL, 84'727'000'000ULL,
    96'004'000'000ULL, 99'087'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 5U> k_conversion_99_752_keV_line_yields{
  0.00045, 0.000222, 0.00021, 0.000213, 6.6e-05,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_100_897_keV yield per parent decay. */
constexpr long double k_conversion_100_897_keV_yield{0.00433L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 5U>
  k_conversion_100_897_keV_energies_micro_eV{
    82'263'000'000ULL, 82'998'000'000ULL,  85'872'000'000ULL,
    97'149'000'000ULL, 100'232'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 5U> k_conversion_100_897_keV_line_yields{
  0.0012, 0.0012, 0.0008, 0.00086, 0.00027,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_103_488_keV yield per parent decay. */
constexpr long double k_conversion_103_488_keV_yield{0.0003093L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_103_488_keV_energies_micro_eV{
    2'358'000'000ULL,  84'854'000'000ULL, 85'589'000'000ULL,
    88'463'000'000ULL, 99'740'000'000ULL, 102'823'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_103_488_keV_line_yields{
  0.00016, 2.8e-05, 4.9e-05, 3.4e-05, 2.9e-05, 9.3e-06,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_108_404_keV yield per parent decay. */
constexpr long double k_conversion_108_404_keV_yield{0.026191L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_108_404_keV_energies_micro_eV{
    7'274'000'000ULL,  89'770'000'000ULL,  90'505'000'000ULL,
    93'379'000'000ULL, 104'656'000'000ULL, 107'739'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_108_404_keV_line_yields{
  0.0184, 0.00309, 0.00173, 0.00102, 0.00148, 0.000471,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_111_517_keV yield per parent decay. */
constexpr long double k_conversion_111_517_keV_yield{0.0011309L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_111_517_keV_energies_micro_eV{
    10'387'000'000ULL, 92'883'000'000ULL,  93'618'000'000ULL,
    96'492'000'000ULL, 107'769'000'000ULL, 110'852'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_111_517_keV_line_yields{
  0.00088, 0.000102, 4.64e-05, 4.24e-05, 4.57e-05, 1.44e-05,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_112_78_keV yield per parent decay. */
constexpr long double k_conversion_112_78_keV_yield{0.000007436L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_112_78_keV_energies_micro_eV{
    11'650'000'000ULL, 94'150'000'000ULL,  94'880'000'000ULL,
    97'760'000'000ULL, 109'030'000'000ULL, 112'120'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_112_78_keV_line_yields{
  5.8e-06, 6.7e-07, 3.01e-07, 2.74e-07, 2.98e-07, 9.3e-08,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_114_091_keV yield per parent decay. */
constexpr long double k_conversion_114_091_keV_yield{0.000085781L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_114_091_keV_energies_micro_eV{
    12'961'000'000ULL, 95'457'000'000ULL,  96'192'000'000ULL,
    99'066'000'000ULL, 110'343'000'000ULL, 113'426'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_114_091_keV_line_yields{
  6.9e-05, 1.14e-05, 1.29e-06, 8.1e-08, 3.04e-06, 9.7e-07,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_119_899_keV yield per parent decay. */
constexpr long double k_conversion_119_899_keV_yield{0.00024293L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_119_899_keV_energies_micro_eV{
    18'769'000'000ULL,  101'265'000'000ULL, 102'000'000'000ULL,
    104'874'000'000ULL, 116'151'000'000ULL, 119'234'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_119_899_keV_line_yields{
  0.00019, 2.21e-05, 9.6e-06, 8.6e-06, 9.6e-06, 3.03e-06,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_121_08_keV yield per parent decay. */
constexpr long double k_conversion_121_08_keV_yield{0.00005103L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_121_08_keV_energies_micro_eV{
    19'950'000'000ULL,  102'450'000'000ULL, 103'180'000'000ULL,
    106'060'000'000ULL, 117'330'000'000ULL, 120'420'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_121_08_keV_line_yields{
  4e-05, 4.6e-06, 2e-06, 1.8e-06, 2e-06, 6.3e-07,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_123_67_keV yield per parent decay. */
constexpr long double k_conversion_123_67_keV_yield{0.00024603L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_123_67_keV_energies_micro_eV{
    22'540'000'000ULL,  105'036'000'000ULL, 105'771'000'000ULL,
    108'645'000'000ULL, 119'922'000'000ULL, 123'005'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_123_67_keV_line_yields{
  0.000193, 2.24e-05, 9.5e-06, 8.4e-06, 9.7e-06, 3.03e-06,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_124_815_keV yield per parent decay. */
constexpr long double k_conversion_124_815_keV_yield{0.001753L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_124_815_keV_energies_micro_eV{
    23'685'000'000ULL,  106'181'000'000ULL, 106'916'000'000ULL,
    109'790'000'000ULL, 121'067'000'000ULL, 124'150'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_124_815_keV_line_yields{
  0.00113, 0.00019, 0.000172, 0.000104, 0.000119, 3.8e-05,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_126_066_keV yield per parent decay. */
constexpr long double k_conversion_126_066_keV_yield{0.00002056272L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_126_066_keV_energies_micro_eV{
    24'936'000'000ULL,  107'432'000'000ULL, 108'167'000'000ULL,
    111'041'000'000ULL, 122'318'000'000ULL, 125'401'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_126_066_keV_line_yields{
  1.67e-05, 1.95e-06, 8.1e-07, 7.2e-10, 8.4e-07, 2.62e-07,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_129_146_keV yield per parent decay. */
constexpr long double k_conversion_129_146_keV_yield{0.0001344L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_129_146_keV_energies_micro_eV{
    28'016'000'000ULL,  110'512'000'000ULL, 111'247'000'000ULL,
    114'121'000'000ULL, 125'398'000'000ULL, 128'481'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_129_146_keV_line_yields{
  8e-05, 1.34e-05, 1.68e-05, 1.03e-05, 1.05e-05, 3.4e-06,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_133_573_keV yield per parent decay. */
constexpr long double k_conversion_133_573_keV_yield{0.00004289318L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_133_573_keV_energies_micro_eV{
    32'443'000'000ULL,  114'939'000'000ULL, 115'674'000'000ULL,
    118'548'000'000ULL, 129'825'000'000ULL, 132'908'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_133_573_keV_line_yields{
  3.63e-05, 4.25e-06, 1.7e-09, 1.48e-09, 1.78e-06, 5.6e-07,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_134_874_keV yield per parent decay. */
constexpr long double k_conversion_134_874_keV_yield{0.00006852504L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_134_874_keV_energies_micro_eV{
    33'744'000'000ULL,  116'240'000'000ULL, 116'975'000'000ULL,
    119'849'000'000ULL, 131'126'000'000ULL, 134'209'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_134_874_keV_line_yields{
  5.8e-05, 6.8e-06, 2.69e-09, 2.35e-09, 2.83e-06, 8.9e-07,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_139_749_keV yield per parent decay. */
constexpr long double k_conversion_139_749_keV_yield{0.00005436L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_139_749_keV_energies_micro_eV{
    38'619'000'000ULL,  121'115'000'000ULL, 121'850'000'000ULL,
    124'724'000'000ULL, 136'001'000'000ULL, 139'084'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_139_749_keV_line_yields{
  3.3e-05, 6e-06, 6e-06, 4.2e-06, 3.9e-06, 1.26e-06,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_144_627_keV yield per parent decay. */
constexpr long double k_conversion_144_627_keV_yield{0.00001746L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_144_627_keV_energies_micro_eV{
    43'497'000'000ULL,  125'993'000'000ULL, 126'728'000'000ULL,
    129'602'000'000ULL, 140'879'000'000ULL, 143'962'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_144_627_keV_line_yields{
  1.18e-05, 2e-06, 1.45e-06, 8e-07, 1.07e-06, 3.4e-07,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_145_147_keV yield per parent decay. */
constexpr long double k_conversion_145_147_keV_yield{0.0002611684L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_145_147_keV_energies_micro_eV{
    44'017'000'000ULL,  126'513'000'000ULL, 127'248'000'000ULL,
    130'122'000'000ULL, 141'399'000'000ULL, 144'482'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_145_147_keV_line_yields{
  0.000221, 2.61e-05, 9.9e-09, 8.5e-09, 1.07e-05, 3.35e-06,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_150_063_keV yield per parent decay. */
constexpr long double k_conversion_150_063_keV_yield{0.0010302417L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_150_063_keV_energies_micro_eV{
    48'933'000'000ULL,  131'429'000'000ULL, 132'164'000'000ULL,
    135'038'000'000ULL, 146'315'000'000ULL, 149'398'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_150_063_keV_line_yields{
  0.000968, 1.153e-06, 4.26e-08, 3.61e-08, 4.64e-05, 1.461e-05,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_152_654_keV yield per parent decay. */
constexpr long double k_conversion_152_654_keV_yield{0.00003120912L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_152_654_keV_energies_micro_eV{
    51'524'000'000ULL,  134'020'000'000ULL, 134'755'000'000ULL,
    137'629'000'000ULL, 148'906'000'000ULL, 151'989'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_152_654_keV_line_yields{
  2.64e-05, 3.15e-06, 1.15e-09, 9.7e-10, 1.26e-06, 3.97e-07,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_153_955_keV yield per parent decay. */
constexpr long double k_conversion_153_955_keV_yield{0.0003180615L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_153_955_keV_energies_micro_eV{
    52'825'000'000ULL,  135'321'000'000ULL, 136'056'000'000ULL,
    138'930'000'000ULL, 150'207'000'000ULL, 153'290'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_153_955_keV_line_yields{
  0.000269, 3.22e-05, 1.17e-08, 9.8e-09, 1.28e-05, 4.04e-06,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_157_243_keV yield per parent decay. */
constexpr long double k_conversion_157_243_keV_yield{0.014035L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_157_243_keV_energies_micro_eV{
    56'113'000'000ULL,  138'609'000'000ULL, 139'344'000'000ULL,
    142'218'000'000ULL, 153'495'000'000ULL, 156'578'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_157_243_keV_line_yields{
  0.0112, 0.0018, 0.00029, 7e-05, 0.00051, 0.000165,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_161_35_keV yield per parent decay. */
constexpr long double k_conversion_161_35_keV_yield{0.00008867L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_161_35_keV_energies_micro_eV{
    60'220'000'000ULL,  142'720'000'000ULL, 143'450'000'000ULL,
    146'320'000'000ULL, 157'600'000'000ULL, 160'690'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_161_35_keV_line_yields{
  5.8e-05, 9.6e-06, 8.6e-06, 4.7e-06, 5.9e-06, 1.87e-06,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_168_733_keV yield per parent decay. */
constexpr long double k_conversion_168_733_keV_yield{0.0002573L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_168_733_keV_energies_micro_eV{
    67'603'000'000ULL,  150'099'000'000ULL, 150'834'000'000ULL,
    153'708'000'000ULL, 164'985'000'000ULL, 168'068'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_168_733_keV_line_yields{
  0.00017, 2.8e-05, 2.4e-05, 1.3e-05, 1.7e-05, 5.3e-06,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_170_805_keV yield per parent decay. */
constexpr long double k_conversion_170_805_keV_yield{0.00001542099L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_170_805_keV_energies_micro_eV{
    69'675'000'000ULL,  152'171'000'000ULL, 152'906'000'000ULL,
    155'780'000'000ULL, 167'057'000'000ULL, 170'140'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_170_805_keV_line_yields{
  1.3e-05, 1.6e-06, 5.4e-10, 4.5e-10, 6.2e-07, 2e-07,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_178_312_keV yield per parent decay. */
constexpr long double k_conversion_178_312_keV_yield{0.000017619077L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_178_312_keV_energies_micro_eV{
    77'182'000'000ULL,  159'678'000'000ULL, 160'413'000'000ULL,
    163'287'000'000ULL, 174'564'000'000ULL, 177'647'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_178_312_keV_line_yields{
  1.49e-05, 1.81e-06, 5.94e-10, 4.83e-10, 6.9e-07, 2.18e-07,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_179_756_keV yield per parent decay. */
constexpr long double k_conversion_179_756_keV_yield{0.00019044L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_179_756_keV_energies_micro_eV{
    78'626'000'000ULL,  161'122'000'000ULL, 161'857'000'000ULL,
    164'731'000'000ULL, 176'008'000'000ULL, 179'091'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_179_756_keV_line_yields{
  0.000129, 2.12e-05, 1.65e-05, 8.3e-06, 1.17e-05, 3.74e-06,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_186_286_keV yield per parent decay. */
constexpr long double k_conversion_186_286_keV_yield{0.000004142246L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_186_286_keV_energies_micro_eV{
    85'156'000'000ULL,  167'652'000'000ULL, 168'387'000'000ULL,
    171'261'000'000ULL, 182'538'000'000ULL, 185'621'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_186_286_keV_line_yields{
  3.5e-06, 4.3e-07, 1.36e-10, 1.1e-10, 1.61e-07, 5.1e-08,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_187_921_keV yield per parent decay. */
constexpr long double k_conversion_187_921_keV_yield{0.0005123102L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_187_921_keV_energies_micro_eV{
    86'791'000'000ULL,  169'287'000'000ULL, 170'022'000'000ULL,
    172'896'000'000ULL, 184'173'000'000ULL, 187'256'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_187_921_keV_line_yields{
  0.000433, 5.31e-05, 1.67e-08, 1.35e-08, 1.99e-05, 6.28e-06,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_195_789_keV yield per parent decay. */
constexpr long double k_conversion_195_789_keV_yield{0.0022141L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_195_789_keV_energies_micro_eV{
    94'659'000'000ULL,  177'155'000'000ULL, 177'890'000'000ULL,
    180'764'000'000ULL, 192'041'000'000ULL, 195'124'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_195_789_keV_line_yields{
  0.0016, 0.00027, 0.00013, 6e-05, 0.000117, 3.71e-05,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_197_511_keV yield per parent decay. */
constexpr long double k_conversion_197_511_keV_yield{0.00002003460L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_197_511_keV_energies_micro_eV{
    96'381'000'000ULL,  178'877'000'000ULL, 179'612'000'000ULL,
    182'486'000'000ULL, 193'763'000'000ULL, 196'846'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_197_511_keV_line_yields{
  1.89e-05, 2.33e-09, 7.1e-10, 5.6e-10, 8.6e-07, 2.71e-07,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_197_824_keV yield per parent decay. */
constexpr long double k_conversion_197_824_keV_yield{0.00002915524L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_197_824_keV_energies_micro_eV{
    96'694'000'000ULL,  179'190'000'000ULL, 179'925'000'000ULL,
    182'799'000'000ULL, 194'076'000'000ULL, 197'159'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_197_824_keV_line_yields{
  2.75e-05, 3.39e-09, 1.03e-09, 8.2e-10, 1.25e-06, 4e-07,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_198_711_keV yield per parent decay. */
constexpr long double k_conversion_198_711_keV_yield{0.000014308563L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_198_711_keV_energies_micro_eV{
    97'581'000'000ULL,  180'077'000'000ULL, 180'812'000'000ULL,
    183'686'000'000ULL, 194'963'000'000ULL, 198'046'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_198_711_keV_line_yields{
  1.35e-05, 1.66e-09, 5.04e-10, 3.99e-10, 6.13e-07, 1.93e-07,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_216_905_keV yield per parent decay. */
constexpr long double k_conversion_216_905_keV_yield{0.00019707511L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_216_905_keV_energies_micro_eV{
    115'775'000'000ULL, 198'271'000'000ULL, 199'006'000'000ULL,
    201'880'000'000ULL, 213'157'000'000ULL, 216'240'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_216_905_keV_line_yields{
  0.000186, 2.33e-08, 6.66e-09, 5.15e-09, 8.4e-06, 2.64e-06,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_224_567_keV yield per parent decay. */
constexpr long double k_conversion_224_567_keV_yield{0.00006364123L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_224_567_keV_energies_micro_eV{
    123'437'000'000ULL, 205'933'000'000ULL, 206'668'000'000ULL,
    209'542'000'000ULL, 220'819'000'000ULL, 223'902'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_224_567_keV_line_yields{
  6.01e-05, 7.5e-09, 2.11e-09, 1.62e-09, 2.68e-06, 8.5e-07,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_231_196_keV yield per parent decay. */
constexpr long double k_conversion_231_196_keV_yield{0.000067050057L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_231_196_keV_energies_micro_eV{
    130'066'000'000ULL, 212'562'000'000ULL, 213'297'000'000ULL,
    216'171'000'000ULL, 227'448'000'000ULL, 230'531'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_231_196_keV_line_yields{
  5.4e-05, 9e-06, 1e-06, 5.7e-11, 2.3e-06, 7.5e-07,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_238_64_keV yield per parent decay. */
constexpr long double k_conversion_238_64_keV_yield{0.0000122470104L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_238_64_keV_energies_micro_eV{
    137'510'000'000ULL, 220'010'000'000ULL, 220'740'000'000ULL,
    223'610'000'000ULL, 234'890'000'000ULL, 237'980'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_238_64_keV_line_yields{
  9.9e-06, 1.6e-06, 1.8e-07, 1.04e-11, 4.3e-07, 1.37e-07,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_240_663_keV yield per parent decay. */
constexpr long double k_conversion_240_663_keV_yield{0.000005661996L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_240_663_keV_energies_micro_eV{
    139'533'000'000ULL, 222'029'000'000ULL, 222'764'000'000ULL,
    225'638'000'000ULL, 236'915'000'000ULL, 239'998'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_240_663_keV_line_yields{
  5.35e-06, 6.8e-10, 1.8e-10, 1.36e-10, 2.36e-07, 7.5e-08,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_243_237_keV yield per parent decay. */
constexpr long double k_conversion_243_237_keV_yield{0.00003590000305L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_243_237_keV_energies_micro_eV{
    142'107'000'000ULL, 224'603'000'000ULL, 225'338'000'000ULL,
    228'212'000'000ULL, 239'489'000'000ULL, 242'572'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_243_237_keV_line_yields{
  2.9e-05, 4.7e-06, 5.4e-07, 3.05e-12, 1.26e-06, 4e-07,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_249_614_keV yield per parent decay. */
constexpr long double k_conversion_249_614_keV_yield{0.00003475L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_249_614_keV_energies_micro_eV{
    148'484'000'000ULL, 230'980'000'000ULL, 231'715'000'000ULL,
    234'589'000'000ULL, 245'866'000'000ULL, 248'949'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_249_614_keV_line_yields{
  1.39e-05, 2.34e-06, 8.8e-06, 4.28e-06, 4.12e-06, 1.31e-06,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_253_551_keV yield per parent decay. */
constexpr long double k_conversion_253_551_keV_yield{0.00005658968L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_253_551_keV_energies_micro_eV{
    152'421'000'000ULL, 234'917'000'000ULL, 235'652'000'000ULL,
    238'526'000'000ULL, 249'803'000'000ULL, 252'886'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_253_551_keV_line_yields{
  5.35e-05, 6.8e-09, 1.75e-09, 1.3e-10, 2.34e-06, 7.41e-07,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_256_144_keV yield per parent decay. */
constexpr long double k_conversion_256_144_keV_yield{1.5545375E-7L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_256_144_keV_energies_micro_eV{
    155'014'000'000ULL, 237'510'000'000ULL, 238'245'000'000ULL,
    241'119'000'000ULL, 252'396'000'000ULL, 255'479'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_256_144_keV_line_yields{
  1.47e-07, 1.86e-11, 4.8e-12, 3.5e-13, 6.4e-09, 2.03e-09,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_279_209_keV yield per parent decay. */
constexpr long double k_conversion_279_209_keV_yield{0.0000104665993L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_279_209_keV_energies_micro_eV{
    178'079'000'000ULL, 260'575'000'000ULL, 261'310'000'000ULL,
    264'184'000'000ULL, 275'461'000'000ULL, 278'544'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_279_209_keV_line_yields{
  9.9e-06, 1.27e-09, 3.07e-10, 2.23e-11, 4.29e-07, 1.36e-07,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_282_201_keV yield per parent decay. */
constexpr long double k_conversion_282_201_keV_yield{0.000004227400351L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_282_201_keV_energies_micro_eV{
    181'071'000'000ULL, 263'567'000'000ULL, 264'302'000'000ULL,
    267'176'000'000ULL, 278'453'000'000ULL, 281'536'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_282_201_keV_line_yields{
  3.42e-06, 5.5e-07, 6.3e-08, 3.51e-13, 1.47e-07, 4.74e-08,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_284_896_keV yield per parent decay. */
constexpr long double k_conversion_284_896_keV_yield{0.00000243070808L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_284_896_keV_energies_micro_eV{
    183'766'000'000ULL, 266'262'000'000ULL, 266'997'000'000ULL,
    269'871'000'000ULL, 281'148'000'000ULL, 284'231'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_284_896_keV_line_yields{
  2.3e-06, 2.96e-10, 7e-12, 5.08e-12, 9.9e-08, 3.14e-08,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_298_33_keV yield per parent decay. */
constexpr long double k_conversion_298_33_keV_yield{0.000008034L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_298_33_keV_energies_micro_eV{
    197'200'000'000ULL, 279'700'000'000ULL, 280'430'000'000ULL,
    283'300'000'000ULL, 294'580'000'000ULL, 297'670'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_298_33_keV_line_yields{
  6e-06, 9.8e-07, 4.1e-07, 1.43e-07, 3.8e-07, 1.21e-07,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_317_119_keV yield per parent decay. */
constexpr long double k_conversion_317_119_keV_yield{0.00000231703519L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_317_119_keV_energies_micro_eV{
    215'989'000'000ULL, 298'485'000'000ULL, 299'220'000'000ULL,
    302'094'000'000ULL, 313'371'000'000ULL, 316'454'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_317_119_keV_line_yields{
  1.9e-06, 3.1e-07, 3.5e-11, 1.9e-13, 8.1e-08, 2.6e-08,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_321_753_keV yield per parent decay. */
constexpr long double k_conversion_321_753_keV_yield{8.2370578E-7L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_321_753_keV_energies_micro_eV{
    220'623'000'000ULL, 303'119'000'000ULL, 303'854'000'000ULL,
    306'728'000'000ULL, 318'005'000'000ULL, 321'088'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_321_753_keV_line_yields{
  7.8e-07, 1.02e-10, 2.22e-12, 1.56e-12, 3.31e-08, 1.05e-08,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_354_754_keV yield per parent decay. */
constexpr long double k_conversion_354_754_keV_yield{4.0115172E-7L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_354_754_keV_energies_micro_eV{
    253'624'000'000ULL, 336'120'000'000ULL, 336'855'000'000ULL,
    339'729'000'000ULL, 351'006'000'000ULL, 354'089'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_354_754_keV_line_yields{
  3.8e-07, 5e-11, 1.02e-12, 7e-13, 1.6e-08, 5.1e-09,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_362_394_keV yield per parent decay. */
constexpr long double k_conversion_362_394_keV_yield{0.00000103423439L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_362_394_keV_energies_micro_eV{
    261'264'000'000ULL, 343'760'000'000ULL, 344'495'000'000ULL,
    347'369'000'000ULL, 358'646'000'000ULL, 361'729'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_362_394_keV_line_yields{
  9.8e-07, 1.3e-10, 2.61e-12, 1.78e-12, 4.11e-08, 1.3e-08,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_367_74_keV yield per parent decay. */
constexpr long double k_conversion_367_74_keV_yield{1.767E-7L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_367_74_keV_energies_micro_eV{
    266'610'000'000ULL, 349'110'000'000ULL, 349'840'000'000ULL,
    352'720'000'000ULL, 363'990'000'000ULL, 367'080'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_367_74_keV_line_yields{
  1.4e-07, 2.2e-08, 3.6e-09, 2.2e-09, 6.8e-09, 2.1e-09,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_374_881_keV yield per parent decay. */
constexpr long double k_conversion_374_881_keV_yield{3.3764440E-7L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_374_881_keV_energies_micro_eV{
    273'751'000'000ULL, 356'247'000'000ULL, 356'982'000'000ULL,
    359'856'000'000ULL, 371'133'000'000ULL, 374'216'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_374_881_keV_line_yields{
  3.2e-07, 4.3e-11, 8.3e-13, 5.7e-13, 1.34e-08, 4.2e-09,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_406_057_keV yield per parent decay. */
constexpr long double k_conversion_406_057_keV_yield{0.00000118065258L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_406_057_keV_energies_micro_eV{
    304'927'000'000ULL, 387'423'000'000ULL, 388'158'000'000ULL,
    391'032'000'000ULL, 402'309'000'000ULL, 405'392'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_406_057_keV_line_yields{
  1.12e-06, 1.48e-10, 2.75e-12, 1.83e-12, 4.59e-08, 1.46e-08,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_443_408_keV yield per parent decay. */
constexpr long double k_conversion_443_408_keV_yield{4.95192E-7L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_443_408_keV_energies_micro_eV{
    342'278'000'000ULL, 424'774'000'000ULL, 425'509'000'000ULL,
    428'383'000'000ULL, 439'660'000'000ULL, 442'743'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_443_408_keV_line_yields{
  4.3e-07, 7e-11, 9e-11, 3.2e-11, 4.9e-08, 1.6e-08,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_450_915_keV yield per parent decay. */
constexpr long double k_conversion_450_915_keV_yield{0.0000063350945L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_450_915_keV_energies_micro_eV{
    349'785'000'000ULL, 432'281'000'000ULL, 433'016'000'000ULL,
    435'890'000'000ULL, 447'167'000'000ULL, 450'250'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_450_915_keV_line_yields{
  5.2e-06, 8.4e-07, 9.4e-11, 5e-13, 2.23e-07, 7.2e-08,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_452_216_keV yield per parent decay. */
constexpr long double k_conversion_452_216_keV_yield{0.0002250333376L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_452_216_keV_energies_micro_eV{
    351'086'000'000ULL, 433'582'000'000ULL, 434'317'000'000ULL,
    437'191'000'000ULL, 448'468'000'000ULL, 451'551'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_452_216_keV_line_yields{
  0.000185, 2.96e-05, 3.32e-09, 1.76e-11, 7.9e-06, 2.53e-06,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_462_266_keV yield per parent decay. */
constexpr long double k_conversion_462_266_keV_yield{5.0566579E-8L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_462_266_keV_energies_micro_eV{
    361'136'000'000ULL, 443'632'000'000ULL, 444'367'000'000ULL,
    447'241'000'000ULL, 458'518'000'000ULL, 461'601'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_462_266_keV_line_yields{
  4.8e-08, 6.4e-12, 1.09e-13, 7e-14, 1.94e-09, 6.2e-10,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_515_165_keV yield per parent decay. */
constexpr long double k_conversion_515_165_keV_yield{0.00003173646444L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_515_165_keV_energies_micro_eV{
    414'036'000'000ULL, 496'532'000'000ULL, 497'267'000'000ULL,
    500'141'000'000ULL, 511'418'000'000ULL, 514'501'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_515_165_keV_line_yields{
  2.61e-05, 4.17e-06, 4.62e-10, 2.44e-12, 1.11e-06, 3.56e-07,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_525_955_keV yield per parent decay. */
constexpr long double k_conversion_525_955_keV_yield{0.0000495157238L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_525_955_keV_energies_micro_eV{
    424'826'000'000ULL, 507'322'000'000ULL, 508'057'000'000ULL,
    510'931'000'000ULL, 522'208'000'000ULL, 525'291'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_525_955_keV_line_yields{
  4.07e-05, 6.53e-06, 7.2e-10, 3.8e-12, 1.73e-06, 5.55e-07,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_532_123_keV yield per parent decay. */
constexpr long double k_conversion_532_123_keV_yield{6.6298710E-8L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_532_123_keV_energies_micro_eV{
    430'994'000'000ULL, 513'490'000'000ULL, 514'225'000'000ULL,
    517'099'000'000ULL, 528'376'000'000ULL, 531'459'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_532_123_keV_line_yields{
  6.3e-08, 8.5e-12, 1.29e-13, 8.1e-14, 2.5e-09, 7.9e-10,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_551_811_keV yield per parent decay. */
constexpr long double k_conversion_551_811_keV_yield{0.000006432093049L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_551_811_keV_energies_micro_eV{
    450'682'000'000ULL, 533'178'000'000ULL, 533'913'000'000ULL,
    536'787'000'000ULL, 548'064'000'000ULL, 551'147'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_551_811_keV_line_yields{
  5.3e-06, 8.4e-07, 9.3e-11, 4.9e-14, 2.2e-07, 7.2e-08,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_570_669_keV yield per parent decay. */
constexpr long double k_conversion_570_669_keV_yield{3.019044949E-7L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_570_669_keV_energies_micro_eV{
    469'540'000'000ULL, 552'036'000'000ULL, 552'771'000'000ULL,
    555'645'000'000ULL, 566'922'000'000ULL, 570'005'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_570_669_keV_line_yields{
  2.87e-07, 3.89e-12, 5.7e-13, 3.49e-14, 1.13e-08, 3.6e-09,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_637_599_keV yield per parent decay. */
constexpr long double k_conversion_637_599_keV_yield{1.0976E-8L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_637_599_keV_energies_micro_eV{
    536'470'000'000ULL, 618'966'000'000ULL, 619'701'000'000ULL,
    622'575'000'000ULL, 633'852'000'000ULL, 636'935'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_637_599_keV_line_yields{
  8.9e-09, 1.32e-09, 1.8e-10, 8.6e-11, 3.7e-10, 1.2e-10,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_645_94_keV yield per parent decay. */
constexpr long double k_conversion_645_94_keV_yield{1.3334E-8L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_645_94_keV_energies_micro_eV{
    544'810'000'000ULL, 627'310'000'000ULL, 628'040'000'000ULL,
    630'920'000'000ULL, 642'190'000'000ULL, 645'280'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_645_94_keV_line_yields{
  1.08e-08, 1.6e-09, 2.2e-10, 1.04e-10, 4.6e-10, 1.5e-10,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_766_44_keV yield per parent decay. */
constexpr long double k_conversion_766_44_keV_yield{1.8726E-8L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_766_44_keV_energies_micro_eV{
    665'310'000'000ULL, 747'810'000'000ULL, 748'540'000'000ULL,
    751'420'000'000ULL, 762'690'000'000ULL, 765'780'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_766_44_keV_line_yields{
  1.53e-08, 2.2e-09, 2.7e-10, 1.26e-10, 6.3e-10, 2e-10,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_779_32_keV yield per parent decay. */
constexpr long double k_conversion_779_32_keV_yield{3.315E-9L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_779_32_keV_energies_micro_eV{
    678'190'000'000ULL, 760'690'000'000ULL, 761'420'000'000ULL,
    764'300'000'000ULL, 775'570'000'000ULL, 778'660'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_779_32_keV_line_yields{
  2.7e-09, 4e-10, 4.8e-11, 2.2e-11, 1.1e-10, 3.5e-11,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_808_48_keV yield per parent decay. */
constexpr long double k_conversion_808_48_keV_yield{1.1540E-7L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_808_48_keV_energies_micro_eV{
    707'350'000'000ULL, 789'850'000'000ULL, 790'580'000'000ULL,
    793'460'000'000ULL, 804'730'000'000ULL, 807'820'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_808_48_keV_line_yields{
  9.4e-08, 1.39e-08, 1.7e-09, 7.6e-10, 3.8e-09, 1.24e-09,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_825_keV yield per parent decay. */
constexpr long double k_conversion_825_keV_yield{2.6398E-9L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U> k_conversion_825_keV_energies_micro_eV{
  723'870'000'000ULL, 806'370'000'000ULL, 807'100'000'000ULL,
  809'980'000'000ULL, 821'250'000'000ULL, 824'340'000'000ULL,
};

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_825_keV_line_yields{
  2.16e-09, 3.1e-10, 3.7e-11, 1.68e-11, 8.8e-11, 2.8e-11,
};

} // namespace

// =============================================================================
// =============================================================================

[[nodiscard]] auto BuildAc225Radionuclide() -> GGEMSRadionuclideDefinition {
  std::vector<GGEMSRadionuclideEmission> emissions;
  emissions.reserve(94U);

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
                         k_conversion_10_79_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_10_79_keV_energies_micro_eV,
                           k_conversion_10_79_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_25_856_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_25_856_keV_energies_micro_eV,
                           k_conversion_25_856_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_36_646_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_36_646_keV_energies_micro_eV,
                           k_conversion_36_646_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_38_546_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_38_546_keV_energies_micro_eV,
                           k_conversion_38_546_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_46_159_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_46_159_keV_energies_micro_eV,
                           k_conversion_46_159_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_49_166_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_49_166_keV_energies_micro_eV,
                           k_conversion_49_166_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_50_311_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_50_311_keV_energies_micro_eV,
                           k_conversion_50_311_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_53_036_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_53_036_keV_energies_micro_eV,
                           k_conversion_53_036_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_57_762_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_57_762_keV_energies_micro_eV,
                           k_conversion_57_762_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_62_351_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_62_351_keV_energies_micro_eV,
                           k_conversion_62_351_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_62_95_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_62_95_keV_energies_micro_eV,
                           k_conversion_62_95_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_63_106_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_63_106_keV_energies_micro_eV,
                           k_conversion_63_106_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_64_251_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_64_251_keV_energies_micro_eV,
                           k_conversion_64_251_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_69_858_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_69_858_keV_energies_micro_eV,
                           k_conversion_69_858_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_71_758_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_71_758_keV_energies_micro_eV,
                           k_conversion_71_758_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_73_74_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_73_74_keV_energies_micro_eV,
                           k_conversion_73_74_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_73_896_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_73_896_keV_energies_micro_eV,
                           k_conversion_73_896_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_75_041_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_75_041_keV_energies_micro_eV,
                           k_conversion_75_041_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_78_812_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_78_812_keV_energies_micro_eV,
                           k_conversion_78_812_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_87_385_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_87_385_keV_energies_micro_eV,
                           k_conversion_87_385_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_94_892_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_94_892_keV_energies_micro_eV,
                           k_conversion_94_892_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_96_037_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_96_037_keV_energies_micro_eV,
                           k_conversion_96_037_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_99_596_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_99_596_keV_energies_micro_eV,
                           k_conversion_99_596_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_99_752_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_99_752_keV_energies_micro_eV,
                           k_conversion_99_752_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_100_897_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_100_897_keV_energies_micro_eV,
                           k_conversion_100_897_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_103_488_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_103_488_keV_energies_micro_eV,
                           k_conversion_103_488_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_108_404_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_108_404_keV_energies_micro_eV,
                           k_conversion_108_404_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_111_517_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_111_517_keV_energies_micro_eV,
                           k_conversion_111_517_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_112_78_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_112_78_keV_energies_micro_eV,
                           k_conversion_112_78_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_114_091_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_114_091_keV_energies_micro_eV,
                           k_conversion_114_091_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_119_899_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_119_899_keV_energies_micro_eV,
                           k_conversion_119_899_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_121_08_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_121_08_keV_energies_micro_eV,
                           k_conversion_121_08_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_123_67_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_123_67_keV_energies_micro_eV,
                           k_conversion_123_67_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_124_815_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_124_815_keV_energies_micro_eV,
                           k_conversion_124_815_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_126_066_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_126_066_keV_energies_micro_eV,
                           k_conversion_126_066_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_129_146_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_129_146_keV_energies_micro_eV,
                           k_conversion_129_146_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_133_573_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_133_573_keV_energies_micro_eV,
                           k_conversion_133_573_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_134_874_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_134_874_keV_energies_micro_eV,
                           k_conversion_134_874_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_139_749_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_139_749_keV_energies_micro_eV,
                           k_conversion_139_749_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_144_627_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_144_627_keV_energies_micro_eV,
                           k_conversion_144_627_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_145_147_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_145_147_keV_energies_micro_eV,
                           k_conversion_145_147_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_150_063_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_150_063_keV_energies_micro_eV,
                           k_conversion_150_063_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_152_654_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_152_654_keV_energies_micro_eV,
                           k_conversion_152_654_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_153_955_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_153_955_keV_energies_micro_eV,
                           k_conversion_153_955_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_157_243_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_157_243_keV_energies_micro_eV,
                           k_conversion_157_243_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_161_35_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_161_35_keV_energies_micro_eV,
                           k_conversion_161_35_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_168_733_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_168_733_keV_energies_micro_eV,
                           k_conversion_168_733_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_170_805_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_170_805_keV_energies_micro_eV,
                           k_conversion_170_805_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_178_312_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_178_312_keV_energies_micro_eV,
                           k_conversion_178_312_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_179_756_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_179_756_keV_energies_micro_eV,
                           k_conversion_179_756_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_186_286_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_186_286_keV_energies_micro_eV,
                           k_conversion_186_286_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_187_921_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_187_921_keV_energies_micro_eV,
                           k_conversion_187_921_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_195_789_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_195_789_keV_energies_micro_eV,
                           k_conversion_195_789_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_197_511_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_197_511_keV_energies_micro_eV,
                           k_conversion_197_511_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_197_824_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_197_824_keV_energies_micro_eV,
                           k_conversion_197_824_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_198_711_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_198_711_keV_energies_micro_eV,
                           k_conversion_198_711_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_216_905_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_216_905_keV_energies_micro_eV,
                           k_conversion_216_905_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_224_567_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_224_567_keV_energies_micro_eV,
                           k_conversion_224_567_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_231_196_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_231_196_keV_energies_micro_eV,
                           k_conversion_231_196_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_238_64_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_238_64_keV_energies_micro_eV,
                           k_conversion_238_64_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_240_663_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_240_663_keV_energies_micro_eV,
                           k_conversion_240_663_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_243_237_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_243_237_keV_energies_micro_eV,
                           k_conversion_243_237_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_249_614_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_249_614_keV_energies_micro_eV,
                           k_conversion_249_614_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_253_551_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_253_551_keV_energies_micro_eV,
                           k_conversion_253_551_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_256_144_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_256_144_keV_energies_micro_eV,
                           k_conversion_256_144_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_279_209_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_279_209_keV_energies_micro_eV,
                           k_conversion_279_209_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_282_201_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_282_201_keV_energies_micro_eV,
                           k_conversion_282_201_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_284_896_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_284_896_keV_energies_micro_eV,
                           k_conversion_284_896_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_298_33_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_298_33_keV_energies_micro_eV,
                           k_conversion_298_33_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_317_119_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_317_119_keV_energies_micro_eV,
                           k_conversion_317_119_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_321_753_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_321_753_keV_energies_micro_eV,
                           k_conversion_321_753_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_354_754_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_354_754_keV_energies_micro_eV,
                           k_conversion_354_754_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_362_394_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_362_394_keV_energies_micro_eV,
                           k_conversion_362_394_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_367_74_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_367_74_keV_energies_micro_eV,
                           k_conversion_367_74_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_374_881_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_374_881_keV_energies_micro_eV,
                           k_conversion_374_881_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_406_057_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_406_057_keV_energies_micro_eV,
                           k_conversion_406_057_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_443_408_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_443_408_keV_energies_micro_eV,
                           k_conversion_443_408_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_450_915_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_450_915_keV_energies_micro_eV,
                           k_conversion_450_915_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_452_216_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_452_216_keV_energies_micro_eV,
                           k_conversion_452_216_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_462_266_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_462_266_keV_energies_micro_eV,
                           k_conversion_462_266_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_515_165_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_515_165_keV_energies_micro_eV,
                           k_conversion_515_165_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_525_955_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_525_955_keV_energies_micro_eV,
                           k_conversion_525_955_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_532_123_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_532_123_keV_energies_micro_eV,
                           k_conversion_532_123_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_551_811_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_551_811_keV_energies_micro_eV,
                           k_conversion_551_811_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_570_669_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_570_669_keV_energies_micro_eV,
                           k_conversion_570_669_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_637_599_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_637_599_keV_energies_micro_eV,
                           k_conversion_637_599_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_645_94_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_645_94_keV_energies_micro_eV,
                           k_conversion_645_94_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_766_44_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_766_44_keV_energies_micro_eV,
                           k_conversion_766_44_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_779_32_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_779_32_keV_energies_micro_eV,
                           k_conversion_779_32_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_808_48_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_808_48_keV_energies_micro_eV,
                           k_conversion_808_48_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_825_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_825_keV_energies_micro_eV,
                           k_conversion_825_keV_line_yields));

  return {"Ac-225", k_half_life_seconds, std::move(emissions)};
}

} // namespace ggems::core::radioactivity::builtins
