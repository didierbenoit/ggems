#include <cstddef>
#include <sstream>
#include <string>
#include <unordered_set>

#include <gtest/gtest.h>

#include "GGEMS/frameworks/GGEMSOpenCLDevice.hh"
#include "GGEMS/frameworks/GGEMSOpenCLExternal.hh"
#include "GGEMS/frameworks/GGEMSOpenCLPlatform.hh"
#include "GGEMS/frameworks/GGEMSOpenCLStrings.hh"
#include "GGEMS/frameworks/GGEMSOpenCLUtils.hh"
#include "GGEMSOpenCLDeviceInventory.hh"

namespace {

template <cl_device_info Info>
[[nodiscard]] auto GetNativeDeviceInfo(cl::Device const &device) {
  cl_int error{CL_SUCCESS};
  auto value = device.getInfo<Info>(&error);
  ggems::ocl::CheckCLError(error, "Failed to query native device info.");
  return value;
}

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLDeviceTest,
     RepresentativePropertiesMatchNativeDeviceInformation) {
  auto const inventory = ggems::test::GetOpenCLDeviceInventory();
  ASSERT_FALSE(inventory.empty());

  for (auto const &entry : inventory) {
    SCOPED_TRACE(ggems::test::DescribeOpenCLDevice(entry));

    auto const &device = entry.device.get();
    auto const &native = device.GetDeviceNative();

    EXPECT_EQ(device.GetName(), GetNativeDeviceInfo<CL_DEVICE_NAME>(native));
    EXPECT_EQ(device.GetVendor(),
              GetNativeDeviceInfo<CL_DEVICE_VENDOR>(native));
    EXPECT_EQ(device.GetVersion(),
              GetNativeDeviceInfo<CL_DEVICE_VERSION>(native));
    EXPECT_EQ(device.GetType(), GetNativeDeviceInfo<CL_DEVICE_TYPE>(native));
    EXPECT_EQ(device.GetMaxComputeUnits(),
              GetNativeDeviceInfo<CL_DEVICE_MAX_COMPUTE_UNITS>(native));
    EXPECT_EQ(device.GetMaxWorkGroupSize(),
              GetNativeDeviceInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>(native));
    EXPECT_EQ(device.GetMaxWorkItemDimensions(),
              GetNativeDeviceInfo<CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS>(native));
    EXPECT_EQ(device.GetMaxWorkItemSizes(),
              GetNativeDeviceInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>(native));
    EXPECT_EQ(device.GetGlobalMemSize(),
              GetNativeDeviceInfo<CL_DEVICE_GLOBAL_MEM_SIZE>(native));
    EXPECT_EQ(device.GetAvailable(),
              GetNativeDeviceInfo<CL_DEVICE_AVAILABLE>(native));
    EXPECT_EQ(device.GetPlatformID(),
              entry.platform.get().GetPlatformNative()());

    EXPECT_FALSE(device.GetName().empty());
    EXPECT_FALSE(device.GetVersion().empty());
    EXPECT_GT(device.GetMaxComputeUnits(), 0U);
    EXPECT_GT(device.GetMaxWorkGroupSize(), 0U);

    auto const work_item_sizes = device.GetMaxWorkItemSizes();
    EXPECT_EQ(work_item_sizes.size(),
              static_cast<std::size_t>(device.GetMaxWorkItemDimensions()));
    for (auto const work_item_size : work_item_sizes) {
      EXPECT_GT(work_item_size, 0U);
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLDeviceTest, CachedExtensionsMatchReportedExtensionString) {
  auto const inventory = ggems::test::GetOpenCLDeviceInventory();
  ASSERT_FALSE(inventory.empty());

  for (auto const &entry : inventory) {
    SCOPED_TRACE(ggems::test::DescribeOpenCLDevice(entry));

    auto const &device = entry.device.get();
    std::istringstream stream{device.GetExtensions()};
    std::unordered_set<std::string> parsed_extensions;
    std::string extension;
    while (stream >> extension) {
      parsed_extensions.insert(extension);
    }

    EXPECT_EQ(device.GetDeviceExtensions(), parsed_extensions);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLDeviceTest, AdvertisedDeviceUuidInformationIsQueryable) {
  std::size_t compatible_device_count{0U};

  for (auto const &entry : ggems::test::GetOpenCLDeviceInventory()) {
    SCOPED_TRACE(ggems::test::DescribeOpenCLDevice(entry));

    auto const &device = entry.device.get();
    if (!ggems::ocl::HasExtension(device.GetDeviceExtensions(),
                                  "cl_khr_device_uuid")) {
      continue;
    }
    ++compatible_device_count;

    auto const &native = device.GetDeviceNative();
    auto const device_uuid = GetNativeDeviceInfo<CL_DEVICE_UUID_KHR>(native);
    auto const driver_uuid = GetNativeDeviceInfo<CL_DRIVER_UUID_KHR>(native);

    EXPECT_EQ(device.GetUUIDKhr(), ggems::ocl::UUIDToString(device_uuid));
    EXPECT_EQ(device.GetDriverUUIDKhr(), ggems::ocl::UUIDToString(driver_uuid));
  }

  if (compatible_device_count == 0U) {
    GTEST_SKIP() << "No GGEMS-discovered device advertises cl_khr_device_uuid.";
  }
}
