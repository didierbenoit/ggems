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
 * \brief Defines the compiled Ru-106 marginal source-emission laws.
 *
 * LNHB: A. Arinc, NPL, April 2013. Selected beta-minus source emissions.
 *
 * BetaShape 2.4 (06/2024): calculated transition shapes. Conditional bin
 * masses integrate piecewise-linear reference densities; full-support
 * references and finite grids remain distinct.
 *
 * Physical yields remain independent emissions per parent decay. EC creates
 * no primary; daughter chains and unsupported Auger energy laws are
 * excluded.
 *
 * Selection and exclusions:
 * validation/radioactivity/data/Ru-106/reference/.
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
#include "GGEMS/radioactivity/detail/GGEMSTabulatedSpectrum.hh"

namespace ggems::core::radioactivity::builtins {
namespace {

// =============================================================================
// =============================================================================

/*! \brief Evaluated Ru-106 parent half-life in seconds. */
constexpr long double k_half_life_seconds{32097600.0L};

// =============================================================================
// =============================================================================

/*! \brief beta minus 39 40 keV yield per parent decay. */
constexpr long double k_beta_minus_39_40_keV_yield{1.0L};

/*! \brief First bin lower edge in micro-eV; full reference support starts at
 * zero. */
constexpr std::uint64_t k_beta_minus_39_40_keV_lower_edge_micro_eV{14'000ULL};

/*! \brief Regular bin width in micro-eV for the 39.40 keV endpoint. */
constexpr std::uint64_t k_beta_minus_39_40_keV_bin_width_micro_eV{
  498'734'000ULL};

/*! \brief Conditional integrated bin masses; physical branch yield is stored
 * separately. */
constexpr std::array<double, 79U> k_beta_minus_39_40_keV_weights{
  {
    0.03932003281207043,    0.037593480244926626,   0.036136621302612977,
    0.034919399662649696,   0.033824940010832132,   0.032804342980433908,
    0.031833693587099143,   0.030900454669931951,   0.029996913887722874,
    0.0291190098151719,     0.028263609999442607,   0.027428738006839544,
    0.026612794540122633,   0.025814322284650922,   0.025031570954222836,
    0.024263648849085339,   0.023509110483687042,   0.022767577284770347,
    0.022038671179741082,   0.02132248957189015,    0.020619542572000625,
    0.019929832358395381,   0.019252910510382294,   0.018588558489347626,
    0.017936273885094098,   0.017295469779198901,   0.016665767221225255,
    0.016047244919812846,   0.015440319005741898,   0.014845190603843565,
    0.014262062572427587,   0.013690897664176624,   0.013131384232838633,
    0.012583154265566981,   0.012046084542905957,   0.011520053588952655,
    0.011005200388068008,   0.010501788730939555,   0.010009992610952003,
    0.0095299435025010624,  0.009061465824868364,   0.0086045021189330457,
    0.0081588606072141561,  0.0077244180153842488,  0.0073012872278918851,
    0.006889615000337598,   0.0064895617379982706,  0.0061012580733624013,
    0.0057246859039068645,  0.0053597970643040642,  0.0050064788799394329,
    0.0046647154587131636,  0.0043345295815239594,  0.0040160416775111545,
    0.0037093925045503001,  0.0034146483501027174,  0.0031318568587392938,
    0.0028610118291252824,  0.0026020776498478573,  0.0023550640635023636,
    0.0021200189793769459,  0.001897018190431591,   0.0016861494155629994,
    0.0014874840522397319,  0.0013010598366634165,  0.0011269067473901123,
    0.00096505225354547616, 0.00081553320984776917, 0.00067840177724121477,
    0.00055371483084210797, 0.00044154122782184146, 0.00034193416411725853,
    0.00025494490608333841, 0.0001806222551909826,  0.00011901384475605113,
    7.0173105409972763e-05, 3.4156263073281519e-05, 1.1020175536603217e-05,
    8.9276483719581907e-07,
  },
};

} // namespace

// =============================================================================
// =============================================================================

[[nodiscard]] auto BuildRu106Radionuclide() -> GGEMSRadionuclideDefinition {
  std::vector<GGEMSRadionuclideEmission> emissions;
  emissions.reserve(1U);

  emissions.emplace_back(
    particles::GGEMSParticleType::Electron, k_beta_minus_39_40_keV_yield,
    detail::BuildTabulatedSpectrum(
      {
        .lower_edge_micro_eV = k_beta_minus_39_40_keV_lower_edge_micro_eV,
        .bin_width_micro_eV = k_beta_minus_39_40_keV_bin_width_micro_eV,
      },
      k_beta_minus_39_40_keV_weights));

  return {"Ru-106", k_half_life_seconds, std::move(emissions)};
}

} // namespace ggems::core::radioactivity::builtins
