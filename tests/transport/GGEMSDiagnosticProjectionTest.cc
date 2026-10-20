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
 * \brief Checks FP32 movement and atomic rejection at the shared device
 * boundary.
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
#include <utility>

#include <gtest/gtest.h>

#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/opencl/GGEMSOpenCLContext.hh"
#include "GGEMS/opencl/GGEMSOpenCLKernel.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMHostAccess.hh"
#include "GGEMS/particles/GGEMSParticleState.hh"
#include "GGEMS/sources/GGEMSSourceRecord.hh"
#include "GGEMS/sources/GGEMSSourceRunRange.hh"
#include "GGEMS/transport/GGEMSDiagnosticProjection.hh"
#include "GGEMS/units/GGEMSBytesUnits.hh"

namespace {

using ParticleState = ggems::core::particles::GGEMSParticleState;

class GGEMSDiagnosticProjectionKernelTest : public ::testing::Test {
protected:
  static auto SetUpTestSuite() -> void {
    auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();
    if (opencl.GetContext().empty()) {
      opencl.SelectDevices({"gpu"});
      opencl.Initialize();
    }
    ASSERT_FALSE(opencl.GetContext().empty());
  }
};

} // namespace

TEST(GGEMSDiagnosticProjection, DoesNotImposeIntegerEndpointRangeOnFP32Flight) {
  std::array<ggems::core::sources::GGEMSSourceRecord, 1U> sources{};
  sources[0].position_z_pm = std::numeric_limits<std::int64_t>::max();
  std::array<ggems::core::sources::GGEMSSourceRunRange, 1U> ranges{};
  ranges[0].primary_count = 1ULL;

  EXPECT_NO_THROW(ggems::core::transport::ValidateDiagnosticTransportSources(
    sources, ranges));
}

TEST_F(GGEMSDiagnosticProjectionKernelTest,
       MovesInFP32AndRejectsInvalidTransactionsWithoutPartialWrite) {
  constexpr auto distance = static_cast<float>(
    ggems::core::transport::k_diagnostic_projection_distance_pm);
  constexpr float nan = std::numeric_limits<float>::quiet_NaN();
  constexpr float infinity = std::numeric_limits<float>::infinity();
  constexpr float maximum = std::numeric_limits<float>::max();
  std::array<float, 15U> parameters{
    distance, 1.5F,    1.0F,      -0.5F, nan,  infinity, 1.0F, 1.0F,
    1.0F,     maximum, -infinity, 0.0F,  1.0F, 1.0F,     1.0F,
  };
  std::array<ParticleState, parameters.size()> states{};
  for (auto &state : states) {
    state.global_particle_id = (1ULL << 40U) + 7ULL;
    state.time_ps = (1ULL << 55U) + 9ULL;
    state.energy_micro_eV = 511'000'000'000ULL;
    state.track_id = 3U;
    state.parent_track_id = 1U;
    state.current_navigator_id = 11U;
    state.current_volume_id = 12U;
    state.material_id = 13U;
    state.region_id = 14U;
    state.position_x_pm = 10.0F;
    state.position_y_pm = -20.0F;
    state.position_z_pm = 30.0F;
    state.direction_x = 1.0F;
    state.direction_y = -1.0F;
    state.direction_z = 0.5F;
    state.particle_type = 1U;
    state.status = 1U;
    state.generation = 2U;
  }
  states[0].position_x_pm = 0.0F;
  states[0].position_y_pm = 0.0F;
  states[0].position_z_pm = 0.0F;
  states[2].position_x_pm = 0x1p60F;
  states[2].position_y_pm = -0x1p60F;
  states[2].position_z_pm = 0x1p60F;
  states[6].direction_z = nan;
  states[7].direction_y = infinity;
  states[8].position_z_pm = nan;
  states[9].direction_z = 2.0F;
  states[12].direction_x = 0.0F;
  states[12].direction_y = -0.0F;
  states[12].direction_z = 0.0F;
  states[13].position_x_pm = infinity;
  states[14].direction_x = -infinity;
  auto const initial = states;
  std::array<std::uint32_t, parameters.size() + 1U> success{};

  auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();
  auto &context = opencl.GetContext().front();
  auto state_buffer =
    context.CreateSVMBuffer(ggems::units::Bytes{sizeof(states)});
  auto parameter_buffer =
    context.CreateSVMBuffer(ggems::units::Bytes{sizeof(parameters)});
  auto success_buffer =
    context.CreateSVMBuffer(ggems::units::Bytes{sizeof(success)});
  ggems::ocl::WriteSVMFromHost(state_buffer, std::span{states});
  ggems::ocl::WriteSVMFromHost(parameter_buffer, std::span{parameters});
  ggems::ocl::WriteSVMFromHost(success_buffer, std::span{success});

  std::filesystem::path const kernel_root{GGEMS_TEST_KERNEL_ROOT};
  auto const &program = opencl.GetOrCreateProgram(
    context, kernel_root / "tests", "diagnostic_projection_probe",
    std::format("-I{}", kernel_root.generic_string()));
  cl::Kernel raw_kernel = program.CreateKernel("diagnostic_projection_probe");
  ggems::ocl::GGEMSOpenCLKernel kernel{context, std::move(raw_kernel),
                                       "diagnostic_projection_probe"};
  kernel.SetArgSVMPointer(0U, state_buffer.GetData());
  kernel.SetArgSVMPointer(1U, parameter_buffer.GetData());
  kernel.SetArgSVMPointer(2U, success_buffer.GetData());
  kernel.SetArg(3U, static_cast<cl_uint>(states.size()));
  kernel.Run({states.size()}, {1U});
  ggems::ocl::ReadSVMToHost(state_buffer, std::span{states});
  ggems::ocl::ReadSVMToHost(success_buffer, std::span{success});

  EXPECT_EQ(success.back(), std::bit_cast<std::uint32_t>(distance));
  for (std::size_t index = 0U; index < states.size(); ++index) {
    SCOPED_TRACE(index);
    bool const accepted = index < 3U || index == 11U;
    EXPECT_EQ(success[index], accepted ? 1U : 0U);
    auto const &state = states[index];
    auto const &before = initial[index];
    std::array<float, 3U> expected{before.position_x_pm, before.position_y_pm,
                                   before.position_z_pm};
    if (index == 0U) {
      expected = {distance, -distance, 0.5F * distance};
    } else if (index == 1U) {
      expected = {11.5F, -21.5F, 30.75F};
    }
    std::array<float, 3U> const actual{state.position_x_pm, state.position_y_pm,
                                       state.position_z_pm};
    for (std::size_t axis = 0U; axis < actual.size(); ++axis) {
      EXPECT_EQ(std::bit_cast<std::uint32_t>(actual[axis]),
                std::bit_cast<std::uint32_t>(expected[axis]));
    }
    EXPECT_EQ(state.global_particle_id, before.global_particle_id);
    EXPECT_EQ(state.time_ps, before.time_ps);
    EXPECT_EQ(state.energy_micro_eV, before.energy_micro_eV);
    EXPECT_EQ(state.track_id, before.track_id);
    EXPECT_EQ(state.parent_track_id, before.parent_track_id);
    EXPECT_EQ(state.current_navigator_id, before.current_navigator_id);
    EXPECT_EQ(state.current_volume_id, before.current_volume_id);
    EXPECT_EQ(state.material_id, before.material_id);
    EXPECT_EQ(state.region_id, before.region_id);
    EXPECT_EQ(state.particle_type, before.particle_type);
    EXPECT_EQ(state.status, before.status);
    EXPECT_EQ(state.generation, before.generation);
    EXPECT_EQ(std::bit_cast<std::uint32_t>(state.direction_x),
              std::bit_cast<std::uint32_t>(before.direction_x));
    EXPECT_EQ(std::bit_cast<std::uint32_t>(state.direction_y),
              std::bit_cast<std::uint32_t>(before.direction_y));
    EXPECT_EQ(std::bit_cast<std::uint32_t>(state.direction_z),
              std::bit_cast<std::uint32_t>(before.direction_z));
  }
}
