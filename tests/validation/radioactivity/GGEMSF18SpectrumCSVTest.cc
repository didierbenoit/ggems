#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/radioactivity/builtins/GGEMSBuiltInRadionuclides.hh"
#include "GGEMS/core/sources/GGEMSEnergyDistribution.hh"
#include "GGEMSF18SpectrumCSV.hh"

namespace {
using ggems::core::GGEMSExceptionBase;
using ggems::core::radioactivity::builtins::BuildF18Radionuclide;
using ggems::core::sources::k_energy_ticket_space_size;
using ggems::validation::radioactivity::ExportF18SpectrumCSV;
using ggems::validation::radioactivity::RunF18SpectrumValidationCLI;
using ggems::validation::radioactivity::WriteF18SpectrumCSV;

inline constexpr std::string_view k_expected_header{
    "bin_index,lower_edge_milli_eV,center_milli_eV,upper_edge_milli_eV,"
    "normalized_relative_weight,assigned_ticket_count,"
    "cumulative_ticket_upper_bound"};

[[nodiscard]] auto SplitFields(std::string const &line)
    -> std::vector<std::string> {
  std::vector<std::string> fields;
  std::istringstream input{line};
  std::string field;

  while (std::getline(input, field, ',')) {
    fields.push_back(field);
  }

  return fields;
}

[[nodiscard]] auto ReadFile(std::filesystem::path const &path) -> std::string {
  std::ifstream input{path, std::ios::binary};
  return {std::istreambuf_iterator<char>{input},
          std::istreambuf_iterator<char>{}};
}
} // namespace

TEST(GGEMSF18SpectrumCSVTest, WritesExactRowsAndReconstructsTicketBounds) {
  auto const definition = BuildF18Radionuclide();
  auto const emissions = definition.GetEmissions();
  ASSERT_FALSE(emissions.empty());

  auto const &distribution = emissions.front().GetEnergyDistribution();
  auto const centers = distribution.GetEnergyValuesMilliElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const bounds = distribution.GetCumulativeTicketUpperBounds();
  std::uint64_t const half_width =
      distribution.GetRegularBinWidthMilliElectronVolt() / 2ULL;

  std::ostringstream output;
  auto const result = WriteF18SpectrumCSV(output, distribution);
  std::string const csv = output.str();

  ASSERT_FALSE(csv.empty());
  EXPECT_EQ(csv.back(), '\n');
  EXPECT_EQ(static_cast<std::size_t>(std::count(csv.begin(), csv.end(), '\n')),
            centers.size() + 1U);
  EXPECT_EQ(result.row_count, centers.size());
  EXPECT_EQ(result.final_cumulative_ticket_upper_bound,
            k_energy_ticket_space_size);

  std::istringstream input{csv};
  std::string line;
  ASSERT_TRUE(static_cast<bool>(std::getline(input, line)));
  EXPECT_EQ(line, k_expected_header);

  std::uint64_t previous_bound{0ULL};

  for (std::size_t index = 0U; index < centers.size(); ++index) {
    ASSERT_TRUE(static_cast<bool>(std::getline(input, line)));
    auto const fields = SplitFields(line);
    ASSERT_EQ(fields.size(), 7U);

    EXPECT_EQ(std::stoull(fields[0U]), index);
    EXPECT_EQ(std::stoull(fields[1U]), centers[index] - half_width);
    EXPECT_EQ(std::stoull(fields[2U]), centers[index]);
    EXPECT_EQ(std::stoull(fields[3U]), centers[index] + half_width);

    double const parsed_weight = std::stod(fields[4U]);
    EXPECT_TRUE(std::isfinite(parsed_weight));
    EXPECT_GT(parsed_weight, 0.0);
    EXPECT_EQ(parsed_weight, weights[index]);

    std::uint64_t const parsed_ticket_count = std::stoull(fields[5U]);
    std::uint64_t const parsed_bound = std::stoull(fields[6U]);
    EXPECT_EQ(parsed_ticket_count, bounds[index] - previous_bound);
    EXPECT_EQ(parsed_bound, bounds[index]);
    previous_bound = parsed_bound;
  }

  EXPECT_EQ(previous_bound, k_energy_ticket_space_size);
  EXPECT_FALSE(static_cast<bool>(std::getline(input, line)));
}

TEST(GGEMSF18SpectrumCSVTest, RejectsEmptyAndUnwritableOutputPaths) {
  auto const definition = BuildF18Radionuclide();
  auto const &distribution =
      definition.GetEmissions().front().GetEnergyDistribution();

  EXPECT_THROW(
      (void)ExportF18SpectrumCSV(std::filesystem::path{}, distribution),
      GGEMSExceptionBase);

  std::filesystem::path const missing_parent =
      std::filesystem::path{::testing::TempDir()} /
      "ggems_f18_csv_missing_parent";
  std::error_code error;
  std::filesystem::remove_all(missing_parent, error);
  ASSERT_FALSE(std::filesystem::exists(missing_parent));

  EXPECT_THROW((void)ExportF18SpectrumCSV(missing_parent / "f18_spectrum.csv",
                                          distribution),
               GGEMSExceptionBase);
}

TEST(GGEMSF18SpectrumCSVTest, CLIRejectsInvalidArgumentCounts) {
  std::ostringstream standard_output;
  std::ostringstream standard_error;

  EXPECT_EQ(RunF18SpectrumValidationCLI({}, standard_output, standard_error),
            EXIT_FAILURE);
  EXPECT_TRUE(standard_output.str().empty());
  EXPECT_NE(standard_error.str().find("Usage:"), std::string::npos);

  std::array<std::string_view, 1U> const empty_argument{""};
  standard_output.str({});
  standard_output.clear();
  standard_error.str({});
  standard_error.clear();

  EXPECT_EQ(RunF18SpectrumValidationCLI(empty_argument, standard_output,
                                        standard_error),
            EXIT_FAILURE);
  EXPECT_TRUE(standard_output.str().empty());
  EXPECT_NE(standard_error.str().find("Usage:"), std::string::npos);

  std::array<std::string_view, 2U> const excess_arguments{"first.csv",
                                                          "second.csv"};
  standard_output.str({});
  standard_output.clear();
  standard_error.str({});
  standard_error.clear();

  EXPECT_EQ(RunF18SpectrumValidationCLI(excess_arguments, standard_output,
                                        standard_error),
            EXIT_FAILURE);
  EXPECT_TRUE(standard_output.str().empty());
  EXPECT_NE(standard_error.str().find("Usage:"), std::string::npos);
}

TEST(GGEMSF18SpectrumCSVTest, CLIExportsToTheExplicitTemporaryPath) {
  std::filesystem::path const output_path =
      std::filesystem::path{::testing::TempDir()} /
      "ggems_f18_evaluated_subset.csv";
  std::error_code error;
  std::filesystem::remove(output_path, error);

  std::string const output_path_text = output_path.string();
  std::array<std::string_view, 1U> const arguments{output_path_text};
  std::ostringstream standard_output;
  std::ostringstream standard_error;

  EXPECT_EQ(
      RunF18SpectrumValidationCLI(arguments, standard_output, standard_error),
      EXIT_SUCCESS);
  EXPECT_TRUE(standard_error.str().empty());
  ASSERT_TRUE(std::filesystem::is_regular_file(output_path));

  std::string const csv = ReadFile(output_path);
  EXPECT_TRUE(csv.starts_with(k_expected_header));
  EXPECT_FALSE(csv.empty());
  EXPECT_EQ(csv.back(), '\n');

  std::string const summary = standard_output.str();
  EXPECT_NE(summary.find("Radionuclide: F-18 EvaluatedSubset"),
            std::string::npos);
  EXPECT_NE(summary.find("LNHB / KRI"), std::string::npos);
  EXPECT_NE(summary.find("Beta model: AllowedPointCoulomb"), std::string::npos);
  EXPECT_NE(summary.find("Target maximum bin width [milli-eV]: 500000"),
            std::string::npos);
  EXPECT_NE(summary.find("Endpoint [milli-eV]: 633900000"), std::string::npos);
  EXPECT_NE(summary.find(output_path.generic_string()), std::string::npos);
}
