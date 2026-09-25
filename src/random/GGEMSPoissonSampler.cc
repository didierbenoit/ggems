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
 * \brief Implements Poisson sampling for GGEMS host random streams.
 *
 * Uses inversion for small means and the PTRS transformed-rejection method for
 * larger means without input or candidate-range validation.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <cmath>
#include <cstdint>
/// \endcond

#include "GGEMS/random/GGEMSHostRandomStream.hh"
#include "GGEMS/random/GGEMSPoissonSampler.hh"

namespace ggems::core::random {
namespace {

// =============================================================================
// =============================================================================

/*!
 * \brief Samples a small Poisson mean by cumulative inversion.
 * \param[in] mean Positive Poisson mean below the inversion threshold.
 * \param[in,out] random Host random stream consumed by the sampler.
 * \return Sampled Poisson count.
 *
 * Uses compensated cumulative addition and returns the current candidate if the
 * next cumulative value no longer changes in working precision.
 */
auto SampleByInversion(long double mean, GGEMSHostRandomStream &random)
  -> std::uint64_t {
  auto const target = static_cast<long double>(random.UniformDoubleOpen01());
  long double probability = std::exp(-mean);
  long double cumulative = probability;
  long double correction{0.0L};
  std::uint64_t candidate{0ULL};

  while (target > cumulative) {
    ++candidate;
    probability *= mean / static_cast<long double>(candidate);

    long double const corrected_probability = probability - correction;
    long double const next_cumulative = cumulative + corrected_probability;
    correction = (next_cumulative - cumulative) - corrected_probability;

    if (next_cumulative == cumulative) {
      return candidate;
    }

    cumulative = next_cumulative;
  }

  return candidate;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Samples a mean of at least 30 using PTRS transformed rejection.
 *
 * \pre mean must be finite and at least 30. Every nonnegative candidate must be
 * finite and fit uint64_t; the cast is unchecked.
 * \param[in] mean Dimensionless expected count.
 * \param[in,out] random Host stream consumed by the sampler.
 * \return The accepted Poisson count.
 */
auto SampleByTransformedRejection(long double mean,
                                  GGEMSHostRandomStream &random)
  -> std::uint64_t {
  long double const sqrt_mean = std::sqrt(mean);
  long double const b = 0.931L + (2.53L * sqrt_mean);
  long double const a = -0.059L + (0.02483L * b);
  long double const inverse_alpha = 1.1239L + (1.1328L / (b - 3.4L));
  long double const vr = 0.9277L - (3.6224L / (b - 2.0L));
  long double const log_mean = std::log(mean);
  long double const log_inverse_alpha = std::log(inverse_alpha);

  while (true) {
    long double const u =
      static_cast<long double>(random.UniformDoubleOpen01()) - 0.5L;
    auto const v = static_cast<long double>(random.UniformDoubleOpen01());
    long double const us = 0.5L - std::abs(u);

    long double const candidate_value =
      std::floor(((((2.0L * a) / us) + b) * u) + mean + 0.43L);

    if (candidate_value < 0.0L) {
      continue;
    }

    auto const candidate = static_cast<std::uint64_t>(candidate_value);

    if (us >= 0.07L && v <= vr) {
      return candidate;
    }

    if (us < 0.013L && v > us) {
      continue;
    }

    long double const log_acceptance =
      std::log(v) + log_inverse_alpha - std::log((a / (us * us)) + b);
    long double const log_probability = -mean + (candidate_value * log_mean) -
                                        std::lgamma(candidate_value + 1.0L);

    if (log_acceptance <= log_probability) {
      return candidate;
    }
  }
}
} // namespace

// =============================================================================
// =============================================================================

auto SamplePoisson(long double mean, GGEMSHostRandomStream &random)
  -> std::uint64_t {
  if (mean == 0.0L) {
    return 0ULL;
  }

  if (mean < 30.0L) {
    return SampleByInversion(mean, random);
  }

  return SampleByTransformedRejection(mean, random);
}
} // namespace ggems::core::random
