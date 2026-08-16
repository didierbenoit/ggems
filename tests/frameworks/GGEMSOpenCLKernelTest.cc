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
 * \brief Unit tests for GGEMS OpenCL kernel execution.
 *
 * Builds and executes the shared OpenCL framework probe on every compiler-capable device and validates coherent kernel metadata and results.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <array>
#include <cstddef>
#include <string>

#include <gtest/gtest.h>

/// \endcond
#include "GGEMS/frameworks/GGEMSOpenCL.hh"
#include "GGEMS/frameworks/GGEMSOpenCLContext.hh"
#include "GGEMS/frameworks/GGEMSOpenCLExternal.hh"
#include "GGEMS/frameworks/GGEMSOpenCLKernel.hh"
#include "GGEMS/frameworks/GGEMSOpenCLUtils.hh"
#include "GGEMSOpenCLCompilerDeviceInventory.hh"
#include "GGEMSOpenCLDeviceInventory.hh"
#include "GGEMSOpenCLFrameworkProbe.hh"

/// \cond

namespace {

constexpr std::size_t k_value_count{4U};

using ggems::test::GetOpenCLFrameworkProbeRoot;
using ggems::test::k_opencl_framework_probe_name;

// =============================================================================
// =============================================================================

[[nodiscard]] auto
CreateProbeBuffer(ggems::ocl::GGEMSOpenCLContext const &context,
                  std::array<cl_uint, k_value_count> &values) -> cl::Buffer {
  cl_int error{CL_SUCCESS};
  cl::Buffer buffer(context.GetContextNative(),
                    CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(values),
                    values.data(), &error);
  ggems::ocl::CheckCLError(error, "Failed to create OpenCL probe buffer.");
  return buffer;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto
ReadProbeBuffer(ggems::ocl::GGEMSOpenCLContext const &context,
                cl::Buffer const &buffer)
    -> std::array<cl_uint, k_value_count> {
  std::array<cl_uint, k_value_count> values{};
  auto const error = context.GetCommandQueueNative().enqueueReadBuffer(
      buffer, CL_TRUE, 0U, sizeof(values), values.data());
  ggems::ocl::CheckCLError(error, "Failed to read OpenCL probe buffer.");
  return values;
}

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLKernelTest,
     ExecutesAndReportsCoherentMetadataOnEveryCompilerDevice) {
  auto const &compiler_devices =
      ggems::test::GetOpenCLCompilerDeviceInventory();
  if (compiler_devices.empty()) {
    GTEST_SKIP() << "No available GGEMS-discovered device has a compiler.";
  }

  auto const probe_root = GetOpenCLFrameworkProbeRoot();
  for (auto const &compiler_device : compiler_devices) {
    SCOPED_TRACE(ggems::test::DescribeOpenCLDevice(compiler_device.inventory));

    auto const &context = *compiler_device.context;

    auto const &program =
        ggems::ocl::GGEMSOpenCL::GetInstance().GetOrCreateProgram(
            context, probe_root, k_opencl_framework_probe_name);

    ggems::ocl::GGEMSOpenCLKernel kernel{
        context, program.CreateKernel(k_opencl_framework_probe_name),
        k_opencl_framework_probe_name};

    std::array<cl_uint, k_value_count> initial_values{1U, 2U, 3U, 4U};
    auto buffer = CreateProbeBuffer(context, initial_values);

    kernel.SetArg(0U, buffer);
    kernel.SetArg(1U, cl_uint{3U});
    kernel.Run({k_value_count}, {1U});
    EXPECT_EQ(ReadProbeBuffer(context, buffer),
              (std::array<cl_uint, k_value_count>{4U, 5U, 6U, 7U}));

    kernel.SetArg(1U, cl_uint{2U});
    auto const event = kernel.RunAndGetEvent({k_value_count}, {1U});
    EXPECT_EQ(ReadProbeBuffer(context, buffer),
              (std::array<cl_uint, k_value_count>{6U, 7U, 8U, 9U}));

    cl_int error{CL_SUCCESS};
    auto const execution_status =
        event.getInfo<CL_EVENT_COMMAND_EXECUTION_STATUS>(&error);
    ggems::ocl::CheckCLError(error,
                             "Failed to query probe event execution status.");
    EXPECT_EQ(execution_status, CL_COMPLETE);

    auto const command_type = event.getInfo<CL_EVENT_COMMAND_TYPE>(&error);
    ggems::ocl::CheckCLError(error,
                             "Failed to query probe event command type.");
    EXPECT_EQ(command_type, CL_COMMAND_NDRANGE_KERNEL);

    EXPECT_EQ(kernel.GetKernelName(), k_opencl_framework_probe_name);
    EXPECT_EQ(kernel.GetFunctionName(), k_opencl_framework_probe_name);
    EXPECT_EQ(kernel.GetNumArgs(), 2U);
    EXPECT_EQ(kernel.GetContextNative()(), context.GetContextNative()());
    EXPECT_EQ(kernel.GetProgramNative()(), program.GetProgramNative()());
    EXPECT_GE(kernel.GetWorkGroupSize(), 1U);
    EXPECT_LE(kernel.GetWorkGroupSize(),
              compiler_device.inventory.device.get().GetMaxWorkGroupSize());
    EXPECT_GE(kernel.GetPreferredWorkGroupSizeMultiple(), 1U);
  }
}
/// \endcond
