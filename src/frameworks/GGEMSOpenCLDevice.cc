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
 * \file GGEMSOpenCLDevice.cc
 * \brief 
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-10-29
 * \copyright GNU General Public License v3.0
 * \version 2.0
 */

/// \cond
#include <sstream>
/// \endcond

#include "GGEMS/tools/GGEMSLogger.hh"
#include "GGEMS/frameworks/GGEMSOpenCLDevice.hh"

using ggocl::utils::Get;
using ggocl::utils::GetArray;
using ggocl::utils::ExtractExtensions;
using ggocl::utils::HasExtension;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

GGEMSOpenCLDevice::GGEMSOpenCLDevice(cl::Device const& device, std::size_t platform_index, std::size_t device_index)
: device_{device}, platform_index_{platform_index}, device_index_{device_index} {
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

std::string GGEMSOpenCLDevice::GetName() const {
  return Get<std::string>(device_, CL_DEVICE_NAME);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

std::string GGEMSOpenCLDevice::GetVendor() const {
  return Get<std::string>(device_, CL_DEVICE_VENDOR);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

std::string GGEMSOpenCLDevice::GetVersion() const {
  return Get<std::string>(device_, CL_DEVICE_VERSION);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

cl_uint GGEMSOpenCLDevice::GetVendorId() const {
  return Get<cl_uint>(device_, CL_DEVICE_VENDOR_ID);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

cl_device_type GGEMSOpenCLDevice::GetType() const {
  return Get<cl_device_type>(device_, CL_DEVICE_TYPE);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

std::string GGEMSOpenCLDevice::DeviceTypeToString(cl_device_type deviceType) const {
  std::ostringstream oss;
  bool first = true;

  auto const append = [&](std::string const& name) {
    if (!first) {
      oss << " | ";
    }
    oss << name;
    first = false;
  };

  if (deviceType & CL_DEVICE_TYPE_CPU) append("CPU");
  if (deviceType & CL_DEVICE_TYPE_GPU) append("GPU");
  if (deviceType & CL_DEVICE_TYPE_ACCELERATOR) append("Accelerator");
  if (deviceType & CL_DEVICE_TYPE_CUSTOM) append("Custom");
  if (deviceType == CL_DEVICE_TYPE_DEFAULT) append("Default");
  if (deviceType == CL_DEVICE_TYPE_ALL) append("All");

  if (first) oss << "Unknown(" << deviceType << ')';

  return oss.str();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSOpenCLDevice::Print() const {
  gglog::info("GGEMSOpenCLDevice", "Print") << "----- Device [" << platform_index_ << ":" << device_index_ << "] -----" << gglog::endl;
  gglog::info("GGEMSOpenCLDevice", "Print") << "-> Name: " << GetName() << gglog::endl;
  gglog::info("GGEMSOpenCLDevice", "Print") << "-> Vendor: " << GetVendor() << " (Id " << GetVendorId() << ")" << gglog::endl;
  gglog::info("GGEMSOpenCLDevice", "Print") << "-> Version: " << GetVersion() << gglog::endl;
  gglog::info("GGEMSOpenCLDevice", "Print") << "-> Type: " << DeviceTypeToString(GetType()) << gglog::endl;

}
