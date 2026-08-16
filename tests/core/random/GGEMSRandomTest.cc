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
 * \brief Unit tests for GGEMS random configuration.
 *
 * Validates engine parsing and selection, seed handling, kernel identifiers and build definitions, state-size reporting, and verbose summaries.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <algorithm>
#include <string>
#include <vector>
#include <cstdint>

#include <gtest/gtest.h>

/// \endcond
#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/random/GGEMSRandom.hh"
#include "GGEMS/core/random/GGEMSRandomEngine.hh"

/// \cond

namespace {

using ggems::core::random::GGEMSRandom;
using ggems::core::random::GGEMSRandomEngine;
using ggems::core::random::ParseRandomEngine;
using ggems::core::random::ToKernelEngineId;
using ggems::core::random::ToString;

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSRandomTest, DefaultsToPhilox) {
  GGEMSRandom random{};

  EXPECT_EQ(random.GetEngine(), GGEMSRandomEngine::Philox);
  EXPECT_EQ(random.GetEngineName(), "Philox");
  EXPECT_EQ(random.GetSeed(), 77'777ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSRandomTest, EngineNamesAndKernelIdsAreStable) {
  EXPECT_EQ(ToString(GGEMSRandomEngine::JKISS), "JKISS");
  EXPECT_EQ(ToString(GGEMSRandomEngine::PCG32), "PCG32");
  EXPECT_EQ(ToString(GGEMSRandomEngine::Philox), "Philox");

  EXPECT_EQ(ToKernelEngineId(GGEMSRandomEngine::JKISS), 1U);
  EXPECT_EQ(ToKernelEngineId(GGEMSRandomEngine::PCG32), 2U);
  EXPECT_EQ(ToKernelEngineId(GGEMSRandomEngine::Philox), 3U);
}

// =============================================================================
// =============================================================================

TEST(GGEMSRandomTest, ParsesCanonicalEngineNames) {
  EXPECT_EQ(ParseRandomEngine("JKISS"), GGEMSRandomEngine::JKISS);
  EXPECT_EQ(ParseRandomEngine("PCG32"), GGEMSRandomEngine::PCG32);
  EXPECT_EQ(ParseRandomEngine("Philox"), GGEMSRandomEngine::Philox);

  EXPECT_EQ(ParseRandomEngine("jkiss"), GGEMSRandomEngine::JKISS);
  EXPECT_EQ(ParseRandomEngine("pcg32"), GGEMSRandomEngine::PCG32);
  EXPECT_EQ(ParseRandomEngine("philox"), GGEMSRandomEngine::Philox);
}

// =============================================================================
// =============================================================================

TEST(GGEMSRandomTest, RejectsUnknownEngineName) {
  EXPECT_THROW((void)ParseRandomEngine("unknown"),
               ggems::core::GGEMSExceptionBase);
}

// =============================================================================
// =============================================================================

TEST(GGEMSRandomTest, KernelBuildDefinitionTracksSelectedEngine) {
  GGEMSRandom random{};

  EXPECT_EQ(random.GetKernelBuildDefinition(), "-DGGEMS_RANDOM_ENGINE=3");

  random.SetEngine(GGEMSRandomEngine::JKISS);
  EXPECT_EQ(random.GetKernelBuildDefinition(), "-DGGEMS_RANDOM_ENGINE=1");

  random.SetEngine(GGEMSRandomEngine::PCG32);
  EXPECT_EQ(random.GetKernelBuildDefinition(), "-DGGEMS_RANDOM_ENGINE=2");
}

// =============================================================================
// =============================================================================

TEST(GGEMSRandomTest, SummaryReflectsCurrentConfiguration) {
  constexpr std::uint64_t test_seed{123'456ULL};

  GGEMSRandom random{};
  random.SetEngine(GGEMSRandomEngine::PCG32).SetSeed(test_seed);

  std::vector<std::string> const lines = random.BuildSummaryLines();

  EXPECT_TRUE(std::ranges::contains(lines, "Random engine           : PCG32"));

  EXPECT_TRUE(std::ranges::contains(lines, "Seed                    : 123456"));

  EXPECT_TRUE(std::ranges::contains(lines, "OpenCL engine id        : 2"));

  EXPECT_TRUE(std::ranges::contains(lines, "OpenCL build definition : "
                                           "-DGGEMS_RANDOM_ENGINE=2"));
}
/// \endcond
