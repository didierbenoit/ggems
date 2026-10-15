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

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string_view>
#include <type_traits>

#include <gtest/gtest.h>

#include "GGEMS/units/GGEMSActivityUnits.hh"
#include "GGEMS/units/GGEMSFrequencyUnits.hh"
#include "GGEMS/units/GGEMSUnitConversion.hh"
#include "GGEMS/units/GGEMSUnitFormatting.hh"

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
static_assert(std::is_same_v<Activity::representation, std::uint64_t>);

// =============================================================================
// =============================================================================

TEST(GGEMSActivityUnitsTest, ConvertsBecquerelSIPrefixesExactly) {
  struct Case {
    std::string_view unit;
    std::uint64_t expected_millibecquerel;
  };

  constexpr std::array<Case, 5> cases{
    {
      {.unit = "Bq", .expected_millibecquerel = 1'250ULL},
      {.unit = "kBq", .expected_millibecquerel = 1'250'000ULL},
      {.unit = "MBq", .expected_millibecquerel = 1'250'000'000ULL},
      {.unit = "GBq", .expected_millibecquerel = 1'250'000'000'000ULL},
      {.unit = "TBq", .expected_millibecquerel = 1'250'000'000'000'000ULL},
    },
  };

  for (Case const &test_case : cases) {
    SCOPED_TRACE(test_case.unit);
    auto const converted = MakeQuantity<Activity>(1.25L, test_case.unit);
    ASSERT_TRUE(converted.has_value());
    EXPECT_EQ(converted->value, test_case.expected_millibecquerel);
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
  EXPECT_EQ(curie->value, 37'000'000'000'000ULL);
  EXPECT_EQ(millicurie->value, 37'000'000'000ULL);
  EXPECT_EQ(microcurie_ascii->value, 37'000'000ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSActivityUnitsTest, AcceptsZeroActivity) {
  auto const converted = MakeQuantity<Activity>(0.0L, "TBq");

  ASSERT_TRUE(converted.has_value());
  EXPECT_EQ(converted->value, 0ULL);
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
  for (long double value : {
         std::numeric_limits<long double>::quiet_NaN(),
         std::numeric_limits<long double>::infinity(),
         -std::numeric_limits<long double>::infinity(),
       }) {
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
  EXPECT_EQ(ggems::units::HumanReadable(Activity{999'000ULL}, 2), "999.00 Bq");
  EXPECT_EQ(ggems::units::HumanReadable(Activity{1'250'000ULL}, 2), "1.25 kBq");
  EXPECT_EQ(ggems::units::HumanReadable(Activity{2'500'000'000ULL}, 1),
            "2.5 MBq");
  EXPECT_EQ(ggems::units::HumanReadable(Activity{3'000'000'000'000ULL}, 0),
            "3 GBq");
  EXPECT_EQ(ggems::units::HumanReadable(Activity{4'000'000'000'000'000ULL}, 3),
            "4.000 TBq");
}

// =============================================================================
// =============================================================================

TEST(GGEMSActivityUnitsTest, HumanReadableSupportsMinimumWidth) {
  EXPECT_EQ(ggems::units::HumanReadable(Activity{1'240'000ULL}, 1, 6),
            "   1.2 kBq");
}

// =============================================================================
// =============================================================================

TEST(GGEMSActivityUnitsTest, LiteralsStoreIntegerMillibecquerels) {
  using namespace ggems::units;

  constexpr Activity becquerel = 2_Bq;
  constexpr Activity kilobecquerel = 2.5_kBq;
  constexpr Activity megabecquerel = 3_MBq;
  constexpr Activity gigabecquerel = 4_GBq;
  constexpr Activity terabecquerel = 5_TBq;
  constexpr Activity curie = 1_Ci;
  constexpr Activity millicurie = 2_mCi;
  constexpr Activity microcurie = 3_uCi;

  EXPECT_EQ(becquerel.value, 2'000ULL);
  EXPECT_EQ(kilobecquerel.value, 2'500'000ULL);
  EXPECT_EQ(megabecquerel.value, 3'000'000'000ULL);
  EXPECT_EQ(gigabecquerel.value, 4'000'000'000'000ULL);
  EXPECT_EQ(terabecquerel.value, 5'000'000'000'000'000ULL);
  EXPECT_EQ(curie.value, 37'000'000'000'000ULL);
  EXPECT_EQ(millicurie.value, 74'000'000'000ULL);
  EXPECT_EQ(microcurie.value, 111'000'000ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSActivityUnitsTest, DirectConstructionUsesCanonicalMillibecquerels) {
  ggems::units::Activity const activity{42ULL};
  EXPECT_EQ(activity.value, 42ULL);
}

TEST(GGEMSActivityUnitsTest,
     QuantizesMillibecquerelsAndRejectsTinyNegativeInput) {
  using namespace ggems::units;
  constexpr std::array input{0.49L, 0.5L, 1.5L};
  constexpr std::array<std::uint64_t, 3> expected{0, 1, 2};
  for (std::size_t index = 0; index < input.size(); ++index) {
    auto const activity = MakeQuantity<Activity>(input[index], "mBq");
    ASSERT_TRUE(activity.has_value());
    EXPECT_EQ(activity->value, expected[index]);
  }
  auto const negative = MakeQuantity<Activity>(-0.001L, "mBq");
  ASSERT_FALSE(negative.has_value());
  EXPECT_EQ(negative.error(), UnitConversionError::NegativeValue);
  EXPECT_EQ(MakeQuantity<Activity>(0.00049L, "Bq").value().value, 0ULL);
  EXPECT_EQ(MakeQuantity<Activity>(0.00051L, "Bq").value().value, 1ULL);
  static_assert((1_mBq).value == 1);
  static_assert((0.5_mBq).value == 1);
}

TEST(GGEMSActivityUnitsTest,
     PreservesExactCanonicalRangeAndIntegralConversions) {
  using namespace ggems::units;
  auto const maximum =
    MakeQuantity<Activity>(std::numeric_limits<std::uint64_t>::max(), "mBq");
  ASSERT_TRUE(maximum.has_value());
  EXPECT_EQ(maximum->value, std::numeric_limits<std::uint64_t>::max());
  EXPECT_EQ(ConvertTo<std::uint64_t>(*maximum, "mBq").value(),
            std::numeric_limits<std::uint64_t>::max());
  EXPECT_EQ(MakeQuantity<Activity>(1, "Bq").value().value, 1'000ULL);
  EXPECT_EQ(MakeQuantity<Activity>(1, "MBq").value().value, 1'000'000'000ULL);
  EXPECT_EQ(MakeQuantity<Activity>(1, "Ci").value().value,
            37'000'000'000'000ULL);
  EXPECT_EQ(ConvertTo<std::uint64_t>(1_Ci, "Bq").value(), 37'000'000'000ULL);
  EXPECT_EQ(ConvertTo(1_mBq, "Bq").value(), 0.001L);
  auto const inexact = ConvertTo<std::uint64_t>(1_mBq, "Bq");
  ASSERT_FALSE(inexact.has_value());
  EXPECT_EQ(inexact.error(), UnitConversionError::InexactConversion);
  auto const overflow =
    MakeQuantity<Activity>(std::numeric_limits<std::uint64_t>::max(), "Bq");
  ASSERT_FALSE(overflow.has_value());
  EXPECT_EQ(overflow.error(), UnitConversionError::OutOfRange);
  auto const floating_overflow = MakeQuantity<Activity>(0x1p64L, "mBq");
  ASSERT_FALSE(floating_overflow.has_value());
  EXPECT_EQ(floating_overflow.error(), UnitConversionError::OutOfRange);
}

TEST(GGEMSActivityUnitsTest,
     PreservesBecquerelDisplayForZeroAndSubBecquerelValues) {
  using namespace ggems::units;
  EXPECT_EQ(HumanReadable(0_mBq, 3), "0.000 Bq");
  EXPECT_EQ(HumanReadable(1_mBq, 3), "0.001 Bq");
  for (auto const *unit : {"mbq", "MBQ", " mBq", "mBq "}) {
    auto const activity = MakeQuantity<Activity>(1, unit);
    ASSERT_FALSE(activity.has_value());
    EXPECT_EQ(activity.error(), UnitConversionError::UnsupportedUnit);
  }
}

} // namespace
