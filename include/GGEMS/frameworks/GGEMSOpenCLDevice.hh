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
 * \brief Declares the GGEMS OpenCL device wrapper.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <array>
#include <cstddef>
#include <string>
#include <unordered_set>
#include <vector>
/// \endcond

#include "GGEMS/frameworks/GGEMSOpenCLExternal.hh"

namespace ggems::ocl {
/*!
 * \brief Wraps one native OpenCL device and exposes its capability information.
 */
class GGEMSOpenCLDevice {
public:
  /*!
   * \brief Constructs an OpenCL device wrapper.
   *
   * \param[in] device Native OpenCL device.
   * \param[in] platform_index GGEMS platform index.
   * \param[in] device_index GGEMS device index within the platform.
   */
  explicit GGEMSOpenCLDevice(cl::Device device, std::size_t platform_index,
                             std::size_t device_index);

  /*!
   * \brief Disables default construction.
   */
  GGEMSOpenCLDevice() = delete;

  /*!
   * \brief Destroys the OpenCL device wrapper.
   */
  ~GGEMSOpenCLDevice() = default;

  /*!
   * \brief Disables copy construction.
   */
  GGEMSOpenCLDevice(GGEMSOpenCLDevice const &) = delete;
  /*!
   * \brief Disables copy assignment.
   *
   * \return Reference to this device wrapper.
   */
  auto operator=(GGEMSOpenCLDevice const &) -> GGEMSOpenCLDevice & = delete;
  /*!
   * \brief Disables move assignment.
   *
   * \return Reference to this device wrapper.
   */
  auto operator=(GGEMSOpenCLDevice &&) noexcept -> GGEMSOpenCLDevice & = delete;

  /*!
   * \brief Move-constructs an OpenCL device wrapper.
   */
  GGEMSOpenCLDevice(GGEMSOpenCLDevice &&) noexcept = default;

  /*!
   * \brief Returns the GGEMS platform index.
   *
   * \return GGEMS platform index.
   */
  [[nodiscard]] auto GetPlatformIndex() const noexcept -> std::size_t {
    return platform_index_;
  }

  /*!
   * \brief Returns the GGEMS device index within its platform.
   *
   * \return GGEMS device index.
   */
  [[nodiscard]] auto GetDeviceIndex() const noexcept -> std::size_t {
    return device_index_;
  }

  /*!
   * \brief Returns the parsed device extension set.
   *
   * \return Parsed OpenCL device extension names.
   */
  [[nodiscard]] auto GetDeviceExtensions() const noexcept
      -> std::unordered_set<std::string> const & {
    return extensions_;
  }

  /*!
   * \brief Returns the native OpenCL device.
   *
   * \return Native OpenCL device.
   */
  [[nodiscard]] auto GetDeviceNative() const noexcept -> cl::Device const & {
    return device_;
  }

  /*!
   * \brief Returns the native OpenCL platform identifier.
   *
   * \return Native OpenCL platform identifier.
   */
  [[nodiscard]] auto GetPlatformID() const -> cl_platform_id;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_NAME information value.
   *
   * \return Value reported for CL_DEVICE_NAME.
   */
  [[nodiscard]] auto GetName() const -> std::string;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_VENDOR information value.
   *
   * \return Value reported for CL_DEVICE_VENDOR.
   */
  [[nodiscard]] auto GetVendor() const -> std::string;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_VERSION information value.
   *
   * \return Value reported for CL_DEVICE_VERSION.
   */
  [[nodiscard]] auto GetVersion() const -> std::string;

  /*!
   * \brief Returns the OpenCL CL_DRIVER_VERSION information value.
   *
   * \return Value reported for CL_DRIVER_VERSION.
   */
  [[nodiscard]] auto GetDriverVersion() const -> std::string;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_PROFILE information value.
   *
   * \return Value reported for CL_DEVICE_PROFILE.
   */
  [[nodiscard]] auto GetProfile() const -> std::string;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_OPENCL_C_VERSION information value.
   *
   * \return Value reported for CL_DEVICE_OPENCL_C_VERSION.
   */
  [[nodiscard]] auto GetOpenCLCVersion() const -> std::string;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_OPENCL_C_ALL_VERSIONS information value.
   *
   * \return Value reported for CL_DEVICE_OPENCL_C_ALL_VERSIONS.
   */
  [[nodiscard]] auto GetOpenCLCAllVersions() const
      -> std::vector<cl_name_version>;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_OPENCL_C_NUMERIC_VERSION_KHR information value.
   *
   * \return Value reported for CL_DEVICE_OPENCL_C_NUMERIC_VERSION_KHR.
   */
  [[nodiscard]] auto GetOpenCLCNumericVersionKhr() const -> cl_version_khr;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_OPENCL_C_FEATURES information value.
   *
   * \return Value reported for CL_DEVICE_OPENCL_C_FEATURES.
   */
  [[nodiscard]] auto GetOpenCLCFeatures() const -> std::vector<cl_name_version>;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_CXX_FOR_OPENCL_NUMERIC_VERSION_EXT information value.
   *
   * \return Value reported for CL_DEVICE_CXX_FOR_OPENCL_NUMERIC_VERSION_EXT.
   */
  [[nodiscard]] auto GetCxxForOpenCLNumericVersionExt() const -> cl_version;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_NUMERIC_VERSION information value.
   *
   * \return Value reported for CL_DEVICE_NUMERIC_VERSION.
   */
  [[nodiscard]] auto GetNumericVersion() const -> cl_version;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_UUID_KHR information value.
   *
   * \return Value reported for CL_DEVICE_UUID_KHR.
   */
  [[nodiscard]] auto GetUUIDKhr() const -> std::string;

  /*!
   * \brief Returns the OpenCL CL_DRIVER_UUID_KHR information value.
   *
   * \return Value reported for CL_DRIVER_UUID_KHR.
   */
  [[nodiscard]] auto GetDriverUUIDKhr() const -> std::string;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_LUID_VALID_KHR information value.
   *
   * \return Value reported for CL_DEVICE_LUID_VALID_KHR.
   */
  [[nodiscard]] auto GetLUIDValidKhr() const -> cl_bool;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_LUID_KHR information value.
   *
   * \return Value reported for CL_DEVICE_LUID_KHR.
   */
  [[nodiscard]] auto GetLUIDKhr() const
      -> std::array<cl_uchar, CL_LUID_SIZE_KHR>;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_VENDOR_ID information value.
   *
   * \return Value reported for CL_DEVICE_VENDOR_ID.
   */
  [[nodiscard]] auto GetVendorId() const -> cl_uint;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_TYPE information value.
   *
   * \return Value reported for CL_DEVICE_TYPE.
   */
  [[nodiscard]] auto GetType() const -> cl_device_type;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_MAX_COMPUTE_UNITS information value.
   *
   * \return Value reported for CL_DEVICE_MAX_COMPUTE_UNITS.
   */
  [[nodiscard]] auto GetMaxComputeUnits() const -> cl_uint;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_MAX_CLOCK_FREQUENCY information value.
   *
   * \return Value reported for CL_DEVICE_MAX_CLOCK_FREQUENCY.
   */
  [[nodiscard]] auto GetMaxClockFrequency() const -> cl_uint;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_MAX_WORK_GROUP_SIZE information value.
   *
   * \return Value reported for CL_DEVICE_MAX_WORK_GROUP_SIZE.
   */
  [[nodiscard]] auto GetMaxWorkGroupSize() const -> std::size_t;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS information value.
   *
   * \return Value reported for CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS.
   */
  [[nodiscard]] auto GetMaxWorkItemDimensions() const -> cl_uint;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_MAX_WORK_ITEM_SIZES information value.
   *
   * \return Value reported for CL_DEVICE_MAX_WORK_ITEM_SIZES.
   */
  [[nodiscard]] auto GetMaxWorkItemSizes() const -> std::vector<std::size_t>;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_PREFERRED_WORK_GROUP_SIZE_MULTIPLE information value.
   *
   * \return Value reported for CL_DEVICE_PREFERRED_WORK_GROUP_SIZE_MULTIPLE.
   */
  [[nodiscard]] auto GetPreferredWorkGroupSizeMultiple() const -> std::size_t;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR information value.
   *
   * \return Value reported for CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR.
   */
  [[nodiscard]] auto GetPreferredVectorWidthChar() const -> cl_uint;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT information value.
   *
   * \return Value reported for CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT.
   */
  [[nodiscard]] auto GetPreferredVectorWidthShort() const -> cl_uint;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT information value.
   *
   * \return Value reported for CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT.
   */
  [[nodiscard]] auto GetPreferredVectorWidthInt() const -> cl_uint;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG information value.
   *
   * \return Value reported for CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG.
   */
  [[nodiscard]] auto GetPreferredVectorWidthLong() const -> cl_uint;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT information value.
   *
   * \return Value reported for CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT.
   */
  [[nodiscard]] auto GetPreferredVectorWidthFloat() const -> cl_uint;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE information value.
   *
   * \return Value reported for CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE.
   */
  [[nodiscard]] auto GetPreferredVectorWidthDouble() const -> cl_uint;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF information value.
   *
   * \return Value reported for CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF.
   */
  [[nodiscard]] auto GetPreferredVectorWidthHalf() const -> cl_uint;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR information value.
   *
   * \return Value reported for CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR.
   */
  [[nodiscard]] auto GetNativeVectorWidthChar() const -> cl_uint;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT information value.
   *
   * \return Value reported for CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT.
   */
  [[nodiscard]] auto GetNativeVectorWidthShort() const -> cl_uint;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_NATIVE_VECTOR_WIDTH_INT information value.
   *
   * \return Value reported for CL_DEVICE_NATIVE_VECTOR_WIDTH_INT.
   */
  [[nodiscard]] auto GetNativeVectorWidthInt() const -> cl_uint;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG information value.
   *
   * \return Value reported for CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG.
   */
  [[nodiscard]] auto GetNativeVectorWidthLong() const -> cl_uint;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT information value.
   *
   * \return Value reported for CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT.
   */
  [[nodiscard]] auto GetNativeVectorWidthFloat() const -> cl_uint;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE information value.
   *
   * \return Value reported for CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE.
   */
  [[nodiscard]] auto GetNativeVectorWidthDouble() const -> cl_uint;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF information value.
   *
   * \return Value reported for CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF.
   */
  [[nodiscard]] auto GetNativeVectorWidthHalf() const -> cl_uint;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_HALF_FP_CONFIG information value.
   *
   * \return Value reported for CL_DEVICE_HALF_FP_CONFIG.
   */
  [[nodiscard]] auto GetHalfFpConfig() const -> cl_device_fp_config;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_SINGLE_FP_CONFIG information value.
   *
   * \return Value reported for CL_DEVICE_SINGLE_FP_CONFIG.
   */
  [[nodiscard]] auto GetSingleFpConfig() const -> cl_device_fp_config;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_DOUBLE_FP_CONFIG information value.
   *
   * \return Value reported for CL_DEVICE_DOUBLE_FP_CONFIG.
   */
  [[nodiscard]] auto GetDoubleFpConfig() const -> cl_device_fp_config;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_IMAGE_SUPPORT information value.
   *
   * \return Value reported for CL_DEVICE_IMAGE_SUPPORT.
   */
  [[nodiscard]] auto GetImageSupport() const -> cl_bool;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_IMAGE2D_MAX_WIDTH information value.
   *
   * \return Value reported for CL_DEVICE_IMAGE2D_MAX_WIDTH.
   */
  [[nodiscard]] auto GetImage2DMaxWidth() const -> std::size_t;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_IMAGE2D_MAX_HEIGHT information value.
   *
   * \return Value reported for CL_DEVICE_IMAGE2D_MAX_HEIGHT.
   */
  [[nodiscard]] auto GetImage2DMaxHeight() const -> std::size_t;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_IMAGE3D_MAX_WIDTH information value.
   *
   * \return Value reported for CL_DEVICE_IMAGE3D_MAX_WIDTH.
   */
  [[nodiscard]] auto GetImage3DMaxWidth() const -> std::size_t;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_IMAGE3D_MAX_HEIGHT information value.
   *
   * \return Value reported for CL_DEVICE_IMAGE3D_MAX_HEIGHT.
   */
  [[nodiscard]] auto GetImage3DMaxHeight() const -> std::size_t;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_IMAGE3D_MAX_DEPTH information value.
   *
   * \return Value reported for CL_DEVICE_IMAGE3D_MAX_DEPTH.
   */
  [[nodiscard]] auto GetImage3DMaxDepth() const -> std::size_t;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_IMAGE_MAX_BUFFER_SIZE information value.
   *
   * \return Value reported for CL_DEVICE_IMAGE_MAX_BUFFER_SIZE.
   */
  [[nodiscard]] auto GetImageMaxBufferSize() const -> std::size_t;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_IMAGE_MAX_ARRAY_SIZE information value.
   *
   * \return Value reported for CL_DEVICE_IMAGE_MAX_ARRAY_SIZE.
   */
  [[nodiscard]] auto GetImageMaxArraySize() const -> std::size_t;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_MAX_READ_IMAGE_ARGS information value.
   *
   * \return Value reported for CL_DEVICE_MAX_READ_IMAGE_ARGS.
   */
  [[nodiscard]] auto GetMaxReadImageArgs() const -> cl_uint;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_MAX_WRITE_IMAGE_ARGS information value.
   *
   * \return Value reported for CL_DEVICE_MAX_WRITE_IMAGE_ARGS.
   */
  [[nodiscard]] auto GetMaxWriteImageArgs() const -> cl_uint;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_MAX_READ_WRITE_IMAGE_ARGS information value.
   *
   * \return Value reported for CL_DEVICE_MAX_READ_WRITE_IMAGE_ARGS.
   */
  [[nodiscard]] auto GetMaxReadWriteImageArgs() const -> cl_uint;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_IMAGE_PITCH_ALIGNMENT information value.
   *
   * \return Value reported for CL_DEVICE_IMAGE_PITCH_ALIGNMENT.
   */
  [[nodiscard]] auto GetImagePitchAlignment() const -> cl_uint;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT information value.
   *
   * \return Value reported for CL_DEVICE_IMAGE_BASE_ADDRESS_ALIGNMENT.
   */
  [[nodiscard]] auto GetImageBaseAddressAlignment() const -> cl_uint;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_MAX_SAMPLERS information value.
   *
   * \return Value reported for CL_DEVICE_MAX_SAMPLERS.
   */
  [[nodiscard]] auto GetMaxSamplers() const -> cl_uint;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_GLOBAL_MEM_SIZE information value.
   *
   * \return Value reported for CL_DEVICE_GLOBAL_MEM_SIZE.
   */
  [[nodiscard]] auto GetGlobalMemSize() const -> cl_ulong;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_GLOBAL_MEM_CACHE_TYPE information value.
   *
   * \return Value reported for CL_DEVICE_GLOBAL_MEM_CACHE_TYPE.
   */
  [[nodiscard]] auto GetGlobalMemCacheType() const -> cl_device_mem_cache_type;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE information value.
   *
   * \return Value reported for CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE.
   */
  [[nodiscard]] auto GetGlobalMemCacheLineSize() const -> cl_uint;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_GLOBAL_MEM_CACHE_SIZE information value.
   *
   * \return Value reported for CL_DEVICE_GLOBAL_MEM_CACHE_SIZE.
   */
  [[nodiscard]] auto GetGlobalMemCacheSize() const -> cl_ulong;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_LOCAL_MEM_SIZE information value.
   *
   * \return Value reported for CL_DEVICE_LOCAL_MEM_SIZE.
   */
  [[nodiscard]] auto GetLocalMemSize() const -> cl_ulong;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_LOCAL_MEM_TYPE information value.
   *
   * \return Value reported for CL_DEVICE_LOCAL_MEM_TYPE.
   */
  [[nodiscard]] auto GetLocalMemType() const -> cl_device_local_mem_type;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_MAX_MEM_ALLOC_SIZE information value.
   *
   * \return Value reported for CL_DEVICE_MAX_MEM_ALLOC_SIZE.
   */
  [[nodiscard]] auto GetMaxMemAllocSize() const -> cl_ulong;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE information value.
   *
   * \return Value reported for CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE.
   */
  [[nodiscard]] auto GetMaxConstantBufferSize() const -> cl_ulong;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_MAX_CONSTANT_ARGS information value.
   *
   * \return Value reported for CL_DEVICE_MAX_CONSTANT_ARGS.
   */
  [[nodiscard]] auto GetMaxConstantArgs() const -> cl_uint;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_MEM_BASE_ADDR_ALIGN information value.
   *
   * \return Value reported for CL_DEVICE_MEM_BASE_ADDR_ALIGN.
   */
  [[nodiscard]] auto GetMemBaseAddrAlign() const -> cl_uint;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE information value.
   *
   * \return Value reported for CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE.
   */
  [[nodiscard]] auto GetMinDataTypeAlignSize() const -> cl_uint;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_HOST_UNIFIED_MEMORY information value.
   *
   * \return Value reported for CL_DEVICE_HOST_UNIFIED_MEMORY.
   */
  [[nodiscard]] auto GetHostUnifiedMemory() const -> cl_bool;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_IL_VERSION information value.
   *
   * \return Value reported for CL_DEVICE_IL_VERSION.
   */
  [[nodiscard]] auto GetILVersion() const -> std::string;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_ILS_WITH_VERSION information value.
   *
   * \return Value reported for CL_DEVICE_ILS_WITH_VERSION.
   */
  [[nodiscard]] auto GetILSWithVersion() const -> std::vector<cl_name_version>;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_SPIR_VERSIONS information value.
   *
   * \return Value reported for CL_DEVICE_SPIR_VERSIONS.
   */
  [[nodiscard]] auto GetSpirVersions() const -> std::string;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_QUEUE_ON_HOST_PROPERTIES information value.
   *
   * \return Value reported for CL_DEVICE_QUEUE_ON_HOST_PROPERTIES.
   */
  [[nodiscard]] auto GetQueueOnHostProperties() const
      -> cl_command_queue_properties;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_QUEUE_ON_DEVICE_PROPERTIES information value.
   *
   * \return Value reported for CL_DEVICE_QUEUE_ON_DEVICE_PROPERTIES.
   */
  [[nodiscard]] auto GetQueueOnDeviceProperties() const
      -> cl_command_queue_properties;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_QUEUE_ON_DEVICE_PREFERRED_SIZE information value.
   *
   * \return Value reported for CL_DEVICE_QUEUE_ON_DEVICE_PREFERRED_SIZE.
   */
  [[nodiscard]] auto GetQueueOnDevicePreferredSize() const -> cl_uint;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_MAX_ON_DEVICE_QUEUES information value.
   *
   * \return Value reported for CL_DEVICE_MAX_ON_DEVICE_QUEUES.
   */
  [[nodiscard]] auto GetMaxOnDeviceQueues() const -> cl_uint;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_MAX_ON_DEVICE_EVENTS information value.
   *
   * \return Value reported for CL_DEVICE_MAX_ON_DEVICE_EVENTS.
   */
  [[nodiscard]] auto GetMaxOnDeviceEvents() const -> cl_uint;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_SVM_CAPABILITIES information value.
   *
   * \return Value reported for CL_DEVICE_SVM_CAPABILITIES.
   */
  [[nodiscard]] auto GetSVMCapabilities() const -> cl_device_svm_capabilities;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_ATOMIC_MEMORY_CAPABILITIES information value.
   *
   * \return Value reported for CL_DEVICE_ATOMIC_MEMORY_CAPABILITIES.
   */
  [[nodiscard]] auto GetAtomicMemoryCapabilities() const
      -> cl_device_atomic_capabilities;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_ATOMIC_FENCE_CAPABILITIES information value.
   *
   * \return Value reported for CL_DEVICE_ATOMIC_FENCE_CAPABILITIES.
   */
  [[nodiscard]] auto GetAtomicFenceCapabilities() const
      -> cl_device_atomic_capabilities;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_MAX_NUM_SUB_GROUPS information value.
   *
   * \return Value reported for CL_DEVICE_MAX_NUM_SUB_GROUPS.
   */
  [[nodiscard]] auto GetMaxNumSubGroups() const -> cl_uint;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_SUB_GROUP_INDEPENDENT_FORWARD_PROGRESS information value.
   *
   * \return Value reported for CL_DEVICE_SUB_GROUP_INDEPENDENT_FORWARD_PROGRESS.
   */
  [[nodiscard]] auto GetSubGroupIndependentForwardProgress() const -> cl_bool;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_NON_UNIFORM_WORK_GROUP_SUPPORT information value.
   *
   * \return Value reported for CL_DEVICE_NON_UNIFORM_WORK_GROUP_SUPPORT.
   */
  [[nodiscard]] auto GetNonUniformWorkGroupSupport() const -> cl_bool;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_WORK_GROUP_COLLECTIVE_FUNCTIONS_SUPPORT information value.
   *
   * \return Value reported for CL_DEVICE_WORK_GROUP_COLLECTIVE_FUNCTIONS_SUPPORT.
   */
  [[nodiscard]] auto GetWorkGroupCollectiveFunctionsSupport() const -> cl_bool;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_GENERIC_ADDRESS_SPACE_SUPPORT information value.
   *
   * \return Value reported for CL_DEVICE_GENERIC_ADDRESS_SPACE_SUPPORT.
   */
  [[nodiscard]] auto GetGenericAddressSpaceSupport() const -> cl_bool;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_DEVICE_ENQUEUE_CAPABILITIES information value.
   *
   * \return Value reported for CL_DEVICE_DEVICE_ENQUEUE_CAPABILITIES.
   */
  [[nodiscard]] auto GetDeviceEnqueueCapabilities() const
      -> cl_device_device_enqueue_capabilities;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_EXECUTION_CAPABILITIES information value.
   *
   * \return Value reported for CL_DEVICE_EXECUTION_CAPABILITIES.
   */
  [[nodiscard]] auto GetExecutionCapabilities() const
      -> cl_device_exec_capabilities;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_REFERENCE_COUNT information value.
   *
   * \return Value reported for CL_DEVICE_REFERENCE_COUNT.
   */
  [[nodiscard]] auto GetReferenceCount() const -> cl_uint;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_LATEST_CONFORMANCE_VERSION_PASSED information value.
   *
   * \return Value reported for CL_DEVICE_LATEST_CONFORMANCE_VERSION_PASSED.
   */
  [[nodiscard]] auto GetLatestConformanceVersionPassed() const -> std::string;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_PARTITION_MAX_SUB_DEVICES information value.
   *
   * \return Value reported for CL_DEVICE_PARTITION_MAX_SUB_DEVICES.
   */
  [[nodiscard]] auto GetPartitionMaxSubDevices() const -> cl_uint;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_PARTITION_PROPERTIES information value.
   *
   * \return Value reported for CL_DEVICE_PARTITION_PROPERTIES.
   */
  [[nodiscard]] auto GetPartitionProperties() const
      -> std::vector<cl_device_partition_property>;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_PARTITION_AFFINITY_DOMAIN information value.
   *
   * \return Value reported for CL_DEVICE_PARTITION_AFFINITY_DOMAIN.
   */
  [[nodiscard]] auto GetPartitionAffinityDomain() const
      -> cl_device_affinity_domain;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_PARTITION_TYPE information value.
   *
   * \return Value reported for CL_DEVICE_PARTITION_TYPE.
   */
  [[nodiscard]] auto GetPartitionType() const
      -> std::vector<cl_device_partition_property>;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_EXTENSIONS information value.
   *
   * \return Value reported for CL_DEVICE_EXTENSIONS.
   */
  [[nodiscard]] auto GetExtensions() const -> std::string;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_EXTENSIONS_WITH_VERSION information value.
   *
   * \return Value reported for CL_DEVICE_EXTENSIONS_WITH_VERSION.
   */
  [[nodiscard]] auto GetExtensionsWithVersion() const
      -> std::vector<cl_name_version>;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_BUILT_IN_KERNELS information value.
   *
   * \return Value reported for CL_DEVICE_BUILT_IN_KERNELS.
   */
  [[nodiscard]] auto GetBuiltInKernels() const -> std::string;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_BUILT_IN_KERNELS_WITH_VERSION information value.
   *
   * \return Value reported for CL_DEVICE_BUILT_IN_KERNELS_WITH_VERSION.
   */
  [[nodiscard]] auto GetBuiltInKernelsWithVersion() const
      -> std::vector<cl_name_version>;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_PREFERRED_PLATFORM_ATOMIC_ALIGNMENT information value.
   *
   * \return Value reported for CL_DEVICE_PREFERRED_PLATFORM_ATOMIC_ALIGNMENT.
   */
  [[nodiscard]] auto GetPreferredPlatformAtomicAlignment() const -> cl_uint;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_PREFERRED_GLOBAL_ATOMIC_ALIGNMENT information value.
   *
   * \return Value reported for CL_DEVICE_PREFERRED_GLOBAL_ATOMIC_ALIGNMENT.
   */
  [[nodiscard]] auto GetPreferredGlobalAtomicAlignment() const -> cl_uint;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_PREFERRED_LOCAL_ATOMIC_ALIGNMENT information value.
   *
   * \return Value reported for CL_DEVICE_PREFERRED_LOCAL_ATOMIC_ALIGNMENT.
   */
  [[nodiscard]] auto GetPreferredLocalAtomicAlignment() const -> cl_uint;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_ADDRESS_BITS information value.
   *
   * \return Value reported for CL_DEVICE_ADDRESS_BITS.
   */
  [[nodiscard]] auto GetAddressBits() const -> cl_uint;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_PROFILING_TIMER_RESOLUTION information value.
   *
   * \return Value reported for CL_DEVICE_PROFILING_TIMER_RESOLUTION.
   */
  [[nodiscard]] auto GetProfilingTimerResolution() const -> std::size_t;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_COMPILER_AVAILABLE information value.
   *
   * \return Value reported for CL_DEVICE_COMPILER_AVAILABLE.
   */
  [[nodiscard]] auto GetCompilerAvailable() const -> cl_bool;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_LINKER_AVAILABLE information value.
   *
   * \return Value reported for CL_DEVICE_LINKER_AVAILABLE.
   */
  [[nodiscard]] auto GetLinkerAvailable() const -> cl_bool;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_AVAILABLE information value.
   *
   * \return Value reported for CL_DEVICE_AVAILABLE.
   */
  [[nodiscard]] auto GetAvailable() const -> cl_bool;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_ENDIAN_LITTLE information value.
   *
   * \return Value reported for CL_DEVICE_ENDIAN_LITTLE.
   */
  [[nodiscard]] auto GetEndianLittle() const -> cl_bool;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_ERROR_CORRECTION_SUPPORT information value.
   *
   * \return Value reported for CL_DEVICE_ERROR_CORRECTION_SUPPORT.
   */
  [[nodiscard]] auto GetErrorCorrectionSupport() const -> cl_bool;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_PRINTF_BUFFER_SIZE information value.
   *
   * \return Value reported for CL_DEVICE_PRINTF_BUFFER_SIZE.
   */
  [[nodiscard]] auto GetPrintfBufferSize() const -> std::size_t;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_PREFERRED_INTEROP_USER_SYNC information value.
   *
   * \return Value reported for CL_DEVICE_PREFERRED_INTEROP_USER_SYNC.
   */
  [[nodiscard]] auto GetPreferredInteropUserSync() const -> cl_bool;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_MAX_PIPE_ARGS information value.
   *
   * \return Value reported for CL_DEVICE_MAX_PIPE_ARGS.
   */
  [[nodiscard]] auto GetMaxPipeArgs() const -> cl_uint;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_PIPE_MAX_ACTIVE_RESERVATIONS information value.
   *
   * \return Value reported for CL_DEVICE_PIPE_MAX_ACTIVE_RESERVATIONS.
   */
  [[nodiscard]] auto GetPipeMaxActiveReservations() const -> cl_uint;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_PIPE_MAX_PACKET_SIZE information value.
   *
   * \return Value reported for CL_DEVICE_PIPE_MAX_PACKET_SIZE.
   */
  [[nodiscard]] auto GetPipeMaxPacketSize() const -> cl_uint;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_PIPE_SUPPORT information value.
   *
   * \return Value reported for CL_DEVICE_PIPE_SUPPORT.
   */
  [[nodiscard]] auto GetPipeSupport() const -> cl_bool;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_MAX_GLOBAL_VARIABLE_SIZE information value.
   *
   * \return Value reported for CL_DEVICE_MAX_GLOBAL_VARIABLE_SIZE.
   */
  [[nodiscard]] auto GetMaxGlobalVariableSize() const -> std::size_t;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_GLOBAL_VARIABLE_PREFERRED_TOTAL_SIZE information value.
   *
   * \return Value reported for CL_DEVICE_GLOBAL_VARIABLE_PREFERRED_TOTAL_SIZE.
   */
  [[nodiscard]] auto GetGlobalVariablePreferredTotalSize() const -> std::size_t;

  /*!
   * \brief Returns the OpenCL CL_DEVICE_MAX_PARAMETER_SIZE information value.
   *
   * \return Value reported for CL_DEVICE_MAX_PARAMETER_SIZE.
   */
  [[nodiscard]] auto GetMaxParameterSize() const -> std::size_t;

  /*!
   * \brief Prints the complete GGEMS OpenCL device report.
   */
  auto Print() const -> void;

private:
  /*!
   * \brief Prints device identity information.
   */
  auto PrintIdentity() const -> void;

  /*!
   * \brief Prints device type and identifier information.
   */
  auto PrintTypeID() const -> void;

  /*!
   * \brief Prints compute information.
   */
  auto PrintCompute() const -> void;

  /*!
   * \brief Prints vectorization information.
   */
  auto PrintVectorization() const -> void;

  /*!
   * \brief Prints floating-point information.
   */
  auto PrintFloatingPoint() const -> void;

  /*!
   * \brief Prints memory information.
   */
  auto PrintMemory() const -> void;

  /*!
   * \brief Prints image information.
   */
  auto PrintImages() const -> void;

  /*!
   * \brief Prints intermediate-language and SPIR-V information.
   */
  auto PrintILSpirV() const -> void;

  /*!
   * \brief Prints device-side queue information.
   */
  auto PrintQueueDeviceSide() const -> void;

  /*!
   * \brief Prints pipe information.
   */
  auto PrintPipe() const -> void;

  /*!
   * \brief Prints partitioning information.
   */
  auto PrintPartition() const -> void;

  /*!
   * \brief Prints extension and miscellaneous information.
   */
  auto PrintExtensionsAndMisc() const -> void;

  /*!
   * \brief Native OpenCL device.
   */
  cl::Device device_;
  /*!
   * \brief GGEMS platform index.
   */
  std::size_t platform_index_;
  /*!
   * \brief GGEMS device index within the platform.
   */
  std::size_t device_index_;
  /*!
   * \brief Parsed OpenCL device extension names.
   */
  std::unordered_set<std::string> extensions_;
};
} // namespace ggems::ocl
