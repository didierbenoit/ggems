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
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/materials/GGEMSIsotope.hh"
#include "GGEMS/materials/GGEMSIsotopeMassAuthority.hh"
#include "GGEMS/materials/GGEMSIsotopicComposition.hh"

namespace {

// =============================================================================
// =============================================================================

namespace materials = ggems::core::materials;

constexpr long double k_relative_budget{
  64.0L * std::numeric_limits<long double>::epsilon(),
};

constexpr std::array<materials::GGEMSIsotope, 15U> k_reference_isotopes{
  {
    {43U, 97U, 0U},
    {61U, 145U, 0U},
    {84U, 209U, 0U},
    {85U, 210U, 0U},
    {86U, 222U, 0U},
    {87U, 223U, 0U},
    {88U, 226U, 0U},
    {89U, 227U, 0U},
    {93U, 237U, 0U},
    {94U, 244U, 0U},
    {95U, 243U, 0U},
    {96U, 247U, 0U},
    {97U, 247U, 0U},
    {98U, 251U, 0U},
    {99U, 252U, 0U},
  },
};

// =============================================================================
// =============================================================================

struct ExpectedFraction {
  materials::GGEMSIsotope isotope;
  long double fraction;
};

// =============================================================================
// =============================================================================

auto ExpectProfile(std::uint32_t atomic_number,
                   std::vector<ExpectedFraction> const &expected) -> void {
  SCOPED_TRACE(atomic_number);

  auto const composition =
    materials::BuildDefaultIsotopicComposition(atomic_number);

  EXPECT_EQ(composition.GetBasis(),
            materials::GGEMSFractionBasis::AtomFraction);
  EXPECT_EQ(composition.GetAtomicNumber(), atomic_number);

  auto const fractions = composition.GetFractions();
  ASSERT_EQ(fractions.size(), expected.size());

  for (std::size_t index = 0U; index < expected.size(); ++index) {
    EXPECT_EQ(fractions[index].isotope, expected[index].isotope);
    EXPECT_LE(std::abs(fractions[index].fraction - expected[index].fraction),
              k_relative_budget * expected[index].fraction);
  }
}

// =============================================================================
// =============================================================================

auto IsReferenceElement(std::uint32_t atomic_number) -> bool {
  return std::ranges::any_of(
    k_reference_isotopes,
    [atomic_number](materials::GGEMSIsotope const &isotope) -> bool {
      return isotope.GetAtomicNumber() == atomic_number;
    });
}

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSIsotopeProfileTest, ResolvesNist41ValidationFixtures) {
  ExpectProfile(1U, {
                      {.isotope = {1U, 1U, 0U}, .fraction = 0.999885L},
                      {.isotope = {1U, 2U, 0U}, .fraction = 0.000115L},
                    });
  ExpectProfile(5U, {
                      {.isotope = {5U, 10U, 0U}, .fraction = 0.199L},
                      {.isotope = {5U, 11U, 0U}, .fraction = 0.801L},
                    });
  ExpectProfile(6U, {
                      {.isotope = {6U, 12U, 0U}, .fraction = 0.9893L},
                      {.isotope = {6U, 13U, 0U}, .fraction = 0.0107L},
                    });
  ExpectProfile(8U, {
                      {.isotope = {8U, 16U, 0U}, .fraction = 0.99757L},
                      {.isotope = {8U, 17U, 0U}, .fraction = 0.00038L},
                      {.isotope = {8U, 18U, 0U}, .fraction = 0.00205L},
                    });
  ExpectProfile(64U, {
                       {.isotope = {64U, 152U, 0U}, .fraction = 0.0020L},
                       {.isotope = {64U, 154U, 0U}, .fraction = 0.0218L},
                       {.isotope = {64U, 155U, 0U}, .fraction = 0.1480L},
                       {.isotope = {64U, 156U, 0U}, .fraction = 0.2047L},
                       {.isotope = {64U, 157U, 0U}, .fraction = 0.1565L},
                       {.isotope = {64U, 158U, 0U}, .fraction = 0.2484L},
                       {.isotope = {64U, 160U, 0U}, .fraction = 0.2186L},
                     });
}

// =============================================================================
// =============================================================================

TEST(GGEMSIsotopeProfileTest, ResolvesHeavyMultiIsotopeElements) {
  ExpectProfile(82U, {
                       {.isotope = {82U, 204U, 0U}, .fraction = 0.014L},
                       {.isotope = {82U, 206U, 0U}, .fraction = 0.241L},
                       {.isotope = {82U, 207U, 0U}, .fraction = 0.221L},
                       {.isotope = {82U, 208U, 0U}, .fraction = 0.524L},
                     });
  ExpectProfile(92U, {
                       {.isotope = {92U, 234U, 0U}, .fraction = 0.000054L},
                       {.isotope = {92U, 235U, 0U}, .fraction = 0.007204L},
                       {.isotope = {92U, 238U, 0U}, .fraction = 0.992742L},
                     });
}

// =============================================================================
// =============================================================================

TEST(GGEMSIsotopeProfileTest, MapsNaturalTantalum180ToItsIsomer) {
  ExpectProfile(73U, {
                       {.isotope = {73U, 180U, 1U}, .fraction = 0.0001201L},
                       {.isotope = {73U, 181U, 0U}, .fraction = 0.9998799L},
                     });
}

// =============================================================================
// =============================================================================

TEST(GGEMSIsotopeProfileTest, DefaultsAreCanonicalAndMassResolvable) {
  auto const &masses = materials::GetIsotopeMassAuthority();

  std::size_t natural_elements{0U};
  std::size_t natural_isotopes{0U};

  for (std::uint32_t atomic_number = 1U; atomic_number <= 99U;
       ++atomic_number) {
    SCOPED_TRACE(atomic_number);

    auto const composition =
      materials::BuildDefaultIsotopicComposition(atomic_number);
    auto const fractions = composition.GetFractions();

    ASSERT_FALSE(fractions.empty());
    EXPECT_EQ(composition.GetAtomicNumber(), atomic_number);
    EXPECT_EQ(composition.GetBasis(),
              materials::GGEMSFractionBasis::AtomFraction);

    long double fraction_sum{0.0L};

    for (std::size_t index = 0U; index < fractions.size(); ++index) {
      auto const &fraction = fractions[index];

      EXPECT_EQ(fraction.isotope.GetAtomicNumber(), atomic_number);
      EXPECT_GT(fraction.fraction, 0.0L);

      if (index > 0U) {
        EXPECT_LT(fractions[index - 1U].isotope, fraction.isotope);
      }

      EXPECT_NE(masses.Find(fraction.isotope), nullptr);
      fraction_sum += fraction.fraction;
    }

    EXPECT_LE(std::abs(fraction_sum - 1.0L), k_relative_budget);

    if (!IsReferenceElement(atomic_number)) {
      ++natural_elements;
      natural_isotopes += fractions.size();
    }
  }

  EXPECT_EQ(natural_elements, 84U);
  EXPECT_EQ(natural_isotopes, 288U);
}

// =============================================================================
// =============================================================================

TEST(GGEMSIsotopeProfileTest, ResolvesExplicitDefaultReferenceIsotopes) {
  for (auto const &isotope : k_reference_isotopes) {
    auto const atomic_number = isotope.GetAtomicNumber();
    SCOPED_TRACE(atomic_number);

    auto const composition =
      materials::BuildDefaultIsotopicComposition(atomic_number);
    auto const fractions = composition.GetFractions();

    ASSERT_EQ(fractions.size(), 1U);
    EXPECT_EQ(fractions.front().isotope, isotope);
    EXPECT_EQ(fractions.front().fraction, 1.0L);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSIsotopeProfileTest, RejectsElementsOutsideMaterialsDomain) {
  for (std::uint32_t const atomic_number : {0U, 100U, 118U}) {
    SCOPED_TRACE(atomic_number);

    EXPECT_THROW(static_cast<void>(
                   materials::BuildDefaultIsotopicComposition(atomic_number)),
                 ggems::core::GGEMSRecoverable);
  }
}
