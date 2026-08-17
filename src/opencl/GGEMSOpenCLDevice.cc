// *****************************************************************************
// * This file is part of GGEMS.                                               *
// *                                                                           *
// * SPDX-License-Identifier: GPL-3.0-or-later                                 *
// * Copyright (C) 2017-2026 CHRU de Brest, Université de Bretagne Occidentale,*
// * Inserm.                                                                   *
// *                                                                           *
// * GGEMS is free software: you can redistribute it and/or modify             *
// * it under the terms of the GNU General Public License as published by      *
// * the Free Software Foundation, either version 3 of the License, or         *
// * (at your option) any later version.                                       *
// *                                                                           *
// * GGEMS is distributed in the hope that it will be useful,                  *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
// * GNU General Public License for more details.                              *
// *                                                                           *
// * You should have received a copy of the GNU General Public License         *
// * along with GGEMS. If not, see <https://www.gnu.org/licenses/>.            *
// *****************************************************************************

/*!
 * \file
 * \brief Implements the GGEMS OpenCL device wrapper.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <utility>
#include <cstddef>
#include <string>
#include <vector>
#include <array>
/// \endcond

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/logging/GGEMSLogMacros.hh"
#include "GGEMS/opencl/GGEMSOpenCLDevice.hh"
#include "GGEMS/opencl/GGEMSOpenCLUtils.hh"
#include "GGEMS/opencl/GGEMSOpenCLStrings.hh"

namespace ggems::ocl {

// =============================================================================
// =============================================================================

GGEMSOpenCLDevice::GGEMSOpenCLDevice(cl::Device device,
                                     std::size_t platform_index,
                                     std::size_t device_index)
    : device_{std::move(device)}, platform_index_{platform_index},
      device_index_{device_index} {
  GGEMS_INFOEX("OpenCL", 3, "Creating OpenCL device [{}:{}].", platform_index_,
               device_index_);
  extensions_ = ExtractExtensions<CL_DEVICE_EXTENSIONS>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetName() const -> std::string {
  return GetInfo<CL_DEVICE_NAME>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetVendor() const -> std::string {
  return GetInfo<CL_DEVICE_VENDOR>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetVersion() const -> std::string {
  return GetInfo<CL_DEVICE_VERSION>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetDriverVersion() const -> std::string {
  return GetInfo<CL_DRIVER_VERSION>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetPlatformID() const -> cl_platform_id {
  cl_platform_id platform_id{};
  cl_int error = device_.getInfo(CL_DEVICE_PLATFORM, &platform_id);
  CheckCLError<core::GGEMSRecoverable>(error,
                                       "Failed to get OpenCL device platform.");
  return platform_id;
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetProfile() const -> std::string {
  return GetInfo<CL_DEVICE_PROFILE>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetOpenCLCVersion() const -> std::string {
  return GetInfo<CL_DEVICE_OPENCL_C_VERSION>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetOpenCLCAllVersions() const
    -> std::vector<cl_name_version> {
  return GetInfo<CL_DEVICE_OPENCL_C_ALL_VERSIONS>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetOpenCLCNumericVersionKhr() const -> cl_version_khr {
  return GetInfo<CL_DEVICE_OPENCL_C_NUMERIC_VERSION_KHR>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetOpenCLCFeatures() const
    -> std::vector<cl_name_version> {
  return GetInfo<CL_DEVICE_OPENCL_C_FEATURES>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetCxxForOpenCLNumericVersionExt() const -> cl_version {
  return GetInfo<CL_DEVICE_CXX_FOR_OPENCL_NUMERIC_VERSION_EXT>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetNumericVersion() const -> cl_version {
  return GetInfo<CL_DEVICE_NUMERIC_VERSION>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetVendorId() const -> cl_uint {
  return GetInfo<CL_DEVICE_VENDOR_ID>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetType() const -> cl_device_type {
  return GetInfo<CL_DEVICE_TYPE>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetMaxComputeUnits() const -> cl_uint {
  return GetInfo<CL_DEVICE_MAX_COMPUTE_UNITS>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetMaxClockFrequency() const -> cl_uint {
  return GetInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetMaxWorkGroupSize() const -> std::size_t {
  return GetInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetMaxWorkItemDimensions() const -> cl_uint {
  return GetInfo<CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetMaxWorkItemSizes() const
    -> std::vector<std::size_t> {
  return GetInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetPreferredWorkGroupSizeMultiple() const
    -> std::size_t {
  return GetInfo<CL_DEVICE_PREFERRED_WORK_GROUP_SIZE_MULTIPLE>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetPreferredVectorWidthChar() const -> cl_uint {
  return GetInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetPreferredVectorWidthShort() const -> cl_uint {
  return GetInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetPreferredVectorWidthInt() const -> cl_uint {
  return GetInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetPreferredVectorWidthLong() const -> cl_uint {
  return GetInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetPreferredVectorWidthFloat() const -> cl_uint {
  return GetInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetPreferredVectorWidthDouble() const -> cl_uint {
  return GetInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetPreferredVectorWidthHalf() const -> cl_uint {
  return GetInfo<CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetNativeVectorWidthChar() const -> cl_uint {
  return GetInfo<CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetNativeVectorWidthShort() const -> cl_uint {
  return GetInfo<CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetNativeVectorWidthInt() const -> cl_uint {
  return GetInfo<CL_DEVICE_NATIVE_VECTOR_WIDTH_INT>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetNativeVectorWidthLong() const -> cl_uint {
  return GetInfo<CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetNativeVectorWidthFloat() const -> cl_uint {
  return GetInfo<CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetNativeVectorWidthDouble() const -> cl_uint {
  return GetInfo<CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetNativeVectorWidthHalf() const -> cl_uint {
  return GetInfo<CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetImage2DMaxWidth() const -> std::size_t {
  return GetInfo<CL_DEVICE_IMAGE2D_MAX_WIDTH>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetImage2DMaxHeight() const -> std::size_t {
  return GetInfo<CL_DEVICE_IMAGE2D_MAX_HEIGHT>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetImage3DMaxWidth() const -> std::size_t {
  return GetInfo<CL_DEVICE_IMAGE3D_MAX_WIDTH>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetImage3DMaxHeight() const -> std::size_t {
  return GetInfo<CL_DEVICE_IMAGE3D_MAX_HEIGHT>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetImage3DMaxDepth() const -> std::size_t {
  return GetInfo<CL_DEVICE_IMAGE3D_MAX_DEPTH>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetImageMaxBufferSize() const -> std::size_t {
  return GetInfo<CL_DEVICE_IMAGE_MAX_BUFFER_SIZE>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetImageMaxArraySize() const -> std::size_t {
  return GetInfo<CL_DEVICE_IMAGE_MAX_ARRAY_SIZE>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetImageSupport() const -> cl_bool {
  return GetInfo<CL_DEVICE_IMAGE_SUPPORT>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetMaxReadImageArgs() const -> cl_uint {
  return GetInfo<CL_DEVICE_MAX_READ_IMAGE_ARGS>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetMaxWriteImageArgs() const -> cl_uint {
  return GetInfo<CL_DEVICE_MAX_WRITE_IMAGE_ARGS>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetMaxReadWriteImageArgs() const -> cl_uint {
  return GetInfo<CL_DEVICE_MAX_READ_WRITE_IMAGE_ARGS>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetImagePitchAlignment() const -> cl_uint {
  return GetInfo<CL_DEVICE_IMAGE_PITCH_ALIGNMENT>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetImageBaseAddressAlignment() const -> cl_uint {
  return GetInfo<CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetMaxSamplers() const -> cl_uint {
  return GetInfo<CL_DEVICE_MAX_SAMPLERS>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetGlobalMemSize() const -> cl_ulong {
  return GetInfo<CL_DEVICE_GLOBAL_MEM_SIZE>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetGlobalMemCacheType() const
    -> cl_device_mem_cache_type {
  return GetInfo<CL_DEVICE_GLOBAL_MEM_CACHE_TYPE>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetGlobalMemCacheLineSize() const -> cl_uint {
  return GetInfo<CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetGlobalMemCacheSize() const -> cl_ulong {
  return GetInfo<CL_DEVICE_GLOBAL_MEM_CACHE_SIZE>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetLocalMemSize() const -> cl_ulong {
  return GetInfo<CL_DEVICE_LOCAL_MEM_SIZE>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetLocalMemType() const -> cl_device_local_mem_type {
  return GetInfo<CL_DEVICE_LOCAL_MEM_TYPE>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetMaxMemAllocSize() const -> cl_ulong {
  return GetInfo<CL_DEVICE_MAX_MEM_ALLOC_SIZE>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetMaxConstantBufferSize() const -> cl_ulong {
  return GetInfo<CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetMaxConstantArgs() const -> cl_uint {
  return GetInfo<CL_DEVICE_MAX_CONSTANT_ARGS>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetMemBaseAddrAlign() const -> cl_uint {
  return GetInfo<CL_DEVICE_MEM_BASE_ADDR_ALIGN>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetMinDataTypeAlignSize() const -> cl_uint {
  return GetInfo<CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetHostUnifiedMemory() const -> cl_bool {
  return GetInfo<CL_DEVICE_HOST_UNIFIED_MEMORY>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetILVersion() const -> std::string {
  return GetInfo<CL_DEVICE_IL_VERSION>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetILSWithVersion() const
    -> std::vector<cl_name_version> {
  return GetInfo<CL_DEVICE_ILS_WITH_VERSION>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetSpirVersions() const -> std::string {
  return GetInfo<CL_DEVICE_SPIR_VERSIONS>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetQueueOnHostProperties() const
    -> cl_command_queue_properties {
  return GetInfo<CL_DEVICE_QUEUE_ON_HOST_PROPERTIES>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetQueueOnDeviceProperties() const
    -> cl_command_queue_properties {
  return GetInfo<CL_DEVICE_QUEUE_ON_DEVICE_PROPERTIES>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetQueueOnDevicePreferredSize() const -> cl_uint {
  return GetInfo<CL_DEVICE_QUEUE_ON_DEVICE_PREFERRED_SIZE>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetMaxOnDeviceQueues() const -> cl_uint {
  return GetInfo<CL_DEVICE_MAX_ON_DEVICE_QUEUES>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetMaxOnDeviceEvents() const -> cl_uint {
  return GetInfo<CL_DEVICE_MAX_ON_DEVICE_EVENTS>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetSVMCapabilities() const
    -> cl_device_svm_capabilities {
  return GetInfo<CL_DEVICE_SVM_CAPABILITIES>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetAtomicMemoryCapabilities() const
    -> cl_device_atomic_capabilities {
  return GetInfo<CL_DEVICE_ATOMIC_MEMORY_CAPABILITIES>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetAtomicFenceCapabilities() const
    -> cl_device_atomic_capabilities {
  return GetInfo<CL_DEVICE_ATOMIC_FENCE_CAPABILITIES>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetMaxNumSubGroups() const -> cl_uint {
  return GetInfo<CL_DEVICE_MAX_NUM_SUB_GROUPS>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetSubGroupIndependentForwardProgress() const
    -> cl_bool {
  return GetInfo<CL_DEVICE_SUB_GROUP_INDEPENDENT_FORWARD_PROGRESS>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetExecutionCapabilities() const
    -> cl_device_exec_capabilities {
  return GetInfo<CL_DEVICE_EXECUTION_CAPABILITIES>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetNonUniformWorkGroupSupport() const -> cl_bool {
  return GetInfo<CL_DEVICE_NON_UNIFORM_WORK_GROUP_SUPPORT>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetWorkGroupCollectiveFunctionsSupport() const
    -> cl_bool {
  return GetInfo<CL_DEVICE_WORK_GROUP_COLLECTIVE_FUNCTIONS_SUPPORT>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetGenericAddressSpaceSupport() const -> cl_bool {
  return GetInfo<CL_DEVICE_GENERIC_ADDRESS_SPACE_SUPPORT>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetDeviceEnqueueCapabilities() const
    -> cl_device_device_enqueue_capabilities {
  return GetInfo<CL_DEVICE_DEVICE_ENQUEUE_CAPABILITIES>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetPartitionMaxSubDevices() const -> cl_uint {
  return GetInfo<CL_DEVICE_PARTITION_MAX_SUB_DEVICES>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetPartitionProperties() const
    -> std::vector<cl_device_partition_property> {
  return GetInfo<CL_DEVICE_PARTITION_PROPERTIES>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetPartitionAffinityDomain() const
    -> cl_device_affinity_domain {
  return GetInfo<CL_DEVICE_PARTITION_AFFINITY_DOMAIN>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetPartitionType() const
    -> std::vector<cl_device_partition_property> {
  return GetInfo<CL_DEVICE_PARTITION_TYPE>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetExtensions() const -> std::string {
  return GetInfo<CL_DEVICE_EXTENSIONS>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetMaxParameterSize() const -> std::size_t {
  return GetInfo<CL_DEVICE_MAX_PARAMETER_SIZE>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetExtensionsWithVersion() const
    -> std::vector<cl_name_version> {
  return GetInfo<CL_DEVICE_EXTENSIONS_WITH_VERSION>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetLatestConformanceVersionPassed() const
    -> std::string {
  return GetInfo<CL_DEVICE_LATEST_CONFORMANCE_VERSION_PASSED>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetBuiltInKernels() const -> std::string {
  return GetInfo<CL_DEVICE_BUILT_IN_KERNELS>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetBuiltInKernelsWithVersion() const
    -> std::vector<cl_name_version> {
  return GetInfo<CL_DEVICE_BUILT_IN_KERNELS_WITH_VERSION>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetPreferredPlatformAtomicAlignment() const -> cl_uint {
  return GetInfo<CL_DEVICE_PREFERRED_PLATFORM_ATOMIC_ALIGNMENT>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetPreferredGlobalAtomicAlignment() const -> cl_uint {
  return GetInfo<CL_DEVICE_PREFERRED_GLOBAL_ATOMIC_ALIGNMENT>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetPreferredLocalAtomicAlignment() const -> cl_uint {
  return GetInfo<CL_DEVICE_PREFERRED_LOCAL_ATOMIC_ALIGNMENT>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetAddressBits() const -> cl_uint {
  return GetInfo<CL_DEVICE_ADDRESS_BITS>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetProfilingTimerResolution() const -> std::size_t {
  return GetInfo<CL_DEVICE_PROFILING_TIMER_RESOLUTION>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetCompilerAvailable() const -> cl_bool {
  return GetInfo<CL_DEVICE_COMPILER_AVAILABLE>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetLinkerAvailable() const -> cl_bool {
  return GetInfo<CL_DEVICE_LINKER_AVAILABLE>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetAvailable() const -> cl_bool {
  return GetInfo<CL_DEVICE_AVAILABLE>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetEndianLittle() const -> cl_bool {
  return GetInfo<CL_DEVICE_ENDIAN_LITTLE>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetErrorCorrectionSupport() const -> cl_bool {
  return GetInfo<CL_DEVICE_ERROR_CORRECTION_SUPPORT>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetPrintfBufferSize() const -> std::size_t {
  return GetInfo<CL_DEVICE_PRINTF_BUFFER_SIZE>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetMaxPipeArgs() const -> cl_uint {
  return GetInfo<CL_DEVICE_MAX_PIPE_ARGS>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetPipeMaxActiveReservations() const -> cl_uint {
  return GetInfo<CL_DEVICE_PIPE_MAX_ACTIVE_RESERVATIONS>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetPipeMaxPacketSize() const -> cl_uint {
  return GetInfo<CL_DEVICE_PIPE_MAX_PACKET_SIZE>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetPipeSupport() const -> cl_bool {
  return GetInfo<CL_DEVICE_PIPE_SUPPORT>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetMaxGlobalVariableSize() const -> std::size_t {
  return GetInfo<CL_DEVICE_MAX_GLOBAL_VARIABLE_SIZE>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetGlobalVariablePreferredTotalSize() const
    -> std::size_t {
  return GetInfo<CL_DEVICE_GLOBAL_VARIABLE_PREFERRED_TOTAL_SIZE>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetUUIDKhr() const -> std::string {
  auto uuid = GetInfo<CL_DEVICE_UUID_KHR>(device_);
  return UUIDToString(uuid);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetDriverUUIDKhr() const -> std::string {
  auto uuid = GetInfo<CL_DRIVER_UUID_KHR>(device_);
  return UUIDToString(uuid);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetLUIDValidKhr() const -> cl_bool {
  if (!HasExtension(extensions_, "cl_khr_device_uuid")) {
    return CL_FALSE;
  }

  return GetInfo<CL_DEVICE_LUID_VALID_KHR>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetLUIDKhr() const
    -> std::array<cl_uchar, CL_LUID_SIZE_KHR> {
  if (GetLUIDValidKhr() != CL_FALSE) {
    return GetInfo<CL_DEVICE_LUID_KHR>(device_);
  }

  return {};
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetHalfFpConfig() const -> cl_device_fp_config {
  return GetInfo<CL_DEVICE_HALF_FP_CONFIG>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetSingleFpConfig() const -> cl_device_fp_config {
  return GetInfo<CL_DEVICE_SINGLE_FP_CONFIG>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetDoubleFpConfig() const -> cl_device_fp_config {
  return GetInfo<CL_DEVICE_DOUBLE_FP_CONFIG>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetReferenceCount() const -> cl_uint {
  return GetInfo<CL_DEVICE_REFERENCE_COUNT>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::GetPreferredInteropUserSync() const -> cl_bool {
  return GetInfo<CL_DEVICE_PREFERRED_INTEROP_USER_SYNC>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::PrintIdentity() const -> void {
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

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::PrintTypeID() const -> void {
  PrintInfo<CL_DEVICE_TYPE>(device_);
  PrintInfo<CL_DEVICE_VENDOR_ID>(device_);

  if (HasExtension(extensions_, "cl_khr_device_uuid")) {
    PrintInfo<CL_DEVICE_UUID_KHR>(device_);
    PrintInfo<CL_DRIVER_UUID_KHR>(device_);
    PrintInfo<CL_DEVICE_LUID_KHR>(device_);
  }
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::PrintCompute() const -> void {
  PrintInfo<CL_DEVICE_MAX_COMPUTE_UNITS>(device_);
  PrintInfo<CL_DEVICE_MAX_CLOCK_FREQUENCY>(device_);
  PrintInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>(device_);
  PrintInfo<CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS>(device_);
  PrintInfo<CL_DEVICE_MAX_WORK_ITEM_SIZES>(device_);
  PrintInfo<CL_DEVICE_PREFERRED_WORK_GROUP_SIZE_MULTIPLE>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::PrintVectorization() const -> void {
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

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::PrintFloatingPoint() const -> void {
  if (HasExtension(extensions_, "cl_khr_fp16")) {
    PrintInfo<CL_DEVICE_HALF_FP_CONFIG>(device_);
  }

  PrintInfo<CL_DEVICE_SINGLE_FP_CONFIG>(device_);

  if (HasExtension(extensions_, "cl_khr_fp64")) {
    PrintInfo<CL_DEVICE_DOUBLE_FP_CONFIG>(device_);
  }
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::PrintMemory() const -> void {
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

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::PrintImages() const -> void {
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

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::PrintILSpirV() const -> void {
  if (HasExtension(extensions_, "cl_khr_spir")) {
    PrintInfo<CL_DEVICE_ILS_WITH_VERSION>(device_);
    PrintInfo<CL_DEVICE_SPIR_VERSIONS>(device_);
  }
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::PrintQueueDeviceSide() const -> void {
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

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::PrintPartition() const -> void {
  PrintInfo<CL_DEVICE_PARTITION_MAX_SUB_DEVICES>(device_);
  PrintInfo<CL_DEVICE_PARTITION_PROPERTIES>(device_);
  PrintInfo<CL_DEVICE_PARTITION_AFFINITY_DOMAIN>(device_);
  PrintInfo<CL_DEVICE_PARTITION_TYPE>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::PrintPipe() const -> void {
  PrintInfo<CL_DEVICE_MAX_PIPE_ARGS>(device_);
  PrintInfo<CL_DEVICE_PIPE_MAX_ACTIVE_RESERVATIONS>(device_);
  PrintInfo<CL_DEVICE_PIPE_MAX_PACKET_SIZE>(device_);
  PrintInfo<CL_DEVICE_PIPE_SUPPORT>(device_);
}

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::PrintExtensionsAndMisc() const -> void {
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

// -----------------------------------------------------------------------------

auto GGEMSOpenCLDevice::Print() const -> void {
  GGEMS_INFO("OpenCL", "Device [{}:{}]", platform_index_, device_index_);
  PrintIdentity();
  PrintTypeID();
  PrintCompute();
  PrintVectorization();
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
