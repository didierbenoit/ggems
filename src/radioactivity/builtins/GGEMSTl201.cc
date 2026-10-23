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
 * \brief Defines the compiled Tl-201 marginal source-emission laws.
 *
 * LNHB: E. Schonfeld and R. Dersch, 1997 evaluation with May 2004 half-life
 * update. Selected EC source emissions.
 *
 * The 141.18 keV transition is an evaluated upper limit and is excluded
 * with its derived conversion lines.
 *
 * Physical yields remain independent emissions per parent decay. Selected
 * photon and conversion lines use absolute LNHB yields; unsupported Auger
 * energy laws are excluded.
 *
 * Selection and exclusions:
 * validation/radioactivity/data/Tl-201/reference/.
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

/*! \brief Evaluated Tl-201 parent half-life in seconds. */
constexpr long double k_half_life_seconds{262837.4400L};

// =============================================================================
// =============================================================================

/*! \brief Nuclear photon yield per parent decay. */
constexpr long double k_nuclear_gamma_yield{0.1378101L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 8U> k_nuclear_gamma_energies_micro_eV{
  {
    1'565'000'000ULL,
    5'869'000'000ULL,
    26'269'000'000ULL,
    30'573'000'000ULL,
    32'138'000'000ULL,
    135'312'000'000ULL,
    165'885'000'000ULL,
    167'450'000'000ULL,
  },
};

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 8U> k_nuclear_gamma_line_yields{
  {
    0.0000081,
    0.005,
    0.000082,
    0.00258,
    0.00263,
    0.02604,
    0.00147,
    0.1,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Atomic X-ray yield per parent decay. */
constexpr long double k_atomic_xray_yield{1.3671L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 5U> k_atomic_xray_energies_micro_eV{
  {
    11'785'000'000ULL,
    68'895'000'000ULL,
    70'820'000'000ULL,
    80'279'700'000ULL,
    82'746'300'000ULL,
  },
};

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 5U> k_atomic_xray_line_yields{
  {
    0.427,
    0.273,
    0.464,
    0.157,
    0.0461,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Conversion-electron group for the 26.269 keV transition yield per
 * parent decay. */
constexpr long double k_conversion_26_269_keV_yield{0.006302L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 5U>
  k_conversion_26_269_keV_energies_micro_eV{
    {
      11'430'000'000ULL,
      12'060'000'000ULL,
      13'985'000'000ULL,
      23'396'000'000ULL,
      25'840'000'000ULL,
    },
};

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 5U> k_conversion_26_269_keV_line_yields{
  {
    0.00435,
    0.000429,
    0.000044,
    0.00113,
    0.000349,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Conversion-electron group for the 30.573 keV transition yield per
 * parent decay. */
constexpr long double k_conversion_30_573_keV_yield{0.126746L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 5U>
  k_conversion_30_573_keV_energies_micro_eV{
    {
      15'734'000'000ULL,
      16'364'000'000ULL,
      18'289'000'000ULL,
      27'700'000'000ULL,
      30'144'000'000ULL,
    },
};

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 5U> k_conversion_30_573_keV_line_yields{
  {
    0.0872,
    0.00875,
    0.001066,
    0.0227,
    0.00703,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Conversion-electron group for the 32.138 keV transition yield per
 * parent decay. */
constexpr long double k_conversion_32_138_keV_yield{0.110641L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 5U>
  k_conversion_32_138_keV_energies_micro_eV{
    {
      17'299'000'000ULL,
      17'929'000'000ULL,
      19'854'000'000ULL,
      29'265'000'000ULL,
      31'709'000'000ULL,
    },
};

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 5U> k_conversion_32_138_keV_line_yields{
  {
    0.0757,
    0.0076,
    0.001131,
    0.02,
    0.00621,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Conversion-electron group for the 135.312 keV transition yield per
 * parent decay. */
constexpr long double k_conversion_135_312_keV_yield{0.0901233L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_135_312_keV_energies_micro_eV{
    {
      52'210'000'000ULL,
      120'473'000'000ULL,
      121'103'000'000ULL,
      123'028'000'000ULL,
      132'439'000'000ULL,
      134'883'000'000ULL,
    },
};

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_135_312_keV_line_yields{
  {
    0.0737,
    0.01128,
    0.001133,
    0.0001333,
    0.00296,
    0.000917,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Conversion-electron group for the 165.885 keV transition yield per
 * parent decay. */
constexpr long double k_conversion_165_885_keV_yield{0.00282944L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_165_885_keV_energies_micro_eV{
    {
      82'783'000'000ULL,
      151'046'000'000ULL,
      151'676'000'000ULL,
      153'601'000'000ULL,
      163'012'000'000ULL,
      165'456'000'000ULL,
    },
};

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_165_885_keV_line_yields{
  {
    0.00231,
    0.00036,
    0.0000343,
    0.00000304,
    0.0000932,
    0.0000289,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Conversion-electron group for the 167.45 keV transition yield per
 * parent decay. */
constexpr long double k_conversion_167_45_keV_yield{0.189371L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_167_45_keV_energies_micro_eV{
    {
      84'348'000'000ULL,
      152'611'000'000ULL,
      153'241'000'000ULL,
      155'166'000'000ULL,
      164'577'000'000ULL,
      167'021'000'000ULL,
    },
};

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_167_45_keV_line_yields{
  {
    0.155,
    0.0236,
    0.00237,
    0.000281,
    0.0062,
    0.00192,
  },
};

} // namespace

// =============================================================================
// =============================================================================

[[nodiscard]] auto BuildTl201Radionuclide() -> GGEMSRadionuclideDefinition {
  std::vector<GGEMSRadionuclideEmission> emissions;
  emissions.reserve(8U);

  emissions.emplace_back(
    particles::GGEMSParticleType::Gamma, k_nuclear_gamma_yield,
    sources::GGEMSEnergyDistribution::BuildDiscreteLines(
      k_nuclear_gamma_energies_micro_eV, k_nuclear_gamma_line_yields));

  emissions.emplace_back(
    particles::GGEMSParticleType::Gamma, k_atomic_xray_yield,
    sources::GGEMSEnergyDistribution::BuildDiscreteLines(
      k_atomic_xray_energies_micro_eV, k_atomic_xray_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_26_269_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_26_269_keV_energies_micro_eV,
                           k_conversion_26_269_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_30_573_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_30_573_keV_energies_micro_eV,
                           k_conversion_30_573_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_32_138_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_32_138_keV_energies_micro_eV,
                           k_conversion_32_138_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_135_312_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_135_312_keV_energies_micro_eV,
                           k_conversion_135_312_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_165_885_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_165_885_keV_energies_micro_eV,
                           k_conversion_165_885_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_167_45_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_167_45_keV_energies_micro_eV,
                           k_conversion_167_45_keV_line_yields));

  return {"Tl-201", k_half_life_seconds, std::move(emissions)};
}

} // namespace ggems::core::radioactivity::builtins
