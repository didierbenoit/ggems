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
 * \brief OpenCL validation tests for the GGEMS JKISS engine.
 *
 * Validates kernel-side JKISS uniform output, deterministic state progression, stream independence, and host/kernel state agreement.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <string>
#include <vector>
#include <utility>

#include <gtest/gtest.h>

/// \endcond
#include "GGEMS/frameworks/GGEMSOpenCLExternal.hh"
#include "GGEMS/frameworks/GGEMSOpenCLLaunchGeometry.hh"
#include "GGEMS/frameworks/GGEMSOpenCL.hh"
#include "GGEMS/frameworks/GGEMSOpenCLKernel.hh"
#include "GGEMS/frameworks/GGEMSOpenCLSVMBuffer.hh"
#include "GGEMS/core/units/GGEMSBytesUnits.hh"
#include "GGEMS/core/random/GGEMSRandomState.hh"
#include "GGEMSOpenCLCompilerDeviceInventory.hh"
#include "GGEMSOpenCLDeviceInventory.hh"

/// \cond

namespace {

using JKissState = ggems::core::random::GGEMSJKissState;

// =============================================================================
// =============================================================================

constexpr std::uint32_t k_seed{77777U};
constexpr std::uint32_t k_alternative_seed{77778U};

constexpr std::size_t k_particle_count{1024U};
constexpr std::size_t k_samples_per_particle{8U};
constexpr std::size_t k_value_count{k_particle_count * k_samples_per_particle};
constexpr std::size_t k_local_size{64U};

constexpr std::size_t k_uniform4_blocks_per_particle{2U};
constexpr std::size_t k_uniform4_value_count{
    k_particle_count * k_uniform4_blocks_per_particle * 4U};

constexpr auto k_padded_global_work_size =
    ggems::ocl::detail::TryComputePaddedGlobalWorkSize(k_particle_count,
                                                       k_local_size);

static_assert(k_padded_global_work_size.has_value());

constexpr std::size_t k_global_work_size = *k_padded_global_work_size;

// =============================================================================
// =============================================================================

auto MakeJKissState(std::uint32_t seed, std::uint32_t index) noexcept
    -> JKissState {
  return JKissState{.x = seed + 123456789U + (1013904223U * index),
                    .y = seed ^ (362436069U + (1664525U * index)),
                    .z = seed + 521288629U + (69069U * index),
                    .w = seed ^ (88675123U + (22695477U * index)),
                    .c = index & 1U};
}

// =============================================================================
// =============================================================================

auto AreStatesEqual(JKissState const &lhs, JKissState const &rhs) noexcept
    -> bool {
  return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.w == rhs.w &&
         lhs.c == rhs.c;
}

// =============================================================================
// =============================================================================

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSJKissKernelTest, UniformValuesAreInsideUnitInterval) {
  auto const &compiler_devices =
      ggems::test::GetOpenCLCompilerDeviceInventory();

  std::size_t tested_device_count{0U};

  for (auto const &compiler_device : compiler_devices) {
    auto &context = *compiler_device.context;

    if (!context.GetSVMSupport().HasAny()) {
      continue;
    }

    SCOPED_TRACE(
        ggems::test::DescribeOpenCLDevice(compiler_device.inventory));

    ++tested_device_count;

    auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();

    std::filesystem::path kernel_root{GGEMS_TEST_KERNEL_ROOT};
    std::filesystem::path kernel_test_root = kernel_root / "tests";

    std::string build_options =
        std::format("-cl-std=CL2.0 -I{}", kernel_root.generic_string());

    auto const &program = opencl.GetOrCreateProgram(
        context, kernel_test_root, "random_jkiss_uniform", build_options);

    cl::Kernel raw_kernel = program.CreateKernel("random_jkiss_uniform");

    ggems::ocl::GGEMSOpenCLKernel kernel{context, std::move(raw_kernel),
                                         "random_jkiss_uniform"};

    std::size_t state_bytes =
        k_particle_count * sizeof(ggems::core::random::GGEMSJKissState);
    std::size_t value_bytes = k_value_count * sizeof(float);

    auto states_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{state_bytes});
    auto values_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{value_bytes});

    auto *states = static_cast<JKissState *>(states_buffer.GetData());
    auto *values = static_cast<float *>(values_buffer.GetData());

    states_buffer.Map(CL_MAP_WRITE);
    values_buffer.Map(CL_MAP_WRITE);

    for (std::size_t i = 0U; i < k_particle_count; ++i) {
      states[i] = MakeJKissState(k_seed, static_cast<std::uint32_t>(i));
    }

    std::fill(values, values + k_value_count, -1.0F);

    values_buffer.Unmap();
    states_buffer.Unmap();

    kernel.SetArgSVMPointer(0U, states);
    kernel.SetArgSVMPointer(1U, values);
    kernel.SetArg(2U, static_cast<cl_uint>(k_particle_count));
    kernel.SetArg(3U, static_cast<cl_uint>(k_samples_per_particle));

    kernel.Run({k_global_work_size}, {k_local_size});

    values_buffer.Map(CL_MAP_READ);

    for (std::size_t i = 0U; i < k_value_count; ++i) {
      EXPECT_GE(values[i], 0.0F);
      EXPECT_LT(values[i], 1.0F);
    }

    values_buffer.Unmap();
  }

  if (tested_device_count == 0U) {
    GTEST_SKIP()
        << "No compiler-capable GGEMS OpenCL device supports SVM.";
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSJKissKernelTest, SequenceContinuesBetweenKernelCalls) {
  auto const &compiler_devices =
      ggems::test::GetOpenCLCompilerDeviceInventory();

  std::size_t tested_device_count{0U};

  for (auto const &compiler_device : compiler_devices) {
    auto &context = *compiler_device.context;

    if (!context.GetSVMSupport().HasAny()) {
      continue;
    }

    SCOPED_TRACE(
        ggems::test::DescribeOpenCLDevice(compiler_device.inventory));

    ++tested_device_count;

    auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();

    std::filesystem::path kernel_root{GGEMS_TEST_KERNEL_ROOT};
    std::filesystem::path kernel_test_root = kernel_root / "tests";

    std::string build_options =
        std::format("-cl-std=CL2.0 -I{}", kernel_root.generic_string());

    auto const &program = opencl.GetOrCreateProgram(
        context, kernel_test_root, "random_jkiss_uniform", build_options);

    cl::Kernel raw_kernel = program.CreateKernel("random_jkiss_uniform");

    ggems::ocl::GGEMSOpenCLKernel kernel{context, std::move(raw_kernel),
                                         "random_jkiss_uniform"};

    std::size_t state_bytes = k_particle_count * sizeof(JKissState);
    std::size_t value_bytes = k_value_count * sizeof(float);

    auto states_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{state_bytes});
    auto values_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{value_bytes});

    auto *states = static_cast<JKissState *>(states_buffer.GetData());
    auto *values = static_cast<float *>(values_buffer.GetData());

    states_buffer.Map(CL_MAP_WRITE);
    values_buffer.Map(CL_MAP_WRITE);

    for (std::size_t i = 0U; i < k_particle_count; ++i) {
      states[i] = MakeJKissState(k_seed, static_cast<std::uint32_t>(i));
    }

    std::fill(values, values + k_value_count, -1.0F);

    values_buffer.Unmap();
    states_buffer.Unmap();

    kernel.SetArgSVMPointer(0U, states);
    kernel.SetArgSVMPointer(1U, values);
    kernel.SetArg(2U, static_cast<cl_uint>(k_particle_count));
    kernel.SetArg(3U, static_cast<cl_uint>(k_samples_per_particle));

    kernel.Run({k_global_work_size}, {k_local_size});

    std::vector<float> first_values(k_value_count);
    values_buffer.Map(CL_MAP_READ);
    std::copy(values, values + k_value_count, first_values.begin());
    values_buffer.Unmap();

    kernel.Run({k_global_work_size}, {k_local_size});

    values_buffer.Map(CL_MAP_READ);

    bool sequence_has_advanced{false};

    for (std::size_t i = 0U; i < k_value_count; ++i) {
      EXPECT_GE(values[i], 0.0F);
      EXPECT_LT(values[i], 1.0F);

      if (values[i] != first_values[i]) {
        sequence_has_advanced = true;
      }
    }

    values_buffer.Unmap();

    EXPECT_TRUE(sequence_has_advanced);
  }

  if (tested_device_count == 0U) {
    GTEST_SKIP()
        << "No compiler-capable GGEMS OpenCL device supports SVM.";
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSJKissKernelTest, SameSeedProducesSameFirstSequence) {
  auto const &compiler_devices =
      ggems::test::GetOpenCLCompilerDeviceInventory();

  std::size_t tested_device_count{0U};

  for (auto const &compiler_device : compiler_devices) {
    auto &context = *compiler_device.context;

    if (!context.GetSVMSupport().HasAny()) {
      continue;
    }

    SCOPED_TRACE(
        ggems::test::DescribeOpenCLDevice(compiler_device.inventory));

    ++tested_device_count;

    auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();

    std::filesystem::path kernel_root{GGEMS_TEST_KERNEL_ROOT};
    std::filesystem::path kernel_test_root = kernel_root / "tests";

    std::string build_options =
        std::format("-cl-std=CL2.0 -I{}", kernel_root.generic_string());

    auto const &program = opencl.GetOrCreateProgram(
        context, kernel_test_root, "random_jkiss_uniform", build_options);

    cl::Kernel raw_kernel = program.CreateKernel("random_jkiss_uniform");

    ggems::ocl::GGEMSOpenCLKernel kernel{context, std::move(raw_kernel),
                                         "random_jkiss_uniform"};

    std::size_t state_bytes = k_particle_count * sizeof(JKissState);
    std::size_t value_bytes = k_value_count * sizeof(float);

    auto states_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{state_bytes});
    auto values_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{value_bytes});

    auto *states = static_cast<JKissState *>(states_buffer.GetData());
    auto *values = static_cast<float *>(values_buffer.GetData());

    auto initialize_states = [&]() -> void {
      states_buffer.Map(CL_MAP_WRITE);
      values_buffer.Map(CL_MAP_WRITE);

      for (std::size_t i = 0U; i < k_particle_count; ++i) {
        states[i] = MakeJKissState(k_seed, static_cast<std::uint32_t>(i));
      }

      std::fill(values, values + k_value_count, -1.0F);

      values_buffer.Unmap();
      states_buffer.Unmap();
    };

    kernel.SetArgSVMPointer(0U, states);
    kernel.SetArgSVMPointer(1U, values);
    kernel.SetArg(2U, static_cast<cl_uint>(k_particle_count));
    kernel.SetArg(3U, static_cast<cl_uint>(k_samples_per_particle));

    initialize_states();

    kernel.Run({k_global_work_size}, {k_local_size});

    std::vector<float> first_values(k_value_count);

    values_buffer.Map(CL_MAP_READ);
    std::copy(values, values + k_value_count, first_values.begin());
    values_buffer.Unmap();

    initialize_states();

    kernel.Run({k_global_work_size}, {k_local_size});

    values_buffer.Map(CL_MAP_READ);

    for (std::size_t i = 0U; i < k_value_count; ++i) {
      EXPECT_EQ(values[i], first_values[i]);
    }

    values_buffer.Unmap();
  }

  if (tested_device_count == 0U) {
    GTEST_SKIP()
        << "No compiler-capable GGEMS OpenCL device supports SVM.";
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSJKissKernelTest, DifferentSeedsProduceDifferentFirstSequence) {
  auto const &compiler_devices =
      ggems::test::GetOpenCLCompilerDeviceInventory();

  std::size_t tested_device_count{0U};

  for (auto const &compiler_device : compiler_devices) {
    auto &context = *compiler_device.context;

    if (!context.GetSVMSupport().HasAny()) {
      continue;
    }

    SCOPED_TRACE(
        ggems::test::DescribeOpenCLDevice(compiler_device.inventory));

    ++tested_device_count;

    auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();

    std::filesystem::path kernel_root{GGEMS_TEST_KERNEL_ROOT};
    std::filesystem::path kernel_test_root = kernel_root / "tests";

    std::string build_options =
        std::format("-cl-std=CL2.0 -I{}", kernel_root.generic_string());

    auto const &program = opencl.GetOrCreateProgram(
        context, kernel_test_root, "random_jkiss_uniform", build_options);

    cl::Kernel raw_kernel = program.CreateKernel("random_jkiss_uniform");

    ggems::ocl::GGEMSOpenCLKernel kernel{context, std::move(raw_kernel),
                                         "random_jkiss_uniform"};

    std::size_t state_bytes = k_particle_count * sizeof(JKissState);
    std::size_t value_bytes = k_value_count * sizeof(float);

    auto states_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{state_bytes});
    auto values_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{value_bytes});

    auto *states = static_cast<JKissState *>(states_buffer.GetData());
    auto *values = static_cast<float *>(values_buffer.GetData());

    auto initialize_states = [&](std::uint32_t seed) -> void {
      states_buffer.Map(CL_MAP_WRITE);
      values_buffer.Map(CL_MAP_WRITE);

      for (std::size_t i = 0U; i < k_particle_count; ++i) {
        states[i] = MakeJKissState(seed, static_cast<std::uint32_t>(i));
      }

      std::fill(values, values + k_value_count, -1.0F);

      values_buffer.Unmap();
      states_buffer.Unmap();
    };

    kernel.SetArgSVMPointer(0U, states);
    kernel.SetArgSVMPointer(1U, values);
    kernel.SetArg(2U, static_cast<cl_uint>(k_particle_count));
    kernel.SetArg(3U, static_cast<cl_uint>(k_samples_per_particle));

    initialize_states(k_seed);

    kernel.Run({k_global_work_size}, {k_local_size});

    std::vector<float> first_seed_values(k_value_count);

    values_buffer.Map(CL_MAP_READ);
    std::copy(values, values + k_value_count, first_seed_values.begin());
    values_buffer.Unmap();

    initialize_states(k_alternative_seed);

    kernel.Run({k_global_work_size}, {k_local_size});

    values_buffer.Map(CL_MAP_READ);

    std::size_t different_value_count{0U};

    for (std::size_t i = 0U; i < k_value_count; ++i) {
      EXPECT_GE(values[i], 0.0F);
      EXPECT_LT(values[i], 1.0F);

      if (values[i] != first_seed_values[i]) {
        ++different_value_count;
      }
    }

    values_buffer.Unmap();

    EXPECT_GT(different_value_count, k_value_count / 2U);
  }

  if (tested_device_count == 0U) {
    GTEST_SKIP()
        << "No compiler-capable GGEMS OpenCL device supports SVM.";
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSJKissKernelTest, RandomStatesAreAdvancedByKernelExecution) {
  auto const &compiler_devices =
      ggems::test::GetOpenCLCompilerDeviceInventory();

  std::size_t tested_device_count{0U};

  for (auto const &compiler_device : compiler_devices) {
    auto &context = *compiler_device.context;

    if (!context.GetSVMSupport().HasAny()) {
      continue;
    }

    SCOPED_TRACE(
        ggems::test::DescribeOpenCLDevice(compiler_device.inventory));

    ++tested_device_count;

    auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();

    std::filesystem::path kernel_root{GGEMS_TEST_KERNEL_ROOT};
    std::filesystem::path kernel_test_root = kernel_root / "tests";

    std::string build_options =
        std::format("-cl-std=CL2.0 -I{}", kernel_root.generic_string());

    auto const &program = opencl.GetOrCreateProgram(
        context, kernel_test_root, "random_jkiss_uniform", build_options);

    cl::Kernel raw_kernel = program.CreateKernel("random_jkiss_uniform");

    ggems::ocl::GGEMSOpenCLKernel kernel{context, std::move(raw_kernel),
                                         "random_jkiss_uniform"};

    std::size_t state_bytes = k_particle_count * sizeof(JKissState);
    std::size_t value_bytes = k_value_count * sizeof(float);

    auto states_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{state_bytes});
    auto values_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{value_bytes});

    auto *states = static_cast<JKissState *>(states_buffer.GetData());
    auto *values = static_cast<float *>(values_buffer.GetData());

    std::vector<JKissState> initial_states(k_particle_count);

    states_buffer.Map(CL_MAP_WRITE);
    values_buffer.Map(CL_MAP_WRITE);

    for (std::size_t i = 0U; i < k_particle_count; ++i) {
      JKissState state = MakeJKissState(k_seed, static_cast<std::uint32_t>(i));

      states[i] = state;
      initial_states[i] = state;
    }

    std::fill(values, values + k_value_count, -1.0F);

    values_buffer.Unmap();
    states_buffer.Unmap();

    kernel.SetArgSVMPointer(0U, states);
    kernel.SetArgSVMPointer(1U, values);
    kernel.SetArg(2U, static_cast<cl_uint>(k_particle_count));
    kernel.SetArg(3U, static_cast<cl_uint>(k_samples_per_particle));

    kernel.Run({k_global_work_size}, {k_local_size});

    states_buffer.Map(CL_MAP_READ);
    values_buffer.Map(CL_MAP_READ);

    std::size_t changed_state_count{0U};

    for (std::size_t i = 0U; i < k_particle_count; ++i) {
      if (!AreStatesEqual(states[i], initial_states[i])) {
        ++changed_state_count;
      }

      EXPECT_NE(states[i].x, initial_states[i].x);
    }

    for (std::size_t i = 0U; i < k_value_count; ++i) {
      EXPECT_GE(values[i], 0.0F);
      EXPECT_LT(values[i], 1.0F);
    }

    values_buffer.Unmap();
    states_buffer.Unmap();

    EXPECT_EQ(changed_state_count, k_particle_count);
  }

  if (tested_device_count == 0U) {
    GTEST_SKIP()
        << "No compiler-capable GGEMS OpenCL device supports SVM.";
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSJKissKernelTest, GenericRandomUniformUsesSelectedJKissEngine) {
  auto const &compiler_devices =
      ggems::test::GetOpenCLCompilerDeviceInventory();

  std::size_t tested_device_count{0U};

  for (auto const &compiler_device : compiler_devices) {
    auto &context = *compiler_device.context;

    if (!context.GetSVMSupport().HasAny()) {
      continue;
    }

    SCOPED_TRACE(
        ggems::test::DescribeOpenCLDevice(compiler_device.inventory));

    ++tested_device_count;

    auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();

    std::filesystem::path kernel_root{GGEMS_TEST_KERNEL_ROOT};
    std::filesystem::path kernel_test_root = kernel_root / "tests";

    std::string build_options =
        std::format("-cl-std=CL2.0 -I{} -DGGEMS_RANDOM_ENGINE=1",
                    kernel_root.generic_string());

    auto const &program = opencl.GetOrCreateProgram(
        context, kernel_test_root, "random_generic_uniform", build_options);

    cl::Kernel raw_kernel = program.CreateKernel("random_generic_uniform");

    ggems::ocl::GGEMSOpenCLKernel kernel{context, std::move(raw_kernel),
                                         "random_generic_uniform"};

    std::size_t state_bytes =
        k_particle_count * sizeof(ggems::core::random::GGEMSJKissState);

    std::size_t value_count = k_particle_count * k_samples_per_particle;

    std::size_t value_bytes = value_count * sizeof(float);

    auto states_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{state_bytes});
    auto values_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{value_bytes});

    auto *states = static_cast<JKissState *>(states_buffer.GetData());

    auto *values = static_cast<float *>(values_buffer.GetData());

    std::vector<JKissState> initial_states(k_particle_count);

    states_buffer.Map(CL_MAP_WRITE);
    values_buffer.Map(CL_MAP_WRITE);

    for (std::size_t i = 0U; i < k_particle_count; ++i) {
      auto state = MakeJKissState(k_seed, static_cast<std::uint32_t>(i));

      states[i] = state;
      initial_states[i] = state;
    }

    std::fill(values, values + value_count, -1.0F);

    values_buffer.Unmap();
    states_buffer.Unmap();

    kernel.SetArgSVMPointer(0U, states);
    kernel.SetArgSVMPointer(1U, values);
    kernel.SetArg(2U, static_cast<cl_uint>(k_particle_count));
    kernel.SetArg(3U, static_cast<cl_uint>(k_samples_per_particle));

    kernel.Run({k_global_work_size}, {k_local_size});

    states_buffer.Map(CL_MAP_READ);
    values_buffer.Map(CL_MAP_READ);

    for (std::size_t i = 0U; i < value_count; ++i) {
      EXPECT_GE(values[i], 0.0F);
      EXPECT_LT(values[i], 1.0F);
    }

    for (std::size_t i = 0U; i < k_particle_count; ++i) {
      EXPECT_NE(states[i].x, initial_states[i].x);
    }

    values_buffer.Unmap();
    states_buffer.Unmap();
  }

  if (tested_device_count == 0U) {
    GTEST_SKIP()
        << "No compiler-capable GGEMS OpenCL device supports SVM.";
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSJKissKernelTest, GenericRandomUniform4UsesSelectedJKissEngine) {
  auto const &compiler_devices =
      ggems::test::GetOpenCLCompilerDeviceInventory();

  std::size_t tested_device_count{0U};

  for (auto const &compiler_device : compiler_devices) {
    auto &context = *compiler_device.context;

    if (!context.GetSVMSupport().HasAny()) {
      continue;
    }

    SCOPED_TRACE(
        ggems::test::DescribeOpenCLDevice(compiler_device.inventory));

    ++tested_device_count;

    auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();

    std::filesystem::path kernel_root{GGEMS_TEST_KERNEL_ROOT};
    std::filesystem::path kernel_test_root = kernel_root / "tests";

    std::string build_options =
        std::format("-cl-std=CL2.0 -I{} -DGGEMS_RANDOM_ENGINE=1",
                    kernel_root.generic_string());

    auto const &program = opencl.GetOrCreateProgram(
        context, kernel_test_root, "random_generic_uniform4", build_options);

    cl::Kernel raw_kernel = program.CreateKernel("random_generic_uniform4");

    ggems::ocl::GGEMSOpenCLKernel kernel{context, std::move(raw_kernel),
                                         "random_generic_uniform4"};

    std::size_t state_bytes =
        k_particle_count * sizeof(ggems::core::random::GGEMSJKissState);

    std::size_t value_bytes = k_uniform4_value_count * sizeof(float);

    auto states_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{state_bytes});
    auto values_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{value_bytes});

    auto *states = static_cast<JKissState *>(states_buffer.GetData());
    auto *values = static_cast<float *>(values_buffer.GetData());

    std::vector<JKissState> initial_states(k_particle_count);

    states_buffer.Map(CL_MAP_WRITE);
    values_buffer.Map(CL_MAP_WRITE);

    for (std::size_t i = 0U; i < k_particle_count; ++i) {
      auto state = MakeJKissState(k_seed, static_cast<std::uint32_t>(i));

      states[i] = state;
      initial_states[i] = state;
    }

    std::fill(values, values + k_uniform4_value_count, -1.0F);

    values_buffer.Unmap();
    states_buffer.Unmap();

    kernel.SetArgSVMPointer(0U, states);
    kernel.SetArgSVMPointer(1U, values);
    kernel.SetArg(2U, static_cast<cl_uint>(k_particle_count));
    kernel.SetArg(3U, static_cast<cl_uint>(k_uniform4_blocks_per_particle));

    kernel.Run({k_global_work_size}, {k_local_size});

    states_buffer.Map(CL_MAP_READ);
    values_buffer.Map(CL_MAP_READ);

    for (std::size_t i = 0U; i < k_uniform4_value_count; ++i) {
      EXPECT_GE(values[i], 0.0F);
      EXPECT_LT(values[i], 1.0F);
    }

    for (std::size_t i = 0U; i < k_particle_count; ++i) {
      EXPECT_NE(states[i].x, initial_states[i].x);
    }

    values_buffer.Unmap();
    states_buffer.Unmap();
  }

  if (tested_device_count == 0U) {
    GTEST_SKIP()
        << "No compiler-capable GGEMS OpenCL device supports SVM.";
  }
}
/// \endcond
