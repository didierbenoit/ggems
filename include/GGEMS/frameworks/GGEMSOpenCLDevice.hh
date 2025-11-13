#pragma once

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
 * \file GGEMSOpenCLDevice.hh
 * \brief Declaration of GGEMSOpenCLDevice class encapsulating a single OpenCL
 * device.
 *
 * This header defines a C++23, move-only wrapper around a native `cl::Device`.
 * The class exposes a comprehensive set of query accessors for device
 * capabilities, memory limits, compute characteristics, and extension support.
 * It is designed to be owned by \c GGEMSOpenCLPlatform (one device per
 * instance)
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-10-29
 * \copyright GNU General Public License v3.0
 * \version 2.0
 */

/// \cond
#include <unordered_map>
/// \endcond

#include "GGEMS/frameworks/GGEMSOpenCLUtils.hh"

/*!
 * \class GGEMSOpenCLDevice
 * \brief Immutable descriptor for an OpenCL device (GPU/CPU).
 *
 * Responsibilities:
 * - Hold a native \c cl::Device handle and its indices (platform/device).
 * - Provide strongly-typed getters to query device properties (names, memory,
 *   compute units, work-group sizes, extensions, etc.).
 * - Offer a formatted \c Print() for quick diagnostics.
 *
 * Semantics:
 * - Non-copyable, movable (exclusive ownership lives in the platform).
 * - No default constructor; the device must be explicitly provided.
 *
 * Thread-safety:
 * - Queries are read-only and thread-safe as per OpenCL C++ bindings contract.
 */
namespace ggems::ocl {
class GGEMSOpenCLDevice {
public:
  /*!
   * \brief Construct a GGEMSOpenCLDevice from a native device handle.
   * \param device          Native OpenCL device (\c cl::Device).
   * \param platform_index  Parent platform index in global enumeration.
   * \param device_index    Device index within the parent platform.
   *
   * The constructor does not perform heavy initialisation; all getters query
   * the underlying OpenCL runtime on demand. Fatal errors are reported via
   * \c GGOCL_CHECK which throws GGEMSException.
   */
  explicit GGEMSOpenCLDevice(cl::Device const &device,
                             std::size_t platform_index,
                             std::size_t device_index);

  GGEMSOpenCLDevice() = delete;
  GGEMSOpenCLDevice(GGEMSOpenCLDevice const &) = default;
  GGEMSOpenCLDevice &operator=(GGEMSOpenCLDevice const &) = default;

  /*!
   * \brief Destructor (defaulted).
   */
  ~GGEMSOpenCLDevice() = default;

  /*!
   * \brief Move constructor (no-throw).
   */
  GGEMSOpenCLDevice(GGEMSOpenCLDevice &&) noexcept = default;

  /*!
   * \brief Move assignment (no-throw).
   * \return reference to GGEMSOpenCLDevice
   */
  GGEMSOpenCLDevice &operator=(GGEMSOpenCLDevice &&) noexcept = default;

public: // ----- Identity & indices -----
  /*!
   * \brief Get the parent platform index.
   */
  [[nodiscard]] std::size_t GetPlatformIndex() const noexcept {
    return platform_index_;
  }

  /*!
   * \brief Get the device index within its platform.
   */
  [[nodiscard]] std::size_t GetDeviceIndex() const noexcept {
    return device_index_;
  }

  /*!
   * \brief Get the native OpenCL device handle.
   */
  [[nodiscard]] cl::Device const &GetNative() const noexcept { return device_; }

  [[nodiscard]] cl_platform_id GetPlatformID() const;

public: // ----- Identity properties -----
  [[nodiscard]] std::string GetName() const;

  [[nodiscard]] std::string GetVendor() const;

  [[nodiscard]] std::string GetVersion() const;

  [[nodiscard]] std::string GetDriverVersion() const;

  [[nodiscard]] std::string GetProfile() const;

  [[nodiscard]] std::string GetOpenCLCVersion() const;

  [[nodiscard]] std::vector<cl_name_version> GetOpenCLCAllVersions() const;

  [[nodiscard]] cl_version_khr GetOpenCLCNumericVersionKhr() const;

  [[nodiscard]] std::vector<cl_name_version> GetOpenCLCFeatures() const;

  [[nodiscard]] cl_version GetCxxForOpenCLNumericVersionExt() const;

  [[nodiscard]] cl_version GetNumericVersion() const;

  [[nodiscard]] std::string GetUUIDKhr() const;

  [[nodiscard]] std::string GetDriverUUIDKhr() const;

  [[nodiscard]] cl_bool GetLUIDValidKhr() const;

  [[nodiscard]] std::string GetLUIDKhr() const;

public: // ----- Numeric identifiers & types -----
  [[nodiscard]] cl_uint GetVendorId() const;

  [[nodiscard]] cl_device_type GetType() const;

public: // ----- Compute properties -----
  [[nodiscard]] cl_uint GetMaxComputeUnits() const;

  [[nodiscard]] cl_uint GetMaxClockFrequency() const;

  [[nodiscard]] std::size_t GetMaxWorkGroupSize() const;

  [[nodiscard]] cl_uint GetMaxWorkItemDimensions() const;

  [[nodiscard]] std::vector<std::size_t> GetMaxWorkItemSizes() const;

  [[nodiscard]] std::size_t GetPreferredWorkGroupSizeMultiple() const;

public: // ----- Vectorisation properties -----
  [[nodiscard]] cl_uint GetPreferredVectorWidthChar() const;

  [[nodiscard]] cl_uint GetPreferredVectorWidthShort() const;

  [[nodiscard]] cl_uint GetPreferredVectorWidthInt() const;

  [[nodiscard]] cl_uint GetPreferredVectorWidthLong() const;

  [[nodiscard]] cl_uint GetPreferredVectorWidthFloat() const;

  [[nodiscard]] cl_uint GetPreferredVectorWidthDouble() const;

  [[nodiscard]] cl_uint GetPreferredVectorWidthHalf() const;

  [[nodiscard]] cl_uint GetNativeVectorWidthChar() const;

  [[nodiscard]] cl_uint GetNativeVectorWidthShort() const;

  [[nodiscard]] cl_uint GetNativeVectorWidthInt() const;

  [[nodiscard]] cl_uint GetNativeVectorWidthLong() const;

  [[nodiscard]] cl_uint GetNativeVectorWidthFloat() const;

  [[nodiscard]] cl_uint GetNativeVectorWidthDouble() const;

  [[nodiscard]] cl_uint GetNativeVectorWidthHalf() const;

  [[nodiscard]] cl_device_fp_config GetHalfFpConfig() const;

  [[nodiscard]] cl_device_fp_config GetSingleFpConfig() const;

  [[nodiscard]] cl_device_fp_config GetDoubleFpConfig() const;

public: // ----- Images -----
  [[nodiscard]] cl_bool GetImageSupport() const;

  [[nodiscard]] std::size_t GetImage2DMaxWidth() const;

  [[nodiscard]] std::size_t GetImage2DMaxHeight() const;

  [[nodiscard]] std::size_t GetImage3DMaxWidth() const;

  [[nodiscard]] std::size_t GetImage3DMaxHeight() const;

  [[nodiscard]] std::size_t GetImage3DMaxDepth() const;

  [[nodiscard]] std::size_t GetImageMaxBufferSize() const;

  [[nodiscard]] std::size_t GetImageMaxArraySize() const;

  [[nodiscard]] cl_uint GetMaxReadImageArgs() const;

  [[nodiscard]] cl_uint GetMaxWriteImageArgs() const;

  [[nodiscard]] cl_uint GetMaxReadWriteImageArgs() const;

  [[nodiscard]] cl_uint GetImagePitchAlignment() const;

  [[nodiscard]] cl_uint GetImageBaseAddressAlignment() const;

  [[nodiscard]] cl_uint GetMaxSamplers() const;

public: // ----- Memory -----
  [[nodiscard]] cl_ulong GetGlobalMemSize() const;

  [[nodiscard]] cl_device_mem_cache_type GetGlobalMemCacheType() const;

  [[nodiscard]] cl_uint GetGlobalMemCacheLineSize() const;

  [[nodiscard]] cl_ulong GetGlobalMemCacheSize() const;

  [[nodiscard]] cl_ulong GetLocalMemSize() const;

  [[nodiscard]] cl_device_local_mem_type GetLocalMemType() const;

  [[nodiscard]] cl_ulong GetMaxMemAllocSize() const;

  [[nodiscard]] cl_ulong GetMaxConstantBufferSize() const;

  [[nodiscard]] cl_uint GetMaxConstantArgs() const;

  [[nodiscard]] cl_uint GetMemBaseAddrAlign() const;

  [[nodiscard]] cl_uint GetMinDataTypeAlignSize() const;

  [[nodiscard]] cl_bool GetHostUnifiedMemory() const;

  [[nodiscard]] std::size_t GetMaxGlobalVariableSize() const;

  [[nodiscard]] std::size_t GetGlobalVariablePreferredTotalSize() const;

  [[nodiscard]] std::size_t GetMaxParameterSize() const;

  [[nodiscard]] cl_uint GetPreferredPlatformAtomicAlignment() const;

  [[nodiscard]] cl_uint GetPreferredGlobalAtomicAlignment() const;

  [[nodiscard]] cl_uint GetPreferredLocalAtomicAlignment() const;

public: // ----- IL/SpirV -----
  [[nodiscard]] std::string GetILVersion() const;

  [[nodiscard]] std::vector<cl_name_version> GetILSWithVersion() const;

  [[nodiscard]] std::string GetSpirVersions() const;

public: // ----- Queue/Device-side -----
  [[nodiscard]] cl_command_queue_properties GetQueueOnHostProperties() const;

  [[nodiscard]] cl_command_queue_properties GetQueueOnDeviceProperties() const;

  [[nodiscard]] cl_uint GetQueueOnDevicePreferredSize() const;

  [[nodiscard]] cl_uint GetMaxOnDeviceQueues() const;

  [[nodiscard]] cl_uint GetMaxOnDeviceEvents() const;

  [[nodiscard]] cl_device_svm_capabilities GetSVMCapabilities() const;

  [[nodiscard]] cl_device_atomic_capabilities
  GetAtomicMemoryCapabilities() const;

  [[nodiscard]] cl_device_atomic_capabilities
  GetAtomicFenceCapabilities() const;

  [[nodiscard]] cl_uint GetMaxNumSubGroups() const;

  [[nodiscard]] cl_bool GetSubGroupIndependentForwardProgress() const;

  [[nodiscard]] cl_bool GetNonUniformWorkGroupSupport() const;

  [[nodiscard]] cl_bool GetWorkGroupCollectiveFunctionsSupport() const;

  [[nodiscard]] cl_bool GetGenericAddressSpaceSupport() const;

  [[nodiscard]] cl_device_device_enqueue_capabilities
  GetDeviceEnqueueCapabilities() const;

  [[nodiscard]] cl_device_exec_capabilities GetExecutionCapabilities() const;

  [[nodiscard]] cl_uint GetReferenceCount() const;

  [[nodiscard]] std::string GetLastestConformanceVersionPassed() const;

public: // ----- Partition -----
  [[nodiscard]] cl_uint GetPartitionMaxSubDevices() const;

  [[nodiscard]] std::vector<cl_device_partition_property>
  GetPartitionProperties() const;

  [[nodiscard]] cl_device_affinity_domain GetPartitionAffinityDomain() const;

  [[nodiscard]] std::vector<cl_device_partition_property>
  GetPartitionType() const;

public: // ----- Extensions & Misc
  [[nodiscard]] std::string GetExtensions() const;

  [[nodiscard]] std::vector<cl_name_version> GetExtensionsWithVersion() const;

  [[nodiscard]] std::string GetBuiltInKernels() const;

  [[nodiscard]] std::vector<cl_name_version>
  GetBuiltInKernelsWithVersion() const;

  [[nodiscard]] cl_uint GetAddressBits() const;

  [[nodiscard]] std::size_t GetProfilingTimerResolution() const;

  [[nodiscard]] cl_bool GetCompilerAvailable() const;

  [[nodiscard]] cl_bool GetLinkerAvailable() const;

  [[nodiscard]] cl_bool GetAvailable() const;

  [[nodiscard]] cl_bool GetEndianLittle() const;

  [[nodiscard]] cl_bool GetErrorCorrectionSupport() const;

  [[nodiscard]] std::size_t GetPrintfBufferSize() const;

  [[nodiscard]] cl_bool GetPreferredInteropUserSync() const;

public: // ----- Pipes -----
  [[nodiscard]] cl_uint GetMaxPipeArgs() const;

  [[nodiscard]] cl_uint GetPipeMaxActiveReservations() const;

  [[nodiscard]] cl_uint GetPipeMaxPacketSize() const;

  [[nodiscard]] cl_bool GetPipeSupport() const;

public:
  /*!
   * \brief Print a comprehensive, human-readable report for this device.
   *
   * The report includes vendor/name, type, compute units, memory sizes,
   * work-group limits, cache, extensions summary, and image capabilities.
   */
  void Print() const;

private:
  void PrintIdentity() const;
  void PrintTypeID() const;
  void PrintCompute() const;
  void PrintVectorisation() const;
  void PrintFloatingPoint() const;
  void PrintMemory() const;
  void PrintImages() const;
  void PrintILSpirV() const;
  void PrintQueueDeviceSide() const;
  void PrintPartition() const;
  void PrintPipe() const;
  void PrintExtensionsAndMisc() const;

private:
  cl::Device device_;          /*!< Native OpenCL device handle */
  std::size_t platform_index_; /*!< Parent platform index */
  std::size_t device_index_;   /*!< Device index within parent platform */
  std::unordered_set<std::string>
      extensions_; /*!< Cached device extension names */
};
} // namespace ggems::ocl
