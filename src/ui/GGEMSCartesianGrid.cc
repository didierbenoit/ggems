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
 * \brief Chooses stable Cartesian lattice steps from the shared camera
 * projection.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>

#include "GGEMS/ui/detail/GGEMSCartesianGrid.hh"
#include "GGEMS/ui/detail/GGEMSSceneCamera.hh"

namespace {

/*! \brief Plane coordinates in meters. */
using Point = std::array<double, 2>;
/*! \brief Double-precision copy of the shared float projection rows. */
using Matrix = std::array<std::array<double, 4>, 4>;
/*! \brief Two world axes spanning XY, XZ, and YZ respectively. */
constexpr std::array k_plane_axes{
  std::array<std::size_t, 2>{0U, 1U},
  std::array<std::size_t, 2>{0U, 2U},
  std::array<std::size_t, 2>{1U, 2U},
};
/*! \brief Hard limit for both line families of one plane combined. */
constexpr std::uint32_t k_max_lines{256U};

/*! \brief World rectangle clipped by six frustum half-planes, at most ten
 * vertices. */
struct Polygon {
  /*! \brief Bounded scratch storage for the convex clipping result. */
  std::array<Point, 12> points{};
  /*! \brief Number of active vertices. */
  std::size_t size{};
};

/*!
 * \brief Clips a convex plane polygon against one homogeneous frustum boundary.
 * \param[in] input Previous convex polygon.
 * \param[in] boundary Coefficients of a*u+b*v+c >= 0.
 * \return Retained polygon.
 */
[[nodiscard]] auto Clip(Polygon const &input,
                        std::array<double, 3> const &boundary) noexcept
  -> Polygon {
  Polygon result{};
  if (input.size == 0U) {
    return result;
  }
  auto distance = [&boundary](Point const &point) -> double {
    return (boundary[0] * point[0]) + (boundary[1] * point[1]) + boundary[2];
  };
  Point previous = input.points[input.size - 1U];
  double previous_distance = distance(previous);
  for (std::size_t index = 0U; index < input.size; ++index) {
    Point const current = input.points[index];
    double const current_distance = distance(current);
    if ((previous_distance >= 0.0) != (current_distance >= 0.0)) {
      double const fraction =
        previous_distance / (previous_distance - current_distance);
      result.points[result.size++] = {
        previous[0] + (fraction * (current[0] - previous[0])),
        previous[1] + (fraction * (current[1] - previous[1])),
      };
    }
    if (current_distance >= 0.0) {
      result.points[result.size++] = current;
    }
    previous = current;
    previous_distance = current_distance;
  }
  return result;
}

/*!
 * \brief Clips a World plane rectangle to the current homogeneous camera
 * frustum.
 * \param[in] matrix Shared camera projection.
 * \param[in] axes World indices spanning the plane.
 * \param[in] world_half_extent_m World half extents in meters.
 * \return Visible convex plane polygon.
 */
[[nodiscard]] auto
VisiblePolygon(Matrix const &matrix, std::array<std::size_t, 2> const &axes,
               std::array<double, 3> const &world_half_extent_m) noexcept
  -> Polygon {
  double const half_u = world_half_extent_m[axes[0]];
  double const half_v = world_half_extent_m[axes[1]];
  Polygon polygon{
    .points =
      {
        Point{-half_u, -half_v},
        Point{half_u, -half_v},
        Point{half_u, half_v},
        Point{-half_u, half_v},
      },
    .size = 4U,
  };
  // Homogeneous Vulkan clip bounds: -w<=x,y<=w and 0<=z<=w.
  for (std::size_t boundary = 0U; boundary < 6U; ++boundary) {
    std::array<double, 3> coefficients{};
    for (std::size_t col = 0U; col < 3U; ++col) {
      std::size_t const source = col < 2U ? axes[col] : 3U;
      if (boundary < 4U) {
        double const sign = boundary % 2U == 0U ? 1.0 : -1.0;
        coefficients[col] =
          matrix[3][source] + (sign * matrix[boundary / 2U][source]);
      } else if (boundary == 4U) {
        coefficients[col] = matrix[2][source];
      } else {
        coefficients[col] = matrix[3][source] - matrix[2][source];
      }
    }
    polygon = Clip(polygon, coefficients);
  }
  return polygon;
}

/*!
 * \brief Stores physical bounds and computes a representative interior point.
 * \param[in,out] plane Receives minimum and maximum in-plane coordinates.
 * \param[in] polygon Visible convex polygon with at least three vertices.
 * \return Mean of the polygon vertices in meters.
 */
[[nodiscard]] auto
SetBounds(ggems::ui::detail::GGEMSCartesianGrid::PlaneLines &plane,
          Polygon const &polygon) noexcept -> Point {
  plane.minimum_m = plane.maximum_m = polygon.points[0];
  Point center{};
  for (std::size_t index = 0U; index < polygon.size; ++index) {
    for (std::size_t axis = 0U; axis < 2U; ++axis) {
      plane.minimum_m[axis] =
        std::min(plane.minimum_m[axis], polygon.points[index][axis]);
      plane.maximum_m[axis] =
        std::max(plane.maximum_m[axis], polygon.points[index][axis]);
      center[axis] +=
        polygon.points[index][axis] / static_cast<double>(polygon.size);
    }
  }

  return center;
}

/*! \brief Conservative projected lattice separation and edge-on compression. */
struct Density {
  /*! \brief Minimum perpendicular line separation per physical meter. */
  double pixels_per_meter{};
  /*! \brief Projected area divided by the squared longest projected basis. */
  double foreshortening{};
};

/*!
 * \brief Measures both projected line families at visible corners and center.
 * \param[in] matrix Shared homogeneous camera projection.
 * \param[in] axes World axes spanning the plane.
 * \param[in] pixel_scale Half logical viewport dimensions.
 * \param[in] polygon Clipped visible plane polygon.
 * \param[in] center Representative interior point.
 * \return Worst sampled density and foreshortening.
 */
[[nodiscard]] auto MeasureProjection(Matrix const &matrix,
                                     std::array<std::size_t, 2> const &axes,
                                     std::array<double, 2> const &pixel_scale,
                                     Polygon const &polygon,
                                     Point const &center) noexcept -> Density {
  double pixels_per_meter = std::numeric_limits<double>::infinity();
  double foreshortening = 1.0;
  for (std::size_t sample = 0U; sample <= polygon.size; ++sample) {
    Point const point =
      sample == polygon.size ? center : polygon.points[sample];
    double const w = (matrix[3][axes[0]] * point[0]) +
                     (matrix[3][axes[1]] * point[1]) + matrix[3][3];
    if (w <= 0.0 || !std::isfinite(w)) {
      pixels_per_meter = 0.0;
      break;
    }
    std::array<Point, 2> derivative{};
    for (std::size_t row = 0U; row < 2U; ++row) {
      double const clip = (matrix[row][axes[0]] * point[0]) +
                          (matrix[row][axes[1]] * point[1]) + matrix[row][3];
      for (std::size_t axis = 0U; axis < 2U; ++axis) {
        derivative[axis][row] =
          pixel_scale[row] *
          ((matrix[row][axes[axis]] * w) - (clip * matrix[3][axes[axis]])) /
          (w * w);
      }
    }
    double const longest =
      std::max(std::hypot(derivative[0][0], derivative[0][1]),
               std::hypot(derivative[1][0], derivative[1][1]));
    double const area = std::abs((derivative[0][0] * derivative[1][1]) -
                                 (derivative[0][1] * derivative[1][0]));
    if (longest <= 0.0 || !std::isfinite(longest)) {
      pixels_per_meter = 0.0;
      break;
    }
    pixels_per_meter = std::min(pixels_per_meter, area / longest);
    foreshortening = std::min(foreshortening, area / (longest * longest));
  }
  return {
    .pixels_per_meter = pixels_per_meter,
    .foreshortening = foreshortening,
  };
}

/*!
 * \brief Rounds upward to the physical 1-2-5 times power-of-ten sequence.
 * \param[in] value Positive requested spacing.
 * \return Smallest sequence value at least as large as the request.
 */
[[nodiscard]] auto NiceCeiling(double value) noexcept -> double {
  double const decade = std::pow(10.0, std::floor(std::log10(value)));
  for (double multiplier : {1.0, 2.0, 5.0}) {
    double const candidate = decade * multiplier;
    if (value <= candidate) {
      return candidate;
    }
  }
  return decade * 10.0;
}

/*!
 * \brief Counts the origin-relative lattice lines intersecting a bounded
 * interval.
 * \param[in,out] plane Physical range and step; receives first indices and
 * counts.
 * \return Combined line count.
 */
auto SetLineIntervals(
  ggems::ui::detail::GGEMSCartesianGrid::PlaneLines &plane) noexcept
  -> std::uint32_t {
  std::uint32_t count = 0U;
  for (std::size_t axis = 0U; axis < 2U; ++axis) {
    double const first = std::ceil(plane.minimum_m[axis] / plane.minor_step_m);
    double const last = std::floor(plane.maximum_m[axis] / plane.minor_step_m);
    plane.first_index[axis] = static_cast<std::int32_t>(first);
    plane.line_count[axis] =
      last >= first ? static_cast<std::uint32_t>(last - first + 1.0) : 0U;
    count += plane.line_count[axis];
  }
  return count;
}

} // namespace

namespace ggems::ui::detail {

auto GGEMSCartesianGrid::Update(
  GGEMSSceneCamera const &camera,
  std::array<double, 3> const &world_half_extent_m) noexcept -> void {
  auto const projection = camera.BuildWorldToClipMatrix();
  std::array const rows{
    projection.row_0,
    projection.row_1,
    projection.row_2,
    projection.row_3,
  };
  Matrix matrix{};
  for (std::size_t row = 0U; row < 4U; ++row) {
    for (std::size_t col = 0U; col < 4U; ++col) {
      matrix[row][col] = rows[row][col];
    }
  }
  auto const camera_state = camera.GetDiagnostics();
  std::array<double, 2> const pixel_scale{
    0.5 * camera_state.viewport_width,
    0.5 * camera_state.viewport_height,
  };

  for (std::size_t plane_index = 0U; plane_index < planes_.size();
       ++plane_index) {
    auto &plane = planes_[plane_index];
    double const previous_step = plane.minor_step_m;
    plane = {};
    auto const axes = k_plane_axes[plane_index];
    Polygon const polygon = VisiblePolygon(matrix, axes, world_half_extent_m);
    if (polygon.size < 3U) {
      continue;
    }
    Point const center = SetBounds(plane, polygon);
    Density const density =
      MeasureProjection(matrix, axes, pixel_scale, polygon, center);
    if (!(density.pixels_per_meter > 0.0) ||
        !std::isfinite(density.pixels_per_meter)) {
      continue;
    }
    plane.opacity = static_cast<float>(
      std::clamp((density.foreshortening - 0.01) / 0.09, 0.0, 1.0));
    if (plane.opacity == 0.0F) {
      continue;
    }

    double const span = plane.maximum_m[0] - plane.minimum_m[0] +
                        plane.maximum_m[1] - plane.minimum_m[1];
    double const largest_coordinate = std::max({
      std::abs(plane.minimum_m[0]),
      std::abs(plane.minimum_m[1]),
      std::abs(plane.maximum_m[0]),
      std::abs(plane.maximum_m[1]),
    });
    // Preserve distinguishable FP32 positions and bounded signed lattice
    // indices.
    double const minimum_step = std::max(
      span / (k_max_lines - 2U),
      largest_coordinate * 4.0 * std::numeric_limits<float>::epsilon());
    double const old_pixel_spacing = previous_step * density.pixels_per_meter;
    plane.minor_step_m =
      old_pixel_spacing >= 16.0 && old_pixel_spacing <= 72.0 &&
          previous_step >= minimum_step
        ? previous_step
        : NiceCeiling(std::max(24.0 / density.pixels_per_meter, minimum_step));
    while (SetLineIntervals(plane) > k_max_lines) {
      plane.minor_step_m = NiceCeiling(plane.minor_step_m * 1.01);
    }
    plane.major_step_m = 5.0 * plane.minor_step_m;
  }
}

// -----------------------------------------------------------------------------

auto GGEMSCartesianGrid::GetPlanes() const noexcept
  -> std::array<PlaneLines, 3> const & {
  return planes_;
}

} // namespace ggems::ui::detail
