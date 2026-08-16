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
 * \brief Unit tests for GGEMS frequency quantities and conversions.
 *
 * Validates registered frequency units, negative-value rejection, automatic SI prefix formatting, and hertz literals.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <array>
#include <cstdint>
#include <string_view>

#include <gtest/gtest.h>

/// \endcond
#include "GGEMS/core/GGEMSLogger.hh"
#include "GGEMS/core/units/GGEMSFrequencyUnits.hh"
#include "GGEMS/core/units/GGEMSUnitConversion.hh"
#include "GGEMS/core/units/GGEMSUnitFormatting.hh"
#include "GGEMSScopedLoggerEncoding.hh"

/// \cond

namespace {

using ggems::test::ScopedLoggerEncoding;
using ggems::units::Frequency;
using ggems::units::HumanReadable;
using ggems::units::MakeQuantity;
using ggems::units::UnitConversionError;

TEST(GGEMSFrequencyUnitsTest, ConvertsEveryOfficialRuntimeToken) {
  struct Case {
    std::string_view unit;
    std::uint64_t expected_hertz;
  };

  constexpr std::array<Case, 5U> cases{{
      {.unit = "Hz", .expected_hertz = 1ULL},
      {.unit = "kHz", .expected_hertz = 1'000ULL},
      {.unit = "MHz", .expected_hertz = 1'000'000ULL},
      {.unit = "GHz", .expected_hertz = 1'000'000'000ULL},
      {.unit = "THz", .expected_hertz = 1'000'000'000'000ULL},
  }};

  for (auto const &test_case : cases) {
    SCOPED_TRACE(test_case.unit);
    auto const converted = MakeQuantity<Frequency>(1, test_case.unit);

    ASSERT_TRUE(converted.has_value());
    EXPECT_EQ(converted->value, test_case.expected_hertz);
  }
}

TEST(GGEMSFrequencyUnitsTest, RejectsNegativeFrequency) {
  auto const converted = MakeQuantity<Frequency>(-1, "Hz");

  ASSERT_FALSE(converted.has_value());
  EXPECT_EQ(converted.error(), UnitConversionError::NegativeValue);
}

TEST(GGEMSFrequencyUnitsTest, AutomaticallyDisplaysSIPrefixes) {
  ScopedLoggerEncoding const encoding{ggems::core::Encoding::Ascii};

  EXPECT_EQ(HumanReadable(Frequency{2'500'000ULL}, 2), "2.50 MHz");
}

TEST(GGEMSFrequencyUnitsTest, LiteralStoresHertz) {
  using namespace ggems::units;

  constexpr Frequency frequency = 3_GHz;

  EXPECT_EQ(frequency.value, 3'000'000'000ULL);
}

} // namespace
/// \endcond
