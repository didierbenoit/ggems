#include <cstdint>
#include <limits>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/particles/GGEMSPrimaryStream.hh"

// =============================================================================
// =============================================================================

TEST(GGEMSPrimaryStream, ProducesContiguousDisjointFixedCountRanges) {
  constexpr std::uint64_t k_primary_count{17ULL};

  ggems::core::particles::GGEMSPrimaryStream stream{};
  stream.SetPrimaryCount(k_primary_count);
  stream.Initialise();

  auto first = stream.PrepareRun(0ULL);
  auto second = stream.PrepareRun(1ULL);
  auto third = stream.PrepareRun(2ULL);

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

TEST(GGEMSPrimaryStream, UsesConfiguredPrimaryCountInRunMetadata) {
  constexpr std::uint64_t k_primary_count{37ULL};

  ggems::core::particles::GGEMSPrimaryStream stream{};
  stream.SetPrimaryCount(k_primary_count);
  stream.Initialise();

  auto run_view = stream.PrepareRun(0ULL);

  EXPECT_EQ(stream.GetPrimaryCount(), k_primary_count);
  EXPECT_EQ(run_view.source_primary_count, k_primary_count);
}

// =============================================================================
// =============================================================================

TEST(GGEMSPrimaryStream, RejectsPrimaryCountMutationAfterInitialise) {
  constexpr std::uint64_t k_initial_primary_count{13ULL};

  ggems::core::particles::GGEMSPrimaryStream stream{};
  stream.SetPrimaryCount(k_initial_primary_count);
  stream.Initialise();

  EXPECT_THROW(stream.SetPrimaryCount(19ULL), ggems::core::GGEMSExceptionBase);

  EXPECT_EQ(stream.GetPrimaryCount(), k_initial_primary_count);
}

// =============================================================================
// =============================================================================

TEST(GGEMSPrimaryStream, RejectsUnrepresentableGlobalRangeOffset) {
  constexpr std::uint64_t k_primary_count{2ULL};

  ggems::core::particles::GGEMSPrimaryStream stream{};
  stream.SetPrimaryCount(k_primary_count);
  stream.Initialise();

  std::uint64_t first_unrepresentable_run_id =
      std::numeric_limits<std::uint64_t>::max() / k_primary_count + 1ULL;

  EXPECT_THROW(
      static_cast<void>(stream.PrepareRun(first_unrepresentable_run_id)),
      ggems::core::GGEMSExceptionBase);
}
