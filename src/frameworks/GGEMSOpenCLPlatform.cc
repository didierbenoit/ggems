#include <utility>
#include <cstddef>
#include <string>
#include <vector>

#include "GGEMS/core/GGEMSLogMacros.hh"
#include "GGEMS/frameworks/GGEMSOpenCLPlatform.hh"
#include "GGEMS/frameworks/GGEMSOpenCLDevice.hh"
#include "GGEMS/frameworks/GGEMSOpenCLUtils.hh"

namespace ggems::ocl {

// =============================================================================
// =============================================================================

GGEMSOpenCLPlatform::GGEMSOpenCLPlatform(cl::Platform platform,
                                         std::size_t platform_index)
    : platform_{std::move(platform)}, platform_index_{platform_index} {
  GGEMS_INFOEX("OpenCL", 3, "Creating OpenCL platform [{}].", platform_index_);

  DiscoverDevices();

  GGEMS_INFOEX("OpenCL", 2, "OpenCL platform [{}] registered with {} device(s)",
               platform_index_, devices_.size());
}

// -----------------------------------------------------------------------------

GGEMSOpenCLPlatform::~GGEMSOpenCLPlatform() {
  GGEMS_INFOEX("OpenCL", 3, "Destroying OpenCL platform [{}].",
               platform_index_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLPlatform::GetName() const -> std::string {
  return GetInfo<CL_PLATFORM_NAME>(platform_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLPlatform::GetProfile() const -> std::string {
  return GetInfo<CL_PLATFORM_PROFILE>(platform_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLPlatform::GetVersion() const -> std::string {
  return GetInfo<CL_PLATFORM_VERSION>(platform_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLPlatform::GetVendor() const -> std::string {
  return GetInfo<CL_PLATFORM_VENDOR>(platform_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLPlatform::GetExtensions() const -> std::string {
  return GetInfo<CL_PLATFORM_EXTENSIONS>(platform_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLPlatform::GetNumericVersion() const -> cl_version {
  return GetInfo<CL_PLATFORM_NUMERIC_VERSION>(platform_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLPlatform::GetHostTimerResolution() const -> cl_ulong {
  return GetInfo<CL_PLATFORM_HOST_TIMER_RESOLUTION>(platform_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLPlatform::GetExtensionsWithVersion() const
    -> std::vector<cl_name_version> {
  return GetInfo<CL_PLATFORM_EXTENSIONS_WITH_VERSION>(platform_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLPlatform::PrintIdentity() const -> void {
  PrintInfo<CL_PLATFORM_PROFILE>(platform_);
  PrintInfo<CL_PLATFORM_VERSION>(platform_);
  PrintInfo<CL_PLATFORM_NUMERIC_VERSION>(platform_);
  PrintInfo<CL_PLATFORM_HOST_TIMER_RESOLUTION>(platform_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLPlatform::PrintExtension() const -> void {
  PrintInfo<CL_PLATFORM_EXTENSIONS>(platform_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLPlatform::Print() const -> void {
  GGEMS_INFO("OpenCL", "Platform [{}]: {} ({})", platform_index_, GetName(),
             GetVendor());
  GGEMS_INFO("OpenCL", "Discovered devices: {}", devices_.size());

  PrintIdentity();
  PrintExtension();
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLPlatform::Clean() -> void {
  GGEMS_INFOEX("OpenCL", 3, "Cleaning OpenCL platform [{}] resources.",
               GetName());

  platform_.unloadCompiler();
  devices_.clear();

  GGEMS_INFOEX("OpenCL", 3, "OpenCL platform [{}] resources cleaned.",
               platform_index_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLPlatform::DiscoverDevices() -> void {
  GGEMS_INFOEX("OpenCL", 2, "Discovering OpenCL devices for platform [{}].",
               platform_index_);

  constexpr cl_device_type mask = CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_GPU;

  std::vector<cl::Device> natives;
  {
    auto const opencl_error_code = (platform_.getDevices(mask, &natives));
    ggems::ocl::CheckCLError(opencl_error_code,
                             "No OpenCL devices detected on this platform.");
  }

  devices_.clear();
  devices_.reserve(natives.size());

  for (std::size_t i = 0; i < natives.size(); ++i) {
    devices_.emplace_back(natives[i], platform_index_, i);
  }

  GGEMS_INFOEX("OpenCL", 2, "{} OpenCL device(s) found on platform [{}].",
               natives.size(), platform_index_);
}
} // namespace ggems::ocl
