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
  cl_int err = 0;
  std::string str = device_.getInfo<CL_DEVICE_NAME>(&err);
  GGOCL_CHECK(err);
  return str;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

std::string GGEMSOpenCLDevice::GetVendor() const {
  cl_int err = 0;
  std::string str = device_.getInfo<CL_DEVICE_VENDOR>(&err);
  GGOCL_CHECK(err);
  return str;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

std::string GGEMSOpenCLDevice::GetVersion() const {
  cl_int err = 0;
  std::string str = device_.getInfo<CL_DEVICE_VERSION>(&err);
  GGOCL_CHECK(err);
  return str;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

cl_uint GGEMSOpenCLDevice::GetVendorId() const {
  cl_int err = 0;
  cl_uint v = device_.getInfo<CL_DEVICE_VENDOR_ID>(&err);
  GGOCL_CHECK(err);
  return v;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

cl_device_type GGEMSOpenCLDevice::GetType() const {
  cl_int err = 0;
  cl_device_type v = device_.getInfo<CL_DEVICE_TYPE>(&err);
  GGOCL_CHECK(err);
  return v;
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

constexpr std::string GGEMSOpenCLDevice::DeviceTypeToString(cl_device_type deviceType) const {
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
