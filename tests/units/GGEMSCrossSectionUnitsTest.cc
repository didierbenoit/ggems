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
 * \brief Unit tests for GGEMS cross-section quantities and conversions.
 *
 * Validates supported barn-family units, exact barn-to-picobarn scaling, negative-value rejection, ASCII/Unicode formatting, and convenience literals.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <array>
#include <cstdint>
#include <string_view>

#include <gtest/gtest.h>

/// \endcond
#include "GGEMS/logging/GGEMSLogger.hh"
#include "GGEMS/units/GGEMSCrossSectionUnits.hh"
#include "GGEMS/units/GGEMSUnitConversion.hh"
#include "GGEMS/units/GGEMSUnitFormatting.hh"
#include "GGEMSScopedLoggerEncoding.hh"

/// \cond

namespace {

using ggems::test::ScopedLoggerEncoding;
using ggems::units::CrossSection;
using ggems::units::HumanReadable;
using ggems::units::MakeQuantity;
using ggems::units::UnitConversionError;

struct CrossSectionConversionCase {
  std::string_view unit;
  std::uint64_t expected;
};

TEST(GGEMSCrossSectionUnits, ConvertsEveryOfficialRuntimeToken) {
  constexpr std::array<CrossSectionConversionCase, 6U> cases{{
      {.unit = "pb", .expected = 1ULL},
      {.unit = "nb", .expected = 1'000ULL},
      {.unit = "ub", .expected = 1'000'000ULL},
      {.unit = "mb", .expected = 1'000'000'000ULL},
      {.unit = "barn", .expected = 1'000'000'000'000ULL},
      {.unit = "kbarn", .expected = 1'000'000'000'000'000ULL},
  }};

  for (auto const &test_case : cases) {
    SCOPED_TRACE(test_case.unit);
    auto const converted = MakeQuantity<CrossSection>(1ULL, test_case.unit);

    ASSERT_TRUE(converted.has_value());
    EXPECT_EQ(converted->value, test_case.expected);
  }
}

TEST(GGEMSCrossSectionUnits, BarnHasExactlyOneTrillionPicobarns) {
  auto const barn = MakeQuantity<CrossSection>(1ULL, "barn");

  ASSERT_TRUE(barn.has_value());
  EXPECT_EQ(barn->value, 1'000'000'000'000ULL);
}

TEST(GGEMSCrossSectionUnits, RejectsNegativeValues) {
  auto const converted = MakeQuantity<CrossSection>(-1, "pb");

  ASSERT_FALSE(converted.has_value());
  EXPECT_EQ(converted.error(), UnitConversionError::NegativeValue);
}

TEST(GGEMSCrossSectionUnits, UsesAsciiAndUnicodeMicrobarnDisplay) {
  using namespace ggems::units;

  {
    ScopedLoggerEncoding const encoding{ggems::core::Encoding::Ascii};
    EXPECT_EQ(HumanReadable(1_ub, 2), "1.00 ub");
  }

  {
    ScopedLoggerEncoding const encoding{ggems::core::Encoding::Unicode};
    EXPECT_EQ(HumanReadable(1_ub, 2), "1.00 µb");
  }
}

TEST(GGEMSCrossSectionUnits, ConvenienceBarnLiteralsMatchRuntimeUnits) {
  using namespace ggems::units;

  EXPECT_EQ((2_pbarn).value, (2_pb).value);
  EXPECT_EQ((2_nbarn).value, (2_nb).value);
  EXPECT_EQ((2_ubarn).value, (2_ub).value);
  EXPECT_EQ((2_mbarn).value, (2_mb).value);
}

} // namespace
/// \endcond
