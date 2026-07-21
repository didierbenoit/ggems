#include <cstddef>
#include <format>
#include <limits>
#include <utility>
#include <vector>
#include <cstdint>
#include <span>
#include <memory>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/core/sources/GGEMSSourceRunSnapshot.hh"
#include "GGEMS/core/sources/GGEMSSource.hh"
#include "GGEMS/core/sources/GGEMSSourceRecord.hh"
#include "GGEMS/core/sources/GGEMSSourceRunRange.hh"

namespace ggems::core::sources {

namespace {

void AppendSourceRunSnapshotEntry(std::vector<GGEMSSourceRecord> &records,
                                  std::vector<GGEMSSourceRunRange> &ranges,
                                  std::uint64_t &total_primary_count,
                                  GGEMSSource const &source,
                                  std::size_t source_index) {
  GGEMSSourceRecord source_record = source.BuildRecord();
  std::uint64_t source_primary_count = source.GetPrimaryCount();

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

GGEMSSourceRunSnapshot::GGEMSSourceRunSnapshot(
    std::vector<GGEMSSourceRecord> records,
    std::vector<GGEMSSourceRunRange> ranges, std::uint64_t total_primary_count)
    : records_{std::move(records)}, ranges_{std::move(ranges)},
      total_primary_count_{total_primary_count} {}

// -----------------------------------------------------------------------------

auto BuildSourceRunSnapshot(
    std::span<std::shared_ptr<GGEMSSource> const> sources)
    -> GGEMSSourceRunSnapshot {
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
                                total_primary_count};
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
                                total_primary_count};
}

} // namespace ggems::core::sources
