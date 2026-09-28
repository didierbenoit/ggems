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

#include <algorithm>
#include <format>
#include <vector>

#include <gtest/gtest.h>

#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/opencl/GGEMSOpenCLDevice.hh"
#include "GGEMS/opencl/GGEMSOpenCLExternal.hh"
#include "GGEMS/opencl/GGEMSOpenCLPlatform.hh"
#include "GGEMS/opencl/GGEMSOpenCLUtils.hh"

namespace {

template <cl_platform_info Info>
[[nodiscard]] auto GetNativePlatformInfo(cl::Platform const &platform) {
  cl_int error{CL_SUCCESS};
  auto value = platform.getInfo<Info>(&error);
  ggems::ocl::CheckCLError(error, "Failed to query native platform info.");
  return value;
}

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLPlatformTest,
     RepresentativePropertiesMatchNativePlatformInformation) {
  auto const &platforms = ggems::ocl::GGEMSOpenCL::GetInstance().GetPlatforms();
  ASSERT_FALSE(platforms.empty());

  for (auto const &platform : platforms) {
    SCOPED_TRACE(std::format("platform={}", platform.GetPlatformIndex()));
    auto const &native = platform.GetPlatformNative();

    EXPECT_EQ(platform.GetName(),
              GetNativePlatformInfo<CL_PLATFORM_NAME>(native));
    EXPECT_EQ(platform.GetVendor(),
              GetNativePlatformInfo<CL_PLATFORM_VENDOR>(native));
    EXPECT_EQ(platform.GetProfile(),
              GetNativePlatformInfo<CL_PLATFORM_PROFILE>(native));
    EXPECT_EQ(platform.GetVersion(),
              GetNativePlatformInfo<CL_PLATFORM_VERSION>(native));
    EXPECT_EQ(platform.GetExtensions(),
              GetNativePlatformInfo<CL_PLATFORM_EXTENSIONS>(native));

    EXPECT_FALSE(platform.GetName().empty());
    EXPECT_FALSE(platform.GetVendor().empty());
    EXPECT_FALSE(platform.GetVersion().empty());
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLPlatformTest, DiscoveredDevicesMatchTheNativePlatformSet) {
  auto const &platforms = ggems::ocl::GGEMSOpenCL::GetInstance().GetPlatforms();
  ASSERT_FALSE(platforms.empty());

  for (auto const &platform : platforms) {
    SCOPED_TRACE(std::format("platform={}", platform.GetPlatformIndex()));

    std::vector<cl::Device> native_devices;
    auto const error = platform.GetPlatformNative().getDevices(
      CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_GPU, &native_devices);
    ggems::ocl::CheckCLError(error, "Failed to query native platform devices.");

    auto const &devices = platform.GetDevices();
    ASSERT_EQ(devices.size(), native_devices.size());

    std::vector<cl_device_id> discovered_ids;
    discovered_ids.reserve(devices.size());

    for (auto const &device : devices) {
      discovered_ids.push_back(device.GetDeviceNative()());
    }

    std::vector<cl_device_id> native_ids;
    native_ids.reserve(native_devices.size());

    for (auto const &device : native_devices) {
      native_ids.push_back(device());
    }

    EXPECT_TRUE(std::ranges::is_permutation(discovered_ids, native_ids));
  }
}
