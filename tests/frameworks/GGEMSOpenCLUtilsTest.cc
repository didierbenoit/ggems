#include <array>
#include <cstddef>
#include <string>
#include <string_view>
#include <unordered_set>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/frameworks/GGEMSOpenCLDevice.hh"
#include "GGEMS/frameworks/GGEMSOpenCLExternal.hh"
#include "GGEMS/frameworks/GGEMSOpenCLUtils.hh"
#include "GGEMSOpenCLDeviceInventory.hh"

namespace {

auto ExpectContains(std::string_view value, std::string_view expected) -> void {
  EXPECT_NE(value.find(expected), std::string_view::npos);
}

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLUtilsTest, ReportsRepresentativeErrorCodes) {
  struct ErrorNameCase {
    cl_int error_code;
    std::string_view expected_name;
  };

  constexpr std::array test_cases{
      ErrorNameCase{.error_code = CL_SUCCESS, .expected_name = "CL_SUCCESS"},
      ErrorNameCase{.error_code = CL_INVALID_VALUE,
                    .expected_name = "CL_INVALID_VALUE"},
      ErrorNameCase{.error_code = CL_INVALID_EVENT,
                    .expected_name = "CL_INVALID_EVENT"},
      ErrorNameCase{.error_code = 123'456, .expected_name = "CL_UNKNOWN_ERROR"},
  };

  for (auto const &test_case : test_cases) {
    SCOPED_TRACE(test_case.expected_name);
    EXPECT_EQ(ggems::ocl::GetErrorCodeName(test_case.error_code),
              test_case.expected_name);
  }

  EXPECT_EQ(ggems::ocl::GetErrorDescription(CL_INVALID_EVENT),
            "Invalid event object.");
  EXPECT_EQ(ggems::ocl::GetErrorDescription(123'456), "Unknown OpenCL error.");
}

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLUtilsTest, BuildsLongErrorDescription) {
  auto const description = ggems::ocl::GetLongErrorString(CL_INVALID_EVENT);
  ExpectContains(description, "CL_INVALID_EVENT");
  ExpectContains(description, "Invalid event object.");
}

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLUtilsTest,
     CheckCLErrorAcceptsSuccessAndPreservesFailureContext) {
  EXPECT_NO_THROW(ggems::ocl::CheckCLError(CL_SUCCESS, "OpenCL success"));

  try {
    ggems::ocl::CheckCLError<ggems::core::GGEMSRecoverable>(CL_INVALID_VALUE,
                                                            "OpenCL probe");
    FAIL() << "Expected GGEMSRecoverable.";
  } catch (ggems::core::GGEMSRecoverable const &exception) {
    auto const diagnostic = std::string_view{exception.what()};
    ExpectContains(diagnostic, "OpenCL probe");
    ExpectContains(diagnostic, "code -30");
    ExpectContains(diagnostic, "CL_INVALID_VALUE");
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLUtilsTest, FindsOnlyExactExtensions) {
  std::unordered_set<std::string> const extensions{"cl_khr_fp64",
                                                   "cl_khr_device_uuid"};

  EXPECT_TRUE(ggems::ocl::HasExtension(extensions, "cl_khr_fp64"));
  EXPECT_FALSE(ggems::ocl::HasExtension(extensions, "cl_khr_fp16"));
  EXPECT_FALSE(ggems::ocl::HasExtension(extensions, "CL_KHR_FP64"));
  EXPECT_FALSE(ggems::ocl::HasExtension(extensions, ""));
}

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLUtilsTest, ReadsRepresentativeNativeInformation) {
  auto const inventory = ggems::test::GetOpenCLDeviceInventory();
  ASSERT_FALSE(inventory.empty());

  for (auto const &entry : inventory) {
    SCOPED_TRACE(ggems::test::DescribeOpenCLDevice(entry));

    auto const &device = entry.device.get();
    auto const &native = device.GetDeviceNative();

    auto const name = ggems::ocl::GetInfo<CL_DEVICE_NAME>(native);
    EXPECT_EQ(name, device.GetName());
    EXPECT_EQ(name.find('\0'), std::string::npos);

    EXPECT_EQ(ggems::ocl::GetInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>(native),
              device.GetMaxWorkItemSizes());
    EXPECT_EQ(ggems::ocl::ExtractExtensions<CL_DEVICE_EXTENSIONS>(native),
              device.GetDeviceExtensions());
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLUtilsTest,
     ReadsFallbackStringWithoutTerminalNullWhenSpirIsAdvertised) {
  std::size_t compatible_device_count{0U};

  for (auto const &entry : ggems::test::GetOpenCLDeviceInventory()) {
    SCOPED_TRACE(ggems::test::DescribeOpenCLDevice(entry));

    auto const &device = entry.device.get();
    if (!ggems::ocl::HasExtension(device.GetDeviceExtensions(),
                                  "cl_khr_spir")) {
      continue;
    }
    ++compatible_device_count;

    auto const versions =
        ggems::ocl::GetInfo<CL_DEVICE_SPIR_VERSIONS>(device.GetDeviceNative());
    EXPECT_EQ(versions.find('\0'), std::string::npos);
  }

  if (compatible_device_count == 0U) {
    GTEST_SKIP() << "No GGEMS-discovered device advertises cl_khr_spir.";
  }
}
