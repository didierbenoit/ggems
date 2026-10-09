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
 * \brief Defines the compiled At-211 marginal source-emission laws.
 *
 * LNHB retained evaluations select independent emissions per parent decay.
 * Prompt photon and conversion yields are included where separable;
 * unsupported Auger energy laws, neutrinos and recoil are excluded.
 *
 * Direct parent alpha lines use evaluated discrete energies and yields.
 * No radioactive daughter-chain emissions are added.
 *
 * The 4895.4 keV alpha upper limit is not a central emission yield.
 *
 * Selection and exclusions: validation/radioactivity/data/At-211/reference/.
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
constexpr long double k_half_life_seconds{25977.600L};

// =============================================================================
// =============================================================================

/*! \brief Absolute alpha yield per parent decay. */
constexpr long double k_alpha_yield{0.417854L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 4U> k_alpha_energies_micro_eV{
  4'993'400'000'000ULL,
  5'140'300'000'000ULL,
  5'211'900'000'000ULL,
  5'869'000'000'000ULL,
};

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 4U> k_alpha_line_yields{
  4e-06,
  1.1e-05,
  3.9e-05,
  0.4178,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute nuclear_gamma yield per parent decay. */
constexpr long double k_nuclear_gamma_yield{0.0025028L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U> k_nuclear_gamma_energies_micro_eV{
  149'720'000'000ULL, 222'690'000'000ULL, 669'770'000'000ULL,
  687'200'000'000ULL, 742'740'000'000ULL, 892'460'000'000ULL,
};

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_nuclear_gamma_line_yields{
  5e-07, 4e-07, 3.8e-05, 0.00245, 1.25e-05, 1.4e-06,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute atomic_xray yield per parent decay. */
constexpr long double k_atomic_xray_yield{0.61860471L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 10U> k_atomic_xray_energies_micro_eV{
  12'564'500'000ULL, 12'935'500'000ULL, 74'815'700'000ULL, 76'864'000'000ULL,
  77'108'800'000ULL, 79'293'000'000ULL, 87'347'000'000ULL, 89'808'700'000ULL,
  90'075'700'000ULL, 92'621'300'000ULL,
};

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 10U> k_atomic_xray_line_yields{
  1.36e-06, 0.186,   9.8e-07, 0.1266,  1.64e-06,
  0.2108,   5.6e-07, 0.0726,  1.7e-07, 0.0226,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_149_72_keV yield per parent decay. */
constexpr long double k_conversion_149_72_keV_yield{0.0000014736L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_149_72_keV_energies_micro_eV{
    59'190'000'000ULL,  133'330'000'000ULL, 134'010'000'000ULL,
    136'300'000'000ULL, 146'490'000'000ULL, 149'200'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_149_72_keV_line_yields{
  1.15e-06, 1.8e-07, 4.5e-08, 2e-08, 6e-08, 1.86e-08,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_222_69_keV yield per parent decay. */
constexpr long double k_conversion_222_69_keV_yield{3.8096E-7L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_222_69_keV_energies_micro_eV{
    132'160'000'000ULL, 206'300'000'000ULL, 206'980'000'000ULL,
    209'270'000'000ULL, 219'460'000'000ULL, 222'170'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_222_69_keV_line_yields{
  3.04e-07, 4.72e-08, 8.8e-09, 2.6e-09, 1.404e-08, 4.32e-09,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_669_77_keV yield per parent decay. */
constexpr long double k_conversion_669_77_keV_yield{0.00000197984L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_669_77_keV_energies_micro_eV{
    579'240'000'000ULL, 653'380'000'000ULL, 654'060'000'000ULL,
    656'350'000'000ULL, 666'540'000'000ULL, 669'250'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_669_77_keV_line_yields{
  1.62e-06, 2.47e-07, 2.57e-08, 2.24e-09, 6.5e-08, 1.99e-08,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_687_2_keV yield per parent decay. */
constexpr long double k_conversion_687_2_keV_yield{0.00013114L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_687_2_keV_energies_micro_eV{
    594'100'000'000ULL, 670'300'000'000ULL, 671'000'000'000ULL,
    673'400'000'000ULL, 683'800'000'000ULL, 686'600'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_687_2_keV_line_yields{
  0.000107, 1.66e-05, 1.72e-06, 1.3e-07, 4.34e-06, 1.35e-06,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_742_74_keV yield per parent decay. */
constexpr long double k_conversion_742_74_keV_yield{4.8889E-7L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_742_74_keV_energies_micro_eV{
    652'210'000'000ULL, 726'350'000'000ULL, 727'030'000'000ULL,
    729'320'000'000ULL, 739'510'000'000ULL, 742'220'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_742_74_keV_line_yields{
  4e-07, 6.1e-08, 6.4e-09, 5.9e-10, 1.6e-08, 4.9e-09,
};

// =============================================================================
// =============================================================================

/*! \brief Absolute conversion_892_46_keV yield per parent decay. */
constexpr long double k_conversion_892_46_keV_yield{2.0341E-8L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_892_46_keV_energies_micro_eV{
    801'930'000'000ULL, 876'070'000'000ULL, 876'750'000'000ULL,
    879'040'000'000ULL, 889'230'000'000ULL, 891'940'000'000ULL,
  };

/*! \brief Absolute line yields used as conditional relative weights. */
constexpr std::array<double, 6U> k_conversion_892_46_keV_line_yields{
  1.64e-08, 2.46e-09, 4.55e-10, 9.7e-11, 7.1e-10, 2.19e-10,
};

} // namespace

// =============================================================================
// =============================================================================

[[nodiscard]] auto BuildAt211Radionuclide() -> GGEMSRadionuclideDefinition {
  std::vector<GGEMSRadionuclideEmission> emissions;
  emissions.reserve(9U);

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
                         k_conversion_149_72_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_149_72_keV_energies_micro_eV,
                           k_conversion_149_72_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_222_69_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_222_69_keV_energies_micro_eV,
                           k_conversion_222_69_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_669_77_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_669_77_keV_energies_micro_eV,
                           k_conversion_669_77_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_687_2_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_687_2_keV_energies_micro_eV,
                           k_conversion_687_2_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_742_74_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_742_74_keV_energies_micro_eV,
                           k_conversion_742_74_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_892_46_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_892_46_keV_energies_micro_eV,
                           k_conversion_892_46_keV_line_yields));

  return {"At-211", k_half_life_seconds, std::move(emissions)};
}

} // namespace ggems::core::radioactivity::builtins
