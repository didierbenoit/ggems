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
 * \file GGEMSLocal.hh
 * \brief Definition of GGEMSLocal structure for thread-local logging
 *
 * GGEMSLocal provides a thread-local storage container to accumulate
 * log messages using an `ostringstream` and dispatch them to the
 * GGEMS logging system. It also tracks the class, method, and log level
 * for structured log output.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-10-09
 * \copyright GNU General Public License v3.0
 * \version 2.0
 */

/// \cond
#include <sstream>
/// \endcond

#include "GGEMS/tools/GGEMSLogger.hh"


/*!
 * \struct GGEMSLocal
 * \brief Thread-local structure storing log information
 *
 * GGEMSLocal is designed to be used as `thread_local`, allowing
 * each thread to have its own logging buffer. It collects
 * class/method context, log level, and formatted message content.
 */
struct GGEMSLocal {
  /*!
   * \brief Default constructor
   *
   * Initializes the log level to `gglog::Level::INFO` and prepares
   * the `ostringstream` buffer.
   */
  GGEMSLocal(void) : level_{gglog::Level::INFO} {}

  /*!
   * \brief Destructor
   *
   * Defaulted; no special cleanup is required.
   */
   ~GGEMSLocal(void) = default;

  // Delete copy and move operations to enforce unique thread-local instance
  GGEMSLocal(GGEMSLocal const& local) = delete;
  GGEMSLocal(GGEMSLocal const&& local) = delete;
  GGEMSLocal& operator=(GGEMSLocal const& local) = delete;
  GGEMSLocal& operator=(GGEMSLocal const&& local) = delete;

  /*!
   * \brief Check if the current log level is active
   * \param level - Log level to test
   * \return True if the log message should be emitted, false otherwise
   *
   * This allows selective filtering of log messages based on
   * runtime or compile-time log level configuration.
   */
  bool IsValidLogLevel(gglog::Level const& level) const;

  /*!
   * \brief Dispatch accumulated message to the logger manager
   *
   * Sends the content of `osstream_` along with class/method
   * information and log level to the GGEMS logging system.
   */
  void WriteMessage(void) const;

  std::ostringstream osstream_; /*!< Buffer storing formatted log message content */
  std::string        class_name_; /*!< Name of the class emitting the log */
  std::string        method_name_; /*!< Name of the method emitting the log */
  gglog::Level       level_; /*!< Severity level of the log message */
};

namespace gglog {
  /*!
   * \brief Thread-local GGEMSLocal instance
   *
   * Each thread maintains its own `GGEMSLocal` to avoid
   * race conditions when logging concurrently.
   */
  extern thread_local GGEMSLocal local;
}
