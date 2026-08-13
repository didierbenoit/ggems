#pragma once

#include <cstddef>
#include <format>
#include <functional>
#include <string>
#include <vector>

#include "GGEMS/frameworks/GGEMSOpenCL.hh"
#include "GGEMS/frameworks/GGEMSOpenCLDevice.hh"
#include "GGEMS/frameworks/GGEMSOpenCLPlatform.hh"
#include "GGEMS/frameworks/GGEMSOpenCLStrings.hh"

namespace ggems::test {

struct OpenCLDeviceInventoryEntry {
  std::size_t platform_index;
  std::size_t device_index;
  std::reference_wrapper<ocl::GGEMSOpenCLPlatform const> platform;
  std::reference_wrapper<ocl::GGEMSOpenCLDevice const> device;
};

[[nodiscard]] inline auto GetOpenCLDeviceInventory()
    -> std::vector<OpenCLDeviceInventoryEntry> {
  std::vector<OpenCLDeviceInventoryEntry> inventory;

  auto const &platforms = ocl::GGEMSOpenCL::GetInstance().GetPlatforms();
  for (auto const &platform : platforms) {
    for (auto const &device : platform.GetDevices()) {
      inventory.push_back(OpenCLDeviceInventoryEntry{
          .platform_index = platform.GetPlatformIndex(),
          .device_index = device.GetDeviceIndex(),
          .platform = std::cref(platform),
          .device = std::cref(device),
      });
    }
  }

  return inventory;
}

[[nodiscard]] inline auto
DescribeOpenCLDevice(OpenCLDeviceInventoryEntry const &entry) -> std::string {
  auto const &device = entry.device.get();
  return std::format("platform={}, device={}, name='{}', vendor='{}', type={}",
                     entry.platform_index, entry.device_index, device.GetName(),
                     device.GetVendor(),
                     ocl::DeviceTypeToString(device.GetType()));
}
} // namespace ggems::test
