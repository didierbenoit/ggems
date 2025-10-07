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
 * \brief ...
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
 * \example
 * \code
 * gglog::info("", "") << "" << gglog::endl;
 * gglog::info2("", "") << "" << gglog::endl;
 * gglog::info3("", "") << "" << gglog::endl;
 * gglog::info4("", "") << "" << gglog::endl;
 * gglog::debug("", "") << "" << gglog::endl;
 * gglog::warn("", "") << "" << gglog::endl;
 * gglog::err("", "") << "" << gglog::endl;
 * \endcode
 */
namespace gglog {
  /*!
   * \enum Level
   * \brief Represent the different level of stream output
  */
  enum class Level : char {
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
    ERROR
  }; // enum Level
} // namespace gglog

/*!
 * \class GGEMSLogger
 * \brief ...
 *
 * @details
 * Cette classe démontre comment :
 * - Documenter les attributs et méthodes
 * - Fournir des descriptions détaillées
 * - Ajouter des notes, avertissements, et exemples d'utilisation
 * 
 * \note
 * This class is thread-safe (since c++11)
 *
 * @example
 * @code
 * @endcode
 */
class GGEMSLogger final {
public:
  /*!
   * \brief Constructor of GGEMSLogger
   * \fn GGEMSLogger(void)
   */
  GGEMSLogger(void)
  : minimum_log_level_(gglog::Level::INFO),
    write_lock_() {}

  /*!
   * \brief Destructor of GGEMSLogger
   * \fn ~GGEMSLogger(void)
   */
  ~GGEMSLogger(void) = default;

  /*!
   * \fn GGEMSLogger(GGEMSLogger const& ggems_logger) = delete
   * \param ggems_logger - reference on the singleton
   * \brief Avoid copy of the singleton by reference
   */
  GGEMSLogger(GGEMSLogger const& ggems_logger) = delete;

  /*!
   * \fn GGEMSLogger(GGEMSLogger const&& ggems_logger) = delete
   * \param ggems_logger - rvalue reference on the singleton
   * \brief Avoid copy of the singleton by rvalue reference
   */
  GGEMSLogger(GGEMSLogger const&& ggems_logger) = delete;

  /*!
   * \fn GGEMSLogger& operator=(GGEMSLogger const& ggems_logger) = delete
   * \param ggems_logger - reference on the singleton
   * \brief Avoid assignement of the singleton by reference
   */
  GGEMSLogger& operator=(GGEMSLogger const& ggems_logger) = delete;

  /*!
   * \fn GGEMSLogger& operator=(GGEMSLogger const&& ggems_logger) = delete
   * \param ggems_logger - rvalue reference on the singleton
   * \brief Avoid copy of the singleton by rvalue reference
   */
  GGEMSLogger& operator=(GGEMSLogger const&& ggems_logger) = delete;

public:
  /*!
   * \fn void SetLevelInfos(gglog::Level const& minimum_log_level)
   * \param minimum_log_level - minimum level of infos for output stream
   * \brief Change the level infos: log::LogLevel::INFO to log::LogLevel::INFO5
   */
  void SetLevelInfos(gglog::Level const& minimum_log_level);

  /*!
   * \fn void LogMessage(gglog::Level const& log_level, std::string const& message, std::chrono::system_clock::time_point const& time)
   * \param log_level - log level of output
   * \param message - message to print
   * \param time - time during the print
   * \brief Print message to terminal
   */
  void LogMessage(gglog::Level const& log_level, std::string const& message, std::chrono::system_clock::time_point const& time);

  /*!
   * \fn bool IsValidLogLevel(gglog::Level const& log_level) const
   * \param log_level - log level of message output
   * \brief check if the message should be print
   */
  bool IsValidLogLevel(gglog::Level const& log_level) const;

private:
  gglog::Level minimum_log_level_; /*!< minimum level of log to print on the standard output */
  std::mutex   write_lock_; /*!< mutex doing ostream thread safe */
}; // class GGEMSLogger

using GGEMSLoggerManager = GGEMSSingletonHolder<GGEMSLogger>;
