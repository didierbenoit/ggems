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
 * \brief Declares an owned flat host package of deduplicated elemental EM
 * material data.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <cstdint>
#include <span>
#include <vector>
/// \endcond

#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/units/GGEMSDensityUnits.hh"

namespace ggems::core::materials {

/*!
 * \brief Stores one isotope-derived elemental contribution in a flat EM array.
 */
struct GGEMSEMElementalConstituent {
  std::uint32_t atomic_number; /*!< Chemical identity Z. */
  long double mass_fraction;   /*!< Derived share of material mass. */
  long double
    number_density_per_cubic_centimeter; /*!< Element atoms in 1/cm3. */
  long double
    electron_density_per_cubic_centimeter; /*!< Electron density in 1/cm3. */
};

/*!
 * \brief Locates one material's elemental rows and stores its bulk properties.
 */
struct GGEMSEMMaterialDescriptor {
  units::Density density;          /*!< Bulk density in canonical pg/pm3. */
  std::uint32_t first_constituent; /*!< Offset into the flat elemental array. */
  std::uint32_t constituent_count; /*!< Number of elemental rows. */
  long double
    total_atom_density_per_cubic_centimeter; /*!< Total atoms in 1/cm3. */
  long double
    electron_density_per_cubic_centimeter; /*!< Electron density in 1/cm3. */
};

/*!
 * \brief Owns flattened EM properties with exact material deduplication.
 *
 * The first occurrence of each scientific identity assigns its dense package
 * ID. Input order therefore affects IDs; IDs are local to this package. One
 * input material index maps to one descriptor ID even when several inputs
 * deduplicate. The package owns copied values and retains no material pointers.
 * It is host preparation data, with long double fields, not an implemented
 * device upload. Isotope rows remain in host materials and are not included in
 * this EM package. All returned spans borrow package storage and can be
 * invalidated by destruction, assignment, or moving from the package.
 */
class GGEMSEMMaterialPackage {
public:
  /*!
   * \brief Copies unique material properties into descriptor and elemental
   * arrays.
   *
   * \pre Material counts, elemental counts, and offsets must fit std::uint32_t.
   *
   * \param[in] materials Input materials in authoring-index order; needed only
   * during construction.
   */
  explicit GGEMSEMMaterialPackage(std::span<GGEMSMaterial const> materials);

  /*!
   * \brief Returns one descriptor per unique scientific material.
   *
   * \return A borrowed span indexed by dense package-local material ID.
   */
  [[nodiscard]] auto GetDescriptors() const noexcept
    -> std::span<GGEMSEMMaterialDescriptor const> {
    return descriptors_;
  }

  /*!
   * \brief Returns the concatenated isotope-derived elemental EM rows.
   *
   * \return A borrowed span addressed by descriptor offset/count pairs.
   */
  [[nodiscard]] auto GetElementalConstituents() const noexcept
    -> std::span<GGEMSEMElementalConstituent const> {
    return elemental_constituents_;
  }

  /*!
   * \brief Maps each constructor input index to its deduplicated descriptor ID.
   *
   * \return A borrowed span in original input order.
   */
  [[nodiscard]] auto GetMaterialIds() const noexcept
    -> std::span<std::uint32_t const> {
    return material_ids_;
  }

private:
  std::vector<GGEMSEMMaterialDescriptor>
    descriptors_; /*!< Owned unique-material descriptors. */
  std::vector<GGEMSEMElementalConstituent>
    elemental_constituents_; /*!< Flat EM rows. */
  std::vector<std::uint32_t>
    material_ids_; /*!< Input-index to descriptor-ID mapping. */
};

} // namespace ggems::core::materials
