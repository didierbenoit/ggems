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
 * \brief Selects the one OpenCL device used by this test process.
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <charconv>
#include <cstddef>
#include <format>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>

#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/opencl/GGEMSOpenCLContext.hh"
#include "GGEMS/opencl/GGEMSOpenCLDevice.hh"
#include "GGEMS/opencl/GGEMSOpenCLExternal.hh"
#include "GGEMS/opencl/GGEMSOpenCLPlatform.hh"
#include "GGEMS/opencl/GGEMSOpenCLStrings.hh"

namespace ggems::test {

struct OpenCLDeviceRequest {
  cl_device_type type{CL_DEVICE_TYPE_GPU};
  std::size_t index{0U};
};

// Set once by the test launcher, before any test can request a device.
inline OpenCLDeviceRequest opencl_device_request;

[[nodiscard]] inline auto ParseOpenCLDeviceRequest(std::string_view selector)
  -> OpenCLDeviceRequest {
  OpenCLDeviceRequest request{.type = 0, .index = 0U};
  auto ordinal = selector;

  if (selector == "cpu" || selector.starts_with("cpu:")) {
    request.type = CL_DEVICE_TYPE_CPU;
    ordinal = selector == "cpu" ? "0" : selector.substr(4U);
  } else if (selector == "gpu" || selector.starts_with("gpu:")) {
    request.type = CL_DEVICE_TYPE_GPU;
    ordinal = selector == "gpu" ? "0" : selector.substr(4U);
  }

  if (ordinal.empty()) {
    throw std::invalid_argument{
      "The OpenCL test device index cannot be empty."};
  }

  auto const parsed = std::from_chars(
    ordinal.data(), ordinal.data() + ordinal.size(), request.index);
  if (parsed.ec != std::errc{} ||
      parsed.ptr != ordinal.data() + ordinal.size()) {
    throw std::invalid_argument{
      "Invalid --ggems-device value '" + std::string{selector} +
      "'; expected cpu[:N], gpu[:N], or a nonnegative overall device index."};
  }

  return request;
}

struct OpenCLDeviceInventoryEntry {
  std::size_t flat_index;
  std::size_t platform_index;
  std::size_t device_index;
  std::reference_wrapper<ocl::GGEMSOpenCLPlatform const> platform;
  std::reference_wrapper<ocl::GGEMSOpenCLDevice const> device;
};

[[nodiscard]] inline auto
DescribeOpenCLDevice(OpenCLDeviceInventoryEntry const &entry) -> std::string {
  auto const &device = entry.device.get();
  return std::format(
    "index={}, platform={} ('{}'), device={}, name='{}', vendor='{}', type={}",
    entry.flat_index, entry.platform_index, entry.platform.get().GetName(),
    entry.device_index, device.GetName(), device.GetVendor(),
    ocl::DeviceTypeToString(device.GetType()));
}

[[nodiscard]] inline auto GetSelectedOpenCLDevice()
  -> OpenCLDeviceInventoryEntry const & {
  static auto const selected = [] -> OpenCLDeviceInventoryEntry {
    auto &opencl = ocl::GGEMSOpenCL::GetInstance();
    std::size_t flat_index{0U};
    std::size_t matching_index{0U};

    for (auto const &platform : opencl.GetPlatforms()) {
      for (auto const &device : platform.GetDevices()) {
        bool const matches_type =
          opencl_device_request.type == 0 ||
          (device.GetType() & opencl_device_request.type) != 0;

        if (matches_type && matching_index++ == opencl_device_request.index) {
          OpenCLDeviceInventoryEntry const entry{
            .flat_index = flat_index,
            .platform_index = platform.GetPlatformIndex(),
            .device_index = device.GetDeviceIndex(),
            .platform = std::cref(platform),
            .device = std::cref(device),
          };

          if (device.GetAvailable() == CL_FALSE ||
              device.GetCompilerAvailable() == CL_FALSE) {
            throw std::runtime_error{"Requested test device requires "
                                     "availability and an OpenCL compiler: " +
                                     DescribeOpenCLDevice(entry)};
          }

          // Production remains the authority for selection and its freeze.
          opencl.SelectDevices({std::to_string(flat_index)});
          std::cout << "[ GGEMS DEVICE ] " << DescribeOpenCLDevice(entry)
                    << '\n';
          return entry;
        }

        ++flat_index;
      }
    }

    throw std::runtime_error{
      std::format("Requested OpenCL test device is absent (type={}, index={}); "
                  "no fallback is permitted.",
                  opencl_device_request.type == 0
                    ? "overall"
                    : ocl::DeviceTypeToString(opencl_device_request.type),
                  opencl_device_request.index)};
  }();

  return selected;
}

[[nodiscard]] inline auto GetSelectedOpenCLContext()
  -> ocl::GGEMSOpenCLContext & {
  static auto *context = [] -> ocl::GGEMSOpenCLContext * {
    (void)GetSelectedOpenCLDevice();
    auto &opencl = ocl::GGEMSOpenCL::GetInstance();
    opencl.Initialize();
    auto &selected = opencl.GetContext().front();
    if (!selected.GetSVMSupport().HasAny()) {
      throw std::runtime_error{
        "Requested test device has no effective GGEMS SVM support: " +
        DescribeOpenCLDevice(GetSelectedOpenCLDevice())};
    }
    return &selected;
  }();
  return *context;
}

} // namespace ggems::test
