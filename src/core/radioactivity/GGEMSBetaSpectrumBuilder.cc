#include <algorithm>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numbers>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/radioactivity/GGEMSBetaSpectrumBuilder.hh"
#include "GGEMS/core/radioactivity/GGEMSBetaTransition.hh"
#include "GGEMS/core/sources/GGEMSEnergyDistribution.hh"

namespace ggems::core::radioactivity {
namespace {

// =============================================================================
// =============================================================================

inline constexpr long double k_electron_rest_energy_milli_eV{510'998'950.69L};
inline constexpr long double k_fine_structure_constant{7.297'352'564'3e-3L};
inline constexpr std::size_t k_simpson_subinterval_count{8U};
inline constexpr long double k_beta_plus_asymptotic_threshold{50.0L};

// =============================================================================
// =============================================================================

struct CompensatedSum {
  auto Add(long double value) noexcept -> void {
    long double const next = sum + value;

    if (std::abs(sum) >= std::abs(value)) {
      correction += (sum - next) + value;
    } else {
      correction += (value - next) + sum;
    }

    sum = next;
  }

  [[nodiscard]] auto Get() const noexcept -> long double {
    return sum + correction;
  }

  long double sum{0.0L};
  long double correction{0.0L};
};

// =============================================================================
// =============================================================================

struct IntegralPair {
  long double mass{0.0L};
  long double first_moment_milli_eV{0.0L};
};

// =============================================================================
// =============================================================================

struct RegularGrid {
  std::uint64_t lower_edge_milli_eV{0ULL};
  std::uint64_t upper_edge_milli_eV{0ULL};
  std::uint64_t bin_width_milli_eV{0ULL};
  std::vector<double> centers_milli_eV;
};

// =============================================================================
// =============================================================================

[[noreturn]] auto Reject(std::string message) -> void {
  throw GGEMSRecoverable(std::move(message));
}

// =============================================================================
// =============================================================================

[[nodiscard]] constexpr auto IsKnownSign(GGEMSBetaSign sign) noexcept -> bool {
  switch (sign) {
  case GGEMSBetaSign::Minus:
  case GGEMSBetaSign::Plus:
    return true;
  }

  return false;
}

// =============================================================================
// =============================================================================

[[nodiscard]] constexpr auto
IsKnownTransitionClass(GGEMSBetaTransitionClass transition_class) noexcept
    -> bool {
  switch (transition_class) {
  case GGEMSBetaTransitionClass::Allowed:
  case GGEMSBetaTransitionClass::FirstForbidden:
  case GGEMSBetaTransitionClass::UniqueFirstForbidden:
  case GGEMSBetaTransitionClass::SecondForbidden:
  case GGEMSBetaTransitionClass::UniqueSecondForbidden:
  case GGEMSBetaTransitionClass::ThirdForbidden:
  case GGEMSBetaTransitionClass::UniqueThirdForbidden:
  case GGEMSBetaTransitionClass::Unclassified:
    return true;
  }

  return false;
}

// =============================================================================
// =============================================================================

[[nodiscard]] constexpr auto IsKnownModel(GGEMSBetaSpectrumModel model) noexcept
    -> bool {
  switch (model) {
  case GGEMSBetaSpectrumModel::BarePhaseSpaceDiagnostic:
  case GGEMSBetaSpectrumModel::AllowedPointCoulomb:
    return true;
  }

  return false;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto CheckedAdd(std::uint64_t lhs, std::uint64_t rhs,
                              char const *message) -> std::uint64_t {
  if (rhs > std::numeric_limits<std::uint64_t>::max() - lhs) {
    Reject(message);
  }

  return lhs + rhs;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto CheckedMultiply(std::uint64_t lhs, std::uint64_t rhs,
                                   char const *message) -> std::uint64_t {
  if (lhs != 0ULL && rhs > std::numeric_limits<std::uint64_t>::max() / lhs) {
    Reject(message);
  }

  return lhs * rhs;
}

// =============================================================================
// =============================================================================

[[nodiscard]] constexpr auto
IsExactlyRepresentableAsDouble(std::uint64_t value) noexcept -> bool {
  if (value == 0ULL) {
    return true;
  }

  auto const used_bits = static_cast<unsigned int>(std::bit_width(value));
  auto const precision_bits =
      static_cast<unsigned int>(std::numeric_limits<double>::digits);

  if (used_bits <= precision_bits) {
    return true;
  }

  unsigned int const discarded_bits = used_bits - precision_bits;
  std::uint64_t const mask = (std::uint64_t{1ULL} << discarded_bits) - 1ULL;
  return (value & mask) == 0ULL;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto
BuildRegularGrid(std::uint64_t endpoint_milli_eV,
                 std::uint64_t target_maximum_bin_width_milli_eV)
    -> RegularGrid {
  if (target_maximum_bin_width_milli_eV == 0ULL) {
    Reject("Beta-spectrum target maximum bin width must be strictly positive.");
  }

  std::uint64_t bin_count =
      endpoint_milli_eV / target_maximum_bin_width_milli_eV;
  std::uint64_t const remainder =
      endpoint_milli_eV % target_maximum_bin_width_milli_eV;

  if (remainder != 0ULL) {
    if (bin_count == std::numeric_limits<std::uint64_t>::max()) {
      Reject("Beta-spectrum grid bin-count arithmetic overflows.");
    }
    ++bin_count;
  }

  bin_count = std::max<std::uint64_t>(2ULL, bin_count);

  if (bin_count >
      static_cast<std::uint64_t>(std::numeric_limits<std::uint32_t>::max())) {
    Reject("Beta-spectrum bin count exceeds the current uint32 energy-table "
           "storage.");
  }

  std::uint64_t bin_width = endpoint_milli_eV / bin_count;
  bin_width -= bin_width & 1ULL;

  if (bin_width == 0ULL) {
    Reject("Beta-spectrum endpoint and target width cannot form two positive "
           "even-width bins.");
  }

  std::uint64_t covered_width = CheckedMultiply(
      bin_count, bin_width,
      "Beta-spectrum grid covered-width multiplication overflows.");
  std::uint64_t lower_edge = endpoint_milli_eV - covered_width;

  if (lower_edge == 0ULL) {
    if (bin_width <= 2ULL) {
      Reject("Beta-spectrum grid cannot preserve a strictly positive first "
             "lower edge.");
    }

    bin_width -= 2ULL;
    covered_width = CheckedMultiply(
        bin_count, bin_width,
        "Beta-spectrum adjusted covered-width multiplication overflows.");
    lower_edge = endpoint_milli_eV - covered_width;
  }

  if (lower_edge == 0ULL || (bin_width & 1ULL) != 0ULL) {
    Reject("Beta-spectrum grid failed its integer-edge invariants.");
  }

  std::uint64_t const half_width = bin_width / 2ULL;
  std::uint64_t const first_center = CheckedAdd(
      lower_edge, half_width, "Beta-spectrum first center overflows.");
  std::uint64_t const center_span =
      CheckedMultiply(bin_count - 1ULL, bin_width,
                      "Beta-spectrum center-span multiplication overflows.");
  std::uint64_t const last_center = CheckedAdd(
      first_center, center_span, "Beta-spectrum last center overflows.");
  std::uint64_t const upper_edge = CheckedAdd(
      last_center, half_width, "Beta-spectrum upper edge overflows.");

  if (upper_edge != endpoint_milli_eV) {
    Reject("Beta-spectrum grid does not end at the exact endpoint.");
  }

  auto const count = static_cast<std::size_t>(bin_count);
  std::vector<double> centers;

  if (count > centers.max_size()) {
    Reject("Beta-spectrum center count exceeds host container capacity.");
  }

  centers.reserve(count);
  std::uint64_t center = first_center;

  for (std::size_t index = 0U; index < count; ++index) {
    if (!IsExactlyRepresentableAsDouble(center)) {
      Reject("Beta-spectrum integer center is not exactly representable by "
             "the current double energy API.");
    }

    auto const converted = static_cast<double>(center);

    if (!std::isfinite(converted)) {
      Reject("Beta-spectrum center is not finite after double conversion.");
    }

    centers.push_back(converted);

    if (index + 1U < count) {
      center = CheckedAdd(center, bin_width,
                          "Beta-spectrum center progression overflows.");
    }
  }

  return {.lower_edge_milli_eV = lower_edge,
          .upper_edge_milli_eV = upper_edge,
          .bin_width_milli_eV = bin_width,
          .centers_milli_eV = std::move(centers)};
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto EvaluateDensity(long double kinetic_energy_milli_eV,
                                   GGEMSBetaTransition const &transition,
                                   GGEMSBetaSpectrumModel model)
    -> long double {
  auto const endpoint = static_cast<long double>(
      transition.GetEndpointKineticEnergyMilliElectronVolt());

  if (kinetic_energy_milli_eV < 0.0L || kinetic_energy_milli_eV >= endpoint) {
    return 0.0L;
  }

  long double const scaled_kinetic_energy =
      kinetic_energy_milli_eV / k_electron_rest_energy_milli_eV;
  long double const total_energy = 1.0L + scaled_kinetic_energy;
  long double const momentum =
      std::sqrt(scaled_kinetic_energy * (scaled_kinetic_energy + 2.0L));
  long double const neutrino_energy =
      (endpoint - kinetic_energy_milli_eV) / k_electron_rest_energy_milli_eV;
  long double const neutrino_factor = neutrino_energy * neutrino_energy;
  long double density{0.0L};

  if (model == GGEMSBetaSpectrumModel::BarePhaseSpaceDiagnostic) {
    density = momentum * total_energy * neutrino_factor;
  } else {
    long double const coulomb_numerator =
        2.0L * std::numbers::pi_v<long double> * k_fine_structure_constant *
        static_cast<long double>(transition.GetDaughterAtomicNumber()) *
        total_energy;
    long double momentum_fermi_factor{0.0L};
    bool density_evaluated{false};

    if (kinetic_energy_milli_eV == 0.0L) {
      momentum_fermi_factor = transition.GetSign() == GGEMSBetaSign::Minus
                                  ? coulomb_numerator
                                  : 0.0L;
    } else {
      long double const x = coulomb_numerator / momentum;

      if (transition.GetSign() == GGEMSBetaSign::Minus) {
        momentum_fermi_factor = coulomb_numerator / (-std::expm1(-x));
      } else if (x > k_beta_plus_asymptotic_threshold) {
        long double const exp_minus_x = std::exp(-x);
        long double const log_density =
            std::log(total_energy) + (2.0L * std::log(neutrino_energy)) +
            std::log(coulomb_numerator) - x - std::log1p(-exp_minus_x);
        density = std::exp(log_density);
        density_evaluated = true;
      } else {
        momentum_fermi_factor = coulomb_numerator / std::expm1(x);
      }
    }

    if (!density_evaluated) {
      density = total_energy * neutrino_factor * momentum_fermi_factor;
    }
  }

  if (!std::isfinite(density) || density < 0.0L) {
    Reject("Beta-spectrum model produced a non-finite or negative density.");
  }

  return density;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto IntegrateInterval(long double lower_milli_eV,
                                     long double upper_milli_eV,
                                     GGEMSBetaTransition const &transition,
                                     GGEMSBetaSpectrumModel model)
    -> IntegralPair {
  if (!(upper_milli_eV > lower_milli_eV)) {
    return {};
  }

  long double const step =
      (upper_milli_eV - lower_milli_eV) /
      static_cast<long double>(k_simpson_subinterval_count);
  CompensatedSum weighted_mass;
  CompensatedSum weighted_moment;

  for (std::size_t node = 0U; node <= k_simpson_subinterval_count; ++node) {
    long double const energy =
        node == k_simpson_subinterval_count
            ? upper_milli_eV
            : lower_milli_eV + (static_cast<long double>(node) * step);
    long double const density = EvaluateDensity(energy, transition, model);

    long double coefficient = 2.0L;

    if (node == 0U || node == k_simpson_subinterval_count) {
      coefficient = 1.0L;
    } else if ((node & 1U) != 0U) {
      coefficient = 4.0L;
    }

    weighted_mass.Add(coefficient * density);
    weighted_moment.Add(coefficient * energy * density);
  }

  long double const scale = step / 3.0L;
  IntegralPair const result{.mass = scale * weighted_mass.Get(),
                            .first_moment_milli_eV =
                                scale * weighted_moment.Get()};

  if (!std::isfinite(result.mass) ||
      !std::isfinite(result.first_moment_milli_eV) || result.mass < 0.0L ||
      result.first_moment_milli_eV < 0.0L) {
    Reject("Beta-spectrum quadrature produced an invalid integral.");
  }

  return result;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto MeanRegularOffset(std::uint64_t width,
                                     std::uint64_t ticket_count)
    -> long double {
  std::uint64_t const quotient = width / ticket_count;
  std::uint64_t const remainder = width % ticket_count;
  auto const last_ticket = static_cast<long double>(ticket_count - 1ULL);
  long double floor_sum{0.0L};

  if (remainder != 0ULL) {
    floor_sum =
        ((static_cast<long double>(remainder - 1ULL) * last_ticket) +
         static_cast<long double>(std::gcd(remainder, ticket_count) - 1ULL)) /
        2.0L;
  }

  return ((static_cast<long double>(quotient) * last_ticket) / 2.0L) +
         (floor_sum / static_cast<long double>(ticket_count));
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto
ComputeRepresentedMean(sources::GGEMSEnergyDistribution const &distribution)
    -> long double {
  auto const centers = distribution.GetEnergyValuesMilliElectronVolt();
  auto const ticket_bounds = distribution.GetCumulativeTicketUpperBounds();
  std::uint64_t const width =
      distribution.GetRegularBinWidthMilliElectronVolt();
  std::uint64_t const half_width = width / 2ULL;
  std::uint64_t previous_upper{0ULL};
  CompensatedSum ticket_weighted_energy;

  for (std::size_t index = 0U; index < centers.size(); ++index) {
    std::uint64_t const ticket_count = ticket_bounds[index] - previous_upper;

    if (ticket_count == 0ULL || centers[index] <= half_width) {
      Reject("Built beta spectrum violates reachable regular-bin invariants.");
    }

    std::uint64_t const lower_edge = centers[index] - half_width;
    long double const mean_energy = static_cast<long double>(lower_edge) +
                                    MeanRegularOffset(width, ticket_count);
    ticket_weighted_energy.Add(static_cast<long double>(ticket_count) *
                               mean_energy);
    previous_upper = ticket_bounds[index];
  }

  if (previous_upper != sources::k_energy_ticket_space_size) {
    Reject("Built beta spectrum does not cover the complete ticket space.");
  }

  return ticket_weighted_energy.Get() /
         static_cast<long double>(sources::k_energy_ticket_space_size);
}

[[nodiscard]] auto
BuildDiagnostics(GGEMSBetaSpectrumModel model, RegularGrid const &grid,
                 IntegralPair const &excluded, long double represented_integral,
                 long double full_integral, long double full_first_moment,
                 sources::GGEMSEnergyDistribution const &distribution)
    -> GGEMSBetaSpectrumDiagnostics {
  auto const weights = distribution.GetRelativeWeights();
  auto const ticket_bounds = distribution.GetCumulativeTicketUpperBounds();
  CompensatedSum stored_weight_sum;
  long double minimum_probability =
      std::numeric_limits<long double>::infinity();
  std::uint64_t minimum_ticket_count =
      std::numeric_limits<std::uint64_t>::max();
  std::uint64_t previous_upper{0ULL};

  for (double const weight : weights) {
    stored_weight_sum.Add(static_cast<long double>(weight));
  }

  long double const weight_sum = stored_weight_sum.Get();

  if (!std::isfinite(weight_sum) || !(weight_sum > 0.0L)) {
    Reject("Built beta spectrum has an invalid stored weight sum.");
  }

  for (std::size_t index = 0U; index < weights.size(); ++index) {
    long double const probability =
        static_cast<long double>(weights[index]) / weight_sum;
    std::uint64_t const ticket_count = ticket_bounds[index] - previous_upper;

    if (!(probability > 0.0L) || ticket_count == 0ULL) {
      Reject("Built beta spectrum lost a physically positive bin.");
    }

    minimum_probability = std::min(minimum_probability, probability);
    minimum_ticket_count = std::min(minimum_ticket_count, ticket_count);
    previous_upper = ticket_bounds[index];
  }

  long double const excluded_probability = excluded.mass / full_integral;
  long double const continuous_mean = full_first_moment / full_integral;
  long double const represented_mean = ComputeRepresentedMean(distribution);

  if (!std::isfinite(excluded_probability) || excluded_probability < 0.0L ||
      excluded_probability > 1.0L || !std::isfinite(continuous_mean) ||
      !std::isfinite(represented_mean)) {
    Reject("Beta-spectrum diagnostics are not finite physical values.");
  }

  return {
      .model = model,
      .lower_edge_milli_eV = grid.lower_edge_milli_eV,
      .upper_edge_milli_eV = grid.upper_edge_milli_eV,
      .bin_width_milli_eV = grid.bin_width_milli_eV,
      .bin_count = grid.centers_milli_eV.size(),
      .full_unnormalized_integral = full_integral,
      .represented_unnormalized_integral = represented_integral,
      .excluded_probability = excluded_probability,
      .continuous_mean_energy_milli_eV = continuous_mean,
      .represented_mean_energy_milli_eV = represented_mean,
      .normalization_residual = std::abs(weight_sum - 1.0L),
      .minimum_positive_bin_probability = minimum_probability,
      .minimum_assigned_ticket_count = minimum_ticket_count,
  };
}

} // namespace

// =============================================================================
// =============================================================================

GGEMSBetaTransition::GGEMSBetaTransition(
    GGEMSBetaSign sign, std::uint32_t daughter_atomic_number,
    std::uint32_t daughter_mass_number,
    std::uint64_t endpoint_kinetic_energy_milli_eV,
    GGEMSBetaTransitionClass transition_class)
    : sign_{sign}, daughter_atomic_number_{daughter_atomic_number},
      daughter_mass_number_{daughter_mass_number},
      endpoint_kinetic_energy_milli_eV_{endpoint_kinetic_energy_milli_eV},
      transition_class_{transition_class} {
  if (!IsKnownSign(sign_)) {
    Reject("Beta-transition sign is not recognized.");
  }
  if (!IsKnownTransitionClass(transition_class_)) {
    Reject("Beta-transition classification is not recognized.");
  }
  if (daughter_atomic_number_ == 0U) {
    Reject("Beta-transition daughter atomic number must be positive.");
  }
  if (daughter_mass_number_ == 0U ||
      daughter_mass_number_ < daughter_atomic_number_) {
    Reject("Beta-transition daughter mass number must be positive and not "
           "smaller than the daughter atomic number.");
  }
  if (endpoint_kinetic_energy_milli_eV_ == 0ULL) {
    Reject("Beta-transition endpoint kinetic energy must be positive.");
  }
}

// =============================================================================
// =============================================================================

auto BuildBetaSpectrum(GGEMSBetaTransition const &transition,
                       GGEMSBetaSpectrumBuildOptions const &options)
    -> GGEMSBetaSpectrumBuildResult {
  if (!IsKnownModel(options.model)) {
    Reject("Beta-spectrum model is not recognized.");
  }
  if (transition.GetTransitionClass() != GGEMSBetaTransitionClass::Allowed) {
    Reject("Beta-spectrum builder supports only explicitly Allowed "
           "transitions.");
  }

  RegularGrid const grid =
      BuildRegularGrid(transition.GetEndpointKineticEnergyMilliElectronVolt(),
                       options.grid.target_maximum_bin_width_milli_eV);
  IntegralPair const excluded = IntegrateInterval(
      0.0L, static_cast<long double>(grid.lower_edge_milli_eV), transition,
      options.model);
  std::vector<long double> bin_masses;
  bin_masses.reserve(grid.centers_milli_eV.size());
  CompensatedSum represented_mass_sum;
  CompensatedSum represented_moment_sum;
  std::uint64_t lower_edge = grid.lower_edge_milli_eV;

  for (std::size_t index = 0U; index < grid.centers_milli_eV.size(); ++index) {
    std::uint64_t const upper_edge =
        CheckedAdd(lower_edge, grid.bin_width_milli_eV,
                   "Beta-spectrum bin upper edge overflows.");
    IntegralPair const bin = IntegrateInterval(
        static_cast<long double>(lower_edge),
        static_cast<long double>(upper_edge), transition, options.model);

    if (!(bin.mass > 0.0L)) {
      Reject("A physically positive beta-spectrum bin integrated to zero.");
    }

    bin_masses.push_back(bin.mass);
    represented_mass_sum.Add(bin.mass);
    represented_moment_sum.Add(bin.first_moment_milli_eV);
    lower_edge = upper_edge;
  }

  if (lower_edge != grid.upper_edge_milli_eV) {
    Reject("Beta-spectrum integration did not reach the exact endpoint.");
  }

  long double const represented_integral = represented_mass_sum.Get();
  long double const full_integral = represented_integral + excluded.mass;
  long double const full_first_moment =
      represented_moment_sum.Get() + excluded.first_moment_milli_eV;

  if (!std::isfinite(represented_integral) || !(represented_integral > 0.0L) ||
      !std::isfinite(full_integral) || !(full_integral > 0.0L) ||
      !std::isfinite(full_first_moment) || full_first_moment < 0.0L) {
    Reject("Beta-spectrum normalization integrals are invalid.");
  }

  long double const comparison_tolerance =
      64.0L * std::numeric_limits<long double>::epsilon() *
      std::max(1.0L, full_integral);

  if (excluded.mass > full_integral + comparison_tolerance) {
    Reject("Excluded beta-spectrum integral exceeds the full integral.");
  }

  std::vector<double> normalized_masses;
  normalized_masses.reserve(bin_masses.size());

  for (long double const mass : bin_masses) {
    long double const probability = mass / represented_integral;
    auto const converted = static_cast<double>(probability);

    if (!std::isfinite(probability) || !(probability > 0.0L) ||
        !std::isfinite(converted) || !(converted > 0.0)) {
      Reject("A positive beta-spectrum probability is not representable by "
             "the current double weight API.");
    }

    normalized_masses.push_back(converted);
  }

  auto distribution = sources::GGEMSEnergyDistribution::BuildRegularSpectrum(
      grid.centers_milli_eV, normalized_masses, "meV");
  auto const stored_centers = distribution.GetEnergyValuesMilliElectronVolt();
  std::uint64_t expected_center =
      grid.lower_edge_milli_eV + (grid.bin_width_milli_eV / 2ULL);

  for (std::size_t index = 0U; index < stored_centers.size(); ++index) {
    if (stored_centers[index] != expected_center) {
      Reject("Beta-spectrum center failed the existing energy API "
             "round-trip.");
    }
    if (index + 1U < stored_centers.size()) {
      expected_center =
          CheckedAdd(expected_center, grid.bin_width_milli_eV,
                     "Beta-spectrum round-trip center progression overflows.");
    }
  }

  auto diagnostics =
      BuildDiagnostics(options.model, grid, excluded, represented_integral,
                       full_integral, full_first_moment, distribution);
  return {.distribution = std::move(distribution), .diagnostics = diagnostics};
}

} // namespace ggems::core::radioactivity
