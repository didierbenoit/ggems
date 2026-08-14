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
 * \brief Implements OpenCL cache fingerprint helpers.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <cstdint>
#include <string_view>
/// \endcond

#include "GGEMS/frameworks/GGEMSOpenCLCacheFingerprint.hh"

namespace ggems::ocl::detail {
namespace {
/*!
 * \brief FNV-1a 64-bit offset basis.
 */
constexpr std::uint64_t k_fnv1a64_offset_basis{14695981039346656037ULL};
/*!
 * \brief FNV-1a 64-bit prime.
 */
constexpr std::uint64_t k_fnv1a64_prime{1099511628211ULL};
} // namespace

[[nodiscard]] auto HashFNV1a64(std::string_view bytes) noexcept
    -> std::uint64_t {
  auto hash = k_fnv1a64_offset_basis;

  for (char const byte : bytes) {
    auto const unsigned_byte = static_cast<unsigned char>(byte);
    hash ^= static_cast<std::uint64_t>(unsigned_byte);
    hash *= k_fnv1a64_prime;
  }

  return hash;
}

} // namespace ggems::ocl::detail
