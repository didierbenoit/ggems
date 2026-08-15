#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/transport/GGEMSTransportWorkloadPlan.hh"

namespace {

using Chunk = ggems::core::transport::GGEMSTransportChunk;
using ChunkIterator = ggems::core::transport::GGEMSTransportChunkIterator;
using WorkloadPlan = ggems::core::transport::GGEMSTransportWorkloadPlan;

[[nodiscard]] auto CollectChunks(std::uint64_t offset, std::uint64_t count,
                                 std::uint32_t limit) -> std::vector<Chunk> {
  ChunkIterator iterator{offset, count, limit};
  std::vector<Chunk> chunks;
  while (iterator.HasNext()) {
    chunks.push_back(iterator.Next());
  }
  return chunks;
}

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSTransportChunking, IteratesBoundaryTotalsWithoutMaterializedPlan) {
  constexpr std::uint32_t k_limit{7U};
  constexpr std::uint64_t k_offset{101ULL};

  EXPECT_TRUE(CollectChunks(k_offset, 0ULL, k_limit).empty());

  auto one = CollectChunks(k_offset, 1ULL, k_limit);
  ASSERT_EQ(one.size(), 1U);
  EXPECT_EQ(one[0U].primary_count, 1U);
  EXPECT_EQ(one[0U].device_primary_offset, k_offset);

  auto exact = CollectChunks(k_offset, k_limit, k_limit);
  ASSERT_EQ(exact.size(), 1U);
  EXPECT_EQ(exact[0U].primary_count, k_limit);
  EXPECT_EQ(exact[0U].device_primary_offset, k_offset);

  auto plus_one = CollectChunks(k_offset, k_limit + 1ULL, k_limit);
  ASSERT_EQ(plus_one.size(), 2U);
  EXPECT_EQ(plus_one[0U].primary_count, k_limit);
  EXPECT_EQ(plus_one[0U].device_primary_offset, k_offset);
  EXPECT_EQ(plus_one[1U].primary_count, 1U);
  EXPECT_EQ(plus_one[1U].device_primary_offset, k_offset + k_limit);

  auto multiple = CollectChunks(k_offset, 3ULL * k_limit, k_limit);
  ASSERT_EQ(multiple.size(), 3U);
  for (std::size_t index = 0U; index < multiple.size(); ++index) {
    EXPECT_EQ(multiple[index].primary_count, k_limit);
    EXPECT_EQ(multiple[index].device_primary_offset,
              k_offset + index * k_limit);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSTransportChunking, SupportsUint64TotalsAndRejectsInvalidIntervals) {
  constexpr std::uint32_t k_workers{64U};
  std::uint32_t const safe_limit =
      ggems::core::transport::ComputeSafeTransportLaunchPrimaryCount(k_workers);
  EXPECT_EQ(safe_limit, std::numeric_limits<std::uint32_t>::max() - k_workers);

  std::uint64_t const total = static_cast<std::uint64_t>(safe_limit) + 123ULL;
  auto chunks = CollectChunks(1'000ULL, total, safe_limit);
  ASSERT_EQ(chunks.size(), 2U);
  EXPECT_EQ(chunks[0U].primary_count, safe_limit);
  EXPECT_EQ(chunks[0U].device_primary_offset, 1'000ULL);
  EXPECT_EQ(chunks[1U].primary_count, 123U);
  EXPECT_EQ(chunks[1U].device_primary_offset,
            1'000ULL + static_cast<std::uint64_t>(safe_limit));

  EXPECT_THROW((void)(ChunkIterator{0ULL, 1ULL, 0U}),
               ggems::core::GGEMSExceptionBase);
  EXPECT_THROW((void)(ChunkIterator{std::numeric_limits<std::uint64_t>::max(),
                                    2ULL, 1U}),
               ggems::core::GGEMSExceptionBase);

  ChunkIterator exhausted{0ULL, 0ULL, 1U};
  EXPECT_THROW((void)exhausted.Next(), ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(
      (void)ggems::core::transport::ComputeSafeTransportLaunchPrimaryCount(0U),
      ggems::core::GGEMSExceptionBase);
  EXPECT_THROW(
      (void)ggems::core::transport::ComputeSafeTransportLaunchPrimaryCount(
          std::numeric_limits<std::uint32_t>::max()),
      ggems::core::GGEMSExceptionBase);
}

// =============================================================================
// =============================================================================

TEST(GGEMSTransportChunking, DeviceSlicesAreDisjointExhaustiveAndUint64) {
  std::uint64_t const total =
      2ULL * std::numeric_limits<std::uint32_t>::max() + 17ULL;
  auto plan = ggems::core::transport::BuildEqualTransportWorkloadPlan(
      9'000ULL, total, 3U, 64U);

  ASSERT_EQ(plan.size(), 3U);
  EXPECT_EQ(plan[0U].primary_count, total / 3ULL + 1ULL);
  EXPECT_EQ(plan[1U].primary_count, total / 3ULL + 1ULL);
  EXPECT_EQ(plan[2U].primary_count, total / 3ULL);
  EXPECT_EQ(plan[0U].device_primary_offset, 0ULL);
  EXPECT_EQ(plan[1U].device_primary_offset, plan[0U].primary_count);
  EXPECT_EQ(plan[2U].device_primary_offset,
            plan[0U].primary_count + plan[1U].primary_count);
  EXPECT_EQ(plan[2U].device_primary_offset + plan[2U].primary_count, total);
  EXPECT_EQ(ggems::core::transport::CountAssignedPrimaries(plan), total);

  for (std::size_t index = 0U; index < plan.size(); ++index) {
    EXPECT_EQ(plan[index].workload_index, index);
    EXPECT_EQ(plan[index].context_index, index);
    EXPECT_EQ(plan[index].projection_history_offset, 9'000ULL);
    EXPECT_EQ(plan[index].worker_count, 64U);
  }

  std::vector<WorkloadPlan> overflowing{
      {.primary_count = std::numeric_limits<std::uint64_t>::max()},
      {.primary_count = 1ULL},
  };
  EXPECT_THROW(
      (void)ggems::core::transport::CountAssignedPrimaries(overflowing),
      ggems::core::GGEMSExceptionBase);
}
