#include <algorithm>
#include <format>
#include <limits>
#include <string>
#include <string_view>
#include <array>
#include <cstddef>
#include <cmath>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/core/observer/GGEMSObserverTypes.hh"
#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/units/GGEMSEnergyUnits.hh"
#include "GGEMS/core/units/GGEMSLengthUnits.hh"
#include "GGEMS/core/units/GGEMSTimeUnits.hh"
#include "GGEMS/core/observer/GGEMSTransportObserver.hh"
#include "GGEMS/utf/GGEMSGlyphs.hh"
#include "GGEMS/utf/GGEMSUTF.hh"

namespace ggems::core::observer {
namespace {

enum class CellAlignment : std::uint8_t { Left, Right };

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

struct ObserverRecordView {
  std::uint32_t record_index;
  GGEMSObserverRecord const *record;
};

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

struct TrackDisplayEntry {
  std::uint64_t global_track_id;
  std::uint32_t local_track_id;
};

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

using TrackDisplayMap = std::vector<TrackDisplayEntry>;

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

bool ContainsGlobalTrackId(TrackDisplayMap const &track_display_map,
                           std::uint64_t global_track_id) {
  return std::find_if(track_display_map.begin(), track_display_map.end(),
                      [global_track_id](TrackDisplayEntry const &entry) {
                        return entry.global_track_id == global_track_id;
                      }) != track_display_map.end();
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

TrackDisplayMap
BuildTrackDisplayMap(std::vector<ObserverRecordView> const &views,
                     std::uint64_t primary_id) {
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
        record.track_id, static_cast<std::uint32_t>(track_display_map.size())});
  }

  return track_display_map;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

std::string FormatTrackDisplayId(TrackDisplayMap const &track_display_map,
                                 std::uint64_t global_track_id) {
  for (TrackDisplayEntry const &entry : track_display_map) {
    if (entry.global_track_id == global_track_id) {
      return std::format("{}", entry.local_track_id);
    }
  }

  return "?";
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

std::string FormatParentTrack(TrackDisplayMap const &track_display_map,
                              std::uint64_t parent_track_id) {
  if (parent_track_id == particles::k_invalid_id_u64) {
    return "-";
  }

  return FormatTrackDisplayId(track_display_map, parent_track_id);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

std::uint32_t RecordKindSortOrder(std::uint32_t record_kind) noexcept {
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

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

std::vector<ObserverRecordView>
BuildSortedRecordView(std::vector<GGEMSObserverRecord> const &records,
                      std::uint32_t max_record_count) {
  std::vector<ObserverRecordView> views;
  views.reserve(records.size());

  for (std::size_t i = 0U; i < records.size(); ++i) {
    views.push_back(
        ObserverRecordView{static_cast<std::uint32_t>(i), &records[i]});
  }

  std::stable_sort(
      views.begin(), views.end(),
      [](ObserverRecordView const &a, ObserverRecordView const &b) {
        GGEMSObserverRecord const &left = *a.record;
        GGEMSObserverRecord const &right = *b.record;

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

        return a.record_index < b.record_index;
      });

  if (views.size() > max_record_count) {
    views.resize(max_record_count);
  }

  return views;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

struct TableColumn {
  std::string_view title;
  std::size_t width;
  CellAlignment alignment;
};

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

constexpr std::array<TableColumn, 13U> k_observer_table_columns{{
    {"Rec", 4U, CellAlignment::Right},
    {"T", 7U, CellAlignment::Right},
    {"Trk", 3U, CellAlignment::Right},
    {"Par", 3U, CellAlignment::Right},
    {"G", 1U, CellAlignment::Right},
    {"Kind", 4U, CellAlignment::Left},
    {"Particle", 8U, CellAlignment::Left},
    {"S", 1U, CellAlignment::Left},
    {"E", 10U, CellAlignment::Right},
    {"X", 8U, CellAlignment::Right},
    {"Y", 8U, CellAlignment::Right},
    {"Z", 8U, CellAlignment::Right},
    {"Dir", 18U, CellAlignment::Left},
}};

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

float CleanDirectionValue(float const value) noexcept {
  if (std::fabs(value) < 0.005F) {
    return 0.0F;
  }

  return value;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

std::string FormatDirection(GGEMSObserverRecord const &record) {
  return std::format("({:.2f}, {:.2f}, {:.2f})",
                     CleanDirectionValue(record.direction_x),
                     CleanDirectionValue(record.direction_y),
                     CleanDirectionValue(record.direction_z));
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

std::size_t UTF8CodePointByteCount(unsigned char first_byte) noexcept {
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

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

std::string CompactHumanReadable(std::string const &value) {
  std::size_t space_pos = value.find(' ');

  if (space_pos == std::string::npos) {
    return value;
  }

  std::string number_text = value.substr(0U, space_pos);
  std::string unit = value.substr(space_pos + 1U);

  double number{0.0};

  try {
    number = std::stod(number_text);
  } catch (...) {
    return value;
  }

  if (std::abs(number) < 1.0e-12) {
    number = 0.0;
  }

  return std::format("{:.5g}{}", number, unit);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

std::string FormatTableTime(std::uint64_t time_ps) {
  return CompactHumanReadable(
      ggems::units::HumanReadable(ggems::units::Time{time_ps}));
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

std::string FormatTableEnergy(std::uint64_t energy_milli_eV) {
  return CompactHumanReadable(
      ggems::units::HumanReadable(ggems::units::Energy{energy_milli_eV}));
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

std::string FormatTableLength(std::int64_t length_pm) {
  return CompactHumanReadable(
      ggems::units::HumanReadableSignedLength(length_pm));
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

std::size_t DisplayWidth(std::string_view text) {
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

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

std::string TruncateToDisplayWidth(std::string_view text, std::size_t width) {
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

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

std::string FormatCell(std::string_view text, std::size_t width,
                       CellAlignment alignment) {
  std::string fitted = TruncateToDisplayWidth(text, width);

  std::size_t fitted_width = DisplayWidth(fitted);

  if (fitted_width >= width) {
    return fitted;
  }

  std::string padding(width - fitted_width, ' ');

  if (alignment == CellAlignment::Right) {
    return padding + fitted;
  }

  return fitted + padding;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

std::string MakeTableBorder() {
  std::string line;
  line += "+";

  for (TableColumn const &column : k_observer_table_columns) {
    line += std::string(column.width + 2U, '-');
    line += "+";
  }

  line += "\n";

  return line;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

std::string MakeTableRow(std::array<std::string, 13U> const &cells) {
  std::string row;
  row += "|";

  for (std::size_t column_index = 0U;
       column_index < k_observer_table_columns.size(); ++column_index) {
    TableColumn const &column = k_observer_table_columns[column_index];

    row += " ";
    row += FormatCell(cells[column_index], column.width, column.alignment);
    row += " |";
  }

  row += "\n";

  return row;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

std::string MakeTableHeader() {
  std::array<std::string, 13U> cells{};

  for (std::size_t column_index = 0U;
       column_index < k_observer_table_columns.size(); ++column_index) {
    cells[column_index] =
        std::string{k_observer_table_columns[column_index].title};
  }

  return MakeTableBorder() + MakeTableRow(cells) + MakeTableBorder();
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

std::string FormatParticleLabel(particles::GGEMSParticleType particle_type) {
  auto const &glyphs = utf::Glyphs();

  switch (particle_type) {
  case particles::GGEMSParticleType::Unknown:
    return "?";

  case particles::GGEMSParticleType::Aionino:
    return std::format("{} Aio", glyphs.aionino);

  case particles::GGEMSParticleType::Gamma:
    return std::format("{} Gamma", glyphs.gamma);

  case particles::GGEMSParticleType::Electron:
    return std::format("{}{} Elec", glyphs.electron, glyphs.minus);

  case particles::GGEMSParticleType::Positron:
    return std::format("{}{} Pos", glyphs.electron, glyphs.plus);

  case particles::GGEMSParticleType::Proton:
    return std::format("{} Prot", glyphs.proton);

  case particles::GGEMSParticleType::Neutron:
    return std::format("{} Neut", glyphs.neutron);

  case particles::GGEMSParticleType::Alpha:
    return std::format("{} Alpha", glyphs.alpha);
  }

  return "?";
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

std::string_view
RecordKindShortName(GGEMSObserverRecordKind const record_kind) noexcept {
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

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

std::string_view ParticleStatusShortName(std::uint32_t const status) noexcept {
  switch (status) {
  case particles::ToKernelParticleStatus(
      particles::GGEMSParticleStatus::Inactive):
    return "I";
  case particles::ToKernelParticleStatus(particles::GGEMSParticleStatus::Alive):
    return "A";
  case particles::ToKernelParticleStatus(
      particles::GGEMSParticleStatus::Killed):
    return "K";
  case particles::ToKernelParticleStatus(
      particles::GGEMSParticleStatus::EscapedWorld):
    return "E";
  case particles::ToKernelParticleStatus(
      particles::GGEMSParticleStatus::Absorbed):
    return "Abs";
  default:
    return "?";
  }
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

std::array<std::string, 13U>

BuildObserverTableRow(std::uint32_t record_index,
                      GGEMSObserverRecord const &record,
                      TrackDisplayMap const &track_display_map) {
  GGEMSObserverRecordKind record_kind =
      FromKernelObserverRecordKind(record.record_kind);

  particles::GGEMSParticleType particle_type =
      particles::FromKernelParticleType(record.particle_type);

  return {
      std::format("{:04}", record_index),
      FormatTableTime(record.time_ps),
      FormatTrackDisplayId(track_display_map, record.track_id),
      FormatParentTrack(track_display_map, record.parent_track_id),
      std::format("{}", record.generation),
      std::string{RecordKindShortName(record_kind)},
      FormatParticleLabel(particle_type),
      std::string{ParticleStatusShortName(record.status)},
      FormatTableEnergy(record.energy_milli_eV),
      FormatTableLength(record.position_x_pm),
      FormatTableLength(record.position_y_pm),
      FormatTableLength(record.position_z_pm),
      FormatDirection(record),
  };
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

bool ContainsTrackId(std::vector<std::uint64_t> const &track_ids,
                     std::uint64_t track_id) {
  return std::find(track_ids.begin(), track_ids.end(), track_id) !=
         track_ids.end();
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

std::uint32_t
CountTracksForPrimary(std::vector<ObserverRecordView> const &views,
                      std::uint64_t primary_id) {
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

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

std::uint32_t
CountRecordsForPrimary(std::vector<ObserverRecordView> const &views,
                       std::uint64_t primary_id) {
  std::uint32_t count{0U};

  for (ObserverRecordView const &view : views) {
    if (view.record->global_primary_id == primary_id) {
      ++count;
    }
  }

  return count;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

bool ContainsPrimaryId(std::vector<std::uint64_t> const &primary_ids,
                       std::uint64_t primary_id) {
  return std::find(primary_ids.begin(), primary_ids.end(), primary_id) !=
         primary_ids.end();
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

std::vector<std::uint64_t>
BuildDisplayedPrimaryIds(std::vector<ObserverRecordView> const &views) {
  std::vector<std::uint64_t> primary_ids;

  for (ObserverRecordView const &view : views) {
    std::uint64_t primary_id = view.record->global_primary_id;

    if (!ContainsPrimaryId(primary_ids, primary_id)) {
      primary_ids.push_back(primary_id);
    }
  }

  return primary_ids;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void AddSaturated(std::uint32_t &destination, std::uint64_t value) {
  std::uint64_t sum = static_cast<std::uint64_t>(destination) + value;

  destination = static_cast<std::uint32_t>(std::min<std::uint64_t>(
      sum,
      static_cast<std::uint64_t>(std::numeric_limits<std::uint32_t>::max())));
}

} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

GGEMSTransportObserver::GGEMSTransportObserver() {
  records_.reserve(record_capacity_);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

GGEMSTransportObserver &
GGEMSTransportObserver::Enable(bool const enabled) noexcept {
  enabled_ = enabled;
  return *this;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

GGEMSTransportObserver &GGEMSTransportObserver::Disable() noexcept {
  enabled_ = false;
  return *this;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

GGEMSTransportObserver &
GGEMSTransportObserver::SetRecordCapacity(std::uint32_t const record_capacity) {
  GGEMS_CHECK_RECOVERABLE(
      record_capacity > 0U,
      "Transport observer record capacity must be non-zero.");

  record_capacity_ = record_capacity;
  return *this;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

GGEMSTransportObserver &GGEMSTransportObserver::SetMaxStoredRecordCount(
    std::uint32_t const max_stored_record_count) {
  GGEMS_CHECK_RECOVERABLE(
      max_stored_record_count > 0U,
      "Transport observer maximum stored record count must be non-zero.");

  max_stored_record_count_ = max_stored_record_count;

  if (records_.size() > max_stored_record_count_) {
    records_.resize(max_stored_record_count_);
    counters_.record_count = static_cast<std::uint32_t>(records_.size());
  }

  return *this;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

GGEMSTransportObserver &GGEMSTransportObserver::CaptureFirstPrimaries(
    std::uint32_t primary_count) noexcept {
  enabled_ = true;
  capture_first_primary_count_ = primary_count;
  return *this;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

GGEMSTransportObserver &GGEMSTransportObserver::CapturePrimary(
    std::uint64_t global_primary_id) noexcept {
  enabled_ = true;
  capture_specific_primary_enabled_ = true;
  capture_global_primary_id_ = global_primary_id;
  return *this;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

GGEMSTransportObserver &
GGEMSTransportObserver::ClearCapturedPrimary() noexcept {
  capture_specific_primary_enabled_ = false;
  capture_global_primary_id_ = 0xFFFFFFFFFFFFFFFFULL;
  return *this;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void GGEMSTransportObserver::Clear() {
  counters_ = GGEMSObserverCounters{};
  records_.clear();
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void GGEMSTransportObserver::Accumulate(
    std::span<GGEMSObserverRecord const> records,
    GGEMSObserverCounters const &counters) {
  if (!enabled_) {
    return;
  }

  AddSaturated(counters_.captured_primary_count,
               counters.captured_primary_count);
  AddSaturated(counters_.overflow_count, counters.overflow_count);

  std::size_t available_record_count =
      std::min<std::size_t>(records.size(), counters.record_count);

  std::size_t remaining_capacity =
      records_.size() < max_stored_record_count_
          ? static_cast<std::size_t>(max_stored_record_count_) - records_.size()
          : 0U;

  std::size_t copied_record_count =
      std::min<std::size_t>(available_record_count, remaining_capacity);

  auto records_to_copy = records.first(copied_record_count);

  records_.insert(records_.end(), records_to_copy.begin(),
                  records_to_copy.end());

  if (copied_record_count < available_record_count) {
    AddSaturated(counters_.overflow_count,
                 available_record_count - copied_record_count);
  }

  counters_.record_count = static_cast<std::uint32_t>(records_.size());
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

GGEMSObserverConfigRecord
GGEMSTransportObserver::BuildConfigRecord() const noexcept {
  GGEMSObserverConfigRecord config{};

  config.enabled = enabled_ ? 1U : 0U;
  config.capture_first_primary_count = capture_first_primary_count_;
  config.capture_specific_primary_enabled =
      capture_specific_primary_enabled_ ? 1U : 0U;
  config.capture_global_primary_id = capture_global_primary_id_;

  return config;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

bool GGEMSTransportObserver::IsEnabled() const noexcept { return enabled_; }

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

std::uint32_t GGEMSTransportObserver::GetRecordCapacity() const noexcept {
  return record_capacity_;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

std::uint32_t GGEMSTransportObserver::GetRecordCount() const noexcept {
  return counters_.record_count;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

std::uint32_t GGEMSTransportObserver::GetOverflowCount() const noexcept {
  return counters_.overflow_count;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

std::uint32_t GGEMSTransportObserver::GetCapturedPrimaryCount() const noexcept {
  return counters_.captured_primary_count;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

std::vector<GGEMSObserverRecord> const &
GGEMSTransportObserver::GetRecords() const noexcept {
  return records_;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

std::string
GGEMSTransportObserver::BuildDump(std::uint32_t const max_record_count) const {
  std::string result;

  std::vector<ObserverRecordView> const views =
      BuildSortedRecordView(records_, max_record_count);

  result += "\n";
  result += "=================================================================="
            "==============\n";
  result += "GGEMS Transport Observer\n";
  result += "=================================================================="
            "==============\n";
  result += std::format(
      "Records: {} | Displayed: {} | Captured primaries: {} | Overflow: {}\n",
      records_.size(), views.size(), counters_.captured_primary_count,
      counters_.overflow_count);
  result += "Track ids are local to this primary history.\n";
  result += "View: primary / local-track / time\n";
  result += "=================================================================="
            "==============\n";

  if (views.empty()) {
    result += "\nNo observer record stored.\n";
    return result;
  }

  std::vector<std::uint64_t> const primary_ids =
      BuildDisplayedPrimaryIds(views);

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

    result += MakeTableHeader();

    std::uint64_t previous_track_id{particles::k_invalid_id_u64};
    bool first_track{true};

    for (ObserverRecordView const &view : views) {
      GGEMSObserverRecord const &record = *view.record;

      if (record.global_primary_id != primary_id) {
        continue;
      }

      if (!first_track && record.track_id != previous_track_id) {
        result += MakeTableBorder();
      }

      first_track = false;
      previous_track_id = record.track_id;

      result += MakeTableRow(
          BuildObserverTableRow(view.record_index, record, track_display_map));
    }

    result += MakeTableBorder();
  }

  if (records_.size() > views.size()) {
    result += std::format("\n... {} observer record(s) not displayed.\n",
                          records_.size() - views.size());
  }

  return result;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void GGEMSTransportObserver::Verbose(std::uint32_t max_record_count) const {
  GGEMS_INFO("Observer", "{}", BuildDump(max_record_count));
}

} // namespace ggems::core::observer
