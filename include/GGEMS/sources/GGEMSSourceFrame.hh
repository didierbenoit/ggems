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
 * \brief Constructs and checks local-to-global source orientation frames.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <array>

#include "GGEMS/geometry/GGEMSGeometryTypes.hh"

namespace ggems::core::sources {
/*! \brief Rejection threshold for 1 - abs(dot(direction, up)). */
inline constexpr double k_source_frame_parallel_tolerance{1.0e-6};

/*! \brief Tolerance for unit length, orthogonality, and handedness checks. */
inline constexpr double k_source_frame_float_tolerance{1.0e-5};

/*! \brief Stores a right-handed orthonormal source basis in binary32. */
struct GGEMSSourceFrame {
  /*! \brief Unit local horizontal axis expressed in the global frame. */
  geometry::Direction3 axis_x{.x = 1.0F, .y = 0.0F, .z = 0.0F};

  /*! \brief Unit local vertical axis expressed in the global frame. */
  geometry::Direction3 axis_y{.x = 0.0F, .y = 1.0F, .z = 0.0F};

  /*! \brief Unit local forward axis expressed in the global frame. */
  geometry::Direction3 axis_z{.x = 0.0F, .y = 0.0F, .z = 1.0F};
};

/*!
 * \brief Checks the finite binary32 frame against geometric tolerances.
 *
 * \param[in] frame Basis to inspect.
 * \return True for unit, mutually orthogonal axes with positive unit
 * handedness.
 */
[[nodiscard]] auto IsValidSourceFrame(GGEMSSourceFrame const &frame) noexcept
  -> bool;

/*!
 * \brief Builds a right-handed basis from a direction and an up reference.
 *
 * \param[in] direction Finite nonzero global forward vector; normalized
 * internally.
 * \param[in] up_reference Finite nonzero global vector defining the vertical
 * half-plane.
 * \return Binary32 axes with x = normalize(up cross direction) and y = z cross
 * x.
 * \throws GGEMSRecoverable If normalization fails or the input axes are too
 * nearly parallel.
 */
[[nodiscard]] auto BuildSourceFrame(std::array<double, 3U> const &direction,
                                    std::array<double, 3U> const &up_reference)
  -> GGEMSSourceFrame;

/*!
 * \brief Builds a source basis with an automatically selected up vector.
 *
 * \param[in] direction Finite nonzero global forward vector.
 * \return Basis using global +Z as up, or +Y when +Z is too nearly parallel.
 * \throws GGEMSRecoverable If the direction cannot be normalized.
 */
[[nodiscard]] auto
BuildSourceFrameWithAutomaticUp(std::array<double, 3U> const &direction)
  -> GGEMSSourceFrame;

} // namespace ggems::core::sources
