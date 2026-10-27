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
 * \brief Saturates diagnostic Observer counters for the public uint32 view.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */
#pragma once

#include <cstdint>
#include <limits>

/*!
 * \brief Converts wide Observer totals to bounded display counters.
 *
 * \namespace ggems::core::observer::detail
 */
namespace ggems::core::observer::detail {

// =============================================================================
// =============================================================================

/*!
 * \brief Clamps a wide diagnostic total to the uint32 counter range.
 *
 * \param[in] value Logical diagnostic count.
 * \return The input when representable, otherwise UINT32_MAX.
 */
[[nodiscard]] constexpr auto
SaturateObserverCounter(std::uint64_t value) noexcept -> std::uint32_t {
  constexpr auto maximum_counter = std::numeric_limits<std::uint32_t>::max();
  constexpr auto maximum_counter_wide =
    static_cast<std::uint64_t>(maximum_counter);

  if (value > maximum_counter_wide) {
    return maximum_counter;
  }

  return static_cast<std::uint32_t>(value);
}

// =============================================================================
// =============================================================================

/*!
 * \brief Adds a diagnostic count without wrapping the public counter.
 *
 * \param[in,out] destination Counter updated in place, saturating at
 * UINT32_MAX.
 * \param[in] value Nonnegative increment.
 */
constexpr auto AddSaturatedObserverCounter(std::uint32_t &destination,
                                           std::uint64_t value) noexcept
  -> void {
  constexpr auto maximum_counter = std::numeric_limits<std::uint32_t>::max();
  constexpr auto maximum_counter_wide =
    static_cast<std::uint64_t>(maximum_counter);
  auto const destination_wide = static_cast<std::uint64_t>(destination);
  auto const remaining_capacity = maximum_counter_wide - destination_wide;

  if (value >= remaining_capacity) {
    destination = maximum_counter;
    return;
  }

  destination += static_cast<std::uint32_t>(value);
}

} // namespace ggems::core::observer::detail
