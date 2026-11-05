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
 * \brief Defines bounded world-origin Cartesian grid sampling for the UI
 * renderer.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <array>
#include <cstdint>

#include "GGEMS/ui/detail/GGEMSSceneCamera.hh"

namespace ggems::ui::detail {

/*!
 * \brief Maintains independent reference grids and their adaptive physical
 * steps.
 */
class GGEMSCartesianGrid {
public:
  /*!
   * \brief Describes two procedural line families in one world-origin plane.
   */
  struct PlaneLines {
    /*! \brief Minor lattice spacing in meters; zero when unavailable. */
    double minor_step_m{};

    /*! \brief Major spacing in meters, equal to five minor intervals. */
    double major_step_m{};

    /*! \brief Lower physical bounds along the two in-plane world axes. */
    std::array<double, 2> minimum_m{};

    /*! \brief Upper physical bounds along the two in-plane world axes. */
    std::array<double, 2> maximum_m{};

    /*! \brief First signed origin-relative lattice index in each family. */
    std::array<std::int32_t, 2> first_index{};

    /*! \brief Number of lines at successive lattice indices in each family. */
    std::array<std::uint32_t, 2> line_count{};

    /*! \brief Visibility weight reducing contrast near edge-on projection. */
    float opacity{};
  };

  /*! \brief Independent XY, XZ, and YZ visibility; all hidden initially. */
  std::array<bool, 3> visible{};

  /*! \brief Cursor reference plane, retained until explicitly changed. */
  GGEMSSceneCamera::ViewPlane reference_plane{GGEMSSceneCamera::ViewPlane::XY};

  /*!
   * \brief Updates origin-anchored steps and bounded visible World intervals.
   *
   * XY uses axes X/Y, XZ uses X/Z, and YZ uses Y/Z. Each plane emits at most
   * 256 lines. Hidden planes are also updated for spacing diagnostics.
   *
   * \param[in] camera Current logical viewport and shared projection.
   * \param[in] world_half_extent_m Positive World half extents in meters.
   */
  auto Update(GGEMSSceneCamera const &camera,
              std::array<double, 3> const &world_half_extent_m) noexcept
    -> void;

  /*!
   * \brief Reads the current procedural line descriptions.
   *
   * \return XY, XZ, and YZ descriptions in that order.
   */
  [[nodiscard]] auto GetPlanes() const noexcept
    -> std::array<PlaneLines, 3> const &;

private:
  /*! \brief Current physical steps, retained between updates for hysteresis. */
  std::array<PlaneLines, 3> planes_{};
};

} // namespace ggems::ui::detail
