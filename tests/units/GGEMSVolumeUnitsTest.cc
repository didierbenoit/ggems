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
 * \brief Unit tests for GGEMS volume quantities and conversions.
 *
 * Validates registered cubic-length units, negative-value rejection, automatic ASCII/Unicode cubic formatting, and canonical cubic-picometer literals.
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
#include "GGEMS/logging/GGEMSLogger.hh"
#include "GGEMS/units/GGEMSUnitConversion.hh"
#include "GGEMS/units/GGEMSUnitFormatting.hh"
#include "GGEMS/units/GGEMSVolumeUnits.hh"
#include "GGEMSScopedLoggerEncoding.hh"

/// \cond

namespace {

using ggems::test::ScopedLoggerEncoding;
using ggems::units::HumanReadable;
using ggems::units::MakeQuantity;
using ggems::units::UnitConversionError;
using ggems::units::Volume;

constexpr long double k_volume_relative_tolerance =
    64.0L * std::numeric_limits<long double>::epsilon();

TEST(GGEMSVolumeUnitsTest, ConvertsEveryOfficialRuntimeToken) {
  struct Case {
    std::string_view unit;
    long double expected_cubic_picometers;
  };

  constexpr std::array<Case, 7U> cases{{
      {.unit = "pm3", .expected_cubic_picometers = 1.0L},
      {.unit = "nm3", .expected_cubic_picometers = 1.0e9L},
      {.unit = "um3", .expected_cubic_picometers = 1.0e18L},
      {.unit = "mm3", .expected_cubic_picometers = 1.0e27L},
      {.unit = "cm3", .expected_cubic_picometers = 1.0e30L},
      {.unit = "m3", .expected_cubic_picometers = 1.0e36L},
      {.unit = "km3", .expected_cubic_picometers = 1.0e45L},
  }};

  for (auto const &test_case : cases) {
    SCOPED_TRACE(test_case.unit);
    auto const converted = MakeQuantity<Volume>(1.0L, test_case.unit);

    ASSERT_TRUE(converted.has_value());
    long double const tolerance =
        std::abs(test_case.expected_cubic_picometers) *
        k_volume_relative_tolerance;
    EXPECT_LE(std::abs(converted->value - test_case.expected_cubic_picometers),
              tolerance);
  }
}

TEST(GGEMSVolumeUnitsTest, RejectsNegativeVolume) {
  auto const converted = MakeQuantity<Volume>(-1.0L, "cm3");

  ASSERT_FALSE(converted.has_value());
  EXPECT_EQ(converted.error(), UnitConversionError::NegativeValue);
}

TEST(GGEMSVolumeUnitsTest, AutomaticallyDisplaysCubicUnits) {
  {
    ScopedLoggerEncoding const encoding{ggems::core::Encoding::Ascii};
    EXPECT_EQ(HumanReadable(Volume{1.0e18L}, 2), "1.00 um3");
    EXPECT_EQ(HumanReadable(Volume{1.0e30L}, 2), "1000.00 mm3");
  }
  {
    ScopedLoggerEncoding const encoding{ggems::core::Encoding::Unicode};
    EXPECT_EQ(HumanReadable(Volume{1.0e18L}, 2), "1.00 µm³");
  }
}

TEST(GGEMSVolumeUnitsTest, LiteralStoresCubicPicometers) {
  using namespace ggems::units;

  constexpr Volume volume = 2_um3;

  EXPECT_LE(std::abs(volume.value - 2.0e18L),
            2.0e18L * k_volume_relative_tolerance);
}

} // namespace
/// \endcond
