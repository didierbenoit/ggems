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
 * \brief Defines independent per-occurrence display overrides and defaults.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>
#include <optional>

namespace ggems::render {

struct GGEMSGeometryOccurrence;

/*! \brief Selects display-only visibility and topology for one occurrence. */
enum class GGEMSGeometryDisplayMode : std::uint8_t {
  /*! \brief Omits this occurrence without hiding its physical children. */
  None,

  /*! \brief Draws the edges of the shared procedural triangles. */
  Wireframe,

  /*! \brief Fills the shared procedural triangles opaquely, except for World.
   */
  Solid,
};

/*!
 * \brief Stores an explicit display mode separately from hierarchy defaults.
 */
struct GGEMSGeometryAppearance {
  /*! \brief Explicit mode, or no override to follow the physical hierarchy. */
  std::optional<GGEMSGeometryDisplayMode> mode_override;
};

/*!
 * \brief Resolves one occurrence's mode without inheriting its parent's choice.
 *
 * World and physical mothers default to Wireframe; physical leaves default to
 * Solid. World never resolves to Solid, including when explicitly requested.
 * \param[in] occurrence Physical identity and snapshot-built child links.
 * \param[in] appearance Explicit choices for this occurrence alone.
 * \return Effective display mode, independent of scientific visibility.
 */
[[nodiscard]] auto
ResolveGeometryDisplayMode(GGEMSGeometryOccurrence const &occurrence,
                           GGEMSGeometryAppearance const &appearance) noexcept
  -> GGEMSGeometryDisplayMode;

} // namespace ggems::render
