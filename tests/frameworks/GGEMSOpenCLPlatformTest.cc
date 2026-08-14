#include <algorithm>
#include <format>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/frameworks/GGEMSOpenCL.hh"
#include "GGEMS/frameworks/GGEMSOpenCLDevice.hh"
#include "GGEMS/frameworks/GGEMSOpenCLExternal.hh"
#include "GGEMS/frameworks/GGEMSOpenCLPlatform.hh"
#include "GGEMS/frameworks/GGEMSOpenCLUtils.hh"
#include "GGEMSOpenCLDeviceInventory.hh"

namespace {

template <cl_platform_info Info>
[[nodiscard]] auto GetNativePlatformInfo(cl::Platform const &platform) {
  cl_int error{CL_SUCCESS};
  auto value = platform.getInfo<Info>(&error);
  ggems::ocl::CheckCLError(error, "Failed to query native platform info.");
  return value;
}

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLPlatformTest,
     RepresentativePropertiesMatchNativePlatformInformation) {
  auto const &platforms = ggems::ocl::GGEMSOpenCL::GetInstance().GetPlatforms();
  ASSERT_FALSE(platforms.empty());

  for (auto const &platform : platforms) {
    SCOPED_TRACE(std::format("platform={}", platform.GetPlatformIndex()));
    auto const &native = platform.GetPlatformNative();

    EXPECT_EQ(platform.GetName(),
              GetNativePlatformInfo<CL_PLATFORM_NAME>(native));
    EXPECT_EQ(platform.GetVendor(),
              GetNativePlatformInfo<CL_PLATFORM_VENDOR>(native));
    EXPECT_EQ(platform.GetProfile(),
              GetNativePlatformInfo<CL_PLATFORM_PROFILE>(native));
    EXPECT_EQ(platform.GetVersion(),
              GetNativePlatformInfo<CL_PLATFORM_VERSION>(native));
    EXPECT_EQ(platform.GetExtensions(),
              GetNativePlatformInfo<CL_PLATFORM_EXTENSIONS>(native));

    EXPECT_FALSE(platform.GetName().empty());
    EXPECT_FALSE(platform.GetVendor().empty());
    EXPECT_FALSE(platform.GetVersion().empty());
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLPlatformTest, DiscoveredDevicesMatchTheNativePlatformSet) {
  auto const &platforms = ggems::ocl::GGEMSOpenCL::GetInstance().GetPlatforms();
  ASSERT_FALSE(platforms.empty());
  auto const inventory = ggems::test::GetOpenCLDeviceInventory();

  for (auto const &platform : platforms) {
    SCOPED_TRACE(std::format("platform={}", platform.GetPlatformIndex()));

    std::vector<cl::Device> native_devices;
    auto const error = platform.GetPlatformNative().getDevices(
        CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_GPU, &native_devices);
    ggems::ocl::CheckCLError(error, "Failed to query native platform devices.");

    auto const &devices = platform.GetDevices();
    EXPECT_EQ(devices.size(), native_devices.size());

    for (auto const &entry : inventory) {
      if (&entry.platform.get() != &platform) {
        continue;
      }

      SCOPED_TRACE(ggems::test::DescribeOpenCLDevice(entry));
      auto const &device = entry.device.get();
      auto const native_iterator = std::ranges::find_if(
          native_devices, [&](auto const &native_device) -> bool {
            return native_device() == device.GetDeviceNative()();
          });
      EXPECT_NE(native_iterator, native_devices.end());
    }
  }
}
