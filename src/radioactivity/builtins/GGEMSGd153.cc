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
 * \brief Defines the compiled Gd-153 marginal source-emission laws.
 *
 * R. G. Helmer, INEEL/LNHB, retained 2011/2012 update. Selected EC source
 * emissions.
 *
 * Compact LNHB photon and PenNuc conversion lines retain absolute yields;
 * unsupported grouped Auger energy laws are excluded.
 *
 * Physical yields remain independent emissions per parent decay. EC creates
 * no primary; neutrinos, recoil and additional radioactive daughter decays
 * are excluded.
 *
 * Selection and exclusions:
 * validation/radioactivity/data/Gd-153/reference/.
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

/*! \brief Evaluated Gd-153 parent half-life in seconds. */
constexpr long double k_half_life_seconds{20770560.0L};

// =============================================================================
// =============================================================================

/*! \brief Nuclear photon yield per parent decay. */
constexpr long double k_nuclear_gamma_yield{0.5292014L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 9U> k_nuclear_gamma_energies_micro_eV{
  {
    14'063'830'000ULL,
    19'812'960'000ULL,
    69'673'000'000ULL,
    75'422'130'000ULL,
    83'367'170'000ULL,
    89'485'950'000ULL,
    97'431'000'000ULL,
    103'180'120'000ULL,
    172'853'070'000ULL,
  },
};

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 9U> k_nuclear_gamma_line_yields{
  {
    0.0002,
    0.0000014,
    0.0242,
    0.00078,
    0.00197,
    0.00069,
    0.29,
    0.211,
    0.00036,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Atomic X-ray yield per parent decay. */
constexpr long double k_atomic_xray_yield{1.4041L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 5U> k_atomic_xray_energies_micro_eV{
  {
    6'483'050'000ULL,
    40'902'400'000ULL,
    41'542'700'000ULL,
    47'105'100'000ULL,
    48'380'000'000ULL,
  },
};

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 5U> k_atomic_xray_line_yields{
  {
    0.201,
    0.342,
    0.617,
    0.194,
    0.0501,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Conversion-electron yield per parent decay. */
constexpr long double k_conversion_14_06383_keV_yield{0.002181L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 5U>
  k_conversion_14_06383_keV_energies_micro_eV{
    {
      6'011'830'000ULL,
      6'446'730'000ULL,
      7'086'930'000ULL,
      12'626'630'000ULL,
      13'830'410'000ULL,
    },
};

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 5U> k_conversion_14_06383_keV_line_yields{
  {
    0.00043,
    0.00049,
    0.00079,
    0.00038,
    0.000091,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Conversion-electron yield per parent decay. */
constexpr long double k_conversion_19_81296_keV_yield{0.00450022L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 5U>
  k_conversion_19_81296_keV_energies_micro_eV{
    {
      11'760'960'000ULL,
      12'195'860'000ULL,
      12'836'060'000ULL,
      18'375'760'000ULL,
      19'579'540'000ULL,
    },
};

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 5U> k_conversion_19_81296_keV_line_yields{
  {
    0.00001382,
    0.00145,
    0.002026,
    0.000809,
    0.0002014,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Conversion-electron yield per parent decay. */
constexpr long double k_conversion_69_673_keV_yield{0.128401L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_69_673_keV_energies_micro_eV{
    {
      21'154'020'000ULL,
      61'621'020'000ULL,
      62'055'920'000ULL,
      62'696'120'000ULL,
      68'235'820'000ULL,
      69'439'600'000ULL,
    },
};

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_69_673_keV_line_yields{
  {
    0.1062,
    0.01384,
    0.00218,
    0.00138,
    0.0038,
    0.001001,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Conversion-electron yield per parent decay. */
constexpr long double k_conversion_75_42213_keV_yield{0.0005957L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_75_42213_keV_energies_micro_eV{
    {
      26'903'150'000ULL,
      67'370'150'000ULL,
      67'805'050'000ULL,
      68'445'250'000ULL,
      73'984'950'000ULL,
      75'188'730'000ULL,
    },
};

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_75_42213_keV_line_yields{
  {
    0.000484,
    0.000058,
    0.0000119,
    0.0000173,
    0.0000195,
    0.000005,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Conversion-electron yield per parent decay. */
constexpr long double k_conversion_83_36717_keV_yield{0.007421L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_83_36717_keV_energies_micro_eV{
    {
      34'848'190'000ULL,
      75'315'190'000ULL,
      75'750'090'000ULL,
      76'390'290'000ULL,
      81'929'990'000ULL,
      83'133'770'000ULL,
    },
};

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_83_36717_keV_line_yields{
  {
    0.00459,
    0.000536,
    0.00081,
    0.00085,
    0.000506,
    0.000129,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Conversion-electron yield per parent decay. */
constexpr long double k_conversion_89_48595_keV_yield{0.0017981L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_89_48595_keV_energies_micro_eV{
    {
      40'966'980'000ULL,
      81'433'980'000ULL,
      81'868'880'000ULL,
      82'509'080'000ULL,
      88'048'780'000ULL,
      89'252'560'000ULL,
    },
};

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_89_48595_keV_line_yields{
  {
    0.00146,
    0.000188,
    0.000041,
    0.000035,
    0.000059,
    0.0000151,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Conversion-electron yield per parent decay. */
constexpr long double k_conversion_97_431_keV_yield{0.088304L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_97_431_keV_energies_micro_eV{
    {
      48'912'030'000ULL,
      89'379'030'000ULL,
      89'813'930'000ULL,
      90'454'130'000ULL,
      95'993'830'000ULL,
      97'197'610'000ULL,
    },
};

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_97_431_keV_line_yields{
  {
    0.0742,
    0.00757,
    0.001557,
    0.00197,
    0.00239,
    0.000617,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Conversion-electron yield per parent decay. */
constexpr long double k_conversion_103_18012_keV_yield{0.35724L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_103_18012_keV_energies_micro_eV{
    {
      54'661'160'000ULL,
      95'128'160'000ULL,
      95'563'060'000ULL,
      96'203'260'000ULL,
      101'742'960'000ULL,
      102'946'740'000ULL,
    },
};

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_103_18012_keV_line_yields{
  {
    0.3,
    0.039,
    0.00424,
    0.00167,
    0.00975,
    0.00258,
  },
};

// =============================================================================
// =============================================================================

/*! \brief Conversion-electron yield per parent decay. */
constexpr long double k_conversion_172_85307_keV_yield{0.00013644L};

/*! \brief Selected line energies in canonical integer micro-eV. */
constexpr std::array<std::uint64_t, 6U>
  k_conversion_172_85307_keV_energies_micro_eV{
    {
      124'334'170'000ULL,
      164'801'170'000ULL,
      165'236'070'000ULL,
      165'876'270'000ULL,
      171'415'970'000ULL,
      172'619'750'000ULL,
    },
};

/*! \brief Absolute line yields per parent decay; relative weights for
 * conditional sampling. */
constexpr std::array<double, 6U> k_conversion_172_85307_keV_line_yields{
  {
    0.000107,
    0.0000129,
    0.0000057,
    0.0000044,
    0.00000511,
    0.00000133,
  },
};

} // namespace

// =============================================================================
// =============================================================================

[[nodiscard]] auto BuildGd153Radionuclide() -> GGEMSRadionuclideDefinition {
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
                         k_conversion_14_06383_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_14_06383_keV_energies_micro_eV,
                           k_conversion_14_06383_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_19_81296_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_19_81296_keV_energies_micro_eV,
                           k_conversion_19_81296_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_69_673_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_69_673_keV_energies_micro_eV,
                           k_conversion_69_673_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_75_42213_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_75_42213_keV_energies_micro_eV,
                           k_conversion_75_42213_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_83_36717_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_83_36717_keV_energies_micro_eV,
                           k_conversion_83_36717_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_89_48595_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_89_48595_keV_energies_micro_eV,
                           k_conversion_89_48595_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_97_431_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_97_431_keV_energies_micro_eV,
                           k_conversion_97_431_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_103_18012_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_103_18012_keV_energies_micro_eV,
                           k_conversion_103_18012_keV_line_yields));

  emissions.emplace_back(particles::GGEMSParticleType::Electron,
                         k_conversion_172_85307_keV_yield,
                         sources::GGEMSEnergyDistribution::BuildDiscreteLines(
                           k_conversion_172_85307_keV_energies_micro_eV,
                           k_conversion_172_85307_keV_line_yields));

  return {"Gd-153", k_half_life_seconds, std::move(emissions)};
}

} // namespace ggems::core::radioactivity::builtins
