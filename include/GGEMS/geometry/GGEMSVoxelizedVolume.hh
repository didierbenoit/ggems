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
#include <string>

#include "GGEMS/geometry/GGEMSRigidPose.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSRotation3D.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/render/GGEMSColorTypes.hh"
#include "GGEMS/render/GGEMSGeometryAppearance.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace ggems::geometry {

/*!
 * \brief Resolved display metadata for one original voxel-label mapping.
 *
 * Presentation does not participate in physical Material identity or compact
 * material labels, including when equal Materials have different styles.
 */
struct GGEMSVoxelDisplayStyle {
  /*! \brief Mapping mode; three-column rows resolve to Solid. */
  render::GGEMSGeometryDisplayMode mode;

  /*! \brief Explicit mapping color or the named Material's default color. */
  render::ColorKey color;
};

/*! \brief Immutable presentation identity of one admitted material-file row. */
struct GGEMSVoxelDisplayMapping {
  /*! \brief Inclusive original anatomical label. */
  std::uint32_t first_label;

  /*! \brief Exclusive endpoint; 65536 includes the largest uint16 label. */
  std::uint32_t end_label;

  /*!
   * \brief Authored Material name, preserved before scientific deduplication.
   */
  std::string material_name;

  /*!
   * \brief Initial mode and color independent of scientific palette identity.
   */
  GGEMSVoxelDisplayStyle style;
};

/*!
 * \brief Owns a centered voxel grid and an independently authored World pose.
 *
 * Copies share immutable imported labels, materials, and mapping presentation
 * while retaining their own pose. Original anatomical labels and compact
 * material labels retain the same X-fastest order. The volume has no global
 * Material, child volumes, or repeaters. World compiles physical device assets;
 * mapping presentation remains host/viewer metadata.
 */
class GGEMSVoxelizedVolume : public GGEMSRigidPose {
public:
  /*!
   * \brief Imports the admitted external little-endian MET_USHORT profile.
   * \param[in] image_file MHD metadata; its directory resolves the RAW
   * filename.
   * \param[in] material_file MIN MAX MATERIAL rows, optionally followed by MODE
   * COLOR. Equal endpoints select one label; other intervals are half-open.
   * Three-column rows use Solid and the Material color. Five-column rows accept
   * None, Wireframe, or Solid and one canonical GGEMS color name.
   * \throws core::GGEMSRecoverable If files, metadata, ranges, materials, or
   * label coverage fail admission.
   */
  GGEMSVoxelizedVolume(std::filesystem::path const &image_file,
                       std::filesystem::path const &material_file);

  /*!
   * \brief Imports the same admitted voxel data with a name-first display
   * label.
   * \param[in] name Owned label; empty selects the viewer's generic fallback.
   * \param[in] image_file MHD metadata; its directory resolves the RAW
   * filename.
   * \param[in] material_file Admitted three- or five-column material mapping.
   * \throws core::GGEMSRecoverable If files, metadata, ranges, materials, or
   * label coverage fail admission.
   */
  GGEMSVoxelizedVolume(std::string name,
                       std::filesystem::path const &image_file,
                       std::filesystem::path const &material_file);

  /*!
   * \brief Returns the captured authoring name independently of the payload.
   * \return Owned label; empty denotes an unnamed voxelized volume.
   */
  [[nodiscard]] auto GetName() const noexcept -> std::string const & {
    return name_;
  }

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
   * \brief Reports the imported grid and pose once through GGEMS output.
   *
   * Reads immutable mappings and the local Material palette without scanning
   * voxel labels or changing output settings.
   * \return This volume for chained configuration.
   */
  auto Verbose() -> GGEMSVoxelizedVolume &;

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
   * \brief Returns the compatibility whole-volume metadata color.
   * \return Existing WHITE_White key; cell rendering uses GetDisplayStyle().
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

  /*!
   * \brief Resolves immutable presentation independently of the Material
   * palette.
   * \param[in] original_label Anatomical label admitted by this import.
   * \pre The label has a mapping, as guaranteed for every GetLabels() value.
   * \return Resolved mapping style owned by the shared imported data.
   */
  [[nodiscard]] auto
  GetDisplayStyle(std::uint16_t original_label) const noexcept
    -> GGEMSVoxelDisplayStyle const &;

  /*!
   * \brief Borrows independently addressable original mapping presentations.
   * \return Immutable rows sorted by exclusive label endpoint, owned by this
   * volume's shared imported data.
   */
  [[nodiscard]] auto GetDisplayMappings() const noexcept
    -> std::span<GGEMSVoxelDisplayMapping const>;

  /*!
   * \brief Finds the presentation row for one admitted anatomical label.
   * \param[in] original_label Anatomical label admitted by this import.
   * \pre The label has a mapping, as guaranteed for every GetLabels() value.
   * \return Index in GetDisplayMappings(), independent of compact Material IDs.
   */
  [[nodiscard]] auto
  GetDisplayMappingIndex(std::uint16_t original_label) const noexcept
    -> std::uint32_t;

private:
  /*!
   * \brief Optional display label copied independently of the immutable data.
   */
  std::string name_;

  /*! \brief Immutable imported grid and local material palette. */
  struct Data;

  /*! \brief Shared payload; construction completes before publication. */
  std::shared_ptr<Data const> data_;
};

} // namespace ggems::geometry
