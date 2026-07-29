#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numbers>
#include <numeric>
#include <string>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/radioactivity/GGEMSBetaSpectrumBuilder.hh"
#include "GGEMS/core/radioactivity/GGEMSBetaTransition.hh"
#include "GGEMS/core/sources/GGEMSEnergyDistribution.hh"
#include "GGEMS/core/sources/GGEMSSourceTypes.hh"

namespace {

// =============================================================================
// =============================================================================

using ggems::core::GGEMSExceptionBase;
using ggems::core::radioactivity::BuildBetaSpectrum;
using ggems::core::radioactivity::GGEMSBetaSign;
using ggems::core::radioactivity::GGEMSBetaSpectrumBuildOptions;
using ggems::core::radioactivity::GGEMSBetaSpectrumBuildResult;
using ggems::core::radioactivity::GGEMSBetaSpectrumModel;
using ggems::core::radioactivity::GGEMSBetaTransition;
using ggems::core::radioactivity::GGEMSBetaTransitionClass;
using ggems::core::sources::GGEMSEnergyDistribution;
using ggems::core::sources::GGEMSEnergyDistributionType;
using ggems::core::sources::k_energy_ticket_space_size;

// =============================================================================
// =============================================================================

[[nodiscard]] auto F18Transition(GGEMSBetaSign sign = GGEMSBetaSign::Plus)
    -> GGEMSBetaTransition {
  return {sign, 8U, 18U, 633'900'000ULL, GGEMSBetaTransitionClass::Allowed};
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto BuildF18(
    GGEMSBetaSpectrumModel model = GGEMSBetaSpectrumModel::AllowedPointCoulomb,
    std::uint64_t target_width = 500'000ULL) -> GGEMSBetaSpectrumBuildResult {
  return BuildBetaSpectrum(
      F18Transition(),
      {.model = model,
       .grid = {.target_maximum_bin_width_milli_eV = target_width}});
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto RefinedAllowedPointCoulombF18MeanMilliElectronVolt()
    -> long double {
  constexpr long double electron_rest_energy_milli_eV{510'998'950.69L};
  constexpr long double fine_structure_constant{7.297'352'564'3e-3L};
  constexpr long double endpoint_milli_eV{633'900'000.0L};
  constexpr std::size_t reference_midpoint_count{131'072U};
  long double const step =
      endpoint_milli_eV / static_cast<long double>(reference_midpoint_count);
  long double density_sum{0.0L};
  long double moment_sum{0.0L};

  for (std::size_t index = 0U; index < reference_midpoint_count; ++index) {
    long double const kinetic_energy =
        (static_cast<long double>(index) + 0.5L) * step;
    long double const scaled_kinetic_energy =
        kinetic_energy / electron_rest_energy_milli_eV;
    long double const total_energy = 1.0L + scaled_kinetic_energy;
    long double const momentum =
        std::sqrt(scaled_kinetic_energy * (scaled_kinetic_energy + 2.0L));
    long double const neutrino_energy =
        (endpoint_milli_eV - kinetic_energy) / electron_rest_energy_milli_eV;
    long double const coulomb_numerator =
        2.0L * std::numbers::pi_v<long double> * fine_structure_constant *
        8.0L * total_energy;
    long double const x = coulomb_numerator / momentum;
    long double const momentum_fermi_factor =
        x > 50.0L ? coulomb_numerator * std::exp(-x)
                  : coulomb_numerator / std::expm1(x);
    long double const density = total_energy * neutrino_energy *
                                neutrino_energy * momentum_fermi_factor;
    density_sum += density;
    moment_sum += kinetic_energy * density;
  }

  return moment_sum / density_sum;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto ExactTicketSamplerMeanMilliElectronVolt(
    GGEMSEnergyDistribution const &distribution) -> long double {
  auto const centers = distribution.GetEnergyValuesMilliElectronVolt();
  auto const bounds = distribution.GetCumulativeTicketUpperBounds();
  std::uint64_t const width =
      distribution.GetRegularBinWidthMilliElectronVolt();
  std::uint64_t const half_width = width / 2ULL;
  std::uint64_t previous_upper{0ULL};
  long double energy_sum{0.0L};

  for (std::size_t index = 0U; index < centers.size(); ++index) {
    std::uint64_t const ticket_count = bounds[index] - previous_upper;
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

    long double const mean_offset =
        ((static_cast<long double>(quotient) * last_ticket) / 2.0L) +
        (floor_sum / static_cast<long double>(ticket_count));
    auto const lower_edge =
        static_cast<long double>(centers[index] - half_width);
    energy_sum +=
        static_cast<long double>(ticket_count) * (lower_edge + mean_offset);
    previous_upper = bounds[index];
  }

  return energy_sum / static_cast<long double>(k_energy_ticket_space_size);
}

// =============================================================================
// =============================================================================

auto ExpectGridInvariants(GGEMSBetaSpectrumBuildResult const &result,
                          std::uint64_t endpoint, std::uint64_t target_width)
    -> void {
  auto const &diagnostics = result.diagnostics;
  auto const centers = result.distribution.GetEnergyValuesMilliElectronVolt();

  EXPECT_EQ(result.distribution.GetType(),
            GGEMSEnergyDistributionType::RegularSpectrum);
  ASSERT_GE(centers.size(), 2U);
  EXPECT_EQ(centers.size(), diagnostics.bin_count);
  EXPECT_GT(diagnostics.lower_edge_milli_eV, 0ULL);
  EXPECT_EQ(diagnostics.upper_edge_milli_eV, endpoint);
  EXPECT_GT(diagnostics.bin_width_milli_eV, 0ULL);
  EXPECT_EQ(diagnostics.bin_width_milli_eV & 1ULL, 0ULL);
  EXPECT_LE(diagnostics.bin_width_milli_eV, target_width);
  EXPECT_EQ(result.distribution.GetRegularBinWidthMilliElectronVolt(),
            diagnostics.bin_width_milli_eV);
  EXPECT_EQ(centers.front() - (diagnostics.bin_width_milli_eV / 2ULL),
            diagnostics.lower_edge_milli_eV);
  EXPECT_EQ(centers.back() + (diagnostics.bin_width_milli_eV / 2ULL), endpoint);

  for (std::size_t index = 1U; index < centers.size(); ++index) {
    EXPECT_EQ(centers[index] - centers[index - 1U],
              diagnostics.bin_width_milli_eV);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSBetaSpectrumBuilderTest, BuildsExactDefaultF18GridAndTickets) {
  auto const result = BuildF18();
  ExpectGridInvariants(result, 633'900'000ULL, 500'000ULL);

  EXPECT_EQ(result.diagnostics.bin_count, 1268U);
  EXPECT_EQ(result.diagnostics.bin_width_milli_eV, 499'920ULL);
  EXPECT_EQ(result.diagnostics.lower_edge_milli_eV, 1'440ULL);
  EXPECT_EQ(result.diagnostics.upper_edge_milli_eV, 633'900'000ULL);

  auto const tickets = result.distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(tickets.size(), 1268U);
  std::uint64_t previous{0ULL};

  for (std::uint64_t const upper : tickets) {
    EXPECT_GT(upper, previous);
    previous = upper;
  }

  EXPECT_EQ(previous, k_energy_ticket_space_size);
  EXPECT_GT(result.diagnostics.minimum_assigned_ticket_count, 0ULL);
  EXPECT_GT(result.diagnostics.minimum_positive_bin_probability, 0.0L);
}

// =============================================================================
// =============================================================================

TEST(GGEMSBetaSpectrumBuilderTest,
     F18ModelsMatchIndependentAuditMeansAndRemainDistinct) {
  auto const bare = BuildF18(GGEMSBetaSpectrumModel::BarePhaseSpaceDiagnostic);
  auto const coulomb = BuildF18(GGEMSBetaSpectrumModel::AllowedPointCoulomb);

  EXPECT_NEAR(static_cast<double>(
                  bare.diagnostics.continuous_mean_energy_milli_eV / 1.0e6L),
              240.58845, 0.001);
  EXPECT_NEAR(static_cast<double>(
                  coulomb.diagnostics.continuous_mean_energy_milli_eV / 1.0e6L),
              250.53206, 0.001);
  EXPECT_GT(coulomb.diagnostics.continuous_mean_energy_milli_eV,
            bare.diagnostics.continuous_mean_energy_milli_eV);
}

// =============================================================================
// =============================================================================

TEST(GGEMSBetaSpectrumBuilderTest,
     ContinuousMeanAgreesWithRefinedIndependentMidpointReference) {
  auto const result = BuildF18();
  long double const reference =
      RefinedAllowedPointCoulombF18MeanMilliElectronVolt();

  EXPECT_NEAR(static_cast<double>(
                  result.diagnostics.continuous_mean_energy_milli_eV / 1.0e6L),
              static_cast<double>(reference / 1.0e6L), 0.001);
}

// =============================================================================
// =============================================================================

TEST(GGEMSBetaSpectrumBuilderTest,
     WiderF18GridsStayValidAndConvergeToContinuousMean) {
  auto const fine = BuildF18();
  auto const one_keV =
      BuildF18(GGEMSBetaSpectrumModel::AllowedPointCoulomb, 1'000'000ULL);
  auto const two_keV =
      BuildF18(GGEMSBetaSpectrumModel::AllowedPointCoulomb, 2'000'000ULL);

  ExpectGridInvariants(one_keV, 633'900'000ULL, 1'000'000ULL);
  ExpectGridInvariants(two_keV, 633'900'000ULL, 2'000'000ULL);

  EXPECT_NEAR(
      static_cast<double>(one_keV.diagnostics.represented_mean_energy_milli_eV /
                          1.0e6L),
      static_cast<double>(fine.diagnostics.represented_mean_energy_milli_eV /
                          1.0e6L),
      0.0015);
  EXPECT_NEAR(
      static_cast<double>(two_keV.diagnostics.represented_mean_energy_milli_eV /
                          1.0e6L),
      static_cast<double>(fine.diagnostics.represented_mean_energy_milli_eV /
                          1.0e6L),
      0.0015);
}

// =============================================================================
// =============================================================================

TEST(GGEMSBetaSpectrumBuilderTest, ReportsFiniteNormalizedDiagnostics) {
  auto const result = BuildF18();
  auto const &diagnostics = result.diagnostics;

  EXPECT_TRUE(std::isfinite(diagnostics.full_unnormalized_integral));
  EXPECT_TRUE(std::isfinite(diagnostics.represented_unnormalized_integral));
  EXPECT_GT(diagnostics.full_unnormalized_integral, 0.0L);
  EXPECT_GT(diagnostics.represented_unnormalized_integral, 0.0L);
  EXPECT_GE(diagnostics.excluded_probability, 0.0L);
  EXPECT_LT(diagnostics.excluded_probability, 1.0e-6L);
  EXPECT_TRUE(std::isfinite(diagnostics.continuous_mean_energy_milli_eV));
  EXPECT_TRUE(std::isfinite(diagnostics.represented_mean_energy_milli_eV));
  EXPECT_LT(diagnostics.normalization_residual, 1.0e-12L);
  EXPECT_NEAR(static_cast<double>(diagnostics.represented_mean_energy_milli_eV),
              static_cast<double>(
                  ExactTicketSamplerMeanMilliElectronVolt(result.distribution)),
              0.01);
}

// =============================================================================
// =============================================================================

TEST(GGEMSBetaSpectrumBuilderTest,
     CoulombSignsEnhanceAndSuppressLowEnergyRelativeToBarePhaseSpace) {
  auto const bare = BuildF18(GGEMSBetaSpectrumModel::BarePhaseSpaceDiagnostic);
  auto const plus = BuildF18(GGEMSBetaSpectrumModel::AllowedPointCoulomb);
  auto const minus =
      BuildBetaSpectrum(F18Transition(GGEMSBetaSign::Minus),
                        {.model = GGEMSBetaSpectrumModel::AllowedPointCoulomb});
  auto const bare_weights = bare.distribution.GetRelativeWeights();
  auto const plus_weights = plus.distribution.GetRelativeWeights();
  auto const minus_weights = minus.distribution.GetRelativeWeights();
  std::size_t const middle = bare_weights.size() / 2U;

  EXPECT_LT(plus_weights.front() / plus_weights[middle],
            bare_weights.front() / bare_weights[middle]);
  EXPECT_GT(minus_weights.front() / minus_weights[middle],
            bare_weights.front() / bare_weights[middle]);
}

// =============================================================================
// =============================================================================

TEST(GGEMSBetaSpectrumBuilderTest, RejectsInvalidOptionsAndTinyGrids) {
  GGEMSBetaSpectrumBuildOptions zero_width{};
  zero_width.grid.target_maximum_bin_width_milli_eV = 0ULL;
  EXPECT_THROW((void)BuildBetaSpectrum(F18Transition(), zero_width),
               GGEMSExceptionBase);

  GGEMSBetaSpectrumBuildOptions unknown_model{};
  unknown_model.model = static_cast<GGEMSBetaSpectrumModel>(255U);
  EXPECT_THROW((void)BuildBetaSpectrum(F18Transition(), unknown_model),
               GGEMSExceptionBase);

  GGEMSBetaTransition const tiny{GGEMSBetaSign::Minus, 1U, 1U, 4ULL,
                                 GGEMSBetaTransitionClass::Allowed};
  EXPECT_THROW((void)BuildBetaSpectrum(tiny), GGEMSExceptionBase);
}

// =============================================================================
// =============================================================================

TEST(GGEMSBetaSpectrumBuilderTest,
     RejectsExtremeUnrepresentableGridBeforeAllocation) {
  GGEMSBetaTransition const extreme{GGEMSBetaSign::Minus, 1U, 1U,
                                    std::numeric_limits<std::uint64_t>::max(),
                                    GGEMSBetaTransitionClass::Allowed};
  GGEMSBetaSpectrumBuildOptions options{};
  options.grid.target_maximum_bin_width_milli_eV =
      std::numeric_limits<std::uint64_t>::max();
  EXPECT_THROW((void)BuildBetaSpectrum(extreme, options), GGEMSExceptionBase);
}

// =============================================================================
// =============================================================================

TEST(GGEMSBetaSpectrumBuilderTest,
     ReproducibleOverfineF18GridFailsPositiveTicketReachability) {
  try {
    (void)BuildF18(GGEMSBetaSpectrumModel::AllowedPointCoulomb, 100'000ULL);
    FAIL() << "Expected positive-ticket reachability rejection.";
  } catch (GGEMSExceptionBase const &exception) {
    std::string const message = exception.what();
    EXPECT_NE(message.find("reachable ticket"), std::string::npos);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSBetaSpectrumBuilderTest, LeavesCallerValuesUnchanged) {
  GGEMSBetaTransition const transition = F18Transition();
  GGEMSBetaTransition const transition_copy = transition;
  GGEMSBetaSpectrumBuildOptions options{};
  GGEMSBetaSpectrumBuildOptions const options_copy = options;

  auto result = BuildBetaSpectrum(transition, options);
  EXPECT_EQ(transition, transition_copy);
  EXPECT_EQ(options.model, options_copy.model);
  EXPECT_EQ(options.grid.target_maximum_bin_width_milli_eV,
            options_copy.grid.target_maximum_bin_width_milli_eV);

  options.grid.target_maximum_bin_width_milli_eV = 100'000ULL;
  EXPECT_THROW((void)BuildBetaSpectrum(transition, options),
               GGEMSExceptionBase);
  EXPECT_EQ(transition, transition_copy);
  EXPECT_EQ(options.grid.target_maximum_bin_width_milli_eV, 100'000ULL);

  EXPECT_EQ(result.distribution.GetTableCount(), 1268U);
}

// =============================================================================
// =============================================================================

TEST(GGEMSBetaSpectrumBuilderTest, ResultOwnsCallerIndependentData) {
  auto const build_from_temporaries = []() -> GGEMSBetaSpectrumBuildResult {
    return BuildBetaSpectrum(
        GGEMSBetaTransition{GGEMSBetaSign::Plus, 8U, 18U, 633'900'000ULL,
                            GGEMSBetaTransitionClass::Allowed},
        GGEMSBetaSpectrumBuildOptions{});
  };

  auto const result = build_from_temporaries();
  EXPECT_EQ(result.distribution.GetTableCount(), 1268U);
  EXPECT_EQ(result.distribution.GetCumulativeTicketUpperBounds().back(),
            k_energy_ticket_space_size);
}

} // namespace
