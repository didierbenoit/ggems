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
#include <cstdint>
/// \endcond

#include "GGEMS/logging/GGEMSLogMacros.hh"
#include "GGEMS/opencl/GGEMSOpenCLProgram.hh"

namespace ggems::ocl {

class GGEMSOpenCLPlatform;
class GGEMSOpenCLContext;
class GGEMSOpenCLDevice;

/*!
 * \brief Owns GGEMS OpenCL platform, device, context, and program runtime
 * state.
 *
 * The process-lifetime singleton discovers OpenCL resources, applies device
 * selection, creates one context per selected device, and caches built
 * programs.
 */
class GGEMSOpenCL {
public:
  /*!
   * \brief Destroys the OpenCL runtime manager.
   */
  ~GGEMSOpenCL() = default;

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
    static GGEMSOpenCL *instance = [] -> GGEMSOpenCL * {
      GGEMS_INFOEX("OpenCL", 3, "Creating GGEMSOpenCL singleton instance.");
      return new GGEMSOpenCL();
    }();
    return *instance;
  }

  /*!
   * \brief Returns a cached program or creates and caches a matching program.
   *
   * Cached programs are never evicted, so the returned reference stays valid
   * for the process lifetime.
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
   * The device selection is frozen once the backend is initialized. After
   * that, a request is resolved against the discovered devices and accepted
   * only when it designates the devices of the active contexts, in the same
   * order. Any request that would really change the selection is rejected
   * before the stored selection is modified, so the selection and the active
   * contexts stay consistent.
   *
   * Device selection and initialization are not synchronized; the caller
   * serializes them.
   *
   * \param[in] filters Ordered device selector expressions.
   * \throws ggems::core::GGEMSFatal If the selectors are malformed, designate
   * no device, or would change the selection after the backend was
   * initialized.
   */
  auto SelectDevices(std::vector<std::string> const &filters) -> void;

  /*!
   * \brief Creates OpenCL contexts for the current device selection.
   *
   * The active contexts are created once and retained for the process
   * lifetime because live SVM buffers, kernels, and transport workloads keep
   * pointers and references to them. Creating the contexts also freezes the
   * device selection; see SelectDevices().
   *
   * Calling this method again once the backend is initialized keeps the active
   * contexts, and the program cache, unchanged.
   *
   * When no device is selected, no context is created and the backend stays
   * uninitialized, so a later call can still create the contexts.
   *
   * Device selection and initialization are not synchronized; the caller
   * serializes them.
   *
   * A context-creation failure terminates the process.
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

  /*!
   * \brief Sets the number of OpenCL workers used by each transport workload.
   *
   * The same worker count is currently used for every active OpenCL context.
   * The value is read when GGEMS transport workloads are created.
   *
   * \param[in] worker_count Number of workers used per OpenCL context.
   */
  auto SetWorkerCount(std::uint32_t worker_count) -> void;

  /*!
   * \brief Returns the number of OpenCL workers used by each transport
   * workload.
   *
   * \return Number of workers used per OpenCL context.
   */
  [[nodiscard]] auto GetWorkerCount() const noexcept -> std::uint32_t {
    return worker_count_;
  }

private:
  /*!
   * \brief Constructs and initializes the process-lifetime OpenCL runtime
   * manager.
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
   *
   * Called only while no context exists yet.
   */
  auto CreateContexts() -> void;

  /*!
   * \brief Resolves selector expressions into the devices they designate.
   *
   * \param[in] filters Device selector expressions.
   * \param[in] all_devices Flattened discovered device inventory.
   * \return Designated device references in selection order.
   */
  [[nodiscard]]
  static auto ResolveDeviceSelection(
    std::vector<std::string> const &filters,
    std::vector<std::reference_wrapper<GGEMSOpenCLDevice const>> const
      &all_devices)
    -> std::vector<std::reference_wrapper<GGEMSOpenCLDevice const>>;

  /*!
   * \brief Checks whether devices match the active contexts.
   *
   * \param[in] devices Device references compared with the active contexts.
   * \return True if one active context exists per device, in the same order.
   */
  [[nodiscard]] auto MatchesActiveContexts(
    std::vector<std::reference_wrapper<GGEMSOpenCLDevice const>> const &devices)
    const noexcept -> bool;

  /*!
   * \brief Disables the NVIDIA driver kernel cache for this process when
   * applicable.
   */
  static auto DisableNvidiaDriverKernelCache() -> void;

  std::vector<GGEMSOpenCLPlatform>
    platforms_; /*!< Discovered OpenCL platforms. */
  std::vector<std::reference_wrapper<GGEMSOpenCLDevice const>>
    selected_devices_; /*!< Selected OpenCL devices. */
  std::vector<GGEMSOpenCLContext>
    contexts_; /*!< Contexts for selected devices, retained for the process
                  lifetime. */
  std::vector<std::unique_ptr<GGEMSOpenCLProgram>>
    program_cache_; /*!< Cached OpenCL programs, never evicted. */
  std::mutex program_cache_mutex_; /*!< Mutex protecting the program cache. */
  bool is_initialized_{false};     /*!< Whether contexts were created. */
  std::uint32_t worker_count_{
    2'097'152}; /*!< Number of OpenCL workers used per transport workload. */
};
} // namespace ggems::ocl
