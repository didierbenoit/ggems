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
 * \brief Maps particle kinds to the shared diagnostic display palette.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/render/GGEMSColorNames.hh"
#include "GGEMS/render/GGEMSColor.hh"
#include "GGEMS/render/GGEMSColorTypes.hh"

namespace ggems::render {

/*!
 * \brief Selects the display color associated with a particle kind.
 *
 * \param[in] particle_type Particle kind to display.
 * \return Named foreground key; unsupported kinds use faint Neutral200 gray.
 */
constexpr auto
GetParticleColorKey(core::particles::GGEMSParticleType particle_type) noexcept
  -> ColorKey {
  using core::particles::GGEMSParticleType;

  switch (particle_type) {
  case GGEMSParticleType::Aionino:
    return MAGENTA_Electric;
  case GGEMSParticleType::Gamma:
    return GREEN_SeaGreen;
  case GGEMSParticleType::Electron:
    return CYAN_Cryo;
  case GGEMSParticleType::Positron:
    return MAGENTA_LightMagenta;
  case GGEMSParticleType::Proton:
    return RED_Tomato;
  case GGEMSParticleType::Neutron:
    return GRAY_Silver;
  case GGEMSParticleType::Alpha:
    return YELLOW_Gold;
  case GGEMSParticleType::Unknown:
  default:
    return GRAY_Neutral200_F;
  }
}

/*!
 * \brief Resolves the display RGB value for a particle kind.
 *
 * \param[in] particle_type Particle kind to display.
 * \return Eight-bit RGB channels from the shared palette and selected variant.
 */
constexpr auto
GetParticleRGB(core::particles::GGEMSParticleType particle_type) noexcept
  -> RGB {
  ColorKey const color = GetParticleColorKey(particle_type);
  return GetColorRGB(color.family, color.shade, color.variant);
}

} // namespace ggems::render
