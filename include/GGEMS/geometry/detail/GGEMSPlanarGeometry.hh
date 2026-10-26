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
 * \brief Prepares the fixed four side planes of authored Trd and Trap solids.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include "GGEMS/geometry/GGEMSAnalyticVolumeRecord.hh"

namespace ggems::geometry {

class GGEMSTrd;
class GGEMSTrap;

namespace detail {

/*!
 * \brief Packs normalized Trd sides and the two implicit Z caps.
 * \param[in] solid Complete positive dimensions.
 * \return Local FP32 shape parameters, before scene identities and tolerance.
 */
[[nodiscard]] auto BuildPlanarRecord(GGEMSTrd const &solid) noexcept
  -> GGEMSPlanarRecord;

/*!
 * \brief Packs normalized Trap sides from its admitted planar end faces.
 * \param[in] solid Complete positive dimensions and admitted side planarity.
 * \return Local FP32 shape parameters, before scene identities and tolerance.
 */
[[nodiscard]] auto BuildPlanarRecord(GGEMSTrap const &solid) noexcept
  -> GGEMSPlanarRecord;

/*!
 * \brief Checks the authored Trap vertices against its packed side planes.
 * \param[in] solid Complete dimensions, inclination and skew.
 * \throws core::GGEMSRecoverable If side deviations exceed 16 local FP32
 * spacings.
 */
auto RequirePlanarTrap(GGEMSTrap const &solid) -> void;

} // namespace detail
} // namespace ggems::geometry
