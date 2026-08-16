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
 * \brief OpenCL validation tests for the GGEMS Philox engine.
 *
 * Validates scalar and vector Philox output, deterministic counter progression, stream independence, and host/kernel state agreement.
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

using PhiloxState = ggems::core::random::GGEMSPhiloxState;

// =============================================================================
// =============================================================================

constexpr std::uint64_t k_seed{77777ULL};
constexpr std::uint64_t k_alternative_seed{77778ULL};

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

auto SplitMix64(std::uint64_t value) noexcept -> std::uint64_t {
  value += 0x9E3779B97F4A7C15ULL;

  value = (value ^ (value >> 30U)) * 0xBF58476D1CE4E5B9ULL;
  value = (value ^ (value >> 27U)) * 0x94D049BB133111EBULL;

  return value ^ (value >> 31U);
}

// =============================================================================
// =============================================================================

auto MakePhiloxState(std::uint64_t seed, std::uint64_t index) noexcept
    -> PhiloxState {
  std::uint64_t key = SplitMix64(seed);

  return PhiloxState{.counter_0 = 0U,
                     .counter_1 = 0U,
                     .counter_2 = static_cast<std::uint32_t>(index),
                     .counter_3 = static_cast<std::uint32_t>(index >> 32U),
                     .key_0 = static_cast<std::uint32_t>(key),
                     .key_1 = static_cast<std::uint32_t>(key >> 32U)};
}

// =============================================================================
// =============================================================================

auto AreStatesEqual(PhiloxState const &lhs, PhiloxState const &rhs) noexcept
    -> bool {
  return lhs.counter_0 == rhs.counter_0 && lhs.counter_1 == rhs.counter_1 &&
         lhs.counter_2 == rhs.counter_2 && lhs.counter_3 == rhs.counter_3 &&
         lhs.key_0 == rhs.key_0 && lhs.key_1 == rhs.key_1;
}

// =============================================================================
// =============================================================================

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSPhiloxKernelTest, UniformValuesAreInsideUnitInterval) {
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
        context, kernel_test_root, "random_philox_uniform", build_options);

    cl::Kernel raw_kernel = program.CreateKernel("random_philox_uniform");

    ggems::ocl::GGEMSOpenCLKernel kernel{context, std::move(raw_kernel),
                                         "random_philox_uniform"};

    std::size_t state_bytes = k_particle_count * sizeof(PhiloxState);
    std::size_t value_bytes = k_value_count * sizeof(float);

    auto states_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{state_bytes});
    auto values_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{value_bytes});

    auto *states = static_cast<PhiloxState *>(states_buffer.GetData());
    auto *values = static_cast<float *>(values_buffer.GetData());

    states_buffer.Map(CL_MAP_WRITE);
    values_buffer.Map(CL_MAP_WRITE);

    for (std::size_t i = 0U; i < k_particle_count; ++i) {
      states[i] = MakePhiloxState(k_seed, static_cast<std::uint64_t>(i));
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

TEST(GGEMSPhiloxKernelTest, SequenceContinuesBetweenKernelCalls) {
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
        context, kernel_test_root, "random_philox_uniform", build_options);

    cl::Kernel raw_kernel = program.CreateKernel("random_philox_uniform");

    ggems::ocl::GGEMSOpenCLKernel kernel{context, std::move(raw_kernel),
                                         "random_philox_uniform"};

    std::size_t state_bytes = k_particle_count * sizeof(PhiloxState);
    std::size_t value_bytes = k_value_count * sizeof(float);

    auto states_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{state_bytes});
    auto values_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{value_bytes});

    auto *states = static_cast<PhiloxState *>(states_buffer.GetData());
    auto *values = static_cast<float *>(values_buffer.GetData());

    states_buffer.Map(CL_MAP_WRITE);
    values_buffer.Map(CL_MAP_WRITE);

    for (std::size_t i = 0U; i < k_particle_count; ++i) {
      states[i] = MakePhiloxState(k_seed, static_cast<std::uint64_t>(i));
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

TEST(GGEMSPhiloxKernelTest, SameSeedProducesSameFirstSequence) {
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
        context, kernel_test_root, "random_philox_uniform", build_options);

    cl::Kernel raw_kernel = program.CreateKernel("random_philox_uniform");

    ggems::ocl::GGEMSOpenCLKernel kernel{context, std::move(raw_kernel),
                                         "random_philox_uniform"};

    std::size_t state_bytes = k_particle_count * sizeof(PhiloxState);
    std::size_t value_bytes = k_value_count * sizeof(float);

    auto states_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{state_bytes});
    auto values_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{value_bytes});

    auto *states = static_cast<PhiloxState *>(states_buffer.GetData());
    auto *values = static_cast<float *>(values_buffer.GetData());

    auto initialize_states = [&]() -> void {
      states_buffer.Map(CL_MAP_WRITE);
      values_buffer.Map(CL_MAP_WRITE);

      for (std::size_t i = 0U; i < k_particle_count; ++i) {
        states[i] = MakePhiloxState(k_seed, static_cast<std::uint64_t>(i));
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

TEST(GGEMSPhiloxKernelTest, DifferentSeedsProduceDifferentFirstSequence) {
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
        context, kernel_test_root, "random_philox_uniform", build_options);

    cl::Kernel raw_kernel = program.CreateKernel("random_philox_uniform");

    ggems::ocl::GGEMSOpenCLKernel kernel{context, std::move(raw_kernel),
                                         "random_philox_uniform"};

    std::size_t state_bytes = k_particle_count * sizeof(PhiloxState);
    std::size_t value_bytes = k_value_count * sizeof(float);

    auto states_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{state_bytes});
    auto values_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{value_bytes});

    auto *states = static_cast<PhiloxState *>(states_buffer.GetData());
    auto *values = static_cast<float *>(values_buffer.GetData());

    auto initialize_states = [&](std::uint64_t seed) -> void {
      states_buffer.Map(CL_MAP_WRITE);
      values_buffer.Map(CL_MAP_WRITE);

      for (std::size_t i = 0U; i < k_particle_count; ++i) {
        states[i] = MakePhiloxState(seed, static_cast<std::uint64_t>(i));
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

TEST(GGEMSPhiloxKernelTest, RandomStatesAreAdvancedByKernelExecution) {
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
        context, kernel_test_root, "random_philox_uniform", build_options);

    cl::Kernel raw_kernel = program.CreateKernel("random_philox_uniform");

    ggems::ocl::GGEMSOpenCLKernel kernel{context, std::move(raw_kernel),
                                         "random_philox_uniform"};

    std::size_t state_bytes = k_particle_count * sizeof(PhiloxState);
    std::size_t value_bytes = k_value_count * sizeof(float);

    auto states_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{state_bytes});
    auto values_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{value_bytes});

    auto *states = static_cast<PhiloxState *>(states_buffer.GetData());
    auto *values = static_cast<float *>(values_buffer.GetData());

    std::vector<PhiloxState> initial_states(k_particle_count);

    states_buffer.Map(CL_MAP_WRITE);
    values_buffer.Map(CL_MAP_WRITE);

    for (std::size_t i = 0U; i < k_particle_count; ++i) {
      PhiloxState state = MakePhiloxState(k_seed, static_cast<std::uint64_t>(i));

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

      EXPECT_EQ(states[i].counter_0,
                initial_states[i].counter_0 +
                    static_cast<std::uint32_t>(k_samples_per_particle));

      EXPECT_EQ(states[i].counter_1, initial_states[i].counter_1);
      EXPECT_EQ(states[i].counter_2, initial_states[i].counter_2);
      EXPECT_EQ(states[i].counter_3, initial_states[i].counter_3);
      EXPECT_EQ(states[i].key_0, initial_states[i].key_0);
      EXPECT_EQ(states[i].key_1, initial_states[i].key_1);
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

TEST(GGEMSPhiloxKernelTest,
       Uniform4ValuesAreInsideUnitIntervalAndAdvanceByBlock) {
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
        context, kernel_test_root, "random_philox_uniform4", build_options);

    cl::Kernel raw_kernel = program.CreateKernel("random_philox_uniform4");

    ggems::ocl::GGEMSOpenCLKernel kernel{context, std::move(raw_kernel),
                                         "random_philox_uniform4"};

    std::size_t state_bytes = k_particle_count * sizeof(PhiloxState);
    std::size_t value_bytes = k_uniform4_value_count * sizeof(float);

    auto states_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{state_bytes});
    auto values_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{value_bytes});

    auto *states = static_cast<PhiloxState *>(states_buffer.GetData());
    auto *values = static_cast<float *>(values_buffer.GetData());

    std::vector<PhiloxState> initial_states(k_particle_count);

    states_buffer.Map(CL_MAP_WRITE);
    values_buffer.Map(CL_MAP_WRITE);

    for (std::size_t i = 0U; i < k_particle_count; ++i) {
      PhiloxState state = MakePhiloxState(k_seed, static_cast<std::uint64_t>(i));

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
      EXPECT_EQ(states[i].counter_0,
                initial_states[i].counter_0 +
                    static_cast<std::uint32_t>(k_uniform4_blocks_per_particle));

      EXPECT_EQ(states[i].counter_1, initial_states[i].counter_1);
      EXPECT_EQ(states[i].counter_2, initial_states[i].counter_2);
      EXPECT_EQ(states[i].counter_3, initial_states[i].counter_3);
      EXPECT_EQ(states[i].key_0, initial_states[i].key_0);
      EXPECT_EQ(states[i].key_1, initial_states[i].key_1);
    }

    values_buffer.Unmap();
    states_buffer.Unmap();
  }

  if (tested_device_count == 0U) {
    GTEST_SKIP()
        << "No compiler-capable GGEMS OpenCL device supports SVM.";
  }
}

TEST(GGEMSPhiloxKernelTest, GenericRandomUniformUsesSelectedPhiloxEngine) {
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
        std::format("-cl-std=CL2.0 -I{} -DGGEMS_RANDOM_ENGINE=3",
                    kernel_root.generic_string());

    auto const &program = opencl.GetOrCreateProgram(
        context, kernel_test_root, "random_generic_uniform", build_options);

    cl::Kernel raw_kernel = program.CreateKernel("random_generic_uniform");

    ggems::ocl::GGEMSOpenCLKernel kernel{context, std::move(raw_kernel),
                                         "random_generic_uniform"};

    std::size_t state_bytes = k_particle_count * sizeof(PhiloxState);
    std::size_t value_bytes = k_value_count * sizeof(float);

    auto states_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{state_bytes});
    auto values_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{value_bytes});

    auto *states = static_cast<PhiloxState *>(states_buffer.GetData());
    auto *values = static_cast<float *>(values_buffer.GetData());

    states_buffer.Map(CL_MAP_WRITE);
    values_buffer.Map(CL_MAP_WRITE);

    for (std::size_t i = 0U; i < k_particle_count; ++i) {
      states[i] = MakePhiloxState(k_seed, static_cast<std::uint64_t>(i));
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

    for (std::size_t i = 0U; i < k_value_count; ++i) {
      EXPECT_GE(values[i], 0.0F);
      EXPECT_LT(values[i], 1.0F);
    }

    for (std::size_t i = 0U; i < k_particle_count; ++i) {
      EXPECT_EQ(states[i].counter_0,
                static_cast<std::uint32_t>(k_samples_per_particle));
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

TEST(GGEMSPhiloxKernelTest, GenericRandomUniform4UsesSelectedPhiloxEngine) {
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
        std::format("-cl-std=CL2.0 -I{} -DGGEMS_RANDOM_ENGINE=3",
                    kernel_root.generic_string());

    auto const &program = opencl.GetOrCreateProgram(
        context, kernel_test_root, "random_generic_uniform4", build_options);

    cl::Kernel raw_kernel = program.CreateKernel("random_generic_uniform4");

    ggems::ocl::GGEMSOpenCLKernel kernel{context, std::move(raw_kernel),
                                         "random_generic_uniform4"};

    std::size_t state_bytes = k_particle_count * sizeof(PhiloxState);
    std::size_t value_bytes = k_uniform4_value_count * sizeof(float);

    auto states_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{state_bytes});

    auto values_buffer =
        context.CreateSVMBuffer(ggems::units::Bytes{value_bytes});

    auto *states = static_cast<PhiloxState *>(states_buffer.GetData());
    auto *values = static_cast<float *>(values_buffer.GetData());

    std::vector<PhiloxState> initial_states(k_particle_count);

    states_buffer.Map(CL_MAP_WRITE);
    values_buffer.Map(CL_MAP_WRITE);

    for (std::size_t i = 0U; i < k_particle_count; ++i) {
      PhiloxState state = MakePhiloxState(k_seed, static_cast<std::uint64_t>(i));

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
      EXPECT_EQ(states[i].counter_0,
                initial_states[i].counter_0 +
                    static_cast<std::uint32_t>(k_uniform4_blocks_per_particle));

      EXPECT_EQ(states[i].counter_1, initial_states[i].counter_1);
      EXPECT_EQ(states[i].counter_2, initial_states[i].counter_2);
      EXPECT_EQ(states[i].counter_3, initial_states[i].counter_3);
      EXPECT_EQ(states[i].key_0, initial_states[i].key_0);
      EXPECT_EQ(states[i].key_1, initial_states[i].key_1);
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
