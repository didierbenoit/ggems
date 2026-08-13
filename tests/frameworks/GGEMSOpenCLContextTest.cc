#include <gtest/gtest.h>

#include "GGEMS/frameworks/GGEMSOpenCLContext.hh"
#include "GGEMS/frameworks/GGEMSOpenCLExternal.hh"
#include "GGEMSOpenCLDeviceInventory.hh"

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLContextTest,
     CreatesCoherentOneDeviceContextAndQueueForEveryAvailableDevice) {
  auto const inventories = ggems::test::GetOpenCLDeviceInventory();
  ASSERT_FALSE(inventories.empty());

  for (auto const &inventory : inventories) {
    auto const description = ggems::test::DescribeOpenCLDevice(inventory);
    SCOPED_TRACE(description);

    auto const &platform = inventory.platform.get();
    auto const &device = inventory.device.get();

    EXPECT_EQ(inventory.platform_index, platform.GetPlatformIndex());
    EXPECT_EQ(inventory.platform_index, device.GetPlatformIndex());
    EXPECT_EQ(inventory.device_index, device.GetDeviceIndex());

    if (device.GetAvailable() == CL_FALSE) {
      continue;
    }

    ggems::ocl::GGEMSOpenCLContext context{device};

    EXPECT_EQ(&context.GetDevice(), &device);
    EXPECT_NE(context.GetContextNative()(), nullptr);
    EXPECT_NE(context.GetCommandQueueNative()(), nullptr);
    EXPECT_EQ(context.GetNumDevices(), 1U);

    auto const native_devices = context.GetNativeDevices();
    EXPECT_EQ(native_devices.size(), 1U);
    if (native_devices.size() == 1U) {
      EXPECT_EQ(native_devices.front()(), device.GetDeviceNative()());
    }

    EXPECT_EQ(context.GetQueueContext()(), context.GetContextNative()());
    EXPECT_EQ(context.GetQueueDevice()(), device.GetDeviceNative()());
    EXPECT_NE(context.GetQueueProperties() & CL_QUEUE_PROFILING_ENABLE,
              static_cast<cl_command_queue_properties>(0));
  }
}
