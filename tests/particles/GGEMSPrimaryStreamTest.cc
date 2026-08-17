#include <cstdint>
#include <limits>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/particles/GGEMSPrimaryStream.hh"

// =============================================================================
// =============================================================================

TEST(GGEMSPrimaryStream, ProducesContiguousDisjointFixedCountRanges) {
  constexpr std::uint64_t k_primary_count{17ULL};

  ggems::core::particles::GGEMSPrimaryStream stream{};
  stream.SetPrimaryCount(k_primary_count);
  stream.Initialize();

  auto first = stream.PrepareRun(0ULL);
  auto second = stream.PrepareRun(1ULL);
  auto third = stream.PrepareRun(2ULL);

  EXPECT_EQ(first.run_id, 0ULL);
  EXPECT_EQ(second.run_id, 1ULL);
  EXPECT_EQ(third.run_id, 2ULL);

  EXPECT_EQ(first.source_primary_count, k_primary_count);
  EXPECT_EQ(second.source_primary_count, k_primary_count);
  EXPECT_EQ(third.source_primary_count, k_primary_count);

  EXPECT_EQ(first.global_history_offset, 0ULL);
  EXPECT_LT(first.global_history_offset, second.global_history_offset);
  EXPECT_LT(second.global_history_offset, third.global_history_offset);

  std::uint64_t first_end =
      first.global_history_offset + first.source_primary_count;

  std::uint64_t second_end =
      second.global_history_offset + second.source_primary_count;

  EXPECT_EQ(first_end, second.global_history_offset);
  EXPECT_EQ(second_end, third.global_history_offset);
}

// =============================================================================
// =============================================================================

TEST(GGEMSPrimaryStream, ReservesContiguousDisjointVariableCountRanges) {
  ggems::core::particles::GGEMSPrimaryStream stream{};
  stream.Initialize();

  auto first = stream.PrepareRun(0ULL, 3ULL);
  auto second = stream.PrepareRun(1ULL, 7ULL);
  auto third = stream.PrepareRun(2ULL, 1ULL);
  auto fourth = stream.PrepareRun(3ULL, 5ULL);

  EXPECT_EQ(first.run_id, 0ULL);
  EXPECT_EQ(second.run_id, 1ULL);
  EXPECT_EQ(third.run_id, 2ULL);
  EXPECT_EQ(fourth.run_id, 3ULL);

  EXPECT_EQ(first.source_primary_count, 3ULL);
  EXPECT_EQ(second.source_primary_count, 7ULL);
  EXPECT_EQ(third.source_primary_count, 1ULL);
  EXPECT_EQ(fourth.source_primary_count, 5ULL);

  EXPECT_EQ(first.global_history_offset, 0ULL);
  EXPECT_EQ(second.global_history_offset, 3ULL);
  EXPECT_EQ(third.global_history_offset, 10ULL);
  EXPECT_EQ(fourth.global_history_offset, 11ULL);

  EXPECT_EQ(first.global_history_offset + first.source_primary_count,
            second.global_history_offset);
  EXPECT_EQ(second.global_history_offset + second.source_primary_count,
            third.global_history_offset);
  EXPECT_EQ(third.global_history_offset + third.source_primary_count,
            fourth.global_history_offset);
}

// =============================================================================
// =============================================================================

TEST(GGEMSPrimaryStream, KeepsRunLabelsIndependentFromReservedRanges) {
  ggems::core::particles::GGEMSPrimaryStream stream{};
  stream.Initialize();

  auto first = stream.PrepareRun(42ULL, 3ULL);
  auto second = stream.PrepareRun(42ULL, 5ULL);
  auto third = stream.PrepareRun(7ULL, 2ULL);

  EXPECT_EQ(first.run_id, 42ULL);
  EXPECT_EQ(second.run_id, 42ULL);
  EXPECT_EQ(third.run_id, 7ULL);

  EXPECT_EQ(first.source_primary_count, 3ULL);
  EXPECT_EQ(second.source_primary_count, 5ULL);
  EXPECT_EQ(third.source_primary_count, 2ULL);

  EXPECT_EQ(first.global_history_offset, 0ULL);
  EXPECT_EQ(second.global_history_offset, 3ULL);
  EXPECT_EQ(third.global_history_offset, 8ULL);

  EXPECT_NE(first.global_history_offset, second.global_history_offset);
  EXPECT_EQ(first.global_history_offset + first.source_primary_count,
            second.global_history_offset);
  EXPECT_EQ(second.global_history_offset + second.source_primary_count,
            third.global_history_offset);
}

// =============================================================================
// =============================================================================

TEST(GGEMSPrimaryStream, UsesConfiguredPrimaryCountInRunMetadata) {
  constexpr std::uint64_t k_primary_count{37ULL};

  ggems::core::particles::GGEMSPrimaryStream stream{};
  stream.SetPrimaryCount(k_primary_count);
  stream.Initialize();

  auto run_view = stream.PrepareRun(0ULL);

  EXPECT_EQ(stream.GetPrimaryCount(), k_primary_count);
  EXPECT_EQ(run_view.source_primary_count, k_primary_count);
}

// =============================================================================
// =============================================================================

TEST(GGEMSPrimaryStream, RejectsPrimaryCountMutationAfterInitialize) {
  constexpr std::uint64_t k_initial_primary_count{13ULL};

  ggems::core::particles::GGEMSPrimaryStream stream{};
  stream.SetPrimaryCount(k_initial_primary_count);
  stream.Initialize();

  EXPECT_THROW(stream.SetPrimaryCount(19ULL), ggems::core::GGEMSExceptionBase);

  EXPECT_EQ(stream.GetPrimaryCount(), k_initial_primary_count);
}

// =============================================================================
// =============================================================================

TEST(GGEMSPrimaryStream, RejectsZeroReservationWithoutConsumingIndentifiers) {
  ggems::core::particles::GGEMSPrimaryStream stream{};
  stream.Initialize();

  EXPECT_THROW(static_cast<void>(stream.PrepareRun(41ULL, 0ULL)),
               ggems::core::GGEMSExceptionBase);

  auto reservation = stream.PrepareRun(42ULL, 4ULL);

  EXPECT_EQ(reservation.run_id, 42ULL);
  EXPECT_EQ(reservation.source_primary_count, 4ULL);
  EXPECT_EQ(reservation.global_history_offset, 0ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSPrimaryStream,
     PreservesStateAfterUnrepresentableRangeAndExhautsAtMaximumId) {
  constexpr std::uint64_t k_maximum_id{
      std::numeric_limits<std::uint64_t>::max()};

  ggems::core::particles::GGEMSPrimaryStream stream{};
  stream.Initialize();

  auto first = stream.PrepareRun(0ULL, k_maximum_id);

  EXPECT_EQ(first.run_id, 0ULL);
  EXPECT_EQ(first.source_primary_count, k_maximum_id);
  EXPECT_EQ(first.global_history_offset, 0ULL);
  EXPECT_EQ(first.source_primary_count - 1ULL, k_maximum_id - 1ULL);

  EXPECT_THROW(static_cast<void>(stream.PrepareRun(1ULL, 2ULL)),
               ggems::core::GGEMSExceptionBase);

  auto last = stream.PrepareRun(2ULL, 1ULL);

  EXPECT_EQ(last.run_id, 2ULL);
  EXPECT_EQ(last.source_primary_count, 1ULL);
  EXPECT_EQ(last.global_history_offset, k_maximum_id);

  EXPECT_THROW(static_cast<void>(stream.PrepareRun(3ULL, 1ULL)),
               ggems::core::GGEMSExceptionBase);
}

// =============================================================================
// =============================================================================

TEST(GGEMSPrimaryStream, RejectsSecondInitializeWithoutResettingReservations) {
  ggems::core::particles::GGEMSPrimaryStream stream{};
  stream.Initialize();

  auto first = stream.PrepareRun(11ULL, 3ULL);

  EXPECT_THROW(stream.Initialize(), ggems::core::GGEMSExceptionBase);

  auto second = stream.PrepareRun(12ULL, 2ULL);

  EXPECT_EQ(first.run_id, 11ULL);
  EXPECT_EQ(first.source_primary_count, 3ULL);
  EXPECT_EQ(first.global_history_offset, 0ULL);

  EXPECT_EQ(second.run_id, 12ULL);
  EXPECT_EQ(second.source_primary_count, 2ULL);
  EXPECT_EQ(second.global_history_offset, 3ULL);
}
