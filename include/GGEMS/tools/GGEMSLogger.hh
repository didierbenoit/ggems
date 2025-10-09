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
 * \brief Definition of GGEMSLogger class
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-10-07
 * \copyright GNU General Public License v3.0
 * \version 2.0
 */

/// \cond
#include <string>
#include <mutex>
#include <chrono>
/// \endcond

#include "GGEMS/tools/GGEMSSingletonHolder.hh"

/*!
 * \namespace gglog
 * \brief Namespace storing the GGEMS standard output replacing the C++ standard 'std::cout'
 */
namespace gglog {
  /*!
   * \enum Level
   * \brief Represent the different level of stream output
  */
  enum class Level : unsigned char {
    /*!
     * \brief The message corresponds to the first level of info. At this level, the message is always displayed on the terminal.
     */
    INFO = 0,

    /*!
     * \brief The message corresponds to the second level of info. At this level, more infos are displayed on the terminal
     */
    INFO2,

    /*!
     * \brief The message corresponds to the third level of info. Very detailed infos displays on the terminal
     */
    INFO3,

    /*!
     * \brief The message corresponds to the last level of info. Use to print enter/exit of each function/method, useful to track a bug.
     */
    INFO4,

    /*!
     * \brief The message corresponds to a debug text. Use this level for debug
     */
    DEBUG,

    /*!
     * \brief The message corresponds to a warning text
     */
    WARNING,

    /*!
     * \brief The message corresponds to an error text
     */
    ERR
  }; // enum Level

  /*!
   * \fn std::ostream& io(Level const& level, std::string const& class_name, std::string const& method_name)
   * \param level - Type of level
   * \param class_name - Class name
   * \param method_name - Method name
   * \brief Handling stream with level of log
   * \return Current ostream with message
   */
  std::ostream& io(Level const& level, std::string const& class_name, std::string const& method_name);

  /*!
   * \fn std::ostream& endl(std::ostream& stream)
   * \param stream - Stream for end of line
   * \brief Handling end of line
   * \return End of line ostream
   */
  std::ostream& endl(std::ostream& stream);

  /*!
   * \fn inline std::ostream& info(std::string const& class_name = "", std::string const& method_name = "")
   * \brief Print data on the terminal with standard info level
   * \param class_name - Class name
   * \param method_name - Method name
   * \return Info ostream message
   */
  inline std::ostream& info(std::string const& class_name = "", std::string const& method_name = "") {
    return io(Level::INFO, class_name, method_name);
  }

  /*!
   * \fn inline std::ostream& info2(std::string const& class_name = "", std::string const& method_name = "")
   * \brief Print data on the terminal with info level 2
   * \param class_name - Class name
   * \param method_name - Method name
   * \return Info level2 ostream message
   */
  inline std::ostream& info2(std::string const& class_name = "", std::string const& method_name = "") {
    return io(Level::INFO2, class_name, method_name);
  }

  /*!
   * \fn inline std::ostream& info3(std::string const& class_name = "", std::string const& method_name = "")
   * \brief Print data on the terminal with info level 3
   * \param class_name - Class name
   * \param method_name - Method name
   * \return Info level3 ostream message
   */
  inline std::ostream& info3(std::string const& class_name = "", std::string const& method_name = "") {
    return io(Level::INFO3, class_name, method_name);
  }

  /*!
   * \fn inline std::ostream& info4(std::string const& class_name = "", std::string const& method_name = "")
   * \brief Print data on the terminal with info level 4
   * \param class_name - Class name
   * \param method_name - Method name
   * \return Info level4 ostream message
   */
  inline std::ostream& info4(std::string const& class_name = "", std::string const& method_name = "") {
    return io(Level::INFO4, class_name, method_name);
  }

  /*!
   * \fn inline std::ostream& warn(std::string const& class_name = "", std::string const& method_name = "")
   * \brief Print data to terminal with message warning
   * \param class_name - Class name
   * \param method_name - Method name
   * \return Warning ostream message
   */
  inline std::ostream& warn(std::string const& class_name = "", std::string const& method_name = "") {
    return io(Level::WARNING, class_name, method_name);
  }

  /*!
   * \fn inline std::ostream& debug(std::string const& class_name = "", std::string const& method_name = "")
   * \brief Print data to terminal with message debug
   * \param class_name - Class name
   * \param method_name - Method name
   * \return Debug ostream message
   */
  inline std::ostream& debug(std::string const& class_name = "", std::string const& method_name = "") {
    return io(Level::DEBUG, class_name, method_name);
  }

  /*!
   * \fn inline std::ostream& err(std::string const& class_name = "", std::string const& method_name = "")
   * \brief print data to terminal with error level
   * \param class_name - Class name
   * \param method_name - Method name
   * \return Error ostream message
   */
  inline std::ostream& err(std::string const& class_name = "", std::string const& method_name = "") {
    return io(Level::ERR, class_name, method_name);
  }

  /*!
   * \fn std::string ToString(Level const& level)
   * \param level - Message level
   * \brief Convert the level number to string
   * \return Level converted to string
   */
  std::string ToString(Level const& level);

  /*!
   * \fn std::string ToString(std::chrono::system_clock::time_point const& time)
   * \param time - Current time
   * \brief Convert the time point to string
   * \return time chrono converted to string
   */
  std::string ToString(std::chrono::system_clock::time_point const& time);
} // namespace gglog

/*!
 * \class GGEMSLogger
 * \brief GGEMSLogger class redefined C++ standard output (on the terminal)
 *
 * \note
 * This class is used as a singleton defined by:
 * using GGEMSLoggerManager = GGEMSSingletonHolder<GGEMSLogger>
 *
 * \code
 * Examples:
 * gglog::info() << "" << gglog::endl;
 * gglog::info("CLASS_NAME", "METHOD_NAME") << "" << gglog::endl;
 * gglog::info2("CLASS_NAME", "METHOD_NAME") << "" << gglog::endl;
 * gglog::info3("CLASS_NAME", "METHOD_NAME") << "" << gglog::endl;
 * gglog::info4("CLASS_NAME", "METHOD_NAME") << "" << gglog::endl;
 * gglog::debug("CLASS_NAME", "METHOD_NAME") << "" << gglog::endl;
 * gglog::warn("CLASS_NAME", "METHOD_NAME") << "" << gglog::endl;
 * gglog::err("CLASS_NAME", "METHOD_NAME") << "" << gglog::endl;
 * \endcode
 */
class GGEMSLogger final {
public:
  /*!
   * \brief Constructor of GGEMSLogger
   * \fn GGEMSLogger(void)
   */
  GGEMSLogger(void)
  : minimum_level_(gglog::Level::INFO),
    write_lock_() {}

  /*!
   * \brief Destructor of GGEMSLogger
   * \fn ~GGEMSLogger(void)
   */
  ~GGEMSLogger(void) = default;

  /*!
   * \fn GGEMSLogger(GGEMSLogger const& logger) = delete
   * \param logger - Reference on GGEMSLogger
   * \brief Avoid copy of GGEMSLogger by reference
   */
  GGEMSLogger(GGEMSLogger const& logger) = delete;

  /*!
   * \fn GGEMSLogger(GGEMSLogger const&& logger) = delete
   * \param logger - RValue reference on GGEMSLogger
   * \brief Avoid copy of GGEMSLogger by rvalue reference
   */
  GGEMSLogger(GGEMSLogger const&& logger) = delete;

  /*!
   * \fn GGEMSLogger& operator=(GGEMSLogger const& logger) = delete
   * \param logger - Reference on GGEMSLogger
   * \brief Avoid assignement of GGEMSLogger by reference
   */
  GGEMSLogger& operator=(GGEMSLogger const& logger) = delete;

  /*!
   * \fn GGEMSLogger& operator=(GGEMSLogger const&& logger) = delete
   * \param logger - RValue reference on GGEMSLogger
   * \brief Avoid copy of GGEMSLogger by rvalue reference
   */
  GGEMSLogger& operator=(GGEMSLogger const&& logger) = delete;

public:
  /*!
   * \fn void SetLevelInfos(gglog::Level const& minimum_level)
   * \param minimum_level - Minimum level of infos for output stream
   * \brief Change the level infos
   */
  void SetLevelInfos(gglog::Level const& minimum_level);

  /*!
   * \fn void LogMessage(gglog::Level const& log_level, std::string const& message, std::chrono::system_clock::time_point const& time, std::string const& class_name, std::string const& method_name)
   * \param log_level - Log level of output
   * \param message - Message to print
   * \param time - Time during the print
   * \param class_name - Class name to print
   * \param method_name - Method name to print
   * \brief Print message to terminal
   */
  void LogMessage(gglog::Level const& log_level, std::string const& message, std::chrono::system_clock::time_point const& time, std::string const& class_name, std::string const& method_name);

  /*!
   * \fn bool IsValidLogLevel(gglog::Level const& log_level) const
   * \param log_level - Log level of message output
   * \brief Check if the message should be print
   * \return True is log level is correct, otherwize false
   */
  bool IsValidLogLevel(gglog::Level const& log_level) const;

private:
  gglog::Level minimum_level_; /*!< minimum level of log to print on the standard output */
  std::mutex   write_lock_; /*!< mutex doing ostream thread safe */
}; // class GGEMSLogger

/*!
 * \fn std::ostream& operator<<(std::ostream& stream, std::string const& str)
 * \param stream - Output stream
 * \param str - String to put in output stream
 * \brief Convenience-wrapper to allow writing std::string into std::ostream
 * \return C++ ostream storing string 'str'
 */
std::ostream& operator<<(std::ostream& stream, std::string const& str);

/*!
 * \brief Alias to GGEMSLogger singleton
 */
using GGEMSLoggerManager = GGEMSSingletonHolder<GGEMSLogger>;
