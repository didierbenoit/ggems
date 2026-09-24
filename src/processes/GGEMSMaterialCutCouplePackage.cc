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
 * \brief Resolves contexts and interns exact material/threshold tuples into
 * sorted host packages.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <algorithm>
#include <array>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>
/// \endcond

#include "GGEMS/materials/GGEMSEMMaterialPackage.hh"
#include "GGEMS/processes/GGEMSMaterialCutCouplePackage.hh"
#include "GGEMS/processes/GGEMSProductionCutConverter.hh"
#include "GGEMS/processes/GGEMSProductionCutPolicy.hh"
#include "GGEMS/units/GGEMSEnergyUnits.hh"

namespace ggems::core::processes {

namespace {

// =============================================================================
// =============================================================================

/*!
 * \brief Keys one conversion by material ID and effective lengths before
 * quantization.
 */
struct ResolvedContext {
  std::uint32_t material_id; /*!< ID in the input EM package. */
  GGEMSResolvedProductionCutLengths
    lengths; /*!< Gamma/e-/e+/Proton lengths in pm. */

  /*!
   * \brief Orders conversion inputs by material ID and then all four exact
   * lengths.
   *
   * \return Lexicographic ordering used to deduplicate converter calls.
   */
  [[nodiscard]] auto operator<=>(ResolvedContext const &) const
    -> std::strong_ordering = default;
};

// =============================================================================
// =============================================================================

/*!
 * \brief Returns a sorted sequence with adjacent equal values removed.
 *
 * \tparam Value Value type supporting ordering and equality.
 * \param[in] values Owned sequence to sort and deduplicate.
 * \return An owned sequence containing one instance of each equal value.
 */
template <typename Value>
[[nodiscard]] auto SortedUnique(std::vector<Value> values)
  -> std::vector<Value> {
  std::ranges::sort(values);
  auto const duplicates = std::ranges::unique(values);
  values.erase(duplicates.begin(), duplicates.end());
  return values;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Locates the lower-bound position of a value in a sorted sequence.
 *
 * \pre Callers that use the result as a stored value index must supply a
 * present value.
 *
 * \tparam Value Value type with ordering compatible with the sequence.
 * \param[in] sorted Sequence sorted in increasing order.
 * \param[in] value Value whose rank is needed.
 * \return The insertion index of value, possibly sorted.size() when absent.
 */
template <typename Value>
[[nodiscard]] auto RankOf(std::vector<Value> const &sorted, Value const &value)
  -> std::size_t {
  return static_cast<std::size_t>(std::ranges::lower_bound(sorted, value) -
                                  sorted.begin());
}

} // namespace

// =============================================================================
// =============================================================================

GGEMSMaterialCutCouplePackage::GGEMSMaterialCutCouplePackage(
  materials::GGEMSEMMaterialPackage const &materials,
  GGEMSProductionCutPolicy const &policy,
  std::span<GGEMSProductionCutContext const> contexts) {

  auto const material_ids = materials.GetMaterialIds();

  std::vector<ResolvedContext> resolved_contexts;
  resolved_contexts.reserve(contexts.size());
  context_provenance_.reserve(contexts.size());

  for (auto const &context : contexts) {
    auto const cuts = ResolveProductionCuts(policy, context);

    resolved_contexts.push_back({
      .material_id = material_ids[context.material_index],
      .lengths = cuts.lengths,
    });

    context_provenance_.push_back({
      .material_index = context.material_index,
      .cuts = cuts,
    });
  }

  auto const conversion_inputs = SortedUnique(resolved_contexts);

  std::vector<GGEMSMaterialCutCouple> converted;
  converted.reserve(conversion_inputs.size());
  for (auto const &input : conversion_inputs) {
    std::array<units::Energy, 4U> thresholds{};
    for (auto const channel : k_production_cut_channels) {
      auto const index = ProductionCutChannelIndex(channel);
      thresholds[index] = ConvertProductionCutLength(
        channel, input.lengths[index], materials, input.material_id);
    }
    converted.push_back({
      .material_id = input.material_id,
      .thresholds = thresholds,
    });
  }

  couples_ = SortedUnique(converted);

  context_couple_ids_.reserve(resolved_contexts.size());
  for (auto const &context : resolved_contexts) {
    auto const &couple = converted[RankOf(conversion_inputs, context)];
    context_couple_ids_.push_back(
      static_cast<std::uint32_t>(RankOf(couples_, couple)));
  }
}

} // namespace ggems::core::processes
