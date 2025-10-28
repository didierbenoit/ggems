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
 * \brief Definition of GGEMSOpenCL class
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-10-14
 * \copyright GNU General Public License v3.0
 * \version 2.0
 */

#include "GGEMS/frameworks/GGEMSOpenCLCommons.hh"

/*!
 * \class GGEMSOpenCLPlatform
 * \brief GGEMSOpenCLPlaform class storing an OpenCL platform
 */
class GGEMSOpenCLPlatform {
public:
  /*!
   * \fn GGEMSOpenCLPlatform(cl::Platform const& platform, std::size_t platform_index)
   * \param platform - OpenCL plaform
   * \param platform_index - Index of the platform
   * \brief Constructor of GGEMSOpenCLPlatform
   */
  GGEMSOpenCLPlatform(cl::Platform const& platform, std::size_t platform_index);

  /*!
   * \fn GGEMSOpenCLPlatform()
   * \brief Default constructor of GGEMSOpenCLPlatform deleted
   */
  GGEMSOpenCLPlatform() = delete;

  /*!
   * \fn ~GGEMSOpenCLPlatform()
   * \brief Default constructor of GGEMSOpenCLPlatform deleted
   */
  ~GGEMSOpenCLPlatform();

public:
  /*!
   * \fn bool CheckExtension(std::string_view extension_name) const
   * \param extension_name - Name of the extension
   * \brief Check if extension_name exists in list of extension
   * \return Return true if the extension exists
   */
  bool CheckExtension(std::string_view extension_name) const;

  /*!
   * \fn std::string GetName() const
   * \brief Return the OpenCL platform name
   * \return Get the OpenCL platform name
   */
  std::string GetName() const;

  /*!
   * \fn std::string GetProfile() const
   * \brief OpenCL profile string
   * \return Returns the profile name supported by the implementation
   */
  std::string GetProfile() const;

  /*!
   * \fn std::string GetVersion() const
   * \brief OpenCL version string
   * \return Returns the OpenCL version supported by the implementation
   */
  std::string GetVersion() const;

  /*!
   * \fn std::string GetVendor() const
   * \brief OpenCL vendor string
   * \return Platform vendor string.
   */
  std::string GetVendor() const;

  /*!
   * \fn std::string GetExtensions() const
   * \brief OpenCL extensions string
   * \return Returns a space separated list of extension names supported by the platform
   */
  std::string GetExtensions() const;

  /*!
   * \fn cl_version GetNumericVersion() const
   * \brief OpenCL numeric version
   * \return Returns the detailed (major, minor, patch) version supported by the platform
   */
  cl_version GetNumericVersion() const;

  /*!
   * \fn cl_ulong GetHostTimerResolution() const
   * \brief OpenCL host timer resolution
   * \return Returns the resolution of the host timer in nanoseconds
   */
  cl_ulong GetHostTimerResolution() const;

  /*!
   * \fn std::vector<cl_name_version> GetExtensionsWithVersion() const
   * \brief OpenCL extensions with version
   * \return Returns an array of description (name and version) structures that lists all the extensions supported by the platform
   */
  std::vector<cl_name_version> GetExtensionsWithVersion() const;

  /*!
   * \fn std::string GetIcdSuffixKhr() const
   * \brief The function name suffix used to identify extension functions to be directed to this platform by the ICD Loader
   * \return Returns the function name suffix
   */
  std::string GetIcdSuffixKhr() const;

  /*!
   * \fn template <typename T> std::vector<T> GetPlatformInfoArray(cl_platform_info const& param) const
   * \tparam T - Type of array
   * \param param - Type of platform parameter
   * \brief Get the platform params in a vector
   * \return Return a vector with a list of params
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
   * \fn void Print() const
   * \brief Print all infos about OpenCL platform to the screen
   */
  void Print() const;

  /*!
   * \fn void Clean()
   * \brief Explicitly releases the internal compilers of the platform
   */
  void Clean();

private:
  cl::Platform platform_; /*!< OpenCL platform */
  std::size_t platform_index_; /*!< Index of the platform */
};
