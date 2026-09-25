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
 * \brief Declares Poisson sampling from a GGEMS host random stream.
 *
 * Provides finite-precision small-mean inversion and transformed-rejection
 * sampling for larger Poisson means.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <cstdint>
/// \endcond

namespace ggems::core::random {

class GGEMSHostRandomStream;

/*!
 * \brief Samples a Poisson-distributed unsigned count.
 *
 * A zero mean returns zero without consuming the stream. Positive means below
 * 30 use cumulative inversion; other means use PTRS transformed rejection.
 * Neither the input nor the representability of a PTRS candidate is checked.
 *
 * \pre The mean must be finite and nonnegative. Every nonnegative PTRS
 * candidate must be finite and representable as uint64_t before its unchecked
 * cast; merely keeping the mean within that range is not sufficient.
 *
 * \param[in] mean Dimensionless expected count.
 * \param[in,out] random Owned caller stream advanced by sampling.
 * \return The sampled unsigned count.
 */
[[nodiscard]] auto SamplePoisson(long double mean,
                                 GGEMSHostRandomStream &random)
  -> std::uint64_t;
} // namespace ggems::core::random
