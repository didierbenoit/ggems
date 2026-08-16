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
 * \brief Unit tests for GGEMS area quantities and conversions.
 *
 * Validates registered square-length units, negative-value rejection, automatic ASCII/Unicode formatting, and canonical square-picometer literals.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <array>
#include <cmath>
#include <limits>
#include <string_view>

#include <gtest/gtest.h>

/// \endcond
#include "GGEMS/core/GGEMSLogger.hh"
#include "GGEMS/core/units/GGEMSAreaUnits.hh"
#include "GGEMS/core/units/GGEMSUnitConversion.hh"
#include "GGEMS/core/units/GGEMSUnitFormatting.hh"
#include "GGEMSScopedLoggerEncoding.hh"

/// \cond

namespace {

using ggems::test::ScopedLoggerEncoding;
using ggems::units::Area;
using ggems::units::HumanReadable;
using ggems::units::MakeQuantity;
using ggems::units::UnitConversionError;

struct AreaConversionCase {
  std::string_view unit;
  long double expected;
};

auto ExpectScaleNear(long double actual, long double expected) -> void {
  constexpr long double k_rounding_operations{64.0L};
  long double const tolerance = std::abs(expected) *
                                std::numeric_limits<long double>::epsilon() *
                                k_rounding_operations;
  EXPECT_LE(std::abs(actual - expected), tolerance);
}

TEST(GGEMSAreaUnits, ConvertsEveryOfficialRuntimeToken) {
  constexpr std::array<AreaConversionCase, 7U> cases{{
      {.unit = "pm2", .expected = 1.0L},
      {.unit = "nm2", .expected = 1.0e6L},
      {.unit = "um2", .expected = 1.0e12L},
      {.unit = "mm2", .expected = 1.0e18L},
      {.unit = "cm2", .expected = 1.0e20L},
      {.unit = "m2", .expected = 1.0e24L},
      {.unit = "km2", .expected = 1.0e30L},
  }};

  for (auto const &test_case : cases) {
    SCOPED_TRACE(test_case.unit);
    auto const converted = MakeQuantity<Area>(1.0L, test_case.unit);

    ASSERT_TRUE(converted.has_value());
    ExpectScaleNear(converted->value, test_case.expected);
  }
}

TEST(GGEMSAreaUnits, RejectsNegativeValues) {
  auto const converted = MakeQuantity<Area>(-1.0L, "pm2");

  ASSERT_FALSE(converted.has_value());
  EXPECT_EQ(converted.error(), UnitConversionError::NegativeValue);
}

TEST(GGEMSAreaUnits, UsesSquaredAutomaticAndUnicodeDisplay) {
  using namespace ggems::units;

  {
    ScopedLoggerEncoding const encoding{ggems::core::Encoding::Ascii};
    EXPECT_EQ(HumanReadable(1_cm2, 2), "100.00 mm2");
  }

  {
    ScopedLoggerEncoding const encoding{ggems::core::Encoding::Unicode};
    EXPECT_EQ(HumanReadable(1_um2, 2), "1.00 µm²");
  }
}

TEST(GGEMSAreaUnits, LiteralStoresCanonicalSquarePicometers) {
  using namespace ggems::units;

  ExpectScaleNear((2_um2).value, 2.0e12L);
}

} // namespace
/// \endcond
