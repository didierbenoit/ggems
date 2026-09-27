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
 * \brief Expands a regular radioactive-spectrum grid into canonical bin
 * centers.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

#include "GGEMS/radioactivity/detail/GGEMSTabulatedSpectrum.hh"
#include "GGEMS/sources/GGEMSEnergyDistribution.hh"

namespace ggems::core::radioactivity::detail {

// =============================================================================
// =============================================================================

[[nodiscard]] auto
BuildTabulatedSpectrum(TabulatedSpectrumGrid grid,
                       std::span<double const> relative_bin_weights)
  -> sources::GGEMSEnergyDistribution {
  std::uint64_t const first_center =
    grid.lower_edge_micro_eV + (grid.bin_width_micro_eV / 2ULL);

  std::vector<std::uint64_t> centers;
  centers.reserve(relative_bin_weights.size());

  for (std::size_t index = 0U; index < relative_bin_weights.size(); ++index) {
    centers.push_back(first_center + (static_cast<std::uint64_t>(index) *
                                      grid.bin_width_micro_eV));
  }

  return sources::GGEMSEnergyDistribution::BuildRegularSpectrum(
    centers, relative_bin_weights);
}

} // namespace ggems::core::radioactivity::detail
