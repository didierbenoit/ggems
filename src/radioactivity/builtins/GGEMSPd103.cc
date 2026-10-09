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
 * \brief Defines the compiled Pd-103 marginal source-emission laws.
 *
 * LNHB: A. L. Nichols and T. Kibedi, 2024. Selected EC source emissions.
 *
 * Delayed daughter-isomer emissions and inseparable prompt/delayed atomic
 * totals are excluded.
 *
 * Physical yields remain independent emissions per parent decay. EC creates
 * no primary; daughter chains and unsupported Auger energy laws are
 * excluded.
 *
 * Selection and exclusions:
 * validation/radioactivity/data/Pd-103/reference/.
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

/*! \brief Evaluated Pd-103 parent half-life in seconds. */
constexpr long double k_half_life_seconds{1468800.00L};

// =============================================================================
// =============================================================================

/*! \brief nuclear gamma yield per parent decay. */
constexpr long double k_nuclear_gamma_yield{0.0003605315L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 8U> k_nuclear_gamma_energies_micro_eV{
  {
    53'283'000'000ULL,
    62'410'000'000ULL,
    241'875'000'000ULL,
    294'962'000'000ULL,
    317'720'000'000ULL,
    357'380'000'000ULL,
    443'809'000'000ULL,
    497'083'000'000ULL,
  },
};

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 8U> k_nuclear_gamma_line_yields{
  {
    6.2E-8,
    0.0000122,
    8.5E-9,
    0.0000334,
    1.74E-7,
    0.000267,
    1.87E-7,
    0.0000475,
  },
};

// =============================================================================
// =============================================================================

/*! \brief conversion 53 283 keV yield per parent decay. */
constexpr long double k_conversion_53_283_keV_yield{1.28745E-7L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_53_283_keV_energies_micro_eV{
    {
      30'063'000'000ULL,
      49'871'000'000ULL,
      50'137'000'000ULL,
      50'279'000'000ULL,
      52'830'000'000ULL,
      53'247'000'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_53_283_keV_line_yields{
  {
    1.12E-7,
    1.26E-8,
    8.7E-10,
    2.59E-10,
    2.57E-9,
    4.46E-10,
  },
};

// =============================================================================
// =============================================================================

/*! \brief conversion 62 41 keV yield per parent decay. */
constexpr long double k_conversion_62_41_keV_yield{0.0000160324L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_62_41_keV_energies_micro_eV{
    {
      39'190'000'000ULL,
      58'998'000'000ULL,
      59'264'000'000ULL,
      59'406'000'000ULL,
      61'957'000'000ULL,
      62'374'000'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_62_41_keV_line_yields{
  {
    0.00001394,
    0.00000158,
    1.055E-7,
    3.15E-8,
    3.2E-7,
    5.54E-8,
  },
};

// =============================================================================
// =============================================================================

/*! \brief conversion 241 875 keV yield per parent decay. */
constexpr long double k_conversion_241_875_keV_yield{1.00464E-10L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_241_875_keV_energies_micro_eV{
    {
      218'655'000'000ULL,
      238'463'000'000ULL,
      238'729'000'000ULL,
      238'871'000'000ULL,
      241'422'000'000ULL,
      241'839'000'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_241_875_keV_line_yields{
  {
    8.8E-11,
    9.2E-12,
    4.28E-13,
    6.2E-13,
    1.89E-12,
    3.26E-13,
  },
};

// =============================================================================
// =============================================================================

/*! \brief conversion 294 962 keV yield per parent decay. */
constexpr long double k_conversion_294_962_keV_yield{6.3810E-7L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_294_962_keV_energies_micro_eV{
    {
      271'742'000'000ULL,
      291'550'000'000ULL,
      291'816'000'000ULL,
      291'958'000'000ULL,
      294'509'000'000ULL,
      294'926'000'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_294_962_keV_line_yields{
  {
    5.57E-7,
    6.25E-8,
    2.9E-9,
    1.19E-9,
    1.236E-8,
    2.15E-9,
  },
};

// =============================================================================
// =============================================================================

/*! \brief conversion 317 72 keV yield per parent decay. */
constexpr long double k_conversion_317_72_keV_yield{9.8118E-10L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_317_72_keV_energies_micro_eV{
    {
      294'500'000'000ULL,
      314'310'000'000ULL,
      314'570'000'000ULL,
      314'720'000'000ULL,
      317'270'000'000ULL,
      317'680'000'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_317_72_keV_line_yields{
  {
    8.6E-10,
    9.1E-11,
    3.5E-12,
    5.1E-12,
    1.84E-11,
    3.18E-12,
  },
};

// =============================================================================
// =============================================================================

/*! \brief conversion 357 38 keV yield per parent decay. */
constexpr long double k_conversion_357_38_keV_yield{0.0000042376L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_357_38_keV_energies_micro_eV{
    {
      334'160'000'000ULL,
      353'970'000'000ULL,
      354'230'000'000ULL,
      354'380'000'000ULL,
      356'930'000'000ULL,
      357'340'000'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_357_38_keV_line_yields{
  {
    0.00000365,
    3.83E-7,
    5.21E-8,
    4.72E-8,
    9E-8,
    1.53E-8,
  },
};

// =============================================================================
// =============================================================================

/*! \brief conversion 443 809 keV yield per parent decay. */
constexpr long double k_conversion_443_809_keV_yield{1.51268E-9L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_443_809_keV_energies_micro_eV{
    {
      420'589'000'000ULL,
      440'397'000'000ULL,
      440'663'000'000ULL,
      440'805'000'000ULL,
      443'356'000'000ULL,
      443'773'000'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_443_809_keV_line_yields{
  {
    1.31E-9,
    1.39E-10,
    1.48E-11,
    1.27E-11,
    3.09E-11,
    5.28E-12,
  },
};

// =============================================================================
// =============================================================================

/*! \brief conversion 497 083 keV yield per parent decay. */
constexpr long double k_conversion_497_083_keV_yield{2.49217E-7L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_497_083_keV_energies_micro_eV{
    {
      473'863'000'000ULL,
      493'671'000'000ULL,
      493'937'000'000ULL,
      494'079'000'000ULL,
      496'630'000'000ULL,
      497'047'000'000ULL,
    },
  };

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_497_083_keV_line_yields{
  {
    2.18E-7,
    2.42E-8,
    9.64E-10,
    4.75E-10,
    4.75E-9,
    8.28E-10,
  },
};

} // namespace

// =============================================================================
// =============================================================================

[[nodiscard]] auto BuildPd103Radionuclide() -> GGEMSRadionuclideDefinition {
  std::vector<GGEMSRadionuclideEmission> emissions;
  emissions.reserve(9U);

  emissions.emplace_back(
    particles::GGEMSParticleType::Gamma, k_nuclear_gamma_yield,
    sources::GGEMSEnergyDistribution::BuildDiscreteLines(
      k_nuclear_gamma_energies_micro_eV, k_nuclear_gamma_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_53_283_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_53_283_keV_energies_micro_eV,
                           k_conversion_53_283_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_62_41_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_62_41_keV_energies_micro_eV,
                           k_conversion_62_41_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_241_875_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_241_875_keV_energies_micro_eV,
                           k_conversion_241_875_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_294_962_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_294_962_keV_energies_micro_eV,
                           k_conversion_294_962_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_317_72_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_317_72_keV_energies_micro_eV,
                           k_conversion_317_72_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_357_38_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_357_38_keV_energies_micro_eV,
                           k_conversion_357_38_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_443_809_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_443_809_keV_energies_micro_eV,
                           k_conversion_443_809_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_497_083_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_497_083_keV_energies_micro_eV,
                           k_conversion_497_083_keV_line_yields));

  return {"Pd-103", k_half_life_seconds, std::move(emissions)};
}

} // namespace ggems::core::radioactivity::builtins
