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
 * \brief Declares the GGEMS OpenCL platform wrapper.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <string>
#include <vector>
#include <cstddef>
/// \endcond

#include "GGEMS/opencl/GGEMSOpenCLExternal.hh"

namespace ggems::ocl {

class GGEMSOpenCLDevice;

/*!
 * \brief Represents one OpenCL platform and its discovered CPU/GPU devices.
 */
class GGEMSOpenCLPlatform {
public:
  /*!
   * \brief Constructs an OpenCL platform wrapper and discovers its devices.
   *
   * \param[in] platform Native OpenCL platform.
   * \param[in] platform_index GGEMS platform index.
   */
  explicit GGEMSOpenCLPlatform(cl::Platform platform,
                               std::size_t platform_index);

  /*!
   * \brief Disables default construction.
   */
  GGEMSOpenCLPlatform() = delete;

  /*!
   * \brief Destroys the OpenCL platform wrapper.
   */
  ~GGEMSOpenCLPlatform();

  /*!
   * \brief Disables copy construction.
   */
  GGEMSOpenCLPlatform(GGEMSOpenCLPlatform const &) = delete;

  /*!
   * \brief Disables copy assignment.
   */
  auto operator=(GGEMSOpenCLPlatform const &) -> GGEMSOpenCLPlatform & = delete;

  /*!
   * \brief Move-constructs an OpenCL platform wrapper.
   */
  GGEMSOpenCLPlatform(GGEMSOpenCLPlatform &&) noexcept = default;

  /*!
   * \brief Disables move assignment.
   */
  auto operator=(GGEMSOpenCLPlatform &&) -> GGEMSOpenCLPlatform & = delete;

  /*!
   * \brief Returns the OpenCL platform name.
   *
   * \return OpenCL platform name.
   */
  [[nodiscard]] auto GetName() const -> std::string;

  /*!
   * \brief Returns the OpenCL platform profile.
   *
   * \return OpenCL platform profile.
   */
  [[nodiscard]] auto GetProfile() const -> std::string;

  /*!
   * \brief Returns the OpenCL platform version.
   *
   * \return OpenCL platform version.
   */
  [[nodiscard]] auto GetVersion() const -> std::string;

  /*!
   * \brief Returns the OpenCL platform vendor.
   *
   * \return OpenCL platform vendor.
   */
  [[nodiscard]] auto GetVendor() const -> std::string;

  /*!
   * \brief Returns the OpenCL platform extension string.
   *
   * \return OpenCL platform extension string.
   */
  [[nodiscard]] auto GetExtensions() const -> std::string;

  /*!
   * \brief Returns the OpenCL numeric platform version.
   *
   * \return OpenCL numeric platform version.
   */
  [[nodiscard]] auto GetNumericVersion() const -> cl_version;

  /*!
   * \brief Returns the OpenCL host timer resolution.
   *
   * \return OpenCL host timer resolution.
   */
  [[nodiscard]] auto GetHostTimerResolution() const -> cl_ulong;

  /*!
   * \brief Returns the OpenCL versioned platform extensions.
   *
   * \return OpenCL versioned platform extensions.
   */
  [[nodiscard]] auto GetExtensionsWithVersion() const
      -> std::vector<cl_name_version>;

  /*!
   * \brief Prints platform identity, capability, and extension information.
   */
  auto Print() const -> void;

  /*!
   * \brief Returns the GGEMS platform index.
   *
   * \return GGEMS platform index.
   */
  [[nodiscard]] auto GetPlatformIndex() const noexcept -> std::size_t {
    return platform_index_;
  }

  /*!
   * \brief Returns the native OpenCL platform.
   *
   * \return Native OpenCL platform.
   */
  [[nodiscard]] auto GetPlatformNative() const noexcept
      -> cl::Platform const & {
    return platform_;
  }

  /*!
   * \brief Returns the devices discovered on this platform.
   *
   * \return Discovered GGEMS OpenCL devices.
   */
  [[nodiscard]] auto GetDevices() const noexcept
      -> std::vector<GGEMSOpenCLDevice> const & {
    return devices_;
  }

private:
  /*!
   * \brief Prints platform identity information.
   */
  auto PrintIdentity() const -> void;

  /*!
   * \brief Prints platform extension information.
   */
  auto PrintExtension() const -> void;

  /*!
   * \brief Discovers CPU and GPU OpenCL devices on this platform.
   */
  auto DiscoverDevices() -> void;

  cl::Platform platform_;                    /*!< Native OpenCL platform. */
  std::size_t platform_index_;              /*!< GGEMS platform index. */
  std::vector<GGEMSOpenCLDevice> devices_;  /*!< Devices discovered on the platform. */
};
} // namespace ggems::ocl
