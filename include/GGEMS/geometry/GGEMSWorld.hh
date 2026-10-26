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
#include <variant>

#include "GGEMS/geometry/GGEMSSphere.hh"
#include "GGEMS/geometry/GGEMSTubs.hh"
#include "GGEMS/geometry/GGEMSEllipsoid.hh"
#include "GGEMS/geometry/GGEMSAnalyticVolumeRecord.hh"
#include "GGEMS/geometry/GGEMSBox.hh"
#include "GGEMS/geometry/GGEMSBoxRecord.hh"
#include "GGEMS/geometry/GGEMSBoxPoseRecord.hh"
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
 * +/- half extent on the host integer picometer lattice. Device records convert
 * these spatial values once to FP32 pm for Navigation. The World medium is an
 * explicit owned Material value; exact Vacuum is admissible.
 *
 * The object is a host authoring value. A default-constructed World has no
 * size, no Material and no volume; SetSize(), SetMaterial() and AddVolume()
 * may be called in any order. Validate() checks the completed scene and is
 * called by GGEMSRun::SetWorld, which copies the World: later edits of the
 * authoring object do not reach the Run. BuildAnalyticVolumeRecord() compiles
 * the single volume into a tagged device record. The World also packs the
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
   * \brief Returns the effective World surface half-band in FP32 pm.
   * \return Uploaded half-band, or zero before the World size is set.
   */
  [[nodiscard]] auto GetNavigationTolerancePM() const noexcept -> float;

  /*!
   * \brief Returns the effective single-volume surface half-band in FP32 pm.
   * \return Uploaded analytic half-band, or zero for a World-only scene.
   */
  [[nodiscard]] auto GetVolumeNavigationTolerancePM() const noexcept -> float;

  /*!
   * \brief Adds one complete physical volume owned by this World.
   *
   * The current slice admits at most one Box, Sphere, Tubs or Ellipsoid; a
   * World without volumes remains valid. Strict containment is checked by
   * Validate(), so the World size may still be set afterwards. Later edits of
   * the caller's Box do not reach the owned copy.
   *
   * \param[in] box Complete Box authoring value copied into the World.
   * \return This World for chained configuration.
   * \throws GGEMSRecoverable If a volume is already registered or the Box is
   * incomplete.
   */
  auto AddVolume(GGEMSBox box) -> GGEMSWorld &;

  /*!
   * \brief Copies one complete Sphere into the single-volume World.
   * \param[in] sphere Complete Sphere, independent of the caller after copying.
   * \return This World for chained configuration.
   * \throws GGEMSRecoverable If any volume is already present or Sphere is
   * incomplete.
   */
  auto AddVolume(GGEMSSphere sphere) -> GGEMSWorld &;

  /*!
   * \brief Copies one complete Tubs into the single-volume World.
   * \param[in] tubs Complete tube, independent of the caller after copying.
   * \return This World for chained configuration.
   * \throws GGEMSRecoverable If a volume is present or the tube is incomplete.
   */
  auto AddVolume(GGEMSTubs tubs) -> GGEMSWorld &;

  /*!
   * \brief Copies one complete Ellipsoid into the single-volume World.
   * \param[in] ellipsoid Complete ellipsoid, copied into the World.
   * \return This World for chained configuration.
   * \throws GGEMSRecoverable If a volume is present or the ellipsoid is
   * incomplete.
   */
  auto AddVolume(GGEMSEllipsoid ellipsoid) -> GGEMSWorld &;

  /*!
   * \brief Packs the single volume, or a None tag for a World-only scene.
   * \param[in] material_id Dense Material identity from the scene compiler.
   * \return Tagged scalar device record with physical VolumeID 1 when present.
   * \pre Validate() has succeeded since the last edit.
   */
  [[nodiscard]] auto BuildAnalyticVolumeRecord(std::uint32_t material_id) const
    -> GGEMSAnalyticVolumeRecord;

  /*!
   * \brief Borrows the single volume's Material, independent of shape.
   * \return Null for World-only; otherwise a pointer valid until World
   * mutation.
   */
  [[nodiscard]] auto GetVolumeMaterial() const
    -> core::materials::GGEMSMaterial const *;

  /*!
   * \brief Checks that the authored scene is complete and consistent.
   *
   * Every volume must lie strictly inside the World: no surface may touch
   * or cross a World face. Sphere containment is exact in integer pm. Rotated
   * Box bounds include FP64 roundoff; identity Box bounds retain the exact
   * integer containment check. Tubs/Ellipsoid use their uploaded posed surface
   * bounds with separated FP32 feature and World bands.
   *
   * \throws GGEMSRecoverable If the World size or Material is not set, or if a
   * volume is not strictly inside the World, or the requested FP32 tolerance
   * cannot resolve the admitted feature and World-clearance bands.
   */
  auto Validate() const -> void;

  /*!
   * \brief Packs the owned Box into device records; empty for other shapes.
   *
   * Admission belongs to Validate(); this method only packs. Volume
   * identity is 1 after the World (identity 0). Every Box stores local half
   * extents in FP32 pm and uses the separate FP32 center and pose record.
   *
   * \param[in] material_ids Dense snapshot-local Material identity of each
   * Box, supplied by the scene compiler in GetBoxes() order.
   * \return One record for Box; empty for other shapes or World-only.
   * \pre Validate() has succeeded and the World has not been modified since.
   * \pre material_ids has exactly one entry per owned Box.
   */
  [[nodiscard]] auto
  BuildBoxRecords(std::span<std::uint32_t const> material_ids) const
    -> std::vector<GGEMSBoxRecord>;

  /*!
   * \brief Packs the shared center and FP32 pose for Box, Tubs or Ellipsoid.
   *
   * \return The posed shape's record, including identity; none for Sphere or
   * World-only. The established Box record and upload argument are reused.
   * \pre Validate() has succeeded since the last scene edit.
   */
  [[nodiscard]] auto BuildBoxPoseRecord() const
    -> std::optional<GGEMSBoxPoseRecord>;

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
   * \return FP32 half extents and surface half-band in pm; all zero before
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
   * \brief Returns the owned Box view; empty for other shapes or World-only.
   *
   * \return A borrowed view; AddVolume() invalidates it.
   */
  [[nodiscard]] auto GetBoxes() const noexcept -> std::span<GGEMSBox const>;

private:
  /*! \brief Validated half extents in picometers; zero until SetSize(). */
  HalfExtent3PM half_extent_pm_{};

  /*! \brief Owned World medium, absent until SetMaterial(). */
  std::optional<core::materials::GGEMSMaterial> material_;

  /*! \brief Owned single physical volume, absent until AddVolume(). */
  std::variant<std::monostate, GGEMSBox, GGEMSSphere, GGEMSTubs, GGEMSEllipsoid>
    volume_;

  /*!
   * \brief Explicit surface half-band; absent selects per-boundary defaults.
   */
  std::optional<units::Length> navigation_tolerance_;
};

} // namespace ggems::geometry
