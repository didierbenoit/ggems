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
#include "GGEMS/geometry/GGEMSPolyhedraRecord.hh"

namespace ggems::geometry {

/*! \brief Selects the implemented geometric predicate for the single volume. */
enum class GGEMSAnalyticKind : std::uint32_t {
  /*! \brief World-only scene. */
  None = 0,

  /*! \brief Local Box with a separate FP32 pose. */
  Box = 1,

  /*!
   * \brief Full-azimuth finite cylinder with an optional bore and shared pose.
   */
  Tubs = 2,

  /*! \brief Full ball with FP32 center and radius. */
  Sphere = 3,

  /*! \brief Complete ellipsoid with local semi-axes and shared pose. */
  Ellipsoid = 4,

  /*! \brief Tapered solid with rectangular end faces. */
  Trd = 5,

  /*! \brief General planar-faced trapezoid. */
  Trap = 6,

  /*! \brief Full-azimuth solid or hollow conical frustum. */
  Cons = 7,

  /*! \brief Regular polygon with continuous piecewise-linear axial profiles. */
  Polyhedra = 8,
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

/*! \brief Scalar 24-byte full-azimuth tube parameters in its local frame. */
struct GGEMSTubsRecord {
  /*! \brief Positive outer radius in pm. */
  float outer_radius_pm{0.0F};

  /*! \brief Bore radius in pm; zero means no inner wall. */
  float inner_radius_pm{0.0F};

  /*! \brief Positive center-to-cap length along local Z in pm. */
  float half_length_pm{0.0F};

  /*! \brief Snapshot-local physical owner. */
  std::uint32_t volume_id{1};

  /*! \brief Exact dense snapshot-local Material identity. */
  std::uint32_t material_id{0};

  /*! \brief Closed radial and cap surface half-band in FP32 pm. */
  float tolerance_pm{0.0F};
};

/*! \brief Scalar 40-byte local conical frustum and ownership record. */
struct GGEMSConsRecord {
  /*! \brief Midplane outer radius in pm. */
  float outer_radius_pm{0.0F};

  /*! \brief Midplane bore radius in pm; zero disables the bore. */
  float inner_radius_pm{0.0F};

  /*! \brief Outer dr/dz in the local frame. */
  float outer_slope{0.0F};

  /*! \brief Inner dr/dz in the local frame. */
  float inner_slope{0.0F};

  /*! \brief Center-to-cap length in pm. */
  float half_length_pm{0.0F};

  /*! \brief sqrt(1 + outer_slope squared). */
  float outer_normal_scale{0.0F};

  /*! \brief sqrt(1 + inner_slope squared). */
  float inner_normal_scale{0.0F};

  /*! \brief Physical owner identity. */
  std::uint32_t volume_id{1};

  /*! \brief Dense exact Material identity. */
  std::uint32_t material_id{0};

  /*! \brief Closed perpendicular surface half-band in pm. */
  float tolerance_pm{0.0F};
};

/*! \brief Scalar 24-byte complete ellipsoid parameters in its local frame. */
struct GGEMSEllipsoidRecord {
  /*! \brief Positive local X semi-axis in pm. */
  float semi_axis_x_pm{0.0F};

  /*! \brief Positive local Y semi-axis in pm. */
  float semi_axis_y_pm{0.0F};

  /*! \brief Positive local Z semi-axis in pm. */
  float semi_axis_z_pm{0.0F};

  /*! \brief Snapshot-local physical owner. */
  std::uint32_t volume_id{1};

  /*! \brief Exact dense snapshot-local Material identity. */
  std::uint32_t material_id{0};

  /*! \brief Closed surface half-band applied to the semi-axes in FP32 pm. */
  float tolerance_pm{0.0F};
};

/*! \brief Outward unit normal and offset for n dot p <= offset in local pm. */
struct GGEMSPlaneRecord {
  /*! \brief Dimensionless outward normal X. */
  float normal_x{0.0F};

  /*! \brief Dimensionless outward normal Y. */
  float normal_y{0.0F};

  /*! \brief Dimensionless outward normal Z. */
  float normal_z{0.0F};

  /*! \brief Signed plane offset in local pm. */
  float offset_pm{0.0F};
};

/*!
 * \brief Scalar 80-byte Trd/Trap payload with four sides and implicit Z caps.
 */
struct GGEMSPlanarRecord {
  /*! \brief Outward normalized sides in -Y, +Y, -X, +X order. */
  GGEMSPlaneRecord sides[4]{};

  /*! \brief Positive center-to-cap Z length in pm. */
  float half_length_pm{0.0F};

  /*! \brief Snapshot-local physical owner. */
  std::uint32_t volume_id{1};

  /*! \brief Exact dense snapshot-local Material identity. */
  std::uint32_t material_id{0};

  /*! \brief Closed normal-distance surface half-band in FP32 pm. */
  float tolerance_pm{0.0F};
};

/*! \brief Stores only the shape payload selected by the enclosing tag. */
union GGEMSAnalyticParameters {
  /*! \brief Compact counts and identities for auxiliary polygon profiles. */
  GGEMSPolyhedraRecord polyhedra;

  /*! \brief Local Box half extents and owner identities. */
  GGEMSBoxRecord box{};

  /*! \brief Full Sphere; no rotation is stored or allocated. */
  GGEMSSphereRecord sphere;

  /*! \brief Full-azimuth finite cylinder or hollow tube. */
  GGEMSTubsRecord tubs;

  /*! \brief Conical end profiles, cap length, and identities. */
  GGEMSConsRecord cons;

  /*! \brief Complete solid ellipsoid. */
  GGEMSEllipsoidRecord ellipsoid;

  /*! \brief Trd or Trap with normalized outward side planes. */
  GGEMSPlanarRecord planar;
};

/*!
 * \brief Scalar 84-byte scene record, including the no-volume case.
 *
 * Mirrors GGEMSAnalyticVolumeRecord.clh. All shapes except Sphere reuse the
 * separately supplied GGEMSBoxPoseRecord. All programs share this scene layout.
 */
struct GGEMSAnalyticVolumeRecord {
  /*! \brief Predicate and active union member. */
  GGEMSAnalyticKind kind{GGEMSAnalyticKind::None};

  /*! \brief Shape parameters; box is active for None and Box. */
  GGEMSAnalyticParameters parameters{};
};

} // namespace ggems::geometry
