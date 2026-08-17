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
 * \brief Unit tests for GGEMS host random streams.
 *
 * Validates deterministic continuation, stream separation, raw output, and scalar uniform contracts across all supported engines.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <limits>
#include <span>
#include <string>
#include <utility>

#include <gtest/gtest.h>

/// \endcond
#include "GGEMS/GGEMSException.hh"
#include "GGEMS/random/GGEMSHostRandomStream.hh"
#include "GGEMS/random/GGEMSRandom.hh"
#include "GGEMS/random/GGEMSRandomEngine.hh"
#include "GGEMS/units/GGEMSBytesUnits.hh"
#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/opencl/GGEMSOpenCLExternal.hh"
#include "GGEMS/opencl/GGEMSOpenCLKernel.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMBuffer.hh"
#include "GGEMSOpenCLCompilerDeviceInventory.hh"
#include "GGEMSOpenCLDeviceInventory.hh"

/// \cond

namespace {

// =============================================================================
// =============================================================================

using ggems::core::random::GGEMSHostRandomStream;
using ggems::core::random::GGEMSRandom;
using ggems::core::random::GGEMSRandomEngine;

// =============================================================================
// =============================================================================

constexpr std::array<GGEMSRandomEngine, 3> k_engines{GGEMSRandomEngine::JKISS,
                                                     GGEMSRandomEngine::PCG32,
                                                     GGEMSRandomEngine::Philox};
constexpr std::size_t k_sequence_size{32U};
constexpr std::size_t k_probe_sample_count{16U};

// =============================================================================
// =============================================================================

auto SequenceDiffers(GGEMSHostRandomStream &lhs,
                     GGEMSHostRandomStream &rhs) noexcept -> bool {
  for (std::size_t index = 0U; index < k_sequence_size; ++index) {
    if (lhs.NextUInt32() != rhs.NextUInt32()) {
      return true;
    }
  }

  return false;
}

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSHostRandomStreamTest, ReportsCapturedEngineAndStreamId) {
  GGEMSRandom random{};
  random.SetEngine(GGEMSRandomEngine::PCG32).SetSeed(123ULL);

  GGEMSHostRandomStream stream{random, 456ULL};

  EXPECT_EQ(stream.GetEngine(), GGEMSRandomEngine::PCG32);
  EXPECT_EQ(stream.GetStreamId(), 456ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSHostRandomStreamTest, SameSeedAndStreamProduceSameSequence) {
  for (GGEMSRandomEngine engine : k_engines) {
    SCOPED_TRACE(static_cast<std::uint32_t>(engine));

    GGEMSRandom random{};
    random.SetEngine(engine).SetSeed(77'777ULL);
    GGEMSHostRandomStream first{random, 42ULL};
    GGEMSHostRandomStream second{random, 42ULL};

    for (std::size_t index = 0U; index < k_sequence_size; ++index) {
      EXPECT_EQ(first.NextUInt32(), second.NextUInt32());
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSHostRandomStreamTest, DifferentSeedChangesSequence) {
  for (GGEMSRandomEngine engine : k_engines) {
    SCOPED_TRACE(static_cast<std::uint32_t>(engine));

    GGEMSRandom first_random{};
    first_random.SetEngine(engine).SetSeed(77'777ULL);
    GGEMSRandom second_random{};
    second_random.SetEngine(engine).SetSeed(77'778ULL);
    GGEMSHostRandomStream first{first_random, 42ULL};
    GGEMSHostRandomStream second{second_random, 42ULL};

    EXPECT_TRUE(SequenceDiffers(first, second));
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSHostRandomStreamTest, DifferentStreamChangesSequence) {
  for (GGEMSRandomEngine engine : k_engines) {
    SCOPED_TRACE(static_cast<std::uint32_t>(engine));

    GGEMSRandom random{};
    random.SetEngine(engine).SetSeed(77'777ULL);
    GGEMSHostRandomStream first{random, 42ULL};
    GGEMSHostRandomStream second{random, 43ULL};

    EXPECT_TRUE(SequenceDiffers(first, second));
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSHostRandomStreamTest, SequenceContinuesAcrossRepeatedCalls) {
  for (GGEMSRandomEngine engine : k_engines) {
    SCOPED_TRACE(static_cast<std::uint32_t>(engine));

    GGEMSRandom random{};
    random.SetEngine(engine).SetSeed(77'777ULL);
    GGEMSHostRandomStream stream{random, 42ULL};
    GGEMSHostRandomStream reference{random, 42ULL};

    for (std::size_t index = 0U; index < 17U; ++index) {
      (void)stream.NextUInt32();
      (void)reference.NextUInt32();
    }

    for (std::size_t index = 0U; index < k_sequence_size; ++index) {
      EXPECT_EQ(stream.NextUInt32(), reference.NextUInt32());
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSHostRandomStreamTest,
     LaterRandomConfigurationChangesDoNotAffectExistingStream) {
  GGEMSRandom random{};
  random.SetEngine(GGEMSRandomEngine::JKISS).SetSeed(77'777ULL);
  GGEMSHostRandomStream stream{random, 42ULL};

  random.SetEngine(GGEMSRandomEngine::Philox).SetSeed(1ULL);

  GGEMSRandom reference_random{};
  reference_random.SetEngine(GGEMSRandomEngine::JKISS).SetSeed(77'777ULL);
  GGEMSHostRandomStream reference{reference_random, 42ULL};

  for (std::size_t index = 0U; index < k_sequence_size; ++index) {
    EXPECT_EQ(stream.NextUInt32(), reference.NextUInt32());
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSHostRandomStreamTest, JKISSRejectsOutOfRangeStreamId) {
  GGEMSRandom random{};
  random.SetEngine(GGEMSRandomEngine::JKISS);

  EXPECT_THROW((GGEMSHostRandomStream{
                   random, static_cast<std::uint64_t>(
                               std::numeric_limits<std::uint32_t>::max()) +
                               1ULL}),
               ggems::core::GGEMSExceptionBase);
}

// =============================================================================
// =============================================================================

TEST(GGEMSHostRandomStreamTest, UniformFloatStaysInsideHalfOpenUnitInterval) {
  for (GGEMSRandomEngine engine : k_engines) {
    SCOPED_TRACE(static_cast<std::uint32_t>(engine));

    GGEMSRandom random{};
    random.SetEngine(engine).SetSeed(77'777ULL);
    GGEMSHostRandomStream stream{random, 42ULL};

    for (std::size_t index = 0U; index < 4096U; ++index) {
      float const value = stream.UniformFloat01();
      EXPECT_GE(value, 0.0F);
      EXPECT_LT(value, 1.0F);
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSHostRandomStreamTest, UniformDoubleStaysInsideOpenUnitInterval) {
  for (GGEMSRandomEngine engine : k_engines) {
    SCOPED_TRACE(static_cast<std::uint32_t>(engine));

    GGEMSRandom random{};
    random.SetEngine(engine).SetSeed(77'777ULL);
    GGEMSHostRandomStream stream{random, 42ULL};

    for (std::size_t index = 0U; index < 4096U; ++index) {
      double const value = stream.UniformDoubleOpen01();
      EXPECT_GT(value, 0.0);
      EXPECT_LT(value, 1.0);
    }
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSHostRandomStreamTest,
     UniformDoubleUsesSpecifiedWordsAndConsumesExactlyTwo) {
  for (GGEMSRandomEngine engine : k_engines) {
    SCOPED_TRACE(static_cast<std::uint32_t>(engine));

    GGEMSRandom random{};
    random.SetEngine(engine).SetSeed(77'777ULL);
    GGEMSHostRandomStream stream{random, 42ULL};
    GGEMSHostRandomStream reference{random, 42ULL};

    auto const high_bits =
        static_cast<std::uint64_t>(reference.NextUInt32() >> 5U);
    auto const low_bits =
        static_cast<std::uint64_t>(reference.NextUInt32() >> 6U);
    std::uint64_t const bits = (high_bits << 26U) + low_bits;
    double expected = (static_cast<double>(bits) + 0.5) * 0x1.0p-53;
    if (expected >= 1.0) {
      expected = 0x1.fffffffffffffp-1;
    }

    EXPECT_EQ(stream.UniformDoubleOpen01(), expected);
    EXPECT_EQ(stream.NextUInt32(), reference.NextUInt32());
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSHostRandomStreamTest,
     ScalarHostProgressionMatchesScalarOpenCLProgression) {
  auto const &compiler_devices =
      ggems::test::GetOpenCLCompilerDeviceInventory();

  if (compiler_devices.empty()) {
    GTEST_SKIP() << "No available GGEMS-discovered device has a compiler.";
  }

  auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();
  std::filesystem::path const kernel_root{GGEMS_TEST_KERNEL_ROOT};
  std::filesystem::path const kernel_test_root = kernel_root / "tests";

  std::size_t tested_device_count{0U};

  for (auto const &compiler_device : compiler_devices) {
    auto &context = *compiler_device.context;

    if (!context.GetSVMSupport().HasAny()) {
      continue;
    }

    SCOPED_TRACE(
        ggems::test::DescribeOpenCLDevice(compiler_device.inventory));

    ++tested_device_count;

    for (GGEMSRandomEngine engine : k_engines) {
      GGEMSRandom random{};
      random.SetEngine(engine);

      std::string const build_options =
          std::format("-cl-std=CL2.0 -I{} {}", kernel_root.generic_string(),
                      random.GetKernelBuildDefinition());
      auto const &program = opencl.GetOrCreateProgram(
          context, kernel_test_root, "random_host_stream_probe", build_options);
      cl::Kernel raw_kernel = program.CreateKernel("random_host_stream_probe");
      ggems::ocl::GGEMSOpenCLKernel kernel{context, std::move(raw_kernel),
                                           "random_host_stream_probe"};

      std::array<std::uint64_t, 2> const seeds{0ULL, 77'777ULL};
      std::array<std::uint64_t, 2> const stream_ids{
          0ULL,
          engine == GGEMSRandomEngine::JKISS ? 42ULL : (1ULL << 40U) + 42ULL};

      for (std::uint64_t seed : seeds) {
        for (std::uint64_t stream_id : stream_ids) {
          SCOPED_TRACE(std::format("engine={} seed={} stream={}",
                                   static_cast<std::uint32_t>(engine), seed,
                                   stream_id));

          random.SetSeed(seed);
          auto raw_state_buffer =
              context.CreateSVMBuffer(ggems::units::Bytes{random.GetStateSize()});
          auto uniform_state_buffer =
              context.CreateSVMBuffer(ggems::units::Bytes{random.GetStateSize()});
          auto raw_values_buffer = context.CreateSVMBuffer(
              ggems::units::Bytes{k_probe_sample_count *
                                  sizeof(std::uint32_t)});
          auto uniform_values_buffer = context.CreateSVMBuffer(
              ggems::units::Bytes{k_probe_sample_count * sizeof(float)});

          raw_state_buffer.Map(CL_MAP_WRITE);
          uniform_state_buffer.Map(CL_MAP_WRITE);
          random.InitializeStates(
              stream_id,
              std::span<std::byte>{
                  static_cast<std::byte *>(raw_state_buffer.GetData()),
                  random.GetStateSize()});
          random.InitializeStates(
              stream_id,
              std::span<std::byte>{
                  static_cast<std::byte *>(uniform_state_buffer.GetData()),
                  random.GetStateSize()});
          uniform_state_buffer.Unmap();
          raw_state_buffer.Unmap();

          kernel.SetArgSVMPointer(0U, raw_state_buffer.GetData());
          kernel.SetArgSVMPointer(1U, uniform_state_buffer.GetData());
          kernel.SetArgSVMPointer(2U, raw_values_buffer.GetData());
          kernel.SetArgSVMPointer(3U, uniform_values_buffer.GetData());
          kernel.SetArg(4U, static_cast<cl_uint>(k_probe_sample_count));
          kernel.Run({1U}, {1U});

          GGEMSHostRandomStream host_raw{random, stream_id};
          GGEMSHostRandomStream host_uniform{random, stream_id};
          auto const *raw_values =
              static_cast<std::uint32_t const *>(raw_values_buffer.GetData());
          auto const *uniform_values =
              static_cast<float const *>(uniform_values_buffer.GetData());

          raw_values_buffer.Map(CL_MAP_READ);
          uniform_values_buffer.Map(CL_MAP_READ);
          for (std::size_t index = 0U; index < k_probe_sample_count; ++index) {
            EXPECT_EQ(host_raw.NextUInt32(), raw_values[index]);
            EXPECT_EQ(
                std::bit_cast<std::uint32_t>(host_uniform.UniformFloat01()),
                std::bit_cast<std::uint32_t>(uniform_values[index]));
          }
          uniform_values_buffer.Unmap();
          raw_values_buffer.Unmap();
        }
      }
    }
  }

  if (tested_device_count == 0U) {
    GTEST_SKIP()
        << "No compiler-capable GGEMS OpenCL device supports SVM.";
  }
}
/// \endcond
