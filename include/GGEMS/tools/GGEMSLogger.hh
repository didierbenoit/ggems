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
 * \brief Definition of GGEMSLogger singleton and gglog namespace for structured terminal output
 *
 * This file provides:
 * - The `gglog` namespace, containing logging levels, helper stream functions, and level-based output methods.
 * - The `GGEMSLogger` singleton class, handling thread-safe, level-filtered output to the terminal.
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
 * \brief Provides structured logging functionality for GGEMS
 *
 * This namespace defines logging levels, inline helper functions for streaming messages,
 * and conversion utilities. Logging messages can include optional class and method context.
 *
 * Usage example:
 * \code
 * gglog::info("MyClass", "MyMethod") << "Hello world" << gglog::endl;
 * gglog::warn() << "This is a warning" << gglog::endl;
 * gglog::debug("MyClass") << "Debugging data: " << some_value << gglog::endl;
 * gglog::err() << "Error occurred!" << gglog::endl;
 * \endcode
 */
namespace gglog {

  /*!
   * \enum Level
   * \brief Severity levels for logging
   *
   * Levels are used to filter and categorise messages in the terminal.
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
   * \brief Retrieve an ostream for structured logging
   * \param level - Severity level
   * \param class_name - Class emitting the log
   * \param method_name - Method emitting the log
   * \return Reference to an ostream for inserting log content
   *
   * Example usage:
   * \code
   * gglog::io(Level::INFO, "MyClass", "MyMethod") << "Message content" << gglog::endl;
   * \endcode
   */
  std::ostream& io(Level const& level, std::string_view class_name, std::string_view method_name);

  /*!
   * \brief Flush the log message and insert a newline
   * \param stream - Output stream
   * \return Reference to the stream
   */
  std::ostream& endl(std::ostream& stream);

  /*!
   * \brief Stream output for standard informational messages (INFO)
   * \param class_name Optional class name emitting the log
   * \param method_name Optional method name emitting the log
   * \return Reference to ostream for streaming message
   */
  inline std::ostream& info(std::string_view class_name = "", std::string_view method_name = "") {
    return io(Level::INFO, class_name, method_name);
  }

  /*!
   * \brief Stream output for INFO2 messages (detailed informational messages)
   * \param class_name Optional class name emitting the log
   * \param method_name Optional method name emitting the log
   * \return Reference to ostream for streaming the INFO2 message
   *
   */
  inline std::ostream& info2(std::string_view class_name = "", std::string_view method_name = "") {
    return io(Level::INFO2, class_name, method_name);
  }

  /*!
   * \brief Stream output for INFO3 messages (very detailed informational messages)
   * \param class_name Optional class name emitting the log
   * \param method_name Optional method name emitting the log
   * \return Reference to ostream for streaming the INFO3 message
   */
  inline std::ostream& info3(std::string_view class_name = "", std::string_view method_name = "") {
    return io(Level::INFO3, class_name, method_name);
  }

  /*!
   * \brief Stream output for INFO4 messages (function/method enter-exit tracing)
   * \param class_name Optional class name emitting the log
   * \param method_name Optional method name emitting the log
   * \return Reference to ostream for streaming the INFO4 message
   */
  inline std::ostream& info4(std::string_view class_name = "", std::string_view method_name = "") {
    return io(Level::INFO4, class_name, method_name);
  }

  /*!
   * \brief Stream output for WARNING messages
   * \param class_name Optional class name emitting the log
   * \param method_name Optional method name emitting the log
   * \return Reference to ostream for streaming the WARNING message
   */
  inline std::ostream& warn(std::string_view class_name = "", std::string_view method_name = "") {
    return io(Level::WARNING, class_name, method_name);
  }

  /*!
   * \brief Stream output for debugging messages
   * \param class_name Optional class name emitting the log
   * \param method_name Optional method name emitting the log
   * \return Reference to ostream for streaming the debug message
   */
  inline std::ostream& debug(std::string_view class_name = "", std::string_view method_name = "") {
    return io(Level::DEBUG, class_name, method_name);
  }

  /*!
   * \brief Stream output for error messages
   * \param class_name Optional class name emitting the log
   * \param method_name Optional method name emitting the log
   * \return Reference to ostream for streaming the error message
   */
  inline std::ostream& err(std::string_view class_name = "", std::string_view method_name = "") {
    return io(Level::ERR, class_name, method_name);
  }

  /*!
   * \brief Convert a logging level to a human-readable string
   * \param level - Log level
   * \return String representation of the level (e.g., "INFO", "DEBUG", "ERR")
   */
  std::string ToString(Level const& level);

} // namespace gglog

/*!
 * \class GGEMSLogger
 * \brief Singleton class for thread-safe logging with level filtering
 *
 * GGEMSLogger centralises terminal output for GGEMS, ensuring:
 * - Thread safety via internal mutex
 * - Filtering messages below the configured minimum level
 * - Structured output including class and method context
 *
 * \note Singleton pattern is used: access via `GetInstance()`.
 *
 * \code
 * // Example usage
 * gglog::info("MyClass", "MyMethod") << "Hello world" << gglog::endl;
 * GGEMSLogger::GetInstance().SetLevelInfos(gglog::Level::DEBUG);
 * \endcode
 */
class GGEMSLogger {
private:
 /*!
  * \brief Constructor of GGEMSLogger
  *
  * Initializes the GGEMSLogger singleton instance with default settings:
  * - Sets the minimum log level to `gglog::Level::INFO`.
  * - Initializes the internal mutex for thread-safe logging.
  *
  * \note
  * This constructor is private to enforce the singleton pattern.
  * Access to the logger must be done through `GGEMSLogger::GetInstance()`.
  */
  GGEMSLogger(void)
    : minimum_level_{gglog::Level::INFO}, write_lock_{} {}

  /*!
   * \brief Destructor of GGEMSLogger
   *
   * Cleans up the GGEMSLogger instance.
   * As the singleton is static, the destructor is called automatically
   * at program termination and no explicit deletion is needed.
   *
   * \note
   * The destructor is defaulted since there are no dynamically allocated resources.
   */
  ~GGEMSLogger(void) = default;

  // Deleted copy/move operations
  GGEMSLogger(GGEMSLogger const&) = delete;
  GGEMSLogger(GGEMSLogger&&) = delete;
  GGEMSLogger& operator=(GGEMSLogger const&) = delete;
  GGEMSLogger& operator=(GGEMSLogger&&) = delete;

public:
  /*!
   * \brief Access the singleton instance
   * \return Reference to the GGEMSLogger singleton
   */
  static GGEMSLogger& GetInstance(void) {
    static GGEMSLogger instance;
    return instance;
  }

  /*!
   * \brief Set the minimum severity level for logging output
   * \param minimum_level - Messages below this level will be ignored
   */
  void SetLevelInfos(gglog::Level const& minimum_level);

  /*!
   * \brief Log a message to the terminal
   * \param log_level - Severity level
   * \param message - Text message
   * \param class_name - Class context
   * \param method_name - Method context
   */
  void LogMessage(gglog::Level const& log_level, std::string_view message,
                  std::string_view class_name, std::string_view method_name);

  /*!
   * \brief Determine whether a message at the given level should be printed
   * \param log_level - Message severity
   * \return True if message passes filtering, false otherwise
   */
  bool IsValidLogLevel(gglog::Level const& log_level) const;

private:
  gglog::Level minimum_level_; /*!< Minimum log level for output */
  std::mutex   write_lock_;    /*!< Mutex protecting terminal output for thread safety */
};

/*!
 * \brief Convenience operator to allow writing std::string_view to ostream
 * \param stream - Output stream
 * \param str - String to insert
 * \return Reference to the output stream
 */
std::ostream& operator<<(std::ostream& stream, std::string_view str);

