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
 * \brief Unit tests for the GGEMS OpenCL facade.
 *
 * Validates singleton identity, discovery hierarchy coherence, device-selector
 * error handling, and the Initialize() freeze and program-cache contract.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <algorithm>
#include <cstddef>
#include <array>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>

/// \endcond
#include "GGEMS/GGEMSException.hh"
#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/opencl/GGEMSOpenCLContext.hh"
#include "GGEMS/opencl/GGEMSOpenCLDevice.hh"
#include "GGEMS/opencl/GGEMSOpenCLExternal.hh"
#include "GGEMS/opencl/GGEMSOpenCLKernel.hh"
#include "GGEMS/opencl/GGEMSOpenCLPlatform.hh"
#include "GGEMS/opencl/GGEMSOpenCLProgram.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMBuffer.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMHostAccess.hh"
#include "GGEMS/units/GGEMSBytesUnits.hh"
#include "GGEMSOpenCLDeviceInventory.hh"
#include "GGEMSOpenCLFrameworkProbe.hh"

/// \cond

namespace {

/*!
 * \brief Returns flattened indices of discovered devices usable for kernels.
 *
 * \return Flattened discovery indices of available devices exposing a compiler.
 */
[[nodiscard]] auto GetUsableDeviceIndices() -> std::vector<std::size_t> {
  std::vector<std::size_t> indices;
  std::size_t flat_index{0U};

  for (auto const &entry : ggems::test::GetOpenCLDeviceInventory()) {
    auto const &device = entry.device.get();

    if (device.GetAvailable() != CL_FALSE &&
        device.GetCompilerAvailable() != CL_FALSE) {
      indices.push_back(flat_index);
    }

    ++flat_index;
  }

  return indices;
}

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLTest, SingletonIdentityIsStable) {
  auto &first = ggems::ocl::GGEMSOpenCL::GetInstance();
  auto &second = ggems::ocl::GGEMSOpenCL::GetInstance();

  EXPECT_EQ(&first, &second);
  EXPECT_EQ(&first.GetPlatforms(), &second.GetPlatforms());
}

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLTest, DiscoveryHierarchyIsCoherent) {
  auto const &platforms = ggems::ocl::GGEMSOpenCL::GetInstance().GetPlatforms();
  ASSERT_FALSE(platforms.empty());

  auto const inventory = ggems::test::GetOpenCLDeviceInventory();
  std::size_t discovered_device_count{0U};
  for (auto const &platform : platforms) {
    discovered_device_count += platform.GetDevices().size();
  }
  EXPECT_EQ(inventory.size(), discovered_device_count);

  for (auto const &entry : inventory) {
    SCOPED_TRACE(ggems::test::DescribeOpenCLDevice(entry));

    auto const &platform = entry.platform.get();
    auto const &device = entry.device.get();

    auto const platform_iterator =
        std::ranges::find_if(platforms, [&](auto const &candidate) -> bool {
          return &candidate == &platform;
        });
    EXPECT_NE(platform_iterator, platforms.end());
    if (platform_iterator == platforms.end()) {
      continue;
    }

    auto const &devices = platform.GetDevices();
    auto const device_iterator =
        std::ranges::find_if(devices, [&](auto const &candidate) -> bool {
          return &candidate == &device;
        });
    EXPECT_NE(device_iterator, devices.end());

    EXPECT_NE(platform.GetPlatformNative()(), nullptr);
    EXPECT_NE(device.GetDeviceNative()(), nullptr);
    EXPECT_EQ(device.GetPlatformID(), platform.GetPlatformNative()());
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLTest, RejectsInvalidDeviceSelectors) {
  auto const inventory = ggems::test::GetOpenCLDeviceInventory();
  ASSERT_FALSE(inventory.empty());

  constexpr std::array<std::string_view, 7> invalid_selectors{
      "toot", "all;gpu", "0;gpu", "1-0", "cpu;gpu", "intel;nvidia", "0;;1",
  };

  auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();

  for (auto const selector : invalid_selectors) {
    SCOPED_TRACE(selector);

    EXPECT_THROW(opencl.SelectDevices({std::string{selector}}),
                 ggems::core::GGEMSFatal);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLTest, RejectsOutOfRangeDeviceIndex) {
  auto const inventory = ggems::test::GetOpenCLDeviceInventory();
  ASSERT_FALSE(inventory.empty());

  auto const out_of_range_index = std::to_string(inventory.size());

  EXPECT_THROW(ggems::ocl::GGEMSOpenCL::GetInstance().SelectDevices(
                   {out_of_range_index}),
               ggems::core::GGEMSFatal);
}

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLTest, FirstInitializeKeepsAlreadyCachedPrograms) {
  auto const inventory = ggems::test::GetOpenCLDeviceInventory();
  auto const usable_devices = GetUsableDeviceIndices();
  if (usable_devices.empty()) {
    GTEST_SKIP() << "No available GGEMS-discovered device has a compiler.";
  }

  auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();
  if (!opencl.GetContext().empty()) {
    GTEST_SKIP() << "OpenCL contexts already exist in this process.";
  }

  auto const &device = inventory[usable_devices.front()].device.get();
  SCOPED_TRACE(device.GetName());

  // A program cached through a caller-owned context before the backend owns
  // any context must survive the first context creation.
  ggems::ocl::GGEMSOpenCLContext owned_context{device};

  auto const probe_root = ggems::test::GetOpenCLFrameworkProbeRoot();
  auto const &program_before = opencl.GetOrCreateProgram(
      owned_context, probe_root, ggems::test::k_opencl_framework_probe_name);
  cl::Program const retained_program{program_before.GetProgramNative()};

  opencl.SelectDevices({std::to_string(usable_devices.front())});
  opencl.Initialize();

  ASSERT_FALSE(opencl.GetContext().empty());

  auto const &program_after = opencl.GetOrCreateProgram(
      owned_context, probe_root, ggems::test::k_opencl_framework_probe_name);

  EXPECT_EQ(&program_after, &program_before);
  EXPECT_EQ(program_after.GetProgramNative()(), retained_program());
}

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLTest, SecondInitializeWithSameSelectionKeepsBackendState) {
  auto const usable_devices = GetUsableDeviceIndices();
  if (usable_devices.empty()) {
    GTEST_SKIP() << "No available GGEMS-discovered device has a compiler.";
  }

  auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();

  opencl.SelectDevices({std::to_string(usable_devices.front())});
  opencl.Initialize();

  auto &contexts = opencl.GetContext();
  ASSERT_EQ(contexts.size(), 1U);

  auto &context = contexts[0];
  SCOPED_TRACE(context.GetDevice().GetName());

  auto const *const context_before = &context;
  auto const *const device_before = &context.GetDevice();

  // Retaining the native handles prevents the OpenCL runtime from recycling
  // handle values, so handle equality really proves that the same objects are
  // still in place.
  cl::Context const retained_context{context.GetContextNative()};

  auto const probe_root = ggems::test::GetOpenCLFrameworkProbeRoot();
  auto const &program = opencl.GetOrCreateProgram(
      context, probe_root, ggems::test::k_opencl_framework_probe_name);
  auto const *const program_before = &program;
  cl::Program const retained_program{program.GetProgramNative()};

  ggems::ocl::GGEMSOpenCLKernel kernel{
      context, program.CreateKernel(ggems::test::k_opencl_framework_probe_name),
      ggems::test::k_opencl_framework_probe_name};

  constexpr std::size_t k_value_count{4U};
  auto const allocation_count_before = context.GetAllocationCountVRAM();
  auto const allocated_before = context.GetAllocatedVRAM();

  std::optional<ggems::ocl::GGEMSOpenCLSVMBuffer> buffer;
  if (context.GetSVMSupport().HasAny()) {
    buffer.emplace(context.CreateSVMBuffer(
        ggems::units::Bytes{k_value_count * sizeof(cl_uint)}));

    std::array<cl_uint, k_value_count> const initial_values{1U, 2U, 3U, 4U};
    ggems::ocl::WriteSVMFromHost(*buffer, std::span{initial_values});
  }

  // Second initialization with the selection that produced the contexts.
  EXPECT_NO_THROW(opencl.Initialize());

  auto &contexts_after = opencl.GetContext();
  ASSERT_EQ(contexts_after.size(), 1U);

  auto &context_after = contexts_after.front();
  EXPECT_EQ(&context_after, context_before);
  EXPECT_EQ(&context_after.GetDevice(), device_before);
  EXPECT_EQ(context_after.GetContextNative()(), retained_context());

  // A kernel kept alive across the second initialization still refers to the
  // live GGEMS context.
  EXPECT_EQ(&kernel.GetContext(), context_before);
  EXPECT_EQ(kernel.GetContextNative()(), retained_context());

  // A program reference obtained before the second initialization is still the
  // cached program.
  auto const &program_after = opencl.GetOrCreateProgram(
      context_after, probe_root, ggems::test::k_opencl_framework_probe_name);
  EXPECT_EQ(&program_after, program_before);
  EXPECT_EQ(program_after.GetProgramNative()(), retained_program());

  if (buffer.has_value()) {
    EXPECT_EQ(context_after.GetAllocationCountVRAM(),
              allocation_count_before + 1U);

    // An SVM buffer allocated before the second initialization is still owned
    // by a live context and still usable by the kernel.
    kernel.SetArgSVMPointer(0U, buffer->GetData());
    kernel.SetArg(1U, cl_uint{3U});
    kernel.Run({k_value_count}, {1U});

    std::array<cl_uint, k_value_count> observed_values{};
    ggems::ocl::ReadSVMToHost(*buffer, std::span{observed_values});
    EXPECT_EQ(observed_values,
              (std::array<cl_uint, k_value_count>{4U, 5U, 6U, 7U}));

    buffer.reset();

    EXPECT_EQ(context_after.GetAllocationCountVRAM(), allocation_count_before);
    EXPECT_EQ(context_after.GetAllocatedVRAM(), allocated_before);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLTest, FreezesDeviceSelectionAfterInitialize) {
  auto const inventory = ggems::test::GetOpenCLDeviceInventory();
  auto const usable_devices = GetUsableDeviceIndices();
  if (usable_devices.empty()) {
    GTEST_SKIP() << "No available GGEMS-discovered device has a compiler.";
  }

  auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();

  auto const active_index = usable_devices.front();
  auto const active_text = std::to_string(active_index);

  std::vector<std::string> const active_selector{active_text};
  auto const range_text = active_text + "-" + active_text;
  std::vector<std::string> const range_selector{range_text};
  std::vector<std::string> const duplicate_selector{
      active_text,
      active_text,
  };

  // Before the contexts exist, the selection can still be changed freely.
  if (opencl.GetContext().empty() && usable_devices.size() >= 2U) {
    std::vector<std::string> const other_selector{
        std::to_string(usable_devices[1]),
    };
    EXPECT_NO_THROW(opencl.SelectDevices(other_selector));
  }

  EXPECT_NO_THROW(opencl.SelectDevices(active_selector));
  opencl.Initialize();

  auto &contexts = opencl.GetContext();
  ASSERT_EQ(contexts.size(), 1U);

  auto const &context = contexts.front();
  SCOPED_TRACE(context.GetDevice().GetName());

  auto const *const context_before = &context;
  auto const *const device_before = &context.GetDevice();
  cl::Context const retained_context{context.GetContextNative()};

  auto const probe_root = ggems::test::GetOpenCLFrameworkProbeRoot();
  auto const &program_before = opencl.GetOrCreateProgram(
      context, probe_root, ggems::test::k_opencl_framework_probe_name);
  cl::Program const retained_program{program_before.GetProgramNative()};

  // Requests are resolved to devices before they are compared, so selector
  // expressions that designate the active devices are accepted whatever their
  // spelling.
  EXPECT_NO_THROW(opencl.SelectDevices(active_selector));
  EXPECT_NO_THROW(opencl.SelectDevices(range_selector));
  EXPECT_NO_THROW(opencl.SelectDevices(duplicate_selector));

  // A request that really designates other devices is rejected.
  if (usable_devices.size() >= 2U) {
    std::vector<std::string> const other_selector{
        std::to_string(usable_devices[1]),
    };
    EXPECT_THROW(opencl.SelectDevices(other_selector), ggems::core::GGEMSFatal);
  }

  if (inventory.size() >= 2U) {
    std::vector<std::string> const all_selector{"all"};
    EXPECT_THROW(opencl.SelectDevices(all_selector), ggems::core::GGEMSFatal);
  }

  // The rejected requests left the active contexts untouched.
  auto &contexts_after = opencl.GetContext();
  ASSERT_EQ(contexts_after.size(), 1U);

  auto const &context_after = contexts_after.front();
  EXPECT_EQ(&context_after, context_before);
  EXPECT_EQ(&context_after.GetDevice(), device_before);
  EXPECT_EQ(context_after.GetContextNative()(), retained_context());

  // An equivalent request is still accepted after the rejections. The freeze
  // compares against the active contexts, so this assertion does not observe
  // the private stored selection; that one is protected by construction,
  // because a rejection returns before the assignment.
  EXPECT_NO_THROW(opencl.SelectDevices(active_selector));

  // The program cache survived the rejected requests.
  auto const &program_after = opencl.GetOrCreateProgram(
      context_after, probe_root, ggems::test::k_opencl_framework_probe_name);
  EXPECT_EQ(&program_after, &program_before);
  EXPECT_EQ(program_after.GetProgramNative()(), retained_program());

  // A later initialization is still a no-op on the same contexts.
  EXPECT_NO_THROW(opencl.Initialize());
  ASSERT_EQ(opencl.GetContext().size(), 1U);
  EXPECT_EQ(&opencl.GetContext().front(), context_before);
  EXPECT_EQ(opencl.GetContext().front().GetContextNative()(),
            retained_context());
}
// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLTest, KeepsBackendFrozenWhenTheContextCollectionIsEmptied) {
  auto const usable_devices = GetUsableDeviceIndices();
  if (usable_devices.empty()) {
    GTEST_SKIP() << "No available GGEMS-discovered device has a compiler.";
  }

  auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();

  std::vector<std::string> const active_selector{
      std::to_string(usable_devices.front()),
  };

  EXPECT_NO_THROW(opencl.SelectDevices(active_selector));
  opencl.Initialize();
  ASSERT_FALSE(opencl.GetContext().empty());

  // Deliberately hostile state: a caller empties the exposed collection. The
  // backend stays initialized, so nothing can rebuild the contexts or reopen
  // the device selection afterwards.
  opencl.GetContext().clear();

  if (usable_devices.size() >= 2U) {
    std::vector<std::string> const other_selector{
        std::to_string(usable_devices[1]),
    };
    EXPECT_THROW(opencl.SelectDevices(other_selector), ggems::core::GGEMSFatal);
  }

  EXPECT_NO_THROW(opencl.Initialize());
  EXPECT_TRUE(opencl.GetContext().empty());
}

/// \endcond
