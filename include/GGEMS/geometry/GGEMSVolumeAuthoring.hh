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
 * \brief Owns immutable, parent-local analytical child recipes.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>
#include <memory>
#include <span>

#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace ggems::geometry {

class GGEMSBox;
class GGEMSSphere;
class GGEMSTubs;
class GGEMSCons;
class GGEMSPolyhedra;
class GGEMSEllipsoid;
class GGEMSTrd;
class GGEMSTrap;

/*! \brief Selects a positive axis in the physical mother's local frame. */
enum class GGEMSRepeatAxis : std::uint8_t {
  /*! \brief Positive local X. */
  X,

  /*! \brief Positive local Y. */
  Y,

  /*! \brief Positive local Z. */
  Z,
};

namespace detail {

/*! \brief Selects the finite placement rule of a captured child subtree. */
enum class GGEMSRepeatKind : std::uint8_t {
  /*! \brief One occurrence at the captured pose. */
  Single,

  /*! \brief Constant positive local-axis translation between centers. */
  Linear,

  /*! \brief Right-handed rigid rotation around the mother's local origin. */
  Circular,
};

struct GGEMSAuthoredVolume;
struct GGEMSVolumeRecipe;
struct GGEMSVolumeRecipes;

} // namespace detail

/*!
 * \brief Gives analytical solids immutable owned child descriptions.
 *
 * Each child operation captures the child's current value and descendants.
 * Later source setters or child additions cannot change that snapshot. The
 * mother's own pose remains independent until it is captured by another mother
 * or World. Immutable sharing preserves deep value semantics without Python
 * references or a mutable physical tree. Final World attachment expands the
 * descriptions and performs complete geometry admission atomically.
 */
class GGEMSVolumeAuthoring {
public:
  /*!
   * \brief Captures one complete child subtree in the mother's local frame.
   * \tparam Self Concrete analytical mother type.
   * \tparam Child One of the eight analytical child types.
   * \param[in,out] self Mother retaining the captured description.
   * \param[in] child Complete child value copied at this call.
   * \return This concrete mother for chained configuration.
   * \throws core::GGEMSRecoverable If child is self, incomplete, too deep, or
   * the expanded occurrence count cannot fit the physical ID domain.
   */
  template <class Self, class Child>
  auto AddVolume(this Self &self, Child const &child) -> Self & {
    self.RequireDifferent(child);
    self.Append(Capture(child), detail::GGEMSRepeatKind::Single, 1U,
                units::Length{0U}, GGEMSRepeatAxis::X);
    return self;
  }

  /*!
   * \brief Captures translated copies including the reference occurrence.
   *
   * Copy i has position p + i * pitch along axis and unchanged orientation.
   * Placement is parent-local; full overlap and containment admission occurs
   * when the complete subtree is attached to a configured World.
   * \tparam Self Concrete analytical mother type.
   * \tparam Child One of the eight analytical child types.
   * \param[in,out] self Mother retaining the captured description.
   * \param[in] child Complete reference subtree copied at this call.
   * \param[in] count Total occurrence count, including index zero.
   * \param[in] pitch Canonical center-to-center distance in picometers.
   * \param[in] axis Positive mother-local axis.
   * \return This concrete mother for chained configuration.
   * \throws core::GGEMSRecoverable If self-reference, shape, count, axis,
   * pitch, endpoint representability, depth, or physical ID capacity fails.
   */
  template <class Self, class Child>
  auto LinearRepeater(this Self &self, Child const &child, std::uint32_t count,
                      units::Length pitch, GGEMSRepeatAxis axis) -> Self & {
    self.RequireDifferent(child);
    self.Append(Capture(child), detail::GGEMSRepeatKind::Linear, count, pitch,
                axis);
    return self;
  }

  /*!
   * \brief Captures a full turn of rigidly rotated child subtrees.
   *
   * Copy i rotates both reference position and orientation by 2*pi*i/count
   * about the mother-local axis through its origin. Index zero is unchanged;
   * there is no extra 360-degree copy. Full geometric admission occurs at World
   * attachment using the final composed pose and existing FP32 policy.
   * \tparam Self Concrete analytical mother type.
   * \tparam Child One of the eight analytical child types.
   * \param[in,out] self Mother retaining the captured description.
   * \param[in] child Complete reference subtree copied at this call.
   * \param[in] count Total occurrence count, including index zero.
   * \param[in] axis Positive mother-local rotation axis.
   * \return This concrete mother for chained configuration.
   * \throws core::GGEMSRecoverable If self-reference, shape, count, axis,
   * zero-radius multiple placement, depth, or physical ID capacity fails.
   */
  template <class Self, class Child>
  auto CircularRepeater(this Self &self, Child const &child,
                        std::uint32_t count, GGEMSRepeatAxis axis) -> Self & {
    self.RequireDifferent(child);
    self.Append(Capture(child), detail::GGEMSRepeatKind::Circular, count,
                units::Length{0U}, axis);
    return self;
  }

  /*!
   * \brief Borrows the immutable direct-child placement recipes.
   * \return Parent-local recipes, valid until this object changes or dies.
   */
  [[nodiscard]] auto GetChildRecipes() const noexcept
    -> std::span<detail::GGEMSVolumeRecipe const>;

  /*!
   * \brief Counts physical descendants after finite repeater expansion.
   * \return Total descendant count, excluding this volume.
   */
  [[nodiscard]] auto GetDescendantCount() const noexcept -> std::uint32_t;

  /*!
   * \brief Returns the deepest authored descendant below this volume.
   * \return Zero for a leaf, otherwise one or two physical levels.
   */
  [[nodiscard]] auto GetDescendantDepth() const noexcept -> std::uint32_t;

  /*!
   * \brief Releases this value's recipes after host occurrence expansion.
   *
   * Independently captured snapshots remain unchanged. This operation does not
   * modify any already attached World occurrence.
   */
  auto ClearChildRecipes() noexcept -> void;

private:
  /*!
   * \brief Rejects directly capturing the mother as its own child.
   * \param[in] child Candidate child's authoring base.
   * \throws core::GGEMSRecoverable If child is this authoring value.
   */
  auto RequireDifferent(GGEMSVolumeAuthoring const &child) const -> void;

  /*!
   * \brief Validates and atomically appends one immutable child recipe.
   * \param[in] child Owned complete child snapshot.
   * \param[in] kind Single, linear, or circular placement.
   * \param[in] count Total reference-root multiplicity.
   * \param[in] pitch Linear center displacement in canonical picometers.
   * \param[in] axis Mother-local positive axis.
   * \throws core::GGEMSRecoverable If recipe, depth, or count admission fails.
   */
  auto Append(std::shared_ptr<detail::GGEMSAuthoredVolume const> child,
              detail::GGEMSRepeatKind kind, std::uint32_t count,
              units::Length pitch, GGEMSRepeatAxis axis) -> void;

  /*!
   * \brief Copies one complete Box and its immutable descendants.
   * \param[in] child Source value to capture.
   * \return Owned immutable snapshot, independent of later source edits.
   */
  [[nodiscard]] static auto Capture(GGEMSBox const &child)
    -> std::shared_ptr<detail::GGEMSAuthoredVolume const>;

  /*!
   * \brief Copies one complete Sphere and its immutable descendants.
   * \param[in] child Source value to capture.
   * \return Owned immutable snapshot, independent of later source edits.
   */
  [[nodiscard]] static auto Capture(GGEMSSphere const &child)
    -> std::shared_ptr<detail::GGEMSAuthoredVolume const>;

  /*!
   * \brief Copies one complete Tubs and its immutable descendants.
   * \param[in] child Source value to capture.
   * \return Owned immutable snapshot, independent of later source edits.
   */
  [[nodiscard]] static auto Capture(GGEMSTubs const &child)
    -> std::shared_ptr<detail::GGEMSAuthoredVolume const>;

  /*!
   * \brief Copies one complete Cons and its immutable descendants.
   * \param[in] child Source value to capture.
   * \return Owned immutable snapshot, independent of later source edits.
   */
  [[nodiscard]] static auto Capture(GGEMSCons const &child)
    -> std::shared_ptr<detail::GGEMSAuthoredVolume const>;

  /*!
   * \brief Copies one complete Polyhedra and its immutable descendants.
   * \param[in] child Source value to capture.
   * \return Owned immutable snapshot, independent of later source edits.
   */
  [[nodiscard]] static auto Capture(GGEMSPolyhedra const &child)
    -> std::shared_ptr<detail::GGEMSAuthoredVolume const>;

  /*!
   * \brief Copies one complete Ellipsoid and its immutable descendants.
   * \param[in] child Source value to capture.
   * \return Owned immutable snapshot, independent of later source edits.
   */
  [[nodiscard]] static auto Capture(GGEMSEllipsoid const &child)
    -> std::shared_ptr<detail::GGEMSAuthoredVolume const>;

  /*!
   * \brief Copies one complete Trd and its immutable descendants.
   * \param[in] child Source value to capture.
   * \return Owned immutable snapshot, independent of later source edits.
   */
  [[nodiscard]] static auto Capture(GGEMSTrd const &child)
    -> std::shared_ptr<detail::GGEMSAuthoredVolume const>;

  /*!
   * \brief Copies one complete Trap and its immutable descendants.
   * \param[in] child Source value to capture.
   * \return Owned immutable snapshot, independent of later source edits.
   */
  [[nodiscard]] static auto Capture(GGEMSTrap const &child)
    -> std::shared_ptr<detail::GGEMSAuthoredVolume const>;

  /*! \brief Shared immutable descriptions; absent for a leaf. */
  std::shared_ptr<detail::GGEMSVolumeRecipes const> children_;
};

} // namespace ggems::geometry
