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
 * \brief Shared OpenCL device inventory utilities for tests.
 *
 * Flattens GGEMS platform/device discovery into stable test records and provides a concise device description suitable for GoogleTest traces.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <cstddef>
#include <format>
#include <functional>
#include <string>
#include <vector>
/// \endcond

#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/opencl/GGEMSOpenCLDevice.hh"
#include "GGEMS/opencl/GGEMSOpenCLPlatform.hh"
#include "GGEMS/opencl/GGEMSOpenCLStrings.hh"

/*!
 * \namespace ggems::test
 * \brief Utilities shared by GGEMS test suites.
 */
namespace ggems::test {

/*!
 * \brief Describes one GGEMS-discovered OpenCL device.
 */
struct OpenCLDeviceInventoryEntry {
  /*! \brief Zero-based GGEMS platform index. */
  std::size_t platform_index;
  /*! \brief Zero-based GGEMS device index within the platform. */
  std::size_t device_index;
  /*! \brief Reference to the discovered GGEMS platform. */
  std::reference_wrapper<ocl::GGEMSOpenCLPlatform const> platform;
  /*! \brief Reference to the discovered GGEMS device. */
  std::reference_wrapper<ocl::GGEMSOpenCLDevice const> device;
};

/*!
 * \brief Builds a flattened inventory of all GGEMS-discovered OpenCL devices.
 *
 * \return Device entries ordered by platform discovery order and then device order.
 */
[[nodiscard]] inline auto GetOpenCLDeviceInventory()
    -> std::vector<OpenCLDeviceInventoryEntry> {
  std::vector<OpenCLDeviceInventoryEntry> inventory;

  auto const &platforms = ocl::GGEMSOpenCL::GetInstance().GetPlatforms();
  for (auto const &platform : platforms) {
    for (auto const &device : platform.GetDevices()) {
      inventory.push_back(OpenCLDeviceInventoryEntry{
          .platform_index = platform.GetPlatformIndex(),
          .device_index = device.GetDeviceIndex(),
          .platform = std::cref(platform),
          .device = std::cref(device),
      });
    }
  }

  return inventory;
}

/*!
 * \brief Builds a concise diagnostic description of an OpenCL device entry.
 *
 * \param[in] entry Device inventory entry to describe.
 * \return Human-readable platform, device, name, vendor, and type summary.
 */
[[nodiscard]] inline auto
DescribeOpenCLDevice(OpenCLDeviceInventoryEntry const &entry) -> std::string {
  auto const &device = entry.device.get();
  return std::format("platform={}, device={}, name='{}', vendor='{}', type={}",
                     entry.platform_index, entry.device_index, device.GetName(),
                     device.GetVendor(),
                     ocl::DeviceTypeToString(device.GetType()));
}
} // namespace ggems::test
