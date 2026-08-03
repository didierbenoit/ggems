#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <limits>
#include <span>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>
#include <iostream>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/sources/GGEMSEnergyDistribution.hh"
#include "GGEMS/core/sources/GGEMSSource.hh"
#include "GGEMS/core/sources/GGEMSSourceTypes.hh"
#include "GGEMS/core/sources/GGEMSSourceRecord.hh"
#include "GGEMS/core/units/GGEMSEnergyUnits.hh"
#include "GGEMS/core/units/GGEMSQuantity.hh"

namespace {

// =============================================================================
// =============================================================================

using Distribution = ggems::core::sources::GGEMSEnergyDistribution;
using DistributionType = ggems::core::sources::GGEMSEnergyDistributionType;
using GGEMSException = ggems::core::GGEMSExceptionBase;

// =============================================================================
// =============================================================================

class TemporarySpectrumFile {
public:
  TemporarySpectrumFile(std::string_view name, std::string_view content)
      : path_{std::filesystem::path{::testing::TempDir()} / name} {
    std::ofstream output{path_, std::ios::binary};
    output << content;
  }

  ~TemporarySpectrumFile() {
    std::error_code ignored;
    std::filesystem::remove(path_, ignored);
  }

  [[nodiscard]] auto GetPath() const noexcept -> std::filesystem::path const & {
    return path_;
  }

private:
  std::filesystem::path path_;
};

// =============================================================================
// =============================================================================

[[nodiscard]] auto CopyValues(Distribution const &distribution)
    -> std::vector<std::uint64_t> {
  auto values = distribution.GetEnergyValuesMilliElectronVolt();
  return {values.begin(), values.end()};
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto CopyWeights(Distribution const &distribution)
    -> std::vector<double> {
  auto values = distribution.GetRelativeWeights();
  return {values.begin(), values.end()};
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto CopyTicketBounds(Distribution const &distribution)
    -> std::vector<std::uint64_t> {
  auto values = distribution.GetCumulativeTicketUpperBounds();
  return {values.begin(), values.end()};
}

// =============================================================================
// =============================================================================

auto ExpectDistributionUnchanged(
    ggems::core::sources::GGEMSSource const &source,
    ggems::core::sources::GGEMSSourceRecord const &record,
    DistributionType type, std::uint64_t mono_energy_milli_eV,
    std::uint64_t regular_bin_width_milli_eV,
    std::vector<std::uint64_t> const &values,
    std::vector<double> const &relative_weights,
    std::vector<std::uint64_t> const &ticket_bounds) -> void {
  Distribution const &distribution = source.GetEnergyDistribution();
  EXPECT_EQ(source.GetRecord().energy_milli_eV, record.energy_milli_eV);
  EXPECT_EQ(distribution.GetType(), type);
  EXPECT_EQ(distribution.GetMonoEnergyMilliElectronVolt(),
            mono_energy_milli_eV);
  EXPECT_EQ(distribution.GetRegularBinWidthMilliElectronVolt(),
            regular_bin_width_milli_eV);
  EXPECT_EQ(CopyValues(distribution), values);
  EXPECT_EQ(CopyWeights(distribution), relative_weights);
  EXPECT_EQ(CopyTicketBounds(distribution), ticket_bounds);
}

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSEnergyDistributionTypes, StableIdentifiersAndNames) {
  using ggems::core::sources::FromKernelEnergyDistributionType;
  using ggems::core::sources::ToKernelEnergyDistributionType;
  using ggems::core::sources::ToLongName;

  EXPECT_EQ(ToKernelEnergyDistributionType(DistributionType::Unknown), 0U);
  EXPECT_EQ(ToKernelEnergyDistributionType(DistributionType::Mono), 1U);
  EXPECT_EQ(ToKernelEnergyDistributionType(DistributionType::DiscreteLines),
            2U);
  EXPECT_EQ(ToKernelEnergyDistributionType(DistributionType::RegularSpectrum),
            3U);
  EXPECT_EQ(FromKernelEnergyDistributionType(0U), DistributionType::Unknown);
  EXPECT_EQ(FromKernelEnergyDistributionType(1U), DistributionType::Mono);
  EXPECT_EQ(FromKernelEnergyDistributionType(2U),
            DistributionType::DiscreteLines);
  EXPECT_EQ(FromKernelEnergyDistributionType(3U),
            DistributionType::RegularSpectrum);
  EXPECT_EQ(FromKernelEnergyDistributionType(999U), DistributionType::Unknown);
  EXPECT_EQ(ToLongName(DistributionType::Unknown), "Unknown");
  EXPECT_EQ(ToLongName(DistributionType::Mono), "Mono");
  EXPECT_EQ(ToLongName(DistributionType::DiscreteLines), "Discrete lines");
  EXPECT_EQ(ToLongName(DistributionType::RegularSpectrum), "Regular spectrum");
}

// =============================================================================
// =============================================================================

TEST(GGEMSEnergyUnits, GenericConversionUsesExactMilliElectronVolts) {
  auto const mev =
      ggems::units::TryMakeQuantity<ggems::units::Energy>(0.120L, "MeV");
  ASSERT_TRUE(mev.has_value());

  EXPECT_EQ(mev->value, 120'000'000ULL);

  auto const zero =
      ggems::units::TryMakeQuantity<ggems::units::Energy>(0.0L, "keV");
  ASSERT_TRUE(zero.has_value());
  EXPECT_EQ(zero->value, 0ULL);

  auto const unsupported =
      ggems::units::TryMakeQuantity<ggems::units::Energy>(1.0L, "joule");
  auto const negative =
      ggems::units::TryMakeQuantity<ggems::units::Energy>(-1.0L, "keV");
  auto const nan = ggems::units::TryMakeQuantity<ggems::units::Energy>(
      std::numeric_limits<long double>::quiet_NaN(), "keV");
  auto const infinity = ggems::units::TryMakeQuantity<ggems::units::Energy>(
      std::numeric_limits<long double>::infinity(), "keV");
  auto const overflow = ggems::units::TryMakeQuantity<ggems::units::Energy>(
      std::ldexp(1.0L, 64), "meV");

  ASSERT_FALSE(negative.has_value());
  ASSERT_FALSE(nan.has_value());
  ASSERT_FALSE(infinity.has_value());
  ASSERT_FALSE(overflow.has_value());

  EXPECT_EQ(unsupported.error(),
            ggems::units::UnitConversionError::UnsupportedUnit);
  EXPECT_EQ(negative.error(), ggems::units::UnitConversionError::NegativeValue);
  EXPECT_EQ(nan.error(), ggems::units::UnitConversionError::NonFinite);
  EXPECT_EQ(infinity.error(), ggems::units::UnitConversionError::NonFinite);
  EXPECT_EQ(overflow.error(), ggems::units::UnitConversionError::OutOfRange);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEnergyDistribution, MonoIsExactAndTableFree) {
  Distribution const distribution = Distribution::BuildMono(100'000'001ULL);

  EXPECT_EQ(distribution.GetType(), DistributionType::Mono);
  EXPECT_EQ(distribution.GetMonoEnergyMilliElectronVolt(), 100'000'001ULL);
  EXPECT_EQ(distribution.GetTableCount(), 0U);
  EXPECT_TRUE(distribution.GetEnergyValuesMilliElectronVolt().empty());
  EXPECT_TRUE(distribution.GetRelativeWeights().empty());
  EXPECT_TRUE(distribution.GetCumulativeTicketUpperBounds().empty());

  auto const record = distribution.BuildRecord(99ULL);
  EXPECT_EQ(record.distribution_type, 1U);
  EXPECT_EQ(record.table_offset, 0ULL);
  EXPECT_EQ(record.table_count, 0U);
  EXPECT_EQ(record.regular_bin_width_milli_eV, 0ULL);

  EXPECT_THROW(static_cast<void>(Distribution::BuildMono(0ULL)),
               GGEMSException);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEnergyDistribution, DiscreteLinesNormalizeAndPreserveExactLines) {
  constexpr std::array<double, 4U> energies{40.0, 60.0, 80.0, 120.0};
  constexpr std::array<double, 4U> weights{1.0, 0.0, 2.0, 1.0};

  Distribution const distribution =
      Distribution::BuildDiscreteLines(energies, weights, "keV");

  EXPECT_EQ(distribution.GetType(), DistributionType::DiscreteLines);
  EXPECT_EQ(CopyValues(distribution),
            (std::vector<std::uint64_t>{40'000'000ULL, 60'000'000ULL,
                                        80'000'000ULL, 120'000'000ULL}));
  EXPECT_EQ(CopyWeights(distribution),
            (std::vector<double>{1.0, 0.0, 2.0, 1.0}));
  EXPECT_EQ(CopyTicketBounds(distribution),
            (std::vector<std::uint64_t>{1'073'741'824ULL, 1'073'741'824ULL,
                                        3'221'225'472ULL, 4'294'967'296ULL}));

  auto const record = distribution.BuildRecord(7ULL);
  EXPECT_EQ(record.distribution_type, 2U);
  EXPECT_EQ(record.table_offset, 7ULL);
  EXPECT_EQ(record.table_count, 4U);
  EXPECT_EQ(record.regular_bin_width_milli_eV, 0ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEnergyDistribution, DiscreteLinesRejectInvalidInput) {
  constexpr std::array<double, 2U> valid_energy{40.0, 80.0};
  constexpr std::array<double, 2U> valid_weight{1.0, 1.0};
  constexpr std::array<double, 0U> empty_energy{};
  constexpr std::array<double, 0U> empty_weight{};
  constexpr std::array<double, 1U> one_energy{40.0};
  constexpr std::array<double, 1U> one_weight{1.0};

  EXPECT_THROW(static_cast<void>(Distribution::BuildDiscreteLines(
                   empty_energy, empty_weight, "keV")),
               GGEMSException);
  EXPECT_THROW(static_cast<void>(Distribution::BuildDiscreteLines(
                   one_energy, one_weight, "keV")),
               GGEMSException);
  EXPECT_THROW(static_cast<void>(Distribution::BuildDiscreteLines(
                   std::span<double const>{valid_energy}.first(1U),
                   valid_weight, "keV")),
               GGEMSException);
  EXPECT_THROW(static_cast<void>(Distribution::BuildDiscreteLines(
                   valid_energy,
                   std::span<double const>{valid_weight}.first(1U), "keV")),
               GGEMSException);

  constexpr std::array<double, 2U> duplicate{40.0, 40.0};
  constexpr std::array<double, 2U> descending{80.0, 40.0};
  constexpr std::array<double, 2U> zero_energy{0.0, 40.0};
  constexpr std::array<double, 2U> negative_weight{-1.0, 1.0};
  constexpr std::array<double, 2U> zero_weight{0.0, 0.0};

  EXPECT_THROW(static_cast<void>(Distribution::BuildDiscreteLines(
                   duplicate, valid_weight, "keV")),
               GGEMSException);
  EXPECT_THROW(static_cast<void>(Distribution::BuildDiscreteLines(
                   descending, valid_weight, "keV")),
               GGEMSException);
  EXPECT_THROW(static_cast<void>(Distribution::BuildDiscreteLines(
                   zero_energy, valid_weight, "keV")),
               GGEMSException);
  EXPECT_THROW(static_cast<void>(Distribution::BuildDiscreteLines(
                   valid_energy, negative_weight, "keV")),
               GGEMSException);
  EXPECT_THROW(static_cast<void>(Distribution::BuildDiscreteLines(
                   valid_energy, zero_weight, "keV")),
               GGEMSException);

  std::array<double, 2U> nonfinite_energy{
      40.0, std::numeric_limits<double>::quiet_NaN()};
  std::array<double, 2U> nonfinite_weight{
      1.0, std::numeric_limits<double>::infinity()};

  EXPECT_THROW(static_cast<void>(Distribution::BuildDiscreteLines(
                   nonfinite_energy, valid_weight, "keV")),
               GGEMSException);
  EXPECT_THROW(static_cast<void>(Distribution::BuildDiscreteLines(
                   valid_energy, nonfinite_weight, "keV")),
               GGEMSException);
  EXPECT_THROW(static_cast<void>(Distribution::BuildDiscreteLines(
                   valid_energy, valid_weight, "invalid")),
               GGEMSException);
  EXPECT_THROW(static_cast<void>(Distribution::BuildDiscreteLines(
                   std::array<double, 2U>{1.0, std::ldexp(1.0, 64)},
                   valid_weight, "meV")),
               GGEMSException);
}

// =============================================================================
// =============================================================================
TEST(GGEMSEnergyDistribution,
     LargestRemainderUsesStableIndexTieBreakingAndExactTotal) {
  constexpr std::array<double, 3U> energies{40.0, 80.0, 120.0};
  constexpr std::array<double, 3U> equal_weights{1.0, 1.0, 1.0};

  Distribution const equal =
      Distribution::BuildDiscreteLines(energies, equal_weights, "keV");

  EXPECT_EQ(CopyTicketBounds(equal),
            (std::vector<std::uint64_t>{1'431'655'766ULL, 2'863'311'531ULL,
                                        4'294'967'296ULL}));

  constexpr std::array<double, 3U> rational_weights{1.0, 2.0, 3.0};
  Distribution const rational =
      Distribution::BuildDiscreteLines(energies, rational_weights, "keV");
  auto const bounds = CopyTicketBounds(rational);
  ASSERT_EQ(bounds.size(), 3U);

  std::array<std::uint64_t, 3U> const ticket_counts{
      bounds[0U], bounds[1U] - bounds[0U], bounds[2U] - bounds[1U]};
  EXPECT_EQ(ticket_counts,
            (std::array<std::uint64_t, 3U>{715'827'883ULL, 1'431'655'765ULL,
                                           2'147'483'648ULL}));
  EXPECT_EQ(bounds.back(), 4'294'967'296ULL);

  constexpr std::array<std::uint64_t, 3U> integer_weights{1ULL, 2ULL, 3ULL};

  for (std::size_t index = 0U; index < ticket_counts.size(); ++index) {
    std::uint64_t const effective_scaled = ticket_counts[index] * 6ULL;
    std::uint64_t const ideal_scaled =
        integer_weights[index] * 4'294'967'296ULL;
    std::uint64_t const error = effective_scaled > ideal_scaled
                                    ? effective_scaled - ideal_scaled
                                    : ideal_scaled - effective_scaled;
    EXPECT_LT(error, 6ULL);
  }
}

// =============================================================================
// =============================================================================
TEST(GGEMSEnergyDistribution,
     RejectsPositiveWeightWithoutTicketAndPreservesSourceState) {
  constexpr std::array<double, 3U> valid_energies{40.0, 80.0, 120.0};
  constexpr std::array<double, 3U> valid_weights{1.0, 2.0, 1.0};
  constexpr std::array<double, 2U> rejected_energies{40.0, 80.0};
  constexpr std::array<double, 2U> rejected_weights{1.0, 0x1p-64};

  ggems::core::sources::GGEMSSource source{};
  source.SetDiscreteEnergyLines(valid_energies, valid_weights, "keV");

  auto const record = source.GetRecord();
  Distribution const &distribution = source.GetEnergyDistribution();
  auto const type = distribution.GetType();
  auto const mono_energy = distribution.GetMonoEnergyMilliElectronVolt();
  auto const regular_width = distribution.GetRegularBinWidthMilliElectronVolt();
  auto const values = CopyValues(distribution);
  auto const weights = CopyWeights(distribution);
  auto const ticket_bounds = CopyTicketBounds(distribution);

  EXPECT_THROW(
      source.SetDiscreteEnergyLines(rejected_energies, rejected_weights, "keV"),
      GGEMSException);
  ExpectDistributionUnchanged(source, record, type, mono_energy, regular_width,
                              values, weights, ticket_bounds);
}

// =============================================================================
// =============================================================================
TEST(GGEMSEnergyDistribution, RegularSpectrumUsesCenterDefinedEvenGrid) {
  constexpr std::array<double, 3U> centers{20.0, 22.0, 24.0};
  constexpr std::array<double, 3U> weights{1.0, 2.0, 1.0};

  Distribution const distribution =
      Distribution::BuildRegularSpectrum(centers, weights, "keV");

  EXPECT_EQ(distribution.GetType(), DistributionType::RegularSpectrum);
  EXPECT_EQ(distribution.GetRegularBinWidthMilliElectronVolt(), 2'000'000ULL);
  EXPECT_EQ(CopyValues(distribution),
            (std::vector<std::uint64_t>{20'000'000ULL, 22'000'000ULL,
                                        24'000'000ULL}));
  EXPECT_EQ(CopyWeights(distribution), (std::vector<double>{1.0, 2.0, 1.0}));
  EXPECT_EQ(CopyTicketBounds(distribution),
            (std::vector<std::uint64_t>{1'073'741'824ULL, 3'221'225'472ULL,
                                        4'294'967'296ULL}));

  std::uint64_t const lower_edge =
      distribution.GetEnergyValuesMilliElectronVolt().front() -
      (distribution.GetRegularBinWidthMilliElectronVolt() / 2ULL);
  EXPECT_EQ(lower_edge, 19'000'000ULL);
}

// =============================================================================
// =============================================================================
TEST(GGEMSEnergyDistribution, RegularSpectrumRejectsInvalidGridAndWeights) {
  constexpr std::array<double, 3U> valid_centers{20.0, 22.0, 24.0};
  constexpr std::array<double, 3U> valid_weights{1.0, 2.0, 1.0};
  constexpr std::array<double, 1U> one_center{20.0};
  constexpr std::array<double, 1U> one_weight{1.0};
  constexpr std::array<double, 3U> irregular{20.0, 22.0, 25.0};
  constexpr std::array<double, 3U> duplicate{20.0, 22.0, 22.0};
  constexpr std::array<double, 3U> descending{24.0, 22.0, 20.0};
  constexpr std::array<double, 3U> zero_energy{0.0, 2.0, 4.0};
  constexpr std::array<double, 3U> negative_energy{-2.0, 2.0, 4.0};
  constexpr std::array<double, 3U> negative_weight{1.0, -1.0, 1.0};
  constexpr std::array<double, 3U> zero_weight{0.0, 0.0, 0.0};

  EXPECT_THROW(static_cast<void>(Distribution::BuildRegularSpectrum(
                   one_center, one_weight, "keV")),
               GGEMSException);
  EXPECT_THROW(static_cast<void>(Distribution::BuildRegularSpectrum(
                   std::span<double const>{valid_centers}.first(2U),
                   valid_weights, "keV")),
               GGEMSException);
  EXPECT_THROW(static_cast<void>(Distribution::BuildRegularSpectrum(
                   valid_centers,
                   std::span<double const>{valid_weights}.first(2U), "keV")),
               GGEMSException);
  EXPECT_THROW(static_cast<void>(Distribution::BuildRegularSpectrum(
                   irregular, valid_weights, "keV")),
               GGEMSException);
  EXPECT_THROW(static_cast<void>(Distribution::BuildRegularSpectrum(
                   duplicate, valid_weights, "keV")),
               GGEMSException);
  EXPECT_THROW(static_cast<void>(Distribution::BuildRegularSpectrum(
                   descending, valid_weights, "keV")),
               GGEMSException);
  EXPECT_THROW(static_cast<void>(Distribution::BuildRegularSpectrum(
                   zero_energy, valid_weights, "keV")),
               GGEMSException);
  EXPECT_THROW(static_cast<void>(Distribution::BuildRegularSpectrum(
                   negative_energy, valid_weights, "keV")),
               GGEMSException);
  EXPECT_THROW(static_cast<void>(Distribution::BuildRegularSpectrum(
                   valid_centers, negative_weight, "keV")),
               GGEMSException);
  EXPECT_THROW(static_cast<void>(Distribution::BuildRegularSpectrum(
                   valid_centers, zero_weight, "keV")),
               GGEMSException);

  constexpr std::array<double, 2U> odd_width_meV{10.0, 11.0};
  constexpr std::array<double, 2U> nonpositive_lower_edge_meV{1.0, 3.0};
  constexpr std::array<double, 2U> two_weights{1.0, 1.0};
  std::array<double, 3U> nonfinite_center{
      20.0, 22.0, std::numeric_limits<double>::quiet_NaN()};
  std::array<double, 3U> nonfinite_weight{
      1.0, std::numeric_limits<double>::infinity(), 1.0};

  EXPECT_THROW(static_cast<void>(Distribution::BuildRegularSpectrum(
                   odd_width_meV, two_weights, "meV")),
               GGEMSException);
  EXPECT_THROW(static_cast<void>(Distribution::BuildRegularSpectrum(
                   nonpositive_lower_edge_meV, two_weights, "meV")),
               GGEMSException);
  EXPECT_THROW(static_cast<void>(Distribution::BuildRegularSpectrum(
                   nonfinite_center, valid_weights, "keV")),
               GGEMSException);
  EXPECT_THROW(static_cast<void>(Distribution::BuildRegularSpectrum(
                   valid_centers, nonfinite_weight, "keV")),
               GGEMSException);
  EXPECT_THROW(static_cast<void>(Distribution::BuildRegularSpectrum(
                   std::array<double, 2U>{10.0, std::ldexp(1.0, 64)},
                   two_weights, "meV")),
               GGEMSException);
}

// =============================================================================
// =============================================================================
TEST(GGEMSEnergyDistribution, InvalidSetterPreservesPreviousState) {
  constexpr std::array<double, 3U> energies{40.0, 80.0, 120.0};
  constexpr std::array<double, 3U> weights{1.0, 2.0, 1.0};
  TemporarySpectrumFile const malformed{"ggems-energy-strong-guarantee.dat",
                                        "0.020 1.0\nmalformed data\n"};

  ggems::core::sources::GGEMSSource source{};
  source.SetDiscreteEnergyLines(energies, weights, "keV");

  auto const record = source.GetRecord();
  Distribution const &distribution = source.GetEnergyDistribution();
  auto const type = distribution.GetType();
  auto const mono_energy = distribution.GetMonoEnergyMilliElectronVolt();
  auto const regular_width = distribution.GetRegularBinWidthMilliElectronVolt();
  auto const values = CopyValues(distribution);
  auto const relative_weights = CopyWeights(distribution);
  auto const ticket_bounds = CopyTicketBounds(distribution);

  auto const expect_unchanged = [&]() -> void {
    ExpectDistributionUnchanged(source, record, type, mono_energy,
                                regular_width, values, relative_weights,
                                ticket_bounds);
  };

  EXPECT_THROW(source.SetEnergyMilliElectronVolt(0ULL), GGEMSException);
  expect_unchanged();

  EXPECT_THROW(source.SetRegularEnergySpectrum(
                   std::array<double, 3U>{20.0, 22.0, 25.0}, weights, "keV"),
               GGEMSException);
  expect_unchanged();

  EXPECT_THROW(source.SetDiscreteEnergyLines(
                   std::array<double, 3U>{40.0, 40.0, 120.0}, weights, "keV"),
               GGEMSException);
  expect_unchanged();

  EXPECT_THROW(source.LoadRegularEnergySpectrum(malformed.GetPath(), "MeV"),
               GGEMSException);
  expect_unchanged();

  source.SetEnergyMilliElectronVolt(90'000'000ULL);
  EXPECT_EQ(source.GetEnergyDistribution().GetType(), DistributionType::Mono);
  EXPECT_TRUE(source.GetEnergyDistribution()
                  .GetEnergyValuesMilliElectronVolt()
                  .empty());
  EXPECT_TRUE(source.GetEnergyDistribution().GetRelativeWeights().empty());
  EXPECT_EQ(source.GetRecord().energy_milli_eV, 90'000'000ULL);
}

// =============================================================================
// =============================================================================
TEST(GGEMSEnergyDistribution, SourceOwnsCopiedCallerTables) {
  std::vector<double> line_energies{40.0, 80.0};
  std::vector<double> line_weights{1.0, 3.0};

  ggems::core::sources::GGEMSSource source{};
  source.SetDiscreteEnergyLines(line_energies, line_weights, "keV");

  auto const expected_line_values = CopyValues(source.GetEnergyDistribution());
  auto const expected_line_weights =
      CopyWeights(source.GetEnergyDistribution());
  auto const expected_line_tickets =
      CopyTicketBounds(source.GetEnergyDistribution());

  line_energies.assign(2U, 999.0);
  line_weights.clear();

  EXPECT_EQ(CopyValues(source.GetEnergyDistribution()), expected_line_values);
  EXPECT_EQ(CopyWeights(source.GetEnergyDistribution()), expected_line_weights);
  EXPECT_EQ(CopyTicketBounds(source.GetEnergyDistribution()),
            expected_line_tickets);

  std::vector<double> bin_centers{20.0, 22.0, 24.0};
  std::vector<double> bin_weights{1.0, 2.0, 1.0};
  source.SetRegularEnergySpectrum(bin_centers, bin_weights, "keV");

  auto const expected_bin_values = CopyValues(source.GetEnergyDistribution());
  auto const expected_bin_weights = CopyWeights(source.GetEnergyDistribution());
  auto const expected_bin_tickets =
      CopyTicketBounds(source.GetEnergyDistribution());

  bin_centers.clear();
  bin_weights.assign(3U, 0.0);

  EXPECT_EQ(CopyValues(source.GetEnergyDistribution()), expected_bin_values);
  EXPECT_EQ(CopyWeights(source.GetEnergyDistribution()), expected_bin_weights);
  EXPECT_EQ(CopyTicketBounds(source.GetEnergyDistribution()),
            expected_bin_tickets);
}

// =============================================================================
// =============================================================================
TEST(GGEMSEnergyDistribution, ParserAcceptsWhitespaceCommentsAndExplicitUnit) {
  TemporarySpectrumFile const file{"ggems-energy-valid.dat",
                                   "# center weight\n"
                                   "\n"
                                   "0.020\t1.0 # first bin\n"
                                   " 0.022  2.0\n"
                                   "0.024 1.0\n"};

  Distribution const distribution =
      Distribution::LoadRegularSpectrum(file.GetPath(), "MeV");

  EXPECT_EQ(distribution.GetRegularBinWidthMilliElectronVolt(), 2'000'000ULL);
  EXPECT_EQ(distribution.GetTableCount(), 3U);
  EXPECT_EQ(distribution.GetCumulativeTicketUpperBounds().back(),
            4'294'967'296ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEnergyDistribution, ParserReportsFilenameAndLine) {
  TemporarySpectrumFile const malformed{"ggems-energy-malformed.dat",
                                        "0.020 1.0\nheader value\n"};
  TemporarySpectrumFile const one_column{"ggems-energy-one-column.dat",
                                         "0.020\n"};
  TemporarySpectrumFile const three_columns{"ggems-energy-three-columns.dat",
                                            "0.020 1.0 9.0\n"};
  TemporarySpectrumFile const empty{"ggems-energy-empty.dat", "# no data\n\n"};
  TemporarySpectrumFile const nonfinite{"ggems-energy-nonfinite.dat",
                                        "0.020 1.0\n0.022 inf\n"};
  TemporarySpectrumFile const one_valid_row{"ggems-energy-one-valid-row.dat",
                                            "0.020 1.0\n"};

  auto const expect_line_diagnostic =
      [](std::filesystem::path const &path,
         std::string_view expected_line) -> void {
    try {
      static_cast<void>(Distribution::LoadRegularSpectrum(path, "MeV"));
      FAIL() << "Expected spectrum parser rejection.";
    } catch (GGEMSException const &exception) {
      std::string const message = exception.what();
      EXPECT_NE(message.find(path.filename().string()), std::string::npos);
      EXPECT_NE(message.find(expected_line), std::string::npos);
    }
  };

  try {
    static_cast<void>(
        Distribution::LoadRegularSpectrum(malformed.GetPath(), "MeV"));
    FAIL() << "Expected malformed spectrum rejection.";
  } catch (GGEMSException const &exception) {
    std::string const message = exception.what();
    EXPECT_NE(message.find("ggems-energy-malformed.dat"), std::string::npos);
    EXPECT_NE(message.find("line 2"), std::string::npos);
  }

  expect_line_diagnostic(one_column.GetPath(), "line 1");
  expect_line_diagnostic(three_columns.GetPath(), "line 1");
  expect_line_diagnostic(nonfinite.GetPath(), "line 2");
  expect_line_diagnostic(one_valid_row.GetPath(), "line 1");
  EXPECT_THROW(static_cast<void>(
                   Distribution::LoadRegularSpectrum(empty.GetPath(), "MeV")),
               GGEMSException);
  EXPECT_THROW(static_cast<void>(Distribution::LoadRegularSpectrum(
                   empty.GetPath().parent_path(), "MeV")),
               GGEMSException);
  EXPECT_THROW(
      static_cast<void>(Distribution::LoadRegularSpectrum(
          empty.GetPath().parent_path() / "missing-spectrum.dat", "MeV")),
      GGEMSException);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEnergyDistribution, Supplied120kVpSpectrumHasExpectedGrid) {
  std::filesystem::path const path =
      std::filesystem::path{GGEMS_TEST_KERNEL_ROOT}.parent_path() /
      "validation" / "source" / "data" / "spectrum_120kVp_2mmAl.dat";

  Distribution const distribution =
      Distribution::LoadRegularSpectrum(path, "MeV");

  EXPECT_EQ(distribution.GetTableCount(), 111U);
  EXPECT_EQ(distribution.GetRegularBinWidthMilliElectronVolt(), 1'000'000ULL);
  EXPECT_EQ(distribution.GetEnergyValuesMilliElectronVolt().front(),
            11'000'000ULL);
  EXPECT_EQ(distribution.GetEnergyValuesMilliElectronVolt().back(),
            121'000'000ULL);

  auto const ticket_bounds = distribution.GetCumulativeTicketUpperBounds();
  ASSERT_EQ(ticket_bounds.size(), 111U);
  EXPECT_GT(ticket_bounds[0U], 0ULL);
  EXPECT_GT(ticket_bounds[1U], ticket_bounds[0U]);

  std::uint64_t previous_upper{0ULL};

  for (std::uint64_t const upper : ticket_bounds) {
    EXPECT_GT(upper, previous_upper);
    previous_upper = upper;
  }

  EXPECT_EQ(previous_upper, 4'294'967'296ULL);
}
