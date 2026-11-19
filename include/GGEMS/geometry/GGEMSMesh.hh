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
 * \brief Declares detached indexed mesh imports and their independent authoring
 * state.
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
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "GGEMS/geometry/GGEMSGeometryTypes.hh"
#include "GGEMS/geometry/GGEMSRigidPose.hh"
#include "GGEMS/geometry/GGEMSRotation3D.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/render/GGEMSColorTypes.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"

namespace ggems::geometry {

/*!
 * brief Identifies the source representation admitted by the mesh importer.
 */
enum class GGEMSMeshFormat : std::uint8_t {
  /*! \brief Triangulated Wavefront OBJ. */
  Obj,

  /*! \brief Little-endian binary STL. */
  BinarySTL,

  /*! \brief Text STL. */
  AsciiSTL,
};

/*! \brief Bounds referenced triangle geometry in the unchanged local frame. */
struct GGEMSMeshBounds {
  /*! \brief Inclusive local minimum in FP32 picometers. */
  std::array<float, 3> minimum{};

  /*! \brief Inclusive local maximum in FP32 picometers. */
  std::array<float, 3> maximum{};
};

/*! \brief Preserves one named OBJ object declaration and its triangle range. */
struct GGEMSMeshComponent {
  /*! \brief Literal trimmed object name; repeated names remain separate. */
  std::string name;

  /*! \brief First triangle in the global triangle array. */
  std::size_t first_triangle{};

  /*! \brief Number of consecutive triangles belonging to this declaration. */
  std::size_t triangle_count{};

  /*! \brief Referenced local bounds, absent for an object without triangles. */
  std::optional<GGEMSMeshBounds> bounds_pm;
};

/*!
 * \brief Owns immutable imported geometry shared by copied mesh authoring
 * values.
 *
 * All OBJ position records are retained in source order, including unused
 * positions; each v record must contain exactly three Cartesian coordinates.
 * STL positions are indexed by exact source-coordinate equality in
 * first-encounter order, before FP32 conversion; there is no approximate weld.
 * Bounds cover referenced triangles only. This is host import data, not a
 * frozen device ABI or evidence of closed, oriented manifold admission.
 */
struct GGEMSMeshData {
  /*! \brief Local source positions converted once to FP32 picometers. */
  std::vector<std::array<float, 3>> vertices_pm;

  /*! \brief Source-order triangles indexing the global position array. */
  std::vector<std::array<std::uint32_t, 3>> triangles;

  /*! \brief Named OBJ objects in declaration order; empty for STL. */
  std::vector<GGEMSMeshComponent> components;

  /*! \brief Bounds of all referenced local triangles. */
  GGEMSMeshBounds bounds_pm;

  /*! \brief Owned path supplied by the caller. */
  std::filesystem::path source_file;

  /*! \brief Explicit registered length unit supplied by the caller. */
  std::string source_unit;

  /*! \brief Detected source representation. */
  GGEMSMeshFormat format{GGEMSMeshFormat::Obj};

  /*! \brief OBJ position records, or STL facet corners before deduplication. */
  std::size_t source_vertex_count{};
};

/*!
 * \brief Imports one detached OBJ or STL mesh with independent pose and style.
 *
 * Copies share immutable geometry and own their name, Material, color override,
 * and rigid pose. World admits physical meshes with closed, oriented shells.
 */
class GGEMSMesh : public GGEMSRigidPose {
public:
  /*!
   * \brief Imports geometry with no authored display name.
   * \param[in] mesh_file OBJ or STL path, selected by its extension.
   * \param[in] unit Registered length unit of source coordinates, default mm.
   * \throws GGEMSRecoverable If input cannot be read or admitted structurally.
   */
  GGEMSMesh(std::filesystem::path const &mesh_file,
            std::string_view unit = "mm");

  /*!
   * \brief Imports geometry with an independently owned display name.
   * \param[in] name Display name, unrelated to the filename or OBJ objects.
   * \param[in] mesh_file OBJ or STL path, selected by its extension.
   * \param[in] unit Explicit registered length unit of source coordinates.
   * \throws GGEMSRecoverable If input cannot be read or admitted structurally.
   */
  GGEMSMesh(std::string name, std::filesystem::path const &mesh_file,
            std::string_view unit);

  /*! \brief Copies the current pose and component offsets independently.
   * \param[in] other Source mesh.
   */
  GGEMSMesh(GGEMSMesh const &other);

  /*! \brief Assigns an independent copy of the source mesh.
   * \param[in] other Source mesh.
   * \return This mesh.
   */
  auto operator=(GGEMSMesh const &other) -> GGEMSMesh &;

  /*! \brief Transfers the source mesh and its live attachment.
   * \param[in] other Source mesh.
   */
  GGEMSMesh(GGEMSMesh &&other) noexcept = default;

  /*! \brief Replaces this mesh with the moved source.
   * \param[in] other Source mesh.
   * \return This mesh.
   */
  auto operator=(GGEMSMesh &&other) noexcept -> GGEMSMesh & = default;

  /*! \brief Captures this mesh for one live attachment.
   * \return Mesh sharing only its authoring pose, offsets, and immutable
   * import.
   */
  [[nodiscard]] auto CaptureForAttachment() -> GGEMSMesh;

  /*! \brief Identifies this occurrence's packed asset.
   * \return Local identity shared only by reusable immutable payloads.
   */
  [[nodiscard]] auto GetAssetIdentity() const noexcept -> void const * {
    if (translations_) {
      return this;
    }
    return data_.get();
  }

  /*!
   * \brief Reads the optional authored display name.
   * \return Owned name, empty when not authored; unrelated to source objects.
   */
  [[nodiscard]] auto GetName() const noexcept -> std::string const & {
    return name_;
  }

  /*!
   * \brief Reads the shared immutable imported geometry.
   * \return Payload reference, valid until this mesh is destroyed or
   * reassigned.
   */
  [[nodiscard]] auto GetData() const noexcept -> GGEMSMeshData const & {
    return *data_;
  }

  /*!
   * \brief Reads the authored OBJ component labels.
   * \return Independent names in declaration order, retaining repeated labels.
   */
  [[nodiscard]] auto GetComponentNames() const -> std::vector<std::string>;

  /*!
   * \brief Replaces one uniquely named OBJ component's local offset.
   * \param[in] name Exact OBJ object name.
   * \param[in] translation Absolute mesh-local offset in pm.
   * \return This mesh.
   * \throws core::GGEMSRecoverable For an unknown or ambiguous name, or a
   * moved component sharing vertices with other faces.
   */
  auto SetComponentTranslation(std::string_view name, Position3PM translation)
    -> GGEMSMesh &;

  /*! \brief Builds local vertices for component motion.
   * \return FP32 coordinates in pm, empty for the imported pose.
   */
  [[nodiscard]] auto BuildTranslatedVertices() const
    -> std::vector<std::array<float, 3>>;

  /*!
   * \brief Assigns an owned homogeneous Material without changing geometry.
   * \param[in] material Configured GGEMS Material.
   * \return This mesh for fluent authoring.
   */
  auto SetMaterial(core::materials::GGEMSMaterial material) -> GGEMSMesh &;

  /*!
   * \brief Borrows the independently owned Material.
   * \return Material reference, valid until reassignment or destruction.
   * \throws GGEMSRecoverable If no Material has been assigned.
   */
  [[nodiscard]] auto GetMaterial() const
    -> core::materials::GGEMSMaterial const &;

  /*!
   * \brief Assigns a presentation override, including an explicit white color.
   * \param[in] color Existing GGEMS named palette key.
   * \return This mesh for fluent authoring.
   */
  auto SetColor(render::ColorKey color) noexcept -> GGEMSMesh &;

  /*!
   * \brief Resolves the effective presentation color.
   * \return Explicit override, Material color, or white when unconfigured.
   */
  [[nodiscard]] auto GetColor() const noexcept -> render::ColorKey;

  /*!
   * \brief Reads the independently authored color override.
   * \return Explicit color, including white, or no value for inheritance.
   */
  [[nodiscard]] auto GetColorOverride() const noexcept
    -> std::optional<render::ColorKey> {
    return color_;
  }

  /*!
   * \brief Replaces the detached mesh translation in canonical picometers.
   * \param[in] position Translation in the eventual parent frame.
   * \return This mesh for fluent authoring.
   */
  auto SetPosition(Position3PM position) noexcept -> GGEMSMesh &;

  /*!
   * \brief Replaces the absolute right-handed rotation Rz * Ry * Rx.
   * \param[in] angle_x Rotation about X.
   * \param[in] angle_y Rotation about Y.
   * \param[in] angle_z Rotation about Z.
   * \return This mesh for fluent authoring.
   */
  auto SetRotation(units::Angle angle_x, units::Angle angle_y,
                   units::Angle angle_z) -> GGEMSMesh &;

  /*!
   * \brief Reads the translation without modifying imported vertices.
   * \return Eventual parent-frame translation in canonical picometers.
   */
  [[nodiscard]] auto GetPositionPM() const noexcept -> Position3PM {
    return LocalPose().position_pm;
  }

  /*!
   * \brief Reads the absolute rigid orientation.
   * \return Basis reference, valid until reassignment or destruction.
   */
  [[nodiscard]] auto GetRotation() const noexcept -> GGEMSRotation3D const & {
    return LocalPose().rotation;
  }

  /*!
   * \brief Emits one import and authoring summary through configured logging.
   * \return This mesh for fluent authoring.
   */
  auto Verbose() -> GGEMSMesh &;

private:
  /*! \brief Absolute local offsets shared only by a live attachment. */
  std::shared_ptr<std::vector<Position3PM>> translations_;

  /*! \brief Independently owned display label. */
  std::string name_;

  /*! \brief Immutable import payload shared across ordinary copies. */
  std::shared_ptr<GGEMSMeshData const> data_;

  /*! \brief Optional independent homogeneous Material. */
  std::optional<core::materials::GGEMSMaterial> material_;

  /*! \brief Optional explicit presentation override. */
  std::optional<render::ColorKey> color_;
};

} // namespace ggems::geometry
