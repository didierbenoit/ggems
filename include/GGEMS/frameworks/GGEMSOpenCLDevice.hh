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

public: // ----- String properties -----
  /*!
   * \brief Device human-readable name.
   */
  [[nodiscard]] std::string GetName() const;

  /*!
   * \brief Device vendor string.
   */
  [[nodiscard]] std::string GetVendor() const;

  /*!
   * \brief OpenCL device version string.
   */
  [[nodiscard]] std::string GetVersion() const;

public: // ----- Numeric identifiers & types -----
  /*!
   * \brief Vendor numeric identifier (implementation-defined).
   */
  [[nodiscard]] cl_uint GetVendorId() const;

  /*!
   * \brief Device type bitfield (e.g., \c CL_DEVICE_TYPE_GPU).
   */
  [[nodiscard]] cl_device_type GetType() const;

public: // ----- Images -----
  /*!
   * \brief Max 2D image width (pixels).
   */
  [[nodiscard]] std::size_t GetImage2DMaxWidth() const;

  /*!
   * \brief Max 2D image height (pixels).
   */
  [[nodiscard]] std::size_t GetImage2DMaxHeight() const;

  /*!
   * \brief Max 3D image width (voxels).
   */
  [[nodiscard]] std::size_t GetImage3DMaxWidth() const;

  /*!
   * \brief Max 3D image height (voxels).
   */
  [[nodiscard]] std::size_t GetImage3DMaxHeight() const;

  /*!
   * \brief Max 3D image depth (voxels).
   */
  [[nodiscard]] std::size_t GetImage3DMaxDepth() const;

public:
  /*!
   * \brief Print a comprehensive, human-readable report for this device.
   *
   * The report includes vendor/name, type, compute units, memory sizes,
   * work-group limits, cache, extensions summary, and image capabilities.
   */
  void Print() const;

  /*!
   * \brief Converts an OpenCL device type bitfield into a human-readable string.
   *
   * This function interprets the bits contained within a `cl_device_type`
   * value and returns a formatted string describing the corresponding
   * OpenCL device categories. Since `cl_device_type` is a bitfield, multiple
   * types may be active simultaneously (e.g. CPU and GPU). The returned string
   * lists all active types separated by a vertical bar `" | "`.
   *
   * The recognised OpenCL device types include:
   *   - `CL_DEVICE_TYPE_CPU` — General-purpose central processing unit.
   *   - `CL_DEVICE_TYPE_GPU` — Graphics processing unit suitable for parallel workloads.
   *   - `CL_DEVICE_TYPE_ACCELERATOR` — Dedicated accelerator such as an FPGA or DSP.
   *   - `CL_DEVICE_TYPE_CUSTOM` — Vendor-defined custom hardware.
   *   - `CL_DEVICE_TYPE_DEFAULT` — Default device as chosen by the OpenCL implementation.
   *   - `CL_DEVICE_TYPE_ALL` — All device types supported by the implementation.
   */
  [[nodiscard]] constexpr std::string DeviceTypeToString(cl_device_type deviceType) const;

private:
  cl::Device  device_; /*!< Native OpenCL device handle */
  std::size_t platform_index_; /*!< Parent platform index */
  std::size_t device_index_; /*!< Device index within parent platform */
};
