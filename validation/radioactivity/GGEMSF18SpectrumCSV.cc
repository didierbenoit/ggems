#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <ios>
#include <limits>
#include <locale>
#include <ostream>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/radioactivity/GGEMSBetaSpectrumBuilder.hh"
#include "GGEMS/core/radioactivity/builtins/GGEMSBuiltInRadionuclides.hh"
#include "GGEMS/core/sources/GGEMSEnergyDistribution.hh"
#include "GGEMS/core/sources/GGEMSSourceTypes.hh"
#include "GGEMSF18SpectrumCSV.hh"

namespace ggems::validation::radioactivity {
namespace {

// =============================================================================
// =============================================================================

inline constexpr std::string_view k_csv_header{
    "bin_index,lower_edge_milli_eV,center_milli_eV,upper_edge_milli_eV,"
    "normalized_relative_weight,assigned_ticket_count,"
    "cumulative_ticket_upper_bound"};

// =============================================================================
// =============================================================================

[[noreturn]] auto Reject(std::string message) -> void {
  throw core::GGEMSRecoverable(std::move(message));
}

// =============================================================================
// =============================================================================

auto PrintFailure(std::ostream &standard_error, std::string_view message)
    -> void {
  standard_error << "ggems_validate_f18_spectrum: " << message;

  if (message.empty() || message.back() != '\n') {
    standard_error << '\n';
  }
}

} // namespace

// =============================================================================
// =============================================================================

[[nodiscard]] auto
WriteF18SpectrumCSV(std::ostream &output,
                    core::sources::GGEMSEnergyDistribution const &distribution)
    -> GGEMSF18SpectrumCSVResult {
  if (!output) {
    Reject("F-18 spectrum CSV output stream is not writable.");
  }

  if (distribution.GetType() !=
      core::sources::GGEMSEnergyDistributionType::RegularSpectrum) {
    Reject("F-18 positron energy distribution must be a RegularSpectrum.");
  }

  auto const centers = distribution.GetEnergyValuesMilliElectronVolt();
  auto const weights = distribution.GetRelativeWeights();
  auto const cumulative_ticket_upper_bounds =
      distribution.GetCumulativeTicketUpperBounds();

  if (centers.size() < 2U || centers.size() != weights.size() ||
      centers.size() != cumulative_ticket_upper_bounds.size() ||
      centers.size() !=
          static_cast<std::size_t>(distribution.GetTableCount())) {
    Reject("F-18 spectrum CSV source arrays have inconsistent sizes.");
  }

  std::uint64_t const bin_width =
      distribution.GetRegularBinWidthMilliElectronVolt();

  if (bin_width == 0ULL || (bin_width & 1ULL) != 0ULL) {
    Reject("F-18 spectrum CSV requires a positive even bin width.");
  }

  std::uint64_t const half_width = bin_width / 2ULL;
  std::uint64_t previous_ticket_upper_bound{0ULL};
  std::uint64_t previous_center{0ULL};

  std::ostringstream csv;
  csv.imbue(std::locale::classic());
  csv << k_csv_header << '\n';
  csv << std::setprecision(std::numeric_limits<double>::max_digits10);

  for (std::size_t index = 0U; index < centers.size(); ++index) {
    std::uint64_t const center = centers[index];
    double const weight = weights[index];
    std::uint64_t const cumulative_ticket_upper_bound =
        cumulative_ticket_upper_bounds[index];

    if (center <= half_width ||
        center > std::numeric_limits<std::uint64_t>::max() - half_width) {
      Reject("F-18 spectrum CSV bin edges exceed uint64 storage.");
    }

    if (index > 0U &&
        (center <= previous_center || center - previous_center != bin_width)) {
      Reject("F-18 spectrum CSV centers do not form the declared regular "
             "grid.");
    }

    if (!std::isfinite(weight) || !(weight > 0.0)) {
      Reject("F-18 spectrum CSV weights must be finite and strictly "
             "positive.");
    }

    if (cumulative_ticket_upper_bound <= previous_ticket_upper_bound) {
      Reject("F-18 spectrum CSV cumulative ticket bounds must be strictly "
             "increasing.");
    }

    std::uint64_t const assigned_ticket_count =
        cumulative_ticket_upper_bound - previous_ticket_upper_bound;
    std::uint64_t const lower_edge = center - half_width;
    std::uint64_t const upper_edge = center + half_width;

    csv << index << ',' << lower_edge << ',' << center << ',' << upper_edge
        << ',' << weight << ',' << assigned_ticket_count << ','
        << cumulative_ticket_upper_bound << '\n';

    previous_center = center;
    previous_ticket_upper_bound = cumulative_ticket_upper_bound;
  }

  if (previous_ticket_upper_bound !=
      core::sources::k_energy_ticket_space_size) {
    Reject("F-18 spectrum CSV cumulative ticket bounds must end at exactly "
           "2^32.");
  }

  if (!csv) {
    Reject("Failed while formatting the F-18 spectrum CSV.");
  }

  output << csv.str();

  if (!output) {
    Reject("Failed while writing the F-18 spectrum CSV.");
  }

  return {.row_count = centers.size(),
          .final_cumulative_ticket_upper_bound = previous_ticket_upper_bound};
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto
ExportF18SpectrumCSV(std::filesystem::path const &output_path,
                     core::sources::GGEMSEnergyDistribution const &distribution)
    -> GGEMSF18SpectrumCSVResult {
  if (output_path.empty()) {
    Reject("F-18 spectrum CSV output path must not be empty.");
  }

  std::ofstream output{output_path,
                       std::ios::binary | std::ios::out | std::ios::trunc};

  if (!output.is_open()) {
    Reject("Cannot open F-18 spectrum CSV output '" + output_path.string() +
           "'.");
  }

  GGEMSF18SpectrumCSVResult const result =
      WriteF18SpectrumCSV(output, distribution);

  output.flush();

  if (!output) {
    Reject("Failed while flushing F-18 spectrum CSV output '" +
           output_path.string() + "'.");
  }

  output.close();

  if (output.fail()) {
    Reject("Failed while closing F-18 spectrum CSV output '" +
           output_path.string() + "'.");
  }

  return result;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto
RunF18SpectrumValidationCLI(std::span<std::string_view const> arguments,
                            std::ostream &standard_output,
                            std::ostream &standard_error) -> int {
  try {
    if (arguments.size() != 1U || arguments.front().empty()) {
      Reject("Usage: ggems_validate_f18_spectrum <output.csv>");
    }

    std::filesystem::path const output_path{std::string{arguments.front()}};
    auto const beta_result =
        core::radioactivity::builtins::BuildF18PositronSpectrum();
    auto const &diagnostics = beta_result.diagnostics;

    if (diagnostics.model !=
        core::radioactivity::GGEMSBetaSpectrumModel::AllowedPointCoulomb) {
      Reject("The F-18 positron spectrum does not use AllowedPointCoulomb.");
    }

    GGEMSF18SpectrumCSVResult const export_result =
        ExportF18SpectrumCSV(output_path, beta_result.distribution);

    if (export_result.row_count != diagnostics.bin_count ||
        export_result.final_cumulative_ticket_upper_bound !=
            core::sources::k_energy_ticket_space_size) {
      Reject("The exported F-18 spectrum does not match its stored beta "
             "diagnostics.");
    }

    std::ostringstream summary;
    summary.imbue(std::locale::classic());
    summary << std::setprecision(
        std::numeric_limits<long double>::max_digits10);
    summary << "Radionuclide: F-18\n";
    summary << "Beta model: AllowedPointCoulomb\n";
    summary << "Bin count: " << diagnostics.bin_count << '\n';
    summary << "Endpoint [milli-eV]: " << diagnostics.upper_edge_milli_eV
            << '\n';
    summary << "Continuous mean [keV]: "
            << diagnostics.continuous_mean_energy_milli_eV / 1'000'000.0L
            << '\n';
    summary << "Represented mean [keV]: "
            << diagnostics.represented_mean_energy_milli_eV / 1'000'000.0L
            << '\n';
    summary << "Excluded probability: " << diagnostics.excluded_probability
            << '\n';
    summary << "Minimum positive bin probability: "
            << diagnostics.minimum_positive_bin_probability << '\n';
    summary << "Minimum assigned ticket count: "
            << diagnostics.minimum_assigned_ticket_count << '\n';
    summary << "Output path: " << output_path.generic_string() << '\n';

    standard_output << summary.str();

    if (!standard_output) {
      Reject("Failed while writing the F-18 validation summary.");
    }

    return EXIT_SUCCESS;
  } catch (core::GGEMSExceptionBase const &error) {
    PrintFailure(standard_error, error.what());
  } catch (std::exception const &error) {
    PrintFailure(standard_error, error.what());
  } catch (...) {
    PrintFailure(standard_error, "unknown failure");
  }

  return EXIT_FAILURE;
}

} // namespace ggems::validation::radioactivity
