#include <filesystem>
#include <string>

#include <gtest/gtest.h>

#include "GGEMS/frameworks/GGEMSOpenCL.hh"
#include "GGEMS/frameworks/GGEMSOpenCLProgram.hh"
#include "GGEMSOpenCLCompilerDeviceInventory.hh"
#include "GGEMSOpenCLDeviceInventory.hh"
#include "GGEMSOpenCLFrameworkProbe.hh"

namespace {

using ggems::test::GetOpenCLFrameworkProbeRoot;
using ggems::test::k_opencl_framework_probe_name;

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLProgramTest,
     BuildsAndReportsCoherentMetadataOnEveryCompilerDevice) {
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

    EXPECT_NE(program.GetProgramNative()(), nullptr);
    EXPECT_EQ(program.GetKernelName(), k_opencl_framework_probe_name);
    EXPECT_FALSE(program.GetBuildOptions().empty());
    EXPECT_EQ(program.GetNumDevices(), 1U);

    auto const expected_source =
        probe_root / (std::string{k_opencl_framework_probe_name} + ".cl");
    EXPECT_EQ(std::filesystem::weakly_canonical(program.GetSourcePath()),
              std::filesystem::weakly_canonical(expected_source));

    auto const binary_sizes = program.GetBinarySizes();
    auto const binaries = program.GetBinaries();
    EXPECT_EQ(binary_sizes.size(), 1U);
    EXPECT_EQ(binaries.size(), 1U);
    if (binary_sizes.size() == 1U && binaries.size() == 1U) {
      EXPECT_EQ(binaries.front().size(), binary_sizes.front());
    }

    EXPECT_TRUE(program.Matches(context, probe_root,
                                k_opencl_framework_probe_name, ""));
    EXPECT_TRUE(program.Matches(context, probe_root / ".",
                                k_opencl_framework_probe_name, ""));
    EXPECT_FALSE(program.Matches(context, probe_root, "different_probe", ""));
    EXPECT_FALSE(program.Matches(context, probe_root.parent_path(),
                                 k_opencl_framework_probe_name, ""));
    EXPECT_FALSE(program.Matches(context, probe_root,
                                 k_opencl_framework_probe_name,
                                 "-DGGEMS_TEST_OPTION=1"));
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLProgramTest, ReusesEquivalentProgramFromMemoryCache) {
  auto const &compiler_devices =
      ggems::test::GetOpenCLCompilerDeviceInventory();
  if (compiler_devices.empty()) {
    GTEST_SKIP() << "No available GGEMS-discovered device has a compiler.";
  }

  auto const &context = *compiler_devices.front().context;
  auto const probe_root = GetOpenCLFrameworkProbeRoot();
  auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();

  auto const &first = opencl.GetOrCreateProgram(context, probe_root,
                                                k_opencl_framework_probe_name);
  auto const &second = opencl.GetOrCreateProgram(context, probe_root,
                                                 k_opencl_framework_probe_name);

  EXPECT_EQ(&first, &second);
}
