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
 * \brief Checks selected-device context and queue coherence.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <type_traits>

#include <gtest/gtest.h>

#include "GGEMS/opencl/GGEMSOpenCLContext.hh"
#include "GGEMS/opencl/GGEMSOpenCLExternal.hh"
#include "GGEMSOpenCLDeviceInventory.hh"

static_assert(!std::is_copy_constructible_v<ggems::ocl::GGEMSOpenCLContext>);
static_assert(!std::is_copy_assignable_v<ggems::ocl::GGEMSOpenCLContext>);

// =============================================================================
// =============================================================================

// Context and profiling queue refer to the requested native device and
// platform.
TEST(GGEMSOpenCLContextTest, CreatesCoherentContextAndQueueForSelectedDevice) {
  auto const &inventory = ggems::test::GetSelectedOpenCLDevice();
  auto const description = ggems::test::DescribeOpenCLDevice(inventory);
  SCOPED_TRACE(description);

  auto const &platform = inventory.platform.get();
  auto const &device = inventory.device.get();

  EXPECT_EQ(inventory.platform_index, platform.GetPlatformIndex());
  EXPECT_EQ(inventory.platform_index, device.GetPlatformIndex());
  EXPECT_EQ(inventory.device_index, device.GetDeviceIndex());

  auto &context = ggems::test::GetSelectedOpenCLContext();

  EXPECT_EQ(&context.GetDevice(), &device);
  EXPECT_NE(context.GetContextNative()(), nullptr);
  EXPECT_NE(context.GetCommandQueueNative()(), nullptr);
  EXPECT_EQ(context.GetNumDevices(), 1U);

  auto const native_devices = context.GetNativeDevices();

  ASSERT_EQ(native_devices.size(), 1U);
  EXPECT_EQ(native_devices.front()(), device.GetDeviceNative()());

  EXPECT_EQ(context.GetQueueContext()(), context.GetContextNative()());
  EXPECT_EQ(context.GetQueueDevice()(), device.GetDeviceNative()());
  EXPECT_NE(context.GetQueueProperties() & CL_QUEUE_PROFILING_ENABLE,
            static_cast<cl_command_queue_properties>(0));
}
