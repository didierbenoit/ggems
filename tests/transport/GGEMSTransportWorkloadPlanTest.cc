#include <cstdint>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/transport/GGEMSTransportWorkloadPlan.hh"

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

TEST(GGEMSTransportWorkloadPlan, SplitsPrimariesEquallyWithRemainder) {
  std::vector<ggems::core::transport::GGEMSTransportWorkloadPlan> plan =
      ggems::core::transport::BuildEqualTransportWorkloadPlan(10'000ULL, 4096U,
                                                              3U, 256U);

  ASSERT_EQ(plan.size(), 3U);

  EXPECT_EQ(plan[0].workload_index, 0U);
  EXPECT_EQ(plan[0].context_index, 0U);
  EXPECT_EQ(plan[0].primary_count, 1366U);
  EXPECT_EQ(plan[0].device_primary_offset, 0ULL);

  EXPECT_EQ(plan[1].workload_index, 1U);
  EXPECT_EQ(plan[1].context_index, 1U);
  EXPECT_EQ(plan[1].primary_count, 1365U);
  EXPECT_EQ(plan[1].device_primary_offset, 1366ULL);

  EXPECT_EQ(plan[2].workload_index, 2U);
  EXPECT_EQ(plan[2].context_index, 2U);
  EXPECT_EQ(plan[2].primary_count, 1365U);
  EXPECT_EQ(plan[2].device_primary_offset, 2731ULL);

  for (auto const &workload : plan) {
    EXPECT_EQ(workload.worker_count, 256U);
    EXPECT_EQ(workload.projection_history_offset, 10'000ULL);
  }

  EXPECT_EQ(ggems::core::transport::CountAssignedPrimaries(plan), 4096ULL);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

TEST(GGEMSTransportWorkloadPlan, AllowsMoreWorkloadsThanPrimaries) {
  std::vector<ggems::core::transport::GGEMSTransportWorkloadPlan> plan =
      ggems::core::transport::BuildEqualTransportWorkloadPlan(0ULL, 2U, 4U,
                                                              128U);

  ASSERT_EQ(plan.size(), 4U);

  EXPECT_EQ(plan[0].primary_count, 1U);
  EXPECT_EQ(plan[0].device_primary_offset, 0ULL);

  EXPECT_EQ(plan[1].primary_count, 1U);
  EXPECT_EQ(plan[1].device_primary_offset, 1ULL);

  EXPECT_EQ(plan[2].primary_count, 0U);
  EXPECT_EQ(plan[2].device_primary_offset, 2ULL);

  EXPECT_EQ(plan[3].primary_count, 0U);
  EXPECT_EQ(plan[3].device_primary_offset, 2ULL);

  EXPECT_EQ(ggems::core::transport::CountAssignedPrimaries(plan), 2ULL);
}
