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
 * \brief Shared inventory of compiler-capable OpenCL devices for tests.
 *
 * Builds stable test entries that pair discovered GGEMS devices with dedicated contexts when the device is available and exposes an OpenCL compiler.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <memory>
#include <vector>
/// \endcond

#include "GGEMS/frameworks/GGEMSOpenCLContext.hh"
#include "GGEMS/frameworks/GGEMSOpenCLDevice.hh"
#include "GGEMS/frameworks/GGEMSOpenCLExternal.hh"
#include "GGEMSOpenCLDeviceInventory.hh"

namespace ggems::test {

/*!
 * \brief Compiler-capable OpenCL device entry used by framework tests.
 */
struct OpenCLCompilerDeviceInventoryEntry {
  /*!
   * \brief Discovered platform/device inventory entry.
   */
  OpenCLDeviceInventoryEntry inventory;
  /*!
   * \brief Dedicated OpenCL context for the compiler-capable device.
   */
  std::unique_ptr<ocl::GGEMSOpenCLContext> context;
};

// =============================================================================
// =============================================================================

/*!
 * \brief Returns compiler-capable OpenCL devices with dedicated contexts.
 *
 * The inventory is constructed once and excludes devices that are unavailable
 * or do not report an OpenCL compiler.
 *
 * \return Stable read-only inventory of compiler-capable devices.
 */
[[nodiscard]] inline auto GetOpenCLCompilerDeviceInventory()
    -> std::vector<OpenCLCompilerDeviceInventoryEntry> const & {
  static auto const inventory =
      [] -> std::vector<OpenCLCompilerDeviceInventoryEntry> {
    std::vector<OpenCLCompilerDeviceInventoryEntry> result;

    for (auto const &entry : GetOpenCLDeviceInventory()) {
      auto const &device = entry.device.get();
      if (device.GetAvailable() == CL_FALSE ||
          device.GetCompilerAvailable() == CL_FALSE) {
        continue;
      }

      result.push_back(OpenCLCompilerDeviceInventoryEntry{
          .inventory = entry,
          .context = std::make_unique<ocl::GGEMSOpenCLContext>(device),
      });
    }

    return result;
  }();

  return inventory;
}
} // namespace ggems::test
