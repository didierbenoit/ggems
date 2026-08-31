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
 * \brief Host/OpenCL ABI tests for GGEMS random-engine states.
 *
 * Validates host and OpenCL state sizes, member offsets, alignments, and array
 * strides for JKISS, PCG32, and Philox.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <span>
#include <string>
#include <utility>

#include <gtest/gtest.h>

/// \endcond
#include "GGEMS/random/GGEMSRandomState.hh"
#include "GGEMS/units/GGEMSBytesUnits.hh"
#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/opencl/GGEMSOpenCLContext.hh"
#include "GGEMS/opencl/GGEMSOpenCLKernel.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMHostAccess.hh"
#include "GGEMSOpenCLCompilerDeviceInventory.hh"
#include "GGEMSOpenCLDeviceInventory.hh"

/// \cond

namespace {

using JKissState = ggems::core::random::GGEMSJKissState;
using PCG32State = ggems::core::random::GGEMSPCG32State;
using PhiloxState = ggems::core::random::GGEMSPhiloxState;

// =============================================================================
// =============================================================================

struct JKissStateAlignmentProbe {
  std::uint8_t prefix;
  JKissState state;
};

struct PCG32StateAlignmentProbe {
  std::uint8_t prefix;
  PCG32State state;
};

struct PhiloxStateAlignmentProbe {
  std::uint8_t prefix;
  PhiloxState state;
};

// =============================================================================
// =============================================================================

constexpr std::size_t k_layout_value_count{22U};
constexpr std::size_t k_state_count{2U};

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSRandomStateKernelTest,
     HostAndKernelLayoutsMatchOnEverySVMCompilerDevice) {
  auto const &compiler_devices =
      ggems::test::GetOpenCLCompilerDeviceInventory();

  if (compiler_devices.empty()) {
    GTEST_SKIP() << "No available GGEMS-discovered device has a compiler.";
  }

  std::filesystem::path const kernel_root{GGEMS_TEST_KERNEL_ROOT};
  std::filesystem::path const kernel_test_root = kernel_root / "tests";

  std::string const build_options =
      std::format("-I{}", kernel_root.generic_string());

  std::size_t tested_device_count{0U};

  for (auto const &compiler_device : compiler_devices) {
    auto &context = *compiler_device.context;

    if (!context.GetSVMSupport().HasAny()) {
      continue;
    }

    SCOPED_TRACE(ggems::test::DescribeOpenCLDevice(compiler_device.inventory));

    ++tested_device_count;

    std::array<std::uint64_t, k_layout_value_count> layout{};

    auto layout_buffer = context.CreateSVMBuffer(
        ggems::units::Bytes{layout.size() * sizeof(std::uint64_t)});

    auto jkiss_buffer = context.CreateSVMBuffer(
        ggems::units::Bytes{k_state_count * sizeof(JKissState)});

    auto pcg32_buffer = context.CreateSVMBuffer(
        ggems::units::Bytes{k_state_count * sizeof(PCG32State)});

    auto philox_buffer = context.CreateSVMBuffer(
        ggems::units::Bytes{k_state_count * sizeof(PhiloxState)});

    auto const &program =
        ggems::ocl::GGEMSOpenCL::GetInstance().GetOrCreateProgram(
            context, kernel_test_root, "random_state_abi_probe", build_options);

    auto raw_kernel = program.CreateKernel("random_state_abi_probe");

    ggems::ocl::GGEMSOpenCLKernel kernel{context, std::move(raw_kernel),
                                         "random_state_abi_probe"};

    kernel.SetArgSVMPointer(0U, layout_buffer.GetData());
    kernel.SetArgSVMPointer(1U, jkiss_buffer.GetData());
    kernel.SetArgSVMPointer(2U, pcg32_buffer.GetData());
    kernel.SetArgSVMPointer(3U, philox_buffer.GetData());

    kernel.Run({1U}, {1U});

    ggems::ocl::ReadSVMToHost(layout_buffer, std::span{layout});

    std::array<std::uint64_t, k_layout_value_count> const expected_layout{{
        static_cast<std::uint64_t>(sizeof(JKissState)),
        static_cast<std::uint64_t>(offsetof(JKissState, x)),
        static_cast<std::uint64_t>(offsetof(JKissState, y)),
        static_cast<std::uint64_t>(offsetof(JKissState, z)),
        static_cast<std::uint64_t>(offsetof(JKissState, w)),
        static_cast<std::uint64_t>(offsetof(JKissState, c)),
        static_cast<std::uint64_t>(sizeof(JKissState)),
        static_cast<std::uint64_t>(offsetof(JKissStateAlignmentProbe, state)),

        static_cast<std::uint64_t>(sizeof(PCG32State)),
        static_cast<std::uint64_t>(offsetof(PCG32State, state)),
        static_cast<std::uint64_t>(offsetof(PCG32State, increment)),
        static_cast<std::uint64_t>(sizeof(PCG32State)),
        static_cast<std::uint64_t>(offsetof(PCG32StateAlignmentProbe, state)),

        static_cast<std::uint64_t>(sizeof(PhiloxState)),
        static_cast<std::uint64_t>(offsetof(PhiloxState, counter_0)),
        static_cast<std::uint64_t>(offsetof(PhiloxState, counter_1)),
        static_cast<std::uint64_t>(offsetof(PhiloxState, counter_2)),
        static_cast<std::uint64_t>(offsetof(PhiloxState, counter_3)),
        static_cast<std::uint64_t>(offsetof(PhiloxState, key_0)),
        static_cast<std::uint64_t>(offsetof(PhiloxState, key_1)),
        static_cast<std::uint64_t>(sizeof(PhiloxState)),
        static_cast<std::uint64_t>(offsetof(PhiloxStateAlignmentProbe, state)),
    }};

    EXPECT_EQ(layout, expected_layout);
  }

  if (tested_device_count == 0U) {
    GTEST_SKIP() << "No compiler-capable GGEMS OpenCL device supports SVM.";
  }
}
/// \endcond
