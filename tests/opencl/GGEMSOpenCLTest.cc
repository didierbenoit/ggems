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
 * \brief Checks backend initialization, selection rejection, and resource
 * lifetime.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <array>
#include <cstdlib>
#include <span>
#include <string>
#include <string_view>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/opencl/GGEMSOpenCLContext.hh"
#include "GGEMS/opencl/GGEMSOpenCLExternal.hh"
#include "GGEMS/opencl/GGEMSOpenCLKernel.hh"
#include "GGEMS/opencl/GGEMSOpenCLProgram.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMBuffer.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMHostAccess.hh"
#include "GGEMS/units/GGEMSBytesUnits.hh"
#include "GGEMSOpenCLDeviceInventory.hh"
#include "GGEMSOpenCLFrameworkProbe.hh"

// A fresh process verifies first initialization without depending on test
// order.
TEST(GGEMSOpenCLDeathTest, FirstInitializeKeepsAlreadyCachedPrograms) {
  GTEST_FLAG_SET(death_test_style, "threadsafe");
  ASSERT_EXIT(
    {
      auto const &device = ggems::test::GetSelectedOpenCLDevice().device.get();
      auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();
      ggems::ocl::GGEMSOpenCLContext owned_context{device};
      auto const probe_root = ggems::test::GetOpenCLFrameworkProbeRoot();
      std::string const probe_name{ggems::test::k_opencl_framework_probe_name};
      auto const &before =
        opencl.GetOrCreateProgram(owned_context, probe_root, probe_name);
      cl::Program const retained{before.GetProgramNative()};

      opencl.Initialize();

      auto const &after =
        opencl.GetOrCreateProgram(owned_context, probe_root, probe_name);
      bool const preserved = opencl.GetContext().size() == 1U &&
                             &before == &after &&
                             after.GetProgramNative()() == retained();
      std::exit(preserved ? EXIT_SUCCESS : EXIT_FAILURE);
    },
    testing::ExitedWithCode(EXIT_SUCCESS), "");
}

// Invalid user selectors fail before changing an initialized backend.
TEST(GGEMSOpenCLTest, RejectsInvalidDeviceSelectorsWithoutChangingContext) {
  auto const &context = ggems::test::GetSelectedOpenCLContext();
  auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();
  auto *const native_context = context.GetContextNative()();
  constexpr std::array<std::string_view, 8> invalid_selectors{
    "toot",    "all;gpu",      "0;gpu", "1-0",
    "cpu;gpu", "intel;nvidia", "0;;1",  "18446744073709551615",
  };

  for (auto const selector : invalid_selectors) {
    SCOPED_TRACE(selector);
    EXPECT_THROW(opencl.SelectDevices({std::string{selector}}),
                 ggems::core::GGEMSFatal);
    ASSERT_EQ(opencl.GetContext().size(), 1U);
    EXPECT_EQ(opencl.GetContext().front().GetContextNative()(), native_context);
  }
}

// Equivalent selection and repeated initialization keep live kernels and SVM
// valid.
TEST(GGEMSOpenCLTest,
     ReinitializationPreservesLiveResourcesAndEquivalentSelectors) {
  auto &context = ggems::test::GetSelectedOpenCLContext();
  auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();
  auto const selected_index =
    std::to_string(ggems::test::GetSelectedOpenCLDevice().flat_index);
  cl::Context const retained_context{context.GetContextNative()};
  auto const probe_root = ggems::test::GetOpenCLFrameworkProbeRoot();
  std::string const probe_name{ggems::test::k_opencl_framework_probe_name};
  auto const &program =
    opencl.GetOrCreateProgram(context, probe_root, probe_name);
  cl::Program const retained_program{program.GetProgramNative()};
  ggems::ocl::GGEMSOpenCLKernel kernel{
    context, program.CreateKernel(probe_name), probe_name};
  auto const allocation_count_before = context.GetAllocationCountVRAM();
  auto const allocated_before = context.GetAllocatedVRAM();

  {
    std::array<cl_uint, 4> const initial_values{1U, 2U, 3U, 4U};
    auto buffer =
      context.CreateSVMBuffer(ggems::units::Bytes{sizeof(initial_values)});
    ggems::ocl::WriteSVMFromHost(buffer, std::span{initial_values});

    EXPECT_NO_THROW(opencl.SelectDevices({selected_index}));
    EXPECT_NO_THROW(
      opencl.SelectDevices({selected_index + "-" + selected_index}));
    EXPECT_NO_THROW(opencl.SelectDevices({selected_index, selected_index}));
    EXPECT_NO_THROW(opencl.Initialize());

    ASSERT_EQ(opencl.GetContext().size(), 1U);
    EXPECT_EQ(&opencl.GetContext().front(), &context);
    EXPECT_EQ(context.GetContextNative()(), retained_context());
    EXPECT_EQ(&kernel.GetContext(), &context);
    EXPECT_EQ(&opencl.GetOrCreateProgram(context, probe_root, probe_name),
              &program);
    EXPECT_EQ(program.GetProgramNative()(), retained_program());
    EXPECT_EQ(context.GetAllocationCountVRAM(), allocation_count_before + 1U);

    kernel.SetArgSVMPointer(0U, buffer.GetData());
    kernel.SetArg(1U, cl_uint{3U});
    kernel.Run({initial_values.size()}, {1U});

    std::array<cl_uint, 4> actual{};
    ggems::ocl::ReadSVMToHost(buffer, std::span{actual});
    EXPECT_EQ(actual, (std::array<cl_uint, 4>{4U, 5U, 6U, 7U}));
  }

  EXPECT_EQ(context.GetAllocationCountVRAM(), allocation_count_before);
  EXPECT_EQ(context.GetAllocatedVRAM(), allocated_before);
}
