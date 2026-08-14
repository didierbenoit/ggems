#pragma once

#include <memory>
#include <vector>

#include "GGEMS/frameworks/GGEMSOpenCLContext.hh"
#include "GGEMS/frameworks/GGEMSOpenCLDevice.hh"
#include "GGEMS/frameworks/GGEMSOpenCLExternal.hh"
#include "GGEMSOpenCLDeviceInventory.hh"

namespace ggems::test {

struct OpenCLCompilerDeviceInventoryEntry {
  OpenCLDeviceInventoryEntry inventory;
  std::unique_ptr<ocl::GGEMSOpenCLContext> context;
};

// =============================================================================
// =============================================================================

[[nodiscard]] inline auto GetOpenCLCompilerDeviceInventory()
    -> std::vector<OpenCLCompilerDeviceInventoryEntry> const & {
  // Cached GGEMSOpenCLProgram objects retain these contexts by reference.
  static auto const inventory =
      [] -> std::vector<OpenCLCompilerDeviceInventoryEntry> {
    std::vector<OpenCLCompilerDeviceInventoryEntry> result;

    for (auto const &entry : GetOpenCLDeviceInventory()) {
      auto const &device = entry.device.get();
      if (device.GetAvailable() == CL_FALSE ||
          device.GetCompilerAvailable() == CL_FALSE) {
        continue;
      }

      result.push_back(OpenCLCompilerDeviceInventoryEntry{
          .inventory = entry,
          .context = std::make_unique<ocl::GGEMSOpenCLContext>(device),
      });
    }

    return result;
  }();

  return inventory;
}
} // namespace ggems::test
