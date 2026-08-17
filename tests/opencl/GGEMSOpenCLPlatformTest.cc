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
 * \brief Unit tests for GGEMS OpenCL platform discovery.
 *
 * Compares representative platform properties with native OpenCL information and checks that discovered GGEMS devices match the native platform device set.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <algorithm>
#include <format>
#include <vector>

#include <gtest/gtest.h>

/// \endcond
#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/opencl/GGEMSOpenCLDevice.hh"
#include "GGEMS/opencl/GGEMSOpenCLExternal.hh"
#include "GGEMS/opencl/GGEMSOpenCLPlatform.hh"
#include "GGEMS/opencl/GGEMSOpenCLUtils.hh"
#include "GGEMSOpenCLDeviceInventory.hh"

/// \cond

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
  auto const inventory = ggems::test::GetOpenCLDeviceInventory();

  for (auto const &platform : platforms) {
    SCOPED_TRACE(std::format("platform={}", platform.GetPlatformIndex()));

    std::vector<cl::Device> native_devices;
    auto const error = platform.GetPlatformNative().getDevices(
        CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_GPU, &native_devices);
    ggems::ocl::CheckCLError(error, "Failed to query native platform devices.");

    auto const &devices = platform.GetDevices();
    EXPECT_EQ(devices.size(), native_devices.size());

    for (auto const &entry : inventory) {
      if (&entry.platform.get() != &platform) {
        continue;
      }

      SCOPED_TRACE(ggems::test::DescribeOpenCLDevice(entry));
      auto const &device = entry.device.get();
      auto const native_iterator = std::ranges::find_if(
          native_devices, [&](auto const &native_device) -> bool {
            return native_device() == device.GetDeviceNative()();
          });
      EXPECT_NE(native_iterator, native_devices.end());
    }
  }
}
/// \endcond
