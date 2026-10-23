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

  /*! \brief Local Box with a separate FP32 pose. */
  Box = 1,

  /*! \brief Full ball with FP32 center and radius. */
  Sphere = 3,
};

/*! \brief Scalar 28-byte Sphere parameters, identities and tolerance. */
struct GGEMSSphereRecord {
  /*! \brief Global center X in pm. */
  float center_x_pm{0.0F};

  /*! \brief Global center Y in pm. */
  float center_y_pm{0.0F};

  /*! \brief Global center Z in pm. */
  float center_z_pm{0.0F};

  /*! \brief Strictly positive radius in pm. */
  float radius_pm{0.0F};

  /*! \brief Snapshot-local physical owner. */
  std::uint32_t volume_id{1};

  /*! \brief Exact dense snapshot-local Material identity. */
  std::uint32_t material_id{0};

  /*! \brief Half-width of the closed radial surface band in FP32 pm. */
  float tolerance_pm{0.0F};
};

/*! \brief Stores only the shape payload selected by the enclosing tag. */
union GGEMSAnalyticParameters {
  /*! \brief Local Box half extents and owner identities. */
  GGEMSBoxRecord box{};

  /*! \brief Full Sphere; no rotation is stored or allocated. */
  GGEMSSphereRecord sphere;
};

/*!
 * \brief Scalar 32-byte scene record, including the no-volume case.
 *
 * Mirrors GGEMSAnalyticVolumeRecord.clh. Every Box uses the separately
 * supplied GGEMSBoxPoseRecord. All programs receive this same scene layout.
 */
struct GGEMSAnalyticVolumeRecord {
  /*! \brief Predicate and active union member. */
  GGEMSAnalyticKind kind{GGEMSAnalyticKind::None};

  /*! \brief Shape parameters; box is active for None and Box. */
  GGEMSAnalyticParameters parameters{};
};

} // namespace ggems::geometry
