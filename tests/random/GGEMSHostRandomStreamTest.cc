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
 * \brief Checks host random stream behavior and observable contracts.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

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

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/random/GGEMSHostRandomStream.hh"
#include "GGEMS/random/GGEMSRandom.hh"
#include "GGEMS/random/GGEMSRandomEngine.hh"
#include "GGEMS/units/GGEMSBytesUnits.hh"
#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/opencl/GGEMSOpenCLExternal.hh"
#include "GGEMS/opencl/GGEMSOpenCLKernel.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMBuffer.hh"
#include "GGEMS/opencl/GGEMSOpenCLContext.hh"
#include "GGEMSOpenCLDeviceInventory.hh"

namespace {

// =============================================================================
// =============================================================================

using ggems::core::random::GGEMSHostRandomStream;
using ggems::core::random::GGEMSRandom;
using ggems::core::random::GGEMSRandomEngine;

// =============================================================================
// =============================================================================

constexpr std::array<GGEMSRandomEngine, 3> k_engines{
  GGEMSRandomEngine::JKISS,
  GGEMSRandomEngine::PCG32,
  GGEMSRandomEngine::Philox,
};
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

// Stream diagnostics must report the identity captured when that stream was
// created.
TEST(GGEMSHostRandomStreamTest, ReportsCapturedEngineAndStreamId) {
  GGEMSRandom random{};
  random.SetEngine(GGEMSRandomEngine::PCG32).SetSeed(123ULL);

  GGEMSHostRandomStream stream{random, 456ULL};

  EXPECT_EQ(stream.GetEngine(), GGEMSRandomEngine::PCG32);
  EXPECT_EQ(stream.GetStreamId(), 456ULL);
}

// =============================================================================
// =============================================================================

// A fixed stream must replay within its contract and continue without
// unintended reset.
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

// Seed and stream identity must control the expected random sequence.
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

// Seed and stream identity must control the expected random sequence.
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

// A fixed stream must replay within its contract and continue without
// unintended reset.
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

// An existing stream must retain its captured engine and seed after
// configuration changes.
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

// The limited stream-ID domain must fail clearly instead of aliasing another
// random stream.
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

// Sampling must respect its interval endpoints and advance the persistent
// random state.
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

// Sampling must respect its interval endpoints and advance the persistent
// random state.
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

// Sampling must respect its interval endpoints and advance the persistent
// random state.
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

// Device sampling must consume the specified random words and preserve the
// observable stream progression.
TEST(GGEMSHostRandomStreamTest,
     ScalarHostProgressionMatchesScalarOpenCLProgression) {
  auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();
  std::filesystem::path const kernel_root{GGEMS_TEST_KERNEL_ROOT};
  std::filesystem::path const kernel_test_root =
    std::filesystem::path{GGEMS_TEST_PROBE_ROOT};

  auto &context = ggems::test::GetSelectedOpenCLContext();

  for (GGEMSRandomEngine engine : k_engines) {
    GGEMSRandom random{};
    random.SetEngine(engine);

    std::string const build_options =
      std::format("-I{} {}", kernel_root.generic_string(),
                  random.GetKernelBuildDefinition());
    auto const &program = opencl.GetOrCreateProgram(
      context, kernel_test_root, "random_host_stream_probe", build_options);
    cl::Kernel raw_kernel = program.CreateKernel("random_host_stream_probe");
    ggems::ocl::GGEMSOpenCLKernel kernel{context, std::move(raw_kernel),
                                         "random_host_stream_probe"};

    std::array<std::uint64_t, 2> const seeds{0ULL, 77'777ULL};
    std::array<std::uint64_t, 2> const stream_ids{
      0ULL,
      engine == GGEMSRandomEngine::JKISS ? 42ULL : (1ULL << 40U) + 42ULL,
    };

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
          ggems::units::Bytes{k_probe_sample_count * sizeof(std::uint32_t)});
        auto uniform_values_buffer = context.CreateSVMBuffer(
          ggems::units::Bytes{k_probe_sample_count * sizeof(float)});

        raw_state_buffer.Map(CL_MAP_WRITE);
        uniform_state_buffer.Map(CL_MAP_WRITE);
        random.InitializeStates(
          stream_id, std::span<std::byte>{
                       static_cast<std::byte *>(raw_state_buffer.GetData()),
                       random.GetStateSize()});
        random.InitializeStates(
          stream_id, std::span<std::byte>{
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
          EXPECT_EQ(std::bit_cast<std::uint32_t>(host_uniform.UniformFloat01()),
                    std::bit_cast<std::uint32_t>(uniform_values[index]));
        }
        uniform_values_buffer.Unmap();
        raw_values_buffer.Unmap();
      }
    }
  }
}
