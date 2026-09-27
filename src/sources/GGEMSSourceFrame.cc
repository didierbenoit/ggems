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
 * \brief Normalizes source axes and builds right-handed binary32 frames.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <array>
#include <cmath>
#include <format>
#include <string_view>

#include "GGEMS/GGEMSException.hh"

#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/sources/GGEMSSourceFrame.hh"

namespace ggems::core::sources {
namespace {

/*! \brief Normalized binary64 working axis used before binary32 packing. */
using PreciseAxis = geometry::detail::NormalizedVector3D;

/*! \brief Three binary64 components used for source-frame construction. */
using Vector3D = std::array<double, 3U>;

// =============================================================================
// =============================================================================

/*!
 * \brief Normalizes a finite nonzero working vector.
 *
 * \param[in] vector Working vector components.
 * \param[in] name Vector role included in failure diagnostics.
 * \return Normalized binary64 axis.
 * \throws GGEMSRecoverable If normalization fails.
 */
[[nodiscard]] auto RequireNormalized(Vector3D const &vector,
                                     std::string_view name) -> PreciseAxis {
  auto const normalized =
    geometry::detail::TryNormalizeVector3D(vector[0], vector[1], vector[2]);

  if (!normalized.has_value()) {
    throw GGEMSRecoverable(std::format(
      "Source {} must have a finite, strictly positive norm.", name));
  }

  return *normalized;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Computes the scalar product of two working axes.
 *
 * \param[in] lhs Left axis.
 * \param[in] rhs Right axis.
 * \return Dimensionless dot product.
 */
[[nodiscard]] auto Dot(PreciseAxis lhs, PreciseAxis rhs) noexcept -> double {
  return (lhs.x * rhs.x) + (lhs.y * rhs.y) + (lhs.z * rhs.z);
}

// =============================================================================
// =============================================================================

/*!
 * \brief Computes the right-handed cross product of two working axes.
 *
 * \param[in] lhs Left axis.
 * \param[in] rhs Right axis.
 * \return Three unnormalized binary64 components.
 */
[[nodiscard]] auto Cross(PreciseAxis lhs, PreciseAxis rhs) noexcept
  -> Vector3D {
  return Vector3D{
    (lhs.y * rhs.z) - (lhs.z * rhs.y),
    (lhs.z * rhs.x) - (lhs.x * rhs.z),
    (lhs.x * rhs.y) - (lhs.y * rhs.x),
  };
}

// =============================================================================
// =============================================================================

/*!
 * \brief Checks the normalized direction/up separation threshold.
 *
 * \param[in] direction Unit forward axis.
 * \param[in] up_reference Unit up reference.
 * \return True when 1 - abs(dot) is no greater than the parallel tolerance.
 */
[[nodiscard]] auto IsTooParallel(PreciseAxis direction,
                                 PreciseAxis up_reference) noexcept -> bool {
  return 1.0 - std::abs(Dot(direction, up_reference)) <=
         k_source_frame_parallel_tolerance;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Narrows a normalized working axis to shared binary32 components.
 *
 * \param[in] direction Normalized binary64 axis.
 * \return Binary32 direction without further normalization.
 */
[[nodiscard]] auto ToFloatDirection(PreciseAxis direction) noexcept
  -> geometry::Direction3 {
  return {
    .x = static_cast<float>(direction.x),
    .y = static_cast<float>(direction.y),
    .z = static_cast<float>(direction.z),
  };
}

// =============================================================================
// =============================================================================

/*!
 * \brief Measures orientation using the scalar triple product.
 *
 * \param[in] frame Binary32 source frame.
 * \return (axis_x cross axis_y) dot axis_z evaluated in binary64.
 */
[[nodiscard]] auto FloatHandedness(GGEMSSourceFrame const &frame) noexcept
  -> double {
  double const cross_x =
    (static_cast<double>(frame.axis_x.y) * frame.axis_y.z) -
    (static_cast<double>(frame.axis_x.z) * frame.axis_y.y);

  double const cross_y =
    (static_cast<double>(frame.axis_x.z) * frame.axis_y.x) -
    (static_cast<double>(frame.axis_x.x) * frame.axis_y.z);

  double const cross_z =
    (static_cast<double>(frame.axis_x.x) * frame.axis_y.y) -
    (static_cast<double>(frame.axis_x.y) * frame.axis_y.x);

  return (cross_x * frame.axis_z.x) + (cross_y * frame.axis_z.y) +
         (cross_z * frame.axis_z.z);
}

// =============================================================================
// =============================================================================

/*!
 * \brief Checks finite axes, unit norms, orthogonality, and handedness.
 *
 * \param[in] frame Binary32 frame to inspect.
 * \return True when all checks satisfy the source-frame tolerance.
 */
[[nodiscard]] auto IsValidFloatFrame(GGEMSSourceFrame const &frame) noexcept
  -> bool {
  auto const finite = [](geometry::Direction3 axis) noexcept -> bool {
    return std::isfinite(axis.x) && std::isfinite(axis.y) &&
           std::isfinite(axis.z);
  };

  if (!finite(frame.axis_x) || !finite(frame.axis_y) || !finite(frame.axis_z)) {
    return false;
  }

  double const tolerance = k_source_frame_float_tolerance;

  return std::abs(static_cast<double>(geometry::Norm(frame.axis_x)) - 1.0) <=
           tolerance &&
         std::abs(static_cast<double>(geometry::Norm(frame.axis_y)) - 1.0) <=
           tolerance &&
         std::abs(static_cast<double>(geometry::Norm(frame.axis_z)) - 1.0) <=
           tolerance &&
         std::abs(static_cast<double>(
           geometry::Dot(frame.axis_x, frame.axis_y))) <= tolerance &&
         std::abs(static_cast<double>(
           geometry::Dot(frame.axis_y, frame.axis_z))) <= tolerance &&
         std::abs(static_cast<double>(
           geometry::Dot(frame.axis_z, frame.axis_x))) <= tolerance &&
         std::abs(FloatHandedness(frame) - 1.0) <= tolerance;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Constructs the transverse axes from normalized forward and up.
 *
 * \param[in] direction Normalized forward axis.
 * \param[in] up_reference Normalized up reference.
 * \return Right-handed binary32 frame.
 * \throws GGEMSRecoverable If the axes are too parallel or transverse
 * normalization fails.
 */
auto BuildSourceFrameFromNormalized(PreciseAxis direction,
                                    PreciseAxis up_reference)
  -> GGEMSSourceFrame {
  if (IsTooParallel(direction, up_reference)) {
    throw GGEMSRecoverable(std::format("Source direction and up are too close "
                                       "to parallel (1 - abs(dot) <= {}).",
                                       k_source_frame_parallel_tolerance));
  }

  PreciseAxis const axis_x =
    RequireNormalized(Cross(up_reference, direction), "frame horizontal axis");
  PreciseAxis const axis_y =
    RequireNormalized(Cross(direction, axis_x), "frame vertical axis");

  GGEMSSourceFrame const frame = {
    .axis_x = ToFloatDirection(axis_x),
    .axis_y = ToFloatDirection(axis_y),
    .axis_z = ToFloatDirection(direction),
  };

  return frame;
}

} // namespace

// =============================================================================
// =============================================================================

auto BuildSourceFrame(std::array<double, 3U> const &direction,
                      std::array<double, 3U> const &up_reference)
  -> GGEMSSourceFrame {
  return BuildSourceFrameFromNormalized(
    RequireNormalized(direction, "direction"),
    RequireNormalized(up_reference, "up vector"));
}

// =============================================================================
// =============================================================================

auto IsValidSourceFrame(GGEMSSourceFrame const &frame) noexcept -> bool {
  return IsValidFloatFrame(frame);
}

// =============================================================================
// =============================================================================

auto BuildSourceFrameWithAutomaticUp(std::array<double, 3U> const &direction)
  -> GGEMSSourceFrame {
  PreciseAxis const axis_z = RequireNormalized(direction, "direction");
  PreciseAxis constexpr preferred_up{.x = 0.0, .y = 0.0, .z = 1.0};
  PreciseAxis constexpr fallback_up{.x = 0.0, .y = 1.0, .z = 0.0};

  PreciseAxis const selected_up =
    IsTooParallel(axis_z, preferred_up) ? fallback_up : preferred_up;

  return BuildSourceFrameFromNormalized(axis_z, selected_up);
}

} // namespace ggems::core::sources
