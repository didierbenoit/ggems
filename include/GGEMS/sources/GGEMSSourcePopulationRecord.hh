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
 * \brief Defines the shared per-source population and birth-time descriptor.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>

#include "GGEMS/sources/GGEMSSourcePopulation.hh"

namespace ggems::core::sources {

/*!
 * \brief Converts a population mode to its shared device encoding.
 *
 * \param[in] mode Host population mode.
 * \return Underlying unsigned mode value; no validation is performed.
 */
[[nodiscard]] constexpr auto
ToKernelSourcePopulationMode(GGEMSSourcePopulationMode mode) noexcept
  -> std::uint32_t {
  return static_cast<std::uint32_t>(mode);
}

/*! \brief Supplies a source slot population mode and radioactive time law. */
struct GGEMSSourcePopulationRecord {
  /*!
   * \brief Device encoding selecting count-driven or activity-driven births.
   */
  std::uint32_t population_mode{
    ToKernelSourcePopulationMode(GGEMSSourcePopulationMode::CountDriven)};

  /*! \brief First entry in the packed emission descriptor and range arrays. */
  std::uint32_t first_emission_index{0U};

  /*! \brief Number of radioactive emission groups; zero for CountDriven. */
  std::uint32_t emission_count{0U};

  /*! \brief Dimensionless ln(2) * window duration / half-life in binary32. */
  float scaled_decay{0.0F};
};

} // namespace ggems::core::sources
