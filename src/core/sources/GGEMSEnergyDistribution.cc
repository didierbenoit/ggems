#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <limits>
#include <span>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

#include "GGEMS/core/GGEMSException.hh"

#include "GGEMS/core/sources/GGEMSEnergyDistribution.hh"
#include "GGEMS/core/sources/GGEMSEnergyDistributionRecord.hh"
#include "GGEMS/core/sources/GGEMSSourceTypes.hh"
#include "GGEMS/core/units/GGEMSEnergyUnits.hh"
#include "GGEMS/core/units/GGEMSUnitConversion.hh"

namespace ggems::core::sources {
namespace {

// =============================================================================
// =============================================================================

struct ValidationContext {
  std::string_view filename;
  std::span<std::size_t const> line_numbers;
};

// =============================================================================
// =============================================================================

[[noreturn]] auto Reject(std::string message) -> void {
  throw GGEMSRecoverable(std::move(message));
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto EntryPrefix(std::string_view distribution_name,
                               std::size_t index, ValidationContext context)
    -> std::string {
  if (!context.filename.empty() && index < context.line_numbers.size()) {
    return std::format("{}: line {}: ", context.filename,
                       context.line_numbers[index]);
  }

  return std::format("{} entry {}: ", distribution_name, index);
}

// =============================================================================
// =============================================================================

[[noreturn]] auto RejectEntry(std::string_view distribution_name,
                              std::size_t index, ValidationContext context,
                              std::string_view message) -> void {
  Reject(EntryPrefix(distribution_name, index, context) + std::string{message});
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
                  "milli-electronvolts.");
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
                "milli-electronvolts.");
  }

  if (conversion.error() == UnitConversionError::UnsupportedUnit) {
    RejectEntry(distribution_name, index, context,
                std::format("unsupported GGEMS energy unit '{}'.", unit));
  }

  RejectEntry(distribution_name, index, context,
              "energy conversion exceeds uint64 milli-electronvolt "
              "storage.");
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
    converted.push_back(ConvertEnergy(energies[index], unit, distribution_name,
                                      index, context));
  }

  for (std::size_t index = 1U; index < converted.size(); ++index) {
    if (converted[index] <= converted[index - 1U]) {
      RejectEntry(
          distribution_name, index, context,
          "energies must be strictly increasing and remain distinct after "
          "conversion to milli-electronvolts.");
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

[[nodiscard]] auto
BuildCumulativeTicketUpperBounds(std::span<double const> relative_weights,
                                 std::string_view distribution_name,
                                 ValidationContext context)
    -> std::vector<std::uint64_t> {
  long double total_weight{0.0L};
  std::size_t positive_weight_count{0U};

  for (std::size_t index = 0U; index < relative_weights.size(); ++index) {
    double const weight = relative_weights[index];

    if (!std::isfinite(weight)) {
      RejectEntry(distribution_name, index, context,
                  "relative weight must be finite.");
    }

    if (weight < 0.0) {
      RejectEntry(distribution_name, index, context,
                  "relative weight must be positive or zero.");
    }

    if (weight > 0.0) {
      ++positive_weight_count;
    }

    total_weight += static_cast<long double>(weight);

    if (!std::isfinite(total_weight)) {
      RejectEntry(distribution_name, index, context,
                  "relative-weight sum is not finite.");
    }
  }

  if (!(total_weight > 0.0L)) {
    RejectEntry(distribution_name, 0U, context,
                "at least one relative weight must be strictly positive.");
  }

  if (positive_weight_count > k_energy_ticket_space_size) {
    Reject(std::format(
        "{} has more positive entries than the 32-bit ticket space.",
        distribution_name));
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

    if (!std::isfinite(quota) || quota < 0.0L || quota > ticket_space ||
        base < 0.0L || base > ticket_space) {
      RejectEntry(distribution_name, index, context,
                  "32-bit ticket quota is not representable.");
    }

    auto const base_ticket_count = static_cast<std::uint64_t>(base);

    if (base_ticket_count >
        k_energy_ticket_space_size - assigned_ticket_count) {
      Reject(std::format("{} 32-bit ticket floors exceed the ticket space.",
                         distribution_name));
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
    Reject(std::format(
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

    if (ticket_counts[index] >
        k_energy_ticket_space_size - cumulative_ticket_count) {
      Reject(std::format(
          "{} cumulative 32-bit ticket count overflows its ticket space.",
          distribution_name));
    }

    cumulative_ticket_count += ticket_counts[index];
    cumulative_ticket_upper.push_back(cumulative_ticket_count);
  }

  if (cumulative_ticket_count != k_energy_ticket_space_size) {
    Reject(std::format(
        "{} cumulative ticket upper bound must end at exactly 2^32.",
        distribution_name));
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
    Reject(std::format("{}: line {}: malformed {} '{}'.", filename, line_number,
                       field_name, field));
  }

  if (!std::isfinite(value)) {
    Reject(std::format("{}: line {}: {} must be finite.", filename, line_number,
                       field_name));
  }

  return value;
}
} // namespace

// =============================================================================
// =============================================================================

GGEMSEnergyDistribution::GGEMSEnergyDistribution(
    GGEMSEnergyDistributionType type, std::uint64_t mono_energy_milli_eV,
    std::uint64_t regular_bin_width_milli_eV,
    std::vector<std::uint64_t> energy_values_milli_eV,
    std::vector<double> relative_weights,
    std::vector<std::uint64_t> cumulative_ticket_upper)
    : type_{type}, mono_energy_milli_eV_{mono_energy_milli_eV},
      regular_bin_width_milli_eV_{regular_bin_width_milli_eV},
      energy_values_milli_eV_{std::move(energy_values_milli_eV)},
      relative_weights_{std::move(relative_weights)},
      cumulative_ticket_upper_{std::move(cumulative_ticket_upper)} {}

// -----------------------------------------------------------------------------

auto GGEMSEnergyDistribution::BuildMono(std::uint64_t energy_milli_eV)
    -> GGEMSEnergyDistribution {
  if (!(energy_milli_eV > 0ULL)) {
    throw ggems::core::GGEMSRecoverable("Source energy must be non-zero.");
  }

  return GGEMSEnergyDistribution{
      GGEMSEnergyDistributionType::Mono, energy_milli_eV, 0ULL, {}, {}, {}};
}

// -----------------------------------------------------------------------------

auto GGEMSEnergyDistribution::BuildDiscreteLines(
    std::span<double const> energies, std::span<double const> relative_weights,
    std::string_view unit) -> GGEMSEnergyDistribution {
  if (!(energies.size() == relative_weights.size())) {
    throw ggems::core::GGEMSRecoverable(
        "Discrete energy line and relative-weight counts must match.");
  }
  if (!(energies.size() >= 2U)) {
    throw ggems::core::GGEMSRecoverable(
        "Discrete energy distributions require at least two lines.");
  }
  if (!(energies.size() <=
          static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max()))) {
    throw ggems::core::GGEMSRecoverable("Discrete energy line count exceeds uint32 storage.");
  }

  ValidationContext const context{};

  auto energy_values =
      ConvertEnergyValues(energies, unit, "Discrete energy lines", context);

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
    std::span<double const> bin_centers,
    std::span<double const> relative_weights, std::string_view unit)

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
  ValidationContext const context{.filename = filename,
                                  .line_numbers = line_numbers};

  if (!(bin_centers.size() == relative_bin_weights.size())) {
    throw ggems::core::GGEMSRecoverable(
        "Regular spectrum center and relative-weight counts must match.");
  }
  if (bin_centers.size() < 2U) {
    RejectEntry("Regular energy spectrum", 0U, context,
                "requires at least two bins.");
  }
  if (!(bin_centers.size() <=
          static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max()))) {
    throw ggems::core::GGEMSRecoverable("Regular spectrum bin count exceeds uint32 storage.");
  }

  auto energy_values = ConvertEnergyValues(bin_centers, unit,
                                           "Regular energy spectrum", context);

  std::uint64_t const bin_width = energy_values[1U] - energy_values[0U];

  for (std::size_t index = 2U; index < energy_values.size(); ++index) {
    if (energy_values[index] - energy_values[index - 1U] != bin_width) {
      RejectEntry("Regular energy spectrum", index, context,
                  "bin centers must form an exactly regular grid after "
                  "conversion to milli-electronvolts.");
    }
  }

  if ((bin_width & 1ULL) != 0ULL) {
    RejectEntry("Regular energy spectrum", 1U, context,
                "internal bin width must be even so that both bin edges are "
                "exact milli-electronvolt values.");
  }

  std::uint64_t const half_width = bin_width / 2ULL;

  if (energy_values.front() <= half_width) {
    RejectEntry("Regular energy spectrum", 0U, context,
                "first lower bin edge must be strictly positive.");
  }

  if (energy_values.back() >
      std::numeric_limits<std::uint64_t>::max() - half_width) {
    RejectEntry("Regular energy spectrum", energy_values.size() - 1U, context,
                "last upper bin edge exceeds uint64 milli-electronvolt "
                "storage.");
  }

  std::vector<double> prepared_relative_weights{relative_bin_weights.begin(),
                                                relative_bin_weights.end()};

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

  std::error_code filesystem_error;
  bool const file_exists = std::filesystem::exists(filename, filesystem_error);

  if (filesystem_error) {
    Reject(std::format("{}: cannot inspect regular spectrum file: {}.",
                       filename_text, filesystem_error.message()));
  }

  if (!file_exists) {
    Reject(std::format("{}: regular spectrum file does not exist.",
                       filename_text));
  }

  std::ifstream input{filename};

  if (!input.is_open()) {
    Reject(std::format("{}: regular spectrum file is not readable.",
                       filename_text));
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
      Reject(std::format(
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
    Reject(std::format("{}: line {}: failed while reading regular spectrum "
                       "file.",
                       filename_text, line_number + 1U));
  }

  if (bin_centers.empty()) {
    Reject(
        std::format("{}: regular spectrum contains no data.", filename_text));
  }

  return BuildRegularSpectrumWithContext(bin_centers, relative_bin_weights,
                                         unit, filename_text, line_numbers);
}

// -----------------------------------------------------------------------------

auto GGEMSEnergyDistribution::BuildRecord(std::uint64_t table_offset)
    const noexcept -> GGEMSEnergyDistributionRecord {
  if (type_ == GGEMSEnergyDistributionType::Mono) {
    return {
        .regular_bin_width_milli_eV = 0ULL,
        .table_offset = 0ULL,
        .distribution_type = ToKernelEnergyDistributionType(type_),
        .table_count = 0U,
    };
  }

  return {
      .regular_bin_width_milli_eV =
          type_ == GGEMSEnergyDistributionType::RegularSpectrum
              ? regular_bin_width_milli_eV_
              : 0ULL,
      .table_offset = table_offset,
      .distribution_type = ToKernelEnergyDistributionType(type_),
      .table_count = GetTableCount(),
  };
}
} // namespace ggems::core::sources
