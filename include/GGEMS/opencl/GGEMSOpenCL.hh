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
 * \brief Declares the GGEMS OpenCL runtime manager.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <mutex>
#include <string>
#include <vector>
#include <filesystem>
#include <functional>
#include <memory>
/// \endcond

#include "GGEMS/logging/GGEMSLogMacros.hh"
#include "GGEMS/opencl/GGEMSOpenCLProgram.hh"

namespace ggems::ocl {

class GGEMSOpenCLPlatform;
class GGEMSOpenCLContext;
class GGEMSOpenCLDevice;

/*!
 * \brief Owns GGEMS OpenCL platform, device, context, and program runtime state.
 *
 * The process-lifetime singleton discovers OpenCL resources, applies device selection, creates one context per selected device, and caches built programs.
 */
class GGEMSOpenCL {
public:
  /*!
   * \brief Destroys the OpenCL runtime manager.
   */
  ~GGEMSOpenCL();

  /*!
   * \brief Disables copy construction of the OpenCL runtime manager.
   *
   * \param[in] openCL Source runtime manager.
   */
  GGEMSOpenCL(GGEMSOpenCL const &openCL) = delete;

  /*!
   * \brief Disables move construction of the OpenCL runtime manager.
   *
   * \param[in] openCL Source runtime manager.
   */
  GGEMSOpenCL(GGEMSOpenCL &&openCL) = delete;

  /*!
   * \brief Disables copy assignment of the OpenCL runtime manager.
   *
   * \param[in] openCL Source runtime manager.
   */
  auto operator=(GGEMSOpenCL const &openCL) -> GGEMSOpenCL & = delete;

  /*!
   * \brief Disables move assignment of the OpenCL runtime manager.
   *
   * \param[in] openCL Source runtime manager.
   */
  auto operator=(GGEMSOpenCL &&openCL) -> GGEMSOpenCL & = delete;

  /*!
   * \brief Returns the process-lifetime OpenCL runtime manager.
   *
   * \return OpenCL runtime manager singleton.
   */
  [[nodiscard]] static auto GetInstance() -> GGEMSOpenCL & {
    static GGEMSOpenCL *instance = []() -> GGEMSOpenCL * {
      GGEMS_INFOEX("OpenCL", 3, "Creating GGEMSOpenCL singleton instance.");
      return new GGEMSOpenCL();
    }();
    return *instance;
  }

  /*!
   * \brief Returns a cached program or creates and caches a matching program.
   *
   * \param[in] ctx OpenCL context used to build the program.
   * \param[in] kernel_root Root directory containing the kernel sources.
   * \param[in] kernel_name Kernel source name.
   * \param[in] build_options Additional OpenCL build options.
   * \return Matching cached or newly created OpenCL program.
   */
  auto GetOrCreateProgram(GGEMSOpenCLContext const &ctx,
                          std::filesystem::path const &kernel_root,
                          std::string const &kernel_name,
                          std::string const &build_options = "")
      -> GGEMSOpenCLProgram const &;

  /*!
   * \brief Prints information for all discovered OpenCL platforms.
   */
  auto PrintPlatforms() const -> void;

  /*!
   * \brief Prints information for all discovered OpenCL devices.
   */
  auto PrintDevices() const -> void;

  /*!
   * \brief Prints information for all active OpenCL contexts.
   */
  auto PrintContexts() const -> void;

  /*!
   * \brief Returns the discovered OpenCL platforms.
   *
   * \return Discovered OpenCL platforms.
   */
  [[nodiscard]] auto GetPlatforms() const noexcept
      -> std::vector<GGEMSOpenCLPlatform> const & {
    return platforms_;
  }

  /*!
   * \brief Selects OpenCL devices from the supplied selector expressions.
   *
   * \param[in] filters Ordered device selector expressions.
   */
  auto SelectDevices(std::vector<std::string> const &filters) -> void;

  /*!
   * \brief Creates OpenCL contexts for the current device selection.
   */
  auto Initialize() -> void;

  /*!
   * \brief Returns the active OpenCL contexts.
   *
   * \return Mutable collection of active OpenCL contexts.
   */
  [[nodiscard]]
  auto GetContext() noexcept -> std::vector<GGEMSOpenCLContext> & {
    return contexts_;
  }

private:
  /*!
   * \brief Constructs and initializes the process-lifetime OpenCL runtime manager.
   */
  GGEMSOpenCL();

  /*!
   * \brief Discovers available OpenCL platforms and devices.
   */
  void InitPlatformsAndDevices();

  /*!
   * \brief Parses device selectors against the discovered device inventory.
   *
   * \param[in] filters Device selector expressions.
   * \param[in] all_devices Flattened discovered device inventory.
   * \return Selected device references in selector order.
   */
  [[nodiscard]]
  static auto ParseDeviceFilters(
      std::vector<std::string> const &filters,
      std::vector<std::reference_wrapper<GGEMSOpenCLDevice const>> const
          &all_devices)
      -> std::vector<std::reference_wrapper<GGEMSOpenCLDevice const>>;

  /*!
   * \brief Creates one OpenCL context for each selected device.
   */
  auto CreateContexts() -> void;

  /*!
   * \brief Disables the NVIDIA driver kernel cache for this process when applicable.
   */
  static auto DisableNvidiaDriverKernelCache() -> void;

  std::vector<GGEMSOpenCLPlatform> platforms_; /*!< Discovered OpenCL platforms. */
  std::vector<std::reference_wrapper<GGEMSOpenCLDevice const>>
      selected_devices_; /*!< Selected OpenCL devices. */
  std::vector<GGEMSOpenCLContext> contexts_; /*!< Contexts for selected devices. */
  std::vector<std::unique_ptr<GGEMSOpenCLProgram>> program_cache_; /*!< Cached OpenCL programs. */
  std::mutex program_cache_mutex_; /*!< Mutex protecting the program cache. */
};
} // namespace ggems::ocl
