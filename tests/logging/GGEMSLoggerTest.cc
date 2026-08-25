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
 * \brief Documents tests for the GGEMS logging pipeline.
 *
 * Validates log formatting, severity colors, plain-text file output, detail
 * filtering, sink replacement, null-sink rejection, and forced color/encoding
 * policies.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <chrono>
#include <filesystem>
#include <fstream>
#include <memory>
#include <source_location>
#include <string>
#include <utility>
#include <vector>
#include <iterator>

#include <gtest/gtest.h>

/// \endcond
#include "GGEMS/GGEMSException.hh"
#include "GGEMS/logging/GGEMSLogger.hh"
#include "GGEMS/render/GGEMSColorNames.hh"

/// \cond

namespace {

using ggems::core::Encoding;
using ggems::core::FileSink;
using ggems::core::GGEMSFatal;
using ggems::core::GGEMSLogger;
using ggems::core::LogFormatter;
using ggems::core::LogLevel;
using ggems::core::LogRecord;
using ggems::core::LogSink;
using ggems::core::RenderedLogLine;

// =============================================================================
// =============================================================================

struct CapturedLines {
  std::vector<RenderedLogLine> lines;
};

// =============================================================================
// =============================================================================

class CapturingSink final : public LogSink {
public:
  explicit CapturingSink(std::shared_ptr<CapturedLines> captured)
      : captured_(std::move(captured)) {}

  auto Write(RenderedLogLine &&log_line) -> void override {
    captured_->lines.push_back(std::move(log_line));
  }

private:
  std::shared_ptr<CapturedLines> captured_;
};

// =============================================================================
// =============================================================================

class GGEMSLoggerTest : public ::testing::Test {
protected:
  auto SetUp() -> void override {
    auto &logger = GGEMSLogger::GetInstance();
    original_encoding_ = logger.GetEncoding();
    original_use_color_ = logger.UseColor();

    logger.ClearSinks();
    logger.SetDetailLevel(1);
  }

  auto TearDown() -> void override {
    auto &logger = GGEMSLogger::GetInstance();
    logger.ClearSinks();
    logger.SetDetailLevel(1);
    logger.SetForceEncoding(original_encoding_);
    logger.SetForceColor(original_use_color_);
  }

private:
  Encoding original_encoding_{Encoding::Ascii};
  bool original_use_color_{false};
};

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSLogFormatterTest, PreservesRecordDataAndBuildsExpectedPrefix) {
  LogRecord record{};
  record.timestamp = std::chrono::system_clock::time_point{};
  record.level = LogLevel::Info;
  record.depth = 2;
  record.thread_id = "T42";
  record.module = "transport";
  record.message = "particle moved";
  record.function = "GGEMSRun::Run";

  RenderedLogLine const rendered = LogFormatter::Format(record, false);

  EXPECT_EQ(rendered.msg, "particle moved");
  EXPECT_EQ(rendered.level, LogLevel::Info);
  EXPECT_EQ(rendered.depth, 2);
  EXPECT_EQ(rendered.module, "transport");
  EXPECT_EQ(rendered.color, ggems::render::DEFAULT_FG);
  EXPECT_TRUE(
      rendered.prefix.ends_with("[INFO2] {T42} [transport] (GGEMSRun::Run):"));
}

// =============================================================================
// =============================================================================

TEST(GGEMSLogFormatterTest, SelectsLevelColorWhenColorIsEnabled) {
  LogRecord record{};
  record.timestamp = std::chrono::system_clock::time_point{};
  record.level = LogLevel::Warn;
  record.thread_id = "T0";
  record.message = "warning";
  record.function = "Check";

  RenderedLogLine const rendered = LogFormatter::Format(record, true);

  EXPECT_EQ(rendered.color, ggems::render::YELLOW_MotherAmber);
  EXPECT_TRUE(rendered.prefix.ends_with("[WARN] {T0} (Check):"));
}

// =============================================================================
// =============================================================================

TEST(GGEMSFileSinkTest, WritesPrefixAndMessageAsPlainText) {
  std::filesystem::path const path =
      std::filesystem::temp_directory_path() / "ggems_logger_test.log";
  std::filesystem::remove(path);

  {
    FileSink sink{path.string()};

    RenderedLogLine prefixed{};
    prefixed.prefix = "[prefix]";
    prefixed.msg = "first";
    sink.Write(std::move(prefixed));

    RenderedLogLine plain{};
    plain.msg = "second";
    sink.Write(std::move(plain));
  }

  std::ifstream input{path};
  ASSERT_TRUE(input.is_open());

  std::string contents{std::istreambuf_iterator<char>{input},
                       std::istreambuf_iterator<char>{}};

  EXPECT_EQ(contents, "[prefix] first\nsecond\n");

  input.close();
  std::filesystem::remove(path);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSLoggerTest, DispatchesFormattedMessageAndHonorsDetailLevel) {
  auto captured = std::make_shared<CapturedLines>();
  auto &logger = GGEMSLogger::GetInstance();
  logger.AddSink(std::make_unique<CapturingSink>(captured));

  constexpr int test_value{17};

  logger.LogFmt<LogLevel::Info>(2, "logger-test", "hidden {}",
                                std::source_location::current(), test_value);
  logger.LogFmt<LogLevel::Info>(1, "logger-test", "value={}",
                                std::source_location::current(), test_value);

  ASSERT_EQ(captured->lines.size(), 1U);
  EXPECT_EQ(captured->lines.front().msg, "value=17");
  EXPECT_EQ(captured->lines.front().level, LogLevel::Info);
  EXPECT_EQ(captured->lines.front().depth, 1);
  EXPECT_EQ(captured->lines.front().module, "logger-test");
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSLoggerTest, SetSinkReplacesPreviouslyInstalledSinks) {
  auto first_capture = std::make_shared<CapturedLines>();
  auto second_capture = std::make_shared<CapturedLines>();
  auto &logger = GGEMSLogger::GetInstance();

  logger.AddSink(std::make_unique<CapturingSink>(first_capture));
  logger.Log(LogLevel::Info, 0, "logger-test", std::source_location::current(),
             "before");

  logger.SetSink(std::make_unique<CapturingSink>(second_capture));
  logger.Log(LogLevel::Warn, 0, "logger-test", std::source_location::current(),
             "after");

  ASSERT_EQ(first_capture->lines.size(), 1U);
  EXPECT_EQ(first_capture->lines.front().msg, "before");

  ASSERT_EQ(second_capture->lines.size(), 1U);
  EXPECT_EQ(second_capture->lines.front().msg, "after");
  EXPECT_EQ(second_capture->lines.front().level, LogLevel::Warn);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSLoggerTest, RejectsNullSink) {
  auto &logger = GGEMSLogger::GetInstance();

  EXPECT_THROW(logger.AddSink(nullptr), GGEMSFatal);
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSLoggerTest, ForcedColorAndEncodingRoundTrip) {
  auto &logger = GGEMSLogger::GetInstance();

  logger.SetForceColor(false);
  EXPECT_FALSE(logger.UseColor());

  logger.SetForceColor(true);
  EXPECT_TRUE(logger.UseColor());

  logger.SetForceEncoding(Encoding::Unicode);
  EXPECT_EQ(logger.GetEncoding(), Encoding::Unicode);

  logger.SetForceEncoding(Encoding::Ascii);
  EXPECT_EQ(logger.GetEncoding(), Encoding::Ascii);
}
/// \endcond
