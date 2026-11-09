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
 * \brief Defines immutable analytical child snapshots for host expansion.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <variant>
#include <vector>

#include "GGEMS/geometry/GGEMSBox.hh"
#include "GGEMS/geometry/GGEMSSphere.hh"
#include "GGEMS/geometry/GGEMSTubs.hh"
#include "GGEMS/geometry/GGEMSCons.hh"
#include "GGEMS/geometry/GGEMSPolyhedra.hh"
#include "GGEMS/geometry/GGEMSEllipsoid.hh"
#include "GGEMS/geometry/GGEMSTrd.hh"
#include "GGEMS/geometry/GGEMSTrap.hh"
#include "GGEMS/geometry/GGEMSVolumeAuthoring.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace ggems::geometry::detail {

/*! \brief Captures one analytical definition, local pose, and descendants. */
struct GGEMSAuthoredVolume {
  /*! \brief Existing eight complete analytical authoring alternatives. */
  using Volume = std::variant<GGEMSBox, GGEMSSphere, GGEMSTubs, GGEMSEllipsoid,
                              GGEMSTrd, GGEMSTrap, GGEMSCons, GGEMSPolyhedra>;

  /*! \brief Complete value with immutable captured child descriptions. */
  Volume volume;
};

/*! \brief Describes a finite group of roots sharing one captured subtree. */
struct GGEMSVolumeRecipe {
  /*! \brief Non-null owned snapshot, captured before recipe publication. */
  std::shared_ptr<GGEMSAuthoredVolume const> child;

  /*! \brief Parent-local placement rule. */
  GGEMSRepeatKind kind{GGEMSRepeatKind::Single};

  /*! \brief Checked total root count, including all Cartesian combinations. */
  std::uint32_t count{1U};

  /*! \brief Positive linear multiplicities ordered X, Y, Z. */
  std::array<std::uint32_t, 3> linear_count{1U, 1U, 1U};

  /*! \brief Nonnegative linear spacings ordered X, Y, Z in picometers. */
  std::array<units::Length, 3> linear_pitch{};

  /*! \brief Circular rotation axis through the mother's local origin. */
  GGEMSRepeatAxis axis{GGEMSRepeatAxis::X};
};

/*! \brief Stores one immutable direct-child list and its expansion bounds. */
struct GGEMSVolumeRecipes {
  /*! \brief Authored groups in deterministic insertion order. */
  std::vector<GGEMSVolumeRecipe> recipes;

  /*! \brief Expanded physical descendants, excluding their mother. */
  std::uint32_t descendant_count{0U};

  /*! \brief Deepest descendant below the mother; at most three. */
  std::uint32_t descendant_depth{0U};
};

/*!
 * \brief Builds a compact Cartesian recipe with checked root multiplicity.
 * \param[in] child Owned immutable reference subtree.
 * \param[in] count Positive occurrence counts ordered X, Y, Z.
 * \param[in] pitch Nonnegative axis spacings in canonical picometers.
 * \return Linear recipe whose root count is the checked Cartesian product.
 * \throws core::GGEMSRecoverable If counts or pitches are invalid, or IDs
 * overflow.
 */
[[nodiscard]] auto
MakeLinearRecipe(std::shared_ptr<GGEMSAuthoredVolume const> child,
                 std::array<std::uint32_t, 3> count,
                 std::array<units::Length, 3> pitch) -> GGEMSVolumeRecipe;

/*!
 * \brief Maps one scalar linear repetition to a compact Cartesian recipe.
 * \param[in] child Owned immutable reference subtree.
 * \param[in] count Positive occurrence count along axis.
 * \param[in] pitch Nonnegative center spacing in canonical picometers.
 * \param[in] axis Mother-local axis receiving the count and pitch.
 * \return Centered linear recipe with singleton inactive axes.
 * \throws core::GGEMSRecoverable If axis, count, or pitch is invalid, or IDs
 * overflow.
 */
[[nodiscard]] auto
MakeLinearRecipe(std::shared_ptr<GGEMSAuthoredVolume const> child,
                 std::uint32_t count, units::Length pitch, GGEMSRepeatAxis axis)
  -> GGEMSVolumeRecipe;

/*!
 * \brief Forms one centered linear position with integer-picometer offsets.
 *
 * Fractional-picometer offsets truncate toward zero before adding the
 * reference.
 * \param[in] reference Captured mother-local reference position.
 * \param[in] index Zero-based root index; X varies fastest, then Y, then Z.
 * \param[in] recipe Linear recipe with checked positive Cartesian counts.
 * \return Reference position plus its centered Cartesian offset.
 */
[[nodiscard]] auto LinearPosition(Position3PM reference, std::uint32_t index,
                                  GGEMSVolumeRecipe const &recipe)
  -> Position3PM;

/*!
 * \brief Admits one user-authored recipe before any scientific mutation.
 *
 * Checks the complete reference shape, count, axis, and nonzero circular radius
 * for multiple copies. The linear factory already admits counts and pitches.
 * Final FP32, overlap, and containment admission belongs to staged
 * World preparation with its final pose and dimensions.
 * \param[in] recipe Finite placement with a non-null captured child.
 * \throws core::GGEMSRecoverable If a shape or placement parameter is invalid.
 */
auto ValidateRecipe(GGEMSVolumeRecipe const &recipe) -> void;

/*!
 * \brief Checks and counts all physical occurrences produced by a recipe.
 * \param[in] recipe Admitted recipe with a non-null captured child.
 * \return Root multiplicity times complete captured subtree size.
 * \throws core::GGEMSRecoverable If physical IDs overflow.
 */
[[nodiscard]] auto RecipeOccurrenceCount(GGEMSVolumeRecipe const &recipe)
  -> std::uint32_t;

} // namespace ggems::geometry::detail
