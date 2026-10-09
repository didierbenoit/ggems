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
#include <string_view>

#include <gtest/gtest.h>
#include <gtest/gtest-spi.h>

#include "GGEMSLongDoubleAssertions.hh"

TEST(GGEMSLongDoubleAssertionsTest, PreservesInclusiveAbsoluteTolerance) {
  GGEMS_EXPECT_NEAR_LD(1.0L, 1.25L, 0.25L);
  GGEMS_EXPECT_NEAR_LD(1.25L, 1.0L, 0.25L);
  GGEMS_EXPECT_NEAR_LD(-1.0L, -1.25L, 0.25L);
  GGEMS_EXPECT_NEAR_LD(0.0L, -0.0L, 0.0L);
  EXPECT_NONFATAL_FAILURE(
    GGEMS_EXPECT_NEAR_LD(std::nextafter(1.25L, 2.0L), 1.0L, 0.25L),
    "absolute difference");
}

TEST(GGEMSLongDoubleAssertionsTest, DetectsDifferencesBelowDoubleResolution) {
  if (std::numeric_limits<long double>::digits <=
      std::numeric_limits<double>::digits) {
    GTEST_SKIP() << "long double has no extra precision on this platform.";
  }
  auto const adjacent = std::nextafter(1.0L, 2.0L);
  EXPECT_EQ(static_cast<double>(adjacent), 1.0);
  EXPECT_NONFATAL_FAILURE(GGEMS_EXPECT_NEAR_LD(adjacent, 1.0L, 0.0L),
                          "absolute difference");
  GGEMS_EXPECT_NEAR_LD(adjacent, 1.0L, adjacent - 1.0L);
}

TEST(GGEMSLongDoubleAssertionsTest, PreservesLongDoubleRange) {
  auto const largest = std::numeric_limits<long double>::max();
  auto const smallest = std::numeric_limits<long double>::min();
  GGEMS_EXPECT_NEAR_LD(largest, largest, 0.0L);
  EXPECT_NONFATAL_FAILURE(GGEMS_EXPECT_NEAR_LD(largest, largest / 2.0L, 0.0L),
                          "absolute difference");
  EXPECT_NONFATAL_FAILURE(GGEMS_EXPECT_NEAR_LD(smallest, 0.0L, 0.0L),
                          "absolute difference");
  EXPECT_NONFATAL_FAILURE(GGEMS_EXPECT_NEAR_LD(largest, -largest, largest),
                          "absolute difference");
}

TEST(GGEMSLongDoubleAssertionsTest, RejectsNaNsAndInvalidTolerances) {
  auto const nan_value = std::numeric_limits<long double>::quiet_NaN();
  auto const infinity = std::numeric_limits<long double>::infinity();
  EXPECT_NONFATAL_FAILURE(GGEMS_EXPECT_NEAR_LD(nan_value, 1.0L, infinity),
                          "absolute difference");
  EXPECT_NONFATAL_FAILURE(GGEMS_EXPECT_NEAR_LD(1.0L, nan_value, infinity),
                          "absolute difference");
  EXPECT_NONFATAL_FAILURE(GGEMS_EXPECT_NEAR_LD(nan_value, nan_value, 0.0L),
                          "absolute difference");
  EXPECT_NONFATAL_FAILURE(GGEMS_EXPECT_NEAR_LD(1.0L, 1.0L, nan_value),
                          "absolute difference");
  EXPECT_NONFATAL_FAILURE(GGEMS_EXPECT_NEAR_LD(1.0L, 1.0L, -1.0L),
                          "absolute difference");
  EXPECT_NONFATAL_FAILURE(GGEMS_EXPECT_NEAR_LD(infinity, infinity, -1.0L),
                          "absolute difference");
}

TEST(GGEMSLongDoubleAssertionsTest, HandlesInfinities) {
  auto const infinity = std::numeric_limits<long double>::infinity();
  GGEMS_EXPECT_NEAR_LD(infinity, infinity, 0.0L);
  GGEMS_EXPECT_NEAR_LD(-infinity, -infinity, 0.0L);
  EXPECT_NONFATAL_FAILURE(GGEMS_EXPECT_NEAR_LD(infinity, -infinity, 1.0L),
                          "absolute difference");
  EXPECT_NONFATAL_FAILURE(GGEMS_EXPECT_NEAR_LD(infinity, 1.0L, 1.0L),
                          "absolute difference");
  EXPECT_NONFATAL_FAILURE(GGEMS_EXPECT_NEAR_LD(1.0L, -infinity, 1.0L),
                          "absolute difference");
  GGEMS_EXPECT_NEAR_LD(infinity, -infinity, infinity);
  GGEMS_EXPECT_NEAR_LD(1.0L, infinity, infinity);
  GGEMS_EXPECT_NEAR_LD(-infinity, 1.0L, infinity);
}

TEST(GGEMSLongDoubleAssertionsTest, EvaluatesEachArgumentOnce) {
  int actual_calls = 0;
  int expected_calls = 0;
  int tolerance_calls = 0;
  auto const actual = [&]() -> long double {
    ++actual_calls;
    return 1.0L;
  };
  auto const expected = [&]() -> long double {
    ++expected_calls;
    return 1.0L;
  };
  auto const tolerance = [&]() -> long double {
    ++tolerance_calls;
    return 0.0L;
  };
  GGEMS_EXPECT_NEAR_LD(actual(), expected(), tolerance());
  EXPECT_EQ(actual_calls, 1);
  EXPECT_EQ(expected_calls, 1);
  EXPECT_EQ(tolerance_calls, 1);
}

TEST(GGEMSLongDoubleAssertionsTest,
     ReportsExpressionsValuesAndStreamedContext) {
  auto const result = ggems::test::LongDoubleNear("measured", "reference",
                                                  "budget", 1.0L, 2.0L, 0.25L);
  ASSERT_FALSE(result);
  std::string_view const message{result.message()};
  EXPECT_NE(message.find("measured evaluates to 1"), std::string_view::npos);
  EXPECT_NE(message.find("reference evaluates to 2"), std::string_view::npos);
  EXPECT_NE(message.find("budget evaluates to 0.25"), std::string_view::npos);
  EXPECT_NE(message.find("absolute difference is 1"), std::string_view::npos);
  EXPECT_NONFATAL_FAILURE(GGEMS_EXPECT_NEAR_LD(1.0L, 2.0L, 0.25L)
                            << "comparison context",
                          "comparison context");
}
