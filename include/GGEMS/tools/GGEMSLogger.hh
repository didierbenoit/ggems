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
 * \brief Declaration of the GGEMSLogger singleton and the gglog namespace API.
 *
 * This header defines:
 * - The `gglog` namespace: logging levels, stream helpers (`io`, `endl`), and
 *   convenience functions for levelled output (`info`, `warn`, `debug`, `err`).
 * - The `GGEMSLogger` singleton: thread-safe, level-filtered terminal output with
 *   optional coloured severity banners. The implementation preserves Windows and
 *   POSIX colouring behaviour and serialises output atomically per message.
 * - The RAII helper `GGEMSScopedLog` for automatic enter/exit tracing at `INFO4`.
 *
 * \author  Julien BERT <julien.bert@univ-brest.fr>
 * \author  Didier BENOIT <didier.benoit@inserm.fr>
 *
 * \date 2025-10-07
 * \version 2.0
 * \copyright GNU General Public License v3.0
 */

/// \cond
#include <string_view>
#include <mutex>
#include <ostream>
/// \endcond

/*!
 * \namespace gglog
 * \brief Structured logging API for GGEMS.
 *
 * The namespace exposes a stream-based logging interface:
 * \code
 * gglog::info("MyClass", "MyMethod") << "Hello" << gglog::endl;
 * gglog::warn() << "Caution" << gglog::endl;
 * gglog::err("Core") << "Failure" << gglog::endl;
 * \endcode
 */
namespace gglog {

  /*!
   * \enum Level
   * \brief Severity levels for logging, ordered informationally.
   */
  enum class Level : unsigned char {
    INFO   = 0,  /*!< General informational messages, always displayed */
    INFO2,       /*!< More detailed information */
    INFO3,       /*!< Very detailed information for debugging */
    INFO4,       /*!< Enter/exit tracing of functions/methods */
    DEBUG,       /*!< Debug messages */
    WARNING,     /*!< Warning messages */
    ERR          /*!< Error messages */
  };

  /*!
   * \brief Acquire a stream for structured logging at a given level.
   * \param level Severity level.
   * \param class_name Optional class context.
   * \param method_name Optional method context.
   * \return Reference to an `std::ostream` that accumulates the message.
   *
   * \details The message is buffered per-thread (thread-local) and is emitted
   *          atomically upon `gglog::endl`.
   */
  std::ostream& io(Level const& level, std::string_view class_name, std::string_view method_name);

  /*!
   * \brief Flush the buffered log line, emit a newline, and reset the buffer.
   * \param stream The stream returned by `gglog::io`.
   * \return The same stream (reset and ready for subsequent messages).
   *
   * \details This function triggers the actual message dispatch to the logger.
   */
  std::ostream& endl(std::ostream& stream);

  /*!
   * \brief Stream output for standard informational messages (`INFO` level)
   * \param cls Optional class name emitting the log
   * \param fn  Optional method name emitting the log
   * \return Reference to a thread-local `std::ostream` used to compose the message
   *
   * This is the most general informational level. It is always printed and
   * intended for user-facing messages describing program progress or state.
   * Example:
   * \code
   * gglog::info("GGEMSOpenCL", "Init") << "Initialising OpenCL" << gglog::endl;
   * \endcode
   */
  inline std::ostream& info(std::string_view cls = "", std::string_view fn = "") {
    return io(Level::INFO, cls, fn);
  }

  /*!
   * \brief Stream output for detailed informational messages (`INFO2` level)
   * \param cls Optional class name emitting the log
   * \param fn  Optional method name emitting the log
   * \return Reference to a thread-local `std::ostream` used to compose the message
   *
   * The `INFO2` level provides additional contextual information useful for
   * diagnostic output during simulation or setup, without overwhelming verbosity.
   */
  inline std::ostream& info2(std::string_view cls = "", std::string_view fn = "") {
    return io(Level::INFO2, cls, fn);
  }

  /*!
   * \brief Stream output for highly detailed messages (`INFO3` level)
   * \param cls Optional class name emitting the log
   * \param fn  Optional method name emitting the log
   * \return Reference to a thread-local `std::ostream` used to compose the message
   *
   * This level is typically used to log intermediate computational data,
   * parameters, or configuration states during internal debugging.
   */
  inline std::ostream& info3(std::string_view cls = "", std::string_view fn = "") {
    return io(Level::INFO3, cls, fn);
  }

  /*!
   * \brief Stream output for enter/exit tracing messages (`INFO4` level)
   * \param cls Optional class name emitting the log
   * \param fn  Optional method name emitting the log
   * \return Reference to a thread-local `std::ostream` used to compose the message
   *
   * The `INFO4` level is designed to trace function and method calls,
   * typically used in conjunction with `GGEMSScopedLog` for automatic
   * entry/exit diagnostics in complex execution flows.
   */
  inline std::ostream& info4(std::string_view cls = "", std::string_view fn = "") {
    return io(Level::INFO4, cls, fn);
  }

  /*!
   * \brief Stream output for warning messages (`WARNING` level)
   * \param cls Optional class name emitting the log
   * \param fn  Optional method name emitting the log
   * \return Reference to a thread-local `std::ostream` used to compose the message
   *
   * This level indicates potentially abnormal or risky behaviour that does not
   * interrupt the simulation but may require user attention or parameter review.
   */
  inline std::ostream& warn(std::string_view cls = "", std::string_view fn = "") {
    return io(Level::WARNING, cls, fn);
  }

  /*!
   * \brief Stream output for debug messages (`DEBUG` level)
   * \param cls Optional class name emitting the log
   * \param fn  Optional method name emitting the log
   * \return Reference to a thread-local `std::ostream` used to compose the message
   *
   * Debug messages are intended for developers. They provide detailed internal
   * state inspection and are typically filtered out in release builds.
   */
  inline std::ostream& debug(std::string_view cls = "", std::string_view fn = "") {
    return io(Level::DEBUG, cls, fn);
  }

  /*!
   * \brief Stream output for error messages (`ERR` level)
   * \param cls Optional class name emitting the log
   * \param fn  Optional method name emitting the log
   * \return Reference to a thread-local `std::ostream` used to compose the message
   *
   * The `ERR` level denotes a recoverable or fatal failure during execution.
   * Messages are always printed and formatted in red for emphasis in terminal mode.
   */
  inline std::ostream& err(std::string_view cls = "", std::string_view fn = "") {
    return io(Level::ERR, cls, fn);
  }

  /*!
   * \brief Compile-time conversion of a logging level to its textual form.
   * \param level Logging severity.
   * \return A constant string view (e.g., "INFO", "DEBUG", "ERROR").
   */
  [[nodiscard]] constexpr std::string_view ToString(Level level) {
    using enum Level;
    switch(level) {
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
}

/*!
 * \class GGEMSLogger
 * \brief Thread-safe singleton for terminal logging with level filtering and colouring.
 *
 * The logger serialises terminal writes per message using a mutex to prevent
 * interleaving of coloured severity banners on Windows and POSIX terminals.
 * Messages are composed in thread-local buffers and flushed atomically.
 *
 * \note Access via `GGEMSLogger::GetInstance()`.
 */
class GGEMSLogger final {
private:
  /*!
   * \brief Constructs the logger with default settings.
   *
   * - Minimum level set to `gglog::Level::INFO`.
   */
  GGEMSLogger(void) noexcept : minimum_level_{gglog::Level::INFO} {}

  /*!
   * \brief Defaulted destructor, no dynamic resources.
   */
  ~GGEMSLogger(void) = default;

  GGEMSLogger(GGEMSLogger const&) = delete;
  GGEMSLogger(GGEMSLogger&&) = delete;
  GGEMSLogger& operator=(GGEMSLogger const&) = delete;
  GGEMSLogger& operator=(GGEMSLogger&&) = delete;

public:
  /*!
   * \brief Returns the singleton instance.
   * \return Reference to the global logger.
   */
  static GGEMSLogger& GetInstance(void) {
    static GGEMSLogger instance;
    return instance;
  }

  /*!
   * \brief Configures the minimum informational level.
   * \param minimum_level Messages below this informational level are ignored,
   * except \c DEBUG, \c WARNING and \c ERR which are always emitted.
   */
  void SetLevelInfos(gglog::Level const& minimum_level);

  /*!
   * \brief Emits a fully formatted message to the terminal.
   * \param log_level Severity level.
   * \param message   Content to print (single line, newline already handled).
   * \param class_name Optional class context.
   * \param method_name Optional method context.
   *
   * \details This method is thread-safe and serialises Windows/POSIX colour
   *          changes to avoid interleaving. The textual payload is written
   *          atomically per call.
   */
  void LogMessage(gglog::Level const& log_level, std::string_view message,
                  std::string_view class_name, std::string_view method_name);

  /*!
   * \brief Queries whether a message should be emitted according to the filter.
   * \param log_level Severity of the message.
   * \return True if the message passes filtering, false otherwise.
   */
  [[nodiscard]] bool IsValidLogLevel(gglog::Level const& log_level) const;

private:
  gglog::Level minimum_level_; /*!< Informational threshold */
  std::mutex   write_lock_;    /*!< Serialises coloured output */
};

/*!
 * \brief Allows streaming `std::string_view` into an `std::ostream`.
 * \param stream Target stream.
 * \param str    View to write.
 * \return The target stream.
 */
std::ostream& operator<<(std::ostream& stream, std::string_view str);

/*!
 * \struct GGEMSScopedLog
 * \brief RAII utility for `INFO4` enter/exit tracing.
 *
 * Construct an instance at the beginning of a scope to emit an `INFO4` "Enter"
 * message; upon destruction, an "Exit" message is emitted. This is useful for
 * deterministic tracing of function lifetimes during performance investigations.
 *
 * \code
 * void f() {
 *   GGEMSScopedLog trace("MyClass", "f");
 *   // work...
 * }
 * \endcode
 */
struct GGEMSScopedLog final {
  /*!
   * \brief Constructs the scope tracer and emits an "Enter" message.
   * \param cls Class context.
   * \param fn  Function/method context.
   */
  GGEMSScopedLog(std::string_view cls, std::string_view fn)
    : cls_{cls}, fn_{fn} {
    gglog::info4(cls_, fn_) << "Enter" << gglog::endl;
  }

  /*!
   * \brief Destructs the scope tracer and emits an "Exit" message.
   */
  ~GGEMSScopedLog() { gglog::info4(cls_, fn_) << "Exit" << gglog::endl; }

private:
  std::string cls_; /*!< Class context */
  std::string fn_; /*!< Function/method context */
};
