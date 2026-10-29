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
#include <cstddef>
#include <optional>
#include <span>
#include <vector>
#include <variant>

#include "GGEMS/geometry/GGEMSSphere.hh"
#include "GGEMS/geometry/GGEMSTubs.hh"
#include "GGEMS/geometry/GGEMSCons.hh"
#include "GGEMS/geometry/GGEMSPolyhedra.hh"
#include "GGEMS/geometry/GGEMSEllipsoid.hh"
#include "GGEMS/geometry/GGEMSTrd.hh"
#include "GGEMS/geometry/GGEMSTrap.hh"
#include "GGEMS/geometry/GGEMSAnalyticVolumeRecord.hh"
#include "GGEMS/geometry/GGEMSAnalyticOccurrenceRecord.hh"
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
 * only; direct children are bounded by strict containment in the World.
 */
inline constexpr DistancePM k_max_world_size_pm{0x7FFF'FFFF'FFFF'FFFEULL};

/*!
 * \brief Owns a finite World and a bounded physical hierarchy of analytical
 * solids.
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
 * authoring object do not reach the Run. Physical identities follow insertion
 * order, starting at one after World identity zero. Each added shape supplies
 * a pose relative to its already added parent; World is parent zero. The
 * maximum physical depth is three below World. Descendants replace the parent
 * medium only inside every ancestor, so their nominal solids may protrude.
 * Sibling effective interiors must be disjoint; surface contact is permitted.
 * There is no automatic overlap correction. BuildAnalyticDeviceRecords()
 * compiles the occurrences into tagged device records with shared variable
 * profile data. The World packs the device-visible GGEMSWorldRecord and offers
 * Contains() for static validation of authored positions only. Per-particle
 * location, intersection and World exit are computed by OpenCL
 * Navigation/Transport.
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
   * \brief Returns the automatically derived World surface half-band.
   *
   * The value is eight upward FP32 spacings at the largest uploaded World
   * half extent. It is half the full closed surface-band width, with no fixed
   * physical-distance floor or public tolerance setter.
   * \return Effective half-band in pm, or zero before the World size is set.
   */
  [[nodiscard]] auto GetNavigationTolerancePM() const noexcept -> float;

  /*!
   * \brief Returns the automatically derived analytic surface half-band.
   *
   * The value is the larger of eight World spacings and sixteen spacings at
   * the solid's uploaded local/global surface envelope. Validate() checks
   * that each nominal feature and direct-child World gap accommodate the bands.
   * \param[in] index Zero-based physical occurrence index.
   * \return Effective half-band in FP32 pm, or zero when index is absent.
   */
  [[nodiscard]] auto
  GetVolumeNavigationTolerancePM(std::size_t index = 0U) const noexcept
    -> float;

  /*!
   * \brief Adds one complete physical volume owned by this World.
   *
   * Box, Sphere, Tubs, Ellipsoid, Trd, Trap, Cons and Polyhedra can be mixed;
   * a World without volumes remains valid. Direct-child World containment is
   * checked by Validate(), so the World size may still be set afterwards. Later
   * edits of the caller's Box do not reach the owned copy.
   *
   * \param[in] box Complete Box authoring value copied into the World.
   * \param[in] parent_volume_id Existing parent VolumeID, or zero for World.
   * The copied pose is relative to that parent; its newly assigned VolumeID
   * is GetVolumeCount() after this call.
   * \return This World for chained configuration.
   * \throws GGEMSRecoverable If the Box is incomplete.
   * Also rejects an absent parent, physical depth above three, or exhausted
   * IDs.
   */
  auto AddVolume(GGEMSBox box, std::uint32_t parent_volume_id = 0U)
    -> GGEMSWorld &;

  /*!
   * \brief Copies one complete Sphere into the World.
   * \param[in] sphere Complete Sphere, independent of the caller after copying.
   * \param[in] parent_volume_id Existing parent VolumeID, or zero for World.
   * The copied pose is relative to that parent; its newly assigned VolumeID
   * is GetVolumeCount() after this call.
   * \return This World for chained configuration.
   * \throws GGEMSRecoverable If Sphere is incomplete.
   * Also rejects an absent parent, physical depth above three, or exhausted
   * IDs.
   */
  auto AddVolume(GGEMSSphere sphere, std::uint32_t parent_volume_id = 0U)
    -> GGEMSWorld &;

  /*!
   * \brief Copies one complete Tubs into the World.
   * \param[in] tubs Complete tube, independent of the caller after copying.
   * \param[in] parent_volume_id Existing parent VolumeID, or zero for World.
   * The copied pose is relative to that parent; its newly assigned VolumeID
   * is GetVolumeCount() after this call.
   * \return This World for chained configuration.
   * \throws GGEMSRecoverable If the tube is incomplete.
   * Also rejects an absent parent, physical depth above three, or exhausted
   * IDs.
   */
  auto AddVolume(GGEMSTubs tubs, std::uint32_t parent_volume_id = 0U)
    -> GGEMSWorld &;

  /*!
   * \brief Copies one complete Ellipsoid into the World.
   * \param[in] ellipsoid Complete ellipsoid, copied into the World.
   * \param[in] parent_volume_id Existing parent VolumeID, or zero for World.
   * The copied pose is relative to that parent; its newly assigned VolumeID
   * is GetVolumeCount() after this call.
   * \return This World for chained configuration.
   * \throws GGEMSRecoverable If the ellipsoid is incomplete.
   * Also rejects an absent parent, physical depth above three, or exhausted
   * IDs.
   */
  auto AddVolume(GGEMSEllipsoid ellipsoid, std::uint32_t parent_volume_id = 0U)
    -> GGEMSWorld &;

  /*!
   * \brief Copies one complete Trd into the World.
   * \param[in] trd Complete solid, copied independently of the caller.
   * \param[in] parent_volume_id Existing parent VolumeID, or zero for World.
   * The copied pose is relative to that parent; its newly assigned VolumeID
   * is GetVolumeCount() after this call.
   * \return This World for chained configuration.
   * \throws GGEMSRecoverable If the solid is incomplete.
   * Also rejects an absent parent, physical depth above three, or exhausted
   * IDs.
   */
  auto AddVolume(GGEMSTrd trd, std::uint32_t parent_volume_id = 0U)
    -> GGEMSWorld &;

  /*!
   * \brief Copies one complete planar Trap into the World.
   * \param[in] trap Complete solid, copied independently of the caller.
   * \param[in] parent_volume_id Existing parent VolumeID, or zero for World.
   * The copied pose is relative to that parent; its newly assigned VolumeID
   * is GetVolumeCount() after this call.
   * \return This World for chained configuration.
   * \throws GGEMSRecoverable If Trap admission fails.
   * Also rejects an absent parent, physical depth above three, or exhausted
   * IDs.
   */
  auto AddVolume(GGEMSTrap trap, std::uint32_t parent_volume_id = 0U)
    -> GGEMSWorld &;

  /*!
   * \brief Copies one complete full-azimuth Cons into this World.
   * \param[in] cons Complete frustum, independent of the caller after copying.
   * \param[in] parent_volume_id Existing parent VolumeID, or zero for World.
   * The copied pose is relative to that parent; its newly assigned VolumeID
   * is GetVolumeCount() after this call.
   * \return This World for chained configuration.
   * \throws GGEMSRecoverable If Cons admission fails.
   * Also rejects an absent parent, physical depth above three, or exhausted
   * IDs.
   */
  auto AddVolume(GGEMSCons cons, std::uint32_t parent_volume_id = 0U)
    -> GGEMSWorld &;

  /*!
   * \brief Copies one complete continuous-profile Polyhedra into this World.
   * \param[in] polyhedra Complete polygonal profile, copied into the World.
   * \param[in] parent_volume_id Existing parent VolumeID, or zero for World.
   * The copied pose is relative to that parent; its newly assigned VolumeID
   * is GetVolumeCount() after this call.
   * \return This World for chained configuration.
   * \throws GGEMSRecoverable If profile admission fails.
   * Also rejects an absent parent, physical depth above three, or exhausted
   * IDs.
   */
  auto AddVolume(GGEMSPolyhedra polyhedra, std::uint32_t parent_volume_id = 0U)
    -> GGEMSWorld &;

  /*!
   * \brief Packs one occurrence pose and its immutable shape data.
   * \param[in] index Zero-based physical occurrence index.
   * \return Empty for an absent index or Sphere, pose bytes for simple solids,
   * or the pose followed by Polyhedra side normals and axial sections.
   * \pre Validate() has succeeded since the last scene edit.
   */
  [[nodiscard]] auto BuildAnalyticAuxiliaryData(std::size_t index = 0U) const
    -> std::vector<std::byte>;

  /*!
   * \brief Packs one occurrence, or a None tag when index is absent.
   * \param[in] material_id Dense Material identity from the scene compiler.
   * \param[in] index Zero-based physical occurrence index.
   * \return Tagged scalar record with physical VolumeID index + 1 when present.
   * \pre Validate() has succeeded since the last edit.
   */
  [[nodiscard]] auto BuildAnalyticVolumeRecord(std::uint32_t material_id,
                                               std::size_t index = 0U) const
    -> GGEMSAnalyticVolumeRecord;

  /*!
   * \brief Borrows one occurrence's Material, independent of shape.
   * \param[in] index Zero-based physical occurrence index.
   * \return Null when index is absent; otherwise a pointer valid until World
   * mutation.
   */
  [[nodiscard]] auto GetVolumeMaterial(std::size_t index = 0U) const
    -> core::materials::GGEMSMaterial const *;

  /*!
   * \brief Counts all owned physical occurrences at every hierarchy depth.
   * \return Number of AddVolume snapshots, excluding the World itself; zero
   * before any volume is added. Valid occurrence indices are smaller than this
   * count, and their physical VolumeIDs are index + 1.
   */
  [[nodiscard]] auto GetVolumeCount() const noexcept -> std::size_t;

  /*!
   * \brief Packs geometry references and parent/child/sibling physical links.
   * \return Insertion-ordered descriptors; empty for World-only.
   * \pre Validate() has succeeded since the last scene edit.
   */
  [[nodiscard]] auto BuildOccurrenceRecords() const
    -> std::vector<GGEMSAnalyticOccurrenceRecord>;

  /*!
   * \brief Packs all evaluated records with exact snapshot-local Materials.
   * \param[in] material_ids One dense Material identity per occurrence.
   * \return Insertion-ordered records, or one None record for World-only.
   * \pre Validate() has succeeded since the last scene edit.
   * \throws GGEMSRecoverable If the Material identity count is inconsistent.
   */
  [[nodiscard]] auto
  BuildAnalyticDeviceRecords(std::span<std::uint32_t const> material_ids) const
    -> std::vector<GGEMSAnalyticVolumeRecord>;

  /*!
   * \brief Packs occurrence poses and deduplicated immutable Polyhedra data.
   *
   * Multiple occurrences use a descriptor prefix, followed by independent
   * poses and shared side/section profiles. At most one occurrence retains
   * the established pose-then-profile format without a descriptor prefix.
   * \return Flat immutable bytes; empty for World-only or a single Sphere.
   * \pre Validate() has succeeded since the last scene edit.
   */
  [[nodiscard]] auto BuildAnalyticAuxiliaryBuffer() const
    -> std::vector<std::byte>;

  /*!
   * \brief Checks that the authored scene is complete and consistent.
   *
   * Direct World children must lie strictly inside the World with separated
   * bands. Sphere and identity-axis Box retain exact integer containment;
   * rotated Box bounds retain their existing FP64 roundoff admission. Nominal
   * descendant solids may protrude beyond ancestors or World; Navigation clips
   * their effective support to every ancestor. All nominal solid features must
   * pass the existing FP32 admission using the composed uploaded global pose.
   * Sibling effective interiors must be disjoint. Only obvious direct World
   * Box/Sphere overlaps are checked here; nested effective-support overlap is
   * an authoring contract, diagnosed on ambiguous device localization.
   * Clipped material intervals and intended gaps must remain resolvable outside
   * adjacent FP32 surface bands. Sub-band gaps may behave as contact. This
   * method does not exhaustively prove clipped-feature or sibling separation.
   *
   * \throws GGEMSRecoverable If World size or Material is absent, direct-child
   * containment or automatic feature/band admission fails, or a checked
   * overlap or device count/offset limit is encountered.
   */
  auto Validate() const -> void;

  /*!
   * \brief Packs the owned Box into device records; empty for other shapes.
   *
   * Admission belongs to Validate(); this method only packs. Volume
   * identity is index + 1 after the World (identity 0). Every Box stores local
   * half extents in FP32 pm and uses the separate FP32 center and pose record.
   *
   * \param[in] material_ids Dense snapshot-local Material identity of each
   * Box, supplied by the scene compiler in GetBoxes(index) order.
   * \param[in] index Zero-based physical occurrence index.
   * \return One record for Box; empty for other shapes or World-only.
   * \pre Validate() has succeeded and the World has not been modified since.
   * \throws GGEMSRecoverable If material_ids does not have one entry per
   * selected Box.
   */
  [[nodiscard]] auto
  BuildBoxRecords(std::span<std::uint32_t const> material_ids,
                  std::size_t index = 0U) const -> std::vector<GGEMSBoxRecord>;

  /*!
   * \brief Packs the composed global FP32 pose for every shape except Sphere.
   *
   * \param[in] index Zero-based physical occurrence index.
   * \return The composed local-to-global record, including identity; none for
   * Sphere or an absent index. The established Box record and upload argument
   * are reused.
   * \pre Validate() has succeeded since the last scene edit.
   */
  [[nodiscard]] auto BuildBoxPoseRecord(std::size_t index = 0U) const
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
   * \return FP32 half extents and surface half-band in pm; these geometric
   * fields are zero before SetSize(), denoting an unconfigured World.
   * Occurrence count, physical depth and the crossing bound accompany the box.
   * \pre Validate() has succeeded before a configured scene is uploaded.
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
   * \param[in] index Zero-based physical occurrence index.
   * \return A borrowed view, empty for an absent index; AddVolume() invalidates
   * it. Its position and rotation remain authored relative to its parent.
   */
  [[nodiscard]] auto GetBoxes(std::size_t index = 0U) const noexcept
    -> std::span<GGEMSBox const>;

private:
  /*! \brief Validated half extents in picometers; zero until SetSize(). */
  HalfExtent3PM half_extent_pm_{};

  /*! \brief Owned World medium, absent until SetMaterial(). */
  std::optional<core::materials::GGEMSMaterial> material_;

  /*! \brief Authoring alternatives for one physical occurrence. */
  using Volume = std::variant<GGEMSBox, GGEMSSphere, GGEMSTubs, GGEMSEllipsoid,
                              GGEMSTrd, GGEMSTrap, GGEMSCons, GGEMSPolyhedra>;

  /*! \brief One owned local authoring value and its existing physical parent.
   */
  struct Occurrence {
    /*! \brief Canonical local shape, pose and Material snapshot. */
    Volume volume;

    /*! \brief Existing physical parent VolumeID, or zero for World. */
    std::uint32_t parent_volume_id;
  };

  /*! \brief Owned occurrences in stable insertion/identity order. */
  std::vector<Occurrence> volumes_;

  /*!
   * \brief Validates and appends one occurrence without modifying existing
   * ones.
   * \param[in] volume Complete local authoring value transferred into World.
   * \param[in] parent_volume_id Existing parent identity, or zero for World.
   * \return This World for chained configuration.
   * \throws GGEMSRecoverable If shape admission, parent, depth or IDs are
   * invalid.
   */
  auto AddOccurrence(Volume volume, std::uint32_t parent_volume_id)
    -> GGEMSWorld &;

  /*!
   * \brief Counts the parent chain from an occurrence to World.
   * \param[in] volume_id Existing physical VolumeID, or zero for World.
   * \return Zero for World; one through three for an admitted occurrence.
   * \pre volume_id is no larger than GetVolumeCount().
   */
  [[nodiscard]] auto GetOccurrenceDepth(std::uint32_t volume_id) const noexcept
    -> std::uint32_t;

  /*!
   * \brief Composes canonical local poses and packs one global FP32 pose.
   * \param[in] index Existing zero-based occurrence index.
   * \return Global center and local-to-global axes, with Sphere's inherited
   * frame.
   * \pre index is smaller than GetVolumeCount().
   * \note The complete chain uses FP64 working arithmetic; only the final pose
   * is converted to FP32. Authored integer positions are never rewritten.
   */
  [[nodiscard]] auto BuildOccurrencePose(std::size_t index) const noexcept
    -> GGEMSBoxPoseRecord;

  /*!
   * \brief Applies existing solid feature and World-clearance admission.
   * \param[in] index Zero-based index of the owned occurrence to validate.
   * \param[in] world_spacing Upward FP32 spacing at the World extent in pm.
   * \param[in] world_tolerance Validated World surface half-band in pm.
   * \pre World size is configured and index is smaller than GetVolumeCount().
   * \throws GGEMSRecoverable If containment or automatic band admission fails.
   */
  auto ValidateVolume(std::size_t index, float world_spacing,
                      float world_tolerance) const -> void;

  /*!
   * \brief Rejects exact nominal overlaps for simple direct World sibling
   * pairs.
   * \param[in] first Zero-based index of the first physical occurrence.
   * \param[in] second Zero-based index of the second physical occurrence.
   * \pre Both indices are distinct and each occurrence passed ValidateVolume().
   * \throws GGEMSRecoverable If identity-axis Boxes or coaxial Spheres overlap.
   */
  auto ValidateSimplePair(std::size_t first, std::size_t second) const -> void;

  /*! \brief Shared plan for descriptor, pose and profile byte packing. */
  struct AuxiliaryLayout {
    /*! \brief One record and pose reference per occurrence. */
    std::vector<GGEMSAnalyticOccurrenceRecord> occurrences;

    /*! \brief Absolute profile byte offset, zero for non-Polyhedra shapes. */
    std::vector<std::uint32_t> profile_offsets;

    /*! \brief First occurrence providing each distinct profile. */
    std::vector<std::size_t> profile_sources;

    /*! \brief Total auxiliary allocation size in bytes. */
    std::size_t byte_count{0};
  };

  /*!
   * \brief Plans flat auxiliary offsets without copying profile payloads.
   * \return Checked layout with independently placed poses and shared profiles.
   * \throws GGEMSRecoverable If a byte offset exceeds the 32-bit device domain.
   */
  [[nodiscard]] auto BuildAuxiliaryLayout() const -> AuxiliaryLayout;

  /*!
   * \brief Counts a conservative straight-ray boundary-event bound.
   * \return One World exit plus each occurrence's maximum material crossings;
   * uint64 maximum if the sum overflows. Validate() rejects a bound that cannot
   * fit the uint32 device field before any scene upload.
   */
  [[nodiscard]] auto CountBoundaryEvents() const noexcept -> std::uint64_t;

  /*!
   * \brief Unset optional retained by World/Box/Sphere tolerance selection.
   *
   * No public setter populates this member; authored scenes use automatic
   * per-boundary half-bands.
   */
  std::optional<units::Length> navigation_tolerance_;
};

} // namespace ggems::geometry
