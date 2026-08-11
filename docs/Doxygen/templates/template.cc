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
 * \brief XXX.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include "GGEMS/.../XXX.hh"

/// \cond
#include <cstdint>
#include <string>
/// \endcond

#include "GGEMS/..."

namespace {

/*!
 * \brief XXX.
 *
 * XXX.
 *
 * \param[in] value XXX.
 * \return XXX.
 */
[[nodiscard]] std::uint64_t XXX(std::uint64_t value) noexcept { return value; }

// =============================================================================
// =============================================================================

/*!
 * \brief XXX.
 *
 * XXX.
 *
 * \param[in] value XXX.
 * \return XXX.
 */
[[nodiscard]] std::uint64_t YYY(std::uint64_t value) noexcept { return value; }

} // namespace

// ============================================================================
// ============================================================================

namespace ggems::xxx {

XXXClass::XXXClass(std::string value) : value_(std::move(value)) {}

// ----------------------------------------------------------------------------

std::string const &XXXClass::GetXXX() const noexcept { return value_; }

// ----------------------------------------------------------------------------

void XXXClass::DoXXX() {}

} // namespace ggems::xxx
