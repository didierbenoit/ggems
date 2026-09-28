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
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <string_view>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/materials/GGEMSElement.hh"

namespace materials = ggems::core::materials;

// =============================================================================
// =============================================================================

TEST(GGEMSElementTest, StoresElementData) {
  constexpr materials::GGEMSElement iron{26U, "Fe", "iron", 55.845L};

  EXPECT_EQ(iron.GetAtomicNumber(), 26U);
  EXPECT_EQ(iron.GetSymbol(), "Fe");
  EXPECT_EQ(iron.GetName(), "iron");
  EXPECT_EQ(iron.GetMolarMass(), 55.845L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSElementTest, RejectsAtomicNumbersOutsideCatalogRange) {
  EXPECT_THROW(
    static_cast<void>(materials::GGEMSElement{0U, "H", "Hydrogen", 1.0080L}),
    ggems::core::GGEMSRecoverable);

  EXPECT_THROW(
    static_cast<void>(materials::GGEMSElement{100U, "Fm", "Fermium", 257.0L}),
    ggems::core::GGEMSRecoverable);
}

// =============================================================================
// =============================================================================

TEST(GGEMSElementTest, AcceptsHighestCatalogAtomicNumber) {
  constexpr materials::GGEMSElement einsteinium{99U, "Es", "Einsteinium",
                                                252.082979173L};

  EXPECT_EQ(einsteinium.GetAtomicNumber(), 99U);
}
