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
 * \brief Declaration of GGEMSOpenCLDevice class encapsulating a single OpenCL device.
 *
 * This header defines a C++23, move-only wrapper around a native `cl::Device`.
 * The class exposes a comprehensive set of query accessors for device capabilities,
 * memory limits, compute characteristics, and extension support. It is designed
 * to be owned by \c GGEMSOpenCLPlatform (one device per instance)
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-10-29
 * \copyright GNU General Public License v3.0
 * \version 2.0
 */

#include "GGEMS/frameworks/GGEMSOpenCLCommons.hh"

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
class GGEMSOpenCLDevice final {
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
  explicit GGEMSOpenCLDevice(cl::Device const& device, std::size_t platform_index, std::size_t device_index);

  GGEMSOpenCLDevice() = delete;
  GGEMSOpenCLDevice(GGEMSOpenCLDevice const&) = delete;
  GGEMSOpenCLDevice& operator=(GGEMSOpenCLDevice const&) = delete;

  /*!
   * \brief Destructor (defaulted).
   */
  ~GGEMSOpenCLDevice() = default;

  /*!
   * \brief Move constructor (no-throw).
   */
  GGEMSOpenCLDevice(GGEMSOpenCLDevice&&) noexcept = default;

  /*!
   * \brief Move assignment (no-throw).
   * \return reference to GGEMSOpenCLDevice
   */
  GGEMSOpenCLDevice& operator=(GGEMSOpenCLDevice&&) noexcept = default;

public: // ----- Identity & indices -----
  /*!
   * \brief Get the parent platform index.
   */
  [[nodiscard]] std::size_t GetPlatformIndex() const noexcept { return platform_index_; }

  /*!
   * \brief Get the device index within its platform.
   */
  [[nodiscard]] std::size_t GetDeviceIndex() const noexcept {return device_index_; }

  /*!
   * \brief Get the native OpenCL device handle.
   */
  [[nodiscard]] cl::Device const& GetNative() const noexcept { return device_; }

public: // ----- Identity properties -----
  [[nodiscard]] std::string GetName() const;

  [[nodiscard]] std::string GetVendor() const;

  [[nodiscard]] std::string GetVersion() const;

  [[nodiscard]] std::string GetDriverVersion() const;

  [[nodiscard]] std::string GetProfile() const;

  [[nodiscard]] std::string GetOpenCLCVersion() const;

  [[nodiscard]] cl_version GetNumericVersion() const;

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

  [[nodiscard]] std::size_t GetMaxBufferSize() const;

  [[nodiscard]] cl_uint GetMaxSamplers() const;

  [[nodiscard]] std::size_t GetGlobalMemSize() const;

  CL_DEVICE_GLOBAL_MEM_SIZE, CL_DEVICE_GLOBAL _MEM_CACHE_TYPE/LINE_SIZE/SIZE, CL_DEVICE_LOCAL_MEM_SIZE, CL_DEVICE_MAX_MEM_ALLOC_SIZE, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, CL_DEVICE_MAX_CONSTANT_ARGS, CL_DEVICE_MEM_BASE_ADDR_ALIGN, CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE

public:
  /*!
   * \brief Print a comprehensive, human-readable report for this device.
   *
   * The report includes vendor/name, type, compute units, memory sizes,
   * work-group limits, cache, extensions summary, and image capabilities.
   */
  void Print() const;

  /*!
   * \brief Convert a raw OpenCL device type flag into a human-readable string.
   * \param deviceType OpenCL device type bitfield (e.g. \c CL_DEVICE_TYPE_CPU, \c CL_DEVICE_TYPE_GPU).
   * \return Descriptive string representation of the given device type.
   *
   * This function translates an OpenCL device type enumeration value into a
   * readable string for logging or debugging purposes. The function supports
   * combined bitfield flags and returns concatenated names (e.g. "GPU | CPU").
   */
  [[nodiscard]] std::string DeviceTypeToString(cl_device_type deviceType) const;

  [[nodiscard]] std::string VendorIdToString(cl_uint vendor_id) const;

  [[nodiscard]] std::string ClBoolToString(cl_bool flag) const;

private:
  void PrintIdentity() const;
  void PrintTypeID() const;
  void PrintCompute() const;
  void PrintVectorisation() const;
  void PrintImages() const;

private:
  cl::Device  device_; /*!< Native OpenCL device handle */
  std::size_t platform_index_; /*!< Parent platform index */
  std::size_t device_index_; /*!< Device index within parent platform */
};
