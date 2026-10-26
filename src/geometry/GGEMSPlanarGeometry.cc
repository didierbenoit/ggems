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
 * \brief Compiles Trd and Trap end parameters into outward FP32 half-spaces.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include "GGEMS/geometry/detail/GGEMSPlanarGeometry.hh"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <limits>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/geometry/GGEMSAnalyticVolumeRecord.hh"
#include "GGEMS/geometry/GGEMSTrd.hh"
#include "GGEMS/geometry/GGEMSTrap.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"

namespace {

/*!
 * \brief Normalizes one static side equation without squaring pm dimensions.
 * \param[in] x Dimensionless normal X.
 * \param[in] y Dimensionless normal Y.
 * \param[in] z Dimensionless normal Z.
 * \param[in] offset Plane offset in pm before normalization.
 * \return Outward unit-normal plane.
 */
[[nodiscard]] auto Plane(float x, float y, float z, float offset) noexcept
  -> ggems::geometry::GGEMSPlaneRecord {
  auto const norm = std::hypot(x, y, z);
  return {
    .normal_x = x / norm,
    .normal_y = y / norm,
    .normal_z = z / norm,
    .offset_pm = offset / norm,
  };
}

/*!
 * \brief Compiles the four fixed trapezoid side equations.
 * \param[in] minus Negative-Z half-height and two half-widths, in pm.
 * \param[in] plus Positive-Z half-height and two half-widths, in pm.
 * \param[in] half_length Local Z half-length in pm.
 * \param[in] skew_minus Tangent of negative-Z end skew.
 * \param[in] skew_plus Tangent of positive-Z end skew.
 * \param[in] tilt_x Center-line X slope versus Z.
 * \param[in] tilt_y Center-line Y slope versus Z.
 * \return Four normalized sides, with separate implicit end planes.
 */
[[nodiscard]] auto CompileSides(std::array<float, 3U> minus,
                                std::array<float, 3U> plus, float half_length,
                                float skew_minus, float skew_plus, float tilt_x,
                                float tilt_y) noexcept
  -> ggems::geometry::GGEMSPlanarRecord {
  auto const mean_y = (minus[0] + plus[0]) * 0.5F;
  auto const slope_y = (plus[0] - minus[0]) / (2.0F * half_length);
  auto const width_minus = (minus[1] + minus[2]) * 0.5F;
  auto const width_plus = (plus[1] + plus[2]) * 0.5F;
  auto const mean_x = (width_minus + width_plus) * 0.5F;
  auto const slope_x = (width_plus - width_minus) / (2.0F * half_length);
  auto const gradient_minus = (minus[2] - minus[1]) / (2.0F * minus[0]);
  auto const gradient_plus = (plus[2] - plus[1]) / (2.0F * plus[0]);

  // A planar side intersects both Z caps in parallel edges. Averaging the
  // two uploaded slopes shares their FP32 roundoff; authoring admission checks
  // every supplied vertex against the resulting physical side plane.
  auto const negative_slope =
    ((skew_minus - gradient_minus) + (skew_plus - gradient_plus)) * 0.5F;

  auto const positive_slope =
    ((skew_minus + gradient_minus) + (skew_plus + gradient_plus)) * 0.5F;

  return {
    .sides =
      {
        Plane(0.0F, -1.0F, tilt_y - slope_y, mean_y),
        Plane(0.0F, 1.0F, -tilt_y - slope_y, mean_y),
        Plane(-1.0F, negative_slope,
              tilt_x - (negative_slope * tilt_y) - slope_x, mean_x),
        Plane(1.0F, -positive_slope,
              -tilt_x + (positive_slope * tilt_y) - slope_x, mean_x),
      },
    .half_length_pm = half_length,
  };
}

} // namespace

namespace ggems::geometry::detail {

auto BuildPlanarRecord(GGEMSTrd const &solid) noexcept -> GGEMSPlanarRecord {
  auto const minus = solid.GetNegativeZHalfSizePM();
  auto const plus = solid.GetPositiveZHalfSizePM();
  return CompileSides(
    {
      static_cast<float>(minus[1]),
      static_cast<float>(minus[0]),
      static_cast<float>(minus[0]),
    },
    {
      static_cast<float>(plus[1]),
      static_cast<float>(plus[0]),
      static_cast<float>(plus[0]),
    },
    static_cast<float>(solid.GetHalfLengthPM()), 0.0F, 0.0F, 0.0F, 0.0F);
}

// -----------------------------------------------------------------------------

auto BuildPlanarRecord(GGEMSTrap const &solid) noexcept -> GGEMSPlanarRecord {
  auto const minus = solid.GetNegativeZHalfSizePM();
  auto const plus = solid.GetPositiveZHalfSizePM();
  auto const theta =
    static_cast<float>(units::ToRadians(solid.GetInclinationTheta()));
  auto const phi =
    static_cast<float>(units::ToRadians(solid.GetInclinationPhi()));

  return CompileSides(
    {
      static_cast<float>(minus[0]),
      static_cast<float>(minus[1]),
      static_cast<float>(minus[2]),
    },
    {
      static_cast<float>(plus[0]),
      static_cast<float>(plus[1]),
      static_cast<float>(plus[2]),
    },
    static_cast<float>(solid.GetHalfLengthPM()),
    std::tan(static_cast<float>(units::ToRadians(solid.GetNegativeZSkew()))),
    std::tan(static_cast<float>(units::ToRadians(solid.GetPositiveZSkew()))),
    std::tan(theta) * std::cos(phi), std::tan(theta) * std::sin(phi));
}

// -----------------------------------------------------------------------------

auto RequirePlanarTrap(GGEMSTrap const &solid) -> void {
  auto const record = BuildPlanarRecord(solid);
  auto const theta =
    static_cast<float>(units::ToRadians(solid.GetInclinationTheta()));
  auto const phi =
    static_cast<float>(units::ToRadians(solid.GetInclinationPhi()));
  auto const tilt_x = std::tan(theta) * std::cos(phi);
  auto const tilt_y = std::tan(theta) * std::sin(phi);
  std::array<std::array<float, 3U>, 8U> vertices{};
  auto scale = record.half_length_pm;

  for (std::size_t end = 0U; end < 2U; ++end) {
    auto const half = end == 0U ? solid.GetNegativeZHalfSizePM()
                                : solid.GetPositiveZHalfSizePM();
    auto const skew = std::tan(static_cast<float>(units::ToRadians(
      end == 0U ? solid.GetNegativeZSkew() : solid.GetPositiveZSkew())));
    auto const z = end == 0U ? -record.half_length_pm : record.half_length_pm;
    for (std::size_t side_y = 0U; side_y < 2U; ++side_y) {
      auto const local_y =
        (side_y == 0U ? -1.0F : 1.0F) * static_cast<float>(half[0]);
      for (std::size_t side_x = 0U; side_x < 2U; ++side_x) {
        auto const x = (z * tilt_x) + (local_y * skew) +
                       ((side_x == 0U ? -1.0F : 1.0F) *
                        static_cast<float>(half[1U + side_y]));
        auto const y = (z * tilt_y) + local_y;
        vertices[(4U * end) + (2U * side_y) + side_x] = {x, y, z};
        scale = std::max({scale, std::abs(x), std::abs(y)});
      }
    }
  }

  auto const band =
    16.0F *
    (std::nextafter(scale, std::numeric_limits<float>::infinity()) - scale);
  for (std::size_t vertex = 0U; vertex < vertices.size(); ++vertex) {
    auto const &point = vertices[vertex];
    auto const &plane = record.sides[2U + (vertex % 2U)];
    auto const distance = (plane.normal_x * point[0]) +
                          (plane.normal_y * point[1]) +
                          (plane.normal_z * point[2]) - plane.offset_pm;
    if (!(std::abs(distance) <= band)) {
      throw core::GGEMSRecoverable(
        "Trap side faces are not planar within the local FP32 surface band; "
        "the two end skews and X-width gradients must define parallel edges.");
    }
  }
}

} // namespace ggems::geometry::detail
