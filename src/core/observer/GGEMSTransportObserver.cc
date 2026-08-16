#include <algorithm>
#include <format>
#include <string>
#include <string_view>
#include <array>
#include <cstddef>
#include <cmath>
#include <span>
#include <cstdint>
#include <vector>
#include <utility>
#include <limits>
#include <memory>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSLogger.hh"
#include "GGEMS/core/observer/GGEMSObserverCounterArithmetic.hh"
#include "GGEMS/core/observer/GGEMSObserverTypes.hh"
#include "GGEMS/core/observer/GGEMSObserverRecord.hh"
#include "GGEMS/core/observer/GGEMSTransportObserver.hh"
#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/units/GGEMSEnergyUnits.hh"
#include "GGEMS/core/units/GGEMSLengthUnits.hh"
#include "GGEMS/core/units/GGEMSUnitFormatting.hh"
#include "GGEMS/utf/GGEMSUTF.hh"

namespace ggems::core::observer {
namespace {

// =============================================================================
// =============================================================================

auto CheckedAccumulateRunResultCounter(std::uint64_t &destination,
                                       std::uint64_t value,
                                       char const *diagnostic) -> void {
  if (!(value <= std::numeric_limits<std::uint64_t>::max() - destination)) {
    throw ggems::core::GGEMSRecoverable(diagnostic);
  }
  destination += value;
}

// =============================================================================
// =============================================================================

struct ObserverRecordView {
  std::size_t record_index;
  GGEMSObserverRecord const *record;
};

// =============================================================================
// =============================================================================

struct TrackDisplayEntry {
  std::uint64_t global_track_id;
  std::uint32_t local_track_id;
};

// =============================================================================
// =============================================================================

using TrackDisplayMap = std::vector<TrackDisplayEntry>;

// =============================================================================
// =============================================================================

auto ContainsGlobalTrackId(TrackDisplayMap const &track_display_map,
                           std::uint64_t global_track_id) -> bool {
  return std::ranges::find_if(
             track_display_map,
             [global_track_id](TrackDisplayEntry const &entry) -> bool {
               return entry.global_track_id == global_track_id;
             }) != track_display_map.end();
}

// =============================================================================
// =============================================================================

auto BuildTrackDisplayMap(std::vector<ObserverRecordView> const &views,
                          std::uint64_t primary_id) -> TrackDisplayMap {
  TrackDisplayMap track_display_map;

  for (ObserverRecordView const &view : views) {
    GGEMSObserverRecord const &record = *view.record;

    if (record.global_primary_id != primary_id) {
      continue;
    }

    if (ContainsGlobalTrackId(track_display_map, record.track_id)) {
      continue;
    }

    track_display_map.push_back(TrackDisplayEntry{
        .global_track_id = record.track_id,
        .local_track_id =
            static_cast<std::uint32_t>(track_display_map.size())});
  }

  return track_display_map;
}

// =============================================================================
// =============================================================================

auto FormatTrackDisplayId(TrackDisplayMap const &track_display_map,
                          std::uint64_t global_track_id) -> std::string {
  for (TrackDisplayEntry const &entry : track_display_map) {
    if (entry.global_track_id == global_track_id) {
      return std::format("{}", entry.local_track_id);
    }
  }

  return "?";
}

// =============================================================================
// =============================================================================

auto FormatParentTrack(TrackDisplayMap const &track_display_map,
                       std::uint64_t parent_track_id) -> std::string {
  if (parent_track_id == particles::k_invalid_id_u64) {
    return "-";
  }

  return FormatTrackDisplayId(track_display_map, parent_track_id);
}

// =============================================================================
// =============================================================================

auto RecordKindSortOrder(std::uint32_t record_kind) noexcept -> std::uint32_t {
  GGEMSObserverRecordKind kind = FromKernelObserverRecordKind(record_kind);

  switch (kind) {
  case GGEMSObserverRecordKind::Source:
    return 0U;
  case GGEMSObserverRecordKind::SecondaryStep:
    return 1U;
  case GGEMSObserverRecordKind::Step:
    return 2U;
  case GGEMSObserverRecordKind::Terminal:
    return 3U;
  case GGEMSObserverRecordKind::Anomaly:
    return 4U;
  case GGEMSObserverRecordKind::Unknown:
    return 5U;
  }

  return 5U;
}

// =============================================================================
// =============================================================================

auto BuildSortedRecordView(std::vector<GGEMSObserverRecord> const &records)
    -> std::vector<ObserverRecordView> {
  std::vector<ObserverRecordView> views;
  views.reserve(records.size());

  for (std::size_t i = 0U; i < records.size(); ++i) {
    views.push_back(
        ObserverRecordView{.record_index = i, .record = &records[i]});
  }

  std::ranges::stable_sort(
      views,
      [](ObserverRecordView const &first,
         ObserverRecordView const &second) -> bool {
        GGEMSObserverRecord const &left = *first.record;
        GGEMSObserverRecord const &right = *second.record;

        if (left.global_primary_id != right.global_primary_id) {
          return left.global_primary_id < right.global_primary_id;
        }

        if (left.track_id != right.track_id) {
          return left.track_id < right.track_id;
        }

        if (left.time_ps != right.time_ps) {
          return left.time_ps < right.time_ps;
        }

        std::uint32_t const left_kind_order =
            RecordKindSortOrder(left.record_kind);

        std::uint32_t const right_kind_order =
            RecordKindSortOrder(right.record_kind);

        if (left_kind_order != right_kind_order) {
          return left_kind_order < right_kind_order;
        }

        return first.record_index < second.record_index;
      });

  return views;
}

// =============================================================================
// =============================================================================

struct TableColumn {
  std::string_view title;
  std::size_t width;
};

// =============================================================================
// =============================================================================

constexpr std::array<TableColumn, 11U> k_observer_table_columns{
    {{.title = "Trk", .width = 3U},
     {.title = "Par", .width = 3U},
     {.title = "Kind", .width = 4U},
     {.title = "P", .width = 2U},
     {.title = "Energy", .width = 10U},
     {.title = "Edep", .width = 10U},
     {.title = "Position", .width = 36U},
     {.title = "Direction", .width = 21U},
     {.title = "Src", .width = 3U},
     {.title = "Time [ps]", .width = 16U},
     {.title = "Record", .width = 10U}}};

// =============================================================================
// =============================================================================

auto CleanDirectionValue(float value) noexcept -> float {
  if (std::fabs(value) < 0.005F) {
    return 0.0F;
  }

  return value;
}

// =============================================================================
// =============================================================================

auto FormatDirection(GGEMSObserverRecord const &record) -> std::string {
  return std::format("({:.2f}, {:.2f}, {:.2f})",
                     CleanDirectionValue(record.direction_x),
                     CleanDirectionValue(record.direction_y),
                     CleanDirectionValue(record.direction_z));
}

// =============================================================================
// =============================================================================

auto UTF8CodePointByteCount(unsigned char first_byte) noexcept -> std::size_t {
  if ((first_byte & 0x80U) == 0U) {
    return 1U;
  }

  if ((first_byte & 0xE0U) == 0xC0U) {
    return 2U;
  }

  if ((first_byte & 0xF0U) == 0xE0U) {
    return 3U;
  }

  if ((first_byte & 0xF8U) == 0xF0U) {
    return 4U;
  }

  return 1U;
}

// =============================================================================
// =============================================================================

auto FormatTableTime(std::uint64_t time_ps) -> std::string {
  return std::format("{}", time_ps);
}

// =============================================================================
// =============================================================================

auto FormatTableEnergy(std::uint64_t energy_milli_eV) -> std::string {
  return ggems::units::HumanReadable(ggems::units::Energy{energy_milli_eV}, 2);
}

// =============================================================================
// =============================================================================

auto FormatTableLength(std::int64_t length_pm) -> std::string {
  return ggems::units::HumanReadableSignedLength(length_pm, 2);
}

// =============================================================================
// =============================================================================

auto FormatPosition(GGEMSObserverRecord const &record) -> std::string {
  return std::format("({}, {}, {})", FormatTableLength(record.position_x_pm),
                     FormatTableLength(record.position_y_pm),
                     FormatTableLength(record.position_z_pm));
}

// =============================================================================
// =============================================================================

auto DisplayWidth(std::string_view text) -> std::size_t {
  std::size_t width{0U};
  std::size_t byte_index{0U};

  while (byte_index < text.size()) {
    std::size_t remaining_byte_count = text.size() - byte_index;

    std::size_t code_point_byte_count = std::min<std::size_t>(
        UTF8CodePointByteCount(static_cast<unsigned char>(text[byte_index])),
        remaining_byte_count);

    byte_index += code_point_byte_count;
    ++width;
  }

  return width;
}

// =============================================================================
// =============================================================================

auto TruncateToDisplayWidth(std::string_view text, std::size_t width)
    -> std::string {
  if (DisplayWidth(text) <= width) {
    return std::string{text};
  }

  if (width == 0U) {
    return {};
  }

  std::string out;
  std::size_t byte_index{0U};
  std::size_t display_width{0U};

  while (byte_index < text.size() && display_width + 1U < width) {
    std::size_t remaining_byte_count = text.size() - byte_index;

    std::size_t code_point_byte_count = std::min<std::size_t>(
        UTF8CodePointByteCount(static_cast<unsigned char>(text[byte_index])),
        remaining_byte_count);

    out.append(text.substr(byte_index, code_point_byte_count));

    byte_index += code_point_byte_count;
    ++display_width;
  }

  out += "~";

  return out;
}

// =============================================================================
// =============================================================================

auto FormatCell(std::string_view text, std::size_t width) -> std::string {
  std::string fitted = TruncateToDisplayWidth(text, width);

  std::size_t fitted_width = DisplayWidth(fitted);

  if (fitted_width >= width) {
    return fitted;
  }

  std::string padding(width - fitted_width, ' ');

  return padding + fitted;
}

// =============================================================================
// =============================================================================

auto MakeTableBorder(std::span<TableColumn const> columns) -> std::string {
  std::string line;
  line += "+";

  for (TableColumn const &column : columns) {
    line += std::string(column.width + 2U, '-');
    line += "+";
  }

  line += "\n";

  return line;
}

// =============================================================================
// =============================================================================

auto MakeTableRow(std::span<TableColumn const> columns,
                  std::span<std::string const> cells) -> std::string {
  std::string row;
  row += "|";

  for (std::size_t column_index = 0U; column_index < columns.size();
       ++column_index) {
    TableColumn const &column = columns[column_index];

    row += " ";
    row += FormatCell(cells[column_index], column.width);
    row += " |";
  }

  row += "\n";

  return row;
}

// =============================================================================
// =============================================================================

auto MakeTableHeader(std::span<TableColumn const> columns) -> std::string {
  std::array<std::string, 11U> cells{};

  for (std::size_t column_index = 0U; column_index < columns.size();
       ++column_index) {
    cells[column_index] = std::string{columns[column_index].title};
  }

  return MakeTableBorder(columns) + MakeTableRow(columns, cells) +
         MakeTableBorder(columns);
}

// =============================================================================
// =============================================================================

auto FormatParticleLabel(particles::GGEMSParticleType particle_type)
    -> std::string {
  auto const encoding = GGEMSLogger::GetInstance().GetEncoding();
  auto const symbol = encoding == Encoding::Ascii
                          ? particles::ToAsciiSymbol(particle_type)
                          : particles::ToUnicodeSymbol(particle_type);

  return utf::UTF32ToUTF8(symbol);
}

// =============================================================================
// =============================================================================

auto RecordKindShortName(GGEMSObserverRecordKind record_kind) noexcept
    -> std::string_view {
  switch (record_kind) {
  case GGEMSObserverRecordKind::Unknown:
    return "?";
  case GGEMSObserverRecordKind::Source:
    return "Src";
  case GGEMSObserverRecordKind::Step:
    return "Step";
  case GGEMSObserverRecordKind::SecondaryStep:
    return "Sec";
  case GGEMSObserverRecordKind::Terminal:
    return "Term";
  case GGEMSObserverRecordKind::Anomaly:
    return "Anom";
  }

  return "?";
}

// =============================================================================
// =============================================================================

auto BuildObserverTableRow(std::size_t record_index,
                           GGEMSObserverRecord const &record,
                           TrackDisplayMap const &track_display_map)
    -> std::array<std::string, 11U> {
  GGEMSObserverRecordKind record_kind =
      FromKernelObserverRecordKind(record.record_kind);

  particles::GGEMSParticleType particle_type =
      particles::FromKernelParticleType(record.particle_type);

  return {
      FormatTrackDisplayId(track_display_map, record.track_id),
      FormatParentTrack(track_display_map, record.parent_track_id),
      std::string{RecordKindShortName(record_kind)},
      FormatParticleLabel(particle_type),
      FormatTableEnergy(record.energy_milli_eV),
      FormatTableEnergy(record.deposited_energy_milli_eV),
      FormatPosition(record),
      FormatDirection(record),
      std::format("{}", record.source_index),
      FormatTableTime(record.time_ps),
      std::format("{}", record_index),
  };
}

// =============================================================================
// =============================================================================

auto ContainsTrackId(std::vector<std::uint64_t> const &track_ids,
                     std::uint64_t track_id) -> bool {
  return std::ranges::find(track_ids, track_id) != track_ids.end();
}

// =============================================================================
// =============================================================================

auto CountTracksForPrimary(std::vector<ObserverRecordView> const &views,
                           std::uint64_t primary_id) -> std::uint32_t {
  std::vector<std::uint64_t> track_ids;

  for (ObserverRecordView const &view : views) {
    GGEMSObserverRecord const &record = *view.record;

    if (record.global_primary_id != primary_id) {
      continue;
    }

    if (!ContainsTrackId(track_ids, record.track_id)) {
      track_ids.push_back(record.track_id);
    }
  }

  return static_cast<std::uint32_t>(track_ids.size());
}

// =============================================================================
// =============================================================================

auto CountRecordsForPrimary(std::vector<ObserverRecordView> const &views,
                            std::uint64_t primary_id) -> std::uint32_t {
  std::uint32_t count{0U};

  for (ObserverRecordView const &view : views) {
    if (view.record->global_primary_id == primary_id) {
      ++count;
    }
  }

  return count;
}

// =============================================================================
// =============================================================================

auto ContainsPrimaryId(std::vector<std::uint64_t> const &primary_ids,
                       std::uint64_t primary_id) -> bool {
  return std::ranges::find(primary_ids, primary_id) != primary_ids.end();
}

// =============================================================================
// =============================================================================

auto BuildPrimaryIds(std::vector<ObserverRecordView> const &views)
    -> std::vector<std::uint64_t> {
  std::vector<std::uint64_t> primary_ids;

  for (ObserverRecordView const &view : views) {
    std::uint64_t primary_id = view.record->global_primary_id;

    if (!ContainsPrimaryId(primary_ids, primary_id)) {
      primary_ids.push_back(primary_id);
    }
  }

  return primary_ids;
}

} // namespace

// =============================================================================
// =============================================================================

GGEMSTransportObserver::GGEMSTransportObserver()
    : GGEMSTransportObserver{true} {}

// -----------------------------------------------------------------------------

GGEMSTransportObserver::GGEMSTransportObserver(bool reserve_record_capacity) {
  if (reserve_record_capacity) {
    records_.reserve(record_capacity_);
  }
}

// -----------------------------------------------------------------------------

auto GGEMSTransportObserver::CreateRunResultCandidate() const
    -> std::unique_ptr<GGEMSTransportObserver> {
  auto candidate = std::unique_ptr<GGEMSTransportObserver>{
      new GGEMSTransportObserver{false}};
  candidate->max_stored_record_count_ = max_stored_record_count_;
  return candidate;
}

// -----------------------------------------------------------------------------

auto GGEMSTransportObserver::Enable(bool enabled) noexcept
    -> GGEMSTransportObserver & {
  enabled_ = enabled;
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSTransportObserver::Disable() noexcept -> GGEMSTransportObserver & {
  enabled_ = false;
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSTransportObserver::SetRecordCapacity(std::uint32_t record_capacity)
    -> GGEMSTransportObserver & {
  if (!(record_capacity > 0U)) {
    throw ggems::core::GGEMSRecoverable(
        "Transport observer record capacity must be non-zero.");
  }

  if (record_capacity > records_.capacity()) {
    records_.reserve(record_capacity);
  }

  record_capacity_ = record_capacity;
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSTransportObserver::SetMaxStoredRecordCount(
    std::uint32_t max_stored_record_count) -> GGEMSTransportObserver & {
  if (!(max_stored_record_count > 0U)) {
    throw ggems::core::GGEMSRecoverable(
        "Transport observer maximum stored record count must be non-zero.");
  }

  max_stored_record_count_ = max_stored_record_count;

  if (records_.size() > max_stored_record_count_) {
    records_.resize(max_stored_record_count_);
    counters_.record_count = static_cast<std::uint32_t>(records_.size());
  }

  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSTransportObserver::CaptureFirstPrimaries(
    std::uint32_t primary_count_per_source) noexcept
    -> GGEMSTransportObserver & {
  enabled_ = true;
  capture_first_primary_count_per_source_ = primary_count_per_source;
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSTransportObserver::CapturePrimary(
    std::uint32_t source_index, std::uint64_t source_local_primary_id) noexcept
    -> GGEMSTransportObserver & {
  enabled_ = true;
  capture_specific_primary_enabled_ = true;
  capture_source_index_ = source_index;
  capture_source_local_primary_id_ = source_local_primary_id;
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSTransportObserver::ClearCapturedPrimary() noexcept
    -> GGEMSTransportObserver & {
  capture_specific_primary_enabled_ = false;
  capture_source_index_ = particles::k_invalid_id_u32;
  capture_source_local_primary_id_ = particles::k_invalid_id_u64;
  return *this;
}

// -----------------------------------------------------------------------------

auto GGEMSTransportObserver::Clear() noexcept -> void {
  run_result_logical_counters_ = GGEMSObserverRunResultCounters{};
  counters_ = GGEMSObserverCounters{};
  records_.clear();
}

// -----------------------------------------------------------------------------

auto GGEMSTransportObserver::CommitRunResult(
    GGEMSTransportObserver &candidate) noexcept -> void {
  std::swap(run_result_logical_counters_,
            candidate.run_result_logical_counters_);
  std::swap(counters_, candidate.counters_);
  records_.swap(candidate.records_);
}

// -----------------------------------------------------------------------------

auto GGEMSTransportObserver::Accumulate(
    std::span<GGEMSObserverRecord const> records,
    GGEMSObserverCounters const &counters) -> void {
  std::size_t available_record_count =
      std::min<std::size_t>(records.size(), counters.record_count);

  AccumulateRunResult(
      records.first(available_record_count),
      GGEMSObserverRunResultCounters{
          .record_count = available_record_count,
          .overflow_count = counters.overflow_count,
          .captured_primary_count = counters.captured_primary_count,
      });
}

// -----------------------------------------------------------------------------

auto GGEMSTransportObserver::AccumulateRunResult(
    std::span<GGEMSObserverRecord const> records,
    GGEMSObserverRunResultCounters const &logical_counters) -> void {
  if (!(logical_counters.record_count == records.size())) {
    throw ggems::core::GGEMSInternal(
        "Transport logical Observer record count does not match its candidate "
        "records.");
  }

  CheckedAccumulateRunResultCounter(
      run_result_logical_counters_.record_count, logical_counters.record_count,
      "Observer logical record aggregation overflows uint64.");
  CheckedAccumulateRunResultCounter(
      run_result_logical_counters_.overflow_count,
      logical_counters.overflow_count,
      "Observer logical overflow aggregation overflows uint64.");
  CheckedAccumulateRunResultCounter(
      run_result_logical_counters_.captured_primary_count,
      logical_counters.captured_primary_count,
      "Observer logical captured-primary aggregation overflows uint64.");

  auto const reserve_count = static_cast<std::size_t>(std::min<std::uint64_t>(
      run_result_logical_counters_.record_count, max_stored_record_count_));
  if (reserve_count > records_.capacity()) {
    records_.reserve(reserve_count);
  }

  std::size_t remaining_capacity =
      records_.size() < max_stored_record_count_
          ? static_cast<std::size_t>(max_stored_record_count_) - records_.size()
          : 0U;

  std::size_t copied_record_count =
      std::min<std::size_t>(records.size(), remaining_capacity);

  auto records_to_copy = records.first(copied_record_count);

  records_.insert(records_.end(), records_to_copy.begin(),
                  records_to_copy.end());

  if (copied_record_count < records.size()) {
    CheckedAccumulateRunResultCounter(
        run_result_logical_counters_.overflow_count,
        records.size() - copied_record_count,
        "Observer host-drop aggregation overflows uint64.");
  }

  counters_.record_count = static_cast<std::uint32_t>(records_.size());
  counters_.overflow_count = detail::SaturateObserverCounter(
      run_result_logical_counters_.overflow_count);
  counters_.captured_primary_count = detail::SaturateObserverCounter(
      run_result_logical_counters_.captured_primary_count);
}

// -----------------------------------------------------------------------------

auto GGEMSTransportObserver::BuildConfigRecord() const noexcept
    -> GGEMSObserverConfigRecord {
  GGEMSObserverConfigRecord config{};

  config.enabled = enabled_ ? 1U : 0U;
  config.capture_first_primary_count_per_source =
      capture_first_primary_count_per_source_;
  config.capture_specific_primary_enabled =
      capture_specific_primary_enabled_ ? 1U : 0U;
  config.capture_source_index = capture_source_index_;
  config.capture_source_local_primary_id = capture_source_local_primary_id_;

  return config;
}

// -----------------------------------------------------------------------------

auto GGEMSTransportObserver::IsEnabled() const noexcept -> bool {
  return enabled_;
}

// -----------------------------------------------------------------------------

auto GGEMSTransportObserver::GetRecordCapacity() const noexcept
    -> std::uint32_t {
  return record_capacity_;
}

// -----------------------------------------------------------------------------

auto GGEMSTransportObserver::GetRecordCount() const noexcept -> std::uint32_t {
  return counters_.record_count;
}

// -----------------------------------------------------------------------------

auto GGEMSTransportObserver::GetOverflowCount() const noexcept
    -> std::uint32_t {
  return counters_.overflow_count;
}

// -----------------------------------------------------------------------------

auto GGEMSTransportObserver::GetCapturedPrimaryCount() const noexcept
    -> std::uint32_t {
  return counters_.captured_primary_count;
}

// -----------------------------------------------------------------------------

auto GGEMSTransportObserver::GetRecords() const noexcept
    -> std::vector<GGEMSObserverRecord> const & {
  return records_;
}

// -----------------------------------------------------------------------------

auto GGEMSTransportObserver::BuildDump() const -> std::string {
  std::string result;

  std::vector<ObserverRecordView> const views = BuildSortedRecordView(records_);

  result += "\n";
  result += "=================================================================="
            "==============\n";
  result += "GGEMS Transport Observer\n";
  result += "=================================================================="
            "==============\n";
  result += std::format("Records: {} | Captured primaries: {} | Overflow: {}\n",
                        records_.size(), counters_.captured_primary_count,
                        counters_.overflow_count);
  result += "Track ids are local to this primary history.\n";
  result += "View: primary / local-track / time\n";
  result += "=================================================================="
            "==============\n";

  if (views.empty()) {
    result += "\nNo observer record stored.\n";
    return result;
  }

  std::vector<std::uint64_t> const primary_ids = BuildPrimaryIds(views);

  bool first_primary{true};

  for (std::uint64_t const primary_id : primary_ids) {
    TrackDisplayMap track_display_map = BuildTrackDisplayMap(views, primary_id);

    if (!first_primary) {
      result += "\n";
      result += "--------------------------------------------------------------"
                "------------------\n";
      result += "\n";
    }

    first_primary = false;

    std::uint32_t const primary_record_count =
        CountRecordsForPrimary(views, primary_id);

    std::uint32_t const primary_track_count =
        CountTracksForPrimary(views, primary_id);

    result += "\n";
    result += std::format("Primary {}\n", primary_id);
    result += std::format("Records: {} | Tracks: {}\n", primary_record_count,
                          primary_track_count);
    result += "\n";

    result += MakeTableHeader(k_observer_table_columns);

    std::uint64_t previous_track_id{particles::k_invalid_id_u64};
    bool first_track{true};

    for (ObserverRecordView const &view : views) {
      GGEMSObserverRecord const &record = *view.record;

      if (record.global_primary_id != primary_id) {
        continue;
      }

      if (!first_track && record.track_id != previous_track_id) {
        result += MakeTableBorder(k_observer_table_columns);
      }

      first_track = false;
      previous_track_id = record.track_id;

      std::array<std::string, 11U> const row =
          BuildObserverTableRow(view.record_index, record, track_display_map);

      result += MakeTableRow(k_observer_table_columns, row);
    }

    result += MakeTableBorder(k_observer_table_columns);
  }

  return result;
}
} // namespace ggems::core::observer
