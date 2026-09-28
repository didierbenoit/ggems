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

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/particles/GGEMSPrimaryStream.hh"

// =============================================================================
// =============================================================================

TEST(GGEMSPrimaryStream, ProducesContiguousDisjointFixedCountRanges) {
  constexpr std::uint64_t k_primary_count{17ULL};

  ggems::core::particles::GGEMSPrimaryStream stream{};

  auto first = stream.PrepareRun(0ULL, k_primary_count);
  auto second = stream.PrepareRun(1ULL, k_primary_count);
  auto third = stream.PrepareRun(2ULL, k_primary_count);

  EXPECT_EQ(first.run_id, 0ULL);
  EXPECT_EQ(second.run_id, 1ULL);
  EXPECT_EQ(third.run_id, 2ULL);

  EXPECT_EQ(first.source_primary_count, k_primary_count);
  EXPECT_EQ(second.source_primary_count, k_primary_count);
  EXPECT_EQ(third.source_primary_count, k_primary_count);

  EXPECT_EQ(first.global_history_offset, 0ULL);
  EXPECT_LT(first.global_history_offset, second.global_history_offset);
  EXPECT_LT(second.global_history_offset, third.global_history_offset);

  std::uint64_t first_end =
    first.global_history_offset + first.source_primary_count;

  std::uint64_t second_end =
    second.global_history_offset + second.source_primary_count;

  EXPECT_EQ(first_end, second.global_history_offset);
  EXPECT_EQ(second_end, third.global_history_offset);
}

// =============================================================================
// =============================================================================

TEST(GGEMSPrimaryStream, ReservesContiguousDisjointVariableCountRanges) {
  ggems::core::particles::GGEMSPrimaryStream stream{};

  auto first = stream.PrepareRun(0ULL, 3ULL);
  auto second = stream.PrepareRun(1ULL, 7ULL);
  auto third = stream.PrepareRun(2ULL, 1ULL);
  auto fourth = stream.PrepareRun(3ULL, 5ULL);

  EXPECT_EQ(first.run_id, 0ULL);
  EXPECT_EQ(second.run_id, 1ULL);
  EXPECT_EQ(third.run_id, 2ULL);
  EXPECT_EQ(fourth.run_id, 3ULL);

  EXPECT_EQ(first.source_primary_count, 3ULL);
  EXPECT_EQ(second.source_primary_count, 7ULL);
  EXPECT_EQ(third.source_primary_count, 1ULL);
  EXPECT_EQ(fourth.source_primary_count, 5ULL);

  EXPECT_EQ(first.global_history_offset, 0ULL);
  EXPECT_EQ(second.global_history_offset, 3ULL);
  EXPECT_EQ(third.global_history_offset, 10ULL);
  EXPECT_EQ(fourth.global_history_offset, 11ULL);

  EXPECT_EQ(first.global_history_offset + first.source_primary_count,
            second.global_history_offset);
  EXPECT_EQ(second.global_history_offset + second.source_primary_count,
            third.global_history_offset);
  EXPECT_EQ(third.global_history_offset + third.source_primary_count,
            fourth.global_history_offset);
}

// =============================================================================
// =============================================================================

TEST(GGEMSPrimaryStream, KeepsRunLabelsIndependentFromReservedRanges) {
  ggems::core::particles::GGEMSPrimaryStream stream{};

  auto first = stream.PrepareRun(42ULL, 3ULL);
  auto second = stream.PrepareRun(42ULL, 5ULL);
  auto third = stream.PrepareRun(7ULL, 2ULL);

  EXPECT_EQ(first.run_id, 42ULL);
  EXPECT_EQ(second.run_id, 42ULL);
  EXPECT_EQ(third.run_id, 7ULL);

  EXPECT_EQ(first.source_primary_count, 3ULL);
  EXPECT_EQ(second.source_primary_count, 5ULL);
  EXPECT_EQ(third.source_primary_count, 2ULL);

  EXPECT_EQ(first.global_history_offset, 0ULL);
  EXPECT_EQ(second.global_history_offset, 3ULL);
  EXPECT_EQ(third.global_history_offset, 8ULL);

  EXPECT_NE(first.global_history_offset, second.global_history_offset);
  EXPECT_EQ(first.global_history_offset + first.source_primary_count,
            second.global_history_offset);
  EXPECT_EQ(second.global_history_offset + second.source_primary_count,
            third.global_history_offset);
}

// =============================================================================
// =============================================================================

TEST(GGEMSPrimaryStream, EmptyReservationDoesNotConsumeIdentifiers) {
  ggems::core::particles::GGEMSPrimaryStream stream{};

  auto const empty = stream.PrepareRun(41ULL, 0ULL);
  EXPECT_EQ(empty.run_id, 41ULL);
  EXPECT_EQ(empty.source_primary_count, 0ULL);
  EXPECT_EQ(empty.global_history_offset, 0ULL);

  auto reservation = stream.PrepareRun(42ULL, 4ULL);

  EXPECT_EQ(reservation.run_id, 42ULL);
  EXPECT_EQ(reservation.source_primary_count, 4ULL);
  EXPECT_EQ(reservation.global_history_offset, 0ULL);
}

// =============================================================================
// =============================================================================
