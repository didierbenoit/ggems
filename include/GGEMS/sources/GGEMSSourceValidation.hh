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
 * \brief Checks analytic source shapes, angular domains, and focus separation.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>

#include "GGEMS/sources/GGEMSSourceRecord.hh"

namespace ggems::core::sources {

/*!
 * \brief Bounds local emission support including a binary32 sampling margin.
 */
struct GGEMSEmissionBounds {
  /*! \brief Conservative transformed component radius in integer pm. */
  std::uint64_t component_radius_pm{0ULL};

  /*! \brief Local mathematical x half-extent in pm. */
  long double half_extent_x_pm{0.0L};

  /*! \brief Local mathematical y half-extent in pm. */
  long double half_extent_y_pm{0.0L};

  /*! \brief Local mathematical z half-extent in pm. */
  long double half_extent_z_pm{0.0L};
};

/*!
 * \brief Checks whether a symmetric pm interval fits signed coordinates.
 *
 * \param[in] center_pm Signed interval center in pm.
 * \param[in] radius_pm Nonnegative half-width in pm.
 * \return True if both center minus radius and center plus radius fit int64.
 */
[[nodiscard]] auto HasSignedPicoMeterEnvelope(std::int64_t center_pm,
                                              std::uint64_t radius_pm) noexcept
  -> bool;

/*!
 * \brief Computes conservative support bounds for an analytic shape.
 *
 * The caller supplies dimensions whose expanded radius fits uint64. This helper
 * checks shape dimensions, not the source position envelope.
 *
 * \param[in] record Source dimensions and finite orientation basis.
 * \return Local half-extents and conservative transformed integer component
 * radius.
 * \throws GGEMSRecoverable If the shape is unsupported or its dimensions are
 * invalid.
 */
[[nodiscard]] auto BuildEmissionBounds(GGEMSSourceRecord const &record)
  -> GGEMSEmissionBounds;

/*!
 * \brief Recognizes the exact stored full-sphere angular bounds.
 *
 * \param[in] record Source record to inspect.
 * \return True for exact default cosine and azimuth values, regardless of mode.
 */
[[nodiscard]] auto
IsDefaultFullSphereIsotropicDomain(GGEMSSourceRecord const &record) noexcept
  -> bool;

/*!
 * \brief Checks the implemented analytic shape, frame, and angular contracts.
 *
 * Checks source kind, frame, shape dimensions, the Unknown angular sentinel,
 * isotropic domains, and focused-support separation. It does not validate
 * particle kind, energy tables, time order, or the signed global position
 * envelope.
 *
 * \param[in] record Candidate shared source record.
 * \throws GGEMSRecoverable If one of the checked analytic contracts fails.
 */
auto ValidateAnalyticSourceRecord(GGEMSSourceRecord const &record) -> void;

} // namespace ggems::core::sources
