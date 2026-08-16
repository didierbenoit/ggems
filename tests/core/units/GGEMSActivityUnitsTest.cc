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
 * \brief Unit tests for GGEMS activity quantities and conversions.
 *
 * Validates becquerel and curie conversions, invalid-input handling,
 * human-readable formatting, activity literals, and aggregate-header exposure.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <array>
#include <limits>
#include <string_view>
#include <type_traits>

#include <gtest/gtest.h>

/// \endcond
#include "GGEMS/core/units/GGEMSActivityUnits.hh"
#include "GGEMS/core/units/GGEMSFrequencyUnits.hh"
#include "GGEMS/core/units/GGEMSUnitConversion.hh"
#include "GGEMS/core/units/GGEMSUnitFormatting.hh"

/// \cond

namespace {

// =============================================================================
// =============================================================================

using ggems::units::Activity;
using ggems::units::Frequency;
using ggems::units::MakeQuantity;
using ggems::units::UnitConversionError;

// =============================================================================
// =============================================================================

static_assert(!std::is_same_v<Activity, Frequency>);

// =============================================================================
// =============================================================================

TEST(GGEMSActivityUnitsTest, ConvertsBecquerelSIPrefixesExactly) {
  struct Case {
    std::string_view unit;
    long double expected_becquerel;
  };

  constexpr std::array<Case, 5> cases{
      {{.unit = "Bq", .expected_becquerel = 1.25L},
       {.unit = "kBq", .expected_becquerel = 1.25e3L},
       {.unit = "MBq", .expected_becquerel = 1.25e6L},
       {.unit = "GBq", .expected_becquerel = 1.25e9L},
       {.unit = "TBq", .expected_becquerel = 1.25e12L}}};

  for (Case const &test_case : cases) {
    SCOPED_TRACE(test_case.unit);
    auto const converted = MakeQuantity<Activity>(1.25L, test_case.unit);
    ASSERT_TRUE(converted.has_value());
    EXPECT_EQ(converted->value, test_case.expected_becquerel);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSActivityUnitsTest, ConvertsCurieFamilyExactly) {
  auto const curie = MakeQuantity<Activity>(1.0L, "Ci");
  auto const millicurie = MakeQuantity<Activity>(1.0L, "mCi");
  auto const microcurie_ascii = MakeQuantity<Activity>(1.0L, "uCi");

  ASSERT_TRUE(curie.has_value());
  ASSERT_TRUE(millicurie.has_value());
  ASSERT_TRUE(microcurie_ascii.has_value());
  EXPECT_EQ(curie->value, 37'000'000'000.0L);
  EXPECT_EQ(millicurie->value, 37'000'000.0L);
  EXPECT_EQ(microcurie_ascii->value, 37'000.0L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSActivityUnitsTest, AcceptsZeroActivity) {
  auto const converted = MakeQuantity<Activity>(0.0L, "TBq");

  ASSERT_TRUE(converted.has_value());
  EXPECT_EQ(converted->value, 0.0L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSActivityUnitsTest, RejectsNegativeActivity) {
  auto const converted = MakeQuantity<Activity>(-1.0L, "Bq");

  ASSERT_FALSE(converted.has_value());
  EXPECT_EQ(converted.error(), UnitConversionError::NegativeValue);
}

// =============================================================================
// =============================================================================

TEST(GGEMSActivityUnitsTest, RejectsNonFiniteActivity) {
  for (long double value : {std::numeric_limits<long double>::quiet_NaN(),
                            std::numeric_limits<long double>::infinity(),
                            -std::numeric_limits<long double>::infinity()}) {
    auto const converted = MakeQuantity<Activity>(value, "Bq");

    ASSERT_FALSE(converted.has_value());
    EXPECT_EQ(converted.error(), UnitConversionError::NonFinite);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSActivityUnitsTest, RejectsUnsupportedUnit) {
  auto const converted = MakeQuantity<Activity>(1.0L, "dpm");

  ASSERT_FALSE(converted.has_value());
  EXPECT_EQ(converted.error(), UnitConversionError::UnsupportedUnit);
}

// =============================================================================
// =============================================================================

TEST(GGEMSActivityUnitsTest, RejectsConversionOverflow) {
  auto const converted =
      MakeQuantity<Activity>(std::numeric_limits<long double>::max(), "TBq");

  ASSERT_FALSE(converted.has_value());
  EXPECT_EQ(converted.error(), UnitConversionError::OutOfRange);
}

// =============================================================================
// =============================================================================

TEST(GGEMSActivityUnitsTest, HumanReadableUsesBecquerelPrefixes) {
  EXPECT_EQ(ggems::units::HumanReadable(Activity{999.0L}, 2), "999.00 Bq");
  EXPECT_EQ(ggems::units::HumanReadable(Activity{1'250.0L}, 2), "1.25 kBq");
  EXPECT_EQ(ggems::units::HumanReadable(Activity{2.5e6L}, 1), "2.5 MBq");
  EXPECT_EQ(ggems::units::HumanReadable(Activity{3.0e9L}, 0), "3 GBq");
  EXPECT_EQ(ggems::units::HumanReadable(Activity{4.0e12L}, 3), "4.000 TBq");
}

// =============================================================================
// =============================================================================

TEST(GGEMSActivityUnitsTest, HumanReadableSupportsMinimumWidth) {
  EXPECT_EQ(ggems::units::HumanReadable(Activity{1'240.0L}, 1, 6),
            "   1.2 kBq");
}

// =============================================================================
// =============================================================================

TEST(GGEMSActivityUnitsTest, LiteralsStoreBecquerels) {
  using namespace ggems::units;

  constexpr Activity becquerel = 2_Bq;
  constexpr Activity kilobecquerel = 2.5_kBq;
  constexpr Activity megabecquerel = 3_MBq;
  constexpr Activity gigabecquerel = 4_GBq;
  constexpr Activity terabecquerel = 5_TBq;
  constexpr Activity curie = 1_Ci;
  constexpr Activity millicurie = 2_mCi;
  constexpr Activity microcurie = 3_uCi;

  EXPECT_EQ(becquerel.value, 2.0L);
  EXPECT_EQ(kilobecquerel.value, 2'500.0L);
  EXPECT_EQ(megabecquerel.value, 3.0e6L);
  EXPECT_EQ(gigabecquerel.value, 4.0e9L);
  EXPECT_EQ(terabecquerel.value, 5.0e12L);
  EXPECT_EQ(curie.value, 37'000'000'000.0L);
  EXPECT_EQ(millicurie.value, 74'000'000.0L);
  EXPECT_EQ(microcurie.value, 111'000.0L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSActivityUnitsTest, AggregateHeaderExposesActivity) {
  ggems::units::Activity const activity{42.0L};
  EXPECT_EQ(activity.value, 42.0L);
}

} // namespace
/// \endcond
