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
 * \brief Defines the compiled Cr-51 marginal source-emission laws.
 *
 * V. P. Chechev and N. K. Kuzmenko, KRI/LNHB, 2014. Selected EC source
 * emissions.
 *
 * Compact LNHB photon and PenNuc conversion lines retain absolute yields;
 * unsupported grouped Auger energy laws are excluded.
 *
 * Physical yields remain independent emissions per parent decay. EC creates
 * no primary; neutrinos, recoil and additional radioactive daughter decays
 * are excluded.
 *
 * Selection and exclusions: validation/radioactivity/data/Cr-51/reference/.
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

/*! \brief Evaluated Cr-51 parent half-life in seconds. */
constexpr long double k_half_life_seconds{2393625.600L};

// =============================================================================
// =============================================================================

/*! \brief Nuclear photon yield per parent decay. */
constexpr long double k_nuclear_gamma_yield{0.0989L};

/*! \brief Selected monoenergy in canonical integer micro-eV. */
constexpr std::uint64_t k_nuclear_gamma_energy_micro_eV{320'083'500'000ULL};

// =============================================================================
// =============================================================================

/*! \brief Atomic X-ray yield per parent decay. */
constexpr long double k_atomic_xray_yield{0.2340L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 4U> k_atomic_xray_energies_micro_eV{
  {
    537'150'000ULL,
    4'944'700'000ULL,
    4'952'240'000ULL,
    5'445'200'000ULL,
  },
};

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 4U> k_atomic_xray_line_yields{
  {
    0.0056,
    0.0679,
    0.1336,
    0.0269,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Conversion-electron yield per parent decay. */
constexpr long double k_conversion_320_0835_keV_yield{0.0001792339L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_320_0835_keV_energies_micro_eV{
    {
      314'618'400'000ULL,
      319'455'300'000ULL,
      319'563'000'000ULL,
      319'570'600'000ULL,
      320'054'200'000ULL,
      320'083'200'000ULL,
    },
};

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_320_0835_keV_line_yields{
  {
    0.0001622,
    0.00001454,
    2.21E-7,
    2.13E-7,
    0.00000196,
    9.99E-8,
  },
};

} // namespace

// =============================================================================
// =============================================================================

[[nodiscard]] auto BuildCr51Radionuclide() -> GGEMSRadionuclideDefinition {
  std::vector<GGEMSRadionuclideEmission> emissions;
  emissions.reserve(3U);

  emissions.emplace_back(particles::GGEMSParticleType::Gamma,
                         k_nuclear_gamma_yield,
                         sources::GGEMSEnergyDistribution::BuildMono(
                           k_nuclear_gamma_energy_micro_eV));

  emissions.emplace_back(
    particles::GGEMSParticleType::Gamma, k_atomic_xray_yield,
    sources::GGEMSEnergyDistribution::BuildDiscreteLines(
      k_atomic_xray_energies_micro_eV, k_atomic_xray_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_320_0835_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_320_0835_keV_energies_micro_eV,
                           k_conversion_320_0835_keV_line_yields));

  return {"Cr-51", k_half_life_seconds, std::move(emissions)};
}

} // namespace ggems::core::radioactivity::builtins
