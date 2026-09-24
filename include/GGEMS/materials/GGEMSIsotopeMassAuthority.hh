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
 * \brief Declares the shared material-isotope molar-mass authority.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include "GGEMS/materials/GGEMSResolvedIsotopeTable.hh"

namespace ggems::core::materials {

/*!
 * \brief Returns the lazily constructed immutable isotope molar-mass table.
 *
 * The compiled authority contains 3298 rows through Z=99, including Ta-180m
 * (M=1). Molar masses include the tabulated isomer excitation contribution.
 * Mass lookup uses exact keys; other isomers are not synthesized.
 *
 * \return A reference to process-lifetime storage initialized on first use.
 */
[[nodiscard]] auto GetIsotopeMassAuthority()
  -> GGEMSResolvedIsotopeTable const &;
} // namespace ggems::core::materials
