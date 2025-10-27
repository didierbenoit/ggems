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
 * \brief Definition of GGEMSOpenCL class
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

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

bool GGEMSOpenCLPlatform::CheckExtension(std::string_view extension_name) const {
  return ((GetExtensions().find(extension_name) != std::string::npos) ? true : false);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

std::string GGEMSOpenCLPlatform::GetName() const {
  cl_int err = 0;
  std::string str = platform_.getInfo<CL_PLATFORM_NAME>(&err);
  GGOCL_ERROR(err);
  return str;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

std::string GGEMSOpenCLPlatform::GetProfile() const {
  cl_int err = 0;
  std::string str = platform_.getInfo<CL_PLATFORM_PROFILE>(&err);
  GGOCL_ERROR(err);
  return str;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

std::string GGEMSOpenCLPlatform::GetVersion() const {
  cl_int err = 0;
  std::string str = platform_.getInfo<CL_PLATFORM_VERSION>(&err);
  GGOCL_ERROR(err);
  return str;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

std::string GGEMSOpenCLPlatform::GetVendor() const {
  cl_int err = 0;
  std::string str = platform_.getInfo<CL_PLATFORM_VENDOR>(&err);
  GGOCL_ERROR(err);
  return str;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

std::string GGEMSOpenCLPlatform::GetExtensions() const {
  cl_int err = 0;
  std::string str = platform_.getInfo<CL_PLATFORM_EXTENSIONS>(&err);
  GGOCL_ERROR(err);
  return str;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

cl_version GGEMSOpenCLPlatform::GetNumericVersion() const {
  cl_int err = 0;
  cl_version version = platform_.getInfo<CL_PLATFORM_NUMERIC_VERSION>(&err);
  GGOCL_ERROR(err);
  return version;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

cl_ulong GGEMSOpenCLPlatform::GetHostTimerResolution() const {
  cl_int err = 0;
  cl_ulong timer = platform_.getInfo<CL_PLATFORM_HOST_TIMER_RESOLUTION>(&err);
  GGOCL_ERROR(err);
  return timer;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

std::vector<cl_name_version> GGEMSOpenCLPlatform::GetExtensionsWithVersion() const {
  cl_int err = 0;
  std::vector<cl_name_version> extensions = platform_.getInfo<CL_PLATFORM_EXTENSIONS_WITH_VERSION>(&err);
  GGOCL_ERROR(err);
  return extensions;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

std::string GGEMSOpenCLPlatform::GetIcdSuffixKhr() const {
  cl_int err = 0;
  std::string str = platform_.getInfo<CL_PLATFORM_ICD_SUFFIX_KHR>(&err);
  GGOCL_ERROR(err);
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

  cl_version numeric_version = GetNumericVersion();
  cl_uint major = (numeric_version >> 22) & 0x3FF;
  cl_uint minor = (numeric_version >> 12) & 0x3FF;
  cl_uint patch = numeric_version & 0x3FF;
  gglog::info("GGEMSOpenCLPlatform", "Print") << "-> Numeric Version: " << std::hex << "0x"
    << numeric_version << std::dec << " (" << major << "." << minor << "." << patch
    << ")" << gglog::endl;

  gglog::info("GGEMSOpenCLPlatform", "Print") << "-> Host Timer Resolution: " << GetHostTimerResolution()
    << " ns" << gglog::endl;

  gglog::info("GGEMSOpenCLPlatform", "Print") << "-> Extensions with version: " << gglog::endl;
  std::ostringstream oss(std::ostringstream::out);
  std::vector<cl_name_version> extensions = GetExtensionsWithVersion();
  for (auto const& extension : extensions) {
    major = (extension.version >> 22) & 0x3FF;
    minor = (extension.version >> 12) & 0x3FF;
    patch = extension.version & 0xFFF;
    oss << extension.name << " (v" << major << "." << minor << "." << patch << ") ";
   }
  gglog::info("GGEMSOpenCLPlatform", "Print") << oss.str() << gglog::endl;
  oss.str("");
  oss.clear();

  gglog::info("GGEMSOpenCLPlatform", "Print") << "-> ICD Suffix: " << GetIcdSuffixKhr() << gglog::endl;

  if (CheckExtension("cl_khr_external_memory")) {
    std::string str("");
    gglog::info("GGEMSOpenCLPlatform", "Print") << "-> External memory import handle types: ";
    for (auto const& ext_memory :
      GetPlatformInfoArray<cl_external_memory_handle_type_khr>(CL_PLATFORM_EXTERNAL_MEMORY_IMPORT_HANDLE_TYPES_KHR)) {
      str += ((ext_memory ^ CL_EXTERNAL_MEMORY_HANDLE_OPAQUE_FD_KHR) == 0) ?
        "CL_EXTERNAL_MEMORY_HANDLE_OPAQUE_FD_KHR " : "";
      str += ((ext_memory ^ CL_EXTERNAL_MEMORY_HANDLE_OPAQUE_WIN32_KHR) == 0) ?
        "CL_EXTERNAL_MEMORY_HANDLE_OPAQUE_WIN32_KHR " : "";
      str += ((ext_memory ^ CL_EXTERNAL_MEMORY_HANDLE_OPAQUE_WIN32_KMT_KHR) == 0) ?
        "CL_EXTERNAL_MEMORY_HANDLE_OPAQUE_WIN32_KMT_KHR " : "";
      str += ((ext_memory ^ CL_EXTERNAL_MEMORY_HANDLE_D3D11_TEXTURE_KHR) == 0) ?
        "CL_EXTERNAL_MEMORY_HANDLE_D3D11_TEXTURE_KHR " : "";
      str += ((ext_memory ^ CL_EXTERNAL_MEMORY_HANDLE_D3D11_TEXTURE_KMT_KHR) == 0) ?
        "CL_EXTERNAL_MEMORY_HANDLE_D3D11_TEXTURE_KMT_KHR " : "";
      str += ((ext_memory ^ CL_EXTERNAL_MEMORY_HANDLE_D3D12_HEAP_KHR) == 0) ?
        "CL_EXTERNAL_MEMORY_HANDLE_D3D12_HEAP_KHR " : "";
      str += ((ext_memory ^ CL_EXTERNAL_MEMORY_HANDLE_D3D12_RESOURCE_KHR) == 0) ?
        "CL_EXTERNAL_MEMORY_HANDLE_D3D12_RESOURCE_KHR " : "";
      str += ((ext_memory ^ CL_EXTERNAL_MEMORY_HANDLE_DMA_BUF_KHR) == 0) ?
        "CL_EXTERNAL_MEMORY_HANDLE_DMA_BUF_KHR " : "";
      str += ((ext_memory ^ CL_EXTERNAL_MEMORY_HANDLE_OPAQUE_WIN32_NAME_KHR) == 0) ?
        "CL_EXTERNAL_MEMORY_HANDLE_OPAQUE_WIN32_NAME_KHR " : "";
    }
    gglog::info("GGEMSOpenCLPlatform", "Print") << str << gglog::endl;
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSOpenCLPlatform::Clean() {
  gglog::info4("GGEMSOpenCLPlatform", "Clean") << "Cleaning Platform " << GetName() << " ressources..." << gglog::endl;

  platform_.unloadCompiler();

  gglog::info4("GGEMSOpenCLPlatform", "Clean") << "Platform ressources cleaned!!!" << gglog::endl;
}
