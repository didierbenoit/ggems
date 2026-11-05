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
 * \brief Selects the physical grid interval shared by the shader and ImGui.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <array>
#include <cmath>

#include "GGEMS/ui/detail/GGEMSCartesianGrid.hh"
#include "GGEMS/ui/detail/GGEMSSceneCamera.hh"

namespace {

/*!
 * \brief Rounds upward to a physical 1-2-5 times power-of-ten interval.
 *
 * \param[in] value Positive requested interval in meters.
 * \return Smallest sequence value at least as large as the request.
 */
[[nodiscard]] auto NiceCeiling(double value) noexcept -> float {
  double const decade = std::pow(10.0, std::floor(std::log10(value)));

  for (double multiplier : std::array{1.0, 2.0, 5.0}) {
    double const candidate = decade * multiplier;
    if (value <= candidate) {
      return static_cast<float>(candidate);
    }
  }
  return static_cast<float>(decade * 10.0F);
}

} // namespace

namespace ggems::ui::detail {

auto GGEMSCartesianGrid::Update(GGEMSSceneCamera const &camera) noexcept
  -> void {
  auto const state = camera.GetDiagnostics();
  double const pixels_per_meter = 0.5 * state.zoom * state.viewport_height;
  double const spacing = minor_step_m_ * pixels_per_meter;

  if (spacing < 16.0 || spacing > 72.0) {
    minor_step_m_ = NiceCeiling(24.0 / pixels_per_meter);
  }
}

// -----------------------------------------------------------------------------

auto GGEMSCartesianGrid::GetMinorStepM() const noexcept -> float {
  return minor_step_m_;
}

} // namespace ggems::ui::detail
