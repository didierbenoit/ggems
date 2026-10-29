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

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/geometry/GGEMSWorld.hh"

namespace ggems::python::detail {

/*!
 * \brief Keeps weak registrations for natural geometry binding methods.
 *
 * One instance is shared by the geometry bindings. World still copies each
 * complete shape immediately; later scientific setters do not update that
 * copy. A registered mother can append daughters to its original World, while
 * GGEMSRun::SetWorld retains its independent value-copy boundary. No scientific
 * objects are retained by this registry. Expired records are reclaimed on root
 * attachment or when the binding methods release the registry.
 *
 * \pre Calls are serialized (the binding methods hold the GIL). Registered
 * World pointees must not be replaced, moved from, or assigned over. The bound
 * Python API exposes none of those operations; this private helper is not a
 * general C++ attachment API. Each bound object has its own shared holder.
 */
class GGEMSGeometryAttachments {
public:
  /*!
   * \brief Copies and registers one direct World child.
   * \tparam Child One of the eight existing analytical shape types.
   * \param[in] world Bound World receiving the copied occurrence.
   * \param[in] child Bound complete shape to register.
   * \throws core::GGEMSRecoverable If either pointer is null, child is already
   * registered to a live World, or existing World admission rejects it.
   */
  template <class Child>
  auto AddVolume(std::shared_ptr<geometry::GGEMSWorld> const &world,
                 std::shared_ptr<Child> const &child) -> void {
    if (!world || !child) {
      throw core::GGEMSRecoverable("World and volume must be non-null.");
    }

    std::erase_if(registrations_, [](auto const &entry) -> bool {
      return entry.first.expired() || entry.second.world.expired();
    });

    Insert(world, child, 0U);
  }

  /*!
   * \brief Copies a daughter below an already registered mother.
   * \tparam Mother One of the eight existing analytical shape types.
   * \tparam Child One of the eight existing analytical shape types.
   * \param[in] mother Bound mother previously registered in a live World.
   * \param[in] child Bound complete shape, not already registered.
   * \throws core::GGEMSRecoverable If either pointer is null, the mother is
   * unregistered or its World expired, child is already registered, or existing
   * World shape/depth admission rejects it. Self and ancestor reuse reject.
   */
  template <class Mother, class Child>
  auto AddVolume(std::shared_ptr<Mother> const &mother,
                 std::shared_ptr<Child> const &child) -> void {
    if (!mother || !child) {
      throw core::GGEMSRecoverable("Mother and daughter must be non-null.");
    }

    auto const found = registrations_.find(std::weak_ptr<void>{mother});

    if (found == registrations_.end()) {
      throw core::GGEMSRecoverable("Register the mother in a World first.");
    }

    auto const world = found->second.world.lock();

    if (!world) {
      throw core::GGEMSRecoverable(
        "The mother's registered World has expired.");
    }

    Insert(world, child, found->second.volume_id);
  }

private:
  /*! \brief Weak World ownership and its existing internal occurrence ID. */
  struct Registration {
    /*! \brief World that owns the independent scientific value copy. */
    std::weak_ptr<geometry::GGEMSWorld> world;

    /*! \brief One-based occurrence ID, committed only after insertion. */
    std::uint32_t volume_id;
  };

  /*!
   * \brief Reserves registration before the original World insertion.
   * \tparam Child Existing analytical shape accepted by World::AddVolume.
   * \param[in] world Live World receiving the copied occurrence.
   * \param[in] child Non-null bound shape whose identity is registered.
   * \param[in] parent_volume_id Existing parent ID, or zero for World.
   * \throws core::GGEMSRecoverable If child already belongs to a live World
   * or World admission fails. Allocation failure precedes World mutation;
   * failed World insertion removes the reserved registration and rethrows.
   */
  template <class Child>
  auto Insert(std::shared_ptr<geometry::GGEMSWorld> const &world,
              std::shared_ptr<Child> const &child,
              std::uint32_t parent_volume_id) -> void {
    std::weak_ptr<void> const identity{child};
    auto const previous = registrations_.find(identity);

    if (previous != registrations_.end()) {
      if (!previous->second.world.expired()) {
        throw core::GGEMSRecoverable(
          "Volume is already registered in a World.");
      }
      registrations_.erase(previous);
    }

    auto const entry =
      registrations_
        .emplace(identity, Registration{.world = world, .volume_id = 0U})
        .first;

    try {
      world->AddVolume(*child, parent_volume_id);
    } catch (...) {
      registrations_.erase(entry);
      throw;
    }

    entry->second.volume_id =
      static_cast<std::uint32_t>(world->GetVolumeCount());
  }

  /*! \brief Weak owner keys distinguish reused object addresses. */
  std::map<std::weak_ptr<void>, Registration,
           std::owner_less<std::weak_ptr<void>>>
    registrations_;
};

} // namespace ggems::python::detail
