#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

#include <gtest/gtest.h>

#include "GGEMS/core/sources/GGEMSSourceRunRange.hh"

// =============================================================================
// =============================================================================

TEST(GGEMSSourceRunRange, IsKernelFriendly) {
  using SourceRunRange = ggems::core::sources::GGEMSSourceRunRange;

  EXPECT_TRUE(std::is_standard_layout_v<SourceRunRange>);
  EXPECT_TRUE(std::is_trivially_copyable_v<SourceRunRange>);

  EXPECT_EQ(sizeof(SourceRunRange), 16U);
  EXPECT_EQ(alignof(SourceRunRange), 8U);

  EXPECT_EQ(offsetof(SourceRunRange, projection_primary_begin), 0U);
  EXPECT_EQ(offsetof(SourceRunRange, primary_count), 8U);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceRunRange, SupportsDisabledSourceRange) {
  ggems::core::sources::GGEMSSourceRunRange range{
      .projection_primary_begin = 3ULL, .primary_count = 0ULL};

  EXPECT_EQ(range.projection_primary_begin, 3ULL);
  EXPECT_EQ(range.primary_count, 0ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceRunRange, SupportsMaximumValues) {
  constexpr std::uint64_t k_maximum = std::numeric_limits<std::uint64_t>::max();

  ggems::core::sources::GGEMSSourceRunRange const maximum_begin{
      .projection_primary_begin = k_maximum, .primary_count = 0ULL};

  ggems::core::sources::GGEMSSourceRunRange const maximum_count{
      .projection_primary_begin = 0ULL, .primary_count = k_maximum};

  EXPECT_EQ(maximum_begin.projection_primary_begin, k_maximum);
  EXPECT_EQ(maximum_begin.primary_count, 0ULL);

  EXPECT_EQ(maximum_count.projection_primary_begin, 0ULL);
  EXPECT_EQ(maximum_count.primary_count, k_maximum);
}
