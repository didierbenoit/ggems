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
 * \brief Owns the parent-local rigid pose of an authored volume.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <memory>
#include <utility>
#include <variant>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSRotation3D.hh"

namespace ggems::geometry {

/*! \brief Keeps value-copy poses independent and shares one explicit World
 * attachment. */
class GGEMSRigidPose {
protected:
  /*! \brief Integer parent-local position and proper local-to-parent basis. */
  struct Coordinates {
    /*! \brief Parent-frame location of the local origin in picometers. */
    Position3PM position_pm{};

    /*! \brief Active orientation about the local origin. */
    GGEMSRotation3D rotation{};
  };

  /*! \brief Initializes a detached identity pose. */
  GGEMSRigidPose() noexcept = default;

  /*!
   * \brief Copies the effective pose as an independent value.
   * \param[in] other Source of the parent-local position and rotation.
   */
  GGEMSRigidPose(GGEMSRigidPose const &other) noexcept
      : pose_(other.LocalPose()) {}

  /*!
   * \brief Replaces this pose with an independent copy.
   * \param[in] other Source of the parent-local position and rotation.
   * \return This pose.
   */
  auto operator=(GGEMSRigidPose const &other) noexcept -> GGEMSRigidPose & {
    if (this != &other) {
      pose_ = Coordinates{other.LocalPose()};
    }

    return *this;
  }

  /*!
   * \brief Transfers the pose and its live attachment, if present.
   * \param[in,out] other Source reset to the identity pose.
   */
  GGEMSRigidPose(GGEMSRigidPose &&other) noexcept
      : pose_(std::exchange(other.pose_, Coordinates{})) {}

  /*!
   * \brief Takes ownership of another pose and its live attachment, if present.
   * \param[in,out] other Source reset to the identity pose on transfer.
   * \return This pose.
   */
  auto operator=(GGEMSRigidPose &&other) noexcept -> GGEMSRigidPose & {
    if (this != &other) {
      pose_ = std::exchange(other.pose_, Coordinates{});
    }

    return *this;
  }

  /*!
   * \brief Accesses the volume's current parent-local pose.
   * \tparam Self Concrete pose owner with its original const qualification.
   * \param[in] self Volume owning or sharing the pose.
   * \return Borrowed position and rotation with the owner's const
   * qualification.
   */
  template <class Self>
  [[nodiscard]] auto LocalPose(this Self &self) noexcept
    -> decltype(std::get<Coordinates>(self.pose_)) {

    if (auto const *shared = std::get_if<Shared>(&self.pose_)) {
      return **shared;
    }

    return std::get<Coordinates>(self.pose_);
  }

public:
  /*! \brief Captures static authoring data and shares the source's rigid pose.
   * \tparam Shape Concrete native volume type.
   * \param[in,out] source Explicit volume being attached.
   * \return An occurrence with an independently captured shape and Material.
   * \throws core::GGEMSRecoverable If the source already has a live occurrence.
   */
  template <class Shape>
  [[nodiscard]] auto CaptureForAttachment(this Shape &source) -> Shape {
    if (auto const *shared = std::get_if<Shared>(&source.pose_);
        shared != nullptr && shared->use_count() > 1) {
      throw core::GGEMSRecoverable("Volume already has a live attachment.");
    }

    auto capture = source;
    auto shared = std::make_shared<Coordinates>(source.LocalPose());
    source.pose_ = shared;
    capture.pose_ = std::move(shared);

    return capture;
  }

private:
  /*! \brief Pose ownership shared only by the source and its explicitly
   * attached copy. */
  using Shared = std::shared_ptr<Coordinates>;

  /*! \brief Detached poses stay inline; only explicit attachments allocate. */
  std::variant<Coordinates, Shared> pose_{Coordinates{}};
};

} // namespace ggems::geometry
