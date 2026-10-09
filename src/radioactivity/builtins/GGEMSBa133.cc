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
 * \brief Defines the compiled Ba-133 marginal source-emission laws.
 *
 * LNHB: V. P. Chechev and N. K. Kuzmenko, KRI, 2015; 2016 tables. Selected
 * EC source emissions.
 *
 * Physical yields remain independent emissions per parent decay. EC creates
 * no primary; daughter chains and unsupported Auger energy laws are
 * excluded.
 *
 * Selection and exclusions:
 * validation/radioactivity/data/Ba-133/reference/.
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

/*! \brief Evaluated Ba-133 parent half-life in seconds. */
constexpr long double k_half_life_seconds{332579520.0L};

// =============================================================================
// =============================================================================

/*! \brief nuclear gamma yield per parent decay. */
constexpr long double k_nuclear_gamma_yield{1.35598L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 9U> k_nuclear_gamma_energies_micro_eV{
  {
    53'162'200'000ULL,
    79'614'200'000ULL,
    80'997'900'000ULL,
    160'612'100'000ULL,
    223'236'800'000ULL,
    276'398'900'000ULL,
    302'850'800'000ULL,
    356'012'900'000ULL,
    383'848'500'000ULL,
  },
};

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 9U> k_nuclear_gamma_line_yields{
  {
    0.0214,
    0.0263,
    0.3331,
    0.00638,
    0.0045,
    0.0713,
    0.1831,
    0.6205,
    0.0894,
  },
};

// =============================================================================
// =============================================================================

/*! \brief atomic xray yield per parent decay. */
constexpr long double k_atomic_xray_yield{1.3476L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 5U> k_atomic_xray_energies_micro_eV{
  {
    4'673'550'000ULL,
    30'625'400'000ULL,
    30'973'100'000ULL,
    35'053'000'000ULL,
    35'900'300'000ULL,
  },
};

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 5U> k_atomic_xray_line_yields{
  {
    0.1587,
    0.338,
    0.624,
    0.1824,
    0.0445,
  },
};

// =============================================================================
// =============================================================================

/*! \brief conversion 53 1622 keV yield per parent decay. */
constexpr long double k_conversion_53_1622_keV_yield{0.12106L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_53_1622_keV_energies_micro_eV{
    {
      17'177'600'000ULL,
      47'447'900'000ULL,
      47'802'800'000ULL,
      48'150'300'000ULL,
      52'213'300'000ULL,
      53'018'200'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_53_1622_keV_line_yields{
  {
    0.1023,
    0.01252,
    0.00152,
    0.0009,
    0.00308,
    0.00074,
  },
};

// =============================================================================
// =============================================================================

/*! \brief conversion 79 6142 keV yield per parent decay. */
constexpr long double k_conversion_79_6142_keV_yield{0.046471L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_79_6142_keV_energies_micro_eV{
    {
      43'629'600'000ULL,
      73'899'900'000ULL,
      74'254'800'000ULL,
      74'602'300'000ULL,
      78'665'300'000ULL,
      79'470'200'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_79_6142_keV_line_yields{
  {
    0.0393,
    0.00479,
    0.00058,
    0.00034,
    0.00118,
    0.000281,
  },
};

// =============================================================================
// =============================================================================

/*! \brief conversion 80 9979 keV yield per parent decay. */
constexpr long double k_conversion_80_9979_keV_yield{0.56750L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_80_9979_keV_energies_micro_eV{
    {
      45'013'300'000ULL,
      75'283'600'000ULL,
      75'638'500'000ULL,
      75'986'000'000ULL,
      80'049'000'000ULL,
      80'853'900'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_80_9979_keV_line_yields{
  {
    0.477,
    0.0576,
    0.00843,
    0.00603,
    0.01489,
    0.00355,
  },
};

// =============================================================================
// =============================================================================

/*! \brief conversion 160 6121 keV yield per parent decay. */
constexpr long double k_conversion_160_6121_keV_yield{0.00187084L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_160_6121_keV_energies_micro_eV{
    {
      124'627'500'000ULL,
      154'897'800'000ULL,
      155'252'700'000ULL,
      155'600'200'000ULL,
      159'663'200'000ULL,
      160'468'100'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_160_6121_keV_line_yields{
  {
    0.001493,
    0.0001627,
    0.0000708,
    0.0000664,
    0.0000632,
    0.00001474,
  },
};

// =============================================================================
// =============================================================================

/*! \brief conversion 223 2368 keV yield per parent decay. */
constexpr long double k_conversion_223_2368_keV_yield{0.000438265L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_223_2368_keV_energies_micro_eV{
    {
      187'252'400'000ULL,
      217'522'700'000ULL,
      217'877'600'000ULL,
      218'225'100'000ULL,
      222'288'100'000ULL,
      223'093'000'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_223_2368_keV_line_yields{
  {
    0.000376,
    0.0000457,
    0.00000306,
    8.9E-7,
    0.00001017,
    0.000002445,
  },
};

// =============================================================================
// =============================================================================

/*! \brief conversion 276 3989 keV yield per parent decay. */
constexpr long double k_conversion_276_3989_keV_yield{0.0040351L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_276_3989_keV_energies_micro_eV{
    {
      240'414'600'000ULL,
      270'684'900'000ULL,
      271'039'800'000ULL,
      271'387'300'000ULL,
      275'450'300'000ULL,
      276'255'200'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_276_3989_keV_line_yields{
  {
    0.00328,
    0.00035,
    0.0001362,
    0.0001138,
    0.0001257,
    0.0000294,
  },
};

// =============================================================================
// =============================================================================

/*! \brief conversion 302 8508 keV yield per parent decay. */
constexpr long double k_conversion_302_8508_keV_yield{0.00793991L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_302_8508_keV_energies_micro_eV{
    {
      266'866'600'000ULL,
      297'136'900'000ULL,
      297'491'800'000ULL,
      297'839'300'000ULL,
      301'902'300'000ULL,
      302'707'200'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_302_8508_keV_line_yields{
  {
    0.00683,
    0.000828,
    0.0000465,
    0.00001091,
    0.0001809,
    0.0000436,
  },
};

// =============================================================================
// =============================================================================

/*! \brief conversion 356 0129 keV yield per parent decay. */
constexpr long double k_conversion_356_0129_keV_yield{0.0157948L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_356_0129_keV_energies_micro_eV{
    {
      320'028'800'000ULL,
      350'299'100'000ULL,
      350'654'000'000ULL,
      351'001'500'000ULL,
      355'064'500'000ULL,
      355'869'400'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_356_0129_keV_line_yields{
  {
    0.01309,
    0.00144,
    0.000403,
    0.0003096,
    0.000447,
    0.0001052,
  },
};

// =============================================================================
// =============================================================================

/*! \brief conversion 383 8485 keV yield per parent decay. */
constexpr long double k_conversion_383_8485_keV_yield{0.00180753L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_383_8485_keV_energies_micro_eV{
    {
      347'864'500'000ULL,
      378'134'800'000ULL,
      378'489'700'000ULL,
      378'837'200'000ULL,
      382'900'200'000ULL,
      383'705'100'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_383_8485_keV_line_yields{
  {
    0.001505,
    0.0001663,
    0.0000425,
    0.00003183,
    0.0000501,
    0.0000118,
  },
};

} // namespace

// =============================================================================
// =============================================================================

[[nodiscard]] auto BuildBa133Radionuclide() -> GGEMSRadionuclideDefinition {
  std::vector<GGEMSRadionuclideEmission> emissions;
  emissions.reserve(11U);

  emissions.emplace_back(
    particles::GGEMSParticleType::Gamma, k_nuclear_gamma_yield,
    sources::GGEMSEnergyDistribution::BuildDiscreteLines(
      k_nuclear_gamma_energies_micro_eV, k_nuclear_gamma_line_yields));

  emissions.emplace_back(
    particles::GGEMSParticleType::Gamma, k_atomic_xray_yield,
    sources::GGEMSEnergyDistribution::BuildDiscreteLines(
      k_atomic_xray_energies_micro_eV, k_atomic_xray_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_53_1622_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_53_1622_keV_energies_micro_eV,
                           k_conversion_53_1622_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_79_6142_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_79_6142_keV_energies_micro_eV,
                           k_conversion_79_6142_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_80_9979_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_80_9979_keV_energies_micro_eV,
                           k_conversion_80_9979_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_160_6121_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_160_6121_keV_energies_micro_eV,
                           k_conversion_160_6121_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_223_2368_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_223_2368_keV_energies_micro_eV,
                           k_conversion_223_2368_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_276_3989_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_276_3989_keV_energies_micro_eV,
                           k_conversion_276_3989_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_302_8508_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_302_8508_keV_energies_micro_eV,
                           k_conversion_302_8508_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_356_0129_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_356_0129_keV_energies_micro_eV,
                           k_conversion_356_0129_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_383_8485_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_383_8485_keV_energies_micro_eV,
                           k_conversion_383_8485_keV_line_yields));

  return {"Ba-133", k_half_life_seconds, std::move(emissions)};
}

} // namespace ggems::core::radioactivity::builtins
