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
 * \brief Unit tests for GGEMS angular quantities and arithmetic.
 *
 * Validates registered angle units, radian/degree helpers and literals, signed angles, arithmetic and comparisons, and human-readable/formatter behavior.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <cmath>
#include <format>
#include <limits>

#include <gtest/gtest.h>

/// \endcond
#include "GGEMS/core/units/GGEMSAngularUnits.hh"
#include "GGEMS/core/units/GGEMSUnitConversion.hh"
#include "GGEMS/core/units/GGEMSUnitFormatting.hh"

/// \cond

namespace {

constexpr long double k_tolerance{1.0e-12L};
constexpr long double k_pi_reference{3.141592653589793238462643383279502884L};
constexpr long double k_rounding_error_bound{8.0L};
constexpr long double k_registered_token_tolerance{
    k_rounding_error_bound * k_pi_reference *
    std::numeric_limits<long double>::epsilon()};

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void ExpectNearLongDouble(long double value, long double reference,
                          long double tolerance = k_tolerance) {
  EXPECT_LE(std::abs(value - reference), tolerance);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

TEST(GGEMSAngularUnits, DefaultAngleIsZeroRadians) {
  ggems::units::Angle angle{};
  ExpectNearLongDouble(ggems::units::ToRadians(angle), 0.0L);
  ExpectNearLongDouble(ggems::units::ToDegrees(angle), 0.0L);
}

TEST(GGEMSAngularUnits, ConvertsEveryRegisteredAngleToken) {
  using namespace ggems::units;

  auto const radians = MakeQuantity<Angle>(2.5L, "rad");
  auto const degrees = MakeQuantity<Angle>(180.0L, "deg");

  ASSERT_TRUE(radians.has_value());
  ASSERT_TRUE(degrees.has_value());
  EXPECT_EQ(radians->value, 2.5L);
  ExpectNearLongDouble(degrees->value, k_pi_reference,
                       k_registered_token_tolerance);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

TEST(GGEMSAngularUnits, MakeRadiansStoresRadiansDirectly) {
  ggems::units::Angle angle = ggems::units::MakeRadians(2.5L);

  ExpectNearLongDouble(ggems::units::ToRadians(angle), 2.5L);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

TEST(GGEMSAngularUnits, MakeDegreesConvertsToRadians) {
  ggems::units::Angle angle = ggems::units::MakeDegrees(180.0L);

  ExpectNearLongDouble(ggems::units::ToRadians(angle),
                       ggems::units::detail::k_pi);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

TEST(GGEMSAngularUnits, RadiansAreConvertedToDegrees) {
  ggems::units::Angle const angle =
      ggems::units::MakeRadians(ggems::units::detail::k_pi);

  ExpectNearLongDouble(ggems::units::ToDegrees(angle), 180.0L);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

TEST(GGEMSAngularUnits, DegreeLiteralStoresRadiansInternally) {
  using namespace ggems::units;

  Angle angle = 90.0_deg;

  ExpectNearLongDouble(ToRadians(angle), detail::k_pi / 2.0L);
  ExpectNearLongDouble(ToDegrees(angle), 90.0L);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

TEST(GGEMSAngularUnits, IntegerDegreeLiteralStoresRadiansInternally) {
  using namespace ggems::units;

  Angle angle = 270_deg;

  ExpectNearLongDouble(ToRadians(angle), 3.0L * detail::k_pi / 2.0L);
  ExpectNearLongDouble(ToDegrees(angle), 270.0L);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

TEST(GGEMSAngularUnits, RadianLiteralStoresRadiansDirectly) {
  using namespace ggems::units;

  Angle angle = 2.0_rad;

  ExpectNearLongDouble(ToRadians(angle), 2.0L);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

TEST(GGEMSAngularUnits, IntegerRadianLiteralStoresRadiansDirectly) {
  using namespace ggems::units;

  Angle const angle = 2_rad;

  ExpectNearLongDouble(ToRadians(angle), 2.0L);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

TEST(GGEMSAngularUnits, SupportsNegativeAngles) {
  using namespace ggems::units;

  Angle angle = -45.0_deg;

  ExpectNearLongDouble(ToDegrees(angle), -45.0L);
  ExpectNearLongDouble(ToRadians(angle), -detail::k_pi / 4.0L);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

TEST(GGEMSAngularUnits, AdditionPreservesAngleSemantics) {
  using namespace ggems::units;

  Angle lhs = 30.0_deg;
  Angle rhs = 15.0_deg;

  ExpectNearLongDouble(ToDegrees(lhs + rhs), 45.0L);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

TEST(GGEMSAngularUnits, SubtractionPreservesAngleSemantics) {
  using namespace ggems::units;

  Angle lhs = 30.0_deg;
  Angle rhs = 15.0_deg;

  ExpectNearLongDouble(ToDegrees(lhs - rhs), 15.0L);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

TEST(GGEMSAngularUnits, UnaryMinusPreservesAngleSemantics) {
  using namespace ggems::units;

  Angle angle = 30.0_deg;

  ExpectNearLongDouble(ToDegrees(-angle), -30.0L);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

TEST(GGEMSAngularUnits, MultiplicationByScalarPreservesAngleSemantics) {
  using namespace ggems::units;

  Angle angle = 30.0_deg;

  ExpectNearLongDouble(ToDegrees(angle * 2.0L), 60.0L);
  ExpectNearLongDouble(ToDegrees(2.0L * angle), 60.0L);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

TEST(GGEMSAngularUnits, DivisionByScalarPreservesAngleSemantics) {
  using namespace ggems::units;

  Angle angle = 30.0_deg;

  ExpectNearLongDouble(ToDegrees(angle / 2.0L), 15.0L);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

TEST(GGEMSAngularUnits, SpaceshipComparisonUsesRadians) {
  using namespace ggems::units;

  EXPECT_EQ(90.0_deg, MakeRadians(detail::k_pi / 2.0L));
  EXPECT_LT(45.0_deg, 90.0_deg);
  EXPECT_GT(180.0_deg, 90.0_deg);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

TEST(GGEMSAngularUnits, HumanReadableUsesDegreesByDefault) {
  using namespace ggems::units;

  Angle angle = 45.0_deg;

  EXPECT_EQ(HumanReadable(angle), "45.000 deg");
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

TEST(GGEMSAngularUnits, HumanReadableSupportsPrecision) {
  using namespace ggems::units;

  Angle angle = 12.3456_deg;

  EXPECT_EQ(HumanReadable(angle, 0), "12 deg");
  EXPECT_EQ(HumanReadable(angle, 1), "12.3 deg");
  EXPECT_EQ(HumanReadable(angle, 2), "12.35 deg");
  EXPECT_EQ(HumanReadable(angle, 3), "12.346 deg");
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

TEST(GGEMSAngularUnits, HumanReadableSupportsWidth) {
  using namespace ggems::units;

  Angle angle = 12.5_deg;

  EXPECT_EQ(HumanReadable(angle, 1, 6), "  12.5 deg");
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

TEST(GGEMSAngularUnits, StdFormatterUsesDefaultHumanReadableDegrees) {
  using namespace ggems::units;

  Angle angle = 12.3456_deg;

  EXPECT_EQ(std::format("{}", angle), "12.346 deg");
}
} // namespace
/// \endcond
