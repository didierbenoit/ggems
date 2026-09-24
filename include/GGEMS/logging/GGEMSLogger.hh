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
 * \brief Declares GGEMS logging records, sinks, formatting, and the singleton
 * logger.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <atomic>
#include <chrono>
#include <cstdint>
#include <mutex>
#include <string>
#include <source_location>
#include <fstream>
#include <string_view>
#include <memory>
#include <optional>
#include <vector>
#include <format>
/// \endcond

#include "GGEMS/render/GGEMSColorNames.hh"
#include "GGEMS/render/GGEMSColor.hh"

namespace ggems::core {

/*!
 * \brief Identifies the severity of a GGEMS log record.
 */
enum class LogLevel : std::uint8_t {
  Debug = 0, /*!<  Diagnostic message intended for development and debugging.*/
  Info,      /*!< Informational message. */
  Warn,      /*!< Warning message. */
  Error,     /*!< Error message. */
};

/*!
 * \brief Identifies the character encoding used for GGEMS textual output.
 */
enum class Encoding : std::uint8_t {
  Unicode = 0, /*!< Enables Unicode output where supported by the active sink.*/
  Ascii,       /*!< Restricts output to the portable ASCII representation. */
};

/*!
 * \brief Stores the metadata and message associated with one log event.
 */
struct LogRecord {
  std::chrono::system_clock::time_point
    timestamp; /*!< Timestamp assigned when the record is created. */
  LogLevel level{LogLevel::Info}; /*!< Severity of the record. */
  std::int32_t depth{0}; /*!< Verbosity depth used for detail filtering. */
  std::string thread_id; /*!< Stable textual tag of the emitting thread. */
  std::string module;    /*!< GGEMS module associated with the message */
  std::string message;   /*!< Formatted message payload */
  std::string function;  /*!< Simplified source function name */
};

/*!
 * \brief Stores a sink-ready log line and the metadata retained after
 * formatting.
 */
struct RenderedLogLine {
  std::string prefix; /*!< Rendered metadata prefix. */
  std::string msg;    /*!< Rendered message payload. */
  render::ColorKey color{
    render::DEFAULT_FG}; /*!< Color associated with the rendered prefix. */
  LogLevel level{LogLevel::Info}; /*!< Original record severity. */
  std::int32_t depth{0};          /*!< Original record verbosity. */
  std::string module; /*!< Module retained from the source record. */
};

/*!
 * \brief Defines the interface implemented by GGEMS log destinations.
 */
class LogSink {
public:
  /*!
   * \brief Destroys the log sink.
   */
  virtual ~LogSink() = default;

  /*!
   * \brief Consumes one rendered log line.
   *
   * \param[in] log_line Rendered line transferred to the sink.
   */
  virtual auto Write(RenderedLogLine &&log_line) -> void = 0;
};

/*!
 * \brief Writes rendered GGEMS log lines to a plain-text file.
 */
class FileSink final : public LogSink {
public:
  /*!
   * \brief Opens a log file and truncates any existing contents.
   *
   * \param[in] path Destination file path.
   * \throws GGEMSFatal If the file cannot be opened.
   */
  explicit FileSink(std::string path);

  /*!
   * \brief Writes a rendered line without ANSI color control sequences.
   *
   * \param[in] log_line Rendered line transferred to the file sink.
   */
  auto Write(RenderedLogLine &&log_line) -> void override;

private:
  std::string path_;  /*!< Destination log-file path. */
  std::ofstream out_; /*!< Output stream owned by the sink. */
  std::mutex mtx_;    /*!< Mutex serializing writes to the file stream. */
};

/*!
 * \brief Writes rendered GGEMS log lines to standard output.
 */
class StdoutSink : public LogSink {
public:
  /*!
   * \brief Constructs a standard-output sink.
   */
  StdoutSink() = default;

  /*!
   * \brief Writes a rendered line to standard output.
   *
   * Applies ANSI prefix coloring when the logger color policy enables it.
   *
   * \param[in] log_line Rendered line transferred to the standard-output sink.
   */
  auto Write(RenderedLogLine &&log_line) -> void override;

private:
  std::mutex mtx_; /*!< Mutex serializing writes to standard output. */
};

/*!
 * \brief Converts structured log records into sink-ready rendered lines.
 */
class LogFormatter {
public:
  /*!
   * \brief Formats one structured log record.
   *
   * \param[in] rec Source log record.
   * \param[in] use_color Whether a severity color should be attached.
   * \return Rendered log line containing the prefix, message, and retained
   * metadata.
   */
  static auto Format(LogRecord const &rec, bool use_color) -> RenderedLogLine;
};

/*!
 * \brief Provides the process-wide GGEMS logging interface.
 *
 * The singleton owns the active log sinks, detail filter, color policy, and
 * output encoding used by GGEMS logging front ends.
 */
class GGEMSLogger {
public:
  /*!
   * \brief Returns the process-wide GGEMS logger singleton.
   *
   * \return Reference to the singleton logger.
   */
  static auto GetInstance() noexcept -> GGEMSLogger &;

  /*!
   * \brief Copy construction is disabled.
   */
  GGEMSLogger(GGEMSLogger const &) = delete;

  /*!
   * \brief Move construction is disabled.
   */
  GGEMSLogger(GGEMSLogger &&) = delete;

  /*!
   * \brief Copy assignment is disabled.
   */
  auto operator=(GGEMSLogger const &) -> GGEMSLogger & = delete;

  /*!
   * \brief Move assignment is disabled.
   */
  auto operator=(GGEMSLogger &&) -> GGEMSLogger & = delete;

  /*!
   * \brief Removes all currently installed log sinks.
   */
  auto ClearSinks() -> void;

  /*!
   * \brief Adds a log sink and transfers its ownership to the logger.
   *
   * \param[in] sink Sink to install.
   * \throws GGEMSFatal If \p sink is null.
   */
  auto AddSink(std::unique_ptr<LogSink> sink) -> void;

  /*!
   * \brief Forces the logger color policy to an explicit value.
   *
   * \param[in] force Whether ANSI color output is enabled.
   */
  auto SetForceColor(bool force) -> void;

  /*!
   * \brief Returns whether the active logger policy uses color.
   *
   * Before an explicit color policy is set, the NO_COLOR environment variable
   * disables color when it contains a non-empty value.
   *
   * \return True when color output is enabled.
   */
  auto UseColor() const -> bool;

  /*!
   * \brief Returns the current logger text encoding.
   *
   * \return Current output encoding.
   */
  auto GetEncoding() const noexcept -> Encoding { return encoding_; }

  /*!
   * \brief Sets the logger text encoding.
   *
   * \param[in] encoding Encoding used by GGEMS textual output.
   */
  auto SetForceEncoding(Encoding encoding) -> void;

  /*!
   * \brief Sets the maximum verbosity depth accepted by formatted logging.
   *
   * Messages whose depth is greater than this value are discarded before
   * formatting.
   *
   * \param[in] detail_level Maximum accepted logging depth.
   */
  auto SetDetailLevel(std::int32_t detail_level) noexcept -> void {
    detail_level_.store(detail_level, std::memory_order_relaxed);
  }

  /*!
   * \brief Creates and dispatches one structured log record.
   *
   * \param[in] lvl Severity level.
   * \param[in] depth Verbosity depth associated with the message.
   * \param[in] module GGEMS module name.
   * \param[in] loc Source location associated with the record.
   * \param[in] msg Formatted message payload.
   */
  auto Log(LogLevel lvl, std::int32_t depth, std::string_view module,
           std::source_location const &loc = std::source_location::current(),
           std::string_view msg = "") -> void;

  /*!
   * \brief Formats and logs a message at a compile-time severity level.
   *
   * Messages above the configured detail level are discarded before the format
   * string is evaluated.
   *
   * \tparam Level Compile-time log severity.
   * \tparam Args Format-argument types.
   * \param[in] depth Verbosity depth associated with the message.
   * \param[in] module GGEMS module name.
   * \param[in] fmt_runtime Runtime format string.
   * \param[in] loc Source location associated with the record.
   * \param[in] args Arguments consumed by the runtime formatter.
   */
  template <LogLevel Level, typename... Args>
  auto LogFmt(std::int32_t depth, std::string_view module,
              std::string_view fmt_runtime,
              std::source_location loc = std::source_location::current(),
              Args &&...args) -> void {
    std::int32_t detail_level = detail_level_.load(std::memory_order_relaxed);
    if (depth > detail_level) {
      return;
    }

    std::string line;
    if constexpr (sizeof...(Args) == 0) {
      line = std::string(fmt_runtime);
    } else {
      line = std::vformat(fmt_runtime, std::make_format_args(args...));
    }
    Log(Level, depth, module, loc, line);
  }

private:
  /*!
   * \brief Constructs the process-wide logger instance.
   */
  GGEMSLogger() = default;

  /*!
   * \brief Formats a record and forwards it to every installed sink.
   *
   * \param[in] rec Structured record to dispatch.
   */
  void Dispatch(LogRecord const &rec);

  std::atomic<std::int32_t> detail_level_{
    1}; /*!< Maximum accepted logging detail depth. */
  mutable std::mutex
    mtx_; /*!< Mutex protecting logger configuration and sink dispatch. */
  std::vector<std::unique_ptr<LogSink>>
    sinks_; /*!< Owned log sinks receiving rendered lines. */
  std::optional<bool>
    force_color_; /*!< Optional explicit color-policy override. */
  Encoding encoding_{
    Encoding::Ascii,
  }; /*!< Current text encoding used by GGEMS output. */
};
} // namespace ggems::core
