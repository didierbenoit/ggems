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
 * \brief Declares the GGEMS OpenCL runtime manager.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>

namespace ggems::core {

/*!
 * \brief Defines the source emission time window for a simulation run.
 *
 * Sources emit particles within this window. Both bounds are expressed in
 * picoseconds on the simulation clock: start_ps is included, while stop_ps
 * is excluded.
 */
struct GGEMSTimeWindow {
  std::uint64_t start_ps{0ULL}; /*!< Inclusive start time in picoseconds. */
  std::uint64_t stop_ps{0ULL};  /*!< Exclusive stop time in picoseconds. */

  /*!
   * \brief Compares both time-window endpoints for equality.
   *
   * \return True if both windows have identical start and stop times;
   *         false otherwise.
   */
  auto operator==(GGEMSTimeWindow const &) const -> bool = default;
};

} // namespace ggems::core
