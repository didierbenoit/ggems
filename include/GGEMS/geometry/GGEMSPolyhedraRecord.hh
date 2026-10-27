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
 * \brief Defines compact Polyhedra metadata and immutable scalar profile
 * records.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>

namespace ggems::geometry {

/*!
 * \brief Stores scalar 20-byte metadata for an immutable polygonal profile.
 *
 * The separate auxiliary byte buffer contains one 48-byte
 * GGEMSBoxPoseRecord, side_count consecutive 8-byte side records, then
 * section_count consecutive 32-byte section records. All fields are
 * 32-bit scalars; host and OpenCL field order and alignment must match.
 * Counts describe one physical volume and one Material across all sections.
 */
struct GGEMSPolyhedraRecord {
  /*! \brief Number of regular polygon sides. */
  std::uint32_t side_count{0};

  /*! \brief Number of positive-height axial sections. */
  std::uint32_t section_count{0};

  /*! \brief Snapshot-local physical owner. */
  std::uint32_t volume_id{1};

  /*! \brief Exact dense Material identity. */
  std::uint32_t material_id{0};

  /*! \brief Automatic normal-distance surface half-band in pm. */
  float tolerance_pm{0.0F};
};

/*! \brief Scalar 8-byte precomputed polygon side normal. */
struct GGEMSPolyhedraSideRecord {
  /*! \brief Local XY outward unit-normal X component. */
  float normal_x{0.0F};

  /*! \brief Local XY outward unit-normal Y component. */
  float normal_y{0.0F};
};

/*!
 * \brief Stores scalar 32-byte coefficients for one continuous axial section.
 *
 * The outer apothem at local z is outer_radius_pm + outer_slope *
 * (z - z_lower_pm); the inner apothem follows the corresponding formula.
 * Slopes are dimensionless. Normal lengths convert a perpendicular
 * surface half-band in pm into a side-plane offset. Adjacent sections
 * meet at their shared Z plane without an authored radial step.
 */
struct GGEMSPolyhedraSectionRecord {
  /*! \brief Lower local Z coordinate in pm. */
  float z_lower_pm{0.0F};

  /*! \brief Upper local Z coordinate in pm. */
  float z_upper_pm{0.0F};

  /*! \brief Outer apothem at the lower plane in pm. */
  float outer_radius_pm{0.0F};

  /*! \brief Outer apothem change per local Z distance. */
  float outer_slope{0.0F};

  /*! \brief Length of the unnormalized outer plane normal. */
  float outer_normal_length{1.0F};

  /*! \brief Inner apothem at the lower plane in pm. */
  float inner_radius_pm{0.0F};

  /*! \brief Inner apothem change per local Z distance. */
  float inner_slope{0.0F};

  /*! \brief Length of the unnormalized inner plane normal. */
  float inner_normal_length{1.0F};
};

} // namespace ggems::geometry
