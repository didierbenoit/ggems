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
 * \brief Unit tests for GGEMS Poisson sampling.
 *
 * Validates input rejection, deterministic continuation, inversion and PTRS sampling regimes, large representable means, and aggregate statistical plausibility.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>

#include <gtest/gtest.h>

/// \endcond
#include "GGEMS/GGEMSException.hh"
#include "GGEMS/random/GGEMSHostRandomStream.hh"
#include "GGEMS/random/GGEMSPoissonSampler.hh"
#include "GGEMS/random/GGEMSRandom.hh"
#include "GGEMS/random/GGEMSRandomEngine.hh"

/// \cond

namespace {

// =============================================================================
// =============================================================================

using ggems::core::random::GGEMSHostRandomStream;
using ggems::core::random::GGEMSRandom;
using ggems::core::random::GGEMSRandomEngine;
using ggems::core::random::SamplePoisson;

// =============================================================================
// =============================================================================

constexpr std::array<GGEMSRandomEngine, 3> k_engines{GGEMSRandomEngine::JKISS,
                                                     GGEMSRandomEngine::PCG32,
                                                     GGEMSRandomEngine::Philox};

// =============================================================================
// =============================================================================

static_assert(std::is_same_v<
              decltype(SamplePoisson(std::declval<long double>(),
                                     std::declval<GGEMSHostRandomStream &>())),
              std::uint64_t>);

// =============================================================================
// =============================================================================

TEST(GGEMSPoissonSamplerTest, RejectsInvalidMeans) {
  GGEMSRandom configuration{};
  GGEMSHostRandomStream random{configuration, 42ULL};

  EXPECT_THROW((void)SamplePoisson(-1.0L, random),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW((void)SamplePoisson(
                   std::numeric_limits<long double>::quiet_NaN(), random),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(
      (void)SamplePoisson(std::numeric_limits<long double>::infinity(), random),
      ggems::core::GGEMSExceptionBase);
  EXPECT_THROW((void)SamplePoisson(18'446'744'073'709'551'616.0L, random),
               ggems::core::GGEMSExceptionBase);
}

// =============================================================================
// =============================================================================

TEST(GGEMSPoissonSamplerTest, ZeroMeanReturnsZeroWithoutConsumingRandomState) {
  for (GGEMSRandomEngine engine : k_engines) {
    SCOPED_TRACE(static_cast<std::uint32_t>(engine));

    GGEMSRandom configuration{};
    configuration.SetEngine(engine).SetSeed(77'777ULL);
    GGEMSHostRandomStream random{configuration, 42ULL};
    GGEMSHostRandomStream reference{configuration, 42ULL};

    EXPECT_EQ(SamplePoisson(0.0L, random), 0ULL);
    EXPECT_EQ(random.NextUInt32(), reference.NextUInt32());
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSPoissonSamplerTest,
     FixedConfigurationProducesDeterministicContinuation) {
  constexpr std::array<long double, 6> means{0.1L,  1.0L,     10.0L,
                                             30.0L, 1'000.0L, 1.0e8L};

  for (GGEMSRandomEngine engine : k_engines) {
    SCOPED_TRACE(static_cast<std::uint32_t>(engine));

    GGEMSRandom configuration{};
    configuration.SetEngine(engine).SetSeed(77'777ULL);
    GGEMSHostRandomStream first{configuration, 42ULL};
    GGEMSHostRandomStream second{configuration, 42ULL};

    for (long double mean : means) {
      for (std::size_t sample = 0U; sample < 32U; ++sample) {
        EXPECT_EQ(SamplePoisson(mean, first), SamplePoisson(mean, second));
      }
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSPoissonSamplerTest, SamplesRepresentativeSmallMeansByInversion) {
  GGEMSRandom configuration{};
  configuration.SetEngine(GGEMSRandomEngine::PCG32).SetSeed(1'234ULL);
  GGEMSHostRandomStream random{configuration, 17ULL};

  for (long double mean : {0.1L, 1.0L, 10.0L, 29.999L}) {
    for (std::size_t sample = 0U; sample < 128U; ++sample) {
      EXPECT_NO_THROW((void)SamplePoisson(mean, random));
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSPoissonSamplerTest, SamplesRepresentativeLargeMeansByPTRS) {
  GGEMSRandom configuration{};
  configuration.SetEngine(GGEMSRandomEngine::Philox).SetSeed(1'234ULL);
  GGEMSHostRandomStream random{configuration, 17ULL};

  for (long double mean : {30.0L, 1'000.0L, 1.0e8L}) {
    for (std::size_t sample = 0U; sample < 128U; ++sample) {
      EXPECT_NO_THROW((void)SamplePoisson(mean, random));
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSPoissonSamplerTest, SamplesLargeRepresentableMeanWithinUInt64Range) {
  GGEMSRandom configuration{};
  configuration.SetEngine(GGEMSRandomEngine::PCG32).SetSeed(1'234ULL);
  GGEMSHostRandomStream random{configuration, 17ULL};

  for (std::size_t sample = 0U; sample < 128U; ++sample) {
    std::uint64_t const value = SamplePoisson(1.0e18L, random);
    EXPECT_LT(value, std::numeric_limits<std::uint64_t>::max());
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSPoissonSamplerTest, AggregateMeanAndVarianceArePlausible) {
  constexpr std::size_t k_sample_count{20'000U};
  constexpr std::array<long double, 3> means{1.0L, 30.0L, 1'000.0L};

  for (long double expected_mean : means) {
    SCOPED_TRACE(static_cast<double>(expected_mean));

    GGEMSRandom configuration{};
    configuration.SetEngine(GGEMSRandomEngine::PCG32).SetSeed(9'876ULL);
    GGEMSHostRandomStream random{configuration, 123ULL};
    long double sample_mean{0.0L};
    long double sum_squared_deviation{0.0L};

    for (std::size_t index = 1U; index <= k_sample_count; ++index) {
      auto const value =
          static_cast<long double>(SamplePoisson(expected_mean, random));
      long double const delta = value - sample_mean;
      sample_mean += delta / static_cast<long double>(index);
      sum_squared_deviation += delta * (value - sample_mean);
    }

    long double const sample_variance =
        sum_squared_deviation / static_cast<long double>(k_sample_count - 1U);
    long double const mean_tolerance = std::max(
        0.05L, 8.0L * std::sqrt(expected_mean /
                                static_cast<long double>(k_sample_count)));
    long double const variance_tolerance = std::max(
        0.2L,
        10.0L * expected_mean *
            std::sqrt(2.0L / static_cast<long double>(k_sample_count - 1U)));

    EXPECT_NEAR(static_cast<double>(sample_mean),
                static_cast<double>(expected_mean),
                static_cast<double>(mean_tolerance));
    EXPECT_NEAR(static_cast<double>(sample_variance),
                static_cast<double>(expected_mean),
                static_cast<double>(variance_tolerance));
  }
}

} // namespace
/// \endcond
