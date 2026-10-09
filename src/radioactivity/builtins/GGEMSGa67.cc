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
 * \brief Defines the compiled Ga-67 marginal source-emission laws.
 *
 * LNHB/DDEP, X. Mougeot and V. P. Chechev (LNHB/KRI), revised March 2011;
 * tables 20/09/2000 - 9/9/2011; PenNuc 14/04/2011. 100% electron capture to
 * Zn-67.
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
 * validation/radioactivity/data/Ga-67/reference/.
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

/*! \brief Evaluated Ga-67 parent half-life in seconds. */
constexpr long double k_half_life_seconds{281776.3200L};

// =============================================================================
// =============================================================================

/*! \brief Nuclear gamma yield per parent decay. */
constexpr long double k_nuclear_gamma_yield{0.859899L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 10U> k_nuclear_gamma_energies_micro_eV{
  {
    91'263'000'000ULL,
    93'307'000'000ULL,
    184'577'000'000ULL,
    208'939'000'000ULL,
    300'232'000'000ULL,
    393'528'000'000ULL,
    494'143'000'000ULL,
    703'110'000'000ULL,
    794'400'000'000ULL,
    887'676'000'000ULL,
  },
};

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 10U> k_nuclear_gamma_line_yields{
  {
    0.0309,
    0.381,
    0.2096,
    0.0237,
    0.166,
    0.0459,
    0.000666,
    0.000113,
    0.000528,
    0.001492,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Atomic xray yield per parent decay. */
constexpr long double k_atomic_xray_yield{0.5883L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 4U> k_atomic_xray_energies_micro_eV{
  {
    1'034'850'000ULL,
    8'615'870'000ULL,
    8'638'960'000ULL,
    9'611'000'000ULL,
  },
};

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 4U> k_atomic_xray_line_yields{
  {
    0.0175,
    0.17,
    0.33,
    0.0708,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Conversion 91 263 keV yield per parent decay. */
constexpr long double k_conversion_91_263_keV_yield{0.00280875L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_91_263_keV_energies_micro_eV{
    {
      81'604'000'000ULL,
      90'069'000'000ULL,
      90'220'000'000ULL,
      90'243'000'000ULL,
      91'198'000'000ULL,
      91'262'000'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_91_263_keV_line_yields{
  {
    0.0025,
    0.000244,
    0.0000136,
    0.0000111,
    0.0000386,
    0.00000145,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Conversion 93 307 keV yield per parent decay. */
constexpr long double k_conversion_93_307_keV_yield{0.3252078L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_93_307_keV_energies_micro_eV{
    {
      83'648'000'000ULL,
      92'113'000'000ULL,
      92'264'000'000ULL,
      92'287'000'000ULL,
      93'242'000'000ULL,
      93'306'000'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_93_307_keV_line_yields{
  {
    0.285,
    0.0254,
    0.00394,
    0.00577,
    0.00495,
    0.0001478,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Conversion 184 577 keV yield per parent decay. */
constexpr long double k_conversion_184_577_keV_yield{0.0035407L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_184_577_keV_energies_micro_eV{
    {
      174'918'000'000ULL,
      183'383'000'000ULL,
      183'534'000'000ULL,
      183'557'000'000ULL,
      184'512'000'000ULL,
      184'576'000'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_184_577_keV_line_yields{
  {
    0.00316,
    0.000306,
    0.000013,
    0.0000119,
    0.000048,
    0.0000018,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Conversion 208 939 keV yield per parent decay. */
constexpr long double k_conversion_208_939_keV_yield{0.0002135254L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_208_939_keV_energies_micro_eV{
    {
      199'280'000'000ULL,
      207'745'000'000ULL,
      207'896'000'000ULL,
      207'919'000'000ULL,
      208'874'000'000ULL,
      208'938'000'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_208_939_keV_line_yields{
  {
    0.000191,
    0.00001889,
    0.000000486,
    0.000000228,
    0.00000281,
    0.0000001114,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Conversion 300 232 keV yield per parent decay. */
constexpr long double k_conversion_300_232_keV_yield{0.000645600L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_300_232_keV_energies_micro_eV{
    {
      290'574'000'000ULL,
      299'039'000'000ULL,
      299'190'000'000ULL,
      299'213'000'000ULL,
      300'168'000'000ULL,
      300'232'000'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_300_232_keV_line_yields{
  {
    0.000578,
    0.0000568,
    0.000001275,
    0.00000076,
    0.00000843,
    0.000000335,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Conversion 393 528 keV yield per parent decay. */
constexpr long double k_conversion_393_528_keV_yield{0.0000885200L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_393_528_keV_energies_micro_eV{
    {
      383'870'000'000ULL,
      392'335'000'000ULL,
      392'486'000'000ULL,
      392'509'000'000ULL,
      393'464'000'000ULL,
      393'528'000'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_393_528_keV_line_yields{
  {
    0.0000793,
    0.00000782,
    0.0000001331,
    0.0000000688,
    0.000001152,
    0.0000000461,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Conversion 494 143 keV yield per parent decay. */
constexpr long double k_conversion_494_143_keV_yield{7.65457E-7L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_494_143_keV_energies_micro_eV{
    {
      484'486'000'000ULL,
      492'951'000'000ULL,
      493'102'000'000ULL,
      493'125'000'000ULL,
      494'080'000'000ULL,
      494'144'000'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_494_143_keV_line_yields{
  {
    0.000000686,
    0.0000000676,
    0.00000000099,
    0.000000000559,
    0.00000000991,
    0.000000000398,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Conversion 703 11 keV yield per parent decay. */
constexpr long double k_conversion_703_11_keV_yield{5.92053E-8L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_703_11_keV_energies_micro_eV{
    {
      693'450'000'000ULL,
      701'920'000'000ULL,
      702'070'000'000ULL,
      702'090'000'000ULL,
      703'040'000'000ULL,
      703'110'000'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_703_11_keV_line_yields{
  {
    0.0000000531,
    0.00000000522,
    0.0000000000583,
    0.0000000000363,
    0.00000000076,
    0.0000000000307,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Conversion 794 4 keV yield per parent decay. */
constexpr long double k_conversion_794_4_keV_yield{2.82516E-7L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_794_4_keV_energies_micro_eV{
    {
      784'746'000'000ULL,
      793'211'000'000ULL,
      793'362'000'000ULL,
      793'385'000'000ULL,
      794'340'000'000ULL,
      794'404'000'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_794_4_keV_line_yields{
  {
    0.000000253,
    0.0000000248,
    0.00000000043,
    0.000000000438,
    0.0000000037,
    0.000000000148,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Conversion 887 676 keV yield per parent decay. */
constexpr long double k_conversion_887_676_keV_yield{5.28567E-7L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_887_676_keV_energies_micro_eV{
    {
      878'023'000'000ULL,
      886'488'000'000ULL,
      886'639'000'000ULL,
      886'662'000'000ULL,
      887'617'000'000ULL,
      887'681'000'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_887_676_keV_line_yields{
  {
    0.000000474,
    0.0000000464,
    0.000000000567,
    0.000000000527,
    0.0000000068,
    0.000000000273,
  },
};

} // namespace

// =============================================================================
// =============================================================================

[[nodiscard]] auto BuildGa67Radionuclide() -> GGEMSRadionuclideDefinition {
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
                         k_conversion_91_263_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_91_263_keV_energies_micro_eV,
                           k_conversion_91_263_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_93_307_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_93_307_keV_energies_micro_eV,
                           k_conversion_93_307_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_184_577_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_184_577_keV_energies_micro_eV,
                           k_conversion_184_577_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_208_939_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_208_939_keV_energies_micro_eV,
                           k_conversion_208_939_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_300_232_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_300_232_keV_energies_micro_eV,
                           k_conversion_300_232_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_393_528_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_393_528_keV_energies_micro_eV,
                           k_conversion_393_528_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_494_143_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_494_143_keV_energies_micro_eV,
                           k_conversion_494_143_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_703_11_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_703_11_keV_energies_micro_eV,
                           k_conversion_703_11_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_794_4_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_794_4_keV_energies_micro_eV,
                           k_conversion_794_4_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_887_676_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_887_676_keV_energies_micro_eV,
                           k_conversion_887_676_keV_line_yields));

  return {"Ga-67", k_half_life_seconds, std::move(emissions)};
}

} // namespace ggems::core::radioactivity::builtins
