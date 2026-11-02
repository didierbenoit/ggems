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
 * \brief Captures and admits immutable parent-local analytical child recipes.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include "GGEMS/geometry/GGEMSVolumeAuthoring.hh"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <memory>
#include <span>
#include <utility>
#include <variant>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/geometry/GGEMSBox.hh"
#include "GGEMS/geometry/GGEMSSphere.hh"
#include "GGEMS/geometry/GGEMSTubs.hh"
#include "GGEMS/geometry/GGEMSCons.hh"
#include "GGEMS/geometry/GGEMSPolyhedra.hh"
#include "GGEMS/geometry/GGEMSEllipsoid.hh"
#include "GGEMS/geometry/GGEMSTrd.hh"
#include "GGEMS/geometry/GGEMSTrap.hh"
#include "GGEMS/geometry/detail/GGEMSVolumeRecipe.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace ggems::geometry::detail {

auto LinearPosition(Position3PM reference, std::uint32_t index,
                    units::Length pitch, GGEMSRepeatAxis axis) -> Position3PM {
  CoordinatePM *coordinate = nullptr;
  switch (axis) {
  case GGEMSRepeatAxis::X:
    coordinate = &reference.x;
    break;
  case GGEMSRepeatAxis::Y:
    coordinate = &reference.y;
    break;
  case GGEMSRepeatAxis::Z:
    coordinate = &reference.z;
    break;
  default:
    throw core::GGEMSRecoverable("Repeater axis must be X, Y, or Z.");
  }

  auto const available =
    static_cast<std::uint64_t>(std::numeric_limits<CoordinatePM>::max()) -
    static_cast<std::uint64_t>(*coordinate);
  if (index != 0U && pitch.value > available / index) {
    throw core::GGEMSRecoverable(
      "Linear repeater endpoint exceeds the signed picometer domain.");
  }
  auto const delta = pitch.value * index;
  if (*coordinate >= 0) {
    *coordinate += static_cast<CoordinatePM>(delta);
  } else {
    auto const magnitude = 0U - static_cast<std::uint64_t>(*coordinate);
    if (delta >= magnitude) {
      *coordinate = static_cast<CoordinatePM>(delta - magnitude);
    } else {
      auto const remaining = magnitude - delta;
      *coordinate = remaining > static_cast<std::uint64_t>(
                                  std::numeric_limits<CoordinatePM>::max())
                      ? std::numeric_limits<CoordinatePM>::min()
                      : -static_cast<CoordinatePM>(remaining);
    }
  }
  return reference;
}

// -----------------------------------------------------------------------------

auto ValidateRecipe(GGEMSVolumeRecipe const &recipe) -> void {
  if (recipe.count == 0U) {
    throw core::GGEMSRecoverable("Repeater count must be positive.");
  }
  if (recipe.axis != GGEMSRepeatAxis::X && recipe.axis != GGEMSRepeatAxis::Y &&
      recipe.axis != GGEMSRepeatAxis::Z) {
    throw core::GGEMSRecoverable("Repeater axis must be X, Y, or Z.");
  }

  auto const position = std::visit(
    [](auto const &shape) -> Position3PM {
      shape.Validate();
      return shape.GetPositionPM();
    },
    recipe.child->volume);

  if (recipe.kind == GGEMSRepeatKind::Linear && recipe.count > 1U) {
    if (recipe.pitch.value == 0U) {
      throw core::GGEMSRecoverable(
        "Linear repeater pitch must be positive for multiple occurrences.");
    }
    static_cast<void>(
      LinearPosition(position, recipe.count - 1U, recipe.pitch, recipe.axis));
  }

  if (recipe.kind == GGEMSRepeatKind::Circular && recipe.count > 1U) {
    bool on_axis = false;
    switch (recipe.axis) {
    case GGEMSRepeatAxis::X:
      on_axis = position.y == 0 && position.z == 0;
      break;
    case GGEMSRepeatAxis::Y:
      on_axis = position.x == 0 && position.z == 0;
      break;
    case GGEMSRepeatAxis::Z:
      on_axis = position.x == 0 && position.y == 0;
      break;
    }
    if (on_axis) {
      throw core::GGEMSRecoverable(
        "Circular repeater radius must be nonzero for multiple occurrences.");
    }
  }
}

// -----------------------------------------------------------------------------

auto RecipeOccurrenceCount(GGEMSVolumeRecipe const &recipe) -> std::uint32_t {
  auto const subtree_count = std::visit(
    [](auto const &shape) -> std::uint32_t {
      return shape.GetDescendantCount() + 1U;
    },
    recipe.child->volume);
  auto constexpr maximum = std::numeric_limits<std::uint32_t>::max() - 1U;
  if (recipe.count == 0U || recipe.count > maximum / subtree_count) {
    throw core::GGEMSRecoverable(
      "Repeater expansion exceeds the physical VolumeID domain.");
  }
  return recipe.count * subtree_count;
}

} // namespace ggems::geometry::detail

// =============================================================================
// =============================================================================

namespace ggems::geometry {

auto GGEMSVolumeAuthoring::GetChildRecipes() const noexcept
  -> std::span<detail::GGEMSVolumeRecipe const> {
  return children_
           ? std::span<detail::GGEMSVolumeRecipe const>{children_->recipes}
           : std::span<detail::GGEMSVolumeRecipe const>{};
}

// -----------------------------------------------------------------------------

auto GGEMSVolumeAuthoring::GetDescendantCount() const noexcept
  -> std::uint32_t {
  return children_ ? children_->descendant_count : 0U;
}

// -----------------------------------------------------------------------------

auto GGEMSVolumeAuthoring::GetDescendantDepth() const noexcept
  -> std::uint32_t {
  return children_ ? children_->descendant_depth : 0U;
}

// -----------------------------------------------------------------------------

auto GGEMSVolumeAuthoring::ClearChildRecipes() noexcept -> void {
  children_.reset();
}

// -----------------------------------------------------------------------------

auto GGEMSVolumeAuthoring::RequireDifferent(
  GGEMSVolumeAuthoring const &child) const -> void {
  if (this == &child) {
    throw core::GGEMSRecoverable("A volume cannot capture itself as a child.");
  }
}

// -----------------------------------------------------------------------------

auto GGEMSVolumeAuthoring::Append(
  std::shared_ptr<detail::GGEMSAuthoredVolume const> child,
  detail::GGEMSRepeatKind kind, std::uint32_t count, units::Length pitch,
  GGEMSRepeatAxis axis) -> void {
  detail::GGEMSVolumeRecipe recipe{
    .child = std::move(child),
    .kind = kind,
    .count = count,
    .pitch = pitch,
    .axis = axis,
  };
  detail::ValidateRecipe(recipe);
  auto const additional = detail::RecipeOccurrenceCount(recipe);
  auto const depth = std::visit(
    [](auto const &shape) -> std::uint32_t {
      return shape.GetDescendantDepth() + 1U;
    },
    recipe.child->volume);
  if (depth > 2U) {
    throw core::GGEMSRecoverable("Detached subtree cannot exceed three "
                                 "physical levels including its root.");
  }
  auto constexpr maximum = std::numeric_limits<std::uint32_t>::max() - 2U;
  if (additional > maximum - GetDescendantCount()) {
    throw core::GGEMSRecoverable(
      "Authored descendants exceed the physical VolumeID domain.");
  }

  auto updated = children_
                   ? std::make_shared<detail::GGEMSVolumeRecipes>(*children_)
                   : std::make_shared<detail::GGEMSVolumeRecipes>();
  updated->recipes.push_back(std::move(recipe));
  updated->descendant_count += additional;
  updated->descendant_depth = std::max(updated->descendant_depth, depth);
  children_ = std::move(updated);
}

// -----------------------------------------------------------------------------

auto GGEMSVolumeAuthoring::Capture(GGEMSBox const &child)
  -> std::shared_ptr<detail::GGEMSAuthoredVolume const> {
  return std::make_shared<detail::GGEMSAuthoredVolume const>(
    detail::GGEMSAuthoredVolume{.volume = child});
}

// -----------------------------------------------------------------------------

auto GGEMSVolumeAuthoring::Capture(GGEMSSphere const &child)
  -> std::shared_ptr<detail::GGEMSAuthoredVolume const> {
  return std::make_shared<detail::GGEMSAuthoredVolume const>(
    detail::GGEMSAuthoredVolume{.volume = child});
}

// -----------------------------------------------------------------------------

auto GGEMSVolumeAuthoring::Capture(GGEMSTubs const &child)
  -> std::shared_ptr<detail::GGEMSAuthoredVolume const> {
  return std::make_shared<detail::GGEMSAuthoredVolume const>(
    detail::GGEMSAuthoredVolume{.volume = child});
}

// -----------------------------------------------------------------------------

auto GGEMSVolumeAuthoring::Capture(GGEMSCons const &child)
  -> std::shared_ptr<detail::GGEMSAuthoredVolume const> {
  return std::make_shared<detail::GGEMSAuthoredVolume const>(
    detail::GGEMSAuthoredVolume{.volume = child});
}

// -----------------------------------------------------------------------------

auto GGEMSVolumeAuthoring::Capture(GGEMSPolyhedra const &child)
  -> std::shared_ptr<detail::GGEMSAuthoredVolume const> {
  return std::make_shared<detail::GGEMSAuthoredVolume const>(
    detail::GGEMSAuthoredVolume{.volume = child});
}

// -----------------------------------------------------------------------------

auto GGEMSVolumeAuthoring::Capture(GGEMSEllipsoid const &child)
  -> std::shared_ptr<detail::GGEMSAuthoredVolume const> {
  return std::make_shared<detail::GGEMSAuthoredVolume const>(
    detail::GGEMSAuthoredVolume{.volume = child});
}

// -----------------------------------------------------------------------------

auto GGEMSVolumeAuthoring::Capture(GGEMSTrd const &child)
  -> std::shared_ptr<detail::GGEMSAuthoredVolume const> {
  return std::make_shared<detail::GGEMSAuthoredVolume const>(
    detail::GGEMSAuthoredVolume{.volume = child});
}

// -----------------------------------------------------------------------------

auto GGEMSVolumeAuthoring::Capture(GGEMSTrap const &child)
  -> std::shared_ptr<detail::GGEMSAuthoredVolume const> {
  return std::make_shared<detail::GGEMSAuthoredVolume const>(
    detail::GGEMSAuthoredVolume{.volume = child});
}

} // namespace ggems::geometry
