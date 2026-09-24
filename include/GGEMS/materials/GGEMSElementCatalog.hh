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
 * \brief Declares lookup in the immutable H-through-Es element catalog.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <cstdint>
#include <span>
#include <string_view>
/// \endcond

#include "GGEMS/materials/GGEMSElement.hh"

namespace ggems::core::materials {

/*!
 * \brief Returns the 99 catalog elements in increasing atomic-number order.
 *
 * \return A span of static storage, valid for the process lifetime.
 */
[[nodiscard]] auto GetElements() noexcept -> std::span<GGEMSElement const>;

/*!
 * \brief Looks up an element by proton number.
 *
 * \param[in] atomic_number Proton number Z.
 * \return A pointer to static catalog storage, or nullptr when no entry
 * matches.
 */
[[nodiscard]] auto
FindElementByAtomicNumber(std::uint32_t atomic_number) noexcept
  -> GGEMSElement const *;

/*!
 * \brief Looks up an element by chemical symbol.
 *
 * \param[in] symbol Exact, case-sensitive chemical symbol.
 * \return A pointer to static catalog storage, or nullptr when no entry
 * matches.
 */
[[nodiscard]] auto FindElementBySymbol(std::string_view symbol) noexcept
  -> GGEMSElement const *;

/*!
 * \brief Looks up an element by catalog name.
 *
 * \param[in] canonical_name Exact, case-sensitive catalog name.
 * \return A pointer to static catalog storage, or nullptr when no entry
 * matches.
 */
[[nodiscard]] auto FindElementByName(std::string_view canonical_name) noexcept
  -> GGEMSElement const *;

/*!
 * \brief Requires an element matching the supplied proton number.
 *
 * \param[in] atomic_number Proton number Z.
 * \return A reference to static catalog storage, valid for the process
 * lifetime.
 * \throws GGEMSRecoverable If no catalog entry matches.
 */
[[nodiscard]] auto RequireElementByAtomicNumber(std::uint32_t atomic_number)
  -> GGEMSElement const &;

/*!
 * \brief Requires an element matching the supplied symbol.
 *
 * \param[in] symbol Exact, case-sensitive chemical symbol.
 * \return A reference to static catalog storage, valid for the process
 * lifetime.
 * \throws GGEMSRecoverable If no catalog entry matches.
 */
[[nodiscard]] auto RequireElementBySymbol(std::string_view symbol)
  -> GGEMSElement const &;

/*!
 * \brief Requires an element matching the supplied catalog name.
 *
 * \param[in] canonical_name Exact, case-sensitive catalog name.
 * \return A reference to static catalog storage, valid for the process
 * lifetime.
 * \throws GGEMSRecoverable If no catalog entry matches.
 */
[[nodiscard]] auto RequireElementByName(std::string_view canonical_name)
  -> GGEMSElement const &;

} // namespace ggems::core::materials
