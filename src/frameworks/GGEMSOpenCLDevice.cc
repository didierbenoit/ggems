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

#include "GGEMS/frameworks/GGEMSOpenCLDevice.hh"
#include "GGEMS/core/GGEMSCoreUtils.hh"
#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/core/GGEMSSystemUtils.hh"
#include "GGEMS/frameworks/GGEMSOpenCLUtils.hh"

namespace ggems::ocl {

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

GGEMSOpenCLDevice::GGEMSOpenCLDevice(cl::Device const &device,
                                     std::size_t platform_index,
                                     std::size_t device_index)
    : device_{device}, platform_index_{platform_index},
      device_index_{device_index} {
  GGEMS_INFOEX("OpenCL", 1, "Allocating GGEMSOpenCLDevice [{}:{}]...",
               platform_index_, device_index_);
  extensions_ = ExtractExtensions<CL_DEVICE_EXTENSIONS>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

std::string GGEMSOpenCLDevice::GetName() const {
  return GetInfo<CL_DEVICE_NAME>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

std::string GGEMSOpenCLDevice::GetVendor() const {
  return GetInfo<CL_DEVICE_VENDOR>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

std::string GGEMSOpenCLDevice::GetVersion() const {
  return GetInfo<CL_DEVICE_VERSION>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

std::string GGEMSOpenCLDevice::GetDriverVersion() const {
  return GetInfo<CL_DRIVER_VERSION>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

cl_platform_id GGEMSOpenCLDevice::GetPlatformID() const {
  cl_platform_id pid{};
  device_.getInfo(CL_DEVICE_PLATFORM, &pid);
  return pid;
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

std::string GGEMSOpenCLDevice::GetProfile() const {
  return GetInfo<CL_DEVICE_PROFILE>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

std::string GGEMSOpenCLDevice::GetOpenCLCVersion() const {
  return GetInfo<CL_DEVICE_OPENCL_C_VERSION>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

std::vector<cl_name_version> GGEMSOpenCLDevice::GetOpenCLCAllVersions() const {
  return GetInfo<CL_DEVICE_OPENCL_C_ALL_VERSIONS>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

cl_version_khr GGEMSOpenCLDevice::GetOpenCLCNumericVersionKhr() const {
  return GetInfo<CL_DEVICE_OPENCL_C_NUMERIC_VERSION_KHR>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

std::vector<cl_name_version> GGEMSOpenCLDevice::GetOpenCLCFeatures() const {
  return GetInfo<CL_DEVICE_OPENCL_C_FEATURES>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

cl_version GGEMSOpenCLDevice::GetCxxForOpenCLNumericVersionExt() const {
  return GetInfo<CL_DEVICE_CXX_FOR_OPENCL_NUMERIC_VERSION_EXT>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

cl_version GGEMSOpenCLDevice::GetNumericVersion() const {
  return GetInfo<CL_DEVICE_NUMERIC_VERSION>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

cl_uint GGEMSOpenCLDevice::GetVendorId() const {
  return GetInfo<CL_DEVICE_VENDOR_ID>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

cl_device_type GGEMSOpenCLDevice::GetType() const {
  return GetInfo<CL_DEVICE_TYPE>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

cl_uint GGEMSOpenCLDevice::GetMaxComputeUnits() const {
  return GetInfo<CL_DEVICE_MAX_COMPUTE_UNITS>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

cl_uint GGEMSOpenCLDevice::GetMaxClockFrequency() const {
  cl_uint freq{0};
  freq = device_.getInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>();

  // Si le driver ne retourne rien ou 0
  if (freq == 0) {
    auto type = device_.getInfo<CL_DEVICE_TYPE>();

    // Fallback CPU uniquement
    if (type & CL_DEVICE_TYPE_CPU) {
      if (auto cpu_freq = core::SystemUsage().cpu_frequency_) {
        return static_cast<cl_uint>(*cpu_freq);
      }
    }
  }
  return freq;
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

std::size_t GGEMSOpenCLDevice::GetMaxWorkGroupSize() const {
  return GetInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

cl_uint GGEMSOpenCLDevice::GetMaxWorkItemDimensions() const {
  return GetInfo<CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

std::vector<std::size_t> GGEMSOpenCLDevice::GetMaxWorkItemSizes() const {
  return GetInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

std::size_t GGEMSOpenCLDevice::GetPreferredWorkGroupSizeMultiple() const {
  return GetInfo<CL_DEVICE_PREFERRED_WORK_GROUP_SIZE_MULTIPLE>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

cl_uint GGEMSOpenCLDevice::GetPreferredVectorWidthChar() const {
  return GetInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

cl_uint GGEMSOpenCLDevice::GetPreferredVectorWidthShort() const {
  return GetInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

cl_uint GGEMSOpenCLDevice::GetPreferredVectorWidthInt() const {
  return GetInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

cl_uint GGEMSOpenCLDevice::GetPreferredVectorWidthLong() const {
  return GetInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

cl_uint GGEMSOpenCLDevice::GetPreferredVectorWidthFloat() const {
  return GetInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

cl_uint GGEMSOpenCLDevice::GetPreferredVectorWidthDouble() const {
  return GetInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

cl_uint GGEMSOpenCLDevice::GetPreferredVectorWidthHalf() const {
  return GetInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

cl_uint GGEMSOpenCLDevice::GetNativeVectorWidthChar() const {
  return GetInfo<CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

cl_uint GGEMSOpenCLDevice::GetNativeVectorWidthShort() const {
  return GetInfo<CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

cl_uint GGEMSOpenCLDevice::GetNativeVectorWidthInt() const {
  return GetInfo<CL_DEVICE_NATIVE_VECTOR_WIDTH_INT>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

cl_uint GGEMSOpenCLDevice::GetNativeVectorWidthLong() const {
  return GetInfo<CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

cl_uint GGEMSOpenCLDevice::GetNativeVectorWidthFloat() const {
  return GetInfo<CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

cl_uint GGEMSOpenCLDevice::GetNativeVectorWidthDouble() const {
  return GetInfo<CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

cl_uint GGEMSOpenCLDevice::GetNativeVectorWidthHalf() const {
  return GetInfo<CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

std::size_t GGEMSOpenCLDevice::GetImage2DMaxWidth() const {
  return GetInfo<CL_DEVICE_IMAGE2D_MAX_WIDTH>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

std::size_t GGEMSOpenCLDevice::GetImage2DMaxHeight() const {
  return GetInfo<CL_DEVICE_IMAGE2D_MAX_HEIGHT>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

std::size_t GGEMSOpenCLDevice::GetImage3DMaxWidth() const {
  return GetInfo<CL_DEVICE_IMAGE3D_MAX_WIDTH>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

std::size_t GGEMSOpenCLDevice::GetImage3DMaxHeight() const {
  return GetInfo<CL_DEVICE_IMAGE3D_MAX_HEIGHT>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

std::size_t GGEMSOpenCLDevice::GetImage3DMaxDepth() const {
  return GetInfo<CL_DEVICE_IMAGE3D_MAX_DEPTH>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

std::size_t GGEMSOpenCLDevice::GetImageMaxBufferSize() const {
  return GetInfo<CL_DEVICE_IMAGE_MAX_BUFFER_SIZE>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

std::size_t GGEMSOpenCLDevice::GetImageMaxArraySize() const {
  return GetInfo<CL_DEVICE_IMAGE_MAX_ARRAY_SIZE>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

cl_bool GGEMSOpenCLDevice::GetImageSupport() const {
  return GetInfo<CL_DEVICE_IMAGE_SUPPORT>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

cl_uint GGEMSOpenCLDevice::GetMaxReadImageArgs() const {
  return GetInfo<CL_DEVICE_MAX_READ_IMAGE_ARGS>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

cl_uint GGEMSOpenCLDevice::GetMaxWriteImageArgs() const {
  return GetInfo<CL_DEVICE_MAX_WRITE_IMAGE_ARGS>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

cl_uint GGEMSOpenCLDevice::GetMaxReadWriteImageArgs() const {
  return GetInfo<CL_DEVICE_MAX_READ_WRITE_IMAGE_ARGS>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

cl_uint GGEMSOpenCLDevice::GetImagePitchAlignment() const {
  return GetInfo<CL_DEVICE_IMAGE_PITCH_ALIGNMENT>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

cl_uint GGEMSOpenCLDevice::GetImageBaseAddressAlignment() const {
  return GetInfo<CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT>(device_);
}

cl_uint GGEMSOpenCLDevice::GetMaxSamplers() const {
  return GetInfo<CL_DEVICE_MAX_SAMPLERS>(device_);
}

cl_ulong GGEMSOpenCLDevice::GetGlobalMemSize() const {
  return GetInfo<CL_DEVICE_GLOBAL_MEM_SIZE>(device_);
}

cl_device_mem_cache_type GGEMSOpenCLDevice::GetGlobalMemCacheType() const {
  return GetInfo<CL_DEVICE_GLOBAL_MEM_CACHE_TYPE>(device_);
}

cl_uint GGEMSOpenCLDevice::GetGlobalMemCacheLineSize() const {
  return GetInfo<CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE>(device_);
}

cl_ulong GGEMSOpenCLDevice::GetGlobalMemCacheSize() const {
  return GetInfo<CL_DEVICE_GLOBAL_MEM_CACHE_SIZE>(device_);
}

cl_ulong GGEMSOpenCLDevice::GetLocalMemSize() const {
  return GetInfo<CL_DEVICE_LOCAL_MEM_SIZE>(device_);
}

cl_device_local_mem_type GGEMSOpenCLDevice::GetLocalMemType() const {
  return GetInfo<CL_DEVICE_LOCAL_MEM_TYPE>(device_);
}

cl_ulong GGEMSOpenCLDevice::GetMaxMemAllocSize() const {
  return GetInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>(device_);
}

cl_ulong GGEMSOpenCLDevice::GetMaxConstantBufferSize() const {
  return GetInfo<CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE>(device_);
}

cl_uint GGEMSOpenCLDevice::GetMaxConstantArgs() const {
  return GetInfo<CL_DEVICE_MAX_CONSTANT_ARGS>(device_);
}

cl_uint GGEMSOpenCLDevice::GetMemBaseAddrAlign() const {
  return GetInfo<CL_DEVICE_MEM_BASE_ADDR_ALIGN>(device_);
}

cl_uint GGEMSOpenCLDevice::GetMinDataTypeAlignSize() const {
  return GetInfo<CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE>(device_);
}

cl_bool GGEMSOpenCLDevice::GetHostUnifiedMemory() const {
  return GetInfo<CL_DEVICE_HOST_UNIFIED_MEMORY>(device_);
}

std::string GGEMSOpenCLDevice::GetILVersion() const {
  return GetInfo<CL_DEVICE_IL_VERSION>(device_);
}

std::vector<cl_name_version> GGEMSOpenCLDevice::GetILSWithVersion() const {
  return GetInfo<CL_DEVICE_ILS_WITH_VERSION>(device_);
}

std::string GGEMSOpenCLDevice::GetSpirVersions() const {
  return GetInfo<CL_DEVICE_SPIR_VERSIONS>(device_);
}

cl_command_queue_properties
GGEMSOpenCLDevice::GetQueueOnHostProperties() const {
  return GetInfo<CL_DEVICE_QUEUE_ON_HOST_PROPERTIES>(device_);
}

cl_command_queue_properties
GGEMSOpenCLDevice::GetQueueOnDeviceProperties() const {
  return GetInfo<CL_DEVICE_QUEUE_ON_DEVICE_PROPERTIES>(device_);
}

cl_uint GGEMSOpenCLDevice::GetQueueOnDevicePreferredSize() const {
  return GetInfo<CL_DEVICE_QUEUE_ON_DEVICE_PREFERRED_SIZE>(device_);
}

cl_uint GGEMSOpenCLDevice::GetMaxOnDeviceQueues() const {
  return GetInfo<CL_DEVICE_MAX_ON_DEVICE_QUEUES>(device_);
}

cl_uint GGEMSOpenCLDevice::GetMaxOnDeviceEvents() const {
  return GetInfo<CL_DEVICE_MAX_ON_DEVICE_EVENTS>(device_);
}

cl_device_svm_capabilities GGEMSOpenCLDevice::GetSVMCapabilities() const {
  return GetInfo<CL_DEVICE_SVM_CAPABILITIES>(device_);
}

cl_device_atomic_capabilities
GGEMSOpenCLDevice::GetAtomicMemoryCapabilities() const {
  return GetInfo<CL_DEVICE_ATOMIC_MEMORY_CAPABILITIES>(device_);
}

cl_device_atomic_capabilities
GGEMSOpenCLDevice::GetAtomicFenceCapabilities() const {
  return GetInfo<CL_DEVICE_ATOMIC_FENCE_CAPABILITIES>(device_);
}

cl_uint GGEMSOpenCLDevice::GetMaxNumSubGroups() const {
  return GetInfo<CL_DEVICE_MAX_NUM_SUB_GROUPS>(device_);
}

cl_bool GGEMSOpenCLDevice::GetSubGroupIndependentForwardProgress() const {
  return GetInfo<CL_DEVICE_SUB_GROUP_INDEPENDENT_FORWARD_PROGRESS>(device_);
}

cl_device_exec_capabilities
GGEMSOpenCLDevice::GetExecutionCapabilities() const {
  return GetInfo<CL_DEVICE_EXECUTION_CAPABILITIES>(device_);
}

cl_bool GGEMSOpenCLDevice::GetNonUniformWorkGroupSupport() const {
  return GetInfo<CL_DEVICE_NON_UNIFORM_WORK_GROUP_SUPPORT>(device_);
}

cl_bool GGEMSOpenCLDevice::GetWorkGroupCollectiveFunctionsSupport() const {
  return GetInfo<CL_DEVICE_WORK_GROUP_COLLECTIVE_FUNCTIONS_SUPPORT>(device_);
}
cl_bool GGEMSOpenCLDevice::GetGenericAddressSpaceSupport() const {
  return GetInfo<CL_DEVICE_GENERIC_ADDRESS_SPACE_SUPPORT>(device_);
}

cl_device_device_enqueue_capabilities
GGEMSOpenCLDevice::GetDeviceEnqueueCapabilities() const {
  return GetInfo<CL_DEVICE_DEVICE_ENQUEUE_CAPABILITIES>(device_);
}

cl_uint GGEMSOpenCLDevice::GetPartitionMaxSubDevices() const {
  return GetInfo<CL_DEVICE_PARTITION_MAX_SUB_DEVICES>(device_);
}

std::vector<cl_device_partition_property>
GGEMSOpenCLDevice::GetPartitionProperties() const {
  return GetInfo<CL_DEVICE_PARTITION_PROPERTIES>(device_);
}

cl_device_affinity_domain
GGEMSOpenCLDevice::GetPartitionAffinityDomain() const {
  return GetInfo<CL_DEVICE_PARTITION_AFFINITY_DOMAIN>(device_);
}

std::vector<cl_device_partition_property>
GGEMSOpenCLDevice::GetPartitionType() const {
  return GetInfo<CL_DEVICE_PARTITION_TYPE>(device_);
}

std::string GGEMSOpenCLDevice::GetExtensions() const {
  return GetInfo<CL_DEVICE_EXTENSIONS>(device_);
}

std::size_t GGEMSOpenCLDevice::GetMaxParameterSize() const {
  return GetInfo<CL_DEVICE_MAX_PARAMETER_SIZE>(device_);
}

std::vector<cl_name_version>
GGEMSOpenCLDevice::GetExtensionsWithVersion() const {
  return GetInfo<CL_DEVICE_EXTENSIONS_WITH_VERSION>(device_);
}

std::string GGEMSOpenCLDevice::GetLastestConformanceVersionPassed() const {
  return GetInfo<CL_DEVICE_LATEST_CONFORMANCE_VERSION_PASSED>(device_);
}

std::string GGEMSOpenCLDevice::GetBuiltInKernels() const {
  return GetInfo<CL_DEVICE_BUILT_IN_KERNELS>(device_);
}

std::vector<cl_name_version>
GGEMSOpenCLDevice::GetBuiltInKernelsWithVersion() const {
  return GetInfo<CL_DEVICE_BUILT_IN_KERNELS_WITH_VERSION>(device_);
}

cl_uint GGEMSOpenCLDevice::GetPreferredPlatformAtomicAlignment() const {
  return GetInfo<CL_DEVICE_PREFERRED_PLATFORM_ATOMIC_ALIGNMENT>(device_);
}

cl_uint GGEMSOpenCLDevice::GetPreferredGlobalAtomicAlignment() const {
  return GetInfo<CL_DEVICE_PREFERRED_GLOBAL_ATOMIC_ALIGNMENT>(device_);
}

cl_uint GGEMSOpenCLDevice::GetPreferredLocalAtomicAlignment() const {
  return GetInfo<CL_DEVICE_PREFERRED_LOCAL_ATOMIC_ALIGNMENT>(device_);
}

cl_uint GGEMSOpenCLDevice::GetAddressBits() const {
  return GetInfo<CL_DEVICE_ADDRESS_BITS>(device_);
}

std::size_t GGEMSOpenCLDevice::GetProfilingTimerResolution() const {
  return GetInfo<CL_DEVICE_PROFILING_TIMER_RESOLUTION>(device_);
}

cl_bool GGEMSOpenCLDevice::GetCompilerAvailable() const {
  return GetInfo<CL_DEVICE_COMPILER_AVAILABLE>(device_);
}

cl_bool GGEMSOpenCLDevice::GetLinkerAvailable() const {
  return GetInfo<CL_DEVICE_LINKER_AVAILABLE>(device_);
}

cl_bool GGEMSOpenCLDevice::GetAvailable() const {
  return GetInfo<CL_DEVICE_AVAILABLE>(device_);
}

cl_bool GGEMSOpenCLDevice::GetEndianLittle() const {
  return GetInfo<CL_DEVICE_ENDIAN_LITTLE>(device_);
}

cl_bool GGEMSOpenCLDevice::GetErrorCorrectionSupport() const {
  return GetInfo<CL_DEVICE_ERROR_CORRECTION_SUPPORT>(device_);
}

std::size_t GGEMSOpenCLDevice::GetPrintfBufferSize() const {
  return GetInfo<CL_DEVICE_PRINTF_BUFFER_SIZE>(device_);
}

cl_uint GGEMSOpenCLDevice::GetMaxPipeArgs() const {
  return GetInfo<CL_DEVICE_MAX_PIPE_ARGS>(device_);
}

cl_uint GGEMSOpenCLDevice::GetPipeMaxActiveReservations() const {
  return GetInfo<CL_DEVICE_PIPE_MAX_ACTIVE_RESERVATIONS>(device_);
}

cl_uint GGEMSOpenCLDevice::GetPipeMaxPacketSize() const {
  return GetInfo<CL_DEVICE_PIPE_MAX_PACKET_SIZE>(device_);
}

cl_bool GGEMSOpenCLDevice::GetPipeSupport() const {
  return GetInfo<CL_DEVICE_PIPE_SUPPORT>(device_);
}

std::size_t GGEMSOpenCLDevice::GetMaxGlobalVariableSize() const {
  return GetInfo<CL_DEVICE_MAX_GLOBAL_VARIABLE_SIZE>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

std::size_t GGEMSOpenCLDevice::GetGlobalVariablePreferredTotalSize() const {
  return GetInfo<CL_DEVICE_GLOBAL_VARIABLE_PREFERRED_TOTAL_SIZE>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

std::string GGEMSOpenCLDevice::GetUUIDKhr() const {
  auto uuid = GetInfo<CL_DEVICE_UUID_KHR>(device_);
  return UUIDToString(uuid);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

std::string GGEMSOpenCLDevice::GetDriverUUIDKhr() const {
  auto uuid = GetInfo<CL_DRIVER_UUID_KHR>(device_);
  return UUIDToString(uuid);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

cl_bool GGEMSOpenCLDevice::GetLUIDValidKhr() const {
  return GetInfo<CL_DEVICE_LUID_VALID_KHR>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

std::array<cl_uchar, CL_LUID_SIZE_KHR> GGEMSOpenCLDevice::GetLUIDKhr() const {
  if (GetLUIDValidKhr()) {
    auto luid = GetInfo<CL_DEVICE_LUID_KHR>(device_);
    return luid;
  } else
    return {};
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

cl_device_fp_config GGEMSOpenCLDevice::GetHalfFpConfig() const {
  return GetInfo<CL_DEVICE_HALF_FP_CONFIG>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

cl_device_fp_config GGEMSOpenCLDevice::GetSingleFpConfig() const {
  return GetInfo<CL_DEVICE_SINGLE_FP_CONFIG>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

cl_device_fp_config GGEMSOpenCLDevice::GetDoubleFpConfig() const {
  return GetInfo<CL_DEVICE_DOUBLE_FP_CONFIG>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

cl_uint GGEMSOpenCLDevice::GetReferenceCount() const {
  return GetInfo<CL_DEVICE_REFERENCE_COUNT>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

cl_bool GGEMSOpenCLDevice::GetPreferredInteropUserSync() const {
  return GetInfo<CL_DEVICE_PREFERRED_INTEROP_USER_SYNC>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

void GGEMSOpenCLDevice::PrintIdentity() const {
  PrintInfo<CL_DEVICE_NAME>(device_);
  PrintInfo<CL_DEVICE_VENDOR>(device_);
  PrintInfo<CL_DEVICE_VERSION>(device_);
  PrintInfo<CL_DRIVER_VERSION>(device_);
  PrintInfo<CL_DEVICE_PROFILE>(device_);
  PrintInfo<CL_DEVICE_OPENCL_C_VERSION>(device_);
  PrintInfo<CL_DEVICE_OPENCL_C_ALL_VERSIONS>(device_);
  PrintInfo<CL_DEVICE_OPENCL_C_FEATURES>(device_);

  if (HasExtension(extensions_, "cl_ext_cxx_for_opencl")) {
    PrintInfo<CL_DEVICE_CXX_FOR_OPENCL_NUMERIC_VERSION_EXT>(device_);
  }

  PrintInfo<CL_DEVICE_NUMERIC_VERSION>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

void GGEMSOpenCLDevice::PrintTypeID() const {
  PrintInfo<CL_DEVICE_TYPE>(device_);
  PrintInfo<CL_DEVICE_VENDOR_ID>(device_);

  if (HasExtension(extensions_, "cl_khr_device_uuid")) {
    PrintInfo<CL_DEVICE_UUID_KHR>(device_);
    PrintInfo<CL_DRIVER_UUID_KHR>(device_);
    PrintInfo<CL_DEVICE_LUID_KHR>(device_);
  }
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

void GGEMSOpenCLDevice::PrintCompute() const {
  PrintInfo<CL_DEVICE_MAX_COMPUTE_UNITS>(device_);
  PrintInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>(device_);
  PrintInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>(device_);
  PrintInfo<CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS>(device_);
  PrintInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>(device_);
  PrintInfo<CL_DEVICE_PREFERRED_WORK_GROUP_SIZE_MULTIPLE>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

void GGEMSOpenCLDevice::PrintVectorisation() const {
  PrintInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR>(device_);
  PrintInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT>(device_);
  PrintInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT>(device_);
  PrintInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG>(device_);
  PrintInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF>(device_);
  PrintInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT>(device_);
  PrintInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE>(device_);
  PrintInfo<CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR>(device_);
  PrintInfo<CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT>(device_);
  PrintInfo<CL_DEVICE_NATIVE_VECTOR_WIDTH_INT>(device_);
  PrintInfo<CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG>(device_);
  PrintInfo<CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF>(device_);
  PrintInfo<CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT>(device_);
  PrintInfo<CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

void GGEMSOpenCLDevice::PrintFloatingPoint() const {
  if (HasExtension(extensions_, "cl_khr_fp16")) {
    PrintInfo<CL_DEVICE_HALF_FP_CONFIG>(device_);
  }

  PrintInfo<CL_DEVICE_SINGLE_FP_CONFIG>(device_);

  if (HasExtension(extensions_, "cl_khr_fp64")) {
    PrintInfo<CL_DEVICE_DOUBLE_FP_CONFIG>(device_);
  }
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

void GGEMSOpenCLDevice::PrintMemory() const {
  PrintInfo<CL_DEVICE_MAX_GLOBAL_VARIABLE_SIZE>(device_);
  PrintInfo<CL_DEVICE_GLOBAL_VARIABLE_PREFERRED_TOTAL_SIZE>(device_);
  PrintInfo<CL_DEVICE_GLOBAL_MEM_SIZE>(device_);
  PrintInfo<CL_DEVICE_GLOBAL_MEM_CACHE_TYPE>(device_);
  PrintInfo<CL_DEVICE_GLOBAL_MEM_CACHE_SIZE>(device_);
  PrintInfo<CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE>(device_);
  PrintInfo<CL_DEVICE_LOCAL_MEM_TYPE>(device_);
  PrintInfo<CL_DEVICE_LOCAL_MEM_SIZE>(device_);
  PrintInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>(device_);
  PrintInfo<CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE>(device_);
  PrintInfo<CL_DEVICE_MAX_CONSTANT_ARGS>(device_);
  PrintInfo<CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE>(device_);
  PrintInfo<CL_DEVICE_HOST_UNIFIED_MEMORY>(device_);
  PrintInfo<CL_DEVICE_MAX_PARAMETER_SIZE>(device_);
  PrintInfo<CL_DEVICE_PREFERRED_PLATFORM_ATOMIC_ALIGNMENT>(device_);
  PrintInfo<CL_DEVICE_PREFERRED_GLOBAL_ATOMIC_ALIGNMENT>(device_);
  PrintInfo<CL_DEVICE_PREFERRED_LOCAL_ATOMIC_ALIGNMENT>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

void GGEMSOpenCLDevice::PrintImages() const {
  PrintInfo<CL_DEVICE_IMAGE_SUPPORT>(device_);
  PrintInfo<CL_DEVICE_IMAGE2D_MAX_WIDTH>(device_);
  PrintInfo<CL_DEVICE_IMAGE2D_MAX_HEIGHT>(device_);
  PrintInfo<CL_DEVICE_IMAGE3D_MAX_WIDTH>(device_);
  PrintInfo<CL_DEVICE_IMAGE3D_MAX_HEIGHT>(device_);
  PrintInfo<CL_DEVICE_IMAGE3D_MAX_DEPTH>(device_);
  PrintInfo<CL_DEVICE_MAX_READ_IMAGE_ARGS>(device_);
  PrintInfo<CL_DEVICE_MAX_WRITE_IMAGE_ARGS>(device_);
  PrintInfo<CL_DEVICE_MAX_READ_WRITE_IMAGE_ARGS>(device_);
  PrintInfo<CL_DEVICE_IMAGE_PITCH_ALIGNMENT>(device_);
  PrintInfo<CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT>(device_);
  PrintInfo<CL_DEVICE_IMAGE_MAX_BUFFER_SIZE>(device_);
  PrintInfo<CL_DEVICE_IMAGE_MAX_ARRAY_SIZE>(device_);
  PrintInfo<CL_DEVICE_MAX_SAMPLERS>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

void GGEMSOpenCLDevice::PrintILSpirV() const {
  if (HasExtension(extensions_, "cl_khr_spir")) {
    PrintInfo<CL_DEVICE_ILS_WITH_VERSION>(device_);
    PrintInfo<CL_DEVICE_SPIR_VERSIONS>(device_);
  }
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

void GGEMSOpenCLDevice::PrintQueueDeviceSide() const {
  PrintInfo<CL_DEVICE_QUEUE_ON_HOST_PROPERTIES>(device_);
  PrintInfo<CL_DEVICE_QUEUE_ON_DEVICE_PROPERTIES>(device_);
  PrintInfo<CL_DEVICE_QUEUE_ON_DEVICE_PREFERRED_SIZE>(device_);
  PrintInfo<CL_DEVICE_MAX_ON_DEVICE_QUEUES>(device_);
  PrintInfo<CL_DEVICE_MAX_ON_DEVICE_EVENTS>(device_);
  PrintInfo<CL_DEVICE_SVM_CAPABILITIES>(device_);
  PrintInfo<CL_DEVICE_ATOMIC_MEMORY_CAPABILITIES>(device_);
  PrintInfo<CL_DEVICE_ATOMIC_FENCE_CAPABILITIES>(device_);
  PrintInfo<CL_DEVICE_MAX_NUM_SUB_GROUPS>(device_);
  PrintInfo<CL_DEVICE_SUB_GROUP_INDEPENDENT_FORWARD_PROGRESS>(device_);
  PrintInfo<CL_DEVICE_NON_UNIFORM_WORK_GROUP_SUPPORT>(device_);
  PrintInfo<CL_DEVICE_WORK_GROUP_COLLECTIVE_FUNCTIONS_SUPPORT>(device_);
  PrintInfo<CL_DEVICE_GENERIC_ADDRESS_SPACE_SUPPORT>(device_);
  PrintInfo<CL_DEVICE_DEVICE_ENQUEUE_CAPABILITIES>(device_);
  PrintInfo<CL_DEVICE_EXECUTION_CAPABILITIES>(device_);
  PrintInfo<CL_DEVICE_REFERENCE_COUNT>(device_);
  PrintInfo<CL_DEVICE_LATEST_CONFORMANCE_VERSION_PASSED>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

void GGEMSOpenCLDevice::PrintPartition() const {
  PrintInfo<CL_DEVICE_PARTITION_MAX_SUB_DEVICES>(device_);
  PrintInfo<CL_DEVICE_PARTITION_PROPERTIES>(device_);
  PrintInfo<CL_DEVICE_PARTITION_AFFINITY_DOMAIN>(device_);
  PrintInfo<CL_DEVICE_PARTITION_TYPE>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

void GGEMSOpenCLDevice::PrintPipe() const {
  PrintInfo<CL_DEVICE_MAX_PIPE_ARGS>(device_);
  PrintInfo<CL_DEVICE_PIPE_MAX_ACTIVE_RESERVATIONS>(device_);
  PrintInfo<CL_DEVICE_PIPE_MAX_PACKET_SIZE>(device_);
  PrintInfo<CL_DEVICE_PIPE_SUPPORT>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

void GGEMSOpenCLDevice::PrintExtensionsAndMisc() const {
  PrintInfo<CL_DEVICE_EXTENSIONS_WITH_VERSION>(device_);
  PrintInfo<CL_DEVICE_BUILT_IN_KERNELS_WITH_VERSION>(device_);
  PrintInfo<CL_DEVICE_ADDRESS_BITS>(device_);
  PrintInfo<CL_DEVICE_PROFILING_TIMER_RESOLUTION>(device_);
  PrintInfo<CL_DEVICE_COMPILER_AVAILABLE>(device_);
  PrintInfo<CL_DEVICE_LINKER_AVAILABLE>(device_);
  PrintInfo<CL_DEVICE_AVAILABLE>(device_);
  PrintInfo<CL_DEVICE_ENDIAN_LITTLE>(device_);
  PrintInfo<CL_DEVICE_ERROR_CORRECTION_SUPPORT>(device_);
  PrintInfo<CL_DEVICE_PRINTF_BUFFER_SIZE>(device_);
  PrintInfo<CL_DEVICE_PREFERRED_INTEROP_USER_SYNC>(device_);
}

/* --------------------------------*/
/* --------------------------------*/
/* --------------------------------*/

void GGEMSOpenCLDevice::Print() const {
  GGEMS_INFO("OpenCL", "==========================");
  GGEMS_INFO("OpenCL", "Device [{}:{}]", platform_index_, device_index_);
  GGEMS_INFO("OpenCL", "==========================");
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
} // namespace ggems::ocl
