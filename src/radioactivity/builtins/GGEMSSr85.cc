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
 * \brief Defines the compiled Sr-85 marginal source-emission laws.
 *
 * LNHB: PTB, 1998. Selected EC source emissions.
 *
 * Delayed daughter-isomer emissions and inseparable prompt/delayed totals
 * are excluded.
 *
 * The evaluated 514.007 keV Rb-85 isomer (1.015 microseconds) is excluded,
 * including its mixed downstream 151.160 keV law. Directly populated
 * non-isomeric levels retain prompt relaxation; no lifetime cutoff is
 * introduced.
 *
 * The weak 129.826 keV line follows the selected table despite tentative
 * upper-limit evidence in the evaluation commentary.
 *
 * Physical yields remain independent emissions per parent decay. EC creates
 * no primary; daughter chains and unsupported Auger energy laws are
 * excluded.
 *
 * Selection and exclusions: validation/radioactivity/data/Sr-85/reference/.
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

/*! \brief Evaluated Sr-85 parent half-life in seconds. */
constexpr long double k_half_life_seconds{5603040.000L};

// =============================================================================
// =============================================================================

/*! \brief nuclear gamma yield per parent decay. */
constexpr long double k_nuclear_gamma_yield{0.0001342L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 4U> k_nuclear_gamma_energies_micro_eV{
  {
    129'826'000'000ULL,
    354'970'000'000ULL,
    717'810'000'000ULL,
    868'980'000'000ULL,
  },
};

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 4U> k_nuclear_gamma_line_yields{
  {
    0.000005,
    0.000005,
    0.0000032,
    0.000121,
  },
};

// =============================================================================
// =============================================================================

/*! \brief conversion 129 826 keV yield per parent decay. */
constexpr long double k_conversion_129_826_keV_yield{3.60766E-7L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_129_826_keV_energies_micro_eV{
    {
      114'626'000'000ULL,
      127'761'000'000ULL,
      127'962'000'000ULL,
      128'022'000'000ULL,
      129'620'000'000ULL,
      129'811'000'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_129_826_keV_line_yields{
  {
    3.18E-7,
    3.38E-8,
    1.515E-9,
    5.6E-10,
    6.04E-9,
    8.51E-10,
  },
};

// =============================================================================
// =============================================================================

/*! \brief conversion 354 97 keV yield per parent decay. */
constexpr long double k_conversion_354_97_keV_yield{1.2637E-8L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_354_97_keV_energies_micro_eV{
    {
      339'770'000'000ULL,
      352'900'000'000ULL,
      353'110'000'000ULL,
      353'170'000'000ULL,
      354'760'000'000ULL,
      354'960'000'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_354_97_keV_line_yields{
  {
    1.12E-8,
    1.14E-9,
    2.6E-11,
    4.2E-11,
    2E-10,
    2.9E-11,
  },
};

// =============================================================================
// =============================================================================

/*! \brief conversion 717 81 keV yield per parent decay. */
constexpr long double k_conversion_717_81_keV_yield{3.9492E-9L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_717_81_keV_energies_micro_eV{
    {
      702'610'000'000ULL,
      715'740'000'000ULL,
      715'950'000'000ULL,
      716'010'000'000ULL,
      717'600'000'000ULL,
      717'800'000'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_717_81_keV_line_yields{
  {
    3.49E-9,
    3.61E-10,
    1.25E-11,
    1.16E-11,
    6.5E-11,
    9.1E-12,
  },
};

// =============================================================================
// =============================================================================

/*! \brief conversion 868 98 keV yield per parent decay. */
constexpr long double k_conversion_868_98_keV_yield{8.8691E-8L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_868_98_keV_energies_micro_eV{
    {
      853'780'000'000ULL,
      866'910'000'000ULL,
      867'120'000'000ULL,
      867'180'000'000ULL,
      868'770'000'000ULL,
      868'970'000'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_868_98_keV_line_yields{
  {
    7.86E-8,
    8.14E-9,
    1.82E-10,
    1.48E-10,
    1.42E-9,
    2.01E-10,
  },
};

} // namespace

// =============================================================================
// =============================================================================

[[nodiscard]] auto BuildSr85Radionuclide() -> GGEMSRadionuclideDefinition {
  std::vector<GGEMSRadionuclideEmission> emissions;
  emissions.reserve(5U);

  emissions.emplace_back(
    particles::GGEMSParticleType::Gamma, k_nuclear_gamma_yield,
    sources::GGEMSEnergyDistribution::BuildDiscreteLines(
      k_nuclear_gamma_energies_micro_eV, k_nuclear_gamma_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_129_826_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_129_826_keV_energies_micro_eV,
                           k_conversion_129_826_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_354_97_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_354_97_keV_energies_micro_eV,
                           k_conversion_354_97_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_717_81_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_717_81_keV_energies_micro_eV,
                           k_conversion_717_81_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_868_98_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_868_98_keV_energies_micro_eV,
                           k_conversion_868_98_keV_line_yields));

  return {"Sr-85", k_half_life_seconds, std::move(emissions)};
}

} // namespace ggems::core::radioactivity::builtins
