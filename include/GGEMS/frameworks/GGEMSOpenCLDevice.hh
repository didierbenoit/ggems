#pragma once

#include <array>
#include <cstddef>
#include <string>
#include <unordered_set>
#include <vector>

#include "GGEMS/frameworks/GGEMSOpenCLUtils.hh"

namespace ggems::ocl {
class GGEMSOpenCLDevice {
public:
  explicit GGEMSOpenCLDevice(cl::Device device, std::size_t platform_index,
                             std::size_t device_index);

  GGEMSOpenCLDevice() = delete;

  ~GGEMSOpenCLDevice() = default;

  GGEMSOpenCLDevice(GGEMSOpenCLDevice const &) = delete;
  auto operator=(GGEMSOpenCLDevice const &) -> GGEMSOpenCLDevice & = delete;
  auto operator=(GGEMSOpenCLDevice &&) noexcept -> GGEMSOpenCLDevice & = delete;

  GGEMSOpenCLDevice(GGEMSOpenCLDevice &&) noexcept = default;

  [[nodiscard]] auto GetPlatformIndex() const noexcept -> std::size_t {
    return platform_index_;
  }

  [[nodiscard]] auto GetDeviceIndex() const noexcept -> std::size_t {
    return device_index_;
  }

  [[nodiscard]] auto GetDeviceExtensions() const noexcept
      -> std::unordered_set<std::string> const & {
    return extensions_;
  }

  [[nodiscard]] auto GetDeviceNative() const noexcept -> cl::Device const & {
    return device_;
  }

  [[nodiscard]] auto GetPlatformID() const -> cl_platform_id;

  [[nodiscard]] auto GetName() const -> std::string;

  [[nodiscard]] auto GetVendor() const -> std::string;

  [[nodiscard]] auto GetVersion() const -> std::string;

  [[nodiscard]] auto GetDriverVersion() const -> std::string;

  [[nodiscard]] auto GetProfile() const -> std::string;

  [[nodiscard]] auto GetOpenCLCVersion() const -> std::string;

  [[nodiscard]] auto GetOpenCLCAllVersions() const
      -> std::vector<cl_name_version>;

  [[nodiscard]] auto GetOpenCLCNumericVersionKhr() const -> cl_version_khr;

  [[nodiscard]] auto GetOpenCLCFeatures() const -> std::vector<cl_name_version>;

  [[nodiscard]] auto GetCxxForOpenCLNumericVersionExt() const -> cl_version;

  [[nodiscard]] auto GetNumericVersion() const -> cl_version;

  [[nodiscard]] auto GetUUIDKhr() const -> std::string;

  [[nodiscard]] auto GetDriverUUIDKhr() const -> std::string;

  [[nodiscard]] auto GetLUIDValidKhr() const -> cl_bool;

  [[nodiscard]] auto GetLUIDKhr() const
      -> std::array<cl_uchar, CL_LUID_SIZE_KHR>;

  [[nodiscard]] auto GetVendorId() const -> cl_uint;

  [[nodiscard]] auto GetType() const -> cl_device_type;

  [[nodiscard]] auto GetMaxComputeUnits() const -> cl_uint;

  [[nodiscard]] auto GetMaxClockFrequency() const -> cl_uint;

  [[nodiscard]] auto GetMaxWorkGroupSize() const -> std::size_t;

  [[nodiscard]] auto GetMaxWorkItemDimensions() const -> cl_uint;

  [[nodiscard]] auto GetMaxWorkItemSizes() const -> std::vector<std::size_t>;

  [[nodiscard]] auto GetPreferredWorkGroupSizeMultiple() const -> std::size_t;

  [[nodiscard]] auto GetPreferredVectorWidthChar() const -> cl_uint;

  [[nodiscard]] auto GetPreferredVectorWidthShort() const -> cl_uint;

  [[nodiscard]] auto GetPreferredVectorWidthInt() const -> cl_uint;

  [[nodiscard]] auto GetPreferredVectorWidthLong() const -> cl_uint;

  [[nodiscard]] auto GetPreferredVectorWidthFloat() const -> cl_uint;

  [[nodiscard]] auto GetPreferredVectorWidthDouble() const -> cl_uint;

  [[nodiscard]] auto GetPreferredVectorWidthHalf() const -> cl_uint;

  [[nodiscard]] auto GetNativeVectorWidthChar() const -> cl_uint;

  [[nodiscard]] auto GetNativeVectorWidthShort() const -> cl_uint;

  [[nodiscard]] auto GetNativeVectorWidthInt() const -> cl_uint;

  [[nodiscard]] auto GetNativeVectorWidthLong() const -> cl_uint;

  [[nodiscard]] auto GetNativeVectorWidthFloat() const -> cl_uint;

  [[nodiscard]] auto GetNativeVectorWidthDouble() const -> cl_uint;

  [[nodiscard]] auto GetNativeVectorWidthHalf() const -> cl_uint;

  [[nodiscard]] auto GetHalfFpConfig() const -> cl_device_fp_config;

  [[nodiscard]] auto GetSingleFpConfig() const -> cl_device_fp_config;

  [[nodiscard]] auto GetDoubleFpConfig() const -> cl_device_fp_config;

  [[nodiscard]] auto GetImageSupport() const -> cl_bool;

  [[nodiscard]] auto GetImage2DMaxWidth() const -> std::size_t;

  [[nodiscard]] auto GetImage2DMaxHeight() const -> std::size_t;

  [[nodiscard]] auto GetImage3DMaxWidth() const -> std::size_t;

  [[nodiscard]] auto GetImage3DMaxHeight() const -> std::size_t;

  [[nodiscard]] auto GetImage3DMaxDepth() const -> std::size_t;

  [[nodiscard]] auto GetImageMaxBufferSize() const -> std::size_t;

  [[nodiscard]] auto GetImageMaxArraySize() const -> std::size_t;

  [[nodiscard]] auto GetMaxReadImageArgs() const -> cl_uint;

  [[nodiscard]] auto GetMaxWriteImageArgs() const -> cl_uint;

  [[nodiscard]] auto GetMaxReadWriteImageArgs() const -> cl_uint;

  [[nodiscard]] auto GetImagePitchAlignment() const -> cl_uint;

  [[nodiscard]] auto GetImageBaseAddressAlignment() const -> cl_uint;

  [[nodiscard]] auto GetMaxSamplers() const -> cl_uint;

  [[nodiscard]] auto GetGlobalMemSize() const -> cl_ulong;

  [[nodiscard]] auto GetGlobalMemCacheType() const -> cl_device_mem_cache_type;

  [[nodiscard]] auto GetGlobalMemCacheLineSize() const -> cl_uint;

  [[nodiscard]] auto GetGlobalMemCacheSize() const -> cl_ulong;

  [[nodiscard]] auto GetLocalMemSize() const -> cl_ulong;

  [[nodiscard]] auto GetLocalMemType() const -> cl_device_local_mem_type;

  [[nodiscard]] auto GetMaxMemAllocSize() const -> cl_ulong;

  [[nodiscard]] auto GetMaxConstantBufferSize() const -> cl_ulong;

  [[nodiscard]] auto GetMaxConstantArgs() const -> cl_uint;

  [[nodiscard]] auto GetMemBaseAddrAlign() const -> cl_uint;

  [[nodiscard]] auto GetMinDataTypeAlignSize() const -> cl_uint;

  [[nodiscard]] auto GetHostUnifiedMemory() const -> cl_bool;

  [[nodiscard]] auto GetILVersion() const -> std::string;

  [[nodiscard]] auto GetILSWithVersion() const -> std::vector<cl_name_version>;

  [[nodiscard]] auto GetSpirVersions() const -> std::string;

  [[nodiscard]] auto GetQueueOnHostProperties() const
      -> cl_command_queue_properties;

  [[nodiscard]] auto GetQueueOnDeviceProperties() const
      -> cl_command_queue_properties;

  [[nodiscard]] auto GetQueueOnDevicePreferredSize() const -> cl_uint;

  [[nodiscard]] auto GetMaxOnDeviceQueues() const -> cl_uint;

  [[nodiscard]] auto GetMaxOnDeviceEvents() const -> cl_uint;

  [[nodiscard]] auto GetSVMCapabilities() const -> cl_device_svm_capabilities;

  [[nodiscard]] auto GetAtomicMemoryCapabilities() const
      -> cl_device_atomic_capabilities;

  [[nodiscard]] auto GetAtomicFenceCapabilities() const
      -> cl_device_atomic_capabilities;

  [[nodiscard]] auto GetMaxNumSubGroups() const -> cl_uint;

  [[nodiscard]] auto GetSubGroupIndependentForwardProgress() const -> cl_bool;

  [[nodiscard]] auto GetNonUniformWorkGroupSupport() const -> cl_bool;

  [[nodiscard]] auto GetWorkGroupCollectiveFunctionsSupport() const -> cl_bool;

  [[nodiscard]] auto GetGenericAddressSpaceSupport() const -> cl_bool;

  [[nodiscard]] auto GetDeviceEnqueueCapabilities() const
      -> cl_device_device_enqueue_capabilities;

  [[nodiscard]] auto GetExecutionCapabilities() const
      -> cl_device_exec_capabilities;

  [[nodiscard]] auto GetReferenceCount() const -> cl_uint;

  [[nodiscard]] auto GetLastestConformanceVersionPassed() const -> std::string;

  [[nodiscard]] auto GetPartitionMaxSubDevices() const -> cl_uint;

  [[nodiscard]] auto GetPartitionProperties() const
      -> std::vector<cl_device_partition_property>;

  [[nodiscard]] auto GetPartitionAffinityDomain() const
      -> cl_device_affinity_domain;

  [[nodiscard]] auto GetPartitionType() const
      -> std::vector<cl_device_partition_property>;

  [[nodiscard]] auto GetExtensions() const -> std::string;

  [[nodiscard]] auto GetExtensionsWithVersion() const
      -> std::vector<cl_name_version>;

  [[nodiscard]] auto GetBuiltInKernels() const -> std::string;

  [[nodiscard]] auto GetBuiltInKernelsWithVersion() const
      -> std::vector<cl_name_version>;

  [[nodiscard]] auto GetPreferredPlatformAtomicAlignment() const -> cl_uint;

  [[nodiscard]] auto GetPreferredGlobalAtomicAlignment() const -> cl_uint;

  [[nodiscard]] auto GetPreferredLocalAtomicAlignment() const -> cl_uint;

  [[nodiscard]] auto GetAddressBits() const -> cl_uint;

  [[nodiscard]] auto GetProfilingTimerResolution() const -> std::size_t;

  [[nodiscard]] auto GetCompilerAvailable() const -> cl_bool;

  [[nodiscard]] auto GetLinkerAvailable() const -> cl_bool;

  [[nodiscard]] auto GetAvailable() const -> cl_bool;

  [[nodiscard]] auto GetEndianLittle() const -> cl_bool;

  [[nodiscard]] auto GetErrorCorrectionSupport() const -> cl_bool;

  [[nodiscard]] auto GetPrintfBufferSize() const -> std::size_t;

  [[nodiscard]] auto GetPreferredInteropUserSync() const -> cl_bool;

  [[nodiscard]] auto GetMaxPipeArgs() const -> cl_uint;

  [[nodiscard]] auto GetPipeMaxActiveReservations() const -> cl_uint;

  [[nodiscard]] auto GetPipeMaxPacketSize() const -> cl_uint;

  [[nodiscard]] auto GetPipeSupport() const -> cl_bool;

  [[nodiscard]] auto GetMaxGlobalVariableSize() const -> std::size_t;

  [[nodiscard]] auto GetGlobalVariablePreferredTotalSize() const -> std::size_t;

  [[nodiscard]] auto GetMaxParameterSize() const -> std::size_t;

  auto Print() const -> void;

private:
  auto PrintIdentity() const -> void;

  auto PrintTypeID() const -> void;

  auto PrintCompute() const -> void;

  auto PrintVectorization() const -> void;

  auto PrintFloatingPoint() const -> void;

  auto PrintMemory() const -> void;

  auto PrintImages() const -> void;

  auto PrintILSpirV() const -> void;

  auto PrintQueueDeviceSide() const -> void;

  auto PrintPipe() const -> void;

  auto PrintPartition() const -> void;

  auto PrintExtensionsAndMisc() const -> void;

  cl::Device device_;
  std::size_t platform_index_;
  std::size_t device_index_;
  std::unordered_set<std::string> extensions_;
};
} // namespace ggems::ocl
