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
 * \brief Resolves independent physical hierarchy display modes.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include "GGEMS/render/GGEMSGeometryAppearance.hh"

#include "GGEMS/render/GGEMSGeometryScene.hh"

namespace ggems::render {

auto ResolveGeometryDisplayMode(
  GGEMSGeometryOccurrence const &occurrence,
  GGEMSGeometryAppearance const &appearance) noexcept
  -> GGEMSGeometryDisplayMode {
  if (occurrence.volume_id == 0U) {
    return appearance.mode_override == GGEMSGeometryDisplayMode::None
             ? GGEMSGeometryDisplayMode::None
             : GGEMSGeometryDisplayMode::Wireframe;
  }

  return appearance.mode_override.value_or(
    occurrence.children.empty() ? GGEMSGeometryDisplayMode::Solid
                                : GGEMSGeometryDisplayMode::Wireframe);
}

} // namespace ggems::render
