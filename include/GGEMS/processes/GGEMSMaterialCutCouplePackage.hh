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
 * \brief Declares exact material/cut-couple identities and owned context
 * mappings.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <array>
#include <compare>
#include <cstdint>
#include <span>
#include <vector>
/// \endcond

#include "GGEMS/materials/GGEMSEMMaterialPackage.hh"
#include "GGEMS/processes/GGEMSProductionCutPolicy.hh"
#include "GGEMS/units/GGEMSEnergyUnits.hh"

namespace ggems::core::processes {

/*!
 * \brief Identifies a material and its four exact base production thresholds.
 *
 * Lengths, winning scopes, and authoring indices are provenance rather than
 * couple identity. A material ID is meaningful only within its matching EM
 * package.
 */
struct GGEMSMaterialCutCouple {
  std::uint32_t material_id; /*!< ID in the associated EM material package. */
  std::array<units::Energy, 4U>
    thresholds; /*!< Gamma/e-/e+/Proton energies in micro-eV. */

  /*!
   * \brief Orders couples by material ID and then the four integer thresholds.
   *
   * \return Lexicographic ordering; equality requires all five key components
   * to match.
   */
  [[nodiscard]] auto operator<=>(GGEMSMaterialCutCouple const &) const
    -> std::strong_ordering = default;
};

/*!
 * \brief Retains context authoring and scope information outside couple
 * identity.
 */
struct GGEMSProductionCutContextProvenance {
  std::uint32_t material_index; /*!< Original input-material index. */
  GGEMSResolvedProductionCuts
    cuts; /*!< Effective lengths and winning scopes. */
};

/*!
 * \brief Owns unique couples and maps original contexts to dense couple IDs.
 *
 * Conversion inputs are deduplicated by material ID plus effective lengths.
 * Converted couples are sorted and deduplicated by exact material ID and energy
 * thresholds; distinct lengths may therefore share a couple after quantization.
 * Couple IDs are sorted ranks for this package. Context order affects only the
 * context mapping and provenance order, for a fixed EM package and policy.
 * Construction retains no references to its inputs. Returned spans borrow this
 * object and can be invalidated by destruction, assignment, or moving from it.
 */
class GGEMSMaterialCutCouplePackage {
public:
  /*!
   * \brief Resolves contexts and builds their exact host material/cut-couple
   * package.
   *
   * \pre Context material indices must be valid, all channels must resolve, and
   * couple IDs must fit std::uint32_t.
   *
   * \param[in] materials EM package whose input-index mapping resolves context
   * materials.
   * \param[in] policy Length defaults and overrides used independently per
   * channel.
   * \param[in] contexts Input contexts; output mappings preserve this order.
   * \throws GGEMSRecoverable If a required channel conversion fails, including
   * Gamma conversion for empty matter.
   */
  GGEMSMaterialCutCouplePackage(
    materials::GGEMSEMMaterialPackage const &materials,
    GGEMSProductionCutPolicy const &policy,
    std::span<GGEMSProductionCutContext const> contexts);

  /*!
   * \brief Returns the sorted unique material/threshold tuples.
   *
   * \return A borrowed span indexed by package-local couple ID.
   */
  [[nodiscard]] auto GetCouples() const noexcept
    -> std::span<GGEMSMaterialCutCouple const> {
    return couples_;
  }

  /*!
   * \brief Returns the couple ID selected for every original context.
   *
   * \return A borrowed span in constructor context order.
   */
  [[nodiscard]] auto GetContextCoupleIds() const noexcept
    -> std::span<std::uint32_t const> {
    return context_couple_ids_;
  }

  /*!
   * \brief Returns effective lengths, winning scopes, and authoring material
   * indices.
   *
   * \return A borrowed span in constructor context order.
   */
  [[nodiscard]] auto GetContextProvenance() const noexcept
    -> std::span<GGEMSProductionCutContextProvenance const> {
    return context_provenance_;
  }

private:
  std::vector<GGEMSMaterialCutCouple>
    couples_; /*!< Owned sorted unique couples. */
  std::vector<std::uint32_t>
    context_couple_ids_; /*!< Context-to-couple-ID mapping. */
  std::vector<GGEMSProductionCutContextProvenance>
    context_provenance_; /*!< Context metadata. */
};

} // namespace ggems::core::processes
