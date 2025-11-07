// ************************************************************************
// * This file is part of GGEMS.                                          *
// *                                                                      *
// * GGEMS is free software: you can redistribute it and/or modify        *
// * it under the terms of the GNU General Public License as published by *
// * the Free Software Foundation, either version 3 of the License, or    *
// * (at your option) any later version.                                  *
// *                                                                      *
// * GGEMS is distributed in the hope that it will be useful,             *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
// * GNU General Public License for more details.                         *
// *                                                                      *
// * You should have received a copy of the GNU General Public License    *
// * along with GGEMS.  If not, see <https://www.gnu.org/licenses/>.      *
// *                                                                      *
// ************************************************************************

/*!
 * \file GGEMSOpenCLPlatform.cc
 * \brief Definition of GGEMSOpenCLPlatform methods (OpenCL 3.0 clean variant).
 */

/// \cond
#include <sstream>
/// \endcond

#include "GGEMS/frameworks/GGEMSOpenCLPlatform.hh"
#include "GGEMS/frameworks/GGEMSOpenCLDevice.hh"
#include "GGEMS/core/GGEMSLogger.hh"
#include "GGEMS/core/GGEMSMacros.hh"

using ggocl::utils::Get;
using ggocl::utils::GetArray;
using ggocl::utils::ExtractExtensions;
using ggocl::utils::HasExtension;
using ggocl::utils::ClVersionToString;
using ggocl::utils::ClNameVersionToString;

GGEMSOpenCLPlatform::GGEMSOpenCLPlatform(cl::Platform const& platform, std::size_t platform_index)
  : platform_{platform}
  , platform_index_{platform_index}
{
  //GGEMS_INFOEX("OpenCL", 3, )
//  gglog::info3("GGEMSOpenCLPlatform", "GGEMSOpenCLPlatform")
  //  << "Allocating GGEMSOpenCLPlatform [" << platform_index_ << "] ..." << gglog::endl;

  extensions_ = ExtractExtensions(platform_, CL_PLATFORM_EXTENSIONS);

  // Discover CPU/GPU devices now; contexts/queues are created later on demand.
  DiscoverDevices();

 // gglog::info2("GGEMSOpenCLPlatform", "GGEMSOpenCLPlatform")
 //   << "GGEMSOpenCLPlatform allocated with " << devices_.size() << " device(s)" << gglog::endl;
}

GGEMSOpenCLPlatform::~GGEMSOpenCLPlatform() {
//  gglog::info3("GGEMSOpenCLPlatform","~GGEMSOpenCLPlatform")
//    << "Releasing GGEMSOpenCLPlatform [" << platform_index_ << "]" << gglog::endl;
}

// -------------------- Queries --------------------

bool GGEMSOpenCLPlatform::CheckExtension(std::string_view name) const {
  return HasExtension(extensions_, name);
}

std::string GGEMSOpenCLPlatform::GetName() const {
  return Get<std::string>(platform_, CL_PLATFORM_NAME);
}

std::string GGEMSOpenCLPlatform::GetProfile() const {
  return Get<std::string>(platform_, CL_PLATFORM_PROFILE);
}

std::string GGEMSOpenCLPlatform::GetVersion() const {
  return Get<std::string>(platform_, CL_PLATFORM_VERSION);
}

std::string GGEMSOpenCLPlatform::GetVendor() const {
  return Get<std::string>(platform_, CL_PLATFORM_VENDOR);
}

std::string GGEMSOpenCLPlatform::GetExtensions() const {
  return Get<std::string>(platform_, CL_PLATFORM_EXTENSIONS);
}

cl_version GGEMSOpenCLPlatform::GetNumericVersion() const {
  return Get<cl_version>(platform_, CL_PLATFORM_NUMERIC_VERSION);
}

cl_ulong GGEMSOpenCLPlatform::GetHostTimerResolution() const {
  return Get<cl_ulong>(platform_, CL_PLATFORM_HOST_TIMER_RESOLUTION);
}

std::vector<cl_name_version> GGEMSOpenCLPlatform::GetExtensionsWithVersion() const {
  return GetArray<cl_name_version>(platform_, CL_PLATFORM_EXTENSIONS_WITH_VERSION);
}

// -------------------- Reporting --------------------
void GGEMSOpenCLPlatform::PrintIdentity() const {
/*   gglog::info("GGEMSOpenCLPlatform", "PrintIdentity")
    << "-> Profile: " << GetProfile() << gglog::endl;

  gglog::info("GGEMSOpenCLPlatform", "PrintIdentity")
    << "-> Version: " << GetVersion() << gglog::endl;

  gglog::info("GGEMSOpenCLPlatform", "PrintIdentity") << "-> Numeric Version: "
    << ClVersionToString(GetNumericVersion()) << gglog::endl;

  gglog::info("GGEMSOpenCLPlatform", "PrintIdentity")
    << "-> Discovered devices: " << devices_.size() << gglog::endl;

  gglog::info("GGEMSOpenCLPlatform", "PrintIdentity")
    << "-> Host Timer Resolution: " << GetHostTimerResolution() << " ns" << gglog::endl;*/
}

void GGEMSOpenCLPlatform::PrintExtension() const {
//  gglog::info("GGEMSOpenCLPlatform", "PrintExtensions")
//    << "-> Extensions with version: " << ClNameVersionToString(GetExtensionsWithVersion()) << gglog::endl;
}

void GGEMSOpenCLPlatform::Print() const {
/*  gglog::info("GGEMSOpenCLPlatform", "Print")
    << "+++++++++++++++++++++++++++++++++++++" << gglog::endl;

  gglog::info("GGEMSOpenCLPlatform", "Print")
    << "Platform [" << platform_index_ << "]: " << GetName()
    << " (" << GetVendor() << ")" << gglog::endl;

  PrintIdentity();
  PrintExtension();

  gglog::info("GGEMSOpenCLPlatform", "Print")
    << "+++++++++++++++++++++++++++++++++++++" << gglog::endl;*/
}

// -------------------- Devices & cleanup --------------------

std::vector<GGEMSOpenCLDevice const*> GGEMSOpenCLPlatform::GetDevices() const {
  std::vector<GGEMSOpenCLDevice const*> out;
  out.reserve(devices_.size());
  for (auto const& d : devices_)
    out.push_back(d.get());
  return out;
}

void GGEMSOpenCLPlatform::Clean() {
/*  gglog::info3("GGEMSOpenCLPlatform", "Clean")
    << "Cleaning Platform " << GetName() << " resources..." << gglog::endl;*/

  platform_.unloadCompiler();
  devices_.clear();
  extensions_.clear();

/*  gglog::info3("GGEMSOpenCLPlatform", "Clean")
    << "Platform resources cleaned." << gglog::endl;*/
}

// -------------------- Internal discovery --------------------

void GGEMSOpenCLPlatform::DiscoverDevices() {
/*  gglog::info3("GGEMSOpenCLPlatform", "DiscoverDevices")
    << "Discovering OpenCL devices for platform [" << platform_index_ << "] ..."
    << gglog::endl;*/

  constexpr cl_device_type mask = CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_GPU;

  std::vector<cl::Device> natives;
 // GGOCL_CHECK(platform_.getDevices(mask, &natives));

  devices_.clear();
  devices_.reserve(natives.size());

  for (std::size_t i = 0; i < natives.size(); ++i) {
    devices_.emplace_back(std::make_unique<GGEMSOpenCLDevice>(natives[i], platform_index_, i));
  }

/*  gglog::info2("GGEMSOpenCLPlatform", "DiscoverDevices")
    << "Found " << natives.size() << " device(s) on platform [" << platform_index_ << "]"
    << gglog::endl;*/
}
