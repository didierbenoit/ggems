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
 * \brief Declares owning material inspection records and human-readable
 * composition reports.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>
/// \endcond

#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/materials/GGEMSMaterialComposition.hh"
#include "GGEMS/materials/GGEMSMaterialManager.hh"
#include "GGEMS/units/GGEMSDensityUnits.hh"

namespace ggems::core::materials {

/*!
 * \brief Describes whether inspection established registration in a manager.
 */
enum class GGEMSMaterialRegistration : std::uint8_t {
  Unknown = 0U,      /*!< No manager registration lookup was performed. */
  Unregistered = 1U, /*!< Available definition not registered in the manager. */
  Registered = 2U,   /*!< A manager index identifies the registered material. */
};

/*!
 * \brief Owns an elemental EM row together with copied catalog labels.
 */
struct GGEMSElementInspection {
  GGEMSDerivedElementalConstituent
    values;           /*!< Copied elemental EM properties. */
  std::string symbol; /*!< Owned chemical symbol. */
  std::string name;   /*!< Owned display name. */
};

/*!
 * \brief Owns a detached material description with optional registration
 * context.
 *
 * Strings and vectors are copies, so the record survives source destruction or
 * manager growth. Producers preserve Z order and isotope (Z, A, M) order. A
 * Registered record carries manager_index; this is an authoring index, not a
 * package-local material ID.
 */
struct GGEMSMaterialInspection {
  std::string name;       /*!< Owned display name. */
  units::Density density; /*!< Bulk density in pg/pm3. */
  GGEMSMaterialRegistration
    registration; /*!< Result of registration inspection. */
  std::optional<std::uint32_t>
    manager_index; /*!< Present for a registered material. */
  std::vector<GGEMSElementInspection>
    elements; /*!< Owned elemental rows by Z. */
  std::vector<GGEMSIsotopeConstituent>
    isotopes; /*!< Owned isotope rows by (Z, A, M). */
  long double
    total_atom_density_per_cubic_centimeter; /*!< Total atoms in 1/cm3. */
  long double
    electron_density_per_cubic_centimeter; /*!< Total electrons in 1/cm3. */
};

/*!
 * \brief Copies material properties without checking registration.
 *
 * \param[in] material Material whose owned data and catalog labels are copied.
 * \return An independent record with Unknown registration and no manager index.
 */
[[nodiscard]] auto InspectMaterial(GGEMSMaterial const &material)
  -> GGEMSMaterialInspection;

/*!
 * \brief Copies a registered material and its manager index.
 *
 * \param[in] manager Registry that owns the material.
 * \param[in] manager_index Index in the manager's registered-material vector.
 * \return An independent record marked Registered.
 * \throws GGEMSRecoverable If the manager index is not registered.
 */
[[nodiscard]] auto InspectMaterial(GGEMSMaterialManager const &manager,
                                   std::uint32_t manager_index)
  -> GGEMSMaterialInspection;

/*!
 * \brief Inspects an available material by name without registering it.
 *
 * Lookup tries registered materials, built-ins, then custom definitions.
 *
 * \param[in] manager Registry and custom-definition owner.
 * \param[in] name Exact, case-sensitive material name.
 * \return An owned Registered or Unregistered inspection record.
 * \throws GGEMSRecoverable If the name is unknown or constructing a built-in
 * material fails.
 */
[[nodiscard]] auto InspectMaterial(GGEMSMaterialManager const &manager,
                                   std::string_view name)
  -> GGEMSMaterialInspection;

/*!
 * \brief Formats material composition and density without registration context.
 *
 * This report contains no Production-Cut reference thresholds. The Python
 * reporting layer appends those separately.
 *
 * \param[in] material Material to inspect and format.
 * \return Owned text using the current logger encoding for unit presentation.
 */
[[nodiscard]] auto DescribeMaterial(GGEMSMaterial const &material)
  -> std::string;

/*!
 * \brief Formats an owning inspection record and its registration context.
 *
 * \param[in] inspection Consistent record in producer order; Registered
 * requires manager_index.
 * \return Owned text with density, elements, isotopes, and atom/electron
 * totals.
 */
[[nodiscard]] auto DescribeMaterial(GGEMSMaterialInspection const &inspection)
  -> std::string;

/*!
 * \brief Logs a material composition report at INFO level under Material.
 *
 * \param[in] Material Material to inspect and describe.
 */
auto VerboseMaterial(GGEMSMaterial const &Material) -> void;

/*!
 * \brief Logs an inspection report at INFO level under Material.
 *
 * \param[in] inspection Consistent owned inspection record to describe.
 */
auto VerboseMaterial(GGEMSMaterialInspection const &inspection) -> void;

} // namespace ggems::core::materials
