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
 * \brief Unit tests for GGEMS OpenCL program management.
 *
 * Validates build metadata, in-memory program-cache reuse, and cached-program lifetime independently of the context wrapper used to create it.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <filesystem>
#include <string>

#include <gtest/gtest.h>

/// \endcond
#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/opencl/GGEMSOpenCLProgram.hh"
#include "GGEMSOpenCLCompilerDeviceInventory.hh"
#include "GGEMSOpenCLDeviceInventory.hh"
#include "GGEMSOpenCLFrameworkProbe.hh"

/// \cond

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

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLProgramTest, CachedProgramOutlivesContextWrapper) {
  auto const &compiler_devices =
      ggems::test::GetOpenCLCompilerDeviceInventory();

  if (compiler_devices.empty()) {
    GTEST_SKIP() << "No available GGEMS-discovered device has a compiler.";
  }

  auto const &device = compiler_devices.front().inventory.device.get();
  auto const probe_root = GetOpenCLFrameworkProbeRoot();
  auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();

  ggems::ocl::GGEMSOpenCLProgram const *cached_program{nullptr};

  {
    ggems::ocl::GGEMSOpenCLContext context{device};

    cached_program = &opencl.GetOrCreateProgram(context, probe_root,
                                                k_opencl_framework_probe_name);

    EXPECT_TRUE(cached_program->Matches(context, probe_root,
                                        k_opencl_framework_probe_name, ""));
  }

  ASSERT_NE(cached_program, nullptr);
  EXPECT_NE(cached_program->GetProgramNative()(), nullptr);
  EXPECT_EQ(cached_program->GetNumDevices(), 1U);

  auto const kernel =
      cached_program->CreateKernel(k_opencl_framework_probe_name);

  EXPECT_NE(kernel(), nullptr);
}
/// \endcond
