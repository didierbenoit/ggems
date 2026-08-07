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
 * \brief Central manager for OpenCL platforms, devices, contexts and kernel
 * programs.
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-10-29
 * \version 2.0
 * \copyright
 * GNU General Public License v3.0
 */

#include <mutex>

#include "GGEMS/frameworks/GGEMSOpenCLProgram.hh"

namespace ggems::ocl {

class GGEMSOpenCLPlatform;
class GGEMSOpenCLContext;
class GGEMSOpenCLDevice;

/*!
 * \class GGEMSOpenCL
 * \brief Singleton managing the OpenCL runtime used by GGEMS.
 *
 * This class initializes OpenCL platforms and devices, creates contexts,
 * manages kernel programs, and exposes runtime inspection utilities.
 * It centralizes all low-level OpenCL interactions required throughout GGEMS.
 */
class GGEMSOpenCL {
private:
  /*!
   * \brief Construct the singleton instance.
   *
   * The constructor initializes OpenCL platforms/devices and configures
   * the runtime environment. It should never be invoked directly and is
   * only used internally by the GetInstance() method.
   */
  GGEMSOpenCL();

  GGEMSOpenCL(GGEMSOpenCL const &openCL) = delete;
  GGEMSOpenCL(GGEMSOpenCL &&openCL) = delete;
  GGEMSOpenCL &operator=(GGEMSOpenCL const &openCL) = delete;
  GGEMSOpenCL &operator=(GGEMSOpenCL &&openCL) = delete;

public:
  /*!
   * \brief Accessor to the unique GGEMSOpenCL singleton.
   *
   * Constructs the instance on first call, then returns a stable reference.
   *
   * \return Reference to the GGEMSOpenCL singleton.
   */
  [[nodiscard]] static GGEMSOpenCL &GetInstance() {
    static GGEMSOpenCL *instance = []() {
      GGEMS_INFOEX("OpenCL", 3, "Creating GGEMSOpenCL singleton instance.");
      return new GGEMSOpenCL();
    }();
    return *instance;
  }

  /*!
   * \brief Retrieve an existing program or build a new one.
   *
   * Returns a cached OpenCL program if available, otherwise creates
   * a new GGEMSOpenCLProgram instance, builds it, stores it in the internal
   * cache, and returns it.
   *
   * \param ctx           Target OpenCL context.
   * \param kernel_root   Root directory containing kernel sources.
   * \param kernel_name   Kernel module name (without extension).
   * \param build_options Optional build flags for compilation.
   *
   * \return Reference to the retrieved or newly created program object.
   */
  GGEMSOpenCLProgram &GetOrCreateProgram(
      GGEMSOpenCLContext &ctx, std::filesystem::path const &kernel_root,
      std::string const &kernel_name, std::string const &build_options = "");

  /*!
   * \brief Destroy the OpenCL runtime.
   *
   * Releases internal resources but intentionally preserves allocated
   * memory to ensure stable shutdown behavior.
   */
  ~GGEMSOpenCL();

  /*!
   * \brief Clean all OpenCL platforms.
   *
   * Invokes the cleanup routine on each platform and releases all device-side
   * allocations tracked by GGEMS.
   */
  void Clean() noexcept;

  /*!
   * \brief Print all detected OpenCL platforms.
   *
   * Display device count, vendor, and OpenCL capabilities through the logger.
   */
  void PrintPlatforms() const noexcept;

  /*!
   * \brief Print all OpenCL devices available on all platforms.
   *
   * Enumerates vendor, type and capabilities of each detected device.
   */
  void PrintDevices() const noexcept;

  /*!
   * \brief Print all created OpenCL contexts.
   *
   * Displays summary of context state and details about their command queues.
   */
  void PrintContexts() const noexcept;

  /*!
   * \brief Access the internal list of OpenCL platforms.
   *
   * \return Constant reference to the vector of OpenCL platforms.
   */
  [[nodiscard]] std::vector<GGEMSOpenCLPlatform> const &
  GetPlatforms() const noexcept {
    return platforms_;
  }

  /*!
   * \brief Select devices that match user-defined filters.
   *
   * Filters may include vendor names, “cpu”, “gpu”, or numeric selections.
   *
   * \param filters List of filtering expressions.
   */
  void SelectDevices(std::vector<std::string> const &filters);

  /*!
   * \brief Build OpenCL contexts for all selected devices.
   *
   * Errors are fatal and terminate the program.
   */
  void Initialize();

  /*!
   * \brief Access the list of OpenCL contexts.
   *
   * \return Reference to the vector of active contexts.
   */
  [[nodiscard]]
  std::vector<GGEMSOpenCLContext> &GetContext() noexcept {
    return contexts_;
  }

private:
  /*!
   * \brief Enumerate platforms and devices and populate internal lists.
   *
   * Called once during construction of the singleton.
   */
  void InitPlatformsAndDevices();

  /*!
   * \brief Parse device filters and return a selection of matching devices.
   *
   * \param filters     Raw filter expressions.
   * \param all_devices All available devices.
   *
   * \return Vector of matching devices.
   */
  [[nodiscard]]
  std::vector<std::reference_wrapper<GGEMSOpenCLDevice const>>
  ParseDeviceFilters(
      std::vector<std::string> const &filters,
      std::vector<std::reference_wrapper<GGEMSOpenCLDevice const>> all_devices);

  /*!
   * \brief Create OpenCL contexts for the selected devices.
   *
   * Each device receives its own context instance.
   */
  void CreateContexts();

  /*!
   * \brief Disable the NVIDIA CUDA cache to ensure fresh kernel builds.
   *
   * Mostly relevant when CUDA and OpenCL coexist on the same machine.
   */
  void DisableNvidiaDriverKernelCache() const;

private:
  std::vector<GGEMSOpenCLPlatform>
      platforms_; /*!< List of detected OpenCL platforms. */
  std::vector<std::reference_wrapper<GGEMSOpenCLDevice const>>
      selected_devices_; /*!< Subset of devices chosen by filtering rules. */
  std::vector<GGEMSOpenCLContext>
      contexts_; /*!< Constructed contexts derived from selected devices. */
  std::vector<std::unique_ptr<GGEMSOpenCLProgram>>
      program_cache_; /*!< Cache of compiled kernel programs. */
  std::mutex program_cache_mutex_;
};
} // namespace ggems::ocl
