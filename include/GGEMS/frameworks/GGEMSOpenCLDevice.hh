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
 * \brief Declaration of GGEMSOpenCLDevice providing an abstraction over OpenCL
 * device queries.
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-12-08
 * \version 2.0
 * \copyright
 * GNU General Public License v3.0
 */

#include "GGEMS/frameworks/GGEMSOpenCLUtils.hh"

namespace ggems::ocl {
/*!
 * \class GGEMSOpenCLDevice
 * \brief Encapsulates OpenCL device properties and metadata.
 *
 * This class provides accessors to all standard and extended OpenCL device
 * properties, vendor information, supported features, memory capacities,
 * vector widths, and additional capabilities. It acts as a structured device
 * descriptor used by GGEMS to query hardware characteristics at runtime.
 */
class GGEMSOpenCLDevice {
public:
  /*!
   * \brief Constructs a device descriptor from an OpenCL device handle.
   *
   * The constructor stores the native OpenCL device, associates it with its
   * platform index and device index inside the enumerated OpenCL environment,
   * and extracts the list of supported device extensions.
   *
   * \param device          Native OpenCL device handle.
   * \param platform_index  Index of the parent platform in the enumeration.
   * \param device_index    Index of this device within its platform.
   */
  explicit GGEMSOpenCLDevice(cl::Device const &device,
                             std::size_t platform_index,
                             std::size_t device_index);

  GGEMSOpenCLDevice() = delete;

  /*!
   * \brief Default destructor.
   *
   * As the class only holds OpenCL wrapper objects and STL containers,
   * destruction is non-throwing and implicitly handled.
   */
  ~GGEMSOpenCLDevice() = default;

  GGEMSOpenCLDevice(GGEMSOpenCLDevice const &) = delete;
  GGEMSOpenCLDevice &operator=(GGEMSOpenCLDevice const &) = delete;
  GGEMSOpenCLDevice &operator=(GGEMSOpenCLDevice &&) noexcept = delete;

  /*!
   * \brief Move constructor.
   *
   * Enables efficient transfer of ownership of the underlying OpenCL device
   * and metadata without deep copying. The moved-from object is left in a
   * valid but unspecified state.
   */
  GGEMSOpenCLDevice(GGEMSOpenCLDevice &&) noexcept = default;

public:
  /*!
   * \brief Returns the index of the platform that owns this device.
   * \return Platform index.
   */
  [[nodiscard]] std::size_t GetPlatformIndex() const noexcept {
    return platform_index_;
  }

  /*!
   * \brief Returns the device index within its parent platform.
   * \return Device index.
   */
  [[nodiscard]] std::size_t GetDeviceIndex() const noexcept {
    return device_index_;
  }

  /*!
   * \brief Returns the set of supported OpenCL device extensions.
   *
   * The extensions are parsed once at construction time and cached here.
   *
   * \return Unordered set of extension names.
   */
  [[nodiscard]] std::unordered_set<std::string> const &
  GetDeviceExtensions() const {
    return extensions_;
  }

  /*!
   * \brief Provides access to the underlying native device handle.
   *
   * This allows integration with OpenCL routines requiring raw cl::Device.
   *
   * \return Reference to the underlying OpenCL device.
   */
  [[nodiscard]] cl::Device const &GetDeviceNative() const noexcept {
    return device_;
  }

  /*!
   * \brief Retrieves the raw OpenCL platform ID associated with this device.
   *
   * \return cl_platform_id for the parent platform.
   */
  [[nodiscard]] cl_platform_id GetPlatformID() const;

public:
  /*!
   * \brief Returns the OpenCL device name as reported by the driver.
   * \return Device name string.
   */
  [[nodiscard]] std::string GetName() const;

  /*!
   * \brief Returns the vendor string of the OpenCL device.
   * \return Vendor name.
   */
  [[nodiscard]] std::string GetVendor() const;

  /*!
   * \brief Retrieves the OpenCL version supported by the device.
   *
   * The returned string follows the OpenCL specification format
   * (e.g., "OpenCL 3.0 ...").
   *
   * \return OpenCL version string.
   */
  [[nodiscard]] std::string GetVersion() const;

  /*!
   * \brief Returns the device driver version string.
   * \return Driver version.
   */
  [[nodiscard]] std::string GetDriverVersion() const;

  /*!
   * \brief Returns the device profile ("FULL_PROFILE" or "EMBEDDED_PROFILE").
   * \return Profile string.
   */
  [[nodiscard]] std::string GetProfile() const;

  /*!
   * \brief Retrieves the OpenCL C version supported by the device.
   * \return OpenCL C version string.
   */
  [[nodiscard]] std::string GetOpenCLCVersion() const;

  /*!
   * \brief Returns all supported OpenCL C versions with structured name-version
   * pairs.
   * \return Vector of cl_name_version entries.
   */
  [[nodiscard]] std::vector<cl_name_version> GetOpenCLCAllVersions() const;

  /*!
   * \brief Retrieves the numeric OpenCL C version (KHR extension).
   *
   * Provides a compact integer version representation encoded by
   * major/minor/patch.
   *
   * \return Numeric KHR version.
   */
  [[nodiscard]] cl_version_khr GetOpenCLCNumericVersionKhr() const;

  /*!
   * \brief Returns the list of OpenCL C features supported by the device.
   * \return Vector of cl_name_version feature descriptors.
   */
  [[nodiscard]] std::vector<cl_name_version> GetOpenCLCFeatures() const;

  /*!
   * \brief Retrieves the numeric version for the C++ for OpenCL extension (if
   * supported).
   *
   * Uses CL_DEVICE_CXX_FOR_OPENCL_NUMERIC_VERSION_EXT.
   *
   * \return Numeric version.
   */
  [[nodiscard]] cl_version GetCxxForOpenCLNumericVersionExt() const;

  /*!
   * \brief Returns the OpenCL numeric device version.
   *
   * Encoded as per the OpenCL specification: major, minor, patch.
   *
   * \return Numeric version value.
   */
  [[nodiscard]] cl_version GetNumericVersion() const;

  /*!
   * \brief Retrieves the device UUID (KHR extension).
   *
   * The returned UUID is encoded as a formatted string.
   *
   * \return Device UUID string.
   */
  [[nodiscard]] std::string GetUUIDKhr() const;

  /*!
   * \brief Retrieves the driver-level UUID (KHR extension).
   * \return Driver UUID as a string.
   */
  [[nodiscard]] std::string GetDriverUUIDKhr() const;

  /*!
   * \brief Indicates whether the device LUID (locally unique ID) is valid.
   * \return CL_TRUE if valid, CL_FALSE otherwise.
   */
  [[nodiscard]] cl_bool GetLUIDValidKhr() const;

  /*!
   * \brief Retrieves the device LUID if available (KHR extension).
   *
   * If unavailable, an empty array is returned.
   *
   * \return Array of bytes representing the LUID.
   */
  [[nodiscard]] std::array<cl_uchar, CL_LUID_SIZE_KHR> GetLUIDKhr() const;

  /*!
   * \brief Returns the vendor ID encoded as a 32-bit integer.
   * \return Vendor ID.
   */
  [[nodiscard]] cl_uint GetVendorId() const;

  /*!
   * \brief Returns the OpenCL device type (CPU, GPU, ACCELERATOR...).
   * \return Device type bitfield.
   */
  [[nodiscard]] cl_device_type GetType() const;

  /*!
   * \brief Retrieves the number of compute units of the device.
   *
   * Corresponds to the number of hardware compute engines available.
   *
   * \return Compute unit count.
   */
  [[nodiscard]] cl_uint GetMaxComputeUnits() const;

  /*!
   * \brief Returns the maximum clock frequency in MHz.
   *
   * Some CPU drivers may report 0; GGEMS attempts a fallback using
   * SystemUsage if applicable.
   *
   * \return Maximum clock frequency in MHz.
   */
  [[nodiscard]] cl_uint GetMaxClockFrequency() const;

  /*!
   * \brief Returns the maximum permissible work-group size.
   * \return Max work-group size.
   */
  [[nodiscard]] std::size_t GetMaxWorkGroupSize() const;

  /*!
   * \brief Returns the dimensionality supported for work-items.
   * \return Number of supported dimensions.
   */
  [[nodiscard]] cl_uint GetMaxWorkItemDimensions() const;

  /*!
   * \brief Returns the maximum size for each dimension of work-items.
   * \return Vector of per-dimension limits.
   */
  [[nodiscard]] std::vector<std::size_t> GetMaxWorkItemSizes() const;

  /*!
   * \brief Returns the preferred work-group size multiple.
   *
   * This value often corresponds to warp/wavefront sizes.
   *
   * \return Preferred multiple.
   */
  [[nodiscard]] std::size_t GetPreferredWorkGroupSizeMultiple() const;

  /*!
   * \brief Returns the preferred vector width for char operations.
   * \return Vector width.
   */
  [[nodiscard]] cl_uint GetPreferredVectorWidthChar() const;

  /*!
   * \brief Returns the preferred vector width for short operations.
   * \return Vector width.
   */
  [[nodiscard]] cl_uint GetPreferredVectorWidthShort() const;

  /*!
   * \brief Returns the preferred vector width for int operations.
   * \return Vector width.
   */
  [[nodiscard]] cl_uint GetPreferredVectorWidthInt() const;

  /*!
   * \brief Returns the preferred vector width for long operations.
   * \return Vector width.
   */
  [[nodiscard]] cl_uint GetPreferredVectorWidthLong() const;

  /*!
   * \brief Returns the preferred vector width for float operations.
   * \return Vector width.
   */
  [[nodiscard]] cl_uint GetPreferredVectorWidthFloat() const;

  /*!
   * \brief Returns the preferred vector width for double operations.
   * \return Vector width.
   */
  [[nodiscard]] cl_uint GetPreferredVectorWidthDouble() const;

  /*!
   * \brief Returns the preferred vector width for half operations.
   * \return Vector width.
   */
  [[nodiscard]] cl_uint GetPreferredVectorWidthHalf() const;

  /*!
   * \brief Returns the native vector width for char operations.
   * \return Native vector width.
   */
  [[nodiscard]] cl_uint GetNativeVectorWidthChar() const;

  /*!
   * \brief Returns the native vector width for short operations.
   * \return Native vector width.
   */
  [[nodiscard]] cl_uint GetNativeVectorWidthShort() const;

  /*!
   * \brief Returns the native vector width for int operations.
   * \return Native vector width.
   */
  [[nodiscard]] cl_uint GetNativeVectorWidthInt() const;

  /*!
   * \brief Returns the native vector width for long operations.
   * \return Native vector width.
   */
  [[nodiscard]] cl_uint GetNativeVectorWidthLong() const;

  /*!
   * \brief Returns the native vector width for float operations.
   * \return Native vector width.
   */
  [[nodiscard]] cl_uint GetNativeVectorWidthFloat() const;

  /*!
   * \brief Returns the native vector width for double operations.
   * \return Native vector width.
   */
  [[nodiscard]] cl_uint GetNativeVectorWidthDouble() const;

  /*!
   * \brief Returns the native vector width for half operations.
   * \return Native vector width.
   */
  [[nodiscard]] cl_uint GetNativeVectorWidthHalf() const;

  /*!
   * \brief Retrieves the FP16 configuration bits, if supported.
   * \return Half precision FP configuration.
   */
  [[nodiscard]] cl_device_fp_config GetHalfFpConfig() const;

  /*!
   * \brief Retrieves the FP32 configuration bits.
   * \return Single precision FP configuration.
   */
  [[nodiscard]] cl_device_fp_config GetSingleFpConfig() const;

  /*!
   * \brief Retrieves the FP64 configuration bits, if supported.
   * \return Double precision FP configuration.
   */
  [[nodiscard]] cl_device_fp_config GetDoubleFpConfig() const;

  /*!
   * \brief Indicates if image objects are supported by the device.
   * \return CL_TRUE or CL_FALSE.
   */
  [[nodiscard]] cl_bool GetImageSupport() const;

  /*!
   * \brief Returns the maximum width of a 2D image.
   * \return Maximum width in pixels.
   */
  [[nodiscard]] std::size_t GetImage2DMaxWidth() const;

  /*!
   * \brief Returns the maximum height of a 2D image.
   * \return Maximum height in pixels.
   */
  [[nodiscard]] std::size_t GetImage2DMaxHeight() const;

  /*!
   * \brief Returns the maximum width of a 3D image.
   * \return Maximum width in pixels.
   */
  [[nodiscard]] std::size_t GetImage3DMaxWidth() const;

  /*!
   * \brief Returns the maximum height of a 3D image.
   * \return Maximum height in pixels.
   */
  [[nodiscard]] std::size_t GetImage3DMaxHeight() const;

  /*!
   * \brief Returns the maximum depth of a 3D image.
   * \return Maximum depth in pixels.
   */
  [[nodiscard]] std::size_t GetImage3DMaxDepth() const;

  /*!
   * \brief Returns the maximum buffer size for image objects.
   * \return Max buffer size.
   */
  [[nodiscard]] std::size_t GetImageMaxBufferSize() const;

  /*!
   * \brief Returns the maximum number of elements allowed in an image array.
   * \return Max array size.
   */
  [[nodiscard]] std::size_t GetImageMaxArraySize() const;

  /*!
   * \brief Returns the maximum number of read-only image arguments.
   * \return Max image read arguments.
   */
  [[nodiscard]] cl_uint GetMaxReadImageArgs() const;

  /*!
   * \brief Returns the maximum number of write-only image arguments.
   * \return Max image write arguments.
   */
  [[nodiscard]] cl_uint GetMaxWriteImageArgs() const;

  /*!
   * \brief Returns the maximum number of read-write image arguments.
   * \return Max image read-write arguments.
   */
  [[nodiscard]] cl_uint GetMaxReadWriteImageArgs() const;

  /*!
   * \brief Returns the required alignment for row pitch in images.
   * \return Pitch alignment in bytes.
   */
  [[nodiscard]] cl_uint GetImagePitchAlignment() const;

  /*!
   * \brief Returns the base address alignment needed for images.
   * \return Alignment in bytes.
   */
  [[nodiscard]] cl_uint GetImageBaseAddressAlignment() const;

  /*!
   * \brief Returns the maximum number of samplers available.
   * \return Sampler count.
   */
  [[nodiscard]] cl_uint GetMaxSamplers() const;

  /*!
   * \brief Returns the total size of global memory in bytes.
   * \return Global memory size.
   */
  [[nodiscard]] cl_ulong GetGlobalMemSize() const;

  /*!
   * \brief Returns the global memory cache type (none, read-only, read-write).
   * \return Cache type.
   */
  [[nodiscard]] cl_device_mem_cache_type GetGlobalMemCacheType() const;

  /*!
   * \brief Returns the size in bytes of a global memory cache line.
   * \return Cache line size.
   */
  [[nodiscard]] cl_uint GetGlobalMemCacheLineSize() const;

  /*!
   * \brief Returns the global memory cache size in bytes.
   * \return Cache size.
   */
  [[nodiscard]] cl_ulong GetGlobalMemCacheSize() const;

  /*!
   * \brief Returns the amount of local memory available per compute unit.
   * \return Local memory size.
   */
  [[nodiscard]] cl_ulong GetLocalMemSize() const;

  /*!
   * \brief Returns the type of local memory (local vs global).
   * \return Local memory type.
   */
  [[nodiscard]] cl_device_local_mem_type GetLocalMemType() const;

  /*!
   * \brief Returns the maximum size of a single allocation.
   * \return Max alloc size in bytes.
   */
  [[nodiscard]] cl_ulong GetMaxMemAllocSize() const;

  /*!
   * \brief Returns the maximum constant buffer size in bytes.
   * \return Max constant buffer size.
   */
  [[nodiscard]] cl_ulong GetMaxConstantBufferSize() const;

  /*!
   * \brief Returns the maximum number of constant arguments.
   * \return Max constant argument count.
   */
  [[nodiscard]] cl_uint GetMaxConstantArgs() const;

  /*!
   * \brief Returns the base address alignment for memory objects.
   * \return Alignment in bytes.
   */
  [[nodiscard]] cl_uint GetMemBaseAddrAlign() const;

  /*!
   * \brief Returns minimal alignment for types in memory allocations.
   * \return Minimal alignment size.
   */
  [[nodiscard]] cl_uint GetMinDataTypeAlignSize() const;

  /*!
   * \brief Indicates whether the device shares memory with the host.
   * \return CL_TRUE if unified memory is supported.
   */
  [[nodiscard]] cl_bool GetHostUnifiedMemory() const;

  /*!
   * \brief Returns the IL (Intermediate Language) version string, if supported.
   * \return IL version string.
   */
  [[nodiscard]] std::string GetILVersion() const;

  /*!
   * \brief Returns the ILs supported by the device, with version descriptors.
   * \return Vector of cl_name_version for supported ILs.
   */
  [[nodiscard]] std::vector<cl_name_version> GetILSWithVersion() const;

  /*!
   * \brief Returns the supported SPIR versions as a string list.
   * \return SPIR version list.
   */
  [[nodiscard]] std::string GetSpirVersions() const;

  /*!
   * \brief Returns command queue properties supported on the host side.
   * \return Host queue properties bitfield.
   */
  [[nodiscard]] cl_command_queue_properties GetQueueOnHostProperties() const;

  /*!
   * \brief Returns command queue properties supported on the device side.
   * \return Device queue properties bitfield.
   */
  [[nodiscard]] cl_command_queue_properties GetQueueOnDeviceProperties() const;

  /*!
   * \brief Returns the preferred size for device-side queues.
   * \return Preferred queue size.
   */
  [[nodiscard]] cl_uint GetQueueOnDevicePreferredSize() const;

  /*!
   * \brief Returns the maximum number of device-side queues supported.
   * \return Max queue count.
   */
  [[nodiscard]] cl_uint GetMaxOnDeviceQueues() const;

  /*!
   * \brief Returns the maximum number of device-side events supported.
   * \return Max event count.
   */
  [[nodiscard]] cl_uint GetMaxOnDeviceEvents() const;

  /*!
   * \brief Returns SVM (Shared Virtual Memory) capabilities.
   * \return SVM capability bitfield.
   */
  [[nodiscard]] cl_device_svm_capabilities GetSVMCapabilities() const;

  /*!
   * \brief Returns supported memory atomic capabilities.
   * \return Atomic memory capability bitfield.
   */
  [[nodiscard]] cl_device_atomic_capabilities
  GetAtomicMemoryCapabilities() const;

  /*!
   * \brief Returns supported fence atomic capabilities.
   * \return Atomic fence capability bitfield.
   */
  [[nodiscard]] cl_device_atomic_capabilities
  GetAtomicFenceCapabilities() const;

  /*!
   * \brief Returns the maximum number of sub-groups supported.
   * \return Max subgroup count.
   */
  [[nodiscard]] cl_uint GetMaxNumSubGroups() const;

  /*!
   * \brief Indicates if sub-groups support independent forward progress.
   * \return CL_TRUE if supported.
   */
  [[nodiscard]] cl_bool GetSubGroupIndependentForwardProgress() const;

  /*!
   * \brief Indicates whether work-groups may be non-uniform in size.
   * \return CL_TRUE if non-uniform work-groups are supported.
   */
  [[nodiscard]] cl_bool GetNonUniformWorkGroupSupport() const;

  /*!
   * \brief Indicates whether collective functions at work-group level are
   * supported.
   * \return CL_TRUE if supported.
   */
  [[nodiscard]] cl_bool GetWorkGroupCollectiveFunctionsSupport() const;

  /*!
   * \brief Indicates support for the generic address space.
   * \return CL_TRUE if supported.
   */
  [[nodiscard]] cl_bool GetGenericAddressSpaceSupport() const;

  /*!
   * \brief Returns capabilities for device-side enqueue.
   * \return Device enqueue capability bitfield.
   */
  [[nodiscard]] cl_device_device_enqueue_capabilities
  GetDeviceEnqueueCapabilities() const;

  /*!
   * \brief Returns execution capabilities (kernel execution, native
   * kernels...).
   * \return Execution capability bitfield.
   */
  [[nodiscard]] cl_device_exec_capabilities GetExecutionCapabilities() const;

  /*!
   * \brief Returns the reference count of the underlying OpenCL device object.
   * \return Reference count.
   */
  [[nodiscard]] cl_uint GetReferenceCount() const;

  /*!
   * \brief Returns the latest OpenCL conformance test version passed by the
   * device.
   * \return Conformance version string.
   */
  [[nodiscard]] std::string GetLastestConformanceVersionPassed() const;

  /*!
   * \brief Returns the maximum number of sub-devices that may be partitioned.
   * \return Max partitionable sub-device count.
   */
  [[nodiscard]] cl_uint GetPartitionMaxSubDevices() const;

  /*!
   * \brief Returns supported device partition properties.
   * \return Vector of partition properties.
   */
  [[nodiscard]] std::vector<cl_device_partition_property>
  GetPartitionProperties() const;

  /*!
   * \brief Returns supported affinity domains for device partitioning.
   * \return Affinity domain bitfield.
   */
  [[nodiscard]] cl_device_affinity_domain GetPartitionAffinityDomain() const;

  /*!
   * \brief Returns the partition type configuration used by this device.
   * \return Vector of partition type properties.
   */
  [[nodiscard]] std::vector<cl_device_partition_property>
  GetPartitionType() const;

  /*!
   * \brief Returns the extension string supported by the device.
   * \return Extension list string.
   */
  [[nodiscard]] std::string GetExtensions() const;

  /*!
   * \brief Returns all extensions with version descriptors.
   * \return Vector of cl_name_version extension descriptors.
   */
  [[nodiscard]] std::vector<cl_name_version> GetExtensionsWithVersion() const;

  /*!
   * \brief Returns built-in kernel names supported by the device.
   * \return Built-in kernel list.
   */
  [[nodiscard]] std::string GetBuiltInKernels() const;

  /*!
   * \brief Returns built-in kernels with associated versions.
   * \return Vector of cl_name_version descriptors.
   */
  [[nodiscard]] std::vector<cl_name_version>
  GetBuiltInKernelsWithVersion() const;

  /*!
   * \brief Returns the preferred atomic alignment for platform-wide scopes.
   * \return Alignment in bytes.
   */
  [[nodiscard]] cl_uint GetPreferredPlatformAtomicAlignment() const;

  /*!
   * \brief Returns preferred atomic alignment for global memory operations.
   * \return Alignment in bytes.
   */
  [[nodiscard]] cl_uint GetPreferredGlobalAtomicAlignment() const;

  /*!
   * \brief Returns preferred atomic alignment for local memory operations.
   * \return Alignment in bytes.
   */
  [[nodiscard]] cl_uint GetPreferredLocalAtomicAlignment() const;

  /*!
   * \brief Returns the device address width (32 or 64 bits).
   * \return Address bit width.
   */
  [[nodiscard]] cl_uint GetAddressBits() const;

  /*!
   * \brief Returns the device profiling timer resolution.
   * \return Resolution in nanoseconds.
   */
  [[nodiscard]] std::size_t GetProfilingTimerResolution() const;

  /*!
   * \brief Indicates whether the device compiler is available.
   * \return CL_TRUE if available.
   */
  [[nodiscard]] cl_bool GetCompilerAvailable() const;

  /*!
   * \brief Indicates whether the device linker is available.
   * \return CL_TRUE if available.
   */
  [[nodiscard]] cl_bool GetLinkerAvailable() const;

  /*!
   * \brief Indicates if the device is operational and available.
   * \return CL_TRUE if available.
   */
  [[nodiscard]] cl_bool GetAvailable() const;

  /*!
   * \brief Indicates whether the architecture is little-endian.
   * \return CL_TRUE if little-endian.
   */
  [[nodiscard]] cl_bool GetEndianLittle() const;

  /*!
   * \brief Indicates whether memory error correction is supported.
   * \return CL_TRUE if ECC is available.
   */
  [[nodiscard]] cl_bool GetErrorCorrectionSupport() const;

  /*!
   * \brief Returns maximum printf buffer size for kernels.
   * \return Buffer size in bytes.
   */
  [[nodiscard]] std::size_t GetPrintfBufferSize() const;

  /*!
   * \brief Indicates whether interop synchronisation with the host is
   * preferred.
   * \return CL_TRUE if preferred.
   */
  [[nodiscard]] cl_bool GetPreferredInteropUserSync() const;

  /*!
   * \brief Returns maximum number of pipe kernel arguments.
   * \return Max pipe args.
   */
  [[nodiscard]] cl_uint GetMaxPipeArgs() const;

  /*!
   * \brief Returns maximum number of active pipe reservations.
   * \return Max active reservations.
   */
  [[nodiscard]] cl_uint GetPipeMaxActiveReservations() const;

  /*!
   * \brief Returns maximum packet size for pipe operations.
   * \return Packet size in bytes.
   */
  [[nodiscard]] cl_uint GetPipeMaxPacketSize() const;

  /*!
   * \brief Indicates whether pipe objects are supported.
   * \return CL_TRUE if pipes are supported.
   */
  [[nodiscard]] cl_bool GetPipeSupport() const;

  /*!
   * \brief Returns the maximum size of a global variable that may be allocated.
   * \return Maximum global variable size in bytes.
   */
  [[nodiscard]] std::size_t GetMaxGlobalVariableSize() const;

  /*!
   * \brief Returns the preferred total size for all global variables.
   * \return Preferred total global variable memory size.
   */
  [[nodiscard]] std::size_t GetGlobalVariablePreferredTotalSize() const;

  /*!
   * \brief Returns the maximum size for a kernel argument list.
   * \return Maximum parameter size in bytes.
   */
  [[nodiscard]] std::size_t GetMaxParameterSize() const;

public:
  /*!
   * \brief Prints all device information grouped by feature category.
   *
   * This utility prints identity, compute capabilities, vector widths,
   * floating-point settings, memory information, image support, IL/SPIR-V
   * capabilities, queue features, atomics, partitioning information,
   * extensions, and general device properties.
   */
  void Print() const;

private:
  /*!
   * \brief Prints device identity information.
   *
   * Includes device name, vendor, driver version, OpenCL version, OpenCL C
   * versions, and (if supported) C++ for OpenCL numeric version.
   */
  void PrintIdentity() const;

  /*!
   * \brief Prints identifiers such as vendor ID, device type, and UUIDs.
   *
   * UUID and LUID information is included when extensions are available.
   */
  void PrintTypeID() const;

  /*!
   * \brief Prints compute-related limits: compute units, frequency, work-group
   * sizes.
   */
  void PrintCompute() const;

  /*!
   * \brief Prints preferred and native vector widths for all scalar types.
   */
  void PrintVectorisation() const;

  /*!
   * \brief Prints floating-point capability information (FP16, FP32, FP64).
   */
  void PrintFloatingPoint() const;

  /*!
   * \brief Prints memory-related information:
   * global memory, cache, local memory, constant buffers, alignment
   * constraints.
   */
  void PrintMemory() const;

  /*!
   * \brief Prints image-related capabilities (2D/3D dimensions, alignment,
   * samplers).
   */
  void PrintImages() const;

  /*!
   * \brief Prints IL and SPIR-V capability information when supported.
   */
  void PrintILSpirV() const;

  /*!
   * \brief Prints properties of command queues on host and device, SVM,
   * atomics, and sub-groups.
   */
  void PrintQueueDeviceSide() const;

  /*!
   * \brief Prints pipe-related capabilities.
   */
  void PrintPipe() const;

  /*!
   * \brief Prints device partitioning capabilities and configuration.
   */
  void PrintPartition() const;

  /*!
   * \brief Prints extension lists, built-in kernels, and miscellaneous
   * features.
   */
  void PrintExtensionsAndMisc() const;

private:
  cl::Device device_;          /*!< Underlying OpenCL device object. */
  std::size_t platform_index_; /*!< Parent platform index. */
  std::size_t device_index_;   /*!< Device index inside the platform. */
  std::unordered_set<std::string>
      extensions_; /*!< Supported OpenCL extensions. */
};
} // namespace ggems::ocl
