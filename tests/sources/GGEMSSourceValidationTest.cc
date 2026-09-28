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

#include <cstdint>
#include <limits>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/sources/GGEMSSource.hh"
#include "GGEMS/sources/GGEMSSourceValidation.hh"
#include "GGEMS/sources/GGEMSSourceTypes.hh"
#include "GGEMS/sources/GGEMSSourceRecord.hh"

// =============================================================================
// =============================================================================

TEST(GGEMSSourceValidation, AcceptsDefaultPointFixedRecord) {
  ggems::core::sources::GGEMSSource source{};
  EXPECT_NO_THROW(
    ggems::core::sources::ValidateAnalyticSourceRecord(source.BuildRecord()));
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceValidation,
     RejectsUnsupportedGeometryUnknownAngularKindAndInvalidFrame) {
  ggems::core::sources::GGEMSSource source{};
  auto record = source.BuildRecord();

  record.emission_geometry_type = 99U;
  EXPECT_THROW(ggems::core::sources::ValidateAnalyticSourceRecord(record),
               ggems::core::GGEMSRecoverable);

  record = source.BuildRecord();
  record.angular_distribution_type = 0U;
  EXPECT_THROW(ggems::core::sources::ValidateAnalyticSourceRecord(record),
               ggems::core::GGEMSRecoverable);

  record = source.BuildRecord();
  record.axis_x_x = 0.0F;
  record.axis_x_y = 0.0F;
  record.axis_x_z = 0.0F;
  EXPECT_THROW(ggems::core::sources::ValidateAnalyticSourceRecord(record),
               ggems::core::GGEMSRecoverable);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceValidation, ChecksExactSignedEnvelopeDistances) {
  using ggems::core::sources::HasSignedPicoMeterEnvelope;

  EXPECT_TRUE(HasSignedPicoMeterEnvelope(0LL, 0ULL));
  EXPECT_TRUE(
    HasSignedPicoMeterEnvelope(std::numeric_limits<std::int64_t>::max(), 0ULL));
  EXPECT_TRUE(
    HasSignedPicoMeterEnvelope(std::numeric_limits<std::int64_t>::min(), 0ULL));
  EXPECT_FALSE(
    HasSignedPicoMeterEnvelope(std::numeric_limits<std::int64_t>::max(), 1ULL));
  EXPECT_FALSE(
    HasSignedPicoMeterEnvelope(std::numeric_limits<std::int64_t>::min(), 1ULL));
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceValidation, EnforcesCanonicalGeometryDimensions) {
  using ggems::core::sources::GGEMSEmissionGeometryType;
  using ggems::core::sources::ToKernelEmissionGeometryType;
  using ggems::core::sources::ValidateAnalyticSourceRecord;

  ggems::core::sources::GGEMSSource source{};
  auto record = source.BuildRecord();
  record.geometry_size_z_pm = 1ULL;
  EXPECT_THROW(ValidateAnalyticSourceRecord(record),
               ggems::core::GGEMSRecoverable);

  record = source.BuildRecord();
  record.emission_geometry_type =
    ToKernelEmissionGeometryType(GGEMSEmissionGeometryType::Rectangle);
  record.geometry_size_x_pm = 10ULL;
  record.geometry_size_y_pm = 20ULL;
  record.geometry_size_z_pm = 1ULL;
  EXPECT_THROW(ValidateAnalyticSourceRecord(record),
               ggems::core::GGEMSRecoverable);

  record = source.BuildRecord();
  record.emission_geometry_type =
    ToKernelEmissionGeometryType(GGEMSEmissionGeometryType::Box);
  record.geometry_size_x_pm = 10ULL;
  record.geometry_size_y_pm = 20ULL;
  record.geometry_size_z_pm = 0ULL;
  EXPECT_THROW(ValidateAnalyticSourceRecord(record),
               ggems::core::GGEMSRecoverable);

  record = source.BuildRecord();
  record.emission_geometry_type =
    ToKernelEmissionGeometryType(GGEMSEmissionGeometryType::Sphere);
  record.geometry_size_x_pm = 10ULL;
  record.geometry_size_y_pm = 10ULL;
  record.geometry_size_z_pm = 11ULL;
  EXPECT_THROW(ValidateAnalyticSourceRecord(record),
               ggems::core::GGEMSRecoverable);

  record = source.BuildRecord();
  record.emission_geometry_type =
    ToKernelEmissionGeometryType(GGEMSEmissionGeometryType::Cylinder);
  record.geometry_size_x_pm = 10ULL;
  record.geometry_size_y_pm = 11ULL;
  record.geometry_size_z_pm = 20ULL;
  EXPECT_THROW(ValidateAnalyticSourceRecord(record),
               ggems::core::GGEMSRecoverable);
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceValidation, RejectsInvalidBinary32AngularRecords) {
  using ggems::core::sources::ValidateAnalyticSourceRecord;

  ggems::core::sources::GGEMSSource source{};
  source.SetIsotropicAngularDistribution();
  auto record = source.BuildRecord();

  record.isotropic_cos_theta_lower = std::numeric_limits<float>::quiet_NaN();
  EXPECT_THROW(ValidateAnalyticSourceRecord(record),
               ggems::core::GGEMSRecoverable);

  record = source.BuildRecord();
  record.isotropic_cos_theta_lower = 0.5F;
  record.isotropic_cos_theta_upper = 0.5F;
  EXPECT_THROW(ValidateAnalyticSourceRecord(record),
               ggems::core::GGEMSRecoverable);

  record = source.BuildRecord();
  record.isotropic_phi_min_rad = 1.0F;
  record.isotropic_phi_max_rad = 1.0F;
  EXPECT_THROW(ValidateAnalyticSourceRecord(record),
               ggems::core::GGEMSRecoverable);

  record = source.BuildRecord();
  record.isotropic_phi_max_rad =
    ggems::core::sources::k_isotropic_full_sphere_phi_max_rad + 1.0F;
  EXPECT_THROW(ValidateAnalyticSourceRecord(record),
               ggems::core::GGEMSRecoverable);
}
