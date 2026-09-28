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
#include <type_traits>

#include <gtest/gtest.h>

#include "GGEMS/sources/GGEMSEnergyDistributionRecord.hh"
#include "GGEMS/sources/GGEMSSourceTypes.hh"

namespace {
using EnergyRecord = ggems::core::sources::GGEMSEnergyDistributionRecord;
}

// =============================================================================
// =============================================================================

TEST(GGEMSEnergyDistributionRecord, DefaultsToUnknownAndEmptyTable) {
  EnergyRecord const record{};
  EXPECT_EQ(record.regular_bin_width_micro_eV, 0ULL);
  EXPECT_EQ(record.table_offset, 0ULL);
  EXPECT_EQ(record.distribution_type,
            ggems::core::sources::ToKernelEnergyDistributionType(
              ggems::core::sources::GGEMSEnergyDistributionType::Unknown));
  EXPECT_EQ(record.table_count, 0U);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEnergyDistributionRecord, HasApprovedHostLayout) {
  EXPECT_TRUE(std::is_standard_layout_v<EnergyRecord>);
  EXPECT_TRUE(std::is_trivially_copyable_v<EnergyRecord>);
  EXPECT_EQ(sizeof(EnergyRecord), 24U);
  EXPECT_EQ(alignof(EnergyRecord), 8U);
  EXPECT_EQ(offsetof(EnergyRecord, regular_bin_width_micro_eV), 0U);
  EXPECT_EQ(offsetof(EnergyRecord, table_offset), 8U);
  EXPECT_EQ(offsetof(EnergyRecord, distribution_type), 16U);
  EXPECT_EQ(offsetof(EnergyRecord, table_count), 20U);
}

// =============================================================================
// =============================================================================

TEST(GGEMSEnergyDistributionRecord, PreservesEveryField) {
  EnergyRecord const record{
    .regular_bin_width_micro_eV = 2'000'000ULL,
    .table_offset = 17ULL,
    .distribution_type = ggems::core::sources::ToKernelEnergyDistributionType(
      ggems::core::sources::GGEMSEnergyDistributionType::RegularSpectrum),
    .table_count = 111U,
  };

  EXPECT_EQ(record.regular_bin_width_micro_eV, 2'000'000ULL);
  EXPECT_EQ(record.table_offset, 17ULL);
  EXPECT_EQ(record.distribution_type, 3U);
  EXPECT_EQ(record.table_count, 111U);
}
