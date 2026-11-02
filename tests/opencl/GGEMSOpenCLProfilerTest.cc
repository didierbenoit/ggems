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
 * \brief Checks profiler lifecycle and native event time conversion.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/opencl/GGEMSOpenCLContext.hh"
#include "GGEMS/opencl/GGEMSOpenCLExternal.hh"
#include "GGEMS/opencl/GGEMSOpenCLKernel.hh"
#include "GGEMS/opencl/GGEMSOpenCLProfiler.hh"
#include "GGEMS/opencl/GGEMSOpenCLUtils.hh"
#include "GGEMSOpenCLDeviceInventory.hh"
#include "GGEMSOpenCLFrameworkProbe.hh"

namespace {

constexpr std::size_t k_value_count{4U};

using ggems::test::GetOpenCLFrameworkProbeRoot;
using ggems::test::k_opencl_framework_probe_name;

} // namespace

// Start, stop, and reset expose coherent measurements without requiring clock
// delays.
TEST(GGEMSOpenCLProfilerTest,
     MaintainsHostMeasurementStateWithoutTimingAssumptions) {
  ggems::ocl::GGEMSOpenCLProfiler profiler;

  EXPECT_FALSE(profiler.IsRunning());
  EXPECT_FALSE(profiler.HasMeasurement());
  EXPECT_FALSE(profiler.HasKernelTiming());
  EXPECT_EQ(profiler.GetElapsedTime().value, 0U);
  EXPECT_EQ(profiler.GetKernelTime().value, 0U);
  EXPECT_EQ(profiler.GetCommandTime().value, 0U);
  EXPECT_DOUBLE_EQ(profiler.ComputeRatePerSecond(1U), 0.0);
  EXPECT_DOUBLE_EQ(profiler.ComputeKernelRatePerSecond(1U), 0.0);

  profiler.Stop();
  EXPECT_FALSE(profiler.HasMeasurement());

  profiler.Start();
  EXPECT_TRUE(profiler.IsRunning());
  EXPECT_FALSE(profiler.HasMeasurement());
  EXPECT_THROW(profiler.RecordKernelEvent(cl::Event{}),
               ggems::core::GGEMSFatal);

  profiler.Stop();
  EXPECT_FALSE(profiler.IsRunning());
  EXPECT_TRUE(profiler.HasMeasurement());

  auto const elapsed_after_stop = profiler.GetElapsedTime();
  profiler.Stop();
  EXPECT_EQ(profiler.GetElapsedTime().value, elapsed_after_stop.value);
  EXPECT_DOUBLE_EQ(profiler.ComputeRatePerSecond(0U), 0.0);

  profiler.Start();
  EXPECT_TRUE(profiler.IsRunning());
  EXPECT_FALSE(profiler.HasMeasurement());
  EXPECT_FALSE(profiler.HasKernelTiming());

  profiler.Reset();
  EXPECT_FALSE(profiler.IsRunning());
  EXPECT_FALSE(profiler.HasMeasurement());
  EXPECT_FALSE(profiler.HasKernelTiming());
  EXPECT_EQ(profiler.GetElapsedTime().value, 0U);
}

// =============================================================================
// =============================================================================

// Native event durations convert exactly to picoseconds and survive a failed
// update.
TEST(GGEMSOpenCLProfilerTest, RecordsOrderedKernelTimestampsOnSelectedDevice) {
  auto const probe_root = GetOpenCLFrameworkProbeRoot();
  std::string const probe_name{k_opencl_framework_probe_name};

  auto const &context = ggems::test::GetSelectedOpenCLContext();
  auto const &program =
    ggems::ocl::GGEMSOpenCL::GetInstance().GetOrCreateProgram(
      context, probe_root, probe_name);

  ggems::ocl::GGEMSOpenCLKernel kernel{
    context, program.CreateKernel(probe_name), probe_name};

  std::array<cl_uint, k_value_count> values{1U, 2U, 3U, 4U};
  cl_int error{CL_SUCCESS};
  cl::Buffer buffer(context.GetContextNative(),
                    CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(values),
                    values.data(), &error);
  ggems::ocl::CheckCLError(error, "Failed to create profiler probe buffer.");

  kernel.SetArg(0U, buffer);
  kernel.SetArg(1U, cl_uint{1U});

  ggems::ocl::GGEMSOpenCLProfiler profiler;
  profiler.Start();
  auto const event = kernel.RunAndGetEvent({k_value_count}, {1U});
  profiler.Stop();
  profiler.RecordKernelEvent(event);

  EXPECT_TRUE(profiler.HasMeasurement());
  ASSERT_TRUE(profiler.HasKernelTiming());

  auto const &timing = profiler.GetKernelTiming();
  EXPECT_LE(timing.time_queued.value, timing.time_submit.value);
  EXPECT_LE(timing.time_submit.value, timing.time_start.value);
  EXPECT_LE(timing.time_start.value, timing.time_end.value);
  EXPECT_GE(timing.command_time.value, timing.kernel_time.value);
  auto const queued =
    event.getProfilingInfo<CL_PROFILING_COMMAND_QUEUED>(&error);
  ASSERT_EQ(error, CL_SUCCESS);
  auto const started =
    event.getProfilingInfo<CL_PROFILING_COMMAND_START>(&error);
  ASSERT_EQ(error, CL_SUCCESS);
  auto const ended = event.getProfilingInfo<CL_PROFILING_COMMAND_END>(&error);
  ASSERT_EQ(error, CL_SUCCESS);

  // OpenCL durations are nanoseconds; the public GGEMS duration is picoseconds.
  ASSERT_GE(ended, started);
  ASSERT_GE(started, queued);
  ASSERT_LE(ended - queued, std::numeric_limits<std::uint64_t>::max() / 1'000U);
  EXPECT_EQ(profiler.GetCommandTime().value, (ended - queued) * 1'000U);
  EXPECT_EQ(profiler.GetKernelTime().value, (ended - started) * 1'000U);

  auto const command_before = profiler.GetCommandTime();
  auto const kernel_before = profiler.GetKernelTime();
  EXPECT_THROW(profiler.RecordKernelEvent(cl::Event{}),
               ggems::core::GGEMSFatal);
  EXPECT_TRUE(profiler.HasKernelTiming());
  EXPECT_EQ(profiler.GetCommandTime(), command_before);
  EXPECT_EQ(profiler.GetKernelTime(), kernel_before);
  EXPECT_GE(profiler.GetElapsedSeconds(), 0.0);
  EXPECT_GE(profiler.GetKernelSeconds(), 0.0);
  EXPECT_DOUBLE_EQ(profiler.ComputeKernelRatePerSecond(0U), 0.0);
}
