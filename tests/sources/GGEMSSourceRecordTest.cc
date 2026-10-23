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
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include <gtest/gtest.h>

#include "GGEMS/sources/GGEMSSourceRecord.hh"
#include "GGEMS/sources/GGEMSSourceDeviceRecord.hh"
#include "GGEMS/sources/GGEMSSource.hh"
#include "GGEMS/GGEMSException.hh"
#include "GGEMS/sources/GGEMSSourceTypes.hh"
#include "GGEMS/particles/GGEMSParticleTypes.hh"

// =============================================================================
// =============================================================================

TEST(GGEMSSourceDeviceRecord, IsKernelFriendly) {
  using SourceRecord = ggems::core::sources::GGEMSSourceDeviceRecord;

  EXPECT_TRUE(std::is_standard_layout_v<SourceRecord>);
  EXPECT_TRUE(std::is_trivially_copyable_v<SourceRecord>);

  EXPECT_EQ(sizeof(SourceRecord), 144U);
  EXPECT_EQ(alignof(SourceRecord), 8U);

  EXPECT_EQ(offsetof(SourceRecord, source_id), 0U);
  EXPECT_EQ(offsetof(SourceRecord, time_start_ps), 8U);
  EXPECT_EQ(offsetof(SourceRecord, time_stop_ps), 16U);
  EXPECT_EQ(offsetof(SourceRecord, energy_micro_eV), 24U);

  EXPECT_EQ(offsetof(SourceRecord, position_x_pm), 32U);
  EXPECT_EQ(offsetof(SourceRecord, position_y_pm), 36U);
  EXPECT_EQ(offsetof(SourceRecord, position_z_pm), 40U);

  EXPECT_EQ(offsetof(SourceRecord, source_type), 44U);
  EXPECT_EQ(offsetof(SourceRecord, emitted_particle_type), 48U);
  EXPECT_EQ(offsetof(SourceRecord, flags), 52U);
  EXPECT_EQ(offsetof(SourceRecord, reserved_0), 56U);

  EXPECT_EQ(offsetof(SourceRecord, axis_x_x), 60U);
  EXPECT_EQ(offsetof(SourceRecord, axis_x_y), 64U);
  EXPECT_EQ(offsetof(SourceRecord, axis_x_z), 68U);
  EXPECT_EQ(offsetof(SourceRecord, axis_y_x), 72U);
  EXPECT_EQ(offsetof(SourceRecord, axis_y_y), 76U);
  EXPECT_EQ(offsetof(SourceRecord, axis_y_z), 80U);
  EXPECT_EQ(offsetof(SourceRecord, axis_z_x), 84U);
  EXPECT_EQ(offsetof(SourceRecord, axis_z_y), 88U);
  EXPECT_EQ(offsetof(SourceRecord, axis_z_z), 92U);
  EXPECT_EQ(offsetof(SourceRecord, emission_geometry_type), 96U);
  EXPECT_EQ(offsetof(SourceRecord, angular_distribution_type), 100U);
  EXPECT_EQ(offsetof(SourceRecord, geometry_size_x_pm), 104U);
  EXPECT_EQ(offsetof(SourceRecord, geometry_size_y_pm), 108U);
  EXPECT_EQ(offsetof(SourceRecord, focus_position_x_pm), 112U);
  EXPECT_EQ(offsetof(SourceRecord, focus_position_y_pm), 116U);
  EXPECT_EQ(offsetof(SourceRecord, focus_position_z_pm), 120U);
  EXPECT_EQ(offsetof(SourceRecord, geometry_size_z_pm), 124U);
  EXPECT_EQ(offsetof(SourceRecord, isotropic_cos_theta_lower), 128U);
  EXPECT_EQ(offsetof(SourceRecord, isotropic_cos_theta_upper), 132U);
  EXPECT_EQ(offsetof(SourceRecord, isotropic_phi_min_rad), 136U);
  EXPECT_EQ(offsetof(SourceRecord, isotropic_phi_max_rad), 140U);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceRecord, DefaultSourceRecordIsAnalyticGammaPointSource) {
  ggems::core::sources::GGEMSSourceRecord source{};

  EXPECT_EQ(source.source_type,
            ggems::core::sources::ToKernelSourceType(
              ggems::core::sources::GGEMSSourceType::Analytic));

  EXPECT_EQ(source.emitted_particle_type,
            ggems::core::particles::ToKernelParticleType(
              ggems::core::particles::GGEMSParticleType::Gamma));

  EXPECT_EQ(source.energy_micro_eV, 511'000'000'000ULL);

  EXPECT_EQ(source.position_x_pm, 0LL);
  EXPECT_EQ(source.position_y_pm, 0LL);
  EXPECT_EQ(source.position_z_pm, 0LL);

  EXPECT_FLOAT_EQ(source.axis_x_x, 1.0F);
  EXPECT_FLOAT_EQ(source.axis_x_y, 0.0F);
  EXPECT_FLOAT_EQ(source.axis_x_z, 0.0F);
  EXPECT_FLOAT_EQ(source.axis_y_x, 0.0F);
  EXPECT_FLOAT_EQ(source.axis_y_y, 1.0F);
  EXPECT_FLOAT_EQ(source.axis_y_z, 0.0F);
  EXPECT_FLOAT_EQ(source.axis_z_x, 0.0F);
  EXPECT_FLOAT_EQ(source.axis_z_y, 0.0F);
  EXPECT_FLOAT_EQ(source.axis_z_z, 1.0F);

  EXPECT_EQ(source.emission_geometry_type,
            ggems::core::sources::ToKernelEmissionGeometryType(
              ggems::core::sources::GGEMSEmissionGeometryType::Point));
  EXPECT_EQ(source.angular_distribution_type,
            ggems::core::sources::ToKernelAngularDistributionType(
              ggems::core::sources::GGEMSAngularDistributionType::Fixed));
  EXPECT_EQ(source.geometry_size_x_pm, 0ULL);
  EXPECT_EQ(source.geometry_size_y_pm, 0ULL);
  EXPECT_EQ(source.geometry_size_z_pm, 0ULL);
  EXPECT_EQ(source.focus_position_x_pm, 0LL);
  EXPECT_EQ(source.focus_position_y_pm, 0LL);
  EXPECT_EQ(source.focus_position_z_pm, 0LL);
  EXPECT_FLOAT_EQ(
    source.isotropic_cos_theta_lower,
    ggems::core::sources::k_isotropic_full_sphere_cos_theta_lower);
  EXPECT_FLOAT_EQ(
    source.isotropic_cos_theta_upper,
    ggems::core::sources::k_isotropic_full_sphere_cos_theta_upper);
  EXPECT_FLOAT_EQ(source.isotropic_phi_min_rad,
                  ggems::core::sources::k_isotropic_full_sphere_phi_min_rad);
  EXPECT_FLOAT_EQ(source.isotropic_phi_max_rad,
                  ggems::core::sources::k_isotropic_full_sphere_phi_max_rad);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceDeviceRecord, ConvertsSpatialConfigurationOnlyAtUpload) {
  ggems::core::sources::GGEMSSource source{};
  constexpr std::int64_t origin{(1LL << 60U) + 123LL};
  source.SetPositionPicoMeter(origin, -origin, 101LL)
    .SetBoxEmissionPicoMeter(16'777'217ULL, 3ULL, 5ULL);
  auto canonical = source.BuildRecord();
  canonical.source_id = (1ULL << 50U) + 7ULL;
  canonical.time_start_ps = (1ULL << 60U) + 11ULL;
  canonical.energy_micro_eV = (1ULL << 50U) + 13ULL;

  auto const device = ggems::core::sources::BuildSourceDeviceRecord(canonical);

  EXPECT_EQ(source.BuildRecord().position_x_pm, origin);
  EXPECT_EQ(source.BuildRecord().geometry_size_x_pm, 16'777'217ULL);
  EXPECT_EQ(device.position_x_pm, static_cast<float>(origin));
  EXPECT_EQ(device.position_y_pm, static_cast<float>(-origin));
  EXPECT_EQ(device.position_z_pm, 101.0F);
  EXPECT_EQ(device.geometry_size_x_pm, 16'777'216.0F);
  EXPECT_EQ(device.geometry_size_y_pm, 3.0F);
  EXPECT_EQ(device.geometry_size_z_pm, 5.0F);
  EXPECT_EQ(device.source_id, canonical.source_id);
  EXPECT_EQ(device.time_start_ps, canonical.time_start_ps);
  EXPECT_EQ(device.energy_micro_eV, canonical.energy_micro_eV);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceDeviceRecord, RejectsFocusedPointCollapsedByFP32Conversion) {
  ggems::core::sources::GGEMSSource source{};
  constexpr std::int64_t origin{1LL << 60U};
  source.SetPositionPicoMeter(origin, 0LL, 0LL)
    .SetFocusedAngularDistributionPicoMeter(origin + 1LL, 0LL, 0LL);
  auto const canonical = source.BuildRecord();

  EXPECT_NE(canonical.position_x_pm, canonical.focus_position_x_pm);
  EXPECT_THROW(
    static_cast<void>(ggems::core::sources::BuildSourceDeviceRecord(canonical)),
    ggems::core::GGEMSRecoverable);
}
