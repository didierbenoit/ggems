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
 * comprehensive abstraction of an OpenCL platform, allowing querying
 * of platform information, extensions, versions, and host timer resolution.
 */

#include "GGEMS/frameworks/GGEMSOpenCLCommons.hh"

/*!
 * \class GGEMSOpenCLPlatform
 * \brief Encapsulates a single OpenCL platform and its properties.
 *
 * This class represents a native OpenCL platform and provides
 * convenient methods to query platform-specific information, including:
 * - Name, vendor, version and profile
 * - Supported extensions (with optional version information)
 * - Host timer resolution
 * - ICD loader function suffix
 * 
 * It serves as a container for platform-level OpenCL operations and
 * acts as a factory for devices belonging to the platform.
 */
class GGEMSOpenCLPlatform {
public:
  /*!
   * \brief Constructs a GGEMSOpenCLPlatform from a native OpenCL platform.
   * \param platform The native OpenCL platform object (cl::Platform).
   * \param platform_index Unique index identifying the platform within the system.
   *
   * Initializes internal structures to manage platform queries and devices.
   */
  GGEMSOpenCLPlatform(cl::Platform const& platform, std::size_t platform_index);

  /*!
   * \brief Default constructor deleted.
   * 
   * Ensures that a platform object cannot be default-constructed without
   * providing a valid native cl::Platform reference and index.
   */
  GGEMSOpenCLPlatform() = delete;

  /*!
   * \brief Destructor for GGEMSOpenCLPlatform.
   * 
   * Releases any internal resources associated with the platform if necessary.
   */
  ~GGEMSOpenCLPlatform();

public:
  /*!
   * \brief Checks whether a specific OpenCL extension is supported by the platform.
   * \param extension_name Name of the extension to query.
   * \return True if the extension is supported, false otherwise.
   */
  bool CheckExtension(std::string_view extension_name) const;

  /*!
   * \brief Retrieves the platform's human-readable name.
   * \return Name of the OpenCL platform.
   */
  std::string GetName() const;

  /*!
   * \brief Retrieves the platform's supported profile.
   * \return Profile string, typically "FULL_PROFILE" or "EMBEDDED_PROFILE".
   */
  std::string GetProfile() const;

  /*!
   * \brief Retrieves the OpenCL version supported by the platform.
   * \return Version string in the format "OpenCL <major>.<minor> <vendor-specific info>".
   */
  std::string GetVersion() const;

  /*!
   * \brief Retrieves the platform vendor name.
   * \return Vendor string identifying the platform provider.
   */
  std::string GetVendor() const;

  /*!
   * \brief Retrieves the list of supported platform extensions as a space-separated string.
   * \return Space-separated string of all extensions supported by the platform.
   */
  std::string GetExtensions() const;

  /*!
   * \brief Retrieves the platform's numeric version.
   * \return A cl_version structure detailing major, minor, and patch levels.
   */
  cl_version GetNumericVersion() const;

  /*!
   * \brief Retrieves the host timer resolution.
   * \return Resolution of the host timer in nanoseconds.
   */
  cl_ulong GetHostTimerResolution() const;

  /*!
   * \brief Retrieves all platform extensions along with their version numbers.
   * \return Vector of cl_name_version structures describing supported extensions.
   */
  std::vector<cl_name_version> GetExtensionsWithVersion() const;

  /*!
   * \brief Retrieves the function name suffix used by the ICD loader for this platform.
   * \return ICD suffix string.
   */
  std::string GetIcdSuffixKhr() const;

  /*!
   * \brief Template function to retrieve arbitrary platform info arrays.
   * \tparam T Data type of the array elements.
   * \param param Platform information parameter to query (cl_platform_info).
   * \return Vector of type T containing the requested information.
   *
   * This method automatically resizes the result vector to hold all returned data.
   */
  template <typename T>
  std::vector<T> GetPlatformInfoArray(cl_platform_info const& param) const {
    std::size_t size = 0;
    GGOCL_ERROR(::clGetPlatformInfo(platform_(), param, 0, nullptr, &size))
    std::vector<T> result(size / sizeof(T));
    GGOCL_ERROR(::clGetPlatformInfo(platform_(), param, size, result.data(), nullptr));
    return result;
  }

  /*!
   * \brief Prints all platform information to the standard output.
   *
   * This includes name, vendor, version, profile, supported extensions,
   * and other platform-level details.
   */
  void Print() const;

  /*!
   * \brief Explicitly releases any internal OpenCL compilers or resources.
   *
   * Useful for cleaning up platform-level objects before application shutdown.
   */
  void Clean();

private:
  cl::Platform platform_; /*!< Native OpenCL platform object */
  std::size_t platform_index_; /*!< Index of this platform within the system */
};
