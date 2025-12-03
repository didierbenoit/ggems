#include "GGEMS/frameworks/GGEMSOpenCLPlatform.hh"
#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/frameworks/GGEMSOpenCLDevice.hh"

namespace ggems::ocl {

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

GGEMSOpenCLPlatform::GGEMSOpenCLPlatform(cl::Platform const &platform,
                                         std::size_t platform_index)
    : platform_{platform}, platform_index_{platform_index} {
  GGEMS_INFOEX("OpenCL", 2, "Allocating GGEMSOpenCLPlatform [{}]...",
               platform_index_);

  extensions_ = ExtractExtensions<CL_PLATFORM_EXTENSIONS>(platform_);

  // Discover CPU/GPU devices now; contexts/queues are created later on demand.
  DiscoverDevices();

  GGEMS_INFOEX("OpenCL", 2, "GGEMSOpenCLPlatform allocated with {} device(s)",
               devices_.size());
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

GGEMSOpenCLPlatform::~GGEMSOpenCLPlatform() {
  GGEMS_INFOEX("OpenCL", 2, "Releasing GGEMSOpenCLPlatform [{}]",
               platform_index_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

std::string GGEMSOpenCLPlatform::GetName() const {
  return GetInfo<CL_PLATFORM_NAME>(platform_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

std::string GGEMSOpenCLPlatform::GetProfile() const {
  return GetInfo<CL_PLATFORM_PROFILE>(platform_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

std::string GGEMSOpenCLPlatform::GetVersion() const {
  return GetInfo<CL_PLATFORM_VERSION>(platform_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

std::string GGEMSOpenCLPlatform::GetVendor() const {
  return GetInfo<CL_PLATFORM_VENDOR>(platform_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

std::string GGEMSOpenCLPlatform::GetExtensions() const {
  return GetInfo<CL_PLATFORM_EXTENSIONS>(platform_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

cl_version GGEMSOpenCLPlatform::GetNumericVersion() const {
  return GetInfo<CL_PLATFORM_NUMERIC_VERSION>(platform_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

cl_ulong GGEMSOpenCLPlatform::GetHostTimerResolution() const {
  return GetInfo<CL_PLATFORM_HOST_TIMER_RESOLUTION>(platform_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

std::vector<cl_name_version>
GGEMSOpenCLPlatform::GetExtensionsWithVersion() const {
  return GetInfo<CL_PLATFORM_EXTENSIONS_WITH_VERSION>(platform_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

void GGEMSOpenCLPlatform::PrintIdentity() const {
  PrintInfo<CL_PLATFORM_PROFILE>(platform_);
  PrintInfo<CL_PLATFORM_VERSION>(platform_);
  PrintInfo<CL_PLATFORM_NUMERIC_VERSION>(platform_);
  PrintInfo<CL_PLATFORM_HOST_TIMER_RESOLUTION>(platform_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

void GGEMSOpenCLPlatform::PrintExtension() const {
  PrintInfo<CL_PLATFORM_EXTENSIONS_WITH_VERSION>(platform_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

void GGEMSOpenCLPlatform::Print() const {
  GGEMS_INFO("OpenCL", "++++++++++++++++++++++++++");

  GGEMS_INFO("OpenCL", "Platform [{}]: {} ({})", platform_index_, GetName(),
             GetVendor());
  GGEMS_INFO("OpenCL", "Discovered devices: {}", devices_.size());
  GGEMS_INFO("OpenCL", "++++++++++++++++++++++++++");

  PrintIdentity();
  PrintExtension();
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

void GGEMSOpenCLPlatform::Clean() {
  GGEMS_INFOEX("OpenCL", 2, "Cleaning Platform {} resources...", GetName());

  platform_.unloadCompiler();
  devices_.clear();
  extensions_.clear();

  GGEMS_INFOEX("OpenCL", 2, "Platform resources cleaned.");
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

void GGEMSOpenCLPlatform::DiscoverDevices() {
  GGEMS_INFOEX("OpenCL", 2, "Discovering OpenCL devices for platform [{}]...",
               platform_index_);

  constexpr cl_device_type mask = CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_GPU;

  std::vector<cl::Device> natives;
  GGEMS_OCL_CHECK(platform_.getDevices(mask, &natives),
                  "No OpenCL devices detected on this platform.");

  devices_.clear();
  devices_.reserve(natives.size());

  for (std::size_t i = 0; i < natives.size(); ++i) {
    devices_.emplace_back(natives[i], platform_index_, i);
  }

  GGEMS_INFOEX("OpenCL", 2, "Found {} device(s) on platform [{}]",
               natives.size(), platform_index_);
}
} // namespace ggems::ocl
