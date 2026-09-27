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
 * \brief Defines the two source population authoring modes.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>
#include <memory>
#include <variant>

#include "GGEMS/units/GGEMSActivityUnits.hh"

namespace ggems::core::radioactivity {
class GGEMSRadionuclideDefinition;
}

namespace ggems::core::sources {

/*!
 * \brief Selects explicit primary counts or activity-driven emission counts.
 */
enum class GGEMSSourcePopulationMode : std::uint8_t {
  /*! \brief Uses a configured count with births at the run-window start. */
  CountDriven = 0U,

  /*!
   * \brief Samples independent Poisson emission counts from decaying activity.
   */
  ActivityDriven,
};

/*! \brief Stores the explicit count for one source slot. */
struct GGEMSCountDrivenSourceConfiguration {
  /*! \brief Number of primaries requested for each run, including zero. */
  std::uint64_t primary_count{4096ULL};
};

/*!
 * \brief Configures independent marginal emissions of a radioactive source.
 *
 * Each emission expectation is the integrated parent activity times its yield.
 * Emission groups are not normalized into one categorical distribution and do
 * not represent correlated products from individually simulated decays.
 */
struct GGEMSActivityDrivenSourceConfiguration {
  /*! \brief Shared immutable definition of independent emission populations. */
  std::shared_ptr<radioactivity::GGEMSRadionuclideDefinition const>
    radionuclide;

  /*! \brief Parent activity in Bq at reference_time_ps. */
  units::Activity activity_at_reference_time{};

  /*! \brief Absolute simulation reference time in picoseconds. */
  std::uint64_t reference_time_ps{0ULL};
};

/*! \brief Owns either count-based or activity-based population settings. */
using GGEMSSourcePopulationConfiguration =
  std::variant<GGEMSCountDrivenSourceConfiguration,
               GGEMSActivityDrivenSourceConfiguration>;

} // namespace ggems::core::sources
