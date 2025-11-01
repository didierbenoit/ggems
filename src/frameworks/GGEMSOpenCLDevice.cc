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
#include <CL/cl.h>
#include <sstream>
/// \endcond

#include "GGEMS/tools/GGEMSLogger.hh"
#include "GGEMS/frameworks/GGEMSOpenCLDevice.hh"

using ggocl::utils::Get;
using ggocl::utils::GetArray;
//using ggocl::utils::ExtractExtensions;
//using ggocl::utils::HasExtension;
using ggocl::utils::ClVersionToString;

GGEMSOpenCLDevice::GGEMSOpenCLDevice(cl::Device const& device, std::size_t platform_index, std::size_t device_index)
: device_{device}, platform_index_{platform_index}, device_index_{device_index} {
}

std::string GGEMSOpenCLDevice::GetName() const {
  return Get<std::string>(device_, CL_DEVICE_NAME);
}

std::string GGEMSOpenCLDevice::GetVendor() const {
  return Get<std::string>(device_, CL_DEVICE_VENDOR);
}

std::string GGEMSOpenCLDevice::GetVersion() const {
  return Get<std::string>(device_, CL_DEVICE_VERSION);
}

std::string GGEMSOpenCLDevice::GetDriverVersion() const {
  return Get<std::string>(device_, CL_DRIVER_VERSION);
}

std::string GGEMSOpenCLDevice::GetProfile() const {
  return Get<std::string>(device_, CL_DEVICE_PROFILE);
}

std::string GGEMSOpenCLDevice::GetOpenCLCVersion() const {
  return Get<std::string>(device_, CL_DEVICE_OPENCL_C_VERSION);
}

cl_version GGEMSOpenCLDevice::GetNumericVersion() const {
  return Get<cl_version>(device_, CL_DEVICE_NUMERIC_VERSION);
}

cl_uint GGEMSOpenCLDevice::GetVendorId() const {
  return Get<cl_uint>(device_, CL_DEVICE_VENDOR_ID);
}

cl_device_type GGEMSOpenCLDevice::GetType() const {
  return Get<cl_device_type>(device_, CL_DEVICE_TYPE);
}

cl_uint GGEMSOpenCLDevice::GetMaxComputeUnits() const {
  return Get<cl_uint>(device_, CL_DEVICE_MAX_COMPUTE_UNITS);
}

cl_uint GGEMSOpenCLDevice::GetMaxClockFrequency() const {
  return Get<cl_uint>(device_, CL_DEVICE_MAX_CLOCK_FREQUENCY);
}

std::size_t GGEMSOpenCLDevice::GetMaxWorkGroupSize() const {
  return Get<std::size_t>(device_, CL_DEVICE_MAX_WORK_GROUP_SIZE);
}

cl_uint GGEMSOpenCLDevice::GetMaxWorkItemDimensions() const {
  return Get<cl_uint>(device_, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS);
}

std::vector<std::size_t> GGEMSOpenCLDevice::GetMaxWorkItemSizes() const {
  return GetArray<std::size_t>(device_, CL_DEVICE_MAX_WORK_ITEM_SIZES);
}

std::size_t GGEMSOpenCLDevice::GetPreferredWorkGroupSizeMultiple() const {
  return Get<std::size_t>(device_, CL_DEVICE_PREFERRED_WORK_GROUP_SIZE_MULTIPLE);
}

cl_uint GGEMSOpenCLDevice::GetPreferredVectorWidthChar() const {
  return Get<cl_uint>(device_, CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR);
}

cl_uint GGEMSOpenCLDevice::GetPreferredVectorWidthShort() const {
  return Get<cl_uint>(device_, CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT);
}

cl_uint GGEMSOpenCLDevice::GetPreferredVectorWidthInt() const {
  return Get<cl_uint>(device_, CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT);
}

cl_uint GGEMSOpenCLDevice::GetPreferredVectorWidthLong() const {
  return Get<cl_uint>(device_, CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG);
}

cl_uint GGEMSOpenCLDevice::GetPreferredVectorWidthFloat() const {
  return Get<cl_uint>(device_, CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT);
}

cl_uint GGEMSOpenCLDevice::GetPreferredVectorWidthDouble() const {
  return Get<cl_uint>(device_, CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE);
}

cl_uint GGEMSOpenCLDevice::GetPreferredVectorWidthHalf() const {
  return Get<cl_uint>(device_, CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF);
}

cl_uint GGEMSOpenCLDevice::GetNativeVectorWidthChar() const {
  return Get<cl_uint>(device_, CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR);
}

cl_uint GGEMSOpenCLDevice::GetNativeVectorWidthShort() const {
  return Get<cl_uint>(device_, CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT);
}

cl_uint GGEMSOpenCLDevice::GetNativeVectorWidthInt() const {
  return Get<cl_uint>(device_, CL_DEVICE_NATIVE_VECTOR_WIDTH_INT);
}

cl_uint GGEMSOpenCLDevice::GetNativeVectorWidthLong() const {
  return Get<cl_uint>(device_, CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG);
}

cl_uint GGEMSOpenCLDevice::GetNativeVectorWidthFloat() const {
  return Get<cl_uint>(device_, CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT);
}

cl_uint GGEMSOpenCLDevice::GetNativeVectorWidthDouble() const {
  return Get<cl_uint>(device_, CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE);
}

cl_uint GGEMSOpenCLDevice::GetNativeVectorWidthHalf() const {
  return Get<cl_uint>(device_, CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF);
}

std::size_t GGEMSOpenCLDevice::GetImage2DMaxWidth() const {
  return Get<std::size_t>(device_, CL_DEVICE_IMAGE2D_MAX_WIDTH);
}

std::size_t GGEMSOpenCLDevice::GetImage2DMaxHeight() const {
  return Get<std::size_t>(device_, CL_DEVICE_IMAGE2D_MAX_HEIGHT);
}

std::size_t GGEMSOpenCLDevice::GetImage3DMaxWidth() const {
  return Get<std::size_t>(device_, CL_DEVICE_IMAGE3D_MAX_WIDTH);
}

std::size_t GGEMSOpenCLDevice::GetImage3DMaxHeight() const {
  return Get<std::size_t>(device_, CL_DEVICE_IMAGE3D_MAX_HEIGHT);
}

std::size_t GGEMSOpenCLDevice::GetImage3DMaxDepth() const {
  return Get<std::size_t>(device_, CL_DEVICE_IMAGE3D_MAX_DEPTH);
}

std::size_t GGEMSOpenCLDevice::GetImageMaxBufferSize() const {
  return Get<std::size_t>(device_, CL_DEVICE_IMAGE_MAX_BUFFER_SIZE);
}

std::size_t GGEMSOpenCLDevice::GetImageMaxArraySize() const {
  return Get<std::size_t>(device_, CL_DEVICE_IMAGE_MAX_ARRAY_SIZE);
}

cl_bool GGEMSOpenCLDevice::GetImageSupport() const {
  return Get<cl_bool>(device_, CL_DEVICE_IMAGE_SUPPORT);
}

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

std::string GGEMSOpenCLDevice::VendorIdToString(cl_uint vendor_id) const {
  switch (vendor_id) {
    case 0x8086: return "Intel Corporation";
    case 0x10DE: return "NVIDIA Corporation";
    case 0x1002: return "AMD / ATI";
    case 0x13B5: return "ARM";
    case 0x5143: return "Qualcomm";
    case 0x106B: return "Apple";
    case 0x1010: return "Imagination Technologies";
    default: {
      std::ostringstream oss;
      oss << "Unknown (0x" << std::hex << vendor_id << ")" << std::dec;
      return oss.str();
    }
  }
}

std::string GGEMSOpenCLDevice::ClBoolToString(cl_bool flag) const {
  return (flag == CL_TRUE) ? "CL_TRUE" : "CL_FALSE";
}

void GGEMSOpenCLDevice::PrintIdentity() const {
  gglog::info("GGEMSOpenCLDevice", "PrintIdentity") << "-> Name: "
    << GetName() << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintIdentity") << "-> Vendor: "
    << GetVendor() << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintIdentity") << "-> Version: "
    << GetVersion() << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintIdentity") << "-> Driver Version: "
    << GetDriverVersion() << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintIdentity") << "-> Profile: "
    << GetProfile() << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintIdentity") << "-> OpenCL C Version: "
    << GetOpenCLCVersion() << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintIdentity") << "-> Numeric Version: "
    << ClVersionToString(GetNumericVersion()) << gglog::endl;
}

void GGEMSOpenCLDevice::PrintTypeID() const {
  gglog::info("GGEMSOpenCLDevice", "PrintTypeID") << "-> Type: "
    << DeviceTypeToString(GetType()) << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintTypeID") << "-> Vendor ID: "
    << VendorIdToString(GetVendorId()) << gglog::endl;
}

void GGEMSOpenCLDevice::PrintCompute() const {
   gglog::info("GGEMSOpenCLDevice", "PrintCompute") << "-> Max Compute Units: "
    << GetMaxComputeUnits() << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintCompute") << "-> Max Clock Frequency: "
    << GetMaxClockFrequency() << " MHz" << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintCompute") << "-> Max Workgroup Size: "
    << GetMaxWorkGroupSize() << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintCompute") << "-> Max Workitem Dimensions: "
    << GetMaxWorkItemDimensions() << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintCompute") << "-> Max Workitem Sizes: "
    << "[" << GetMaxWorkItemSizes()[0]
    << "," << GetMaxWorkItemSizes()[1]
    << "," << GetMaxWorkItemSizes()[2] << "]" << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintCompute") << "-> Preferred Workgroup Size Multiple: "
    << GetPreferredWorkGroupSizeMultiple() << gglog::endl;
}

void GGEMSOpenCLDevice::PrintVectorisation() const {
  gglog::info("GGEMSOpenCLDevice", "PrintVectorisation") << "-> Preferred Vector Width Char: "
    << GetPreferredVectorWidthChar() << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintVectorisation") << "-> Preferred Vector Width Short: "
    << GetPreferredVectorWidthShort() << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintVectorisation") << "-> Preferred Vector Width Int: "
    << GetPreferredVectorWidthInt() << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintVectorisation") << "-> Preferred Vector Width Long: "
    << GetPreferredVectorWidthLong() << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintVectorisation") << "-> Preferred Vector Width Float: "
    << GetPreferredVectorWidthFloat() << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintVectorisation") << "-> Preferred Vector Width Double: "
    << GetPreferredVectorWidthDouble() << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintVectorisation") << "-> Preferred Vector Width Half: "
    << GetPreferredVectorWidthHalf() << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintVectorisation") << "-> Native Vector Width Char: "
    << GetNativeVectorWidthChar() << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintVectorisation") << "-> Native Vector Width Short: "
    << GetNativeVectorWidthShort() << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintVectorisation") << "-> Native Vector Width Int: "
    << GetNativeVectorWidthInt() << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintVectorisation") << "-> Native Vector Width Long: "
    << GetNativeVectorWidthLong() << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintVectorisation") << "-> Native Vector Width Float: "
    << GetNativeVectorWidthFloat() << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintVectorisation") << "-> Native Vector Width Double: "
    << GetNativeVectorWidthDouble() << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintVectorisation") << "-> Native Vector Width Half: "
    << GetNativeVectorWidthHalf() << gglog::endl;
}

void GGEMSOpenCLDevice::PrintImages() const {
   gglog::info("GGEMSOpenCLDevice", "PrintImages") << "-> Image Support: "
    << ClBoolToString(GetImageSupport()) << gglog::endl;

   gglog::info("GGEMSOpenCLDevice", "PrintImages") << "-> Image2D Max Width: "
    << GetImage2DMaxWidth() << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintImages") << "-> Image2D Max Height: "
    << GetImage2DMaxHeight() << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintImages") << "-> Image3D Max Width: "
    << GetImage3DMaxWidth() << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintImages") << "-> Image3D Max Height: "
    << GetImage3DMaxHeight() << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintImages") << "-> Image3D Max Depth: "
    << GetImage3DMaxDepth() << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintImages") << "-> Image Max Buffer Size: "
    << GetImageMaxBufferSize() << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintImages") << "-> Image Max Array Size: "
    << GetImageMaxArraySize() << gglog::endl;
}

void GGEMSOpenCLDevice::Print() const {
  gglog::info("GGEMSOpenCLDevice", "Print") << "----- Device [" 
    << platform_index_ << ":" << device_index_ << "] -----" << gglog::endl;

  PrintIdentity();
  PrintTypeID();
  PrintCompute();
  PrintVectorisation();
  PrintImages();
}
