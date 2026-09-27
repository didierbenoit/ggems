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
 * \brief Builds radioactive energy laws from integrated regular-bin masses.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>
#include <span>

#include "GGEMS/sources/GGEMSEnergyDistribution.hh"

/*!
 * \namespace ggems::core::radioactivity::detail
 * \brief Prepares internal tabulated radioactive source spectra.
 */
namespace ggems::core::radioactivity::detail {

/*! \brief Describes an exact regular-bin grid in canonical energy ticks. */
struct TabulatedSpectrumGrid {
  /*! \brief Strictly positive lower edge of the first bin, in micro-eV. */
  std::uint64_t lower_edge_micro_eV{0ULL};

  /*! \brief Positive even width shared by all bins, in micro-eV. */
  std::uint64_t bin_width_micro_eV{0ULL};
};

/*!
 * \brief Constructs canonical centers from a lower edge and common width.
 *
 * Center i is lower_edge + width/2 + i*width. The caller supplies an even width
 * and arithmetic that fits; this helper does not check overflow before forming
 * centers.
 *
 * \param[in] grid Grid whose centers and upper edge must fit uint64 micro-eV.
 * \param[in] relative_bin_weights At least two finite nonnegative integrated
 * bin masses.
 * \return Regular-spectrum distribution with quantized selection tickets.
 * \throws GGEMSRecoverable If regular-spectrum construction rejects centers or
 * weights.
 */
[[nodiscard]] auto
BuildTabulatedSpectrum(TabulatedSpectrumGrid grid,
                       std::span<double const> relative_bin_weights)
  -> sources::GGEMSEnergyDistribution;

} // namespace ggems::core::radioactivity::detail
