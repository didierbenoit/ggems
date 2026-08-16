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
 * Uses inversion for small means and the PTRS transformed-rejection method for larger representable means.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <cmath>
#include <cstdint>
#include <limits>

/// \endcond
#include "GGEMS/core/GGEMSException.hh"

#include "GGEMS/core/random/GGEMSHostRandomStream.hh"
#include "GGEMS/core/random/GGEMSPoissonSampler.hh"

namespace ggems::core::random {
namespace {

// =============================================================================
// =============================================================================

/*! \brief Mean below which direct inversion sampling is used. */
constexpr long double k_inversion_threshold{30.0L};
/*! \brief Exact upper bound used when checking uint64_t representability. */
constexpr long double k_uint64_upper_exclusive{18'446'744'073'709'551'616.0L};
/*! \brief PTRS b-parameter offset. */
constexpr long double k_ptrs_b_offset{0.931L};
/*! \brief PTRS b-parameter square-root scale. */
constexpr long double k_ptrs_b_scale{2.53L};
/*! \brief PTRS a-parameter offset. */
constexpr long double k_ptrs_a_offset{-0.059L};
/*! \brief PTRS a-parameter scale. */
constexpr long double k_ptrs_a_scale{0.02483L};
/*! \brief PTRS inverse-alpha offset. */
constexpr long double k_ptrs_inverse_alpha_offset{1.1239L};
/*! \brief PTRS inverse-alpha numerator scale. */
constexpr long double k_ptrs_inverse_alpha_scale{1.1328L};
/*! \brief PTRS inverse-alpha denominator offset. */
constexpr long double k_ptrs_inverse_alpha_denominator_offset{3.4L};
/*! \brief PTRS fast-acceptance v-ratio offset. */
constexpr long double k_ptrs_vr_offset{0.9277L};
/*! \brief PTRS fast-acceptance v-ratio scale. */
constexpr long double k_ptrs_vr_scale{3.6224L};
/*! \brief PTRS v-ratio denominator offset. */
constexpr long double k_ptrs_vr_denominator_offset{2.0L};
/*! \brief Center used to map a uniform variate to the symmetric PTRS coordinate. */
constexpr long double k_ptrs_center{0.5L};
/*! \brief PTRS integer-candidate rounding offset. */
constexpr long double k_ptrs_candidate_offset{0.43L};
/*! \brief PTRS threshold enabling the fast-acceptance branch. */
constexpr long double k_ptrs_fast_acceptance_us_min{0.07L};
/*! \brief PTRS threshold for the early rejection squeeze. */
constexpr long double k_ptrs_squeeze_us_max{0.013L};

// =============================================================================
// =============================================================================

/*!
 * \brief Checks whether a Poisson mean can be represented safely by the uint64_t result type.
 * \param[in] mean Candidate Poisson mean.
 * \return True when the mean is within the representable count domain.
 */
auto IsRepresentableMean(long double mean) noexcept -> bool {
  if constexpr (std::numeric_limits<long double>::digits >
                std::numeric_limits<std::uint64_t>::digits) {
    return mean <=
           static_cast<long double>(std::numeric_limits<std::uint64_t>::max());
  }

  return mean < k_uint64_upper_exclusive;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Samples a small Poisson mean by cumulative inversion.
 * \param[in] mean Positive Poisson mean below the inversion threshold.
 * \param[in,out] random Host random stream consumed by the sampler.
 * \return Sampled Poisson count.
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
 * \brief Samples a larger Poisson mean using the PTRS transformed-rejection method.
 * \param[in] mean Positive Poisson mean in the transformed-rejection regime.
 * \param[in,out] random Host random stream consumed by the sampler.
 * \return Sampled Poisson count.
 * \throws ggems::core::GGEMSRecoverable If a candidate is non-finite or outside uint64_t range.
 */
auto SampleByTransformedRejection(long double mean,
                                  GGEMSHostRandomStream &random)
    -> std::uint64_t {
  long double const sqrt_mean = std::sqrt(mean);
  long double const b = k_ptrs_b_offset + (k_ptrs_b_scale * sqrt_mean);
  long double const a = k_ptrs_a_offset + (k_ptrs_a_scale * b);
  long double const inverse_alpha =
      k_ptrs_inverse_alpha_offset +
      (k_ptrs_inverse_alpha_scale /
       (b - k_ptrs_inverse_alpha_denominator_offset));
  long double const vr =
      k_ptrs_vr_offset - (k_ptrs_vr_scale / (b - k_ptrs_vr_denominator_offset));

  while (true) {
    long double const u =
        static_cast<long double>(random.UniformDoubleOpen01()) - k_ptrs_center;
    auto const v = static_cast<long double>(random.UniformDoubleOpen01());
    long double const us = k_ptrs_center - std::abs(u);

    if (!(us > 0.0L)) {
      continue;
    }

    long double const candidate_value = std::floor(
        ((((2.0L * a) / us) + b) * u) + mean + k_ptrs_candidate_offset);

    if (!std::isfinite(candidate_value)) {
      throw ggems::core::GGEMSRecoverable("Sampled Poisson candidate is not finite.");
    }

    if (candidate_value < 0.0L) {
      continue;
    }

    if (!(candidate_value < k_uint64_upper_exclusive)) {
      throw ggems::core::GGEMSRecoverable("Sampled Poisson candidate exceeds uint64_t range.");
    }

    auto const candidate = static_cast<std::uint64_t>(candidate_value);

    if (us >= k_ptrs_fast_acceptance_us_min && v <= vr) {
      return candidate;
    }

    if (us < k_ptrs_squeeze_us_max && v > us) {
      continue;
    }

    long double const log_acceptance =
        std::log(v) + std::log(inverse_alpha) - std::log((a / (us * us)) + b);
    long double const log_probability = -mean +
                                        (candidate_value * std::log(mean)) -
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
  if (!(std::isfinite(mean))) {
    throw ggems::core::GGEMSRecoverable("Poisson mean must be finite.");
  }
  if (!(mean >= 0.0L)) {
    throw ggems::core::GGEMSRecoverable("Poisson mean must be non-negative.");
  }
  if (!(IsRepresentableMean(mean))) {
    throw ggems::core::GGEMSRecoverable("Poisson mean exceeds uint64_t range.");
  }

  if (mean == 0.0L) {
    return 0ULL;
  }

  if (mean < k_inversion_threshold) {
    return SampleByInversion(mean, random);
  }

  return SampleByTransformedRejection(mean, random);
}
} // namespace ggems::core::random
