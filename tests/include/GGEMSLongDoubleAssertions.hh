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

#pragma once

#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>

#include <gtest/gtest.h>

namespace ggems::test {

// Keep the operands, absolute tolerance, and diagnostics in long double.
[[nodiscard]] inline auto
LongDoubleNear(char const *actual_expression, char const *expected_expression,
               char const *tolerance_expression, long double actual,
               long double expected, long double tolerance)
  -> testing::AssertionResult {
  // Equality handles same-signed infinities without subtracting them.
  // NaN operands and negative or NaN tolerances never compare successfully.
  if (tolerance >= 0.0L &&
      (actual == expected || std::fabs(actual - expected) <= tolerance)) {
    return testing::AssertionSuccess();
  }

  std::ostringstream diagnostic;
  diagnostic << std::setprecision(
                  std::numeric_limits<long double>::max_digits10)
             << "Expected absolute difference <= " << tolerance_expression
             << ", where\n"
             << actual_expression << " evaluates to " << actual << ",\n"
             << expected_expression << " evaluates to " << expected << ",\n"
             << tolerance_expression << " evaluates to " << tolerance << ",\n"
             << "and the absolute difference is "
             << std::fabs(actual - expected) << ".";
  return testing::AssertionFailure() << diagnostic.str();
}

} // namespace ggems::test

#define GGEMS_EXPECT_NEAR_LD(actual, expected, tolerance)                      \
  EXPECT_PRED_FORMAT3(::ggems::test::LongDoubleNear, actual, expected,         \
                      tolerance)
