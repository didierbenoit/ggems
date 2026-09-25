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
 * \brief Defines canonical picometer positions, displacements, and directions.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <cmath>
#include <cstdint>
#include <optional>
/// \endcond

/*!
 * \namespace ggems::geometry
 * \brief Provides signed picometer geometry and dimensionless directions.
 */
namespace ggems::geometry {

/*! \brief Signed coordinate or displacement component in picometers. */
using CoordinatePM = std::int64_t;

/*! \brief Nonnegative distance in picometers. */
using DistancePM = std::uint64_t;

/*!
 * \brief Represents a 3D point with signed coordinates in picometers.
 *
 * Coordinates locate the point relative to its coordinate-system origin.
 * Default initialization places the point at that origin.
 */
struct Position3PM {
  /*! \brief X coordinate in picometers. */
  CoordinatePM x{0};

  /*! \brief Y coordinate in picometers. */
  CoordinatePM y{0};

  /*! \brief Z coordinate in picometers. */
  CoordinatePM z{0};

  /*!
   * \brief Compares positions lexicographically by x, then y, then z.
   *
   * \return The ordering determined by the first differing coordinate,
   *         or equality if all three coordinates match.
   */
  constexpr auto operator<=>(Position3PM const &) const = default;
};

/*!
 * \brief Represents a 3D displacement with signed components in picometers.
 *
 * Adding this vector to a position translates that position.
 * Default initialization represents zero displacement.
 */
struct Displacement3PM {
  /*! \brief Signed displacement along X in picometers. */
  CoordinatePM x{0};

  /*! \brief Signed displacement along Y in picometers. */
  CoordinatePM y{0};

  /*! \brief Signed displacement along Z in picometers. */
  CoordinatePM z{0};

  /*!
   * \brief Compares displacements lexicographically by x, then y, then z.
   *
   * \return The ordering determined by the first differing component,
   *         or equality if all three components match.
   */
  constexpr auto operator<=>(Displacement3PM const &) const = default;
};

/*!
 * \brief Represents a 3D direction with dimensionless components.
 *
 * Intended for unit vectors; direct construction does not normalize the
 * components. The default direction points along the positive Z axis.
 */
struct Direction3 {
  /*! \brief Dimensionless X component. */
  float x{0.0F};

  /*! \brief Dimensionless Y component. */
  float y{0.0F};

  /*! \brief Dimensionless Z component. */
  float z{1.0F};
};

/*!
 * \brief Creates a position from Cartesian coordinates in picometers.
 *
 * \param[in] pos_x X coordinate in picometers.
 * \param[in] pos_y Y coordinate in picometers.
 * \param[in] pos_z Z coordinate in picometers.
 * \return A position containing the supplied coordinates.
 */
constexpr auto MakePositionPM(CoordinatePM pos_x, CoordinatePM pos_y,
                              CoordinatePM pos_z) noexcept -> Position3PM {
  return Position3PM{.x = pos_x, .y = pos_y, .z = pos_z};
}

/*!
 * \brief Creates a displacement from signed components in picometers.
 *
 * \param[in] dis_x Displacement along X in picometers.
 * \param[in] dis_y Displacement along Y in picometers.
 * \param[in] dis_z Displacement along Z in picometers.
 * \return A displacement containing the supplied components.
 */
constexpr auto MakeDisplacementPM(CoordinatePM dis_x, CoordinatePM dis_y,
                                  CoordinatePM dis_z) noexcept
  -> Displacement3PM {
  return Displacement3PM{.x = dis_x, .y = dis_y, .z = dis_z};
}

/*!
 * \brief Translates a position by a displacement.
 *
 * \param[in] position Initial position.
 * \param[in] displacement Translation to apply.
 * \return The position after adding each displacement component.
 *
 * \pre Every signed component result must fit CoordinatePM; overflow is not
 * checked.
 */
constexpr auto operator+(Position3PM position,
                         Displacement3PM displacement) noexcept -> Position3PM {
  return Position3PM{
    .x = position.x + displacement.x,
    .y = position.y + displacement.y,
    .z = position.z + displacement.z,
  };
}

/*!
 * \brief Translates a position by the opposite of a displacement.
 *
 * \param[in] position Initial position.
 * \param[in] displacement Translation to subtract.
 * \return The position after subtracting each displacement component.
 *
 * \pre Every signed component result must fit CoordinatePM; overflow is not
 * checked.
 */
constexpr auto operator-(Position3PM position,
                         Displacement3PM displacement) noexcept -> Position3PM {
  return Position3PM{
    .x = position.x - displacement.x,
    .y = position.y - displacement.y,
    .z = position.z - displacement.z,
  };
}

/*!
 * \brief Computes the displacement from rhs to lhs.
 *
 * \param[in] lhs Destination position.
 * \param[in] rhs Starting position.
 * \return The displacement that translates rhs to lhs.
 *
 * \pre Every signed component result must fit CoordinatePM; overflow is not
 * checked.
 */
constexpr auto operator-(Position3PM lhs, Position3PM rhs) noexcept
  -> Displacement3PM {
  return Displacement3PM{
    .x = lhs.x - rhs.x,
    .y = lhs.y - rhs.y,
    .z = lhs.z - rhs.z,
  };
}

/*!
 * \brief Combines two displacement vectors by component-wise addition.
 *
 * \param[in] lhs First displacement.
 * \param[in] rhs Second displacement.
 * \return The combined displacement.
 *
 * \pre Every signed component result must fit CoordinatePM; overflow is not
 * checked.
 */
constexpr auto operator+(Displacement3PM lhs, Displacement3PM rhs) noexcept
  -> Displacement3PM {
  return Displacement3PM{
    .x = lhs.x + rhs.x,
    .y = lhs.y + rhs.y,
    .z = lhs.z + rhs.z,
  };
}

/*!
 * \brief Subtracts one displacement vector from another.
 *
 * \param[in] lhs Initial displacement.
 * \param[in] rhs Displacement to subtract.
 * \return The component-wise difference lhs minus rhs.
 *
 * \pre Every signed component result must fit CoordinatePM; overflow is not
 * checked.
 */
constexpr auto operator-(Displacement3PM lhs, Displacement3PM rhs) noexcept
  -> Displacement3PM {
  return Displacement3PM{
    .x = lhs.x - rhs.x,
    .y = lhs.y - rhs.y,
    .z = lhs.z - rhs.z,
  };
}

/*!
 * \brief Reverses a displacement vector.
 *
 * \param[in] displacement Displacement to reverse.
 * \return A displacement with each component negated.
 *
 * \pre Every signed component result must fit CoordinatePM; overflow is not
 * checked.
 */
constexpr auto operator-(Displacement3PM displacement) noexcept
  -> Displacement3PM {
  return Displacement3PM{
    .x = -displacement.x,
    .y = -displacement.y,
    .z = -displacement.z,
  };
}

/*!
 * \brief Computes the squared Euclidean length of a direction vector.
 *
 * The calculation uses double-precision intermediates.
 *
 * \param[in] direction Vector to measure; normalization is not required.
 * \return The sum of the squared components, returned in single precision.
 */
[[nodiscard]] inline auto SquaredNorm(Direction3 direction) noexcept -> float {
  auto const dir_x = static_cast<double>(direction.x);
  auto const dir_y = static_cast<double>(direction.y);
  auto const dir_z = static_cast<double>(direction.z);
  return static_cast<float>((dir_x * dir_x) + (dir_y * dir_y) +
                            (dir_z * dir_z));
}

/*!
 * \brief Computes the Euclidean length of a direction vector.
 *
 * The length is calculated in double precision before conversion to float.
 *
 * \param[in] direction Vector to measure; normalization is not required.
 * \return The dimensionless vector length.
 */
[[nodiscard]] inline auto Norm(Direction3 direction) noexcept -> float {
  return static_cast<float>(std::hypot(static_cast<double>(direction.x),
                                       static_cast<double>(direction.y),
                                       static_cast<double>(direction.z)));
}

/*!
 * \namespace ggems::geometry::detail
 * \brief Provides internal direction-normalization helpers.
 */
namespace detail {
/*!
 * \brief Stores a vector-normalization result in double precision.
 *
 * Components are dimensionless. Normalization retains double precision
 * before any conversion to the float components of Direction3.
 */
struct NormalizedVector3D {
  /*! \brief Normalized X component. */
  double x;

  /*! \brief Normalized Y component. */
  double y;

  /*! \brief Normalized Z component. */
  double z;
};

/*!
 * \brief Attempts to normalize a 3D vector in double precision.
 *
 * Each component is divided by the vector's Euclidean length.
 *
 * \param[in] x_val X component of the input vector.
 * \param[in] y_val Y component of the input vector.
 * \param[in] z_val Z component of the input vector.
 * \return The normalized vector, or std::nullopt if an input component,
 *         the computed length, or a result component is nonfinite,
 *         or if the computed length is zero.
 */
[[nodiscard]] inline auto TryNormalizeVector3D(double x_val, double y_val,
                                               double z_val) noexcept
  -> std::optional<NormalizedVector3D> {
  if (!std::isfinite(x_val) || !std::isfinite(y_val) || !std::isfinite(z_val)) {
    return std::nullopt;
  }

  double const norm = std::hypot(x_val, y_val, z_val);
  if (!std::isfinite(norm) || !(norm > 0.0)) {
    return std::nullopt;
  }

  NormalizedVector3D const result{
    .x = x_val / norm,
    .y = y_val / norm,
    .z = z_val / norm,
  };

  if (!std::isfinite(result.x) || !std::isfinite(result.y) ||
      !std::isfinite(result.z)) {
    return std::nullopt;
  }

  return result;
}

} // namespace detail

/*!
 * \brief Attempts to create a unit direction from a 3D vector.
 *
 * Normalization uses double precision before conversion to float.
 * The stored direction has unit length within floating-point rounding.
 *
 * \param[in] dir_x X component of the input vector.
 * \param[in] dir_y Y component of the input vector.
 * \param[in] dir_z Z component of the input vector.
 * \return The normalized direction, or std::nullopt if normalization fails
 *         or conversion does not produce a finite, nonzero direction.
 */
[[nodiscard]] inline auto TryMakeDirection3(double dir_x, double dir_y,
                                            double dir_z) noexcept
  -> std::optional<Direction3> {
  auto const precise = detail::TryNormalizeVector3D(dir_x, dir_y, dir_z);
  if (!precise.has_value()) {
    return std::nullopt;
  }

  Direction3 const result{
    .x = static_cast<float>(precise->x),
    .y = static_cast<float>(precise->y),
    .z = static_cast<float>(precise->z),
  };

  if (!std::isfinite(result.x) || !std::isfinite(result.y) ||
      !std::isfinite(result.z) || !(Norm(result) > 0.0F)) {
    return std::nullopt;
  }

  return result;
}

/*!
 * \brief Computes the scalar product of two direction vectors.
 *
 * Uses double-precision intermediates. For unit vectors, the scalar
 * product is the cosine of the angle between them.
 *
 * \param[in] lhs First direction vector.
 * \param[in] rhs Second direction vector.
 * \return The sum of the products of corresponding components,
 *         returned in single precision.
 */
[[nodiscard]] inline auto Dot(Direction3 lhs, Direction3 rhs) noexcept
  -> float {
  return static_cast<float>(
    (static_cast<double>(lhs.x) * static_cast<double>(rhs.x)) +
    (static_cast<double>(lhs.y) * static_cast<double>(rhs.y)) +
    (static_cast<double>(lhs.z) * static_cast<double>(rhs.z)));
}
} // namespace ggems::geometry
