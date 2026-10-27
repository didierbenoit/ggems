// *****************************************************************************
// * This file is part of GGEMS.                                               *
// *                                                                           *
// * SPDX-License-Identifier: GPL-3.0-or-later                                 *
// * Copyright (C) 2017-2026 CHRU de Brest, Université de Bretagne Occidentale,*
// * Inserm.                                                                   *
// *                                                                           *
// * GGEMS is free software: you can redistribute it and/or modify             *
// * it under the terms of the GNU General Public License as published by      *
// * the Free Software Foundation, either version 3 of the License, or         *
// * (at your option) any later version.                                       *
// *                                                                           *
// * GGEMS is distributed in the hope that it will be useful,                  *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
// * GNU General Public License for more details.                              *
// *                                                                           *
// * You should have received a copy of the GNU General Public License         *
// * along with GGEMS. If not, see <https://www.gnu.org/licenses/>.            *
// *****************************************************************************
/*!
 * \file
 * \brief Aggregates Observer results and formats diagnostic track tables.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */
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

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/logging/GGEMSLogger.hh"
#include "GGEMS/observer/GGEMSObserverCounterArithmetic.hh"
#include "GGEMS/observer/GGEMSObserverTypes.hh"
#include "GGEMS/observer/GGEMSObserverRecord.hh"
#include "GGEMS/observer/GGEMSTransportObserver.hh"
#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/units/GGEMSEnergyUnits.hh"
#include "GGEMS/units/GGEMSUnitFormatting.hh"
#include "GGEMS/utf/GGEMSUTF.hh"

namespace ggems::core::observer {
namespace {

// =============================================================================
// =============================================================================

/*!
 * \brief Adds a diagnostic contribution with checked uint64 arithmetic.
 *
 * \param[in,out] destination Logical total to update.
 * \param[in] value Count to add.
 * \param[in] diagnostic Failure message for an unrepresentable sum.
 * \throws GGEMSRecoverable If the sum exceeds UINT64_MAX.
 */
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

/*! \brief Borrows a record while retaining its original capture-order index. */
struct ObserverRecordView {
  /*! \brief Original index used as the final stable display-order key. */
  std::size_t record_index;
  /*! \brief Borrowed immutable record; its owning vector must remain alive. */
  GGEMSObserverRecord const *record;
};

// =============================================================================
// =============================================================================

/*! \brief Maps a recorded track ID to a compact label within one primary. */
struct TrackDisplayEntry {
  /*! \brief Recorded track ID used as the lookup key within this primary. */
  std::uint64_t global_track_id;
  /*! \brief Zero-based display label assigned in first-encounter order. */
  std::uint32_t local_track_id;
};

// =============================================================================
// =============================================================================

/*!
 * \brief Stores the per-primary correspondence between track IDs and labels.
 */
using TrackDisplayMap = std::vector<TrackDisplayEntry>;

// =============================================================================
// =============================================================================

/*!
 * \brief Tests whether one primary's display map already contains a track.
 *
 * \param[in] track_display_map Per-primary display map.
 * \param[in] global_track_id Recorded track ID to find.
 * \return True when the map contains that track ID.
 */
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

/*!
 * \brief Assigns compact track labels in a selected primary's encounter order.
 *
 * \param[in] views Borrowed records in display order.
 * \param[in] primary_id Primary-history identifier to retain.
 * \return One entry per distinct track ID in the selected primary.
 */
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
      .local_track_id = static_cast<std::uint32_t>(track_display_map.size())});
  }

  return track_display_map;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Formats a track's compact per-primary display label.
 *
 * \param[in] track_display_map Per-primary track-label mapping.
 * \param[in] global_track_id Recorded track ID to display.
 * \return Decimal local label, or a question mark when not present.
 */
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

/*!
 * \brief Formats a parent-track label with an explicit no-parent marker.
 *
 * \param[in] track_display_map Per-primary track-label mapping.
 * \param[in] parent_track_id Recorded parent ID or invalid uint64 sentinel.
 * \return A dash for no parent, otherwise the mapped label or question mark.
 */
auto FormatParentTrack(TrackDisplayMap const &track_display_map,
                       std::uint64_t parent_track_id) -> std::string {
  if (parent_track_id == particles::k_invalid_id_u64) {
    return "-";
  }

  return FormatTrackDisplayId(track_display_map, parent_track_id);
}

// =============================================================================
// =============================================================================

/*!
 * \brief Orders diagnostic event kinds for equal track and time keys.
 *
 * \param[in] record_kind Unsigned event-kind code.
 * \return Source, secondary-step, step, terminal, anomaly, then unknown rank.
 */
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

/*!
 * \brief Builds a sorted nonowning display view without changing capture
 * storage.
 *
 * \param[in] records Owned records that must outlive the returned views.
 * \return Views ordered by primary, track, time, event-kind rank, and original
 * index.
 */
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

/*! \brief Defines one column in the fixed-width diagnostic table. */
struct TableColumn {
  /*! \brief Static column heading. */
  std::string_view title;
  /*! \brief Column width measured in the formatter's code-point units. */
  std::size_t width;
};

// =============================================================================
// =============================================================================

/*!
 * \brief Defines headings and widths for the eleven diagnostic display
 * columns.
 */
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

/*!
 * \brief Removes tiny direction components from the two-decimal display.
 *
 * \param[in] value Dimensionless stored direction component.
 * \return Zero when the absolute value is below 0.005, otherwise the input.
 */
auto CleanDirectionValue(float value) noexcept -> float {
  if (std::fabs(value) < 0.005F) {
    return 0.0F;
  }

  return value;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Formats the stored global direction with two decimal places.
 *
 * \param[in] record Record providing dimensionless direction components.
 * \return Parenthesized direction tuple for display.
 */
auto FormatDirection(GGEMSObserverRecord const &record) -> std::string {
  return std::format("({:.2f}, {:.2f}, {:.2f})",
                     CleanDirectionValue(record.direction_x),
                     CleanDirectionValue(record.direction_y),
                     CleanDirectionValue(record.direction_z));
}

// =============================================================================
// =============================================================================

/*!
 * \brief Estimates a UTF-8 sequence width from its first byte.
 *
 * This display helper does not validate continuation bytes or Unicode scalars.
 *
 * \param[in] first_byte Leading byte of the next display sequence.
 * \return One through four bytes; unsupported leading patterns use one byte.
 */
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

/*!
 * \brief Formats an absolute diagnostic time as integer picoseconds.
 *
 * \param[in] time_ps Unsigned simulation timestamp in ps.
 * \return Decimal timestamp without a unit suffix.
 */
auto FormatTableTime(std::uint64_t time_ps) -> std::string {
  return std::format("{}", time_ps);
}

// =============================================================================
// =============================================================================

/*!
 * \brief Formats canonical micro-eV using the shared energy display policy.
 *
 * \param[in] energy_micro_eV Unsigned canonical energy.
 * \return Human-readable energy with two-decimal precision and units.
 */
auto FormatTableEnergy(std::uint64_t energy_micro_eV) -> std::string {
  return ggems::units::HumanReadable(ggems::units::Energy{energy_micro_eV}, 2);
}

// =============================================================================
// =============================================================================

/*!
 * \brief Formats signed picometers using the shared length display policy.
 *
 * \param[in] length_pm Signed canonical diagnostic coordinate.
 * \return Human-readable length with two-decimal precision and units.
 */
auto FormatTableLength(std::int64_t length_pm) -> std::string {
  return ggems::units::HumanReadableSignedLength(length_pm, 2);
}

// =============================================================================
// =============================================================================

/*!
 * \brief Formats the exported global diagnostic position when representable.
 *
 * \param[in] record Record containing integer-pm coordinates and validity
 * flag.
 * \return A coordinate tuple, or the unrepresentable marker.
 */
auto FormatPosition(GGEMSObserverRecord const &record) -> std::string {
  if (record.position_valid == 0U) {
    return "unrepresentable";
  }

  return std::format("({}, {}, {})", FormatTableLength(record.position_x_pm),
                     FormatTableLength(record.position_y_pm),
                     FormatTableLength(record.position_z_pm));
}

// =============================================================================
// =============================================================================

/*!
 * \brief Counts UTF-8 code-point sequences for diagnostic table alignment.
 *
 * This is not a terminal grapheme-width calculation: wide and combining
 * characters do not receive special handling.
 *
 * \param[in] text UTF-8 display text.
 * \return Count of sequences estimated from leading bytes.
 */
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

/*!
 * \brief Fits diagnostic text to a code-point width using a tilde suffix.
 *
 * \param[in] text UTF-8 display text.
 * \param[in] width Maximum estimated code-point count.
 * \return Original text when it fits, otherwise a truncated copy; empty for
 * zero width.
 */
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

/*!
 * \brief Right-aligns a fitted diagnostic value within its table column.
 *
 * \param[in] text UTF-8 cell contents.
 * \param[in] width Requested estimated code-point width.
 * \return Truncated or left-padded cell contents.
 */
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

/*!
 * \brief Builds a horizontal border for the configured diagnostic columns.
 *
 * \param[in] columns Ordered column descriptors.
 * \return ASCII border with a trailing newline.
 */
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

/*!
 * \brief Formats one row using the configured column widths.
 *
 * \param[in] columns Ordered column descriptors.
 * \param[in] cells Cell values; must contain at least columns.size() entries.
 * \return Delimited row with a trailing newline.
 */
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

/*!
 * \brief Builds the heading row between two diagnostic table borders.
 *
 * \param[in] columns Column descriptors; at most eleven entries.
 * \return Table heading including borders and newlines.
 */
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

/*!
 * \brief Chooses an ASCII or Unicode particle symbol for the current logger
 * mode.
 *
 * \param[in] particle_type Particle species to label.
 * \return UTF-8 particle display symbol.
 */
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

/*!
 * \brief Returns the compact event-kind label used in diagnostic tables.
 *
 * \param[in] record_kind Event kind to label.
 * \return Static compact label, or a question mark for unknown values.
 */
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

/*!
 * \brief Formats the eleven cells describing one captured particle event.
 *
 * \param[in] record_index Original capture-storage index.
 * \param[in] record Captured diagnostic event.
 * \param[in] track_display_map Track labels for this primary history.
 * \return Formatted cells in the configured column order.
 */
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
    FormatTableEnergy(record.energy_micro_eV),
    FormatTableEnergy(record.deposited_energy_micro_eV),
    FormatPosition(record),
    FormatDirection(record),
    std::format("{}", record.source_index),
    FormatTableTime(record.time_ps),
    std::format("{}", record_index),
  };
}

// =============================================================================
// =============================================================================

/*!
 * \brief Tests whether a collected track-ID list already includes a track.
 *
 * \param[in] track_ids Unique track IDs collected so far.
 * \param[in] track_id Track ID to find.
 * \return True when the ID is present.
 */
auto ContainsTrackId(std::vector<std::uint64_t> const &track_ids,
                     std::uint64_t track_id) -> bool {
  return std::ranges::find(track_ids, track_id) != track_ids.end();
}

// =============================================================================
// =============================================================================

/*!
 * \brief Counts distinct recorded tracks in one primary's diagnostic view.
 *
 * \param[in] views Borrowed diagnostic records.
 * \param[in] primary_id Primary-history identifier to select.
 * \return Number of distinct recorded track IDs for that primary.
 */
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

/*!
 * \brief Counts diagnostic records belonging to one primary history.
 *
 * \param[in] views Borrowed diagnostic records.
 * \param[in] primary_id Primary-history identifier to select.
 * \return Number of matching records.
 */
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

/*!
 * \brief Tests whether a diagnostic primary list already contains an ID.
 *
 * \param[in] primary_ids Primary IDs collected so far.
 * \param[in] primary_id Primary-history identifier to find.
 * \return True when the ID is present.
 */
auto ContainsPrimaryId(std::vector<std::uint64_t> const &primary_ids,
                       std::uint64_t primary_id) -> bool {
  return std::ranges::find(primary_ids, primary_id) != primary_ids.end();
}

// =============================================================================
// =============================================================================

/*!
 * \brief Collects distinct primary IDs in first-encounter display order.
 *
 * \param[in] views Borrowed diagnostic records.
 * \return Unique primary-history IDs without changing the record views.
 */
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
  auto candidate =
    std::unique_ptr<GGEMSTransportObserver>{new GGEMSTransportObserver{false}};
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
  std::uint32_t primary_count_per_source) noexcept -> GGEMSTransportObserver & {
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
