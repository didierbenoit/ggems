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
 * \brief Defines shared key ordering and normalization for authored fractions.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <algorithm>
#include <cmath>
#include <format>
#include <string_view>
#include <vector>
/// \endcond

#include "GGEMS/GGEMSException.hh"

namespace ggems::core::materials::detail {

/*!
 * \brief Defines the provisional absolute tolerance for admitting a fraction
 * sum.
 *
 * This dimensionless 1.0e-5 authoring tolerance is not a physics accuracy
 * bound.
 */
inline constexpr long double k_fraction_sum_tolerance{1.0e-5L};

/*!
 * \brief Sorts and normalizes a fraction list after key and sum checks.
 *
 * Entries are sorted by key. Duplicate keys are rejected before exact-zero
 * entries are removed. The remaining sum must differ from one by no more than
 * 1.0e-5, then every retained fraction is divided by that sum. Individual signs
 * and finiteness are not checked here. The input is mutated before a possible
 * exception; there is no rollback.
 *
 * \pre Fractions must be finite and nonnegative; this helper does not enforce
 * that.
 *
 * \tparam Entry Record type carrying a long double fraction member.
 * \tparam KeyProjection Projection supplying sortable and equality-comparable
 * semantic keys.
 * \param[in,out] entries Records to sort, prune, and normalize.
 * \param[in] key_projection Projection defining key order and duplicate
 * identity.
 * \param[in] fraction_member Pointer to the dimensionless fraction field in
 * each entry.
 * \param[in] subject Borrowed diagnostic label, used only during this call.
 * \throws GGEMSRecoverable If duplicate keys exist or the retained sum differs
 * from one by more than the tolerance.
 */
template <typename Entry, typename KeyProjection>
auto CanonicalizeFractions(std::vector<Entry> &entries,
                           KeyProjection key_projection,
                           long double Entry::*fraction_member,
                           std::string_view subject) -> void {
  std::ranges::sort(entries, {}, key_projection);

  if (std::ranges::adjacent_find(entries, {}, key_projection) !=
      entries.end()) {
    throw GGEMSRecoverable{
      std::format("{} fractions contain duplicate keys.", subject)};
  }

  std::erase_if(entries, [fraction_member](Entry const &entry) -> bool {
    return entry.*fraction_member == 0.0L;
  });

  long double fraction_sum{0.0L};
  for (auto const &entry : entries) {
    fraction_sum += entry.*fraction_member;
  }

  if (std::abs(fraction_sum - 1.0L) > k_fraction_sum_tolerance) {
    throw GGEMSRecoverable{
      std::format("{} fractions must sum to one within the provisional 1.0e-5 "
                  "admission tolerance.",
                  subject)};
  }

  for (auto &entry : entries) {
    entry.*fraction_member /= fraction_sum;
  }
}

} // namespace ggems::core::materials::detail
