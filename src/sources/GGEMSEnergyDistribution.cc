#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <span>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

#include "GGEMS/GGEMSException.hh"

#include "GGEMS/sources/GGEMSEnergyDistribution.hh"
#include "GGEMS/sources/GGEMSEnergyDistributionRecord.hh"
#include "GGEMS/sources/GGEMSSourceTypes.hh"
#include "GGEMS/units/GGEMSEnergyUnits.hh"
#include "GGEMS/units/GGEMSUnitConversion.hh"

namespace ggems::core::sources {
namespace {

// =============================================================================
// =============================================================================

constexpr std::uint64_t k_energy_ticket_space_size{1ULL << 32U};

// =============================================================================
// =============================================================================

struct ValidationContext {
  std::string_view filename;
  std::span<std::size_t const> line_numbers;
};

// =============================================================================
// =============================================================================

[[noreturn]] auto RejectEntry(std::string_view distribution_name,
                              std::size_t index, ValidationContext context,
                              std::string_view message) -> void {
  if (!context.filename.empty() && index < context.line_numbers.size()) {
    throw GGEMSRecoverable(std::format("{}: line {}: {}", context.filename,
                                       context.line_numbers[index], message));
  }

  throw GGEMSRecoverable(
    std::format("{} entry {}: {}", distribution_name, index, message));
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto ConvertEnergy(double energy, std::string_view unit,
                                 std::string_view distribution_name,
                                 std::size_t index, ValidationContext context)
  -> std::uint64_t {
  auto const conversion = ggems::units::MakeQuantity<ggems::units::Energy>(
    static_cast<long double>(energy), unit);

  if (conversion.has_value()) {
    if (conversion->value == 0ULL) {
      RejectEntry(distribution_name, index, context,
                  "energy must remain strictly positive after conversion to "
                  "micro-electronvolts.");
    }
    return conversion->value;
  }

  using ggems::units::UnitConversionError;

  if (conversion.error() == UnitConversionError::NonFinite) {
    RejectEntry(distribution_name, index, context, "energy must be finite.");
  }

  if (conversion.error() == UnitConversionError::NegativeValue) {
    RejectEntry(distribution_name, index, context,
                "energy must remain strictly positive after conversion to "
                "micro-electronvolts.");
  }

  if (conversion.error() == UnitConversionError::UnsupportedUnit) {
    RejectEntry(distribution_name, index, context,
                std::format("unsupported GGEMS energy unit '{}'.", unit));
  }

  RejectEntry(distribution_name, index, context,
              "energy conversion exceeds uint64 micro-electronvolt "
              "storage.");
}

// =============================================================================
// =============================================================================

auto ValidateCanonicalEnergyValues(std::span<std::uint64_t const> energies,
                                   std::string_view distribution_name) -> void {
  for (std::size_t index = 0U; index < energies.size(); ++index) {
    if (energies[index] == 0ULL) {
      RejectEntry(
        distribution_name, index, {},
        "energy must remain strictly positive in micro-electronvolts.");
    }
    if (index > 0U && energies[index] <= energies[index - 1U]) {
      RejectEntry(distribution_name, index, {},
                  "canonical energies must be strictly increasing.");
    }
  }
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto ConvertEnergyValues(std::span<double const> energies,
                                       std::string_view unit,
                                       std::string_view distribution_name,
                                       ValidationContext context)
  -> std::vector<std::uint64_t> {
  std::vector<std::uint64_t> converted;
  converted.reserve(energies.size());

  for (std::size_t index = 0U; index < energies.size(); ++index) {
    converted.push_back(
      ConvertEnergy(energies[index], unit, distribution_name, index, context));
  }

  for (std::size_t index = 1U; index < converted.size(); ++index) {
    if (converted[index] <= converted[index - 1U]) {
      RejectEntry(
        distribution_name, index, context,
        "energies must be strictly increasing and remain distinct after "
        "conversion to micro-electronvolts.");
    }
  }

  return converted;
}

// =============================================================================
// =============================================================================

struct TicketRemainder {
  long double remainder{0.0L};
  std::size_t index{0U};
};

// =============================================================================
// =============================================================================

[[nodiscard]] auto BuildCumulativeTicketUpperBounds(
  std::span<double const> relative_weights, std::string_view distribution_name,
  ValidationContext context) -> std::vector<std::uint64_t> {
  long double total_weight{0.0L};
  std::size_t positive_weight_count{0U};

  for (std::size_t index = 0U; index < relative_weights.size(); ++index) {
    double const weight = relative_weights[index];

    if (weight < 0.0) {
      RejectEntry(distribution_name, index, context,
                  "relative weight must be positive or zero.");
    }

    if (weight > 0.0) {
      ++positive_weight_count;
    }

    total_weight += static_cast<long double>(weight);
  }

  if (total_weight == 0.0L) {
    throw GGEMSRecoverable(std::format(
      "{} requires at least one strictly positive weight.", distribution_name));
  }

  std::vector<std::uint64_t> ticket_counts(relative_weights.size(), 0ULL);

  std::vector<TicketRemainder> remainders;
  remainders.reserve(positive_weight_count);

  std::uint64_t assigned_ticket_count{0ULL};

  auto const ticket_space =
    static_cast<long double>(k_energy_ticket_space_size);

  for (std::size_t index = 0U; index < relative_weights.size(); ++index) {
    double const weight = relative_weights[index];

    if (weight == 0.0) {
      continue;
    }

    long double const quota =
      static_cast<long double>(weight) / total_weight * ticket_space;
    long double const base = std::floor(quota);

    auto const base_ticket_count = static_cast<std::uint64_t>(base);

    if (base_ticket_count >
        k_energy_ticket_space_size - assigned_ticket_count) {
      throw GGEMSRecoverable(std::format(
        "{} 32-bit ticket floors exceed the ticket space.", distribution_name));
    }

    ticket_counts[index] = base_ticket_count;
    assigned_ticket_count += base_ticket_count;

    long double const remainder = quota - base;

    if (remainder > 0.0L) {
      remainders.push_back({.remainder = remainder, .index = index});
    }
  }

  std::uint64_t const remaining_ticket_count =
    k_energy_ticket_space_size - assigned_ticket_count;

  if (remaining_ticket_count > static_cast<std::uint64_t>(remainders.size())) {
    throw GGEMSRecoverable(std::format(
      "{} 32-bit ticket remainders cannot complete the ticket space.",
      distribution_name));
  }

  std::ranges::sort(remainders,
                    [](TicketRemainder const &lhs,
                       TicketRemainder const &rhs) noexcept -> bool {
                      if (lhs.remainder != rhs.remainder) {
                        return lhs.remainder > rhs.remainder;
                      }

                      return lhs.index < rhs.index;
                    });

  for (std::uint64_t remainder_index = 0ULL;
       remainder_index < remaining_ticket_count; ++remainder_index) {
    ++ticket_counts[remainders[static_cast<std::size_t>(remainder_index)]
                      .index];
  }

  std::vector<std::uint64_t> cumulative_ticket_upper;
  cumulative_ticket_upper.reserve(relative_weights.size());
  std::uint64_t cumulative_ticket_count{0ULL};

  for (std::size_t index = 0U; index < relative_weights.size(); ++index) {
    if (relative_weights[index] > 0.0 && ticket_counts[index] == 0ULL) {
      RejectEntry(
        distribution_name, index, context,
        "strictly positive relative weight receives no reachable ticket "
        "in the 32-bit random space.");
    }

    cumulative_ticket_count += ticket_counts[index];
    cumulative_ticket_upper.push_back(cumulative_ticket_count);
  }

  return cumulative_ticket_upper;
}

// =============================================================================
// =============================================================================

[[nodiscard]] constexpr auto IsSeparator(char character) noexcept -> bool {
  return character == ' ' || character == '\t' || character == '\r';
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto SplitDataFields(std::string_view line)
  -> std::vector<std::string_view> {
  std::size_t const comment = line.find('#');

  if (comment != std::string_view::npos) {
    line = line.substr(0U, comment);
  }

  std::vector<std::string_view> fields;
  std::size_t cursor{0U};

  while (cursor < line.size()) {
    while (cursor < line.size() && IsSeparator(line[cursor])) {
      ++cursor;
    }

    if (cursor == line.size()) {
      break;
    }

    std::size_t const field_begin = cursor;

    while (cursor < line.size() && !IsSeparator(line[cursor])) {
      ++cursor;
    }

    fields.push_back(line.substr(field_begin, cursor - field_begin));
  }

  return fields;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto ParseNumber(std::string_view field,
                               std::string_view filename,
                               std::size_t line_number,
                               std::string_view field_name) -> double {
  double value{0.0};

  auto const result = std::from_chars(field.data(), field.data() + field.size(),
                                      value, std::chars_format::general);

  if (result.ec != std::errc{} || result.ptr != field.data() + field.size()) {
    throw GGEMSRecoverable(std::format("{}: line {}: malformed {} '{}'.",
                                       filename, line_number, field_name,
                                       field));
  }

  if (!std::isfinite(value)) {
    throw GGEMSRecoverable(std::format("{}: line {}: {} must be finite.",
                                       filename, line_number, field_name));
  }

  return value;
}
} // namespace

// =============================================================================
// =============================================================================

GGEMSEnergyDistribution::GGEMSEnergyDistribution(
  GGEMSEnergyDistributionType type, std::uint64_t mono_energy_micro_eV,
  std::uint64_t regular_bin_width_micro_eV,
  std::vector<std::uint64_t> energy_values_micro_eV,
  std::vector<double> relative_weights,
  std::vector<std::uint64_t> cumulative_ticket_upper)
    : type_{type}, mono_energy_micro_eV_{mono_energy_micro_eV},
      regular_bin_width_micro_eV_{regular_bin_width_micro_eV},
      energy_values_micro_eV_{std::move(energy_values_micro_eV)},
      relative_weights_{std::move(relative_weights)},
      cumulative_ticket_upper_{std::move(cumulative_ticket_upper)} {}

// -----------------------------------------------------------------------------

auto GGEMSEnergyDistribution::BuildMono(std::uint64_t energy_micro_eV)
  -> GGEMSEnergyDistribution {
  if (!(energy_micro_eV > 0ULL)) {
    throw GGEMSRecoverable("Source energy must be non-zero.");
  }

  return GGEMSEnergyDistribution{
    GGEMSEnergyDistributionType::Mono, energy_micro_eV, 0ULL, {}, {}, {}};
}

// -----------------------------------------------------------------------------

auto GGEMSEnergyDistribution::BuildDiscreteLines(
  std::span<double const> energies, std::span<double const> relative_weights,
  std::string_view unit) -> GGEMSEnergyDistribution {

  if (!(energies.size() == relative_weights.size())) {
    throw GGEMSRecoverable(
      "Discrete energy line and relative-weight counts must match.");
  }

  if (!(energies.size() >= 2U)) {
    throw GGEMSRecoverable(
      "Discrete energy distributions require at least two lines.");
  }

  ValidationContext const context{};

  auto energy_values =
    ConvertEnergyValues(energies, unit, "Discrete energy lines", context);

  return BuildDiscreteLinesFromValues(std::move(energy_values),
                                      relative_weights);
}

// -----------------------------------------------------------------------------

auto GGEMSEnergyDistribution::BuildDiscreteLines(
  std::span<std::uint64_t const> energies_micro_eV,
  std::span<double const> relative_weights) -> GGEMSEnergyDistribution {

  if (!(energies_micro_eV.size() == relative_weights.size())) {
    throw GGEMSRecoverable(
      "Discrete energy line and relative-weight counts must match.");
  }

  if (!(energies_micro_eV.size() >= 2U)) {
    throw GGEMSRecoverable(
      "Discrete energy distributions require at least two lines.");
  }

  ValidateCanonicalEnergyValues(energies_micro_eV, "Discrete energy lines");
  return BuildDiscreteLinesFromValues(
    {energies_micro_eV.begin(), energies_micro_eV.end()}, relative_weights);
}

// -----------------------------------------------------------------------------

auto GGEMSEnergyDistribution::BuildDiscreteLinesFromValues(
  std::vector<std::uint64_t> energy_values,
  std::span<double const> relative_weights) -> GGEMSEnergyDistribution {
  ValidationContext const context{};

  std::vector<double> prepared_relative_weights{relative_weights.begin(),
                                                relative_weights.end()};

  auto cumulative_ticket_upper = BuildCumulativeTicketUpperBounds(
    prepared_relative_weights, "Discrete energy lines", context);

  return GGEMSEnergyDistribution{GGEMSEnergyDistributionType::DiscreteLines,
                                 0ULL,
                                 0ULL,
                                 std::move(energy_values),
                                 std::move(prepared_relative_weights),
                                 std::move(cumulative_ticket_upper)};
}

// -----------------------------------------------------------------------------

auto GGEMSEnergyDistribution::BuildRegularSpectrum(
  std::span<double const> bin_centers, std::span<double const> relative_weights,
  std::string_view unit)

  -> GGEMSEnergyDistribution {
  return BuildRegularSpectrumWithContext(bin_centers, relative_weights, unit,
                                         {}, {});
}

// -----------------------------------------------------------------------------

auto GGEMSEnergyDistribution::BuildRegularSpectrumWithContext(
  std::span<double const> bin_centers,
  std::span<double const> relative_bin_weights, std::string_view unit,
  std::string_view filename, std::span<std::size_t const> line_numbers)
  -> GGEMSEnergyDistribution {
  ValidationContext const context{
    .filename = filename,
    .line_numbers = line_numbers,
  };

  if (!(bin_centers.size() == relative_bin_weights.size())) {
    throw ggems::core::GGEMSRecoverable(
      "Regular spectrum center and relative-weight counts must match.");
  }

  if (bin_centers.size() < 2U) {
    RejectEntry("Regular energy spectrum", 0U, context,
                "requires at least two bins.");
  }

  auto energy_values =
    ConvertEnergyValues(bin_centers, unit, "Regular energy spectrum", context);

  return BuildRegularSpectrumFromValues(
    std::move(energy_values), relative_bin_weights, filename, line_numbers);
}

// -----------------------------------------------------------------------------

auto GGEMSEnergyDistribution::BuildRegularSpectrum(
  std::span<std::uint64_t const> bin_centers_micro_eV,
  std::span<double const> relative_weights) -> GGEMSEnergyDistribution {
  ValidationContext const context{};

  if (!(bin_centers_micro_eV.size() == relative_weights.size())) {
    throw ggems::core::GGEMSRecoverable(
      "Regular spectrum center and relative-weight counts must match.");
  }

  if (bin_centers_micro_eV.size() < 2U) {
    RejectEntry("Regular energy spectrum", 0U, context,
                "requires at least two bins.");
  }

  ValidateCanonicalEnergyValues(bin_centers_micro_eV,
                                "Regular energy spectrum");

  return BuildRegularSpectrumFromValues(
    {bin_centers_micro_eV.begin(), bin_centers_micro_eV.end()},
    relative_weights, {}, {});
}

// -----------------------------------------------------------------------------

auto GGEMSEnergyDistribution::BuildRegularSpectrumFromValues(
  std::vector<std::uint64_t> energy_values,
  std::span<double const> relative_bin_weights, std::string_view filename,
  std::span<std::size_t const> line_numbers) -> GGEMSEnergyDistribution {
  ValidationContext const context{
    .filename = filename,
    .line_numbers = line_numbers,
  };

  std::uint64_t const bin_width = energy_values[1U] - energy_values[0U];

  for (std::size_t index = 2U; index < energy_values.size(); ++index) {
    if (energy_values[index] - energy_values[index - 1U] != bin_width) {
      RejectEntry("Regular energy spectrum", index, context,
                  "bin centers must form an exactly regular grid after "
                  "conversion to micro-electronvolts.");
    }
  }

  if ((bin_width & 1ULL) != 0ULL) {
    RejectEntry("Regular energy spectrum", 1U, context,
                "internal bin width must be even so that both bin edges are "
                "exact micro-electronvolt values.");
  }

  std::uint64_t const half_width = bin_width / 2ULL;

  if (energy_values.front() <= half_width) {
    RejectEntry("Regular energy spectrum", 0U, context,
                "first lower bin edge must be strictly positive.");
  }

  std::vector<double> prepared_relative_weights{
    relative_bin_weights.begin(),
    relative_bin_weights.end(),
  };

  auto cumulative_ticket_upper = BuildCumulativeTicketUpperBounds(
    prepared_relative_weights, "Regular energy spectrum", context);

  return GGEMSEnergyDistribution{GGEMSEnergyDistributionType::RegularSpectrum,
                                 0ULL,
                                 bin_width,
                                 std::move(energy_values),
                                 std::move(prepared_relative_weights),
                                 std::move(cumulative_ticket_upper)};
}

// -----------------------------------------------------------------------------

auto GGEMSEnergyDistribution::LoadRegularSpectrum(
  std::filesystem::path const &filename, std::string_view unit)
  -> GGEMSEnergyDistribution {
  std::string const filename_text = filename.string();

  std::ifstream input{filename};

  if (!input.is_open()) {
    throw GGEMSRecoverable(
      std::format("{}: regular spectrum file is not readable.", filename_text));
  }

  std::vector<double> bin_centers;
  std::vector<double> relative_bin_weights;
  std::vector<std::size_t> line_numbers;

  std::string line;
  std::size_t line_number{0U};

  while (std::getline(input, line)) {
    ++line_number;

    auto const fields = SplitDataFields(line);

    if (fields.empty()) {
      continue;
    }

    if (fields.size() != 2U) {
      throw GGEMSRecoverable(std::format(
        "{}: line {}: expected exactly two numeric fields before any "
        "comment, found {}.",
        filename_text, line_number, fields.size()));
    }

    bin_centers.push_back(
      ParseNumber(fields[0U], filename_text, line_number, "energy center"));

    relative_bin_weights.push_back(ParseNumber(
      fields[1U], filename_text, line_number, "relative bin weight"));

    line_numbers.push_back(line_number);
  }

  if (input.bad()) {
    throw GGEMSRecoverable(
      std::format("{}: line {}: failed while reading regular spectrum "
                  "file.",
                  filename_text, line_number + 1U));
  }

  if (bin_centers.empty()) {
    throw GGEMSRecoverable(
      std::format("{}: regular spectrum contains no data.", filename_text));
  }

  return BuildRegularSpectrumWithContext(bin_centers, relative_bin_weights,
                                         unit, filename_text, line_numbers);
}

// -----------------------------------------------------------------------------

auto GGEMSEnergyDistribution::BuildRecord(
  std::uint64_t table_offset) const noexcept -> GGEMSEnergyDistributionRecord {
  if (type_ == GGEMSEnergyDistributionType::Mono) {
    return {
      .regular_bin_width_micro_eV = 0ULL,
      .table_offset = 0ULL,
      .distribution_type = ToKernelEnergyDistributionType(type_),
      .table_count = 0U,
    };
  }

  return {
    .regular_bin_width_micro_eV = regular_bin_width_micro_eV_,
    .table_offset = table_offset,
    .distribution_type = ToKernelEnergyDistributionType(type_),
    .table_count = GetTableCount(),
  };
}
} // namespace ggems::core::sources
