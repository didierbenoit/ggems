#include <algorithm>
#include <cstddef>
#include <array>
#include <string>
#include <string_view>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/frameworks/GGEMSOpenCL.hh"
#include "GGEMS/frameworks/GGEMSOpenCLDevice.hh"
#include "GGEMS/frameworks/GGEMSOpenCLPlatform.hh"
#include "GGEMSOpenCLDeviceInventory.hh"

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLTest, SingletonIdentityIsStable) {
  auto &first = ggems::ocl::GGEMSOpenCL::GetInstance();
  auto &second = ggems::ocl::GGEMSOpenCL::GetInstance();

  EXPECT_EQ(&first, &second);
  EXPECT_EQ(&first.GetPlatforms(), &second.GetPlatforms());
}

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLTest, DiscoveryHierarchyIsCoherent) {
  auto const &platforms = ggems::ocl::GGEMSOpenCL::GetInstance().GetPlatforms();
  ASSERT_FALSE(platforms.empty());

  auto const inventory = ggems::test::GetOpenCLDeviceInventory();
  std::size_t discovered_device_count{0U};
  for (auto const &platform : platforms) {
    discovered_device_count += platform.GetDevices().size();
  }
  EXPECT_EQ(inventory.size(), discovered_device_count);

  for (auto const &entry : inventory) {
    SCOPED_TRACE(ggems::test::DescribeOpenCLDevice(entry));

    auto const &platform = entry.platform.get();
    auto const &device = entry.device.get();

    auto const platform_iterator =
        std::ranges::find_if(platforms, [&](auto const &candidate) -> bool {
          return &candidate == &platform;
        });
    EXPECT_NE(platform_iterator, platforms.end());
    if (platform_iterator == platforms.end()) {
      continue;
    }

    auto const &devices = platform.GetDevices();
    auto const device_iterator =
        std::ranges::find_if(devices, [&](auto const &candidate) -> bool {
          return &candidate == &device;
        });
    EXPECT_NE(device_iterator, devices.end());

    EXPECT_NE(platform.GetPlatformNative()(), nullptr);
    EXPECT_NE(device.GetDeviceNative()(), nullptr);
    EXPECT_EQ(device.GetPlatformID(), platform.GetPlatformNative()());
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLTest, RejectsInvalidDeviceSelectors) {
  auto const inventory = ggems::test::GetOpenCLDeviceInventory();
  ASSERT_FALSE(inventory.empty());

  constexpr std::array<std::string_view, 7> invalid_selectors{
      "toot", "all;gpu", "0;gpu", "1-0", "cpu;gpu", "intel;nvidia", "0;;1",
  };

  auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();

  for (auto const selector : invalid_selectors) {
    SCOPED_TRACE(selector);

    EXPECT_THROW(opencl.SelectDevices({std::string{selector}}),
                 ggems::core::GGEMSFatal);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLTest, RejectsOutOfRangeDeviceIndex) {
  auto const inventory = ggems::test::GetOpenCLDeviceInventory();
  ASSERT_FALSE(inventory.empty());

  auto const out_of_range_index = std::to_string(inventory.size());

  EXPECT_THROW(ggems::ocl::GGEMSOpenCL::GetInstance().SelectDevices(
                   {out_of_range_index}),
               ggems::core::GGEMSFatal);
}
