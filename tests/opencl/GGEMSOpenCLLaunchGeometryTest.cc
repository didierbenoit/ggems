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
 * \brief Unit tests for OpenCL launch-geometry helpers.
 *
 * Validates padded global work-size computation, rejection of zero local work sizes, representative explicit cases, and size-limit boundary handling.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <array>
#include <cstddef>
#include <limits>

#include <gtest/gtest.h>

/// \endcond
#include "GGEMS/opencl/GGEMSOpenCLLaunchGeometry.hh"

/// \cond

namespace {
struct PaddedGlobalWorkSizeCase {
  char const *label;
  std::size_t logical_work_size;
  std::size_t local_work_size;
  std::size_t expected_global_work_size;
};
} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLLaunchGeometryTest, RejectsZeroLocalWorkSize) {
  EXPECT_FALSE(
      ggems::ocl::detail::TryComputePaddedGlobalWorkSize(1U, 0U).has_value());
  EXPECT_FALSE(
      ggems::ocl::detail::TryComputePaddedGlobalWorkSize(0U, 0U).has_value());
}

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLLaunchGeometryTest, ComputesExplicitRepresentativeCases) {
  constexpr std::array<PaddedGlobalWorkSizeCase, 10U> test_cases{{
      {.label = "zero logical work size",
       .logical_work_size = 0U,
       .local_work_size = 64U,
       .expected_global_work_size = 0U},
      {.label = "local work size one",
       .logical_work_size = 257U,
       .local_work_size = 1U,
       .expected_global_work_size = 257U},
      {.label = "exact multiple",
       .logical_work_size = 128U,
       .local_work_size = 64U,
       .expected_global_work_size = 128U},
      {.label = "ordinary non-multiple",
       .logical_work_size = 17U,
       .local_work_size = 8U,
       .expected_global_work_size = 24U},
      {.label = "one below a multiple",
       .logical_work_size = 127U,
       .local_work_size = 64U,
       .expected_global_work_size = 128U},
      {.label = "non-power-of-two local work size",
       .logical_work_size = 10U,
       .local_work_size = 6U,
       .expected_global_work_size = 12U},
      {.label = "GGEMS aligned launch",
       .logical_work_size = 64U,
       .local_work_size = 64U,
       .expected_global_work_size = 64U},
      {.label = "GGEMS one-over launch",
       .logical_work_size = 65U,
       .local_work_size = 64U,
       .expected_global_work_size = 128U},
      {.label = "GGEMS multi-group launch",
       .logical_work_size = 257U,
       .local_work_size = 64U,
       .expected_global_work_size = 320U},
      {.label = "local work size larger than logical work size",
       .logical_work_size = 5U,
       .local_work_size = 8U,
       .expected_global_work_size = 8U},
  }};

  for (auto const &test_case : test_cases) {
    SCOPED_TRACE(test_case.label);
    auto const padded_global_work_size =
        ggems::ocl::detail::TryComputePaddedGlobalWorkSize(
            test_case.logical_work_size, test_case.local_work_size);
    ASSERT_TRUE(padded_global_work_size.has_value());
    EXPECT_EQ(*padded_global_work_size, test_case.expected_global_work_size);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLLaunchGeometryTest, HandlesSizeMaximumBoundaries) {
  constexpr std::size_t k_max_size{std::numeric_limits<std::size_t>::max()};

  auto const representable_rounded_work_size =
      ggems::ocl::detail::TryComputePaddedGlobalWorkSize(k_max_size - 2U, 2U);
  ASSERT_TRUE(representable_rounded_work_size.has_value());
  EXPECT_EQ(*representable_rounded_work_size, k_max_size - 1U);

  auto const aligned_work_size =
      ggems::ocl::detail::TryComputePaddedGlobalWorkSize(k_max_size - 1U, 2U);
  ASSERT_TRUE(aligned_work_size.has_value());
  EXPECT_EQ(*aligned_work_size, k_max_size - 1U);

  auto const overflowing_work_size =
      ggems::ocl::detail::TryComputePaddedGlobalWorkSize(k_max_size, 2U);
  EXPECT_FALSE(overflowing_work_size.has_value());
}
/// \endcond
