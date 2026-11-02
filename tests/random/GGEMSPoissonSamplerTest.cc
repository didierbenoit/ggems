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
 * \brief Checks poisson sampler behavior and observable contracts.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>

#include <gtest/gtest.h>

#include "GGEMS/random/GGEMSHostRandomStream.hh"
#include "GGEMS/random/GGEMSPoissonSampler.hh"
#include "GGEMS/random/GGEMSRandom.hh"
#include "GGEMS/random/GGEMSRandomEngine.hh"

namespace {

// =============================================================================
// =============================================================================

using ggems::core::random::GGEMSHostRandomStream;
using ggems::core::random::GGEMSRandom;
using ggems::core::random::GGEMSRandomEngine;
using ggems::core::random::SamplePoisson;

// =============================================================================
// =============================================================================

constexpr std::array<GGEMSRandomEngine, 3> k_engines{
  GGEMSRandomEngine::JKISS,
  GGEMSRandomEngine::PCG32,
  GGEMSRandomEngine::Philox,
};

// =============================================================================
// =============================================================================

static_assert(std::is_same_v<
              decltype(SamplePoisson(std::declval<long double>(),
                                     std::declval<GGEMSHostRandomStream &>())),
              std::uint64_t>);

// =============================================================================
// =============================================================================

// A valid empty or zero-valued configuration must retain its explicit state
// and boundaries.
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

// A fixed stream must replay within its contract and continue without
// unintended reset.
TEST(GGEMSPoissonSamplerTest,
     FixedConfigurationProducesDeterministicContinuation) {
  constexpr std::array<long double, 6> means{
    0.1L, 1.0L, 10.0L, 30.0L, 1'000.0L, 1.0e8L,
  };

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

// Sampled populations must agree with the independent Poisson law over the
// declared mean range.
TEST(GGEMSPoissonSamplerTest, SmallMeanSamplesMatchIndependentPoissonCDF) {
  GGEMSRandom configuration{};
  configuration.SetEngine(GGEMSRandomEngine::PCG32).SetSeed(1'234ULL);
  GGEMSHostRandomStream random{configuration, 17ULL};
  GGEMSHostRandomStream reference{configuration, 17ULL};

  for (long double mean : {0.1L, 1.0L, 10.0L, 29.999L}) {
    SCOPED_TRACE(mean);

    for (std::size_t sample = 0U; sample < 128U; ++sample) {
      auto const value = SamplePoisson(mean, random);
      auto const target =
        static_cast<long double>(reference.UniformDoubleOpen01());

      // Evaluate each PMF term independently from its closed logarithmic form.
      long double lower_cdf{0.0L};
      for (std::uint64_t count = 0ULL; count < value; ++count) {
        auto const count_value = static_cast<long double>(count);
        lower_cdf += std::exp(-mean + (count_value * std::log(mean)) -
                              std::lgamma(count_value + 1.0L));
      }
      auto const value_as_real = static_cast<long double>(value);
      long double const upper_cdf =
        lower_cdf + std::exp(-mean + (value_as_real * std::log(mean)) -
                             std::lgamma(value_as_real + 1.0L));

      EXPECT_LT(lower_cdf, target);
      EXPECT_GE(upper_cdf, target);
    }
  }

  EXPECT_EQ(random.NextUInt32(), reference.NextUInt32());
}

// =============================================================================
// =============================================================================

// Representation limits must remain exact and reject values that cannot be
// represented.
TEST(GGEMSPoissonSamplerTest, SamplesLargeRepresentableMeanWithinUInt64Range) {
  GGEMSRandom configuration{};
  configuration.SetEngine(GGEMSRandomEngine::PCG32).SetSeed(1'234ULL);
  GGEMSHostRandomStream random{configuration, 17ULL};

  for (std::size_t sample = 0U; sample < 128U; ++sample) {
    std::uint64_t const value = SamplePoisson(1.0e18L, random);
    EXPECT_GT(value, std::numeric_limits<std::uint32_t>::max());
    EXPECT_LT(value, std::numeric_limits<std::uint64_t>::max());
  }
}

// =============================================================================
// =============================================================================

// Sampled populations must agree with the independent Poisson law over the
// declared mean range.
TEST(GGEMSPoissonSamplerTest, AggregateMeanAndVarianceArePlausible) {
  constexpr std::size_t k_sample_count{20'000U};
  constexpr std::array<long double, 4> means{1.0L, 30.0L, 1'000.0L, 1.0e8L};

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
    long double const mean_tolerance =
      std::max(0.05L, 8.0L * std::sqrt(expected_mean / static_cast<long double>(
                                                         k_sample_count)));
    long double const variance_tolerance = std::max(
      0.2L, 10.0L * expected_mean *
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
