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

#include <cstddef>
#include <cstdint>
#include <array>
#include <expected>
#include <limits>
#include <numbers>
#include <type_traits>
#include <utility>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/GGEMSTimeWindow.hh"
#include "GGEMS/radioactivity/GGEMSDecayStatistics.hh"
#include "GGEMS/random/GGEMSHostRandomStream.hh"
#include "GGEMS/random/GGEMSRandom.hh"
#include "GGEMS/random/GGEMSRandomEngine.hh"
#include "GGEMS/units/GGEMSActivityUnits.hh"
#include "GGEMS/units/GGEMSUnitConversion.hh"

namespace {

// =============================================================================
// =============================================================================

using ggems::core::GGEMSTimeWindow;
using ggems::core::radioactivity::ComputeExpectedDecayEventCount;
using ggems::core::radioactivity::SampleDecayEventCount;
using ggems::core::random::GGEMSHostRandomStream;
using ggems::core::random::GGEMSRandom;
using ggems::core::random::GGEMSRandomEngine;
using ggems::units::Activity;

// =============================================================================
// =============================================================================

static_assert(
  std::is_same_v<decltype(ComputeExpectedDecayEventCount(
                   std::declval<Activity>(), std::declval<long double>(),
                   std::declval<std::uint64_t>(),
                   std::declval<GGEMSTimeWindow>())),
                 long double>);
static_assert(std::is_same_v<
              decltype(SampleDecayEventCount(
                std::declval<Activity>(), std::declval<long double>(),
                std::declval<std::uint64_t>(), std::declval<GGEMSTimeWindow>(),
                std::declval<GGEMSHostRandomStream &>())),
              std::uint64_t>);

// =============================================================================
// =============================================================================

TEST(GGEMSDecayStatisticsTest, ZeroActivityHasZeroExpectedCount) {
  EXPECT_EQ(
    ComputeExpectedDecayEventCount(
      ggems::units::MakeQuantity<Activity>(0.0L, "Bq").value(), 1'000.0L, 0ULL,
      GGEMSTimeWindow{.start_ps = 0ULL, .stop_ps = 1'000'000'000'000ULL}),
    0.0L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSDecayStatisticsTest, EmptyWindowHasZeroExpectedCount) {
  EXPECT_EQ(ComputeExpectedDecayEventCount(
              ggems::units::MakeQuantity<Activity>(1.0e6L, "Bq").value(),
              1'000.0L, 0ULL,
              GGEMSTimeWindow{.start_ps = 42ULL, .stop_ps = 42ULL}),
            0.0L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSDecayStatisticsTest, EquivalentUnitsEnterDecayFormulaInBecquerels) {
  std::array<Activity, 4U> const activities{
    ggems::units::MakeQuantity<Activity>(1'000.0L, "mBq").value(),
    ggems::units::MakeQuantity<Activity>(1.0L, "Bq").value(),
    ggems::units::MakeQuantity<Activity>(1.0e-6L, "MBq").value(),
    ggems::units::MakeQuantity<Activity>(1.0L / 37'000'000'000.0L, "Ci")
      .value(),
  };
  // One Bq with a one-second half-life integrates to 1/(2*ln(2)) decays.
  long double const expected = 0.5L / std::numbers::ln2_v<long double>;
  GGEMSTimeWindow const window{.start_ps = 0ULL,
                               .stop_ps = 1'000'000'000'000ULL};
  for (auto const activity : activities) {
    EXPECT_EQ(activity.value, 1'000ULL);
    EXPECT_DOUBLE_EQ(static_cast<double>(ComputeExpectedDecayEventCount(
                       activity, 1.0L, 0ULL, window)),
                     static_cast<double>(expected));
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSDecayStatisticsTest, RejectsInvalidActivityAtUnitsBoundary) {
  GGEMSTimeWindow const window{.start_ps = 0ULL, .stop_ps = 1ULL};

  EXPECT_THROW((void)ComputeExpectedDecayEventCount(
                 ggems::units::MakeQuantity<Activity>(-1.0L, "Bq").value(),
                 1.0L, 0ULL, window),
               std::bad_expected_access<ggems::units::UnitConversionError>);
  EXPECT_THROW((void)ComputeExpectedDecayEventCount(
                 ggems::units::MakeQuantity<Activity>(
                   std::numeric_limits<long double>::quiet_NaN(), "Bq")
                   .value(),
                 1.0L, 0ULL, window),
               std::bad_expected_access<ggems::units::UnitConversionError>);
}

// =============================================================================
// =============================================================================

TEST(GGEMSDecayStatisticsTest, RejectsReferenceTimeAfterWindowStart) {
  EXPECT_THROW((void)ComputeExpectedDecayEventCount(
                 ggems::units::MakeQuantity<Activity>(1.0L, "Bq").value(), 1.0L,
                 2ULL, GGEMSTimeWindow{.start_ps = 1ULL, .stop_ps = 1ULL}),
               ggems::core::GGEMSRecoverable);
}

// =============================================================================
// =============================================================================

TEST(GGEMSDecayStatisticsTest, AdjacentWindowsAreAdditive) {
  auto const activity =
    ggems::units::MakeQuantity<Activity>(1.0e6L, "Bq").value();
  long double const half_life_seconds{1'000.0L};
  GGEMSTimeWindow const first{
    .start_ps = 0ULL,
    .stop_ps = 10'000'000'000'000ULL,
  };
  GGEMSTimeWindow const second{
    .start_ps = first.stop_ps,
    .stop_ps = 20'000'000'000'000ULL,
  };
  GGEMSTimeWindow const combined{
    .start_ps = first.start_ps,
    .stop_ps = second.stop_ps,
  };

  long double const adjacent =
    ComputeExpectedDecayEventCount(activity, half_life_seconds, 0ULL, first) +
    ComputeExpectedDecayEventCount(activity, half_life_seconds, 0ULL, second);
  long double const total =
    ComputeExpectedDecayEventCount(activity, half_life_seconds, 0ULL, combined);

  EXPECT_NEAR(static_cast<double>(adjacent), static_cast<double>(total),
              static_cast<double>(total * 1.0e-12L));
}

// =============================================================================
// =============================================================================

TEST(GGEMSDecayStatisticsTest,
     VeryLongHalfLifeApproachesActivityTimesDuration) {
  auto const activity =
    ggems::units::MakeQuantity<Activity>(1'234.5L, "Bq").value();
  long double const expected = 1'234.5L * 1.0L;
  long double const actual = ComputeExpectedDecayEventCount(
    activity, 1.0e20L, 0ULL,
    GGEMSTimeWindow{.start_ps = 0ULL, .stop_ps = 1'000'000'000'000ULL});

  EXPECT_NEAR(static_cast<double>(actual), static_cast<double>(expected),
              1.0e-9);
}

// =============================================================================
// =============================================================================

TEST(GGEMSDecayStatisticsTest, MatchesF18HalfLifeNumericalReference) {
  long double const actual = ComputeExpectedDecayEventCount(
    ggems::units::MakeQuantity<Activity>(1.0e6L, "Bq").value(),
    1.82890L * 3'600.0L, 0ULL,
    GGEMSTimeWindow{.start_ps = 0ULL, .stop_ps = 3'600'000'000'000'000ULL});

  EXPECT_NEAR(static_cast<double>(actual), 2'996'405'056.4, 1.0);
}

// =============================================================================
// =============================================================================

TEST(GGEMSDecayStatisticsTest,
     SampledCountHasDeterministicContinuationForFixedStream) {
  GGEMSRandom configuration{};
  configuration.SetEngine(GGEMSRandomEngine::PCG32).SetSeed(77'777ULL);
  GGEMSHostRandomStream first{configuration, 42ULL};
  GGEMSHostRandomStream second{configuration, 42ULL};
  GGEMSTimeWindow const window{
    .start_ps = 0ULL,
    .stop_ps = 1'000'000'000'000ULL,
  };

  for (std::size_t sample = 0U; sample < 64U; ++sample) {
    EXPECT_EQ(SampleDecayEventCount(
                ggems::units::MakeQuantity<Activity>(1'000.0L, "Bq").value(),
                1'000.0L, 0ULL, window, first),
              SampleDecayEventCount(
                ggems::units::MakeQuantity<Activity>(1'000.0L, "Bq").value(),
                1'000.0L, 0ULL, window, second));
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSDecayStatisticsTest, ExpectedAndSampledCountsRemainDistinct) {
  GGEMSRandom configuration{};
  configuration.SetEngine(GGEMSRandomEngine::Philox).SetSeed(77'777ULL);
  GGEMSHostRandomStream random{configuration, 42ULL};
  GGEMSTimeWindow const window{.start_ps = 0ULL,
                               .stop_ps = 1'000'000'000'000ULL};
  long double const expected = ComputeExpectedDecayEventCount(
    ggems::units::MakeQuantity<Activity>(123.45L, "Bq").value(), 1'000.0L, 0ULL,
    window);
  std::uint64_t const sampled = SampleDecayEventCount(
    ggems::units::MakeQuantity<Activity>(123.45L, "Bq").value(), 1'000.0L, 0ULL,
    window, random);

  EXPECT_GT(expected, 0.0L);
  EXPECT_NE(expected, static_cast<long double>(sampled));
}

// =============================================================================
// =============================================================================

TEST(GGEMSDecayStatisticsTest, ZeroExpectedCountDoesNotConsumeHostRandomState) {
  GGEMSRandom configuration{};
  configuration.SetEngine(GGEMSRandomEngine::JKISS).SetSeed(77'777ULL);
  GGEMSHostRandomStream random{configuration, 42ULL};
  GGEMSHostRandomStream reference{configuration, 42ULL};

  EXPECT_EQ(SampleDecayEventCount(
              ggems::units::MakeQuantity<Activity>(0.0L, "Bq").value(),
              1'000.0L, 0ULL,
              GGEMSTimeWindow{.start_ps = 0ULL, .stop_ps = 1ULL}, random),
            0ULL);
  EXPECT_EQ(random.NextUInt32(), reference.NextUInt32());
}

// =============================================================================
// =============================================================================

TEST(GGEMSDecayStatisticsTest,
     SubMillibecquerelQuantizationPreservesZeroDraws) {
  auto const activity =
    ggems::units::MakeQuantity<Activity>(0.0004L, "Bq").value();
  EXPECT_EQ(activity.value, 0ULL);
  GGEMSRandom configuration{};
  configuration.SetEngine(GGEMSRandomEngine::JKISS).SetSeed(77'777ULL);
  GGEMSHostRandomStream random{configuration, 42ULL};
  GGEMSHostRandomStream reference{configuration, 42ULL};
  EXPECT_EQ(
    SampleDecayEventCount(
      activity, 1.0L, 0ULL,
      GGEMSTimeWindow{.start_ps = 0ULL, .stop_ps = 1'000'000'000'000ULL},
      random),
    0ULL);
  EXPECT_EQ(random.NextUInt32(), reference.NextUInt32());
}

} // namespace
