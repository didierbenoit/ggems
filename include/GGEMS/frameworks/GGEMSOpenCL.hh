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
 * \file GGEMSOpenCL.hh
 * \brief Declaration of the GGEMSOpenCL singleton class for OpenCL management
 *
 * This file contains the GGEMSOpenCL singleton class which manages
 * the initialization and lifecycle of OpenCL platforms, devices, and contexts.
 * It provides utility functions to inspect available OpenCL platforms.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-10-12
 * \copyright GNU General Public License v3.0
 * \version 2.0
 */

#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/frameworks/GGEMSOpenCLDevice.hh"

/// \cond
#include <functional>
#include <vector>
/// \endcond

namespace ggems::ocl {

class GGEMSOpenCLPlatform;
class GGEMSOpenCLContext;
class GGEMSOpenCLDevice;

/*!
 * \class GGEMSOpenCL
 * \brief Singleton managing OpenCL platforms and devices for GGEMS.
 *
 * GGEMSOpenCL ensures a unique and persistent OpenCL runtime context
 * for the entire GGEMS framework. It handles discovery of platforms,
 * devices, and manages global OpenCL resources.
 *
 * \note
 * The instance is **intentionally heap-allocated** (via `new`)
 * and never freed. This design ensures compatibility with Pybind11
 * and multiple interpreter environments (`per_interpreter_gil`),
 * avoiding premature destruction of OpenCL resources.
 */
class GGEMSOpenCL {
private:
  /*!
   * \brief Default constructor (private).
   *
   * Discovers all OpenCL platforms and their associated devices.
   * Throws GGEMSException on failure.
   */
  GGEMSOpenCL();

  GGEMSOpenCL(GGEMSOpenCL const &openCL) = delete;
  GGEMSOpenCL(GGEMSOpenCL const &&openCL) = delete;
  GGEMSOpenCL &operator=(GGEMSOpenCL const &openCL) = delete;
  GGEMSOpenCL &operator=(GGEMSOpenCL const &&openCL) = delete;

public:
  /*!
   * \brief Access the GGEMSOpenCL singleton instance.
   *
   * The instance is lazily created upon the first call.
   * Memory is intentionally leaked to guarantee persistent lifetime
   * when used within Python bindings or multi-interpreter environments.
   *
   * \return Reference to the singleton GGEMSOpenCL instance.
   */
  [[nodiscard]] static GGEMSOpenCL &GetInstance() {
    static GGEMSOpenCL *instance = []() {
      GGEMS_INFOEX("OpenCL", 2, "First Instance of GGEMSOpenCL singleton...");
      return new GGEMSOpenCL(); // intentionally leaked
    }();
    return *instance;
  }

  /*!
   * \brief Destructor.
   *
   * Performs a graceful shutdown of internal OpenCL resources.
   * The singleton memory itself is not freed by design.
   */
  ~GGEMSOpenCL();

  /*!
   * \brief Explicitly releases OpenCL compilers and related platform resources.
   *
   * This can be safely called before program termination or module unloading.
   */
  void Clean() noexcept;

  /*!
   * \brief Prints detailed information about all discovered OpenCL platforms.
   *
   * Each platform print includes vendor, version, profile, and supported
   * extensions.
   */
  void PrintPlatforms() const;

  /*!
   * \brief Print all devices across all platforms (aggregated list).
   */
  void PrintDevices() const;

  void PrintContexts() const;

  /*!
   * \brief Provides read-only access to the discovered OpenCL platforms.
   * \return Constant reference to the list of available platforms.
   */
  [[nodiscard]] std::vector<GGEMSOpenCLPlatform> const &
  GetPlatforms() const noexcept {
    return platforms_;
  }

  void SelectDevices(std::vector<std::string> const &filters);
  void Initialise();

  [[nodiscard]]
  std::vector<GGEMSOpenCLContext> &GetContext() noexcept {
    return contexts_;
  }

private:
  /*!
   * \brief Enumerates OpenCL platforms and creates GGEMSOpenCLPlatform
   * instances.
   *
   * Called internally during construction.
   */
  void InitPlatformsAndDevices();

  [[nodiscard]]
  std::vector<std::reference_wrapper<GGEMSOpenCLDevice const>>
  ParseDeviceFilters(
      std::vector<std::string> const &filters,
      std::vector<std::reference_wrapper<GGEMSOpenCLDevice const>> all_devices);

  void CreateContexts();
  /*!
   * \brief Disable GPU driver kernel caching (development convenience).
   *
   * On NVIDIA platforms, OpenCL kernels compiled via the CUDA driver
   * may be cached on disk (typically under `~/.nv/ComputeCache`).
   * This function disables that cache by setting the environment variable
   * `CUDA_CACHE_DISABLE=1`. It is primarily useful during debugging or
   * dynamic recompilation of OpenCL kernels.
   *
   * \note
   * On Windows, `_putenv_s()` is used.
   * On POSIX systems, `setenv()` is called safely.
   */
  void DisableKernelCache() const;

private:
  std::vector<GGEMSOpenCLPlatform>
      platforms_; /*!< Vector storing all detected OpenCL platforms */
  std::vector<std::reference_wrapper<GGEMSOpenCLDevice const>>
      selected_devices_;
  std::vector<GGEMSOpenCLContext> contexts_;
};
} // namespace ggems::ocl
