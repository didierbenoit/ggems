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
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <memory>
#include <vector>

#include "GGEMS/opencl/GGEMSOpenCLContext.hh"
#include "GGEMS/opencl/GGEMSOpenCLDevice.hh"
#include "GGEMS/opencl/GGEMSOpenCLExternal.hh"
#include "GGEMSOpenCLDeviceInventory.hh"

namespace ggems::test {

// =============================================================================
// =============================================================================

struct OpenCLCompilerDeviceInventoryEntry {
  OpenCLDeviceInventoryEntry inventory;
  std::unique_ptr<ocl::GGEMSOpenCLContext> context;
};

// =============================================================================
// =============================================================================

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
