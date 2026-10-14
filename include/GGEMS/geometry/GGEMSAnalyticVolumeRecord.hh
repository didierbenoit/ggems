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
 * \brief Defines the single analytic volume shared with OpenCL.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>

#include "GGEMS/geometry/GGEMSBoxRecord.hh"

namespace ggems::geometry {

/*! \brief Selects the implemented geometric predicate for the single volume. */
enum class GGEMSAnalyticKind : std::uint32_t {
  /*! \brief World-only scene. */
  None = 0,

  /*! \brief Exact global axis-aligned Box faces. */
  Box = 1,

  /*! \brief Local Box faces with a separately allocated FP64 pose. */
  RotatedBox = 2,

  /*! \brief Full ball with integer center and radius. */
  Sphere = 3,
};

/*! \brief Scalar 40-byte Sphere parameters and owner identities. */
struct GGEMSSphereRecord {
  /*! \brief Global center X in pm. */
  std::int64_t center_x_pm{0};

  /*! \brief Global center Y in pm. */
  std::int64_t center_y_pm{0};

  /*! \brief Global center Z in pm. */
  std::int64_t center_z_pm{0};

  /*! \brief Strictly positive radius in pm. */
  std::uint64_t radius_pm{0};

  /*! \brief Snapshot-local physical owner. */
  std::uint32_t volume_id{1};

  /*! \brief Exact dense snapshot-local Material identity. */
  std::uint32_t material_id{0};
};

/*! \brief Stores only the shape payload selected by the enclosing tag. */
union GGEMSAnalyticParameters {
  /*! \brief Exact global or centered local faces and owner identities. */
  GGEMSBoxRecord box{};

  /*! \brief Full Sphere; no rotation is stored or allocated. */
  GGEMSSphereRecord sphere;
};

/*!
 * \brief Scalar 64-byte scene record, including the no-volume case.
 *
 * Mirrors GGEMSAnalyticVolumeRecord.clh. Only RotatedBox uses the separately
 * supplied GGEMSBoxPoseRecord. All programs receive this same scene layout.
 */
struct GGEMSAnalyticVolumeRecord {
  /*! \brief Predicate and active union member. */
  GGEMSAnalyticKind kind{GGEMSAnalyticKind::None};

  /*! \brief Explicit alignment padding, initialized to zero. */
  std::uint32_t padding{0};

  /*! \brief Shape parameters; box is active for None, Box, and RotatedBox. */
  GGEMSAnalyticParameters parameters{};
};

} // namespace ggems::geometry
