#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSLogger.hh"
#include "GGEMS/frameworks/GGEMSOpenCLExternal.hh"
#include "GGEMS/frameworks/GGEMSOpenCLInfoTraits.hh"
#include "GGEMSScopedLoggerEncoding.hh"

namespace {

using ClockFrequencyTraits =
    ggems::ocl::InfoTraits<CL_DEVICE_MAX_CLOCK_FREQUENCY>;
using ggems::test::ScopedLoggerEncoding;

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLInfoTraitsTest, ReportsUnavailableClockFrequency) {
  EXPECT_EQ(ClockFrequencyTraits::ToString(0U), "N/A");
}

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLInfoTraitsTest, FormatsReportedClockFrequency) {
  ScopedLoggerEncoding const encoding{ggems::core::Encoding::Ascii};

  EXPECT_EQ(ClockFrequencyTraits::ToString(2'000U), "  2.0 GHz");
}
