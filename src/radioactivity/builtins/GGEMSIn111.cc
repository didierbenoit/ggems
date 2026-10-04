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
 * \brief Defines the compiled In-111 marginal source-emission laws.
 *
 * LNHB: V. P. Chechev, March 2006; April 2006 tables. Selected EC source
 * emissions.
 *
 * Selected LNHB per-parent marginal lines include the weak 150.81 keV
 * isomer-fed transition; the 48.50 min level delay is not modeled.
 *
 * Physical yields remain independent emissions per parent decay. Selected
 * photon and conversion lines use absolute LNHB yields; unsupported Auger
 * energy laws are excluded.
 *
 * Selection and exclusions:
 * validation/radioactivity/data/In-111/reference/.
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

/*! \brief Evaluated In-111 parent half-life in seconds. */
constexpr long double k_half_life_seconds{242343.3600L};

// =============================================================================
// =============================================================================

/*! \brief Nuclear photon yield per parent decay. */
constexpr long double k_nuclear_gamma_yield{1.847315L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 3U> k_nuclear_gamma_energies_micro_eV{
  {
    150'810'000'000ULL,
    171'280'000'000ULL,
    245'350'000'000ULL,
  },
};

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 3U> k_nuclear_gamma_line_yields{
  {
    0.000015,
    0.9061,
    0.9412,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Atomic X-ray yield per parent decay. */
constexpr long double k_atomic_xray_yield{0.8956L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 5U> k_atomic_xray_energies_micro_eV{
  {
    3'360'000'000ULL,
    22'984'300'000ULL,
    23'173'800'000ULL,
    26'153'800'000ULL,
    26'677'300'000ULL,
  },
};

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 5U> k_atomic_xray_line_yields{
  {
    0.0678,
    0.2365,
    0.4447,
    0.124,
    0.0226,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Conversion-electron group for the 150.81 keV transition yield per
 * parent decay. */
constexpr long double k_conversion_150_81_keV_yield{0.0000341L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 3U>
  k_conversion_150_81_keV_energies_micro_eV{
    {
      124'099'000'000ULL,
      147'049'000'000ULL,
      150'240'000'000ULL,
    },
};

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 3U> k_conversion_150_81_keV_line_yields{
  {
    0.000022,
    0.00001,
    0.0000021,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Conversion-electron group for the 171.28 keV transition yield per
 * parent decay. */
constexpr long double k_conversion_171_28_keV_yield{0.093506L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 3U>
  k_conversion_171_28_keV_energies_micro_eV{
    {
      144'569'000'000ULL,
      167'519'000'000ULL,
      170'710'000'000ULL,
    },
};

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 3U> k_conversion_171_28_keV_line_yields{
  {
    0.0813,
    0.01024,
    0.001966,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Conversion-electron group for the 245.35 keV transition yield per
 * parent decay. */
constexpr long double k_conversion_245_35_keV_yield{0.058497L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 3U>
  k_conversion_245_35_keV_energies_micro_eV{
    {
      218'639'000'000ULL,
      241'589'000'000ULL,
      244'780'000'000ULL,
    },
};

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 3U> k_conversion_245_35_keV_line_yields{
  {
    0.0493,
    0.0077,
    0.001497,
  },
};

} // namespace

// =============================================================================
// =============================================================================

[[nodiscard]] auto BuildIn111Radionuclide() -> GGEMSRadionuclideDefinition {
  std::vector<GGEMSRadionuclideEmission> emissions;
  emissions.reserve(5U);

  emissions.emplace_back(
    particles::GGEMSParticleType::Gamma, k_nuclear_gamma_yield,
    sources::GGEMSEnergyDistribution::BuildDiscreteLines(
      k_nuclear_gamma_energies_micro_eV, k_nuclear_gamma_line_yields));

  emissions.emplace_back(
    particles::GGEMSParticleType::Gamma, k_atomic_xray_yield,
    sources::GGEMSEnergyDistribution::BuildDiscreteLines(
      k_atomic_xray_energies_micro_eV, k_atomic_xray_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_150_81_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_150_81_keV_energies_micro_eV,
                           k_conversion_150_81_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_171_28_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_171_28_keV_energies_micro_eV,
                           k_conversion_171_28_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_245_35_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_245_35_keV_energies_micro_eV,
                           k_conversion_245_35_keV_line_yields));

  return {"In-111", k_half_life_seconds, std::move(emissions)};
}

} // namespace ggems::core::radioactivity::builtins
