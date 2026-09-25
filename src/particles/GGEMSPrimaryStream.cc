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
 * \brief Implements unchecked sequential primary-identifier reservations.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <cstdint>

#include "GGEMS/particles/GGEMSPrimaryStream.hh"

namespace ggems::core::particles {

// =============================================================================
// =============================================================================

auto GGEMSPrimaryStream::PrepareRun(std::uint64_t run_id,
                                    std::uint64_t primary_count)
  -> GGEMSPrimaryStreamRunView {
  GGEMSPrimaryStreamRunView const run_view{
    .run_id = run_id,
    .source_primary_count = primary_count,
    .global_history_offset = next_global_primary_id_,
  };

  next_global_primary_id_ += primary_count;

  return run_view;
}

} // namespace ggems::core::particles
