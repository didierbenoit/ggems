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

#include <gtest/gtest.h>

#include "GGEMS/particles/GGEMSParticleTypes.hh"

// =============================================================================
// =============================================================================

TEST(GGEMSParticleTypes, ParsesLongShortAndSymbolicNames) {
  using ggems::core::particles::GGEMSParticleType;
  using ggems::core::particles::ParseParticleType;

  EXPECT_EQ(ParseParticleType("aionino"), GGEMSParticleType::Aionino);
  EXPECT_EQ(ParseParticleType("l"), GGEMSParticleType::Aionino);

  EXPECT_EQ(ParseParticleType("gamma"), GGEMSParticleType::Gamma);
  EXPECT_EQ(ParseParticleType("photon"), GGEMSParticleType::Gamma);
  EXPECT_EQ(ParseParticleType("g"), GGEMSParticleType::Gamma);

  EXPECT_EQ(ParseParticleType("electron"), GGEMSParticleType::Electron);
  EXPECT_EQ(ParseParticleType("e-"), GGEMSParticleType::Electron);
  EXPECT_EQ(ParseParticleType("b-"), GGEMSParticleType::Electron);

  EXPECT_EQ(ParseParticleType("positron"), GGEMSParticleType::Positron);
  EXPECT_EQ(ParseParticleType("e+"), GGEMSParticleType::Positron);
  EXPECT_EQ(ParseParticleType("b+"), GGEMSParticleType::Positron);

  EXPECT_EQ(ParseParticleType("proton"), GGEMSParticleType::Proton);
  EXPECT_EQ(ParseParticleType("p"), GGEMSParticleType::Proton);

  EXPECT_EQ(ParseParticleType("neutron"), GGEMSParticleType::Neutron);
  EXPECT_EQ(ParseParticleType("n"), GGEMSParticleType::Neutron);

  EXPECT_EQ(ParseParticleType("alpha"), GGEMSParticleType::Alpha);
  EXPECT_EQ(ParseParticleType("a"), GGEMSParticleType::Alpha);
}

// =============================================================================
// =============================================================================

TEST(GGEMSParticleTypes, HasStableLongNames) {
  using ggems::core::particles::GGEMSParticleType;
  using ggems::core::particles::ToLongName;

  EXPECT_EQ(ToLongName(GGEMSParticleType::Unknown), "Unknown");
  EXPECT_EQ(ToLongName(GGEMSParticleType::Aionino), "Aionino");
  EXPECT_EQ(ToLongName(GGEMSParticleType::Gamma), "Gamma");
  EXPECT_EQ(ToLongName(GGEMSParticleType::Electron), "Electron");
  EXPECT_EQ(ToLongName(GGEMSParticleType::Positron), "Positron");
  EXPECT_EQ(ToLongName(GGEMSParticleType::Proton), "Proton");
  EXPECT_EQ(ToLongName(GGEMSParticleType::Neutron), "Neutron");
  EXPECT_EQ(ToLongName(GGEMSParticleType::Alpha), "Alpha");
}

// =============================================================================
// =============================================================================

TEST(GGEMSParticleTypes, HasStableShortNames) {
  using ggems::core::particles::GGEMSParticleType;
  using ggems::core::particles::ToShortName;

  EXPECT_EQ(ToShortName(GGEMSParticleType::Unknown), "?");
  EXPECT_EQ(ToShortName(GGEMSParticleType::Aionino), "l");
  EXPECT_EQ(ToShortName(GGEMSParticleType::Gamma), "g");
  EXPECT_EQ(ToShortName(GGEMSParticleType::Electron), "b-");
  EXPECT_EQ(ToShortName(GGEMSParticleType::Positron), "b+");
  EXPECT_EQ(ToShortName(GGEMSParticleType::Proton), "p");
  EXPECT_EQ(ToShortName(GGEMSParticleType::Neutron), "n");
  EXPECT_EQ(ToShortName(GGEMSParticleType::Alpha), "a");
}

// =============================================================================
// =============================================================================

TEST(GGEMSParticleTypes, HasStableAsciiAndUnicodeSymbols) {
  using ggems::core::particles::GGEMSParticleType;
  using ggems::core::particles::ToAsciiSymbol;
  using ggems::core::particles::ToUnicodeSymbol;

  EXPECT_EQ(ToAsciiSymbol(GGEMSParticleType::Unknown), U"?");
  EXPECT_EQ(ToUnicodeSymbol(GGEMSParticleType::Unknown), U"?");

  EXPECT_EQ(ToAsciiSymbol(GGEMSParticleType::Aionino), U"l");
  EXPECT_EQ(ToUnicodeSymbol(GGEMSParticleType::Aionino), U"λ");

  EXPECT_EQ(ToAsciiSymbol(GGEMSParticleType::Gamma), U"g");
  EXPECT_EQ(ToUnicodeSymbol(GGEMSParticleType::Gamma), U"γ");

  EXPECT_EQ(ToAsciiSymbol(GGEMSParticleType::Electron), U"e-");
  EXPECT_EQ(ToUnicodeSymbol(GGEMSParticleType::Electron), U"β-");

  EXPECT_EQ(ToAsciiSymbol(GGEMSParticleType::Positron), U"e+");
  EXPECT_EQ(ToUnicodeSymbol(GGEMSParticleType::Positron), U"β+");

  EXPECT_EQ(ToAsciiSymbol(GGEMSParticleType::Proton), U"p");
  EXPECT_EQ(ToUnicodeSymbol(GGEMSParticleType::Proton), U"p");

  EXPECT_EQ(ToAsciiSymbol(GGEMSParticleType::Neutron), U"n");
  EXPECT_EQ(ToUnicodeSymbol(GGEMSParticleType::Neutron), U"ν");

  EXPECT_EQ(ToAsciiSymbol(GGEMSParticleType::Alpha), U"a");
  EXPECT_EQ(ToUnicodeSymbol(GGEMSParticleType::Alpha), U"α");
}
