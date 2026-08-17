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
 * \brief Unit tests for GGEMS density quantities and conversions.
 *
 * Validates registered density units, finite nonnegative input requirements, fixed grams-per-cubic-centimeter formatting, and canonical density literals.
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
#include "GGEMS/units/GGEMSDensityUnits.hh"
#include "GGEMS/units/GGEMSUnitConversion.hh"
#include "GGEMS/units/GGEMSUnitFormatting.hh"
#include "GGEMSScopedLoggerEncoding.hh"

/// \cond

namespace {

using ggems::test::ScopedLoggerEncoding;
using ggems::units::Density;
using ggems::units::HumanReadable;
using ggems::units::MakeQuantity;
using ggems::units::UnitConversionError;

constexpr long double k_density_relative_tolerance =
    32.0L * std::numeric_limits<long double>::epsilon();

TEST(GGEMSDensityUnitsTest, ConvertsEveryOfficialRuntimeToken) {
  struct Case {
    long double value;
    std::string_view unit;
    long double expected_picograms_per_cubic_picometer;
  };

  constexpr std::array<Case, 2U> cases{{
      {.value = 1.0L,
       .unit = "pg/pm3",
       .expected_picograms_per_cubic_picometer = 1.0L},
      {.value = 1.0L,
       .unit = "g/cm3",
       .expected_picograms_per_cubic_picometer = 1.0e-18L},
  }};

  for (auto const &test_case : cases) {
    SCOPED_TRACE(test_case.unit);
    auto const converted =
        MakeQuantity<Density>(test_case.value, test_case.unit);

    ASSERT_TRUE(converted.has_value());
    long double const tolerance =
        std::abs(test_case.expected_picograms_per_cubic_picometer) *
        k_density_relative_tolerance;
    EXPECT_LE(std::abs(converted->value -
                       test_case.expected_picograms_per_cubic_picometer),
              tolerance);
  }
}

TEST(GGEMSDensityUnitsTest, EnforcesFiniteNonNegativeInput) {
  auto const negative = MakeQuantity<Density>(-1.0L, "pg/pm3");
  auto const infinity = MakeQuantity<Density>(
      std::numeric_limits<long double>::infinity(), "g/cm3");
  auto const not_a_number = MakeQuantity<Density>(
      std::numeric_limits<long double>::quiet_NaN(), "pg/pm3");

  ASSERT_FALSE(negative.has_value());
  ASSERT_FALSE(infinity.has_value());
  ASSERT_FALSE(not_a_number.has_value());
  EXPECT_EQ(negative.error(), UnitConversionError::NegativeValue);
  EXPECT_EQ(infinity.error(), UnitConversionError::NonFinite);
  EXPECT_EQ(not_a_number.error(), UnitConversionError::NonFinite);
}

TEST(GGEMSDensityUnitsTest, UsesFixedGramsPerCubicCentimeterDisplay) {
  Density const density{1.0e-18L};

  {
    ScopedLoggerEncoding const encoding{ggems::core::Encoding::Ascii};
    EXPECT_EQ(HumanReadable(density, 2), "1.00 g/cm3");
  }
  {
    ScopedLoggerEncoding const encoding{ggems::core::Encoding::Unicode};
    EXPECT_EQ(HumanReadable(density, 2), "1.00 g/cm³");
  }
}

TEST(GGEMSDensityUnitsTest, LiteralsStoreCanonicalDensity) {
  using namespace ggems::units;

  constexpr Density canonical = 3_pg_pm3;
  constexpr Density laboratory_density = 2_g_cm3;

  EXPECT_EQ(canonical.value, 3.0L);
  EXPECT_LE(std::abs(laboratory_density.value - 2.0e-18L),
            2.0e-18L * k_density_relative_tolerance);
}

} // namespace
/// \endcond
