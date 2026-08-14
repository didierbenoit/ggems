#include <array>
#include <cstddef>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/frameworks/GGEMSOpenCL.hh"
#include "GGEMS/frameworks/GGEMSOpenCLContext.hh"
#include "GGEMS/frameworks/GGEMSOpenCLExternal.hh"
#include "GGEMS/frameworks/GGEMSOpenCLKernel.hh"
#include "GGEMS/frameworks/GGEMSOpenCLProfiler.hh"
#include "GGEMS/frameworks/GGEMSOpenCLUtils.hh"
#include "GGEMSOpenCLCompilerDeviceInventory.hh"
#include "GGEMSOpenCLDeviceInventory.hh"
#include "GGEMSOpenCLFrameworkProbe.hh"

namespace {

constexpr std::size_t k_value_count{4U};

using ggems::test::GetOpenCLFrameworkProbeRoot;
using ggems::test::k_opencl_framework_probe_name;

} // namespace

// =============================================================================
// =============================================================================

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
               ggems::core::GGEMSRecoverable);

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

TEST(GGEMSOpenCLProfilerTest,
     RecordsOrderedKernelTimestampsOnEveryCompilerDevice) {
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
    EXPECT_TRUE(profiler.HasKernelTiming());
    if (!profiler.HasKernelTiming()) {
      continue;
    }
    auto const &timing = profiler.GetKernelTiming();
    EXPECT_LE(timing.time_queued.value, timing.time_submit.value);
    EXPECT_LE(timing.time_submit.value, timing.time_start.value);
    EXPECT_LE(timing.time_start.value, timing.time_end.value);
    EXPECT_GE(timing.command_time.value, timing.kernel_time.value);
    EXPECT_EQ(profiler.GetCommandTime().value, timing.command_time.value);
    EXPECT_EQ(profiler.GetKernelTime().value, timing.kernel_time.value);
    EXPECT_GE(profiler.GetElapsedSeconds(), 0.0);
    EXPECT_GE(profiler.GetKernelSeconds(), 0.0);
    EXPECT_DOUBLE_EQ(profiler.ComputeKernelRatePerSecond(0U), 0.0);
  }
}
