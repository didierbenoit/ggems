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
 * \brief Defines the compiled Co-57 marginal source-emission laws.
 *
 * LNHB/DDEP, V. P. Chechev and N. K. Kuzmenko (KRI), updated August 2014;
 * tables 07/07/2014 - 1/3/2017; PenNuc 09/09/2014. 100% electron capture to
 * Fe-57.
 *
 * BetaShape 2.4 (06/2024) retained EC outputs audit capture probabilities
 * only; no continuous beta source law is selected.
 *
 * Physical yields are independent emissions per parent decay. LARA supplies
 * absolute gamma/X-ray yields; PenNuc supplies conversion electrons,
 * grouped by nuclear transition to preserve weak-line reachability. Auger
 * energy ranges do not define a selected energy law.
 *
 * Independent evidence and representation residuals:
 * validation/radioactivity/data/Co-57/reference/.
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

/*! \brief Evaluated Co-57 parent half-life in seconds. */
constexpr long double k_half_life_seconds{23484384.00L};

// =============================================================================
// =============================================================================

/*! \brief Nuclear gamma yield per parent decay. */
constexpr long double k_nuclear_gamma_yield{1.055677L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 10U> k_nuclear_gamma_energies_micro_eV{
  {
    14'412'950'000ULL,
    122'060'650'000ULL,
    136'473'560'000ULL,
    230'270'000'000ULL,
    339'670'000'000ULL,
    352'340'000'000ULL,
    366'740'000'000ULL,
    569'940'000'000ULL,
    692'010'000'000ULL,
    706'415'000'000ULL,
  },
};

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 10U> k_nuclear_gamma_line_yields{
  {
    0.0918,
    0.8549,
    0.1071,
    0.000004,
    0.000038,
    0.000032,
    0.000013,
    0.00015,
    0.00159,
    0.00005,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Atomic xray yield per parent decay. */
constexpr long double k_atomic_xray_yield{0.5885L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 4U> k_atomic_xray_energies_micro_eV{
  {
    705'400'000ULL,
    6'390'910'000ULL,
    6'403'910'000ULL,
    7'083'200'000ULL,
  },
};

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 4U> k_atomic_xray_line_yields{
  {
    0.013,
    0.1712,
    0.335,
    0.0693,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Conversion 14 41295 keV yield per parent decay. */
constexpr long double k_conversion_14_41295_keV_yield{0.784623L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_14_41295_keV_energies_micro_eV{
    {
      7'300'950'000ULL,
      13'566'850'000ULL,
      13'691'850'000ULL,
      13'704'850'000ULL,
      14'371'330'000ULL,
      14'412'550'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_14_41295_keV_line_yields{
  {
    0.702,
    0.0666,
    0.004,
    0.001662,
    0.00993,
    0.000431,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Conversion 122 06065 keV yield per parent decay. */
constexpr long double k_conversion_122_06065_keV_yield{0.02017715L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_122_06065_keV_energies_micro_eV{
    {
      114'948'790'000ULL,
      121'214'690'000ULL,
      121'339'690'000ULL,
      121'352'690'000ULL,
      122'019'170'000ULL,
      122'060'390'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_122_06065_keV_line_yields{
  {
    0.01812,
    0.001701,
    0.000056,
    0.0000412,
    0.0002479,
    0.00001105,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Conversion 136 47356 keV yield per parent decay. */
constexpr long double k_conversion_136_47356_keV_yield{0.01593619L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_136_47356_keV_energies_micro_eV{
    {
      129'361'740'000ULL,
      135'627'640'000ULL,
      135'752'640'000ULL,
      135'765'640'000ULL,
      136'432'120'000ULL,
      136'473'340'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_136_47356_keV_line_yields{
  {
    0.01426,
    0.001271,
    0.0000812,
    0.0001155,
    0.0002003,
    0.00000819,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Conversion 230 27 keV yield per parent decay. */
constexpr long double k_conversion_230_27_keV_yield{1.6648E-8L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_230_27_keV_energies_micro_eV{
    {
      223'158'000'000ULL,
      229'424'000'000ULL,
      229'549'000'000ULL,
      229'562'000'000ULL,
      230'228'000'000ULL,
      230'270'000'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_230_27_keV_line_yields{
  {
    0.000000015,
    0.0000000014,
    0.000000000026,
    0.000000000013,
    0.0000000002,
    0.000000000009,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Conversion 339 67 keV yield per parent decay. */
constexpr long double k_conversion_339_67_keV_yield{6.33053E-8L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_339_67_keV_energies_micro_eV{
    {
      332'558'000'000ULL,
      338'824'000'000ULL,
      338'949'000'000ULL,
      338'962'000'000ULL,
      339'628'000'000ULL,
      339'670'000'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_339_67_keV_line_yields{
  {
    0.000000057,
    0.0000000054,
    0.000000000077,
    0.0000000000436,
    0.00000000075,
    0.0000000000347,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Conversion 352 34 keV yield per parent decay. */
constexpr long double k_conversion_352_34_keV_yield{4.77934E-8L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_352_34_keV_energies_micro_eV{
    {
      345'228'000'000ULL,
      351'494'000'000ULL,
      351'619'000'000ULL,
      351'632'000'000ULL,
      352'298'000'000ULL,
      352'340'000'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_352_34_keV_line_yields{
  {
    0.000000043,
    0.0000000041,
    0.000000000056,
    0.0000000000309,
    0.00000000058,
    0.0000000000265,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Conversion 366 74 keV yield per parent decay. */
constexpr long double k_conversion_366_74_keV_yield{2.32566E-8L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_366_74_keV_energies_micro_eV{
    {
      359'628'000'000ULL,
      365'894'000'000ULL,
      366'019'000'000ULL,
      366'032'000'000ULL,
      366'698'000'000ULL,
      366'740'000'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_366_74_keV_line_yields{
  {
    0.000000021,
    0.0000000019,
    0.000000000034,
    0.00000000003,
    0.00000000028,
    0.0000000000126,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Conversion 569 94 keV yield per parent decay. */
constexpr long double k_conversion_569_94_keV_yield{7.6554E-8L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_569_94_keV_energies_micro_eV{
    {
      562'828'000'000ULL,
      569'094'000'000ULL,
      569'219'000'000ULL,
      569'232'000'000ULL,
      569'898'000'000ULL,
      569'940'000'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_569_94_keV_line_yields{
  {
    0.000000069,
    0.0000000065,
    0.000000000062,
    0.00000000004,
    0.00000000091,
    0.000000000042,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Conversion 692 01 keV yield per parent decay. */
constexpr long double k_conversion_692_01_keV_yield{5.80152E-7L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_692_01_keV_energies_micro_eV{
    {
      684'898'000'000ULL,
      691'164'000'000ULL,
      691'289'000'000ULL,
      691'302'000'000ULL,
      691'968'000'000ULL,
      692'010'000'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_692_01_keV_line_yields{
  {
    0.000000523,
    0.0000000491,
    0.000000000464,
    0.000000000388,
    0.00000000688,
    0.00000000032,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Conversion 706 415 keV yield per parent decay. */
constexpr long double k_conversion_706_415_keV_yield{2.41978E-8L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_706_415_keV_energies_micro_eV{
    {
      699'308'000'000ULL,
      705'574'000'000ULL,
      705'699'000'000ULL,
      705'712'000'000ULL,
      706'378'000'000ULL,
      706'420'000'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_706_415_keV_line_yields{
  {
    0.0000000218,
    0.00000000204,
    0.0000000000266,
    0.00000000003,
    0.000000000288,
    0.0000000000132,
  },
};

} // namespace

// =============================================================================
// =============================================================================

[[nodiscard]] auto BuildCo57Radionuclide() -> GGEMSRadionuclideDefinition {
  std::vector<GGEMSRadionuclideEmission> emissions;
  emissions.reserve(12U);

  emissions.emplace_back(
    particles::GGEMSParticleType::Gamma, k_nuclear_gamma_yield,
    sources::GGEMSEnergyDistribution::BuildDiscreteLines(
      k_nuclear_gamma_energies_micro_eV, k_nuclear_gamma_line_yields));

  emissions.emplace_back(
    particles::GGEMSParticleType::Gamma, k_atomic_xray_yield,
    sources::GGEMSEnergyDistribution::BuildDiscreteLines(
      k_atomic_xray_energies_micro_eV, k_atomic_xray_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_14_41295_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_14_41295_keV_energies_micro_eV,
                           k_conversion_14_41295_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_122_06065_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_122_06065_keV_energies_micro_eV,
                           k_conversion_122_06065_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_136_47356_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_136_47356_keV_energies_micro_eV,
                           k_conversion_136_47356_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_230_27_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_230_27_keV_energies_micro_eV,
                           k_conversion_230_27_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_339_67_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_339_67_keV_energies_micro_eV,
                           k_conversion_339_67_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_352_34_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_352_34_keV_energies_micro_eV,
                           k_conversion_352_34_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_366_74_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_366_74_keV_energies_micro_eV,
                           k_conversion_366_74_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_569_94_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_569_94_keV_energies_micro_eV,
                           k_conversion_569_94_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_692_01_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_692_01_keV_energies_micro_eV,
                           k_conversion_692_01_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_706_415_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_706_415_keV_energies_micro_eV,
                           k_conversion_706_415_keV_line_yields));

  return {"Co-57", k_half_life_seconds, std::move(emissions)};
}

} // namespace ggems::core::radioactivity::builtins
