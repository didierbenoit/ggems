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

#include <algorithm>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/materials/GGEMSIsotope.hh"

namespace materials = ggems::core::materials;

// =============================================================================
// =============================================================================

TEST(GGEMSIsotopeTest, StoresExplicitKey) {
  constexpr materials::GGEMSIsotope tantalum_180m{73U, 180U, 1U};

  static_assert(tantalum_180m.GetAtomicNumber() == 73U);
  EXPECT_EQ(tantalum_180m.GetAtomicNumber(), 73U);
  EXPECT_EQ(tantalum_180m.GetMassNumber(), 180U);
  EXPECT_EQ(tantalum_180m.GetIsomerState(), 1U);
}

// =============================================================================
// =============================================================================

TEST(GGEMSIsotopeTest, IdentityIsTheCompleteKey) {
  materials::GGEMSIsotope const boron_10{5U, 10U, 0U};

  EXPECT_EQ(boron_10, (materials::GGEMSIsotope{5U, 10U, 0U}));
  EXPECT_NE(boron_10, (materials::GGEMSIsotope{6U, 10U, 0U}));
  EXPECT_NE(boron_10, (materials::GGEMSIsotope{5U, 11U, 0U}));
  EXPECT_NE(boron_10, (materials::GGEMSIsotope{5U, 10U, 1U}));
}

// =============================================================================
// =============================================================================

TEST(GGEMSIsotopeTest, OrdersLexicographicallyByAtomicMassAndIsomerKeys) {
  std::vector<materials::GGEMSIsotope> isotopes{
    {73U, 181U, 0U}, {5U, 11U, 0U},   {73U, 180U, 1U}, {1U, 2U, 0U},
    {8U, 16U, 0U},   {73U, 180U, 0U}, {5U, 10U, 0U},   {1U, 1U, 0U},
  };

  std::ranges::sort(isotopes);

  std::vector<materials::GGEMSIsotope> const expected{
    {1U, 1U, 0U},  {1U, 2U, 0U},    {5U, 10U, 0U},   {5U, 11U, 0U},
    {8U, 16U, 0U}, {73U, 180U, 0U}, {73U, 180U, 1U}, {73U, 181U, 0U},
  };

  EXPECT_EQ(isotopes, expected);

  EXPECT_LT((materials::GGEMSIsotope{5U, 20U, 9U}),
            (materials::GGEMSIsotope{6U, 11U, 0U}));
  EXPECT_LT((materials::GGEMSIsotope{73U, 180U, 1U}),
            (materials::GGEMSIsotope{73U, 181U, 0U}));
}

// =============================================================================
// =============================================================================

TEST(GGEMSIsotopeTest, RejectsAtomicNumbersOutsideCatalogRange) {
  EXPECT_THROW(static_cast<void>(materials::GGEMSIsotope{0U, 1U, 0U}),
               ggems::core::GGEMSRecoverable);

  EXPECT_THROW(static_cast<void>(materials::GGEMSIsotope{100U, 257U, 0U}),
               ggems::core::GGEMSRecoverable);
}

// =============================================================================
// =============================================================================

TEST(GGEMSIsotopeTest, AcceptsKeysWithoutRequiringTabulatedMasses) {
  EXPECT_NO_THROW(static_cast<void>(materials::GGEMSIsotope{1U, 1U, 0U}));

  EXPECT_NO_THROW(static_cast<void>(materials::GGEMSIsotope{99U, 252U, 0U}));

  EXPECT_NO_THROW(static_cast<void>(materials::GGEMSIsotope{5U, 30U, 0U}));

  EXPECT_NO_THROW(static_cast<void>(materials::GGEMSIsotope{73U, 180U, 2U}));
}
