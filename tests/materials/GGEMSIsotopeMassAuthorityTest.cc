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

#include <cmath>
#include <limits>

#include <gtest/gtest.h>

#include "GGEMS/materials/GGEMSIsotope.hh"
#include "GGEMS/materials/GGEMSIsotopeMassAuthority.hh"
#include "GGEMS/materials/GGEMSElementCatalog.hh"
#include "GGEMS/materials/GGEMSIsotopicComposition.hh"

namespace {

// =============================================================================
// =============================================================================

namespace materials = ggems::core::materials;

constexpr long double k_relative_budget{
  64.0L * std::numeric_limits<long double>::epsilon(),
};

constexpr long double k_molar_mass_constant{1.00000000105L};

// =============================================================================
// =============================================================================

auto ExpectMolarMass(materials::GGEMSIsotope const &isotope,
                     long double expected) -> void {
  auto const *resolved = materials::GetIsotopeMassAuthority().Find(isotope);

  ASSERT_NE(resolved, nullptr);
  EXPECT_LE(std::abs(resolved->molar_mass_grams_per_mole - expected),
            k_relative_budget * expected);
}

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSIsotopeMassAuthorityTest, ResolvesAme2020MolarMasses) {
  ExpectMolarMass({1U, 1U, 0U}, 1.007825031898L * k_molar_mass_constant);
  ExpectMolarMass({1U, 2U, 0U}, 2.014101777844L * k_molar_mass_constant);
  ExpectMolarMass({5U, 10U, 0U}, 10.012936862L * k_molar_mass_constant);
  ExpectMolarMass({5U, 11U, 0U}, 11.009305166L * k_molar_mass_constant);
  ExpectMolarMass({6U, 12U, 0U}, 12.0L * k_molar_mass_constant);
  ExpectMolarMass({8U, 16U, 0U}, 15.99491461926L * k_molar_mass_constant);
  ExpectMolarMass({64U, 157U, 0U}, 156.923967424L * k_molar_mass_constant);
  ExpectMolarMass({82U, 208U, 0U}, 207.976652005L * k_molar_mass_constant);
  ExpectMolarMass({92U, 238U, 0U}, 238.050786936L * k_molar_mass_constant);
  ExpectMolarMass({43U, 97U, 0U}, 96.90636072L * k_molar_mass_constant);
  ExpectMolarMass({89U, 227U, 0U}, 227.027750594L * k_molar_mass_constant);
}

// =============================================================================
// =============================================================================

TEST(GGEMSIsotopeMassAuthorityTest, ResolvesTantalumGroundAndIsomerMasses) {
  ExpectMolarMass({73U, 180U, 0U}, 179.947467589L * k_molar_mass_constant);

  ExpectMolarMass({73U, 180U, 1U}, 179.94754861581571781900690434961243L);

  ExpectMolarMass({73U, 181U, 0U}, 180.947998528L * k_molar_mass_constant);
}

// =============================================================================
// =============================================================================

TEST(GGEMSIsotopeMassAuthorityTest, CoversEveryDefaultIsotopicComposition) {
  auto const &authority = materials::GetIsotopeMassAuthority();

  for (auto const &element : materials::GetElements()) {
    SCOPED_TRACE(element.GetAtomicNumber());

    auto const composition =
      materials::BuildDefaultIsotopicComposition(element.GetAtomicNumber());

    for (auto const &fraction : composition.GetFractions()) {
      EXPECT_NE(authority.Find(fraction.isotope), nullptr);
    }
  }
}
