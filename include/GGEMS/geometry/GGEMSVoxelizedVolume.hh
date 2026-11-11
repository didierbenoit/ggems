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
 * \brief Declares an imported host voxel grid with one physical-volume pose.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <span>

#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSRotation3D.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/render/GGEMSColorTypes.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace ggems::geometry {

/*!
 * \brief Owns a centered voxel grid and an independently authored World pose.
 *
 * Copies share immutable imported labels and materials while retaining their
 * own pose. Original anatomical labels and compact material labels retain
 * the same X-fastest order. The volume has no global Material, child volumes,
 * or repeaters. World compiles its device location assets; cell traversal and
 * visualization remain deferred.
 */
class GGEMSVoxelizedVolume {
public:
  /*!
   * \brief Imports the admitted external little-endian MET_USHORT profile.
   * \param[in] image_file MHD metadata; its directory resolves the RAW
   * filename.
   * \param[in] material_file MIN MAX MATERIAL rows; equal endpoints select one
   * label, otherwise the interval is half-open.
   * \throws core::GGEMSRecoverable If files, metadata, ranges, materials, or
   * label coverage fail admission.
   */
  GGEMSVoxelizedVolume(std::filesystem::path const &image_file,
                       std::filesystem::path const &material_file);

  /*!
   * \brief Sets the World-frame position of the grid center.
   * \param[in] center Center in canonical picometers.
   * \return This volume for chained configuration.
   */
  auto SetPosition(Position3PM center) noexcept -> GGEMSVoxelizedVolume &;

  /*!
   * \brief Sets the absolute active rotation Rz * Ry * Rx.
   * \param[in] angle_x Rotation about the fixed X axis.
   * \param[in] angle_y Rotation about the fixed Y axis.
   * \param[in] angle_z Rotation about the fixed Z axis.
   * \return This volume for chained configuration.
   * \throws core::GGEMSRecoverable If the existing rotation admission fails.
   */
  auto SetRotation(units::Angle angle_x, units::Angle angle_y,
                   units::Angle angle_z) -> GGEMSVoxelizedVolume &;

  /*!
   * \brief Returns the grid-center placement.
   * \return World-frame center in picometers.
   */
  [[nodiscard]] auto GetPositionPM() const noexcept -> Position3PM;

  /*!
   * \brief Returns the immutable local-to-World orientation.
   * \return Validated rotation owned by this volume.
   */
  [[nodiscard]] auto GetRotation() const noexcept -> GGEMSRotation3D const &;

  /*!
   * \brief Returns the initial whole-volume display color.
   * \return Existing WHITE_White palette key.
   */
  [[nodiscard]] auto GetColor() const noexcept -> render::ColorKey;

  /*!
   * \brief Returns the grid dimensions.
   * \return Positive voxel counts along local X, Y, and Z.
   */
  [[nodiscard]] auto GetDimensions() const noexcept
    -> std::array<std::size_t, 3> const &;

  /*!
   * \brief Returns the canonical voxel spacing.
   * \return Positive picometer Length values along local X, Y, and Z.
   */
  [[nodiscard]] auto GetSpacing() const noexcept
    -> std::array<units::Length, 3> const &;

  /*!
   * \brief Returns dimensions times canonical spacing.
   * \return Full local sizes; centered bounds are minus/plus half these sizes.
   * Odd picometer sizes are retained without rounding the centered bounds.
   */
  [[nodiscard]] auto GetSize() const noexcept
    -> std::array<units::Length, 3> const &;

  /*!
   * \brief Returns the original labels without spatial merging or relabeling.
   * \return Immutable X-fastest uint16 labels owned by the shared import.
   */
  [[nodiscard]] auto GetLabels() const noexcept
    -> std::span<std::uint16_t const>;

  /*!
   * \brief Returns the locally owned, exactly deduplicated materials.
   * \return One value per scientific identity, independent of registry growth.
   */
  [[nodiscard]] auto GetMaterials() const noexcept
    -> std::span<core::materials::GGEMSMaterial const>;

  /*!
   * \brief Returns one compact local material label per voxel.
   * \return GetMaterials() indices in the same X-fastest order as GetLabels().
   * The original anatomical labels remain unchanged.
   */
  [[nodiscard]] auto GetMaterialLabels() const noexcept
    -> std::span<std::uint16_t const>;

private:
  /*! \brief Immutable imported grid and local material palette. */
  struct Data;

  /*! \brief Shared payload; construction completes before publication. */
  std::shared_ptr<Data const> data_;

  /*! \brief World-frame center, initially the origin. */
  Position3PM center_pm_{};

  /*! \brief Local-to-World rotation, initially identity. */
  GGEMSRotation3D rotation_{};
};

} // namespace ggems::geometry
