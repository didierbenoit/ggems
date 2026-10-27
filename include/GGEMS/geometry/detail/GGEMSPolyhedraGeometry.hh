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
 * \brief Prepares immutable scalar Polyhedra sections, sides, and pose data.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstddef>
#include <vector>

#include "GGEMS/geometry/GGEMSPolyhedra.hh"
#include "GGEMS/geometry/GGEMSPolyhedraRecord.hh"

namespace ggems::geometry::detail {

/*!
 * \brief Packs one section from distinct uploaded endpoint coordinates.
 * \param[in] lower Lower authored profile plane.
 * \param[in] upper Upper authored profile plane.
 * \return Scalar coefficients with normal lengths for surface-band offsets.
 * \pre The uploaded Z coordinates differ and each radial wall is positive.
 */
[[nodiscard]] auto
BuildPolyhedraSection(GGEMSPolyhedraProfile const &lower,
                      GGEMSPolyhedraProfile const &upper) noexcept
  -> GGEMSPolyhedraSectionRecord;

/*!
 * \brief Packs continuous sections using the uploaded FP32 endpoint values.
 * \param[in] polyhedra Complete authored dimensions.
 * \return Scalar section coefficients in increasing local Z order.
 * \throws core::GGEMSRecoverable If packing collapses a section or radial wall.
 */
[[nodiscard]] auto BuildPolyhedraSections(GGEMSPolyhedra const &polyhedra)
  -> std::vector<GGEMSPolyhedraSectionRecord>;

/*!
 * \brief Precomputes XY side normals without per-ray trigonometry.
 * \param[in] polyhedra Configured polygon count and first-vertex angle.
 * \return Side normals in increasing azimuth.
 */
[[nodiscard]] auto BuildPolyhedraSides(GGEMSPolyhedra const &polyhedra)
  -> std::vector<GGEMSPolyhedraSideRecord>;

/*!
 * \brief Packs pose, side normals, then sections into one immutable buffer.
 * \param[in] polyhedra Complete admitted solid.
 * \return Bytes consumed through the existing auxiliary pose kernel argument.
 * \throws core::GGEMSRecoverable If counts or byte size exceed indexed limits.
 */
[[nodiscard]] auto BuildPolyhedraData(GGEMSPolyhedra const &polyhedra)
  -> std::vector<std::byte>;

} // namespace ggems::geometry::detail
