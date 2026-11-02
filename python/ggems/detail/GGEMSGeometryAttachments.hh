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
 * \brief Resolves bound geometry objects to existing World occurrence IDs.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <type_traits>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/geometry/GGEMSVolumeAuthoring.hh"
#include "GGEMS/geometry/GGEMSWorld.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace ggems::python::detail {

/*!
 * \brief Routes detached authoring and unique top-down World attachments.
 *
 * Detached solids own immutable child recipes in C++; this registry owns no
 * scientific objects. Each explicit ordinary attachment registers only its
 * subtree root for subsequent top-down additions to the original live World.
 * Captured descendants and repeater sources never acquire a World alias.
 * Later source setters do not change any captured occurrence.
 * GGEMSRun::SetWorld retains its independent completed-scene copy.
 *
 * \pre Calls hold the Python GIL. Registered World pointees must not be
 * replaced, moved from, or assigned over; the bound API exposes none of these
 * operations. Each bound object has its own shared holder.
 */
class GGEMSGeometryAttachments {
public:
  /*!
   * \brief Captures one child subtree under a detached or attached parent.
   * \tparam Parent Bound World or one of the eight analytical mother types.
   * \tparam Child One of the eight existing analytical shape types.
   * \param[in] parent Detached mother, registered mother, or World.
   * \param[in] child Complete child whose current value is captured.
   * \throws core::GGEMSRecoverable If either pointer is null, self-use or live
   * child reuse occurs, the parent's registered World expired, or authoring
   * admission fails. Every World attachment requires configured size and
   * material and commits atomically, including an ordinary leaf.
   */
  template <class Parent, class Child>
  auto AddVolume(std::shared_ptr<Parent> const &parent,
                 std::shared_ptr<Child> const &child) -> void {
    CheckChild(parent, child);
    auto const target = ResolveParent(parent);
    if (target.world) {
      Insert(target.world, child, target.volume_id);
    } else {
      parent->AddVolume(*child);
    }
  }

  /*!
   * \brief Captures a linear subtree repeater in the parent's local frame.
   * \tparam Parent Bound World or one of the eight analytical mother types.
   * \tparam Child One of the eight existing analytical shape types.
   * \param[in] parent Detached mother, registered mother, or World.
   * \param[in] child Complete reference subtree captured at occurrence zero.
   * \param[in] count Total number of physical copies, including index zero.
   * \param[in] pitch Center-to-center displacement in canonical picometers.
   * \param[in] axis Positive parent-local repetition axis.
   * \throws core::GGEMSRecoverable If ownership, repetition, depth, capacity,
   * or geometry admission fails. No source registration or partial batch is
   * published on failure.
   */
  template <class Parent, class Child>
  auto LinearRepeater(std::shared_ptr<Parent> const &parent,
                      std::shared_ptr<Child> const &child, std::uint32_t count,
                      units::Length pitch, geometry::GGEMSRepeatAxis axis)
    -> void {
    CheckChild(parent, child);
    auto const target = ResolveParent(parent);
    if (target.world) {
      target.world->LinearRepeater(*child, count, pitch, axis,
                                   target.volume_id);
    } else {
      parent->LinearRepeater(*child, count, pitch, axis);
    }
  }

  /*!
   * \brief Captures rigidly rotated subtree copies around the parent origin.
   * \tparam Parent Bound World or one of the eight analytical mother types.
   * \tparam Child One of the eight existing analytical shape types.
   * \param[in] parent Detached mother, registered mother, or World.
   * \param[in] child Complete reference subtree captured at occurrence zero.
   * \param[in] count Total physical copies over one turn, with no end
   * duplicate.
   * \param[in] axis Positive parent-local axis through the parent's origin.
   * \throws core::GGEMSRecoverable If ownership, repetition, depth, capacity,
   * or geometry admission fails. No source registration or partial batch is
   * published on failure.
   */
  template <class Parent, class Child>
  auto CircularRepeater(std::shared_ptr<Parent> const &parent,
                        std::shared_ptr<Child> const &child,
                        std::uint32_t count, geometry::GGEMSRepeatAxis axis)
    -> void {
    CheckChild(parent, child);
    auto const target = ResolveParent(parent);
    if (target.world) {
      target.world->CircularRepeater(*child, count, axis, target.volume_id);
    } else {
      parent->CircularRepeater(*child, count, axis);
    }
  }

private:
  /*! \brief Weak World ownership and one explicit subtree-root identity. */
  struct Registration {
    /*! \brief World owning an independent scientific value copy. */
    std::weak_ptr<geometry::GGEMSWorld> world;

    /*! \brief One-based root VolumeID, committed after successful insertion. */
    std::uint32_t volume_id;
  };

  /*! \brief Temporarily retains a live target during one binding call. */
  struct AttachmentTarget {
    /*! \brief Target World, or null for detached authoring. */
    std::shared_ptr<geometry::GGEMSWorld> world;

    /*! \brief Parent ID, or zero for World and detached authoring. */
    std::uint32_t volume_id{0U};
  };

  /*!
   * \brief Rejects null pointers, self-use, and live child reuse.
   * \tparam Parent Bound World or analytical mother type.
   * \tparam Child Bound analytical child type.
   * \param[in] parent Parent identity used to reject direct self-capture.
   * \param[in] child Child identity checked against unique live attachments.
   * \throws core::GGEMSRecoverable If a pointer is null, identities match, or
   * the child already has a live registered World occurrence.
   */
  template <class Parent, class Child>
  auto CheckChild(std::shared_ptr<Parent> const &parent,
                  std::shared_ptr<Child> const &child) const -> void {
    if (!parent || !child) {
      throw core::GGEMSRecoverable("Parent and volume must be non-null.");
    }
    if (static_cast<void const *>(parent.get()) ==
        static_cast<void const *>(child.get())) {
      throw core::GGEMSRecoverable("A volume cannot capture itself.");
    }
    auto const found = registrations_.find(std::weak_ptr<void>{child});
    if (found != registrations_.end() && !found->second.world.expired()) {
      throw core::GGEMSRecoverable("Volume is already registered in a World.");
    }
  }

  /*!
   * \brief Resolves a unique live World target or a detached mother.
   * \tparam Parent Bound World or analytical mother type.
   * \param[in] parent Non-null bound parent.
   * \return Live World and parent ID, or an empty target for a detached mother.
   * \throws core::GGEMSRecoverable If the registered mother's World expired.
   */
  template <class Parent>
  [[nodiscard]] auto ResolveParent(std::shared_ptr<Parent> const &parent)
    -> AttachmentTarget {
    if constexpr (std::is_same_v<Parent, geometry::GGEMSWorld>) {
      std::erase_if(registrations_, [](auto const &entry) -> bool {
        return entry.first.expired() || entry.second.world.expired();
      });
      return {.world = parent, .volume_id = 0U};
    } else {
      auto const found = registrations_.find(std::weak_ptr<void>{parent});
      if (found == registrations_.end()) {
        return {};
      }
      auto const world = found->second.world.lock();
      if (!world) {
        throw core::GGEMSRecoverable(
          "The mother's registered World has expired.");
      }
      return {.world = world, .volume_id = found->second.volume_id};
    }
  }

  /*!
   * \brief Reserves a root registration and commits it after World insertion.
   * \tparam Child Existing analytical shape accepted by World::AttachVolume.
   * \param[in] world Live World receiving the independent subtree snapshot.
   * \param[in] child Non-null shape without a live registration.
   * \param[in] parent_volume_id Existing parent ID, or zero for World.
   * \throws core::GGEMSRecoverable If World admission fails. Registration
   * allocation precedes World mutation; failed insertion erases the
   * reservation.
   */
  template <class Child>
  auto Insert(std::shared_ptr<geometry::GGEMSWorld> const &world,
              std::shared_ptr<Child> const &child,
              std::uint32_t parent_volume_id) -> void {
    std::weak_ptr<void> const identity{child};
    registrations_.erase(identity);
    auto const entry =
      registrations_
        .emplace(identity, Registration{.world = world, .volume_id = 0U})
        .first;
    auto const previous_count = world->GetVolumeCount();
    try {
      world->AttachVolume(*child, parent_volume_id);
    } catch (...) {
      registrations_.erase(entry);
      throw;
    }
    entry->second.volume_id = static_cast<std::uint32_t>(previous_count + 1U);
  }

  /*! \brief Weak identities distinguish reused addresses without ownership. */
  std::map<std::weak_ptr<void>, Registration,
           std::owner_less<std::weak_ptr<void>>>
    registrations_;
};

} // namespace ggems::python::detail
