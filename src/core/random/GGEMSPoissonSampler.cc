#include <cmath>
#include <cstdint>
#include <limits>

#include "GGEMS/core/GGEMSException.hh"

#include "GGEMS/core/random/GGEMSHostRandomStream.hh"
#include "GGEMS/core/random/GGEMSPoissonSampler.hh"

namespace ggems::core::random {
namespace {

// =============================================================================
// =============================================================================

constexpr long double k_inversion_threshold{30.0L};
constexpr long double k_uint64_upper_exclusive{18'446'744'073'709'551'616.0L};
constexpr long double k_ptrs_b_offset{0.931L};
constexpr long double k_ptrs_b_scale{2.53L};
constexpr long double k_ptrs_a_offset{-0.059L};
constexpr long double k_ptrs_a_scale{0.02483L};
constexpr long double k_ptrs_inverse_alpha_offset{1.1239L};
constexpr long double k_ptrs_inverse_alpha_scale{1.1328L};
constexpr long double k_ptrs_inverse_alpha_denominator_offset{3.4L};
constexpr long double k_ptrs_vr_offset{0.9277L};
constexpr long double k_ptrs_vr_scale{3.6224L};
constexpr long double k_ptrs_vr_denominator_offset{2.0L};
constexpr long double k_ptrs_center{0.5L};
constexpr long double k_ptrs_candidate_offset{0.43L};
constexpr long double k_ptrs_fast_acceptance_us_min{0.07L};
constexpr long double k_ptrs_squeeze_us_max{0.013L};

// =============================================================================
// =============================================================================

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
