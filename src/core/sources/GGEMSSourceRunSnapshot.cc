#include <array>
#include <cstdint>
#include <memory>
#include <span>
#include <cstddef>
#include <format>
#include <limits>
#include <utility>
#include <vector>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/core/sources/GGEMSEnergyDistribution.hh"
#include "GGEMS/core/sources/GGEMSEnergyDistributionRecord.hh"
#include "GGEMS/core/sources/GGEMSSource.hh"
#include "GGEMS/core/sources/GGEMSSourceRecord.hh"
#include "GGEMS/core/sources/GGEMSSourceRunRange.hh"
#include "GGEMS/core/sources/GGEMSSourceRunSnapshot.hh"

namespace ggems::core::sources {
namespace {

struct PackedSourceConfiguration {
  std::vector<GGEMSEnergyDistributionRecord> energy_distribution_records;
  std::vector<std::uint64_t> energy_values_milli_eV;
  std::vector<double> relative_weights;
  std::vector<std::uint64_t> cumulative_ticket_upper;
};

// =============================================================================
// =============================================================================

auto PackSourceConfiguration(std::span<GGEMSSource const *const> sources)
    -> PackedSourceConfiguration {
  GGEMS_CHECK_RECOVERABLE(
      sources.size() <=
          static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max()),
      "GGEMSSource configuration slot count exceeds uint32 storage.");

  PackedSourceConfiguration packed;
  packed.energy_distribution_records.reserve(sources.size());

  std::size_t total_table_count{0U};

  for (std::size_t source_index = 0U; source_index < sources.size();
       ++source_index) {
    GGEMSSource const *source = sources[source_index];

    GGEMS_CHECK_RECOVERABLE(
        source != nullptr,
        std::format("Cannot pack source configuration: source at index {} is "
                    "null.",
                    source_index));

    GGEMSEnergyDistribution const &distribution =
        source->GetEnergyDistribution();
    auto const energy_values = distribution.GetEnergyValuesMilliElectronVolt();
    auto const relative_weights = distribution.GetRelativeWeights();
    auto const cumulative_ticket_upper =
        distribution.GetCumulativeTicketUpperBounds();

    GGEMS_CHECK_INTERNAL(
        energy_values.size() == relative_weights.size() &&
            energy_values.size() == cumulative_ticket_upper.size(),
        "GGEMSSource energy value, weight, and ticket-bound counts do not "
        "match.");

    GGEMS_CHECK_RECOVERABLE(
        energy_values.size() <=
                packed.energy_values_milli_eV.max_size() - total_table_count &&
            energy_values.size() <=
                packed.relative_weights.max_size() - total_table_count &&
            energy_values.size() <=
                packed.cumulative_ticket_upper.max_size() - total_table_count,
        "Packed GGEMSSource energy table size exceeds host vector storage.");

    total_table_count += energy_values.size();
  }

  packed.energy_values_milli_eV.reserve(total_table_count);
  packed.relative_weights.reserve(total_table_count);
  packed.cumulative_ticket_upper.reserve(total_table_count);

  for (GGEMSSource const *source : sources) {
    GGEMSEnergyDistribution const &distribution =
        source->GetEnergyDistribution();
    auto const energy_values = distribution.GetEnergyValuesMilliElectronVolt();
    auto const relative_weights = distribution.GetRelativeWeights();
    auto const cumulative_ticket_upper =
        distribution.GetCumulativeTicketUpperBounds();

    GGEMS_CHECK_RECOVERABLE(
        std::in_range<std::uint64_t>(packed.energy_values_milli_eV.size()),
        "Packed GGEMSSource energy table offset exceeds uint64 storage.");

    auto const table_offset =
        static_cast<std::uint64_t>(packed.energy_values_milli_eV.size());

    packed.energy_distribution_records.push_back(
        distribution.BuildRecord(table_offset));
    packed.energy_values_milli_eV.insert(packed.energy_values_milli_eV.end(),
                                         energy_values.begin(),
                                         energy_values.end());
    packed.relative_weights.insert(packed.relative_weights.end(),
                                   relative_weights.begin(),
                                   relative_weights.end());
    packed.cumulative_ticket_upper.insert(packed.cumulative_ticket_upper.end(),
                                          cumulative_ticket_upper.begin(),
                                          cumulative_ticket_upper.end());
  }

  return packed;
}

// =============================================================================
// =============================================================================

auto AppendSourceRunSnapshotEntry(std::vector<GGEMSSourceRecord> &records,
                                  std::vector<GGEMSSourceRunRange> &ranges,
                                  std::uint64_t &total_primary_count,
                                  GGEMSSource const &source,
                                  std::size_t source_index) -> void {
  GGEMSSourceRecord source_record = source.BuildRecord();
  std::uint64_t const source_primary_count = source.GetPrimaryCount();

  GGEMS_CHECK_RECOVERABLE(
      source_primary_count <=
          std::numeric_limits<std::uint64_t>::max() - total_primary_count,
      std::format("GGEMSSourceRunSnapshot total primary count overflows "
                  "std::uint64_t at source index {}.",
                  source_index));

  records.push_back(source_record);
  ranges.push_back({.projection_primary_begin = total_primary_count,
                    .primary_count = source_primary_count});

  total_primary_count += source_primary_count;
}

} // namespace

// =============================================================================
// =============================================================================

GGEMSSourceConfigurationSnapshot::GGEMSSourceConfigurationSnapshot(
    std::vector<GGEMSEnergyDistributionRecord> energy_distribution_records,
    std::vector<std::uint64_t> energy_values_milli_eV,
    std::vector<double> relative_weights,
    std::vector<std::uint64_t> cumulative_ticket_upper)
    : energy_distribution_records_{std::move(energy_distribution_records)},
      energy_values_milli_eV_{std::move(energy_values_milli_eV)},
      relative_weights_{std::move(relative_weights)},
      cumulative_ticket_upper_{std::move(cumulative_ticket_upper)} {}

// -----------------------------------------------------------------------------

auto BuildSourceConfigurationSnapshot(GGEMSSource const &source)
    -> GGEMSSourceConfigurationSnapshotPtr {
  std::array<GGEMSSource const *, 1U> sources{&source};
  PackedSourceConfiguration packed = PackSourceConfiguration(sources);

  return GGEMSSourceConfigurationSnapshotPtr{
      new GGEMSSourceConfigurationSnapshot{
          std::move(packed.energy_distribution_records),
          std::move(packed.energy_values_milli_eV),
          std::move(packed.relative_weights),
          std::move(packed.cumulative_ticket_upper)}};
}

// -----------------------------------------------------------------------------

auto BuildSourceConfigurationSnapshot(
    std::span<std::shared_ptr<GGEMSSource> const> sources)
    -> GGEMSSourceConfigurationSnapshotPtr {
  std::vector<GGEMSSource const *> source_pointers;
  source_pointers.reserve(sources.size());

  for (auto const &source : sources) {
    source_pointers.push_back(source.get());
  }

  PackedSourceConfiguration packed = PackSourceConfiguration(source_pointers);

  return GGEMSSourceConfigurationSnapshotPtr{
      new GGEMSSourceConfigurationSnapshot{
          std::move(packed.energy_distribution_records),
          std::move(packed.energy_values_milli_eV),
          std::move(packed.relative_weights),
          std::move(packed.cumulative_ticket_upper)}};
}

// =============================================================================
// =============================================================================

GGEMSSourceRunSnapshot::GGEMSSourceRunSnapshot(
    std::vector<GGEMSSourceRecord> records,
    std::vector<GGEMSSourceRunRange> ranges,
    GGEMSSourceConfigurationSnapshotPtr source_configuration,
    std::uint64_t total_primary_count)
    : records_{std::move(records)}, ranges_{std::move(ranges)},
      source_configuration_{std::move(source_configuration)},
      total_primary_count_{total_primary_count} {}

// -----------------------------------------------------------------------------

auto BuildSourceRunSnapshot(
    std::span<std::shared_ptr<GGEMSSource> const> sources,
    GGEMSSourceConfigurationSnapshotPtr source_configuration)
    -> GGEMSSourceRunSnapshot {
  GGEMS_CHECK_INTERNAL(source_configuration != nullptr,
                       "GGEMSSource configuration snapshot is null.");
  GGEMS_CHECK_INTERNAL(
      source_configuration->GetSourceCount() == sources.size(),
      "GGEMSSource configuration and run snapshot slot counts do not match.");

  std::vector<GGEMSSourceRecord> records;
  std::vector<GGEMSSourceRunRange> ranges;
  records.reserve(sources.size());
  ranges.reserve(sources.size());

  std::uint64_t total_primary_count{0ULL};

  for (std::size_t source_index = 0U; source_index < sources.size();
       ++source_index) {
    auto const &source = sources[source_index];

    GGEMS_CHECK_RECOVERABLE(
        source != nullptr,
        std::format(
            "Cannot build GGEMSSourceRunSnapshot: source at index {} is null.",
            source_index));

    AppendSourceRunSnapshotEntry(records, ranges, total_primary_count, *source,
                                 source_index);
  }

  return GGEMSSourceRunSnapshot{std::move(records), std::move(ranges),
                                std::move(source_configuration),
                                total_primary_count};
}

// -----------------------------------------------------------------------------

auto BuildSourceRunSnapshot(
    std::span<std::shared_ptr<GGEMSSource> const> sources)
    -> GGEMSSourceRunSnapshot {
  auto source_configuration = BuildSourceConfigurationSnapshot(sources);
  return BuildSourceRunSnapshot(sources, std::move(source_configuration));
}

// -----------------------------------------------------------------------------

auto BuildSourceRunSnapshot(GGEMSSource const &source)
    -> GGEMSSourceRunSnapshot {
  std::vector<GGEMSSourceRecord> records;
  std::vector<GGEMSSourceRunRange> ranges;
  records.reserve(1U);
  ranges.reserve(1U);

  std::uint64_t total_primary_count{0ULL};
  AppendSourceRunSnapshotEntry(records, ranges, total_primary_count, source,
                               0U);

  return GGEMSSourceRunSnapshot{std::move(records), std::move(ranges),
                                BuildSourceConfigurationSnapshot(source),
                                total_primary_count};
}

} // namespace ggems::core::sources
