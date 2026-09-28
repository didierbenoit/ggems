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

#include <limits>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/materials/GGEMSIsotope.hh"
#include "GGEMS/materials/GGEMSResolvedIsotopeTable.hh"

namespace materials = ggems::core::materials;

// =============================================================================
// =============================================================================

TEST(GGEMSResolvedIsotopeTableTest, FindsExactKeysOnly) {
  materials::GGEMSResolvedIsotopeTable const table{{
    {.isotope = {73U, 180U, 1U}, .molar_mass_grams_per_mole = 179.95L},
    {.isotope = {5U, 10U, 0U}, .molar_mass_grams_per_mole = 10.01L},
  }};

  auto const *boron_10 = table.Find({5U, 10U, 0U});

  ASSERT_NE(boron_10, nullptr);
  EXPECT_EQ(boron_10->isotope, (materials::GGEMSIsotope{5U, 10U, 0U}));
  EXPECT_EQ(boron_10->molar_mass_grams_per_mole, 10.01L);
  EXPECT_EQ(&table.Require({5U, 10U, 0U}), boron_10);

  auto const *tantalum_180m = table.Find({73U, 180U, 1U});

  ASSERT_NE(tantalum_180m, nullptr);
  EXPECT_EQ(tantalum_180m->isotope, (materials::GGEMSIsotope{73U, 180U, 1U}));
  EXPECT_EQ(tantalum_180m->molar_mass_grams_per_mole, 179.95L);
  EXPECT_EQ(&table.Require({73U, 180U, 1U}), tantalum_180m);

  // No substitution of a neighboring isotope or a different isomer state.
  EXPECT_EQ(table.Find({5U, 11U, 0U}), nullptr);
  EXPECT_EQ(table.Find({73U, 180U, 0U}), nullptr);

  EXPECT_THROW(static_cast<void>(table.Require({5U, 11U, 0U})),
               ggems::core::GGEMSRecoverable);
  EXPECT_THROW(static_cast<void>(table.Require({73U, 180U, 0U})),
               ggems::core::GGEMSRecoverable);
}

// =============================================================================
// =============================================================================

TEST(GGEMSResolvedIsotopeTableTest, PropertiesDoNotRedefineIdentity) {
  materials::GGEMSResolvedIsotope const first{
    .isotope = {5U, 10U, 0U},
    .molar_mass_grams_per_mole = 10.01L,
  };

  materials::GGEMSResolvedIsotope const second{
    .isotope = {5U, 10U, 0U},
    .molar_mass_grams_per_mole = 10.02L,
  };

  EXPECT_EQ(first.isotope, second.isotope);
  EXPECT_NE(first.molar_mass_grams_per_mole, second.molar_mass_grams_per_mole);
}
