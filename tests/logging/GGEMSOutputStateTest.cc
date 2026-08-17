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
 * \brief Documents tests for GGEMS output-state buffering.
 *
 * Validates run-status storage, capacity clamping, ring-buffer ordering and wraparound, newest-line snapshots, clearing, reuse, and output-state sink forwarding.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <string>
#include <string_view>
#include <utility>

#include <gtest/gtest.h>

/// \endcond
#include "GGEMS/logging/GGEMSLogger.hh"
#include "GGEMS/logging/GGEMSOutputState.hh"
#include "GGEMS/logging/GGEMSOutputStateSink.hh"

/// \cond

namespace {

using ggems::core::GGEMSOutputState;
using ggems::core::GGEMSOutputStateSink;
using ggems::core::LogLevel;
using ggems::core::RenderedLogLine;
using ggems::core::RunStatus;

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeRenderedLine(std::string_view message)
    -> RenderedLogLine {
  RenderedLogLine line{};
  line.msg = std::string{message};
  line.module = "GGEMSOutputStateTest";
  return line;
}

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSOutputStateTest, StartsEmptyWithStartingStatus) {
  GGEMSOutputState state{};

  EXPECT_EQ(state.GetRunStatus(), RunStatus::Starting);
  EXPECT_EQ(state.GetLogCount(), 0U);
  EXPECT_TRUE(state.GetLastLogLinesSnapshot(state.GetLogCapacity()).empty());
  EXPECT_GT(state.GetLogCapacity(), 0U);
}

// =============================================================================
// =============================================================================

TEST(GGEMSOutputStateTest, RunStatusRoundTrips) {
  GGEMSOutputState state{};

  state.SetRunStatus(RunStatus::Running);
  EXPECT_EQ(state.GetRunStatus(), RunStatus::Running);

  state.SetRunStatus(RunStatus::Finished);
  EXPECT_EQ(state.GetRunStatus(), RunStatus::Finished);
}

// =============================================================================
// =============================================================================

TEST(GGEMSOutputStateTest, ZeroCapacityClampsToOneAndClearsExistingLogs) {
  GGEMSOutputState state{};

  state.PushLogLine(MakeRenderedLine("before"));
  ASSERT_EQ(state.GetLogCount(), 1U);

  state.SetLogCapacity(0U);

  EXPECT_EQ(state.GetLogCapacity(), 1U);
  EXPECT_EQ(state.GetLogCount(), 0U);
  EXPECT_TRUE(state.GetLastLogLinesSnapshot(1U).empty());
}

// =============================================================================
// =============================================================================

TEST(GGEMSOutputStateTest, KeepsInsertionOrderBeforeCapacityIsReached) {
  GGEMSOutputState state{};
  state.SetLogCapacity(3U);

  state.PushLogLine(MakeRenderedLine("first"));
  state.PushLogLine(MakeRenderedLine("second"));

  auto const snapshot = state.GetLastLogLinesSnapshot(state.GetLogCapacity());

  ASSERT_EQ(snapshot.size(), 2U);
  EXPECT_EQ(snapshot[0U].msg, "first");
  EXPECT_EQ(snapshot[1U].msg, "second");
}

// =============================================================================
// =============================================================================

TEST(GGEMSOutputStateTest, KeepsNewestLinesInOrderAfterWrap) {
  GGEMSOutputState state{};
  state.SetLogCapacity(3U);

  state.PushLogLine(MakeRenderedLine("first"));
  state.PushLogLine(MakeRenderedLine("second"));
  state.PushLogLine(MakeRenderedLine("third"));
  state.PushLogLine(MakeRenderedLine("fourth"));

  auto const snapshot = state.GetLastLogLinesSnapshot(state.GetLogCapacity());

  ASSERT_EQ(snapshot.size(), 3U);
  EXPECT_EQ(snapshot[0U].msg, "second");
  EXPECT_EQ(snapshot[1U].msg, "third");
  EXPECT_EQ(snapshot[2U].msg, "fourth");
  EXPECT_EQ(state.GetLogCount(), 3U);
}

// =============================================================================
// =============================================================================

TEST(GGEMSOutputStateTest, SnapshotReturnsOnlyRequestedNewestLines) {
  GGEMSOutputState state{};
  state.SetLogCapacity(4U);

  state.PushLogLine(MakeRenderedLine("first"));
  state.PushLogLine(MakeRenderedLine("second"));
  state.PushLogLine(MakeRenderedLine("third"));
  state.PushLogLine(MakeRenderedLine("fourth"));

  auto const snapshot = state.GetLastLogLinesSnapshot(2U);

  ASSERT_EQ(snapshot.size(), 2U);
  EXPECT_EQ(snapshot[0U].msg, "third");
  EXPECT_EQ(snapshot[1U].msg, "fourth");
  EXPECT_TRUE(state.GetLastLogLinesSnapshot(0U).empty());
}

// =============================================================================
// =============================================================================

TEST(GGEMSOutputStateTest, ClearLogsPreservesCapacityAndAllowsReuse) {
  GGEMSOutputState state{};
  state.SetLogCapacity(3U);

  state.PushLogLine(MakeRenderedLine("first"));
  state.PushLogLine(MakeRenderedLine("second"));

  state.ClearLogs();

  EXPECT_EQ(state.GetLogCapacity(), 3U);
  EXPECT_EQ(state.GetLogCount(), 0U);
  EXPECT_TRUE(state.GetLastLogLinesSnapshot(3U).empty());

  state.PushLogLine(MakeRenderedLine("after-clear"));

  auto const snapshot = state.GetLastLogLinesSnapshot(3U);
  ASSERT_EQ(snapshot.size(), 1U);
  EXPECT_EQ(snapshot.front().msg, "after-clear");
}

// =============================================================================
// =============================================================================

TEST(GGEMSOutputStateTest, SinkForwardsRenderedLineWithoutLosingMetadata) {
  GGEMSOutputState state{};
  state.SetLogCapacity(2U);
  GGEMSOutputStateSink sink{state};

  RenderedLogLine line{};
  line.prefix = "[prefix]";
  line.msg = "payload";
  line.level = LogLevel::Warn;
  line.depth = 7;
  line.module = "sink-test";

  sink.Write(std::move(line));

  auto const snapshot = state.GetLastLogLinesSnapshot(1U);

  ASSERT_EQ(snapshot.size(), 1U);
  EXPECT_EQ(snapshot.front().prefix, "[prefix]");
  EXPECT_EQ(snapshot.front().msg, "payload");
  EXPECT_EQ(snapshot.front().level, LogLevel::Warn);
  EXPECT_EQ(snapshot.front().depth, 7);
  EXPECT_EQ(snapshot.front().module, "sink-test");
}
/// \endcond
