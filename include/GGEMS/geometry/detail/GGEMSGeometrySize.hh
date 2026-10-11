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
 * \brief Shares the even-size lattice rule of centered analytic volumes.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <string_view>

#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

/*!
 * \namespace ggems::geometry::detail
 * \brief Provides internal geometry authoring helpers.
 */
namespace ggems::geometry::detail {

/*!
 * \brief Converts one authored full size of a centered volume into its half
 * extent.
 *
 * The full size must be strictly positive and an even number of picometers so
 * that both faces, center -/+ half extent, lie on the integer picometer
 * lattice. Any further representability cap belongs to the calling volume.
 *
 * \param[in] size Full extent along the axis.
 * \param[in] volume Volume label used in diagnostics, such as "Box".
 * \param[in] axis Axis label used in diagnostics.
 * \return Half extent in picometers.
 * \throws GGEMSRecoverable If the size is zero or odd.
 */
[[nodiscard]] auto RequireEvenHalfExtentPM(units::Length size,
                                           std::string_view volume,
                                           std::string_view axis) -> DistancePM;

} // namespace ggems::geometry::detail
