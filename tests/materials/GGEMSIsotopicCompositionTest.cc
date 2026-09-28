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
#include <cstddef>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/materials/GGEMSIsotope.hh"
#include "GGEMS/materials/GGEMSIsotopicComposition.hh"

namespace {

// =============================================================================
// =============================================================================

namespace materials = ggems::core::materials;

using Basis = materials::GGEMSFractionBasis;

constexpr materials::GGEMSIsotope k_boron_10{5U, 10U, 0U};
constexpr materials::GGEMSIsotope k_boron_11{5U, 11U, 0U};
constexpr materials::GGEMSIsotope k_carbon_12{6U, 12U, 0U};
constexpr materials::GGEMSIsotope k_oxygen_16{8U, 16U, 0U};
constexpr materials::GGEMSIsotope k_oxygen_17{8U, 17U, 0U};
constexpr materials::GGEMSIsotope k_oxygen_18{8U, 18U, 0U};

// =============================================================================
// =============================================================================

auto MakeComposition(Basis basis,
                     std::vector<materials::GGEMSIsotopeFraction> fractions)
  -> materials::GGEMSIsotopicComposition {
  return materials::GGEMSIsotopicComposition{basis, std::move(fractions)};
}

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSIsotopicCompositionTest, CanonicalOrderIsIndependentOfInputOrder) {
  std::vector<materials::GGEMSIsotopeFraction> const entries{
    {.isotope = k_oxygen_18, .fraction = 0.00205L},
    {.isotope = k_oxygen_16, .fraction = 0.99757L},
    {.isotope = k_oxygen_17, .fraction = 0.00038L},
  };

  auto const reference = MakeComposition(Basis::AtomFraction, entries);

  EXPECT_EQ(reference.GetAtomicNumber(), 8U);
  EXPECT_EQ(reference.GetBasis(), Basis::AtomFraction);

  auto const fractions = reference.GetFractions();
  ASSERT_EQ(fractions.size(), 3U);
  EXPECT_EQ(fractions[0].isotope, k_oxygen_16);
  EXPECT_EQ(fractions[1].isotope, k_oxygen_17);
  EXPECT_EQ(fractions[2].isotope, k_oxygen_18);

  std::array<std::size_t, 3U> order{0U, 1U, 2U};
  do {
    std::vector<materials::GGEMSIsotopeFraction> permuted;
    permuted.reserve(order.size());
    for (auto const index : order) {
      permuted.push_back(entries[index]);
    }

    EXPECT_EQ(MakeComposition(Basis::AtomFraction, std::move(permuted)),
              reference);
  } while (std::ranges::next_permutation(order).found);
}

// =============================================================================
// =============================================================================

TEST(GGEMSIsotopicCompositionTest, BasisIsPartOfTheComposition) {
  auto const atom = MakeComposition(Basis::AtomFraction,
                                    {
                                      {.isotope = k_boron_10, .fraction = 0.9L},
                                      {.isotope = k_boron_11, .fraction = 0.1L},
                                    });

  auto const mass = MakeComposition(Basis::MassFraction,
                                    {
                                      {.isotope = k_boron_10, .fraction = 0.9L},
                                      {.isotope = k_boron_11, .fraction = 0.1L},
                                    });

  EXPECT_EQ(mass.GetBasis(), Basis::MassFraction);
  EXPECT_TRUE(std::ranges::equal(atom.GetFractions(), mass.GetFractions()));
  EXPECT_NE(atom, mass);
}

// =============================================================================
// =============================================================================

TEST(GGEMSIsotopicCompositionTest, NormalizesAdmittedSumOnce) {
  long double const scale = 1.0L + 0x1p-18L;

  auto const composition = MakeComposition(
    Basis::MassFraction, {
                           {.isotope = k_boron_11, .fraction = 0.625L * scale},
                           {.isotope = k_boron_10, .fraction = 0.375L * scale},
                         });

  auto const fractions = composition.GetFractions();
  ASSERT_EQ(fractions.size(), 2U);
  EXPECT_EQ(fractions[0].isotope, k_boron_10);
  EXPECT_EQ(fractions[0].fraction, 0.375L);
  EXPECT_EQ(fractions[1].isotope, k_boron_11);
  EXPECT_EQ(fractions[1].fraction, 0.625L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSIsotopicCompositionTest, RemovesExplicitZeroEntries) {
  auto const with_zero = MakeComposition(
    Basis::AtomFraction, {
                           {.isotope = k_boron_11, .fraction = 0.0L},
                           {.isotope = k_boron_10, .fraction = 1.0L},
                         });

  auto const without_zero = MakeComposition(
    Basis::AtomFraction, {{.isotope = k_boron_10, .fraction = 1.0L}});

  ASSERT_EQ(with_zero.GetFractions().size(), 1U);
  EXPECT_EQ(with_zero, without_zero);
}

// =============================================================================
// =============================================================================

TEST(GGEMSIsotopicCompositionTest, AppliesProvisionalSumAdmissionPolicy) {
  auto const make_single =
    [](long double fraction) -> materials::GGEMSIsotopicComposition {
    return MakeComposition(Basis::AtomFraction,
                           {{.isotope = k_boron_10, .fraction = fraction}});
  };

  EXPECT_NO_THROW(static_cast<void>(make_single(1.0L + 0.9e-5L)));
  EXPECT_NO_THROW(static_cast<void>(make_single(1.0L - 0.9e-5L)));
  EXPECT_THROW(static_cast<void>(make_single(1.0L + 1.1e-5L)),
               ggems::core::GGEMSRecoverable);
  EXPECT_THROW(static_cast<void>(make_single(1.0L - 1.1e-5L)),
               ggems::core::GGEMSRecoverable);
}

// =============================================================================
// =============================================================================

TEST(GGEMSIsotopicCompositionTest, RejectsInvalidKeysAndFractionSums) {
  auto const expect_rejected =
    [](std::vector<materials::GGEMSIsotopeFraction> fractions) -> void {
    EXPECT_THROW(static_cast<void>(
                   MakeComposition(Basis::AtomFraction, std::move(fractions))),
                 ggems::core::GGEMSRecoverable);
  };

  // Empty or all-zero composition.
  expect_rejected({});
  expect_rejected({
    {.isotope = k_boron_10, .fraction = 0.0L},
    {.isotope = k_boron_11, .fraction = 0.0L},
  });

  // Duplicate isotope keys, including a zero-fraction duplicate.
  expect_rejected({
    {.isotope = k_boron_10, .fraction = 0.5L},
    {.isotope = k_boron_10, .fraction = 0.5L},
  });
  expect_rejected({
    {.isotope = k_boron_10, .fraction = 1.0L},
    {.isotope = k_boron_10, .fraction = 0.0L},
  });

  // Mixed elements, including a zero-fraction foreign element.
  expect_rejected({
    {.isotope = k_boron_10, .fraction = 0.5L},
    {.isotope = k_carbon_12, .fraction = 0.5L},
  });
  expect_rejected({
    {.isotope = k_boron_10, .fraction = 1.0L},
    {.isotope = k_carbon_12, .fraction = 0.0L},
  });

  // Fraction sums outside the admission tolerance.
  expect_rejected({
    {.isotope = k_boron_10, .fraction = 0.9L},
  });
  expect_rejected({
    {.isotope = k_boron_10, .fraction = 0.6L},
    {.isotope = k_boron_11, .fraction = 0.5L},
  });
}
