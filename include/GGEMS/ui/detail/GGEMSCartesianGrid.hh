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
 * \brief Shares grid visibility, cursor plane, and physical spacing with UI.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <array>

#include "GGEMS/ui/detail/GGEMSSceneCamera.hh"

namespace ggems::ui::detail {

/*! \brief Shares physical spacing across independently visible grids. */
class GGEMSCartesianGrid {
public:
  /*! \brief Independent XY, XZ, and YZ visibility; all hidden initially. */
  std::array<bool, 3U> visible{};

  /*! \brief Last selected axial cursor plane; visibility does not change it. */
  GGEMSSceneCamera::ViewPlane reference_plane{GGEMSSceneCamera::ViewPlane::XY};

  /*!
   * \brief Chooses one physical 1-2-5 interval with zoom hysteresis.
   *
   * This scalar is shared by the shader and ImGui. Projection, clipping,
   * density filtering, and line appearance remain shader responsibilities.
   *
   * \param[in] camera Current zoom and logical viewport height.
   */
  auto Update(GGEMSSceneCamera const &camera) noexcept -> void;

  /*!
   * \brief Reads the exact minor spacing supplied to the grid shader.
   *
   * \return Minor spacing in meters; zero before the first update.
   */
  [[nodiscard]] auto GetMinorStepM() const noexcept -> float;

private:
  /*! \brief Shared shader interval, retained between updates for hysteresis. */
  float minor_step_m_{};
};

} // namespace ggems::ui::detail
