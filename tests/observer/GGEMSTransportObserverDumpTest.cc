#include <array>
#include <cstdint>
#include <span>
#include <format>
#include <string>
#include <string_view>
#include <cstddef>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/logging/GGEMSLogger.hh"
#include "GGEMS/observer/GGEMSObserverRecord.hh"
#include "GGEMS/observer/GGEMSObserverTypes.hh"
#include "GGEMS/observer/GGEMSTransportObserver.hh"
#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/utf/GGEMSUTF.hh"
#include "GGEMSScopedLoggerEncoding.hh"

namespace {

using ggems::core::observer::GGEMSObserverCounters;
using ggems::core::observer::GGEMSObserverRecord;
using ggems::core::observer::GGEMSObserverRecordKind;
using ggems::core::observer::GGEMSTransportObserver;
using ggems::core::particles::GGEMSParticleStatus;
using ggems::core::particles::GGEMSParticleType;
using ggems::test::ScopedLoggerEncoding;

// =============================================================================
// =============================================================================

[[nodiscard]] auto
MakeRecord(std::uint64_t track_id, std::uint64_t time_ps,
           GGEMSParticleType particle_type = GGEMSParticleType::Gamma)
    -> GGEMSObserverRecord {
  GGEMSObserverRecord record{};

  record.run_id = 0ULL;
  record.global_primary_id = 4ULL;
  record.source_local_primary_id = 0ULL;
  record.global_particle_id = track_id;
  record.track_id = track_id;
  record.parent_track_id = ggems::core::particles::k_invalid_id_u64;
  record.time_ps = time_ps;
  record.record_kind = ggems::core::observer::ToKernelObserverRecordKind(
      GGEMSObserverRecordKind::Source);
  record.particle_type =
      ggems::core::particles::ToKernelParticleType(particle_type);
  record.status = ggems::core::particles::ToKernelParticleStatus(
      GGEMSParticleStatus::Alive);
  record.source_index = 0U;

  return record;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto
BuildObserverDump(std::span<GGEMSObserverRecord const> records) -> std::string {
  GGEMSTransportObserver observer{};
  observer.Enable();

  GGEMSObserverCounters counters{};
  counters.record_count = static_cast<std::uint32_t>(records.size());
  counters.captured_primary_count = records.empty() ? 0U : 1U;

  observer.Accumulate(records, counters);
  return observer.BuildDump();
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto ExtractTableHeader(std::string const &dump) -> std::string {
  std::size_t const header_begin = dump.find("| Trk |");

  if (header_begin == std::string::npos) {
    return {};
  }

  std::size_t const header_end = dump.find('\n', header_begin);

  if (header_end == std::string::npos) {
    return dump.substr(header_begin);
  }

  return dump.substr(header_begin, header_end - header_begin);
}

// =============================================================================
// =============================================================================

TEST(GGEMSTransportObserverDump, UsesRequestedColumnOrderAndHeaders) {
  std::array<GGEMSObserverRecord, 1U> const records{{
      MakeRecord(8ULL, 10ULL),
  }};

  std::string const header = ExtractTableHeader(BuildObserverDump(records));

  ASSERT_FALSE(header.empty());

  std::array<std::string_view, 11U> const ordered_tokens{
      "Trk",      "Par",       "Kind", "|  P |",    "Energy", "Edep",
      "Position", "Direction", "Src",  "Time [ps]", "Record",
  };

  std::size_t search_begin{0U};

  for (std::string_view const token : ordered_tokens) {
    std::size_t const token_position = header.find(token, search_begin);

    ASSERT_NE(token_position, std::string::npos)
        << "Missing or misplaced column: " << token << "\nHeader: " << header;

    search_begin = token_position + token.size();
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSTransportObserverDump, DisplaysAsciiParticleSymbols) {
  ScopedLoggerEncoding const encoding{ggems::core::Encoding::Ascii};

  std::array<GGEMSObserverRecord, 3U> const records{{
      MakeRecord(1ULL, 0ULL, GGEMSParticleType::Gamma),
      MakeRecord(2ULL, 0ULL, GGEMSParticleType::Electron),
      MakeRecord(3ULL, 0ULL, GGEMSParticleType::Positron),
  }};

  std::string const dump = BuildObserverDump(records);

  EXPECT_NE(dump.find("|  g |"), std::string::npos);
  EXPECT_NE(dump.find("| e- |"), std::string::npos);
  EXPECT_NE(dump.find("| e+ |"), std::string::npos);
}

// =============================================================================
// =============================================================================

TEST(GGEMSTransportObserverDump, DisplaysUnicodeParticleSymbols) {
  ScopedLoggerEncoding const encoding{ggems::core::Encoding::Unicode};

  std::array<GGEMSObserverRecord, 3U> const records{{
      MakeRecord(1ULL, 0ULL, GGEMSParticleType::Gamma),
      MakeRecord(2ULL, 0ULL, GGEMSParticleType::Electron),
      MakeRecord(3ULL, 0ULL, GGEMSParticleType::Positron),
  }};

  std::string const dump = BuildObserverDump(records);

  std::string const gamma = ggems::utf::UTF32ToUTF8(
      ggems::core::particles::ToUnicodeSymbol(GGEMSParticleType::Gamma));
  std::string const electron = ggems::utf::UTF32ToUTF8(
      ggems::core::particles::ToUnicodeSymbol(GGEMSParticleType::Electron));
  std::string const positron = ggems::utf::UTF32ToUTF8(
      ggems::core::particles::ToUnicodeSymbol(GGEMSParticleType::Positron));

  EXPECT_NE(dump.find("|  " + gamma + " |"), std::string::npos);
  EXPECT_NE(dump.find("| " + electron + " |"), std::string::npos);
  EXPECT_NE(dump.find("| " + positron + " |"), std::string::npos);
}

// =============================================================================
// =============================================================================

TEST(GGEMSTransportObserverDump,
     FormatsEnergyDepositPositionAndDirectionIndependently) {
  GGEMSObserverRecord record = MakeRecord(8ULL, 10ULL);

  record.energy_milli_eV = 123'456'000ULL;
  record.deposited_energy_milli_eV = 7'890'000ULL;

  record.position_x_pm = -999'990'000'000LL;
  record.position_y_pm = 1'000'000'000'000LL;
  record.position_z_pm = 10'000LL;

  record.direction_x = -1.0F;
  record.direction_y = -0.001F;
  record.direction_z = 1.0F;

  std::array<GGEMSObserverRecord, 1U> const records{{record}};
  std::string const dump = BuildObserverDump(records);

  EXPECT_NE(dump.find("123.46 keV"), std::string::npos);
  EXPECT_NE(dump.find("7.89 keV"), std::string::npos);

  EXPECT_NE(dump.find("(-999.99 mm, 1.00 m, 10.00 nm)"), std::string::npos);

  EXPECT_NE(dump.find("(-1.00, 0.00, 1.00)"), std::string::npos);
  EXPECT_EQ(dump.find("-0.00"), std::string::npos);
}

// =============================================================================
// =============================================================================

TEST(GGEMSTransportObserverDump,
     EscapedWorldRecordPreservesStoredPhysicalPosition) {
  GGEMSObserverRecord record = MakeRecord(8ULL, 10ULL);

  record.status = ggems::core::particles::ToKernelParticleStatus(
      GGEMSParticleStatus::EscapedWorld);
  record.position_x_pm = 10'000LL;
  record.position_y_pm = 1'000'000'000'000LL;
  record.position_z_pm = 0LL;

  std::array<GGEMSObserverRecord, 1U> const records{{record}};
  std::string const dump = BuildObserverDump(records);

  EXPECT_NE(dump.find("(10.00 nm, 1.00 m, 0.00 pm)"), std::string::npos);
  EXPECT_EQ(dump.find("OOW"), std::string::npos);
}

// =============================================================================
// =============================================================================

TEST(GGEMSTransportObserverDump, KeepsOriginalRecordIndexAfterDisplaySorting) {
  std::array<GGEMSObserverRecord, 2U> const records{{
      MakeRecord(8ULL, 20ULL),
      MakeRecord(8ULL, 10ULL),
  }};

  std::string const dump = BuildObserverDump(records);

  std::string const original_second_record_cell = std::format("| {:>10} |", 1U);
  std::string const original_first_record_cell = std::format("| {:>10} |", 0U);

  std::size_t const original_second_record =
      dump.find(original_second_record_cell);
  std::size_t const original_first_record =
      dump.find(original_first_record_cell);

  ASSERT_NE(original_second_record, std::string::npos);
  ASSERT_NE(original_first_record, std::string::npos);
  EXPECT_LT(original_second_record, original_first_record);
}

// =============================================================================
// =============================================================================

TEST(GGEMSTransportObserverDump, DumpsEveryStoredRecordWithoutDisplayLimit) {
  constexpr std::size_t k_record_count{130U};

  std::vector<GGEMSObserverRecord> records;
  records.reserve(k_record_count);

  for (std::size_t i = 0U; i < k_record_count; ++i) {
    records.push_back(MakeRecord(8ULL, static_cast<std::uint64_t>(i)));
  }

  std::string const dump = BuildObserverDump(records);
  std::string const header = ExtractTableHeader(dump);

  EXPECT_NE(dump.find("Records: 130 | Captured primaries: 1 | Overflow: 0"),
            std::string::npos);
  EXPECT_NE(dump.find("|        129 |"), std::string::npos);
  EXPECT_EQ(dump.find("0129 |"), std::string::npos);

  EXPECT_EQ(dump.find("Displayed:"), std::string::npos);
  EXPECT_EQ(dump.find("not displayed"), std::string::npos);

  EXPECT_NE(header.find("Trk"), std::string::npos);
  EXPECT_NE(header.find("Time [ps]"), std::string::npos);
}

} // namespace
