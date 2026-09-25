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
 * \brief Declares the process-wide custom-material catalog and used-material
 * registry.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <cstdint>
#include <optional>
#include <span>
#include <string_view>
#include <vector>
/// \endcond

#include "GGEMS/materials/GGEMSMaterial.hh"

namespace ggems::core::materials {

/*!
 * \brief Owns custom definitions separately from materials registered by use.
 *
 * GetOrAdd() registers by exact case-sensitive name, in first-use order. Adding
 * a custom definition alone does not register it. Registration indices are
 * positions in this manager, not the deduplicated IDs of an EM package.
 * Appending can reallocate the affected vector: reacquire spans, pointers,
 * references, and borrowed names after registration or custom-definition
 * growth. Concurrent mutation is not synchronized.
 */
class GGEMSMaterialManager {
public:
  /*!
   * \brief Returns the process-wide material manager.
   *
   * \return A reference to the function-local static manager.
   */
  [[nodiscard]] static auto GetInstance() noexcept -> GGEMSMaterialManager &;

  /*! \brief Disallows copying the process-wide registry. */
  GGEMSMaterialManager(GGEMSMaterialManager const &) = delete;

  /*! \brief Disallows moving the process-wide registry. */
  GGEMSMaterialManager(GGEMSMaterialManager &&) = delete;

  /*! \brief Disallows copy assignment of the process-wide registry. */
  auto operator=(GGEMSMaterialManager const &)
    -> GGEMSMaterialManager & = delete;

  /*! \brief Disallows move assignment of the process-wide registry. */
  auto operator=(GGEMSMaterialManager &&) -> GGEMSMaterialManager & = delete;

  /*!
   * \brief Finds a registered name or registers its available material
   * definition.
   *
   * Lookup order is registered materials, built-ins, then custom definitions. A
   * new registration appends an owned material; it does not deduplicate
   * different names with equal scientific payloads.
   *
   * \param[in] name Exact material name.
   * \return The existing or newly appended manager index.
   * \throws GGEMSRecoverable If the name is unknown or building its material
   * fails.
   */
  [[nodiscard]] auto GetOrAdd(std::string_view name) -> std::uint32_t;

  /*!
   * \brief Adds an owned custom definition without registering it for use.
   *
   * \param[in] material Material definition to move into the custom catalog.
   * \throws GGEMSRecoverable If its name matches a built-in or an existing
   * custom definition.
   */
  auto AddCustomMaterial(GGEMSMaterial material) -> void;

  /*!
   * \brief Returns registered materials in first-use order.
   *
   * \return A borrowed span; registration growth can invalidate it.
   */
  [[nodiscard]] auto GetMaterials() const noexcept
    -> std::span<GGEMSMaterial const>;

  /*!
   * \brief Returns custom definitions in insertion order.
   *
   * \return A borrowed span; adding custom definitions can invalidate it.
   */
  [[nodiscard]] auto GetCustomMaterials() const noexcept
    -> std::span<GGEMSMaterial const>;

  /*!
   * \brief Looks up an exact name among registered materials only.
   *
   * \param[in] name Exact, case-sensitive registered name.
   * \return Its manager index, or std::nullopt if it is not registered.
   */
  [[nodiscard]] auto FindIndex(std::string_view name) const noexcept
    -> std::optional<std::uint32_t>;

  /*!
   * \brief Finds a registered material by exact name.
   *
   * \param[in] name Exact, case-sensitive registered name.
   * \return A borrowed pointer, or nullptr; registration growth can invalidate
   * it.
   */
  [[nodiscard]] auto Find(std::string_view name) const noexcept
    -> GGEMSMaterial const *;

  /*!
   * \brief Finds a custom definition by exact name.
   *
   * \param[in] name Exact, case-sensitive custom name.
   * \return A borrowed pointer, or nullptr; custom-catalog growth can
   * invalidate it.
   */
  [[nodiscard]] auto FindCustom(std::string_view name) const noexcept
    -> GGEMSMaterial const *;

  /*!
   * \brief Requires a material at a registered manager index.
   *
   * \param[in] material_index Index in GetMaterials(), not an EM package ID.
   * \return A borrowed reference; registration growth can invalidate it.
   * \throws GGEMSRecoverable If material_index is outside the registered
   * vector.
   */
  [[nodiscard]] auto Require(std::uint32_t material_index) const
    -> GGEMSMaterial const &;

private:
  /*! \brief Creates empty registered and custom-material collections. */
  GGEMSMaterialManager() = default;

  /*! \brief Owned registered materials in first-use order. */
  std::vector<GGEMSMaterial> materials_;

  /*! \brief Owned available custom definitions. */
  std::vector<GGEMSMaterial> custom_materials_;
};

} // namespace ggems::core::materials
