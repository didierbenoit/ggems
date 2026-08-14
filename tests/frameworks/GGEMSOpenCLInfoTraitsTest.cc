#include <algorithm>
#include <cstddef>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSLogger.hh"
#include "GGEMS/frameworks/GGEMSOpenCLExternal.hh"
#include "GGEMS/frameworks/GGEMSOpenCLInfoTraits.hh"
#include "GGEMSScopedLoggerEncoding.hh"

namespace {

using ClockFrequencyTraits =
    ggems::ocl::InfoTraits<CL_DEVICE_MAX_CLOCK_FREQUENCY>;
using PlatformNameTraits = ggems::ocl::InfoTraits<CL_PLATFORM_NAME>;
using DeviceAvailableTraits = ggems::ocl::InfoTraits<CL_DEVICE_AVAILABLE>;
using ComputeUnitTraits = ggems::ocl::InfoTraits<CL_DEVICE_MAX_COMPUTE_UNITS>;
using WorkGroupSizeTraits =
    ggems::ocl::InfoTraits<CL_DEVICE_MAX_WORK_GROUP_SIZE>;
using GlobalMemoryTraits = ggems::ocl::InfoTraits<CL_DEVICE_GLOBAL_MEM_SIZE>;
using QueuePropertiesTraits =
    ggems::ocl::InfoTraits<CL_DEVICE_QUEUE_ON_HOST_PROPERTIES>;
using WorkItemSizesTraits =
    ggems::ocl::InfoTraits<CL_DEVICE_MAX_WORK_ITEM_SIZES>;
using BinarySizesTraits = ggems::ocl::InfoTraits<CL_PROGRAM_BINARY_SIZES>;
using NameVersionsTraits =
    ggems::ocl::InfoTraits<CL_DEVICE_OPENCL_C_ALL_VERSIONS>;

using ggems::test::ScopedLoggerEncoding;

// =============================================================================
// =============================================================================

static_assert(std::is_same_v<PlatformNameTraits::type, std::string>);
static_assert(std::is_same_v<DeviceAvailableTraits::type, cl_bool>);
static_assert(PlatformNameTraits::name == "CL_PLATFORM_NAME");
static_assert(ggems::ocl::InfoTraits<CL_DEVICE_MAX_PARAMETER_SIZE>::name ==
              "CL_DEVICE_MAX_PARAMETER_SIZE");
static_assert(BinarySizesTraits::name == "CL_PROGRAM_BINARY_SIZES");

// =============================================================================
// =============================================================================

auto SetName(cl_name_version &name_version, std::string_view name) -> void {
  std::ranges::copy(name, name_version.name);
}

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

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLInfoTraitsTest, FormatsRepresentativeScalarCategories) {
  EXPECT_EQ(PlatformNameTraits::ToString("Portable platform"),
            "Portable platform");
  EXPECT_EQ(DeviceAvailableTraits::ToString(CL_TRUE), "Yes");
  EXPECT_EQ(DeviceAvailableTraits::ToString(CL_FALSE), "No");
  EXPECT_EQ(ComputeUnitTraits::ToString(12U), "12");
  EXPECT_EQ(WorkGroupSizeTraits::ToString(std::size_t{256U}), "256");
  EXPECT_EQ(ggems::ocl::InfoTraits<CL_PLATFORM_NUMERIC_VERSION>::ToString(
                CL_MAKE_VERSION(3, 0, 5)),
            "3.0.5");
}

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLInfoTraitsTest, FormatsMemoryAndFlagCategories) {
  ScopedLoggerEncoding const encoding{ggems::core::Encoding::Ascii};

  auto const memory = GlobalMemoryTraits::ToString(1'024U);
  EXPECT_NE(memory.find("KiB"), std::string::npos);
  EXPECT_EQ(QueuePropertiesTraits::ToString(0), "None");
  EXPECT_EQ(QueuePropertiesTraits::ToString(CL_QUEUE_PROFILING_ENABLE),
            "Profiling enabled");

  auto const alignment =
      ggems::ocl::InfoTraits<CL_DEVICE_MEM_BASE_ADDR_ALIGN>::ToString(128U);
  EXPECT_NE(alignment.find("bit"), std::string::npos);
}

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLInfoTraitsTest, FormatsVectorAndStructuredCategories) {
  auto const work_item_sizes =
      WorkItemSizesTraits::ToString(std::vector<std::size_t>{4U, 8U, 16U});
  EXPECT_NE(work_item_sizes.find("4 8 16"), std::string::npos);

  EXPECT_EQ(BinarySizesTraits::ToString(std::vector<std::size_t>{4U, 8U}),
            "[4, 8]");

  cl_name_version name_version{};
  name_version.version = CL_MAKE_VERSION(3, 0, 1);
  SetName(name_version, "opencl_c_feature");
  auto const structured = NameVersionsTraits::ToString({name_version});
  EXPECT_NE(structured.find("opencl_c_feature"), std::string::npos);
  EXPECT_NE(structured.find("3.0.1"), std::string::npos);
}
