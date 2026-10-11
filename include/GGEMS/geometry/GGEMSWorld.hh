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
 * \brief Declares the finite World authoring root that owns the scene volumes.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <vector>

#include "GGEMS/geometry/GGEMSBox.hh"
#include "GGEMS/geometry/GGEMSBoxRecord.hh"
#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSWorldRecord.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"

namespace ggems::geometry {

/*!
 * \brief Largest admitted World full size along one axis, in picometers.
 *
 * The value is the largest even integer below the signed 64-bit maximum. It
 * keeps every World half extent at or below 2^62 - 1 pm, so every World point
 * fits CoordinatePM and the displacement between any two World points fits
 * CoordinatePM without further overflow checks. The cap applies to the World
 * only; volumes are bounded by strict containment in the World.
 */
inline constexpr DistancePM k_max_world_size_pm{0x7FFF'FFFF'FFFF'FFFEULL};

/*!
 * \brief Finite explicit World: an axis-aligned box centered on the global
 * origin that owns the authored physical volumes of the scene.
 *
 * The World frame is the canonical global frame of Position3PM and Direction3:
 * right-handed with X x Y = Z and default direction +Z. Faces lie at exactly
 * +/- half extent on the integer picometer lattice, for device Navigation. The
 * World medium is an explicit owned Material value; exact Vacuum is admissible.
 *
 * The object is a host authoring value. A default-constructed World has no
 * size, no Material and no volume; SetSize(), SetMaterial() and AddVolume()
 * may be called in any order. Validate() checks the completed scene and is
 * called by GGEMSRun::SetWorld, which copies the World: later edits of the
 * authoring object do not reach the Run. BuildBoxRecords() then compiles the
 * validated volumes into device records. The World also packs the
 * device-visible GGEMSWorldRecord and offers Contains() for static validation
 * of authored positions only. Per-particle location, intersection and World
 * exit are computed by OpenCL Navigation/Transport.
 */
class GGEMSWorld {
public:
  /*! \brief Creates a World with no size, no Material and no volume. */
  GGEMSWorld() = default;

  /*!
   * \brief Sets the full sizes along the global axes.
   *
   * \param[in] size_x Full extent along X.
   * \param[in] size_y Full extent along Y.
   * \param[in] size_z Full extent along Z.
   * \return This World for chained configuration.
   * \throws GGEMSRecoverable If a size is zero, is an odd number of
   * picometers, or exceeds k_max_world_size_pm; the previous size is kept.
   */
  auto SetSize(units::Length size_x, units::Length size_y, units::Length size_z)
    -> GGEMSWorld &;

  /*!
   * \brief Sets the owned World medium.
   *
   * \param[in] material Owned World medium; no default medium exists.
   * \return This World for chained configuration.
   */
  auto SetMaterial(core::materials::GGEMSMaterial material) -> GGEMSWorld &;

  /*!
   * \brief Adds one complete physical volume owned by this World.
   *
   * The current slice admits at most one Box; a World without volumes remains
   * valid. Strict containment is checked by Validate(), so the World size may
   * still be set afterwards. Later edits of the caller's Box do not reach the
   * owned copy.
   *
   * \param[in] box Complete Box authoring value copied into the World.
   * \return This World for chained configuration.
   * \throws GGEMSRecoverable If a volume is already registered or the Box is
   * incomplete.
   */
  auto AddVolume(GGEMSBox box) -> GGEMSWorld &;

  /*!
   * \brief Checks that the authored scene is complete and consistent.
   *
   * Every volume must lie strictly inside the World: no Box face may coincide
   * with or cross a World face, so one picometer of clearance is required.
   *
   * \throws GGEMSRecoverable If the World size or Material is not set, or if a
   * volume is not strictly inside the World.
   */
  auto Validate() const -> void;

  /*!
   * \brief Packs the owned volumes into device records.
   *
   * Admission belongs to Validate(); this method only packs. Volume
   * identities are 1 upward in registration order after the World (identity
   * 0). Faces are center -/+ half extent; after Validate() every face lies
   * strictly between the World faces and fits CoordinatePM.
   *
   * \param[in] material_ids Dense snapshot-local Material identity of each
   * volume, supplied by the scene compiler in GetBoxes() order.
   * \return One record per volume; empty for a World without volumes.
   * \pre Validate() has succeeded and the World has not been modified since.
   * \pre material_ids has exactly one entry per owned volume.
   */
  [[nodiscard]] auto
  BuildBoxRecords(std::span<std::uint32_t const> material_ids) const
    -> std::vector<GGEMSBoxRecord>;

  /*!
   * \brief Returns the distances from the origin to the World faces.
   *
   * \return Half extents in picometers along X, Y, and Z; zero before
   * SetSize().
   */
  [[nodiscard]] auto GetHalfExtentPM() const noexcept -> HalfExtent3PM;

  /*!
   * \brief Returns the World corner with the smallest coordinates.
   *
   * \return Position equal to the negated half extents; the origin before
   * SetSize().
   */
  [[nodiscard]] auto GetLowerCornerPM() const noexcept -> Position3PM;

  /*!
   * \brief Returns the World corner with the largest coordinates.
   *
   * \return Position equal to the half extents; the origin before SetSize().
   */
  [[nodiscard]] auto GetUpperCornerPM() const noexcept -> Position3PM;

  /*!
   * \brief Packs immutable geometry parameters for the device.
   *
   * \return Three signed half extents in canonical pm; all zero before
   * SetSize(), which the device treats as an unconfigured World.
   */
  [[nodiscard]] auto BuildRecord() const noexcept -> GGEMSWorldRecord;

  /*!
   * \brief Checks closed-box membership of a configured position.
   *
   * This is static host input validation for authored positions such as
   * source origins; it is not a per-particle Navigation query, which OpenCL
   * performs on the device.
   *
   * \param[in] position Global position in picometers.
   * \return True when every coordinate magnitude is at most its half extent;
   * only the origin before SetSize().
   */
  [[nodiscard]] auto Contains(Position3PM position) const noexcept -> bool;

  /*!
   * \brief Returns the World medium.
   *
   * \return A borrowed reference to the owned Material; it must not outlive
   * this World.
   * \throws GGEMSRecoverable If no Material has been set.
   */
  [[nodiscard]] auto GetMaterial() const
    -> core::materials::GGEMSMaterial const &;

  /*!
   * \brief Returns the owned physical volumes in registration order.
   *
   * \return A borrowed view; AddVolume() invalidates it.
   */
  [[nodiscard]] auto GetBoxes() const noexcept -> std::span<GGEMSBox const>;

private:
  /*! \brief Validated half extents in picometers; zero until SetSize(). */
  HalfExtent3PM half_extent_pm_{};

  /*! \brief Owned World medium, absent until SetMaterial(). */
  std::optional<core::materials::GGEMSMaterial> material_;

  /*! \brief Owned physical volumes in registration order. */
  std::vector<GGEMSBox> boxes_;
};

} // namespace ggems::geometry
