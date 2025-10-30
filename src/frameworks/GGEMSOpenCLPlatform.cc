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
 * \brief Declaration of the GGEMSOpenCLPlatform class for OpenCL platform abstraction.
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-10-14
 * \copyright GNU General Public License v3.0
 * \version 2.0
 */

/// \cond
#include <sstream>
/// \endcond

#include "GGEMS/tools/GGEMSLogger.hh"
#include "GGEMS/frameworks/GGEMSOpenCLPlatform.hh"
#include "GGEMS/frameworks/GGEMSOpenCLDevice.hh"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

GGEMSOpenCLPlatform::GGEMSOpenCLPlatform(cl::Platform const& platform, std::size_t platform_index)
: platform_{platform}, platform_index_{platform_index} {
  gglog::info2("GGEMSOpenCLPlatform", "GGEMSOpenCLPlatform") << "Allocating GGEMSOpenCLPlatform ["
    << platform_index_ << "] ..." << gglog::endl;

  DiscoverDevices();

  gglog::info2("GGEMSOpenCLPlatform", "GGEMSOpenCLPlatform") << "GGEMSOpenCLPlatform allocated with "
    << devices_.size() << " device(s)" << gglog::endl;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

GGEMSOpenCLPlatform::~GGEMSOpenCLPlatform() {
  gglog::info3("GGEMSOpenCLPlatform","~GGEMSOpenCLPlatform") << "Deleting GGEMSOpenCLPlatform ["
    << platform_index_ << "] ..." << gglog::endl;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSOpenCLPlatform::DiscoverDevices() {
  gglog::info2("GGEMSOpenCLPlatform", "DiscoverDevices") << "Discovering OpenCL device(s) for GGEMSOpenCLPlatform ["
    << platform_index_ << "] ..." << gglog::endl;

 // Only CPU and GPU
  constexpr cl_device_type mask = CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_GPU;

  std::vector<cl::Device> devices;
  GGOCL_CHECK(platform_.getDevices(mask, &devices));

  devices_.clear();
  devices_.reserve(devices.size());
  std::size_t dev_index{0};
  for (auto const& d : devices) {
    devices_.emplace_back(std::make_unique<GGEMSOpenCLDevice>(d, platform_index_, dev_index++));
  }

  gglog::info2("GGEMSOpenCLPlatform", "DiscoverDevices") << "GGEMSOpenCLPlatform [" << platform_index_ << "] found "
    << devices.size() << " device(s)" << gglog::endl;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

bool GGEMSOpenCLPlatform::CheckExtension(std::string_view extension_name) const {
  return (GetExtensions().find(extension_name) != std::string::npos);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

std::string GGEMSOpenCLPlatform::GetName() const {
  cl_int err = 0;
  std::string str = platform_.getInfo<CL_PLATFORM_NAME>(&err);
  GGOCL_CHECK(err);
  return str;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

std::string GGEMSOpenCLPlatform::GetProfile() const {
  cl_int err = 0;
  std::string str = platform_.getInfo<CL_PLATFORM_PROFILE>(&err);
  GGOCL_CHECK(err);
  return str;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

std::string GGEMSOpenCLPlatform::GetVersion() const {
  cl_int err = 0;
  std::string str = platform_.getInfo<CL_PLATFORM_VERSION>(&err);
  GGOCL_CHECK(err);
  return str;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

std::string GGEMSOpenCLPlatform::GetVendor() const {
  cl_int err = 0;
  std::string str = platform_.getInfo<CL_PLATFORM_VENDOR>(&err);
  GGOCL_CHECK(err);
  return str;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

std::string GGEMSOpenCLPlatform::GetExtensions() const {
  cl_int err = 0;
  std::string str = platform_.getInfo<CL_PLATFORM_EXTENSIONS>(&err);
  GGOCL_CHECK(err);
  return str;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

cl_version GGEMSOpenCLPlatform::GetNumericVersion() const {
  cl_int err = 0;
  cl_version version = platform_.getInfo<CL_PLATFORM_NUMERIC_VERSION>(&err);
  GGOCL_CHECK(err);
  return version;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

cl_ulong GGEMSOpenCLPlatform::GetHostTimerResolution() const {
  cl_int err = 0;
  cl_ulong timer = platform_.getInfo<CL_PLATFORM_HOST_TIMER_RESOLUTION>(&err);
  GGOCL_CHECK(err);
  return timer;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

std::vector<cl_name_version> GGEMSOpenCLPlatform::GetExtensionsWithVersion() const {
  cl_int err = 0;
  std::vector<cl_name_version> extensions = platform_.getInfo<CL_PLATFORM_EXTENSIONS_WITH_VERSION>(&err);
  GGOCL_CHECK(err);
  return extensions;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

std::string GGEMSOpenCLPlatform::GetIcdSuffixKhr() const {
  cl_int err = 0;
  std::string str = platform_.getInfo<CL_PLATFORM_ICD_SUFFIX_KHR>(&err);
  GGOCL_CHECK(err);
  return str;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSOpenCLPlatform::Print() const {
  gglog::info("GGEMSOpenCLPlatform", "Print") << "+++++++++++++++++++++++++++++++++++++" << gglog::endl;
  gglog::info("GGEMSOpenCLPlatform", "Print") << "-> Name: " << GetName() << gglog::endl;
  gglog::info("GGEMSOpenCLPlatform", "Print") << "-> Vendor: " << GetVendor() << gglog::endl;
  gglog::info("GGEMSOpenCLPlatform", "Print") << "-> Profile: " << GetProfile() << gglog::endl;
  gglog::info("GGEMSOpenCLPlatform", "Print") << "-> Version: " << GetVersion() << gglog::endl;

  // Decode numeric version: major(10 bits), minor(10 bits), patch(12 bits).
  cl_version numeric_version = GetNumericVersion();
  cl_uint major = (numeric_version >> 22) & 0x3FFu;
  cl_uint minor = (numeric_version >> 12) & 0x3FFu;
  cl_uint patch = (numeric_version >>  0) & 0xFFFu;

  {
    std::ostringstream oss;
    oss << "-> Numeric Version: 0x" << std::hex << std::uppercase << numeric_version
      << std::dec << " (" << major << "." << minor << "." << patch << ")";
    gglog::info("GGEMSOpenCLPlatform", "Print") << oss.str() << gglog::endl;
  }

  gglog::info("GGEMSOpenCLPlatform", "Print") << "-> Host Timer Resolution: " << GetHostTimerResolution()
    << " ns" << gglog::endl;

  {
    std::ostringstream oss;
    oss << "-> Extensions with version: ";
    std::vector<cl_name_version> extensions = GetExtensionsWithVersion();
    for (auto const& extension : extensions) {
      cl_uint eMajor = (extension.version >> 22) & 0x3FFu;
      cl_uint eMinor = (extension.version >> 12) & 0x3FFu;
      cl_uint ePatch = (extension.version >>  0) & 0xFFFu;
      oss << extension.name << " (v" << eMajor << "." << eMinor << "." << ePatch << ") ";
    }
    gglog::info("GGEMSOpenCLPlatform", "Print") << oss.str() << gglog::endl;
  }

  gglog::info("GGEMSOpenCLPlatform", "Print") << "-> ICD Suffix: " << GetIcdSuffixKhr() << gglog::endl;

  // External memory import handle types (if extension present)
  if (CheckExtension("cl_khr_external_memory")) {
    auto const handles = GetPlatformInfoArray<cl_external_memory_handle_type_khr>(CL_PLATFORM_EXTERNAL_MEMORY_IMPORT_HANDLE_TYPES_KHR);

    auto to_string = [](cl_external_memory_handle_type_khr h) -> std::string_view {
      switch (h) {
        case CL_EXTERNAL_MEMORY_HANDLE_OPAQUE_FD_KHR:         return "CL_EXTERNAL_MEMORY_HANDLE_OPAQUE_FD_KHR";
        case CL_EXTERNAL_MEMORY_HANDLE_OPAQUE_WIN32_KHR:      return "CL_EXTERNAL_MEMORY_HANDLE_OPAQUE_WIN32_KHR";
        case CL_EXTERNAL_MEMORY_HANDLE_OPAQUE_WIN32_KMT_KHR:  return "CL_EXTERNAL_MEMORY_HANDLE_OPAQUE_WIN32_KMT_KHR";
        case CL_EXTERNAL_MEMORY_HANDLE_D3D11_TEXTURE_KHR:     return "CL_EXTERNAL_MEMORY_HANDLE_D3D11_TEXTURE_KHR";
        case CL_EXTERNAL_MEMORY_HANDLE_D3D11_TEXTURE_KMT_KHR: return "CL_EXTERNAL_MEMORY_HANDLE_D3D11_TEXTURE_KMT_KHR";
        case CL_EXTERNAL_MEMORY_HANDLE_D3D12_HEAP_KHR:        return "CL_EXTERNAL_MEMORY_HANDLE_D3D12_HEAP_KHR";
        case CL_EXTERNAL_MEMORY_HANDLE_D3D12_RESOURCE_KHR:    return "CL_EXTERNAL_MEMORY_HANDLE_D3D12_RESOURCE_KHR";
        case CL_EXTERNAL_MEMORY_HANDLE_DMA_BUF_KHR:           return "CL_EXTERNAL_MEMORY_HANDLE_DMA_BUF_KHR";
        case CL_EXTERNAL_MEMORY_HANDLE_OPAQUE_WIN32_NAME_KHR: return "CL_EXTERNAL_MEMORY_HANDLE_OPAQUE_WIN32_NAME_KHR";
        default:                                              return "UNKNOWN_EXTERNAL_MEMORY_HANDLE";
      }
    };

    std::ostringstream oss;
    oss << "-> External memory import handle types: ";
    for (auto const& h : handles) {
      oss << to_string(h) << ' ';
    }
    gglog::info("GGEMSOpenCLPlatform", "Print") << oss.str() << gglog::endl;
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

std::vector<GGEMSOpenCLDevice const*> GGEMSOpenCLPlatform::GetDevices() const {
  std::vector<GGEMSOpenCLDevice const*> result;
  result.reserve(devices_.size());
  for (auto const& d : devices_) {
    result.push_back(d.get());
  }
  return result;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSOpenCLPlatform::Clean() {
  gglog::info2("GGEMSOpenCLPlatform", "Clean") << "Cleaning Platform "
    << GetName() << " resources..." << gglog::endl;

  platform_.unloadCompiler();
  devices_.clear();

  gglog::info2("GGEMSOpenCLPlatform", "Clean")
    << "Platform resources cleaned." << gglog::endl;
}
