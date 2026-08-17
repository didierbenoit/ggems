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
 * \brief Unit tests for GGEMS OpenCL contexts.
 *
 * Validates one-device context construction, command-queue coherence, native device association, and profiling-enabled queue properties for available devices.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <type_traits>

#include <gtest/gtest.h>

/// \endcond
#include "GGEMS/opencl/GGEMSOpenCLContext.hh"
#include "GGEMS/opencl/GGEMSOpenCLExternal.hh"
#include "GGEMSOpenCLDeviceInventory.hh"

/// \cond

static_assert(!std::is_copy_constructible_v<ggems::ocl::GGEMSOpenCLContext>);
static_assert(!std::is_copy_assignable_v<ggems::ocl::GGEMSOpenCLContext>);

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLContextTest,
     CreatesCoherentOneDeviceContextAndQueueForEveryAvailableDevice) {
  auto const inventories = ggems::test::GetOpenCLDeviceInventory();
  ASSERT_FALSE(inventories.empty());

  for (auto const &inventory : inventories) {
    auto const description = ggems::test::DescribeOpenCLDevice(inventory);
    SCOPED_TRACE(description);

    auto const &platform = inventory.platform.get();
    auto const &device = inventory.device.get();

    EXPECT_EQ(inventory.platform_index, platform.GetPlatformIndex());
    EXPECT_EQ(inventory.platform_index, device.GetPlatformIndex());
    EXPECT_EQ(inventory.device_index, device.GetDeviceIndex());

    if (device.GetAvailable() == CL_FALSE) {
      continue;
    }

    ggems::ocl::GGEMSOpenCLContext context{device};

    EXPECT_EQ(&context.GetDevice(), &device);
    EXPECT_NE(context.GetContextNative()(), nullptr);
    EXPECT_NE(context.GetCommandQueueNative()(), nullptr);
    EXPECT_EQ(context.GetNumDevices(), 1U);

    auto const native_devices = context.GetNativeDevices();
    EXPECT_EQ(native_devices.size(), 1U);
    if (native_devices.size() == 1U) {
      EXPECT_EQ(native_devices.front()(), device.GetDeviceNative()());
    }

    EXPECT_EQ(context.GetQueueContext()(), context.GetContextNative()());
    EXPECT_EQ(context.GetQueueDevice()(), device.GetDeviceNative()());
    EXPECT_NE(context.GetQueueProperties() & CL_QUEUE_PROFILING_ENABLE,
              static_cast<cl_command_queue_properties>(0));
  }
}
/// \endcond
