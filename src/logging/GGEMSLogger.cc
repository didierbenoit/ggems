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
 * \brief Implements GGEMS log formatting, sinks, and singleton dispatch.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <iostream>
#include <format>
#include <string>
#include <chrono>
#include <ctime>
#include <cstdio>
#include <array>
#include <optional>
#include <cstdlib>
#include <mutex>
#include <utility>
#include <memory>
#include <vector>
#include <string_view>
#include <cstdint>
#include <source_location>

/// \endcond
#include "GGEMS/logging/GGEMSLogger.hh"
#include "GGEMS/GGEMSException.hh"

#include "GGEMS/render/GGEMSColor.hh"
#include "GGEMS/render/GGEMSColorNames.hh"
#include "GGEMS/logging/detail/GGEMSLoggerMetadata.hh"

namespace ggems::core {

// =============================================================================
// =============================================================================

/*!
 * \brief Maps a log severity to its GGEMS display color.
 *
 * \param[in] lvl Log severity.
 * \return Color used for the rendered log prefix.
 */
static auto LogLevelColor(LogLevel lvl) -> render::ColorKey {
  switch (lvl) {
  case LogLevel::Debug:
    return render::CYAN_Cryo;
  case LogLevel::Info:
    return render::GREEN_Neon;
  case LogLevel::Warn:
    return render::YELLOW_MotherAmber;
  case LogLevel::Error:
    return render::RED_XenoBlood;
  }
  return render::GREEN_Acid;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Builds the textual severity label shown in a log prefix.
 *
 * Informational records with positive depth use labels such as ``INFO2``.
 *
 * \param[in] rec Record whose severity and depth are formatted.
 * \return Textual severity label.
 */
static auto LogLevelName(LogRecord const &rec) -> std::string {
  switch (rec.level) {
  case LogLevel::Debug:
    return "DEBUG";
  case LogLevel::Info:
    if (rec.depth > 0) {
      return std::format("INFO{}", rec.depth);
    }
    return "INFO";
  case LogLevel::Warn:
    return "WARN";
  case LogLevel::Error:
    return "ERROR";
  }
  return "unknown";
}

// =============================================================================
// =============================================================================

/*!
 * \brief Formats a system-clock timestamp in local time with millisecond
 * precision.
 *
 * \param[in] time_point Timestamp to format.
 * \return Timestamp formatted as ``YYYY-MM-DD HH:MM:SS.mmm``.
 */
static auto FormatTimestamp(
    std::chrono::system_clock::time_point const &time_point) -> std::string {
  using namespace std::chrono;
  auto time = system_clock::to_time_t(time_point);
  auto m_sec =
      duration_cast<milliseconds>(time_point.time_since_epoch()) % 1000;
  std::tm tm_buf{};
#ifdef _WIN32
  localtime_s(&tm_buf, &time);
#else
  localtime_r(&time, &tm_buf);
#endif
  std::array<char, 64> buf;
  std::snprintf(buf.data(), buf.size(), "%04d-%02d-%02d %02d:%02d:%02d.%03d",
                tm_buf.tm_year + 1900, tm_buf.tm_mon + 1, tm_buf.tm_mday,
                tm_buf.tm_hour, tm_buf.tm_min, tm_buf.tm_sec,
                (int)m_sec.count());

  return std::string{buf.data()};
}

// =============================================================================
// =============================================================================

/*!
 * \brief Reads an environment variable without transferring platform-owned
 * storage.
 *
 * \param[in] name Environment-variable name.
 * \return Variable value when present, otherwise an empty optional.
 */
[[nodiscard]] auto GetEnvVar(const char *name) -> std::optional<std::string> {
#if defined(_WIN32)
  char *buffer = nullptr;
  std::size_t len = 0;
  if (_dupenv_s(&buffer, &len, name) == 0 && buffer != nullptr) {
    std::string value(buffer);
    std::free(buffer);
    return value;
  }
  return std::nullopt;
#else
  if (const char *value = std::getenv(name)) {
    return std::string(value);
  }

  return std::nullopt;
#endif
}

// =============================================================================
// =============================================================================

void StdoutSink::Write(RenderedLogLine &&log_line) {
  std::scoped_lock lock(mtx_);

  bool use_color = GGEMSLogger::GetInstance().UseColor();

  if (!log_line.prefix.empty()) {
    if (use_color) {
      std::cout << render::AnsiColor(log_line.color) << log_line.prefix
                << render::AnsiControlCode(render::AnsiControl::ResetColor)
                << ' ';
    } else {
      std::cout << log_line.prefix << ' ';
    }
  }

  std::cout << log_line.msg << '\n';
}

// =============================================================================
// =============================================================================

FileSink::FileSink(std::string path)
    : path_(std::move(path)), out_(path_, std::ios::out | std::ios::trunc) {
  if (!(out_)) {
    throw ggems::core::GGEMSFatal("Cannot open log file: " + path_);
  }
}

// -----------------------------------------------------------------------------

auto FileSink::Write(RenderedLogLine &&log_line) -> void {
  std::scoped_lock lock(mtx_);

  if (!log_line.prefix.empty()) {
    out_ << log_line.prefix << ' ';
  }

  out_ << log_line.msg << '\n';
}

// =============================================================================
// =============================================================================

auto LogFormatter::Format(LogRecord const &rec,
                          bool use_color) -> RenderedLogLine {
  RenderedLogLine log_line;
  log_line.msg = rec.message;
  log_line.level = rec.level;
  log_line.depth = rec.depth;
  log_line.module = rec.module;

  if (use_color) {
    log_line.color = LogLevelColor(rec.level);
  }

  auto const time_stamp = FormatTimestamp(rec.timestamp);
  std::string module_part = rec.module.empty() ? "" : " [" + rec.module + "]";
  std::string level_name = LogLevelName(rec);

  log_line.prefix =
      std::format("{} [{}] {{{}}}{} ({}):", time_stamp, level_name,
                  rec.thread_id, module_part, rec.function);
  return log_line;
}

// =============================================================================
// =============================================================================

auto GGEMSLogger::GetInstance() noexcept -> GGEMSLogger & {
  static GGEMSLogger instance;
  return instance;
}

// -----------------------------------------------------------------------------

auto GGEMSLogger::Log(LogLevel lvl, std::int32_t depth, std::string_view module,
                      std::source_location const &loc,
                      std::string_view msg) -> void {
  LogRecord rec;
  rec.timestamp = std::chrono::system_clock::now();
  rec.level = lvl;
  rec.depth = depth;
  rec.thread_id = logging::detail::ThreadTag();
  rec.module = std::string(module);
  rec.message = msg;
  rec.function = logging::detail::SimplifyFunctionName(loc.function_name());
  rec.file = loc.file_name();
  rec.line = static_cast<int>(loc.line());
  Dispatch(rec);
}

// -----------------------------------------------------------------------------

auto GGEMSLogger::ClearSinks() -> void {
  std::vector<std::unique_ptr<LogSink>> sinks_to_delete;

  {
    std::scoped_lock lock(mtx_);
    sinks_to_delete = std::move(sinks_);
  }
}

// -----------------------------------------------------------------------------

auto GGEMSLogger::AddSink(std::unique_ptr<LogSink> sink) -> void {
  if (!sink) {
    throw ggems::core::GGEMSFatal("Log sink is null.");
  }

  std::scoped_lock lock(mtx_);
  sinks_.push_back(std::move(sink));
}

// -----------------------------------------------------------------------------

auto GGEMSLogger::SetSink(std::unique_ptr<LogSink> sink) -> void {
  ClearSinks();
  AddSink(std::move(sink));
}

// -----------------------------------------------------------------------------

auto GGEMSLogger::SetForceColor(bool force) -> void {
  std::scoped_lock lock(mtx_);
  force_color_ = force;
}

// -----------------------------------------------------------------------------

auto GGEMSLogger::SetForceEncoding(Encoding encoding) -> void {
  std::scoped_lock lock(mtx_);
  encoding_ = encoding;
}

// -----------------------------------------------------------------------------

auto GGEMSLogger::UseColor() const -> bool {
  if (force_color_.has_value()) {
    return *force_color_;
  }

  if (auto no_color = GetEnvVar("NO_COLOR"); no_color && !no_color->empty()) {
    return false;
  }

  return true;
}

// -----------------------------------------------------------------------------

auto GGEMSLogger::Dispatch(LogRecord const &rec) -> void {
  RenderedLogLine log_line = LogFormatter::Format(rec, UseColor());

  std::scoped_lock lock(mtx_);

  for (std::unique_ptr<LogSink> const &sink : sinks_) {
    sink->Write(RenderedLogLine{log_line});
  }
}
} // namespace ggems::core
