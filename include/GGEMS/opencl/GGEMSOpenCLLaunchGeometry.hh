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
 * \brief Provides OpenCL launch-geometry calculations.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <cstddef>
#include <limits>
#include <optional>
/// \endcond

namespace ggems::ocl::detail {

/*!
 * \brief Computes a global work size padded to a local work-group multiple.
 *
 * \param[in] logical_work_size Logical number of work-items to launch.
 * \param[in] local_work_size Requested local work-group size.
 * \return Padded global work size, or an empty optional if the local size is zero or padding would overflow.
 */
[[nodiscard]] constexpr auto
TryComputePaddedGlobalWorkSize(std::size_t logical_work_size,
                               std::size_t local_work_size) noexcept
    -> std::optional<std::size_t> {
  if (local_work_size == 0U) {
    return std::nullopt;
  }

  auto const remainder = logical_work_size % local_work_size;
  if (remainder == 0U) {
    return logical_work_size;
  }

  auto const increment = local_work_size - remainder;
  if (logical_work_size > std::numeric_limits<std::size_t>::max() - increment) {
    return std::nullopt;
  }

  return logical_work_size + increment;
}

} // namespace ggems::ocl::detail
