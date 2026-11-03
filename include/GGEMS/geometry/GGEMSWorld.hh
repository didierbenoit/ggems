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

#include <array>
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
#include "GGEMS/geometry/GGEMSRotation3D.hh"
#include "GGEMS/geometry/GGEMSWorldRecord.hh"
#include "GGEMS/geometry/GGEMSVolumeAuthoring.hh"
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
 * only; direct children are bounded by containment in the World.
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
 * maximum physical depth is three below World. Detached solid values may own
 * immutable child/repeater descriptions; attaching such a subtree requires a
 * configured World and atomically admits its preorder physical expansion.
 * Ordinary leaf AddVolume retains its deferred Validate() behavior.
 * Descendants must fit in their
 * immediate parent's physical support and replace its medium there.
 * Sibling interiors must be disjoint; surface contact is permitted.
 * There is no automatic overlap correction. BuildAnalyticDeviceRecords()
 * compiles the occurrences into tagged device records with shared variable
 * profile data. The World packs the device-visible GGEMSWorldRecord and offers
 * Contains() for static validation of authored positions only. Per-particle
 * location, intersection and World exit are computed by OpenCL
 * Navigation/Transport.
 */
class GGEMSWorld {
public:
  /*! \brief Read-only shape alternatives captured for one occurrence. */
  using Volume = std::variant<GGEMSBox, GGEMSSphere, GGEMSTubs, GGEMSEllipsoid,
                              GGEMSTrd, GGEMSTrap, GGEMSCons, GGEMSPolyhedra>;

  /*! \brief Composed host pose before conversion to device FP32 parameters. */
  struct Pose {
    /*! \brief Local origin in World coordinates, in picometers. */
    std::array<double, 3> position_pm;

    /*! \brief Local-to-World orthonormal basis. */
    GGEMSRotation3D rotation;
  };

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
   * that nominal features accommodate the bands; sub-band gaps may act as
   * contact.
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
   * checked by Validate() for a leaf, so World size may be set afterwards.
   * A captured subtree requires configured World size and Material and passes
   * complete scene admission atomically. Later source edits do not reach it.
   *
   * \param[in] box Complete Box authoring value copied into the World.
   * \param[in] parent_volume_id Existing parent VolumeID, or zero for World.
   * The copied pose is relative to that parent. The root VolumeID is the
   * previous GetVolumeCount() + 1; descendants follow in preorder. A subtree
   * requires a configured World and complete atomic scene admission.
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
   * The copied pose is relative to that parent. The root VolumeID is the
   * previous GetVolumeCount() + 1; descendants follow in preorder. A subtree
   * requires a configured World and complete atomic scene admission.
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
   * The copied pose is relative to that parent. The root VolumeID is the
   * previous GetVolumeCount() + 1; descendants follow in preorder. A subtree
   * requires a configured World and complete atomic scene admission.
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
   * The copied pose is relative to that parent. The root VolumeID is the
   * previous GetVolumeCount() + 1; descendants follow in preorder. A subtree
   * requires a configured World and complete atomic scene admission.
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
   * The copied pose is relative to that parent. The root VolumeID is the
   * previous GetVolumeCount() + 1; descendants follow in preorder. A subtree
   * requires a configured World and complete atomic scene admission.
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
   * The copied pose is relative to that parent. The root VolumeID is the
   * previous GetVolumeCount() + 1; descendants follow in preorder. A subtree
   * requires a configured World and complete atomic scene admission.
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
   * The copied pose is relative to that parent. The root VolumeID is the
   * previous GetVolumeCount() + 1; descendants follow in preorder. A subtree
   * requires a configured World and complete atomic scene admission.
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
   * The copied pose is relative to that parent. The root VolumeID is the
   * previous GetVolumeCount() + 1; descendants follow in preorder. A subtree
   * requires a configured World and complete atomic scene admission.
   * \return This World for chained configuration.
   * \throws GGEMSRecoverable If profile admission fails.
   * Also rejects an absent parent, physical depth above three, or exhausted
   * IDs.
   */
  auto AddVolume(GGEMSPolyhedra polyhedra, std::uint32_t parent_volume_id = 0U)
    -> GGEMSWorld &;

  /*!
   * \brief Atomically attaches a complete independently captured subtree.
   *
   * The World size and Material must already be configured. The complete
   * staged scene passes existing CPU admission before any occurrence changes.
   * Children were captured when authored; this call captures the root's final
   * pose. Later edits of any source value cannot change this World.
   * \tparam Child One of the eight analytical authoring types.
   * \param[in] volume Root value and its immutable child descriptions.
   * \param[in] parent_volume_id Existing physical parent, or zero for World.
   * \return This World for chained configuration.
   * \throws core::GGEMSRecoverable If configuration, depth, capacity, or
   * complete scene admission fails. The original World is retained.
   */
  template <class Child>
  auto AttachVolume(Child const &volume, std::uint32_t parent_volume_id = 0U)
    -> GGEMSWorld & {
    return AttachBatch(Volume{volume}, 1U, units::Length{0}, GGEMSRepeatAxis::X,
                       false, parent_volume_id);
  }

  /*!
   * \brief Atomically expands a mother-local linear subtree repeater.
   * \tparam Child One of the eight analytical authoring types.
   * \param[in] volume Captured root; its position is the arrangement center
   * and its orientation is retained by every copy.
   * \param[in] count Total positive number of copies.
   * \param[in] pitch Center-to-center length, positive when count exceeds one.
   * \param[in] axis Positive X, Y, or Z in the selected parent's local frame.
   * \param[in] parent_volume_id Existing physical parent, or zero for World.
   * \return This World for chained configuration.
   * \throws core::GGEMSRecoverable If inputs, placements, capacity, or staged
   * geometry admission fail. A configured World is required; no partial batch
   * is retained. Runtime Navigation uses ordinary physical occurrences.
   * \note Copy i is centered at p + (i - (count - 1)/2) * pitch along axis.
   * Fractional-picometer offsets use ordinary signed integer division.
   */
  template <class Child>
  auto LinearRepeater(Child const &volume, std::uint32_t count,
                      units::Length pitch, GGEMSRepeatAxis axis,
                      std::uint32_t parent_volume_id = 0U) -> GGEMSWorld & {
    return AttachBatch(Volume{volume}, count, pitch, axis, false,
                       parent_volume_id);
  }

  /*!
   * \brief Atomically attaches a centered Cartesian arrangement of subtrees.
   *
   * All roots are direct siblings under the selected parent, without added
   * Row or Plane volumes. X varies fastest, then Y, then Z; each root's
   * descendants follow that root before the next copy. Each axis applies the
   * same centering arithmetic as the scalar overload.
   * \tparam Child One of the eight analytical authoring types.
   * \param[in] volume Reference center, orientation and captured descendants.
   * \param[in] count Positive X/Y/Z copy counts; their product counts roots.
   * \param[in] pitch X/Y/Z center spacings in canonical pm. A singleton axis
   * ignores its spacing; every other axis requires positive spacing.
   * \param[in] parent_volume_id Existing physical parent, or zero for World.
   * \return This World for chained configuration.
   * \throws core::GGEMSRecoverable If inputs, depth, capacity or
   * complete scene admission fail; no partial batch is retained.
   */
  template <class Child>
  auto LinearRepeater(Child const &volume, std::array<std::uint32_t, 3> count,
                      std::array<units::Length, 3> pitch,
                      std::uint32_t parent_volume_id = 0U) -> GGEMSWorld & {
    return AttachBatch(Volume{volume}, count, pitch, parent_volume_id);
  }

  /*!
   * \brief Atomically expands a full-turn mother-local circular repeater.
   * \tparam Child One of the eight analytical authoring types.
   * \param[in] volume Reference pose and independently captured descendants.
   * \param[in] count Total positive number of copies, without a final
   * duplicate.
   * \param[in] axis Positive axis through the parent's local origin.
   * \param[in] parent_volume_id Existing physical parent, or zero for World.
   * \return This World for chained configuration.
   * \throws core::GGEMSRecoverable If inputs, zero-radius multi-copy placement,
   * depth, capacity, or staged admission fail. A configured World is required.
   * \note Position and orientation both rotate by 2*pi*i/count. Expansion is
   * host preparation; device navigation and its linear child scans are
   * unchanged.
   */
  template <class Child>
  auto CircularRepeater(Child const &volume, std::uint32_t count,
                        GGEMSRepeatAxis axis,
                        std::uint32_t parent_volume_id = 0U) -> GGEMSWorld & {
    return AttachBatch(Volume{volume}, count, units::Length{0}, axis, true,
                       parent_volume_id);
  }

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
   * \return Number of expanded physical occurrences, excluding World; zero
   * before any volume is added. Valid occurrence indices are smaller than this
   * count, and their physical VolumeIDs are index + 1.
   */
  [[nodiscard]] auto GetVolumeCount() const noexcept -> std::size_t;

  /*!
   * \brief Borrows the canonical authored shape and parent-local pose.
   * \param[in] index Physical occurrence index, smaller than GetVolumeCount().
   * \return Owned shape value; World mutation or destruction invalidates it.
   */
  [[nodiscard]] auto GetVolume(std::size_t index) const noexcept
    -> Volume const &;

  /*!
   * \brief Composes the host local-to-World pose without FP32 packing.
   * \param[in] index Physical occurrence index, smaller than GetVolumeCount().
   * \return Global translation and rotation, including Sphere daughter frames.
   */
  [[nodiscard]] auto BuildVolumePose(std::size_t index) const noexcept -> Pose;

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
   * Each daughter must fit within its mother's physical support, including
   * hollow walls, and sibling interiors must be disjoint. Coincident surfaces
   * and positive gaps are allowed. Exact simple checks and bounded surface
   * samples reject detected violations; sampling is not an exhaustive proof.
   * Users remain responsible for verifying the complete scene. All nominal
   * features must pass existing FP32 admission using the uploaded global pose.
   * Gaps within adjacent surface bands may behave as contact; admission does
   * not guarantee a distinct material interval for an unresolved gap.
   *
   * \throws GGEMSRecoverable If World size or Material is absent, detected
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
   * \brief Plans and admits an entire finite subtree batch before committing
   * it.
   * \param[in] volume Captured local root with immutable child descriptions.
   * \param[in] count Number of repeated roots.
   * \param[in] pitch Linear center spacing in canonical pm; ignored for
   * circular.
   * \param[in] axis Positive parent-local repeat axis.
   * \param[in] circular Selects rigid full-turn rotation instead of
   * translation.
   * \param[in] parent_volume_id Existing physical parent identity.
   * \return This World after successful atomic attachment.
   * \throws core::GGEMSRecoverable If preparation or complete admission fails.
   */
  auto AttachBatch(Volume volume, std::uint32_t count, units::Length pitch,
                   GGEMSRepeatAxis axis, bool circular,
                   std::uint32_t parent_volume_id) -> GGEMSWorld &;

  /*!
   * \brief Captures a Cartesian recipe for atomic World attachment.
   * \param[in] volume Local reference center, orientation and descendants.
   * \param[in] count Positive X/Y/Z root counts.
   * \param[in] pitch Canonical X/Y/Z center spacings.
   * \param[in] parent_volume_id Existing physical parent identity.
   * \return This World after successful atomic attachment.
   * \throws core::GGEMSRecoverable If recipe or complete admission fails.
   */
  auto AttachBatch(Volume volume, std::array<std::uint32_t, 3> count,
                   std::array<units::Length, 3> pitch,
                   std::uint32_t parent_volume_id) -> GGEMSWorld &;

  /*!
   * \brief Plans, expands and admits one complete recipe before publication.
   * \param[in] recipe Immutable captured placement and subtree description.
   * \param[in] parent_volume_id Existing physical parent identity.
   * \return This World after successful atomic attachment.
   * \throws core::GGEMSRecoverable If World, recipe, depth, capacity or
   * complete geometry admission fails.
   */
  auto AttachRecipe(detail::GGEMSVolumeRecipe const &recipe,
                    std::uint32_t parent_volume_id) -> GGEMSWorld &;

  /*!
   * \brief Checks expanded device byte and crossing bounds before allocation.
   * \param[in] recipe Admitted finite recipe appended to existing occurrences.
   * \param[in] total Checked combined physical occurrence count.
   * \throws core::GGEMSRecoverable If device offsets or crossing bounds exceed
   * their existing 32-bit domains.
   */
  auto ValidateBatchCapacity(detail::GGEMSVolumeRecipe const &recipe,
                             std::size_t total) const -> void;

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
   * \brief Applies feature-band and exact simple World containment checks.
   * \param[in] index Zero-based index of the owned occurrence to validate.
   * \param[in] pose Already composed global occurrence pose.
   * \param[in] world_spacing Upward FP32 spacing at the World extent in pm.
   * \return The admitted automatic surface half-band in pm.
   * \pre World size is configured and index is smaller than GetVolumeCount().
   * \throws GGEMSRecoverable If containment or automatic band admission fails.
   */
  [[nodiscard]] auto ValidateVolume(std::size_t index,
                                    GGEMSBoxPoseRecord const &pose,
                                    float world_spacing) const -> float;

  /*!
   * \brief Rejects exact nominal overlaps for simple direct World sibling
   * pairs.
   * \param[in] first Zero-based index of the first physical occurrence.
   * \param[in] second Zero-based index of the second physical occurrence.
   * \pre Both indices are distinct and each occurrence passed ValidateVolume().
   * \throws GGEMSRecoverable If identity-axis Boxes or coaxial Spheres overlap.
   */
  auto ValidateSimplePair(std::size_t first, std::size_t second) const -> void;

  /*!
   * \brief Rejects detected containment and sibling overlap violations.
   * \throws GGEMSRecoverable If exact checks or bounded samples find a defect.
   * \note This initialization diagnostic cannot certify arbitrary geometry.
   */
  auto ValidateHierarchy() const -> void;

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
   * Validate() rejects a bound that cannot fit the uint32 device field before
   * any scene upload.
   */
  [[nodiscard]] auto CountBoundaryEvents() const noexcept -> std::uint64_t;

  /*!
   * \brief Packs one occurrence using its already prepared pose and half-band.
   * \param[in] material_id Dense snapshot-local Material identity.
   * \param[in] index Existing zero-based physical occurrence index.
   * \param[in] pose Composed global occurrence pose.
   * \param[in] tolerance Automatic surface half-band in pm.
   * \return Tagged scalar shape record with the unchanged device layout.
   */
  [[nodiscard]] auto BuildAnalyticVolumeRecord(std::uint32_t material_id,
                                               std::size_t index,
                                               GGEMSBoxPoseRecord const &pose,
                                               float tolerance) const
    -> GGEMSAnalyticVolumeRecord;
};

} // namespace ggems::geometry
