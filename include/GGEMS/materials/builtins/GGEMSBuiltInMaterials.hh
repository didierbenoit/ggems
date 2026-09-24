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
 * \brief Declares the built-in material name catalog and material factories.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <span>
#include <string_view>
/// \endcond

#include "GGEMS/materials/GGEMSMaterial.hh"

/*!
 * \namespace ggems::core::materials::builtins
 * \brief Provides compiled elemental, compound, and exact-vacuum material
 * presets.
 */
namespace ggems::core::materials::builtins {

/*!
 * \brief Lists built-in names without constructing or registering materials.
 *
 * Order is Vacuum, the 92 elemental presets from Hydrogen through Uranium, then
 * 24 compound presets in table order. This preset list is smaller than the
 * 99-element identity catalog.
 *
 * \return A span of 117 names and static string storage, valid for the process
 * lifetime.
 */
[[nodiscard]] auto GetAvailableMaterialNames() noexcept
  -> std::span<std::string_view const>;

/*!
 * \brief Builds an independently owned material from an exact built-in name.
 *
 * Matter presets use elemental mass fractions and default isotope mixtures.
 * Vacuum has exactly zero density and no matter constituents. Construction does
 * not register the material with GGEMSMaterialManager.
 *
 * \param[in] canonical_name Exact, case-sensitive name from
 * GetAvailableMaterialNames().
 * \return An owned material value.
 * \throws GGEMSRecoverable If the name is unknown or preset composition
 * preparation fails.
 */
[[nodiscard]] auto BuildBuiltInMaterial(std::string_view canonical_name)
  -> GGEMSMaterial;

} // namespace ggems::core::materials::builtins
