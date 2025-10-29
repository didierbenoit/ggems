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
 * \brief Thread-local accumulator for GGEMS logging.
 *
 * GGEMSLocal buffers a single log line per thread (via `std::ostringstream`),
 * storing severity and optional class/method context, which is finally flushed
 * atomically by the global logger when `gglog::endl` is invoked.
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
 * \brief Per-thread logging buffer and context.
 *
 * Each thread owns one GGEMSLocal instance, avoiding contention during message
 * composition. The global logger only serialises the final emission step.
 */
struct GGEMSLocal final {
  /*!
   * \brief Constructs a local buffer with default informational level.
   */
  GGEMSLocal() : level_{gglog::Level::INFO} {}

  /*!
   * \brief Default destructor, no dynamic resources.
   */
   ~GGEMSLocal() = default;

  GGEMSLocal(GGEMSLocal const& local) = delete;
  GGEMSLocal(GGEMSLocal const&& local) = delete;
  GGEMSLocal& operator=(GGEMSLocal const& local) = delete;
  GGEMSLocal& operator=(GGEMSLocal const&& local) = delete;

  /*!
   * \brief Tests whether a level passes the global filter.
   * \param level Candidate severity.
   * \return True if the message should be printed, false otherwise.
   */
  [[nodiscard]] bool IsValidLogLevel(gglog::Level const& level) const;

  /*!
   * \brief Flushes the buffered message to the global logger.
   *
   * \details Emits the line atomically; afterwards the buffer is meant to be
   *          cleared by `gglog::endl`.
   */
  void WriteMessage() const noexcept;

  std::ostringstream osstream_; /*!< Per-thread line buffer */
  std::string        class_name_; /*!< Optional class context */
  std::string        method_name_; /*!< Optional method context */
  gglog::Level       level_; /*!< Severity associated with the message */
};

namespace gglog {
  /*!
   * \brief Thread-local instance used by `gglog::io`/`gglog::endl`
   */
  extern thread_local GGEMSLocal local;
}
