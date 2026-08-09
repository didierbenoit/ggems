#include <array>
#include <cstdint>
#include <limits>

#include <gtest/gtest.h>

#include "GGEMS/core/observer/GGEMSTransportObserver.hh"
#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/observer/GGEMSObserverRecord.hh"

// =============================================================================
// =============================================================================

TEST(GGEMSTransportObserver, DefaultsSpecificCaptureToInvalidCoordinates) {
  ggems::core::observer::GGEMSTransportObserver observer{};

  auto const config = observer.BuildConfigRecord();

  EXPECT_EQ(config.enabled, 0U);
  EXPECT_EQ(config.capture_first_primary_count_per_source, 0U);
  EXPECT_EQ(config.capture_specific_primary_enabled, 0U);
  EXPECT_EQ(config.capture_source_index,
            ggems::core::particles::k_invalid_id_u32);
  EXPECT_EQ(config.capture_source_local_primary_id,
            ggems::core::particles::k_invalid_id_u64);
}

// =============================================================================
// =============================================================================

TEST(GGEMSTransportObserver, BuildsCombinedPerSourceAndSpecificCaptureConfig) {
  ggems::core::observer::GGEMSTransportObserver observer{};

  observer.CaptureFirstPrimaries(4U);

  auto const first_config = observer.BuildConfigRecord();

  EXPECT_EQ(first_config.enabled, 1U);
  EXPECT_EQ(first_config.capture_first_primary_count_per_source, 4U);
  EXPECT_EQ(first_config.capture_specific_primary_enabled, 0U);
  EXPECT_EQ(first_config.capture_source_index,
            ggems::core::particles::k_invalid_id_u32);
  EXPECT_EQ(first_config.capture_source_local_primary_id,
            ggems::core::particles::k_invalid_id_u64);

  observer.CapturePrimary(2U, 7ULL);

  auto const combined_config = observer.BuildConfigRecord();

  EXPECT_EQ(combined_config.enabled, 1U);
  EXPECT_EQ(combined_config.capture_first_primary_count_per_source, 4U);
  EXPECT_EQ(combined_config.capture_specific_primary_enabled, 1U);
  EXPECT_EQ(combined_config.capture_source_index, 2U);
  EXPECT_EQ(combined_config.capture_source_local_primary_id, 7ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSTransportObserver, ClearResetsOnlySpecificCapture) {
  ggems::core::observer::GGEMSTransportObserver observer{};

  observer.CaptureFirstPrimaries(5U).CapturePrimary(3U, 11ULL);

  auto const configured = observer.BuildConfigRecord();

  ASSERT_EQ(configured.enabled, 1U);
  ASSERT_EQ(configured.capture_first_primary_count_per_source, 5U);
  ASSERT_EQ(configured.capture_specific_primary_enabled, 1U);
  ASSERT_EQ(configured.capture_source_index, 3U);
  ASSERT_EQ(configured.capture_source_local_primary_id, 11ULL);

  observer.ClearCapturedPrimary();

  auto const cleared = observer.BuildConfigRecord();

  EXPECT_EQ(cleared.enabled, 1U);
  EXPECT_EQ(cleared.capture_first_primary_count_per_source, 5U);
  EXPECT_EQ(cleared.capture_specific_primary_enabled, 0U);
  EXPECT_EQ(cleared.capture_source_index,
            ggems::core::particles::k_invalid_id_u32);
  EXPECT_EQ(cleared.capture_source_local_primary_id,
            ggems::core::particles::k_invalid_id_u64);
}

// =============================================================================
// =============================================================================

TEST(GGEMSTransportObserver,
     AccumulatePreservesRetentionAndSaturationAcrossInputs) {
  using Observer = ggems::core::observer::GGEMSTransportObserver;
  using ObserverCounters = ggems::core::observer::GGEMSObserverCounters;
  using ObserverRecord = ggems::core::observer::GGEMSObserverRecord;

  constexpr auto k_counter_maximum = std::numeric_limits<std::uint32_t>::max();

  Observer observer{};
  observer.SetMaxStoredRecordCount(3U);

  std::array<ObserverRecord, 2U> const first_records{
      ObserverRecord{.run_id = 11ULL}, ObserverRecord{.run_id = 12ULL}};
  ObserverCounters const first_counters{
      .record_count = 2U,
      .overflow_count = k_counter_maximum - 2U,
      .captured_primary_count = k_counter_maximum - 1U,
      .reserved_0 = 0U,
  };

  observer.Accumulate(first_records, first_counters);

  ASSERT_EQ(observer.GetRecords().size(), 2U);
  EXPECT_EQ(observer.GetRecordCount(), 2U);
  EXPECT_EQ(observer.GetOverflowCount(), k_counter_maximum - 2U);
  EXPECT_EQ(observer.GetCapturedPrimaryCount(), k_counter_maximum - 1U);

  std::array<ObserverRecord, 2U> const second_records{
      ObserverRecord{.run_id = 21ULL}, ObserverRecord{.run_id = 22ULL}};
  ObserverCounters const second_counters{
      .record_count = 2U,
      .overflow_count = 1U,
      .captured_primary_count = 2U,
      .reserved_0 = 0U,
  };

  observer.Accumulate(second_records, second_counters);

  ASSERT_EQ(observer.GetRecords().size(), 3U);
  EXPECT_EQ(observer.GetRecordCount(), 3U);
  EXPECT_EQ(observer.GetOverflowCount(), k_counter_maximum);
  EXPECT_EQ(observer.GetCapturedPrimaryCount(), k_counter_maximum);
  EXPECT_EQ(observer.GetRecords()[0U].run_id, 11ULL);
  EXPECT_EQ(observer.GetRecords()[1U].run_id, 12ULL);
  EXPECT_EQ(observer.GetRecords()[2U].run_id, 21ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSTransportObserver,
     RunResultCandidateAccumulatesReportsAndCommitsTransactionally) {
  using Observer = ggems::core::observer::GGEMSTransportObserver;
  using ObserverCounters = ggems::core::observer::GGEMSObserverCounters;
  using ObserverRecord = ggems::core::observer::GGEMSObserverRecord;
  using RunResultCounters =
      ggems::core::observer::GGEMSObserverRunResultCounters;

  Observer observer{};
  observer.SetMaxStoredRecordCount(3U).CaptureFirstPrimaries(7U).CapturePrimary(
      2U, 9ULL);

  std::array<ObserverRecord, 1U> const previous_records{
      ObserverRecord{.run_id = 1ULL}};
  observer.Accumulate(previous_records,
                      ObserverCounters{.record_count = 1U,
                                       .overflow_count = 2U,
                                       .captured_primary_count = 1U,
                                       .reserved_0 = 0U});

  auto const configuration_before_commit = observer.BuildConfigRecord();
  auto candidate = observer.CreateRunResultCandidate();

  ASSERT_NE(candidate, nullptr);
  EXPECT_TRUE(candidate->GetRecords().empty());
  EXPECT_EQ(candidate->GetRecordCount(), 0U);
  EXPECT_EQ(candidate->GetOverflowCount(), 0U);
  EXPECT_EQ(candidate->GetCapturedPrimaryCount(), 0U);

  std::array<ObserverRecord, 2U> const first_report_records{
      ObserverRecord{.run_id = 11ULL}, ObserverRecord{.run_id = 12ULL}};
  candidate->AccumulateRunResult(
      first_report_records, RunResultCounters{.record_count = 2ULL,
                                              .overflow_count = 4ULL,
                                              .captured_primary_count = 1ULL});

  std::array<ObserverRecord, 2U> const second_report_records{
      ObserverRecord{.run_id = 21ULL}, ObserverRecord{.run_id = 22ULL}};
  candidate->AccumulateRunResult(
      second_report_records, RunResultCounters{.record_count = 2ULL,
                                               .overflow_count = 5ULL,
                                               .captured_primary_count = 2ULL});

  ASSERT_EQ(candidate->GetRecords().size(), 3U);
  EXPECT_EQ(candidate->GetRecordCount(), 3U);
  EXPECT_EQ(candidate->GetOverflowCount(), 10U);
  EXPECT_EQ(candidate->GetCapturedPrimaryCount(), 3U);
  EXPECT_EQ(candidate->GetRecords()[0U].run_id, 11ULL);
  EXPECT_EQ(candidate->GetRecords()[1U].run_id, 12ULL);
  EXPECT_EQ(candidate->GetRecords()[2U].run_id, 21ULL);

  ASSERT_EQ(observer.GetRecords().size(), 1U);
  EXPECT_EQ(observer.GetRecords()[0U].run_id, 1ULL);
  EXPECT_EQ(observer.GetRecordCount(), 1U);
  EXPECT_EQ(observer.GetOverflowCount(), 2U);
  EXPECT_EQ(observer.GetCapturedPrimaryCount(), 1U);

  observer.CommitRunResult(*candidate);

  auto const configuration_after_commit = observer.BuildConfigRecord();
  EXPECT_EQ(configuration_after_commit.enabled,
            configuration_before_commit.enabled);
  EXPECT_EQ(configuration_after_commit.capture_first_primary_count_per_source,
            configuration_before_commit.capture_first_primary_count_per_source);
  EXPECT_EQ(configuration_after_commit.capture_specific_primary_enabled,
            configuration_before_commit.capture_specific_primary_enabled);
  EXPECT_EQ(configuration_after_commit.capture_source_index,
            configuration_before_commit.capture_source_index);
  EXPECT_EQ(configuration_after_commit.capture_source_local_primary_id,
            configuration_before_commit.capture_source_local_primary_id);

  ASSERT_EQ(observer.GetRecords().size(), 3U);
  EXPECT_EQ(observer.GetRecordCount(), 3U);
  EXPECT_EQ(observer.GetOverflowCount(), 10U);
  EXPECT_EQ(observer.GetCapturedPrimaryCount(), 3U);

  std::array<ObserverRecord, 1U> const additional_records{
      ObserverRecord{.run_id = 31ULL}};
  observer.Accumulate(additional_records,
                      ObserverCounters{.record_count = 1U,
                                       .overflow_count = 1U,
                                       .captured_primary_count = 1U,
                                       .reserved_0 = 0U});

  EXPECT_EQ(observer.GetRecordCount(), 3U);
  EXPECT_EQ(observer.GetOverflowCount(), 12U);
  EXPECT_EQ(observer.GetCapturedPrimaryCount(), 4U);
}

// =============================================================================
// =============================================================================

TEST(GGEMSTransportObserver, RunResultCandidateSaturatesWideLogicalCounters) {
  using Observer = ggems::core::observer::GGEMSTransportObserver;
  using ObserverRecord = ggems::core::observer::GGEMSObserverRecord;
  using RunResultCounters =
      ggems::core::observer::GGEMSObserverRunResultCounters;

  constexpr auto k_counter_maximum = std::numeric_limits<std::uint32_t>::max();
  constexpr auto k_counter_maximum_wide =
      static_cast<std::uint64_t>(k_counter_maximum);

  Observer observer{};
  auto candidate = observer.CreateRunResultCandidate();
  std::array<ObserverRecord, 1U> const records{ObserverRecord{.run_id = 41ULL}};

  candidate->AccumulateRunResult(
      records, RunResultCounters{
                   .record_count = 1ULL,
                   .overflow_count = k_counter_maximum_wide + 7ULL,
                   .captured_primary_count = k_counter_maximum_wide + 11ULL,
               });

  EXPECT_EQ(candidate->GetRecordCount(), 1U);
  EXPECT_EQ(candidate->GetOverflowCount(), k_counter_maximum);
  EXPECT_EQ(candidate->GetCapturedPrimaryCount(), k_counter_maximum);
}

// =============================================================================
// =============================================================================

TEST(GGEMSTransportObserver,
     EmptyRunResultCandidateClearsPublishedResultAndPreservesConfiguration) {
  using Observer = ggems::core::observer::GGEMSTransportObserver;
  using ObserverCounters = ggems::core::observer::GGEMSObserverCounters;
  using ObserverRecord = ggems::core::observer::GGEMSObserverRecord;

  Observer observer{};
  observer.CaptureFirstPrimaries(5U).CapturePrimary(3U, 8ULL);

  std::array<ObserverRecord, 1U> const records{ObserverRecord{.run_id = 51ULL}};
  observer.Accumulate(records, ObserverCounters{.record_count = 1U,
                                                .overflow_count = 2U,
                                                .captured_primary_count = 1U,
                                                .reserved_0 = 0U});

  auto const configuration_before_commit = observer.BuildConfigRecord();
  auto candidate = observer.CreateRunResultCandidate();

  observer.CommitRunResult(*candidate);

  EXPECT_TRUE(observer.GetRecords().empty());
  EXPECT_EQ(observer.GetRecordCount(), 0U);
  EXPECT_EQ(observer.GetOverflowCount(), 0U);
  EXPECT_EQ(observer.GetCapturedPrimaryCount(), 0U);

  auto const configuration_after_commit = observer.BuildConfigRecord();
  EXPECT_EQ(configuration_after_commit.enabled,
            configuration_before_commit.enabled);
  EXPECT_EQ(configuration_after_commit.capture_first_primary_count_per_source,
            configuration_before_commit.capture_first_primary_count_per_source);
  EXPECT_EQ(configuration_after_commit.capture_specific_primary_enabled,
            configuration_before_commit.capture_specific_primary_enabled);
  EXPECT_EQ(configuration_after_commit.capture_source_index,
            configuration_before_commit.capture_source_index);
  EXPECT_EQ(configuration_after_commit.capture_source_local_primary_id,
            configuration_before_commit.capture_source_local_primary_id);
}
