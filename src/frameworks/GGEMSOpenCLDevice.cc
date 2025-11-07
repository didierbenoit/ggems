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
#include <iomanip>
/// \endcond

#include "GGEMS/core/GGEMSLogger.hh"
#include "GGEMS/frameworks/GGEMSOpenCLDevice.hh"

using ggems::ocl::Get;
using ggems::ocl::GetArray;
using ggems::ocl::ExtractExtensions;
using ggems::ocl::HasExtension;
using ggems::ocl::ClVersionToString;
using ggems::ocl::ClNameVersionToString;

GGEMSOpenCLDevice::GGEMSOpenCLDevice(cl::Device const& device, std::size_t platform_index, std::size_t device_index)
: device_{device}, platform_index_{platform_index}, device_index_{device_index} {
  extensions_ = ExtractExtensions(device_, CL_DEVICE_EXTENSIONS);
}

bool GGEMSOpenCLDevice::CheckExtension(std::string_view name) const {
  return HasExtension(extensions_, name);
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

std::vector<cl_name_version> GGEMSOpenCLDevice::GetOpenCLCAllVersions() const {
  return GetArray<cl_name_version>(device_, CL_DEVICE_OPENCL_C_ALL_VERSIONS);
}

cl_version_khr GGEMSOpenCLDevice::GetOpenCLCNumericVersionKhr() const {
  return Get<cl_version_khr>(device_, CL_DEVICE_OPENCL_C_NUMERIC_VERSION_KHR);
}

std::vector<cl_name_version> GGEMSOpenCLDevice::GetOpenCLCFeatures() const {
  return GetArray<cl_name_version>(device_, CL_DEVICE_OPENCL_C_FEATURES);
}

cl_version GGEMSOpenCLDevice::GetCxxForOpenCLNumericVersionExt() const {
  return Get<cl_version>(device_, CL_DEVICE_CXX_FOR_OPENCL_NUMERIC_VERSION_EXT);
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

cl_uint GGEMSOpenCLDevice::GetMaxReadImageArgs() const {
  return Get<cl_uint>(device_, CL_DEVICE_MAX_READ_IMAGE_ARGS);
}

cl_uint GGEMSOpenCLDevice::GetMaxWriteImageArgs() const {
  return Get<cl_uint>(device_, CL_DEVICE_MAX_WRITE_IMAGE_ARGS);
}

cl_uint GGEMSOpenCLDevice::GetMaxReadWriteImageArgs() const {
  return Get<cl_uint>(device_, CL_DEVICE_MAX_READ_WRITE_IMAGE_ARGS);
}

cl_uint GGEMSOpenCLDevice::GetImagePitchAlignment() const {
  return Get<cl_uint>(device_, CL_DEVICE_IMAGE_PITCH_ALIGNMENT);
}

cl_uint GGEMSOpenCLDevice::GetImageBaseAddressAlignment() const {
  return Get<cl_uint>(device_, CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT);
}

std::size_t GGEMSOpenCLDevice::GetMaxBufferSize() const {
  return Get<std::size_t>(device_, CL_DEVICE_IMAGE_MAX_BUFFER_SIZE);
}

cl_uint GGEMSOpenCLDevice::GetMaxSamplers() const {
  return Get<cl_uint>(device_, CL_DEVICE_MAX_SAMPLERS);
}

cl_ulong GGEMSOpenCLDevice::GetGlobalMemSize() const {
  return Get<cl_ulong>(device_, CL_DEVICE_GLOBAL_MEM_SIZE);
}

cl_device_mem_cache_type GGEMSOpenCLDevice::GetGlobalMemCacheType() const {
  return Get<cl_device_mem_cache_type>(device_, CL_DEVICE_GLOBAL_MEM_CACHE_TYPE);
}

cl_uint GGEMSOpenCLDevice::GetGlobalMemCacheLineSize() const {
  return Get<cl_uint>(device_, CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE);
}

cl_ulong GGEMSOpenCLDevice::GetGlobalMemCacheSize() const {
  return Get<cl_ulong>(device_, CL_DEVICE_GLOBAL_MEM_CACHE_SIZE);
}

cl_ulong GGEMSOpenCLDevice::GetLocalMemSize() const {
  return Get<cl_ulong>(device_, CL_DEVICE_LOCAL_MEM_SIZE);
}

cl_device_local_mem_type GGEMSOpenCLDevice::GetLocalMemType() const {
  return Get<cl_device_local_mem_type>(device_, CL_DEVICE_LOCAL_MEM_TYPE);
}

cl_ulong GGEMSOpenCLDevice::GetMaxMemAllocSize() const {
  return Get<cl_ulong>(device_, CL_DEVICE_MAX_MEM_ALLOC_SIZE);
}

cl_ulong GGEMSOpenCLDevice::GetMaxConstantBufferSize() const {
  return Get<cl_ulong>(device_, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE);
}

cl_uint GGEMSOpenCLDevice::GetMaxConstantArgs() const {
  return Get<cl_uint>(device_, CL_DEVICE_MAX_CONSTANT_ARGS);
}

cl_uint GGEMSOpenCLDevice::GetMemBaseAddrAlign() const {
  return Get<cl_uint>(device_, CL_DEVICE_MEM_BASE_ADDR_ALIGN);
}

cl_uint GGEMSOpenCLDevice::GetMinDataTypeAlignSize() const {
  return Get<cl_uint>(device_, CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE);
}

cl_bool GGEMSOpenCLDevice::GetHostUnifiedMemory() const {
  return Get<cl_bool>(device_, CL_DEVICE_HOST_UNIFIED_MEMORY);
}

std::string GGEMSOpenCLDevice::GetILVersion() const {
  return Get<std::string>(device_, CL_DEVICE_IL_VERSION);
}

std::vector<cl_name_version> GGEMSOpenCLDevice::GetILSWithVersion() const {
  return GetArray<cl_name_version>(device_, CL_DEVICE_ILS_WITH_VERSION);
}

std::string GGEMSOpenCLDevice::GetSpirVersions() const {
  if (!GetILVersion().empty()) {
    return Get<std::string>(device_, CL_DEVICE_SPIR_VERSIONS);
  } else {
    return "";
  }
}

cl_command_queue_properties GGEMSOpenCLDevice::GetQueueProperties() const {
  return Get<cl_command_queue_properties>(device_, CL_DEVICE_QUEUE_PROPERTIES);
}

cl_command_queue_properties GGEMSOpenCLDevice::GetQueueOnHostProperties() const {
  return Get<cl_command_queue_properties>(device_, CL_DEVICE_QUEUE_ON_HOST_PROPERTIES);
}

cl_command_queue_properties GGEMSOpenCLDevice::GetQueueOnDeviceProperties() const {
  return Get<cl_command_queue_properties>(device_, CL_DEVICE_QUEUE_ON_DEVICE_PROPERTIES);
}

cl_uint GGEMSOpenCLDevice::GetQueueOnDevicePreferredSize() const {
  return Get<cl_uint>(device_, CL_DEVICE_QUEUE_ON_DEVICE_PREFERRED_SIZE);
}

cl_uint GGEMSOpenCLDevice::GetMaxOnDeviceQueues() const {
  return Get<cl_uint>(device_, CL_DEVICE_MAX_ON_DEVICE_QUEUES);
}

cl_uint GGEMSOpenCLDevice::GetMaxOnDeviceEvents() const {
  return Get<cl_uint>(device_, CL_DEVICE_MAX_ON_DEVICE_EVENTS);
}

cl_device_svm_capabilities GGEMSOpenCLDevice::GetSVMCapabilities() const {
  return Get<cl_device_svm_capabilities>(device_, CL_DEVICE_SVM_CAPABILITIES);
}

cl_device_atomic_capabilities GGEMSOpenCLDevice::GetAtomicMemoryCapabilities() const {
  return Get<cl_device_atomic_capabilities>(device_, CL_DEVICE_ATOMIC_MEMORY_CAPABILITIES);
}

cl_device_atomic_capabilities GGEMSOpenCLDevice::GetAtomicFenceCapabilities() const {
  return Get<cl_device_atomic_capabilities>(device_, CL_DEVICE_ATOMIC_FENCE_CAPABILITIES);
}

cl_uint GGEMSOpenCLDevice::GetMaxNumSubGroups() const {
  return Get<cl_uint>(device_, CL_DEVICE_MAX_NUM_SUB_GROUPS);
}

cl_bool GGEMSOpenCLDevice::GetSubGroupIndependentForwardProgress() const {
  return Get<cl_bool>(device_, CL_DEVICE_SUB_GROUP_INDEPENDENT_FORWARD_PROGRESS);
}

cl_device_exec_capabilities GGEMSOpenCLDevice::GetExecutionCapabilities() const {
  return Get<cl_device_exec_capabilities>(device_, CL_DEVICE_EXECUTION_CAPABILITIES);
}

cl_bool GGEMSOpenCLDevice::GetNonUniformWorkGroupSupport() const {
  return Get<cl_bool>(device_, CL_DEVICE_NON_UNIFORM_WORK_GROUP_SUPPORT);
}

cl_bool GGEMSOpenCLDevice::GetWorkGroupCollectiveFunctionsSupport() const {
  return Get<cl_bool>(device_, CL_DEVICE_WORK_GROUP_COLLECTIVE_FUNCTIONS_SUPPORT);
}
cl_bool GGEMSOpenCLDevice::GetGenericAddressSpaceSupport() const {
  return Get<cl_bool>(device_, CL_DEVICE_GENERIC_ADDRESS_SPACE_SUPPORT);
}

cl_device_device_enqueue_capabilities GGEMSOpenCLDevice::GetDeviceEnqueueCapabilities() const {
  return Get<cl_device_device_enqueue_capabilities>(device_, CL_DEVICE_DEVICE_ENQUEUE_CAPABILITIES);
}

cl_uint GGEMSOpenCLDevice::GetPartitionMaxSubDevices() const {
  return Get<cl_uint>(device_, CL_DEVICE_PARTITION_MAX_SUB_DEVICES);
}

std::vector<cl_device_partition_property> GGEMSOpenCLDevice::GetPartitionProperties() const {
  return GetArray<cl_device_partition_property>(device_, CL_DEVICE_PARTITION_PROPERTIES);
}

cl_device_affinity_domain GGEMSOpenCLDevice::GetPartitionAffinityDomain() const {
  return Get<cl_device_affinity_domain>(device_, CL_DEVICE_PARTITION_AFFINITY_DOMAIN);
}

std::vector<cl_device_partition_property> GGEMSOpenCLDevice::GetPartitionType() const {
  return GetArray<cl_device_partition_property>(device_, CL_DEVICE_PARTITION_TYPE);
}

std::string GGEMSOpenCLDevice::GetExtensions() const {
  return Get<std::string>(device_, CL_DEVICE_EXTENSIONS);
}

std::size_t GGEMSOpenCLDevice::GetMaxParameterSize() const {
  return Get<std::size_t>(device_, CL_DEVICE_MAX_PARAMETER_SIZE);
}

std::vector<cl_name_version> GGEMSOpenCLDevice::GetExtensionsWithVersion() const {
  return GetArray<cl_name_version>(device_, CL_DEVICE_EXTENSIONS_WITH_VERSION);
}

std::string GGEMSOpenCLDevice::GetLastestConformanceVersionPassed() const {
  return Get<std::string>(device_, CL_DEVICE_LATEST_CONFORMANCE_VERSION_PASSED);
}

std::string GGEMSOpenCLDevice::GetBuiltInKernels() const {
  return Get<std::string>(device_, CL_DEVICE_BUILT_IN_KERNELS);
}

std::vector<cl_name_version> GGEMSOpenCLDevice::GetBuiltInKernelsWithVersion() const {
  return GetArray<cl_name_version>(device_, CL_DEVICE_BUILT_IN_KERNELS_WITH_VERSION);
}

cl_uint GGEMSOpenCLDevice::GetPreferredPlatformAtomicAlignment() const {
  return Get<cl_uint>(device_, CL_DEVICE_PREFERRED_PLATFORM_ATOMIC_ALIGNMENT);
}

cl_uint GGEMSOpenCLDevice::GetPreferredGlobalAtomicAlignment() const {
  return Get<cl_uint>(device_, CL_DEVICE_PREFERRED_GLOBAL_ATOMIC_ALIGNMENT);
}

cl_uint GGEMSOpenCLDevice::GetPreferredLocalAtomicAlignment() const {
  return Get<cl_uint>(device_, CL_DEVICE_PREFERRED_LOCAL_ATOMIC_ALIGNMENT);
}

cl_uint GGEMSOpenCLDevice::GetAddressBits() const {
  return Get<cl_uint>(device_, CL_DEVICE_ADDRESS_BITS);
}

std::size_t GGEMSOpenCLDevice::GetProfilingTimerResolution() const {
  return Get<std::size_t>(device_, CL_DEVICE_PROFILING_TIMER_RESOLUTION);
}

cl_bool GGEMSOpenCLDevice::GetCompilerAvailable() const {
  return Get<cl_bool>(device_, CL_DEVICE_COMPILER_AVAILABLE);
}

cl_bool GGEMSOpenCLDevice::GetLinkerAvailable() const {
  return Get<cl_bool>(device_, CL_DEVICE_LINKER_AVAILABLE);
}

cl_bool GGEMSOpenCLDevice::GetAvailable() const {
  return Get<cl_bool>(device_, CL_DEVICE_AVAILABLE);
}

cl_bool GGEMSOpenCLDevice::GetEndianLittle() const {
  return Get<cl_bool>(device_, CL_DEVICE_ENDIAN_LITTLE);
}

cl_bool GGEMSOpenCLDevice::GetErrorCorrectionSupport() const {
  return Get<cl_bool>(device_, CL_DEVICE_ERROR_CORRECTION_SUPPORT);
}

std::size_t GGEMSOpenCLDevice::GetPrintfBufferSize() const {
  return Get<std::size_t>(device_, CL_DEVICE_PRINTF_BUFFER_SIZE);
}

cl_uint GGEMSOpenCLDevice::GetMaxPipeArgs() const {
  return Get<cl_uint>(device_, CL_DEVICE_MAX_PIPE_ARGS);
}

cl_uint GGEMSOpenCLDevice::GetPipeMaxActiveReservations() const {
  return Get<cl_uint>(device_, CL_DEVICE_PIPE_MAX_ACTIVE_RESERVATIONS);
}

cl_uint GGEMSOpenCLDevice::GetPipeMaxPacketSize() const {
  return Get<cl_uint>(device_, CL_DEVICE_PIPE_MAX_PACKET_SIZE);
}

cl_bool GGEMSOpenCLDevice::GetPipeSupport() const {
  return Get<cl_bool>(device_, CL_DEVICE_PIPE_SUPPORT);
}

std::size_t GGEMSOpenCLDevice::GetMaxGlobalVariableSize() const {
  return Get<std::size_t>(device_, CL_DEVICE_MAX_GLOBAL_VARIABLE_SIZE);
}

std::size_t GGEMSOpenCLDevice::GetGlobalVariablePreferredTotalSize() const {
  return Get<std::size_t>(device_, CL_DEVICE_GLOBAL_VARIABLE_PREFERRED_TOTAL_SIZE);
}

std::string GGEMSOpenCLDevice::GetUUIDKhr() const {
  auto uuid = GetArray<cl_uchar>(device_, CL_DEVICE_UUID_KHR);
  return UUIDToString(uuid.data());
}

std::string GGEMSOpenCLDevice::GetDriverUUIDKhr() const {
  auto uuid = GetArray<cl_uchar>(device_, CL_DRIVER_UUID_KHR);
  return UUIDToString(uuid.data());
}

cl_bool GGEMSOpenCLDevice::GetLUIDValidKhr() const {
  return Get<cl_bool>(device_, CL_DEVICE_LUID_VALID_KHR);
}

std::string GGEMSOpenCLDevice::GetLUIDKhr() const {
  if (GetLUIDValidKhr()) {
    auto luid = GetArray<cl_uchar>(device_, CL_DEVICE_LUID_KHR);
    return LUIDToString(luid.data());
  }
  else
    return "Not available";
}

cl_device_fp_config GGEMSOpenCLDevice::GetHalfFpConfig() const {
  return Get<cl_device_fp_config>(device_, CL_DEVICE_HALF_FP_CONFIG);
}

cl_device_fp_config GGEMSOpenCLDevice::GetSingleFpConfig() const {
  return Get<cl_device_fp_config>(device_, CL_DEVICE_SINGLE_FP_CONFIG);
}
 
cl_device_fp_config GGEMSOpenCLDevice::GetDoubleFpConfig() const {
  return Get<cl_device_fp_config>(device_, CL_DEVICE_DOUBLE_FP_CONFIG);
}

cl_uint GGEMSOpenCLDevice::GetReferenceCount() const {
  return Get<cl_uint>(device_, CL_DEVICE_REFERENCE_COUNT);
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

std::string GGEMSOpenCLDevice::CacheTypeToString(cl_device_mem_cache_type type) const {
  switch (type) {
    case CL_NONE: return "None";
    case CL_READ_ONLY_CACHE: return "Read-only cache";
    case CL_READ_WRITE_CACHE: return "Read/Write cache";
    default: {
      std::ostringstream oss;
      oss << "Unknown (0x" << std::hex << type << std::dec << ")";
      return oss.str();
    }
  }
}

std::string GGEMSOpenCLDevice::ClBoolToString(cl_bool flag) const {
  return (flag == CL_TRUE) ? "CL_TRUE" : "CL_FALSE";
}

std::string GGEMSOpenCLDevice::LocalMemTypeToString(cl_device_local_mem_type type) const {
  switch (type) {
    case CL_NONE: return "None";
    case CL_LOCAL: return "Local memory (on-chip)";
    case CL_GLOBAL: return "Global memory (emulated local)";
    default: {
      std::ostringstream oss;
      oss << "Unknown (0x" << std::hex << type << std::dec << ")";
      return oss.str();
    }
  }
}

std::string GGEMSOpenCLDevice::QueuePropertiesToString(cl_command_queue_properties props) const {
  std::ostringstream oss;
  if (props & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE)
    oss << "Out-of-order execution, ";
  if (props & CL_QUEUE_PROFILING_ENABLE)
    oss << "Profiling enabled, ";
  #ifdef CL_QUEUE_ON_DEVICE
  if (props & CL_QUEUE_ON_DEVICE)
    oss << "On-device queue, ";
  #endif
  #ifdef CL_QUEUE_ON_DEVICE_DEFAULT
  if (props & CL_QUEUE_ON_DEVICE_DEFAULT)
    oss << "Default on-device queue, ";
  #endif

  std::string result = oss.str();
  if (!result.empty())
    result.erase(result.size() - 2);
  else
    result = "None";

  return result;
}

std::string GGEMSOpenCLDevice::SVMCapabilitiesToString(cl_device_svm_capabilities caps) const {
  std::ostringstream oss;
  if (caps & CL_DEVICE_SVM_COARSE_GRAIN_BUFFER)
    oss << "Coarse-grain buffer, ";
  if (caps & CL_DEVICE_SVM_FINE_GRAIN_BUFFER)
    oss << "Fine-grain buffer, ";
  if (caps & CL_DEVICE_SVM_FINE_GRAIN_SYSTEM)
    oss << "Fine-grain system, ";
  if (caps & CL_DEVICE_SVM_ATOMICS)
    oss << "Atomics, ";

  std::string out = oss.str();
  if (!out.empty())
    out.erase(out.size() - 2);
  else
    out = "None";
  return out;
}

std::string GGEMSOpenCLDevice::AtomicCapabilitiesToString(cl_device_atomic_capabilities caps) const {
  std::ostringstream oss;
  #ifdef CL_DEVICE_ATOMIC_ORDER_RELAXED
  if (caps & CL_DEVICE_ATOMIC_ORDER_RELAXED) oss << "Relaxed order, ";
  #endif
  #ifdef CL_DEVICE_ATOMIC_ORDER_ACQ_REL
  if (caps & CL_DEVICE_ATOMIC_ORDER_ACQ_REL) oss << "Acquire/Release, ";
  #endif
  #ifdef CL_DEVICE_ATOMIC_ORDER_SEQ_CST
  if (caps & CL_DEVICE_ATOMIC_ORDER_SEQ_CST) oss << "Sequentially consistent, ";
  #endif
  #ifdef CL_DEVICE_ATOMIC_SCOPE_WORK_ITEM
  if (caps & CL_DEVICE_ATOMIC_SCOPE_WORK_ITEM) oss << "Scope: Work-item, ";
  #endif
  #ifdef CL_DEVICE_ATOMIC_SCOPE_WORK_GROUP
  if (caps & CL_DEVICE_ATOMIC_SCOPE_WORK_GROUP) oss << "Scope: Work-group, ";
  #endif
  #ifdef CL_DEVICE_ATOMIC_SCOPE_DEVICE
  if (caps & CL_DEVICE_ATOMIC_SCOPE_DEVICE) oss << "Scope: Device, ";
  #endif
  #ifdef CL_DEVICE_ATOMIC_SCOPE_ALL_DEVICES
  if (caps & CL_DEVICE_ATOMIC_SCOPE_ALL_DEVICES) oss << "Scope: All devices, ";
  #endif

  std::string s = oss.str();
  if (!s.empty()) s.erase(s.size() - 2);
  else s = "None";
  return s;
}

std::string GGEMSOpenCLDevice::DeviceEnqueueCapabilitiesToString(cl_device_device_enqueue_capabilities caps) const {
  std::ostringstream oss;
  #ifdef CL_DEVICE_QUEUE_SUPPORTED
  if (caps & CL_DEVICE_QUEUE_SUPPORTED) oss << "Device queues supported, ";
  #endif
  #ifdef CL_DEVICE_QUEUE_REPLACEABLE_DEFAULT
  if (caps & CL_DEVICE_QUEUE_REPLACEABLE_DEFAULT) oss << "Default queue replaceable, ";
  #endif
  #ifdef CL_DEVICE_QUEUE_CROSS_DEVICE
  if (caps & CL_DEVICE_QUEUE_CROSS_DEVICE) oss << "Cross-device enqueue, ";
  #endif
  #ifdef CL_DEVICE_QUEUE_CROSS_CONTEXT
  if (caps & CL_DEVICE_QUEUE_CROSS_CONTEXT) oss << "Cross-context enqueue, ";
  #endif

  std::string s = oss.str();
  if (!s.empty())
    s.erase(s.size() - 2);
  else
    s = "None";
  return s;
}

std::string GGEMSOpenCLDevice::PartitionPropertiesToString(std::vector<cl_device_partition_property> const& props) const {
  if (props.empty()) return "None";

  std::ostringstream oss;
  for (auto p : props) {
    switch (p) {
      case CL_DEVICE_PARTITION_EQUALLY:
       oss << "Equally, ";
        break;
      case CL_DEVICE_PARTITION_BY_COUNTS:
        oss << "By counts, ";
        break;
      case CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN:
        oss << "By affinity domain, ";
        break;
      default:
        break;
    }
  }

  std::string s = oss.str();
  if (!s.empty())
    s.erase(s.size() - 2);
  else s = "None";
    return s;
}

std::string GGEMSOpenCLDevice::AffinityDomainToString(cl_device_affinity_domain domain) const {
  std::ostringstream oss;

  #ifdef CL_DEVICE_AFFINITY_DOMAIN_NUMA
  if (domain & CL_DEVICE_AFFINITY_DOMAIN_NUMA) oss << "NUMA, ";
  #endif
  #ifdef CL_DEVICE_AFFINITY_DOMAIN_L4_CACHE
  if (domain & CL_DEVICE_AFFINITY_DOMAIN_L4_CACHE) oss << "L4 cache, ";
  #endif
  #ifdef CL_DEVICE_AFFINITY_DOMAIN_L3_CACHE
  if (domain & CL_DEVICE_AFFINITY_DOMAIN_L3_CACHE) oss << "L3 cache, ";
  #endif
  #ifdef CL_DEVICE_AFFINITY_DOMAIN_L2_CACHE
  if (domain & CL_DEVICE_AFFINITY_DOMAIN_L2_CACHE)
    oss << "L2 cache, ";
  #endif
  #ifdef CL_DEVICE_AFFINITY_DOMAIN_L1_CACHE
  if (domain & CL_DEVICE_AFFINITY_DOMAIN_L1_CACHE)
    oss << "L1 cache, ";
  #endif
  #ifdef CL_DEVICE_AFFINITY_DOMAIN_NEXT_PARTITIONABLE
  if (domain & CL_DEVICE_AFFINITY_DOMAIN_NEXT_PARTITIONABLE)
    oss << "Next partitionable, ";
  #endif

  std::string s = oss.str();
  if (!s.empty())
    s.erase(s.size() - 2);
  else
    s = "None";
  return s;
}

std::string GGEMSOpenCLDevice::UUIDToString(cl_uchar const* uuid) const {
  std::ostringstream oss;
  oss << std::hex << std::setfill('0');
  for (int i = 0; i < CL_UUID_SIZE_KHR; ++i) {
    oss << std::setw(2) << static_cast<int>(uuid[i]);
    if (i == 3 || i == 5 || i == 7 || i == 9)
      oss << '-';
  }
  return oss.str();
}

std::string GGEMSOpenCLDevice::LUIDToString(cl_uchar const* luid) const {
  uint64_t value{0};
  for (std::size_t i = 0; i < CL_LUID_SIZE_KHR; ++i) {
    value |= static_cast<uint64_t>(luid[i]) << (8ULL * i);
  }

  std::ostringstream oss;
  oss << std::hex << std::setfill('0') << std::setw(2) << value;
  return oss.str();
}

std::string GGEMSOpenCLDevice::FPConfigToString(cl_device_fp_config cfg) const {
  std::ostringstream oss;
  if (cfg & CL_FP_DENORM) oss << "Denormals, ";
  if (cfg & CL_FP_INF_NAN) oss << "Inf/NaN, ";
  if (cfg & CL_FP_ROUND_TO_NEAREST) oss << "RoundToNearest, ";
  if (cfg & CL_FP_ROUND_TO_ZERO) oss << "RoundToZero, ";
  if (cfg & CL_FP_ROUND_TO_INF) oss << "RoundToInf, ";
  if (cfg & CL_FP_FMA) oss << "FMA, ";
  if (cfg & CL_FP_SOFT_FLOAT) oss << "SoftFloat, ";
  if (cfg & CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT) oss << "CorrectDivideSqrt, ";

  auto s = oss.str();
  if (!s.empty())
    s.pop_back(), s.pop_back();

  return s.empty() ? "None" : s;
}

std::string GGEMSOpenCLDevice::ExecCapabilitiesToString(cl_device_exec_capabilities caps) const {
  if (caps == 0) return "None";

  std::ostringstream oss;
  bool first = true;

  auto add = [&](std::string_view name) {
    if (!first) oss << ", ";
    oss << name;
    first = false;
  };

  if (caps & CL_EXEC_KERNEL)
    add("Kernel execution");
  if (caps & CL_EXEC_NATIVE_KERNEL)
    add("Native kernel execution");

  return oss.str();
}

void GGEMSOpenCLDevice::PrintIdentity() const {
/*  gglog::info("GGEMSOpenCLDevice", "PrintIdentity") << "-> Name: "
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

  gglog::info("GGEMSOpenCLDevice", "PrintIdentity") << "-> OpenCL C All Versions: "
    << ClNameVersionToString(GetOpenCLCAllVersions()) << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintIdentity") << "-> OpenCL C Features: "
    << ClNameVersionToString(GetOpenCLCFeatures()) << gglog::endl;

  if (CheckExtension("cl_ext_cxx_for_opencl")) {
    gglog::info("GGEMSOpenCLDevice", "PrintIdentity") << "-> CXX For OpenCL Numeric Version Ext: "
      << ClVersionToString(GetCxxForOpenCLNumericVersionExt()) << gglog::endl;
  }

  gglog::info("GGEMSOpenCLDevice", "PrintIdentity") << "-> Numeric Version: "
    << ClVersionToString(GetNumericVersion()) << gglog::endl;*/
}

void GGEMSOpenCLDevice::PrintTypeID() const {
 /* gglog::info("GGEMSOpenCLDevice", "PrintTypeID") << "-> Type: "
    << DeviceTypeToString(GetType()) << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintTypeID") << "-> Vendor ID: "
    << VendorIdToString(GetVendorId()) << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintTypeID") << "-> UUID: "
    << GetUUIDKhr() << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintTypeID") << "-> Driver UUID: "
    << GetDriverUUIDKhr() << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintTypeID") << "-> LUID: "
    << GetLUIDKhr() << gglog::endl;*/
}

void GGEMSOpenCLDevice::PrintCompute() const {
/*   gglog::info("GGEMSOpenCLDevice", "PrintCompute") << "-> Max Compute Units: "
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
    << GetPreferredWorkGroupSizeMultiple() << gglog::endl;*/
}

void GGEMSOpenCLDevice::PrintVectorisation() const {
/*  gglog::info("GGEMSOpenCLDevice", "PrintVectorisation") << "-> Preferred Vector Width Char: "
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
    << GetNativeVectorWidthHalf() << gglog::endl;*/
}

void GGEMSOpenCLDevice::PrintFloatingPoint() const {
/*  if (CheckExtension("cl_khr_fp16")) {
    gglog::info("GGEMSOpenCLDevice", "PrintFloatingPoint") << "-> Half FP Config: "
      << FPConfigToString(GetHalfFpConfig()) << gglog::endl;
  }

  gglog::info("GGEMSOpenCLDevice", "PrintFloatingPoint") << "-> Single FP Config: "
    << FPConfigToString(GetSingleFpConfig()) << gglog::endl;

  if (CheckExtension("cl_khr_fp64")) {
    gglog::info("GGEMSOpenCLDevice", "PrintFloatingPoint") << "-> Double FP Config: "
      << FPConfigToString(GetDoubleFpConfig()) << gglog::endl;
  }*/
}

void GGEMSOpenCLDevice::PrintMemory() const {
/*  gglog::info("GGEMSOpenCLDevice", "PrintMemory") << "-> Max Global Variable Size: "
    << GetMaxGlobalVariableSize() << " bytes" << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintMemory") << "-> Global Variable Preferred Total Size: "
    << GetGlobalVariablePreferredTotalSize() << " bytes" << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintMemory") << "-> Global Mem Size: "
    << GetGlobalMemSize() << " bytes" << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintMemory") << "-> Global Mem Cache Type: "
    << CacheTypeToString(GetGlobalMemCacheType()) << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintMemory") << "-> Global Mem Cache Line Size: "
    << GetGlobalMemCacheLineSize() << " bytes" << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintMemory") << "-> Global Mem Cache Size: "
    << GetGlobalMemCacheSize() << " bytes" << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintMemory") << "-> Local Mem Size: "
    << GetLocalMemSize() << " bytes" << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintMemory") << "-> Local Mem Size: "
    << LocalMemTypeToString(GetLocalMemType()) << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintMemory") << "-> Max Mem Alloc Size: "
    << GetMaxMemAllocSize() << " bytes" << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintMemory") << "-> Max Constant Buffer Size: "
    << GetMaxConstantBufferSize() << " bytes" << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintMemory") << "-> Max Constant Args: "
    << GetMaxConstantArgs() << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintMemory") << "-> Mem Base Addr Align: "
    << GetMemBaseAddrAlign() << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintMemory") << "-> Min Data Type Align Size: "
    << GetMinDataTypeAlignSize() << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintMemory") << "-> Host Unified Memory: "
    << ClBoolToString(GetHostUnifiedMemory()) << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintMemory") << "-> Max Parameter Size: "
    << GetMaxParameterSize() << " bytes" << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintMemory") << "-> Preferred Platform Atomic Alignement: "
    << GetPreferredPlatformAtomicAlignment() << " bytes" << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintMemory") << "-> Preferred Global Atomic Alignement: "
    << GetPreferredGlobalAtomicAlignment() << " bytes" << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintMemory") << "-> Preferred Local Atomic Alignment: "
    << GetPreferredLocalAtomicAlignment() << " bytes" << gglog::endl;*/
}

void GGEMSOpenCLDevice::PrintImages() const {
/*   gglog::info("GGEMSOpenCLDevice", "PrintImages") << "-> Image Support: "
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

  gglog::info("GGEMSOpenCLDevice", "PrintImages") << "-> Max Read Image Args: "
    << GetMaxReadImageArgs() << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintImages") << "-> Max Write Image Args: "
    << GetMaxWriteImageArgs() << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintImages") << "-> Max Read Write Image Args: "
    << GetMaxReadWriteImageArgs() << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintImages") << "-> Image Pitch Alignment: "
    << GetImagePitchAlignment() << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintImages") << "-> Image Base Address Alignment: "
    << GetImageBaseAddressAlignment() << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintImages") << "-> Image Max Buffer Size: "
    << GetImageMaxBufferSize() << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintImages") << "-> Image Max Array Size: "
    << GetImageMaxArraySize() << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintImages") << "-> Max Samplers: "
    << GetMaxSamplers() << gglog::endl;*/
}

void GGEMSOpenCLDevice::PrintILSpirV() const {
/*  gglog::info("GGEMSOpenCLDevice", "PrintILSpirV") << "-> IL Version: "
    << ClNameVersionToString(GetILSWithVersion()) << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintILSpirV") << "-> Spir-V Versions: "
    << GetSpirVersions() << gglog::endl;*/
}

void GGEMSOpenCLDevice::PrintQueueDeviceSide() const {
/*  gglog::info("GGEMSOpenCLDevice", "PrintQueueDeviceSide") << "-> Queue Properties: "
    << QueuePropertiesToString(GetQueueProperties()) << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintQueueDeviceSide") << "-> Queue On Host Properties: "
    << QueuePropertiesToString(GetQueueOnHostProperties()) << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintQueueDeviceSide") << "-> Queue On Device Properties: "
    << QueuePropertiesToString(GetQueueOnDeviceProperties()) << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintQueueDeviceSide") << "-> Queue On Device Preferred Size: "
    << GetQueueOnDevicePreferredSize() << " bytes" << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintQueueDeviceSide") << "-> Max On Device Queues: "
    << GetMaxOnDeviceQueues() << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintQueueDeviceSide") << "-> Max On Device Events: "
    << GetMaxOnDeviceEvents() << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintQueueDeviceSide") << "-> SVM Capabilities: "
    << SVMCapabilitiesToString(GetSVMCapabilities()) << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintQueueDeviceSide") << "-> Atomic Memory Capabilities: "
    << AtomicCapabilitiesToString(GetAtomicMemoryCapabilities()) << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintQueueDeviceSide") << "-> Atomic Fence Capabilities: "
    << AtomicCapabilitiesToString(GetAtomicFenceCapabilities()) << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintQueueDeviceSide") << "-> Max Num Sub Groups: "
    << GetMaxNumSubGroups() << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintQueueDeviceSide") << "-> Sub Group Independent Forward Progress: "
    << ClBoolToString(GetSubGroupIndependentForwardProgress()) << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintQueueDeviceSide") << "-> Non Uniform Workgroup Support: "
    << ClBoolToString(GetNonUniformWorkGroupSupport()) << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintQueueDeviceSide") << "-> Workgroup Collective Functions Support: "
    << ClBoolToString(GetWorkGroupCollectiveFunctionsSupport()) << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintQueueDeviceSide") << "-> Generic Address Space Support: "
    << ClBoolToString(GetGenericAddressSpaceSupport()) << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintQueueDeviceSide") << "-> Device Enqueue Capabilities: "
    << DeviceEnqueueCapabilitiesToString(GetDeviceEnqueueCapabilities()) << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintQueueDeviceSide") << "-> Execution Capabilities: "
    << ExecCapabilitiesToString(GetExecutionCapabilities()) << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintQueueDeviceSide") << "-> Reference Count: "
    << GetReferenceCount() << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintQueueDeviceSide") << "-> Lastest Conformance Version Passed: "
    << GetLastestConformanceVersionPassed() << gglog::endl;*/
}

void GGEMSOpenCLDevice::PrintPartition() const {
/*  gglog::info("GGEMSOpenCLDevice", "PrintPartition") << "-> Partition Max Sub Devices: "
    << GetPartitionMaxSubDevices() << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintPartition") << "-> Partition Properties: "
    << PartitionPropertiesToString(GetPartitionProperties()) << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintPartition") << "-> Partition Affinity Domain: "
    << AffinityDomainToString(GetPartitionAffinityDomain()) << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintPartition") << "-> Partition Type: "
    << PartitionPropertiesToString(GetPartitionType()) << gglog::endl;*/
}

void GGEMSOpenCLDevice::PrintPipe() const {
/*  gglog::info("GGEMSOpenCLDevice", "PrintPipe") << "-> Max Pipe Args: "
    << GetMaxPipeArgs() << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintPipe") << "-> Pipe Max Active Reservations: "
    << GetPipeMaxActiveReservations() << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintPipe") << "-> Pipe Max Packet Size: "
    << GetPipeMaxPacketSize() << " bytes" << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintPipe") << "-> Pipe Support: "
    << GetPipeSupport() << " bytes" << gglog::endl;*/
}

void GGEMSOpenCLDevice::PrintExtensionsAndMisc() const {
/*  gglog::info("GGEMSOpenCLDevice", "PrintExtensionsAndMisc") << "-> Extensions With Version: "
    << ClNameVersionToString(GetExtensionsWithVersion()) << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintExtensionsAndMisc") << "-> Built In Kernels With Version: "
    << ClNameVersionToString(GetBuiltInKernelsWithVersion()) << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintExtensionsAndMisc") << "-> Address Bits: "
    << GetAddressBits() << " bytes" << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintExtensionsAndMisc") << "-> Profiling Timer Resolution: "
    << GetProfilingTimerResolution() << " ns" << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintExtensionsAndMisc") << "-> Compiler Available: "
    << ClBoolToString(GetCompilerAvailable()) << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintExtensionsAndMisc") << "-> Linker Available: "
    << ClBoolToString(GetLinkerAvailable()) << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintExtensionsAndMisc") << "-> Available: "
    << ClBoolToString(GetAvailable()) << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintExtensionsAndMisc") << "-> Endian Little: "
    << ClBoolToString(GetEndianLittle()) << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintExtensionsAndMisc") << "-> Error Correction Support: "
    << ClBoolToString(GetErrorCorrectionSupport()) << gglog::endl;

  gglog::info("GGEMSOpenCLDevice", "PrintExtensionsAndMisc") << "-> Printf Buffer Size: "
    << GetPrintfBufferSize() << " bytes" << gglog::endl;*/
}

void GGEMSOpenCLDevice::Print() const {
//  gglog::info("GGEMSOpenCLDevice", "Print") << "----- Device [" 
//    << platform_index_ << ":" << device_index_ << "] -----" << gglog::endl;

  PrintIdentity();
  PrintTypeID();
  PrintCompute();
  PrintVectorisation();
  PrintFloatingPoint();
  PrintMemory();
  PrintImages();
  PrintILSpirV();
  PrintQueueDeviceSide();
  PrintPipe();
  PrintPartition();
  PrintExtensionsAndMisc();
}
