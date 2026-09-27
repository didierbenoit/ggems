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
 * \brief Defines the packed descriptor for one source energy distribution.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>

#include "GGEMS/sources/GGEMSSourceTypes.hh"

namespace ggems::core::sources {

/*!
 * \brief Locates one source energy law in shared host/device arrays.
 *
 * Monoenergetic energy is carried by the source or emission record. Tabulated
 * laws use aligned energy, relative-weight, and cumulative-ticket arrays;
 * offsets count entries, not bytes.
 */
struct GGEMSEnergyDistributionRecord {
  /*! \brief Common spectrum-bin width in micro-eV; zero for lines and mono. */
  std::uint64_t regular_bin_width_micro_eV{0ULL};

  /*!
   * \brief Element offset into the parallel packed energy and ticket arrays.
   */
  std::uint64_t table_offset{0ULL};

  /*! \brief Device encoding of the energy distribution kind. */
  std::uint32_t distribution_type{
    ToKernelEnergyDistributionType(GGEMSEnergyDistributionType::Unknown)};

  /*! \brief Number of table entries; zero for a monoenergetic distribution. */
  std::uint32_t table_count{0U};
};

} // namespace ggems::core::sources
