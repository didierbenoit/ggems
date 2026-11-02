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

  /*! \brief Total root count, including the captured reference pose. */
  std::uint32_t count{1U};

  /*! \brief Center-to-center linear displacement in canonical picometers. */
  units::Length pitch{0U};

  /*! \brief Positive axis through the mother's local origin. */
  GGEMSRepeatAxis axis{GGEMSRepeatAxis::X};
};

/*! \brief Stores one immutable direct-child list and its expansion bounds. */
struct GGEMSVolumeRecipes {
  /*! \brief Authored groups in deterministic insertion order. */
  std::vector<GGEMSVolumeRecipe> recipes;

  /*! \brief Expanded physical descendants, excluding their mother. */
  std::uint32_t descendant_count{0U};

  /*! \brief Deepest descendant below the mother; at most two. */
  std::uint32_t descendant_depth{0U};
};

/*!
 * \brief Forms one exact checked linear reference position in integer pm.
 * \param[in] reference Captured mother-local reference position.
 * \param[in] index Zero-based physical copy index.
 * \param[in] pitch Nonnegative center displacement in canonical picometers.
 * \param[in] axis Positive mother-local axis.
 * \return Reference position translated by index times pitch along axis.
 * \throws core::GGEMSRecoverable If axis is invalid or the signed endpoint
 * cannot be represented; no floating-point conversion is used.
 */
[[nodiscard]] auto LinearPosition(Position3PM reference, std::uint32_t index,
                                  units::Length pitch, GGEMSRepeatAxis axis)
  -> Position3PM;

/*!
 * \brief Admits one user-authored recipe before any scientific mutation.
 *
 * Checks the complete reference shape, count, axis, linear pitch and final
 * translated coordinate, and nonzero circular radius for multiple copies.
 * Final FP32, overlap, and containment admission belongs to staged World
 * preparation with its final pose and dimensions.
 * \param[in] recipe Finite placement with a non-null captured child.
 * \throws core::GGEMSRecoverable If a shape or placement parameter is invalid.
 */
auto ValidateRecipe(GGEMSVolumeRecipe const &recipe) -> void;

/*!
 * \brief Checks and counts all physical occurrences produced by a recipe.
 * \param[in] recipe Recipe with a non-null captured child.
 * \return Root multiplicity times complete captured subtree size.
 * \throws core::GGEMSRecoverable If count is zero or physical IDs overflow.
 */
[[nodiscard]] auto RecipeOccurrenceCount(GGEMSVolumeRecipe const &recipe)
  -> std::uint32_t;

} // namespace ggems::geometry::detail
