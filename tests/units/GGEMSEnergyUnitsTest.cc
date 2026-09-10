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
 * \brief Unit tests for GGEMS energy quantities and conversions.
 *
 * Validates registered energy units, nonnegative energy values, signed energy changes and rounding, automatic display-unit selection, and energy literals.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <format>
#include <limits>
#include <string_view>

#include <gtest/gtest.h>

/// \endcond
#include "GGEMS/logging/GGEMSLogger.hh"
#include "GGEMS/units/GGEMSEnergyUnits.hh"
#include "GGEMS/units/GGEMSUnitConversion.hh"
#include "GGEMS/units/GGEMSUnitFormatting.hh"
#include "GGEMSScopedLoggerEncoding.hh"

/// \cond

namespace {

using ggems::test::ScopedLoggerEncoding;
using ggems::units::ConvertTo;
using ggems::units::Energy;
using ggems::units::EnergyChange;
using ggems::units::HumanReadable;
using ggems::units::MakeQuantity;
using ggems::units::UnitConversionError;

TEST(GGEMSEnergyUnitsTest, ConvertsEveryOfficialRuntimeToken) {
  struct Case {
    std::string_view unit;
    std::uint64_t expected_micro_electron_volts;
  };

  constexpr std::array<Case, 6U> cases{{
      {.unit = "meV", .expected_micro_electron_volts = 1'000ULL},
      {.unit = "eV", .expected_micro_electron_volts = 1'000'000ULL},
      {.unit = "keV", .expected_micro_electron_volts = 1'000'000'000ULL},
      {.unit = "MeV", .expected_micro_electron_volts = 1'000'000'000'000ULL},
      {.unit = "GeV",
       .expected_micro_electron_volts = 1'000'000'000'000'000ULL},
      {.unit = "TeV",
       .expected_micro_electron_volts = 1'000'000'000'000'000'000ULL},
  }};

  for (auto const &test_case : cases) {
    SCOPED_TRACE(test_case.unit);
    auto const converted = MakeQuantity<Energy>(1, test_case.unit);

    ASSERT_TRUE(converted.has_value());
    EXPECT_EQ(converted->value, test_case.expected_micro_electron_volts);
  }
}

TEST(GGEMSEnergyUnitsTest, RejectsNegativeEnergy) {
  auto const converted = MakeQuantity<Energy>(-1, "eV");

  ASSERT_FALSE(converted.has_value());
  EXPECT_EQ(converted.error(), UnitConversionError::NegativeValue);
}

TEST(GGEMSEnergyUnitsTest, EnergyChangeSupportsSignedValuesAndRounding) {
  auto const positive = MakeQuantity<EnergyChange>(1.5L, "eV");
  auto const negative = MakeQuantity<EnergyChange>(-1.5L, "eV");
  auto const positive_half = MakeQuantity<EnergyChange>(0.0005L, "meV");
  auto const negative_half = MakeQuantity<EnergyChange>(-0.0005L, "meV");

  ASSERT_TRUE(positive.has_value());
  ASSERT_TRUE(negative.has_value());
  ASSERT_TRUE(positive_half.has_value());
  ASSERT_TRUE(negative_half.has_value());
  EXPECT_EQ(positive->value, 1'500'000LL);
  EXPECT_EQ(negative->value, -1'500'000LL);
  EXPECT_EQ(positive_half->value, 1LL);
  EXPECT_EQ(negative_half->value, -1LL);
}

TEST(GGEMSEnergyUnitsTest, AutomaticallyDisplaysTheLargestApplicableUnit) {
  ScopedLoggerEncoding const encoding{ggems::core::Encoding::Ascii};

  EXPECT_EQ(HumanReadable(Energy{1'500'000'000ULL}, 2), "1.50 keV");
  EXPECT_EQ(HumanReadable(EnergyChange{-2'000'000'000LL}, 1), "-2.0 keV");
}

TEST(GGEMSEnergyUnitsTest, LiteralStoresMicroElectronVolts) {
  using namespace ggems::units;

  constexpr Energy energy = 2_keV;

  EXPECT_EQ(energy.value, 2'000'000'000ULL);
}

TEST(GGEMSEnergyUnitsTest, ResolvesMicroElectronVoltQuantaAndThermalNeutrons) {
  struct Case {
    long double milli_electron_volts;
    std::uint64_t expected;
  };
  constexpr std::array cases{
      Case{.milli_electron_volts = 0.0L, .expected = 0ULL},
      Case{.milli_electron_volts = 0.001L, .expected = 1ULL},
      Case{.milli_electron_volts = 0.01L, .expected = 10ULL},
      Case{.milli_electron_volts = 0.00049L, .expected = 0ULL},
      Case{.milli_electron_volts = 0.0005L, .expected = 1ULL},
      Case{.milli_electron_volts = 0.00051L, .expected = 1ULL},
      Case{.milli_electron_volts = 0.0015L, .expected = 2ULL}};
  for (auto const &test_case : cases) {
    SCOPED_TRACE(test_case.milli_electron_volts);
    auto const energy =
        MakeQuantity<Energy>(test_case.milli_electron_volts, "meV");
    ASSERT_TRUE(energy.has_value());
    EXPECT_EQ(energy->value, test_case.expected);
    auto const change =
        MakeQuantity<EnergyChange>(-test_case.milli_electron_volts, "meV");
    ASSERT_TRUE(change.has_value());
    EXPECT_EQ(change->value, -static_cast<std::int64_t>(test_case.expected));
  }
  auto const negative_quantum = MakeQuantity<Energy>(-0.0001L, "meV");
  ASSERT_FALSE(negative_quantum.has_value());
  EXPECT_EQ(negative_quantum.error(), UnitConversionError::NegativeValue);
}

TEST(GGEMSEnergyUnitsTest, PreservesExactIntegralConversionAndRejectsOverflow) {
  constexpr auto maximum = std::numeric_limits<std::uint64_t>::max();
  constexpr Energy maximum_energy{maximum};
  EXPECT_EQ(maximum_energy.value, 18'446'744'073'709'551'615ULL);
  constexpr auto largest_milli_eV = maximum / 1'000ULL;
  auto const exact = MakeQuantity<Energy>(largest_milli_eV, "meV");
  ASSERT_TRUE(exact.has_value());
  EXPECT_EQ(exact->value, 18'446'744'073'709'551'000ULL);
  auto const round_trip = ConvertTo<std::uint64_t>(*exact, "meV");
  ASSERT_TRUE(round_trip.has_value());
  EXPECT_EQ(*round_trip, largest_milli_eV);
  auto const overflow = MakeQuantity<Energy>(largest_milli_eV + 1ULL, "meV");
  ASSERT_FALSE(overflow.has_value());
  EXPECT_EQ(overflow.error(), UnitConversionError::OutOfRange);
  auto const tera_overflow = MakeQuantity<Energy>(19ULL, "TeV");
  ASSERT_FALSE(tera_overflow.has_value());
  EXPECT_EQ(tera_overflow.error(), UnitConversionError::OutOfRange);
  auto const floating_overflow = MakeQuantity<Energy>(19.0L, "TeV");
  ASSERT_FALSE(floating_overflow.has_value());
  EXPECT_EQ(floating_overflow.error(), UnitConversionError::OutOfRange);
  for (auto const energy : {Energy{1ULL}, Energy{10ULL}, maximum_energy}) {
    auto const inexact = ConvertTo<std::uint64_t>(energy, "meV");
    ASSERT_FALSE(inexact.has_value());
    EXPECT_EQ(inexact.error(), UnitConversionError::InexactConversion);
  }
  auto const exact_electron_volt =
      ConvertTo<std::uint64_t>(Energy{1'000'000ULL}, "eV");
  ASSERT_TRUE(exact_electron_volt.has_value());
  EXPECT_EQ(*exact_electron_volt, 1ULL);
  auto const zero = ConvertTo<std::uint64_t>(Energy{}, "TeV");
  ASSERT_TRUE(zero.has_value());
  EXPECT_EQ(*zero, 0ULL);
}

TEST(GGEMSEnergyUnitsTest, PreservesSignedRangeAndExactNegativeOutput) {
  constexpr auto maximum = std::numeric_limits<std::int64_t>::max();
  constexpr EnergyChange minimum_change{
      std::numeric_limits<std::int64_t>::min()};
  EXPECT_EQ(minimum_change.value, (-9'223'372'036'854'775'807LL - 1LL));
  auto const exact = MakeQuantity<EnergyChange>(-(maximum / 1'000LL), "meV");
  ASSERT_TRUE(exact.has_value());
  EXPECT_EQ(exact->value, -9'223'372'036'854'775'000LL);
  auto const round_trip = ConvertTo<std::int64_t>(*exact, "meV");
  ASSERT_TRUE(round_trip.has_value());
  EXPECT_EQ(*round_trip, -(maximum / 1'000LL));
  for (auto const sign : {-1LL, 1LL}) {
    auto const overflow =
        MakeQuantity<EnergyChange>(sign * ((maximum / 1'000LL) + 1LL), "meV");
    ASSERT_FALSE(overflow.has_value());
    EXPECT_EQ(overflow.error(), UnitConversionError::OutOfRange);
  }
  auto const negative_output =
      ConvertTo<std::uint64_t>(EnergyChange{-1'000LL}, "meV");
  ASSERT_FALSE(negative_output.has_value());
  EXPECT_EQ(negative_output.error(), UnitConversionError::NegativeValue);
}

TEST(GGEMSEnergyUnitsTest, PreservesRuntimeVocabularyAndSmallEnergyDisplay) {
  for (std::string_view const token : {"ueV", "uEV", "micro_eV", "milli_eV",
                                       "\xC2\xB5"
                                       "eV",
                                       "\xCE\xBC"
                                       "eV",
                                       "ev", " mev", "meV "}) {
    auto const rejected = MakeQuantity<Energy>(1ULL, token);
    ASSERT_FALSE(rejected.has_value());
    EXPECT_EQ(rejected.error(), UnitConversionError::UnsupportedUnit);
  }
  for (auto const mode :
       {ggems::core::Encoding::Ascii, ggems::core::Encoding::Unicode}) {
    ScopedLoggerEncoding const encoding{mode};
    EXPECT_EQ(HumanReadable(Energy{1ULL}), "0.0010000 meV");
    EXPECT_EQ(HumanReadable(Energy{10ULL}), "0.0100000 meV");
    EXPECT_EQ(std::format("{}", Energy{1'000'000ULL}), "1.0000000 eV");
  }
}

TEST(GGEMSEnergyUnitsTest, EveryLiteralPreservesItsPhysicalUnit) {
  using namespace ggems::units;
  constexpr std::array integral{1_meV, 1_eV, 1_keV, 1_MeV, 1_GeV, 1_TeV};
  constexpr std::array floating{1.0_meV, 1.0_eV,  1.0_keV,
                                1.0_MeV, 1.0_GeV, 1.0_TeV};
  constexpr std::array expected{1'000ULL,
                                1'000'000ULL,
                                1'000'000'000ULL,
                                1'000'000'000'000ULL,
                                1'000'000'000'000'000ULL,
                                1'000'000'000'000'000'000ULL};
  for (std::size_t index = 0U; index < expected.size(); ++index) {
    EXPECT_EQ(integral[index].value, expected[index]);
    EXPECT_EQ(floating[index].value, expected[index]);
  }
  static_assert((0.001_meV).value == 1ULL);
  static_assert((0.01_meV).value == 10ULL);
  static_assert((0.0005_meV).value == 1ULL);
}

} // namespace
/// \endcond
