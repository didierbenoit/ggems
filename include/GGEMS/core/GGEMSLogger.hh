#pragma once

// ************************************************************************
// * This file is part of GGEMS.                                          *
// *                                                                      *
// * GGEMS is free software: you can redistribute it and/or modify        *
// * it under the terms of the GNU General Public License as published by *
// * the Free Software Foundation, either version 3 of the License, or    *
// * (at your option) any later version.                                  *
// *                                                                      *
// * GGEMS is distributed in the hope that it will be useful,             *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
// * GNU General Public License for more details.                         *
// *                                                                      *
// * You should have received a copy of the GNU General Public License    *
// * along with GGEMS.  If not, see <https://www.gnu.org/licenses/>.      *
// *                                                                      *
// ************************************************************************

/*!
 * \file GGEMSLogger.hh
 * \brief Definition of the GGEMSLogger singleton and the \c gglog namespace helpers.
 *
 * This header declares:
 * - the \c gglog namespace (log levels, front-end streaming helpers, formatting utilities),
 * - the \c GGEMSLogger singleton (thread-safe, level-filtered terminal logging).
 *
 * Documentation is provided in scientific UK English and designed to be strict Doxygen-friendly.
 */

/// \cond
#include <string_view>
#include <mutex>
#include <ostream>
/// \endcond

/*!
 * \namespace gglog
 * \brief Front-end logging API for structured terminal output.
 *
 * Typical use:
 * \code
 * gglog::info("MyClass", "Init") << "Hello world" << gglog::endl;
 * gglog::warn("Loader" ) << "Cache disabled" << gglog::endl;
 * gglog::debug() << "v=" << v << gglog::endl;
 * gglog::err("Driver" ) << "Failure: " << msg << gglog::endl;
 * \endcode
 */
namespace gglog {
  /*!
   * \enum Level
   * \brief Severity levels used for filtering and colour formatting.
   *
   * Levels are ordered from most general information to errors.
   * Filtering uses a "minimum visible level" threshold.
   */
  enum class Level : unsigned char {
    INFO   = 0,  /*!< General informational messages */
    INFO2,       /*!< Detailed informational messages */
    INFO3,       /*!< Highly detailed informational messages */
    INFO4,       /*!< Trace-level (function enter/leave) */
    DEBUG,       /*!< Debug output */
    WARNING,     /*!< Warnings that require attention */
    ERR          /*!< Errors: the message should be clearly visible */
  };

  /*!
   * \brief Convert a log \c Level to a human-readable, stable string.
   * \param level The log level.
   * \return The textual representation, e.g. "INFO", "DEBUG", "ERROR".
   */
  [[nodiscard]] constexpr std::string_view ToString(Level level) noexcept {
    using enum Level;
    switch (level) {
      case INFO:    return "INFO";
      case INFO2:   return "INFO2";
      case INFO3:   return "INFO3";
      case INFO4:   return "INFO4";
      case DEBUG:   return "DEBUG";
      case WARNING: return "WARNING";
      case ERR:     return "ERROR";
      default:      return "UNKNOWN";
    }
  }

  /*!
   * \brief Obtain a stream for structured logging at a given severity.
   * \param level       Severity level.
   * \param class_name  Optional class context.
   * \param method_name Optional function/method context.
   * \return A reference to an \c std::ostream acting as a per-thread staging buffer.
   *
   * The message is actually emitted only when \ref endl is inserted.
   */
  std::ostream& io(Level level, std::string_view class_name, std::string_view method_name);

  /*!
   * \brief Manipulator that flushes the staged line to the logger sink.
   * \param stream The staging output stream returned by \ref io and its wrappers.
   * \return The same stream, reset and ready for the next line.
   */
  std::ostream& endl(std::ostream& stream);

  /*!
   * \name Convenience front-ends by severity
   * \brief Thin wrappers around \ref io.
   * \{
   */
  /*!
   * \brief Stream output for informational messages (\ref gglog::Level::INFO).
   * \param cls Optional class name of the message origin.
   * \param fn  Optional method or function name of the message origin.
   * \return Reference to the internal staging \c std::ostream.
   */
  inline std::ostream& info(std::string_view cls = "", std::string_view fn = "") {
    return io(Level::INFO, cls, fn);
  }

  /*!
   * \brief Stream output for detailed informational messages (\ref gglog::Level::INFO2).
   * \param cls Optional class name of the message origin.
   * \param fn  Optional method or function name of the message origin.
   * \return Reference to the internal staging \c std::ostream.
   */
  inline std::ostream& info2(std::string_view cls = "", std::string_view fn = "") {
    return io(Level::INFO2, cls, fn);
  }

  /*!
   * \brief Stream output for very detailed informational messages (\ref gglog::Level::INFO3).
   * \param cls Optional class name of the message origin.
   * \param fn  Optional method or function name of the message origin.
   * \return Reference to the internal staging \c std::ostream.
   */
  inline std::ostream& info3(std::string_view cls = "", std::string_view fn = "") {
    return io(Level::INFO3, cls, fn);
  }

  /*!
   * \brief Stream output for trace-level messages (\ref gglog::Level::INFO4),
   * typically used for function entry/exit.
   * \param cls Optional class name of the message origin.
   * \param fn  Optional method or function name of the message origin.
   * \return Reference to the internal staging \c std::ostream.
   */
  inline std::ostream& info4(std::string_view cls = "", std::string_view fn = "") {
    return io(Level::INFO4, cls, fn);
  }

  /*!
   * \brief Stream output for warning messages (\ref gglog::Level::WARNING).
   * \param cls Optional class name of the message origin.
   * \param fn  Optional method or function name of the message origin.
   * \return Reference to the internal staging \c std::ostream.
   */
  inline std::ostream& warn(std::string_view cls = "", std::string_view fn = "") {
    return io(Level::WARNING, cls, fn);
  }

  /*!
   * \brief Stream output for debug messages (\ref gglog::Level::DEBUG).
   * \param cls Optional class name of the message origin.
   * \param fn  Optional method or function name of the message origin.
   * \return Reference to the internal staging \c std::ostream.
   */
  inline std::ostream& debug(std::string_view cls = "", std::string_view fn = "") {
    return io(Level::DEBUG, cls, fn);
  }

  /*!
   * \brief Stream output for error messages (\ref gglog::Level::ERR).
   * \param cls Optional class name of the message origin.
   * \param fn  Optional method or function name of the message origin.
   * \return Reference to the internal staging \c std::ostream.
   */
  inline std::ostream& err(std::string_view cls = "", std::string_view fn = "") {
    return io(Level::ERR, cls, fn);
  }
  /*! \} */

} // namespace gglog

/*!
 * \class GGEMSLogger
 * \brief Thread-safe singleton performing the actual terminal output.
 *
 * Features:
 * - Mutex-protected, line-oriented emission.
 * - Level filtering via a minimum visible \ref gglog::Level threshold.
 * - Optional class/method context decoration.
 * - ANSI colours on POSIX, Win32 console attributes on Windows.
 */
class GGEMSLogger {
private:
  /*!
   * \brief Constructor (singleton).
   *
   * Sets default minimum level to \c gglog::Level::INFO.
   */
  GGEMSLogger() noexcept
    : minimum_level_{gglog::Level::INFO} {}

  ~GGEMSLogger() = default;

  GGEMSLogger(GGEMSLogger const&)            = delete;
  GGEMSLogger(GGEMSLogger&&)                 = delete;
  GGEMSLogger& operator=(GGEMSLogger const&) = delete;
  GGEMSLogger& operator=(GGEMSLogger&&)      = delete;

public:
  /*!
   * \brief Access the process-wide singleton instance.
   * \return Reference to the singleton.
   */
  static GGEMSLogger& GetInstance() {
    static GGEMSLogger instance;
    return instance;
  }

  /*!
   * \brief Set the minimum visible severity level (inclusive).
   * \param minimum_level Messages "lighter" than this are filtered out.
   *
   * \note \c WARNING, \c DEBUG and \c ERR are always considered for visibility
   *       by \ref IsVisible to preserve important diagnostics.
   */
  void SetLevelInfos(gglog::Level minimum_level) noexcept;

  /*!
   * \brief Back-end called by the front-end to emit a formatted line.
   * \param level       Severity level of the line.
   * \param message     The textual payload (one complete line).
   * \param class_name  Optional class context.
   * \param method_name Optional method context.
   */
  void LogMessage(gglog::Level level, std::string_view message,
    std::string_view class_name, std::string_view method_name);

  /*!
   * \brief Decide whether a message at \p level should be printed.
   * \param level Candidate severity.
   * \return \c true if the message passes filtering, \c false otherwise.
   */
  [[nodiscard]] bool IsVisible(gglog::Level level) const noexcept;

private:
  gglog::Level minimum_level_; /*!< Minimum visible level (inclusive). */
  std::mutex   write_lock_;    /*!< Serialises terminal writes. */
};

/*!
 * \brief Convenience operator to pipe \c std::string_view into streams.
 * \param stream Output stream.
 * \param str    String view to insert.
 * \return The same output stream.
 */
std::ostream& operator<<(std::ostream& stream, std::string_view str);
