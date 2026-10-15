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
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <format>
#include <limits>
#include <numbers>
#include <type_traits>

#include <gtest/gtest.h>

#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/units/GGEMSUnitConversion.hh"
#include "GGEMS/units/GGEMSUnitFormatting.hh"

namespace {

// Bounds below describe only rounding to the canonical one-picoradian grid.
constexpr long double k_half_quantum_rad{0.5e-12L};
constexpr long double k_pi_reference{std::numbers::pi_v<long double>};
constexpr long double k_half_quantum_deg{0.5e-12L * 180.0L / k_pi_reference};

static_assert(
  std::is_same_v<ggems::units::Angle::representation, std::int64_t>);

// =============================================================================
// =============================================================================

auto ExpectNearLongDouble(long double value, long double reference,
                          long double tolerance = k_half_quantum_rad) -> void {
  EXPECT_LE(std::abs(value - reference), tolerance);
}

// =============================================================================
// =============================================================================

TEST(GGEMSAngularUnits, DefaultAngleIsZeroRadians) {
  ggems::units::Angle angle{};
  ExpectNearLongDouble(ggems::units::ToRadians(angle), 0.0L);
  ExpectNearLongDouble(ggems::units::ToDegrees(angle), 0.0L);
}
// =============================================================================
// =============================================================================

TEST(GGEMSAngularUnits, ConvertsEveryRegisteredAngleToken) {
  using namespace ggems::units;

  auto const radians = MakeQuantity<Angle>(2.5L, "rad");
  auto const degrees = MakeQuantity<Angle>(180.0L, "deg");

  ASSERT_TRUE(radians.has_value());
  ASSERT_TRUE(degrees.has_value());
  EXPECT_EQ(radians->value, 2'500'000'000'000LL);
  EXPECT_EQ(degrees->value, 3'141'592'653'590LL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSAngularUnits, MakeRadiansStoresIntegerPicoradians) {
  ggems::units::Angle angle = ggems::units::MakeRadians(2.5L);

  ExpectNearLongDouble(ggems::units::ToRadians(angle), 2.5L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSAngularUnits, MakeDegreesConvertsToRadians) {
  ggems::units::Angle angle = ggems::units::MakeDegrees(180.0L);

  ExpectNearLongDouble(ggems::units::ToRadians(angle),
                       std::numbers::pi_v<long double>);
}

// =============================================================================
// =============================================================================

TEST(GGEMSAngularUnits, RadiansAreConvertedToDegrees) {
  ggems::units::Angle const angle =
    ggems::units::MakeRadians(std::numbers::pi_v<long double>);

  ExpectNearLongDouble(ggems::units::ToDegrees(angle), 180.0L,
                       k_half_quantum_deg);
}

// =============================================================================
// =============================================================================

TEST(GGEMSAngularUnits, DegreeLiteralStoresIntegerPicoradians) {
  using namespace ggems::units;

  Angle angle = 90.0_deg;

  ExpectNearLongDouble(ToRadians(angle),
                       std::numbers::pi_v<long double> / 2.0L);
  ExpectNearLongDouble(ToDegrees(angle), 90.0L, k_half_quantum_deg);
}

// =============================================================================
// =============================================================================

TEST(GGEMSAngularUnits, IntegerDegreeLiteralStoresIntegerPicoradians) {
  using namespace ggems::units;

  Angle angle = 270_deg;

  ExpectNearLongDouble(ToRadians(angle),
                       3.0L * std::numbers::pi_v<long double> / 2.0L);
  ExpectNearLongDouble(ToDegrees(angle), 270.0L, k_half_quantum_deg);
}

// =============================================================================
// =============================================================================

TEST(GGEMSAngularUnits, RadianLiteralStoresIntegerPicoradians) {
  using namespace ggems::units;

  Angle angle = 2.0_rad;

  ExpectNearLongDouble(ToRadians(angle), 2.0L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSAngularUnits, IntegerRadianLiteralStoresIntegerPicoradians) {
  using namespace ggems::units;

  Angle const angle = 2_rad;

  ExpectNearLongDouble(ToRadians(angle), 2.0L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSAngularUnits, SupportsNegativeAngles) {
  using namespace ggems::units;

  Angle angle = -45.0_deg;

  ExpectNearLongDouble(ToDegrees(angle), -45.0L, k_half_quantum_deg);
  ExpectNearLongDouble(ToRadians(angle),
                       -std::numbers::pi_v<long double> / 4.0L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSAngularUnits, AdditionPreservesAngleSemantics) {
  using namespace ggems::units;

  Angle lhs = 30.0_deg;
  Angle rhs = 15.0_deg;

  EXPECT_EQ((lhs + rhs).value, lhs.value + rhs.value);
}

// =============================================================================
// =============================================================================

TEST(GGEMSAngularUnits, SubtractionPreservesAngleSemantics) {
  using namespace ggems::units;

  Angle lhs = 30.0_deg;
  Angle rhs = 15.0_deg;

  EXPECT_EQ((lhs - rhs).value, lhs.value - rhs.value);
}

// =============================================================================
// =============================================================================

TEST(GGEMSAngularUnits, UnaryMinusPreservesAngleSemantics) {
  using namespace ggems::units;

  Angle angle = 30.0_deg;

  EXPECT_EQ((-angle).value, -angle.value);
}

// =============================================================================
// =============================================================================

TEST(GGEMSAngularUnits, QuantizesSubPicoradianRadianInputs) {
  using namespace ggems::units;
  constexpr std::array input{0.49e-12L,  0.51e-12L,  1.49e-12L,  1.51e-12L,
                             -0.49e-12L, -0.51e-12L, -1.49e-12L, -1.51e-12L};
  constexpr std::array<std::int64_t, 8> expected{0, 1, 1, 2, 0, -1, -1, -2};
  for (std::size_t index = 0; index < input.size(); ++index) {
    auto const angle = MakeQuantity<Angle>(input[index], "rad");
    ASSERT_TRUE(angle.has_value());
    EXPECT_EQ(angle->value, expected[index]);
  }
}

TEST(GGEMSAngularUnits, PreservesExactCanonicalIntegerRange) {
  using namespace ggems::units;
  for (auto value : {std::numeric_limits<std::int64_t>::min(),
                     std::numeric_limits<std::int64_t>::max()}) {
    Angle const angle{value};
    EXPECT_EQ(angle.value, value);
  }
  for (long double value : {1.0e7L, -1.0e7L}) {
    auto const angle = MakeQuantity<Angle>(value, "rad");
    ASSERT_FALSE(angle.has_value());
    EXPECT_EQ(angle.error(), UnitConversionError::OutOfRange);
  }
}

TEST(GGEMSAngularUnits, RejectsNonfiniteAndUnsupportedInputs) {
  using namespace ggems::units;
  for (long double value : {std::numeric_limits<long double>::quiet_NaN(),
                            std::numeric_limits<long double>::infinity(),
                            -std::numeric_limits<long double>::infinity()}) {
    auto const angle = MakeQuantity<Angle>(value, "rad");
    ASSERT_FALSE(angle.has_value());
    EXPECT_EQ(angle.error(), UnitConversionError::NonFinite);
    EXPECT_THROW(MakeRadians(value),
                 std::bad_expected_access<UnitConversionError>);
    EXPECT_THROW(MakeDegrees(value),
                 std::bad_expected_access<UnitConversionError>);
  }
  for (auto const *unit : {"prad", "PRAD", " prad", "prad ", "radian"}) {
    auto const angle = MakeQuantity<Angle>(1, unit);
    ASSERT_FALSE(angle.has_value());
    EXPECT_EQ(angle.error(), UnitConversionError::UnsupportedUnit);
  }
  EXPECT_THROW(MakeRadians(1.0e7L),
               std::bad_expected_access<UnitConversionError>);
  EXPECT_THROW(MakeDegrees(1.0e9L),
               std::bad_expected_access<UnitConversionError>);
}

TEST(GGEMSAngularUnits, ConvertsIntegralOutputsOnlyWhenExact) {
  using namespace ggems::units;
  EXPECT_EQ(ConvertTo<std::int64_t>(-2_rad, "rad").value(), -2);
  Angle const one_picoradian{1};
  auto const inexact = ConvertTo<std::int64_t>(one_picoradian, "rad");
  ASSERT_FALSE(inexact.has_value());
  EXPECT_EQ(inexact.error(), UnitConversionError::InexactConversion);
  EXPECT_EQ(ConvertTo(one_picoradian, "rad").value(), 1.0e-12L);
}

TEST(GGEMSAngularUnits, QuantizesEachCardinalAngleIndependently) {
  using namespace ggems::units;
  constexpr std::array degrees{0.0L,   90.0L,  180.0L, 270.0L,
                               360.0L, 450.0L, 720.0L};
  constexpr std::array<std::int64_t, 7> picoradians{0,
                                                    1'570'796'326'795,
                                                    3'141'592'653'590,
                                                    4'712'388'980'385,
                                                    6'283'185'307'180,
                                                    7'853'981'633'974,
                                                    12'566'370'614'359};
  for (std::size_t index = 0; index < degrees.size(); ++index) {
    EXPECT_EQ(MakeDegrees(degrees[index]).value, picoradians[index]);
    EXPECT_EQ(MakeDegrees(-degrees[index]).value, -picoradians[index]);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSAngularUnits, SpaceshipComparisonUsesCanonicalPicoradians) {
  using namespace ggems::units;

  EXPECT_EQ(90.0_deg, MakeRadians(std::numbers::pi_v<long double> / 2.0L));
  EXPECT_LT(45.0_deg, 90.0_deg);
  EXPECT_GT(180.0_deg, 90.0_deg);
}

// =============================================================================
// =============================================================================

TEST(GGEMSAngularUnits, HumanReadableUsesDegreesByDefault) {
  using namespace ggems::units;

  Angle angle = 45.0_deg;

  EXPECT_EQ(HumanReadable(angle), "45.000 deg");
}

// =============================================================================
// =============================================================================

TEST(GGEMSAngularUnits, HumanReadableSupportsPrecision) {
  using namespace ggems::units;

  Angle angle = 12.3456_deg;

  EXPECT_EQ(HumanReadable(angle, 0), "12 deg");
  EXPECT_EQ(HumanReadable(angle, 1), "12.3 deg");
  EXPECT_EQ(HumanReadable(angle, 2), "12.35 deg");
  EXPECT_EQ(HumanReadable(angle, 3), "12.346 deg");
}

// =============================================================================
// =============================================================================

TEST(GGEMSAngularUnits, HumanReadableSupportsWidth) {
  using namespace ggems::units;

  Angle angle = 12.5_deg;

  EXPECT_EQ(HumanReadable(angle, 1, 6), "  12.5 deg");
}

// =============================================================================
// =============================================================================

TEST(GGEMSAngularUnits, StdFormatterUsesDefaultHumanReadableDegrees) {
  using namespace ggems::units;

  Angle angle = 12.3456_deg;

  EXPECT_EQ(std::format("{}", angle), "12.346 deg");
}
} // namespace
