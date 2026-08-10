#pragma once
// ************************************************************************
// ************************************************************************


#include "GGEMS/frameworks/GGEMSOpenCLUtils.hh"

namespace ggems::ocl {
class GGEMSOpenCLDevice {
public:
  explicit GGEMSOpenCLDevice(cl::Device const &device,
                             std::size_t platform_index,
                             std::size_t device_index);

  GGEMSOpenCLDevice() = delete;

  ~GGEMSOpenCLDevice() = default;

  GGEMSOpenCLDevice(GGEMSOpenCLDevice const &) = delete;
  GGEMSOpenCLDevice &operator=(GGEMSOpenCLDevice const &) = delete;
  GGEMSOpenCLDevice &operator=(GGEMSOpenCLDevice &&) noexcept = delete;

  GGEMSOpenCLDevice(GGEMSOpenCLDevice &&) noexcept = default;

public:
  [[nodiscard]] std::size_t GetPlatformIndex() const noexcept {
    return platform_index_;
  }

  [[nodiscard]] std::size_t GetDeviceIndex() const noexcept {
    return device_index_;
  }

  [[nodiscard]] std::unordered_set<std::string> const &
  GetDeviceExtensions() const noexcept {
    return extensions_;
  }

  [[nodiscard]] cl::Device const &GetDeviceNative() const noexcept {
    return device_;
  }

  [[nodiscard]] cl_platform_id GetPlatformID() const;

public:
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

  [[nodiscard]] std::array<cl_uchar, CL_LUID_SIZE_KHR> GetLUIDKhr() const;

  [[nodiscard]] cl_uint GetVendorId() const;

  [[nodiscard]] cl_device_type GetType() const;

  [[nodiscard]] cl_uint GetMaxComputeUnits() const;

  [[nodiscard]] cl_uint GetMaxClockFrequency() const;

  [[nodiscard]] std::size_t GetMaxWorkGroupSize() const;

  [[nodiscard]] cl_uint GetMaxWorkItemDimensions() const;

  [[nodiscard]] std::vector<std::size_t> GetMaxWorkItemSizes() const;

  [[nodiscard]] std::size_t GetPreferredWorkGroupSizeMultiple() const;

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

  [[nodiscard]] std::string GetILVersion() const;

  [[nodiscard]] std::vector<cl_name_version> GetILSWithVersion() const;

  [[nodiscard]] std::string GetSpirVersions() const;

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

  [[nodiscard]] cl_uint GetPartitionMaxSubDevices() const;

  [[nodiscard]] std::vector<cl_device_partition_property>
  GetPartitionProperties() const;

  [[nodiscard]] cl_device_affinity_domain GetPartitionAffinityDomain() const;

  [[nodiscard]] std::vector<cl_device_partition_property>
  GetPartitionType() const;

  [[nodiscard]] std::string GetExtensions() const;

  [[nodiscard]] std::vector<cl_name_version> GetExtensionsWithVersion() const;

  [[nodiscard]] std::string GetBuiltInKernels() const;

  [[nodiscard]] std::vector<cl_name_version>
  GetBuiltInKernelsWithVersion() const;

  [[nodiscard]] cl_uint GetPreferredPlatformAtomicAlignment() const;

  [[nodiscard]] cl_uint GetPreferredGlobalAtomicAlignment() const;

  [[nodiscard]] cl_uint GetPreferredLocalAtomicAlignment() const;

  [[nodiscard]] cl_uint GetAddressBits() const;

  [[nodiscard]] std::size_t GetProfilingTimerResolution() const;

  [[nodiscard]] cl_bool GetCompilerAvailable() const;

  [[nodiscard]] cl_bool GetLinkerAvailable() const;

  [[nodiscard]] cl_bool GetAvailable() const;

  [[nodiscard]] cl_bool GetEndianLittle() const;

  [[nodiscard]] cl_bool GetErrorCorrectionSupport() const;

  [[nodiscard]] std::size_t GetPrintfBufferSize() const;

  [[nodiscard]] cl_bool GetPreferredInteropUserSync() const;

  [[nodiscard]] cl_uint GetMaxPipeArgs() const;

  [[nodiscard]] cl_uint GetPipeMaxActiveReservations() const;

  [[nodiscard]] cl_uint GetPipeMaxPacketSize() const;

  [[nodiscard]] cl_bool GetPipeSupport() const;

  [[nodiscard]] std::size_t GetMaxGlobalVariableSize() const;

  [[nodiscard]] std::size_t GetGlobalVariablePreferredTotalSize() const;

  [[nodiscard]] std::size_t GetMaxParameterSize() const;

public:
  void Print() const;

private:
  void PrintIdentity() const;

  void PrintTypeID() const;

  void PrintCompute() const;

  void PrintVectorization() const;

  void PrintFloatingPoint() const;

  void PrintMemory() const;

  void PrintImages() const;

  void PrintILSpirV() const;

  void PrintQueueDeviceSide() const;

  void PrintPipe() const;

  void PrintPartition() const;

  void PrintExtensionsAndMisc() const;

private:
  cl::Device device_;
  std::size_t platform_index_;
  std::size_t device_index_;
  std::unordered_set<std::string>
      extensions_;
};
} // namespace ggems::ocl
