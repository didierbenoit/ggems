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
 * \file GGEMSOpenCLPlatform.hh
 * \brief Declaration of the GGEMSOpenCLPlatform class for OpenCL platform abstraction.
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-10-14
 * \copyright GNU General Public License v3.0
 * \version 2.0
 *
 * This header defines the GGEMSOpenCLPlatform class which provides a
 * high-level abstraction of an OpenCL platform, allowing comprehensive
 * querying of platform properties, extensions, versions, and attached devices.
 */

#include "GGEMS/frameworks/GGEMSOpenCLCommons.hh"

class GGEMSOpenCLDevice;

/*!
 * \class GGEMSOpenCLPlatform
 * \brief Encapsulates a single OpenCL platform and its properties.
 *
 * This class represents a native OpenCL platform and provides
 * convenient methods to query platform-specific information, including:
 * - Name, vendor, version, and profile
 * - Supported extensions (with or without version information)
 * - Host timer resolution
 * - ICD loader suffix
 *
 * Each platform instance is responsible for enumerating and holding
 * its associated OpenCL devices (CPU and GPU) through
 * \ref DiscoverDevices.
 */
class GGEMSOpenCLPlatform final {
public:
  /*!
   * \brief Constructs a GGEMSOpenCLPlatform from a native OpenCL platform.
   * \param platform The native OpenCL platform object (cl::Platform).
   * \param platform_index Unique index identifying the platform within the system.
   *
   * The constructor stores the underlying OpenCL handle and prepares
   * the object for further device discovery and information retrieval.
   */
  explicit GGEMSOpenCLPlatform(cl::Platform const& platform, std::size_t platform_index);

  /*!
   * \brief Default constructor deleted.
   * \note Platforms must be constructed explicitly with a valid OpenCL handle.
   */
  GGEMSOpenCLPlatform() = delete;

  /*!
   * \brief Destructor.
   * \note Automatically cleans platform internals; no explicit action required.
   */
  ~GGEMSOpenCLPlatform();

  GGEMSOpenCLPlatform(GGEMSOpenCLPlatform const&) = delete;
  GGEMSOpenCLPlatform& operator=(GGEMSOpenCLPlatform const&) = delete;

  /*!
   * \brief Move constructor (no-throw).
   */
  GGEMSOpenCLPlatform(GGEMSOpenCLPlatform&&) noexcept = default;

  /*!
   * \brief Move assignment (no-throw).
   * \return Reference to GGEMSOpenCLPlatform
   */
  GGEMSOpenCLPlatform& operator=(GGEMSOpenCLPlatform&&) noexcept = default;

public:
  /*!
   * \brief Checks whether a specific OpenCL extension is supported.
   * \param extension_name Name of the extension to test.
   * \return True if the extension is supported, false otherwise.
   */
  [[nodiscard]] bool CheckExtension(std::string_view extension_name) const;

  /*!
   * \brief Retrieves the platform’s human-readable name.
   * \return Name of the OpenCL platform.
   */
  [[nodiscard]] std::string GetName() const;

  /*!
   * \brief Retrieves the supported OpenCL profile string.
   * \return Typically "FULL_PROFILE" or "EMBEDDED_PROFILE".
   */
  [[nodiscard]] std::string GetProfile() const;

  /*!
   * \brief Retrieves the OpenCL version string.
   * \return Version string formatted as "OpenCL <major>.<minor> <vendor info>".
   */
  [[nodiscard]] std::string GetVersion() const;

  /*!
   * \brief Retrieves the platform vendor string.
   * \return Vendor name.
   */
  [[nodiscard]] std::string GetVendor() const;

  /*!
   * \brief Retrieves a space-separated list of platform extensions.
   * \return Space-separated string of extension names.
   */
  [[nodiscard]] std::string GetExtensions() const;

  /*!
   * \brief Retrieves the platform’s numeric version.
   * \return The cl_version structure describing (major, minor, patch).
   */
  [[nodiscard]] cl_version GetNumericVersion() const;

  /*!
   * \brief Retrieves the host timer resolution.
   * \return Timer resolution in nanoseconds.
   */
  [[nodiscard]] cl_ulong GetHostTimerResolution() const;

  /*!
   * \brief Retrieves platform extensions along with their version numbers.
   * \return A vector of cl_name_version elements.
   */
  [[nodiscard]] std::vector<cl_name_version> GetExtensionsWithVersion() const;

  /*!
   * \brief Retrieves the ICD loader suffix for this platform.
   * \return ICD suffix string (usually vendor-specific).
   */
  [[nodiscard]] std::string GetIcdSuffixKhr() const;

  /*!
   * \brief Template method for querying raw platform information arrays.
   * \tparam T Data type of returned elements.
   * \param param The OpenCL platform info parameter to query.
   * \return A vector of type T containing the requested data.
   */
  template <typename T>
  [[nodiscard]] std::vector<T> GetPlatformInfoArray(cl_platform_info const& param) const {
    std::size_t size = 0;
    GGOCL_CHECK(::clGetPlatformInfo(platform_(), param, 0, nullptr, &size))
    std::vector<T> result(size / sizeof(T));
    GGOCL_CHECK(::clGetPlatformInfo(platform_(), param, size, result.data(), nullptr));
    return result;
  }

  /*!
   * \brief Get a constant view of all devices discovered on this platform.
   * \return A vector of raw pointers to `GGEMSOpenCLDevice` objects (non-owning).
   *
   * The returned pointers are valid as long as the `GGEMSOpenCLPlatform`
   * instance remains alive. They must not be deleted or modified.
   */
  [[nodiscard]] std::vector<GGEMSOpenCLDevice const*> GetDevices() const;

  /*!
   * \brief Retrieve the index of this OpenCL platform within the system.
   * \return The unique index assigned during platform enumeration.
   *
   * This index corresponds to the order of discovery in the system’s
   * OpenCL platform list. It is primarily used for debugging or mapping
   * user selections to specific platforms.
   */
  [[nodiscard]] constexpr std::size_t GetPlatformIndex() const noexcept { return platform_index_; }

  /*!
   * \brief Retrieve the underlying native OpenCL platform object.
   * \return Constant reference to the native \c cl::Platform object.
   *
   * This method allows interoperability with raw OpenCL APIs or
   * external libraries that require access to the native handle.
   * It should be used with care, as modifying the returned object
   * directly can lead to inconsistent internal state.
   */
  [[nodiscard]] cl::Platform const& GetNative() const noexcept { return platform_; }

  /*!
   * \brief Prints a summary of platform properties to the terminal.
   *
   * Information includes name, vendor, version, extensions, and numeric version.
   */
  void Print() const;

  /*!
   * \brief Releases any internal OpenCL compiler or cached resource.
   */
  void Clean();

private:
  /*!
   * \brief Enumerates all OpenCL devices (CPU and GPU) for this platform.
   */
  void DiscoverDevices();

private:
  cl::Platform                                    platform_; /*!< Native OpenCL platform object */
  std::size_t                                     platform_index_; /*!< Index of this platform in the system */
  std::vector<std::unique_ptr<GGEMSOpenCLDevice>> devices_; /*!< Devices belonging to this platform */
};
