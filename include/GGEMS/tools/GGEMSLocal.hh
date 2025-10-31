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
 * \brief Definition of \c GGEMSLocal, the per-thread staging buffer for logging.
 */

/// \cond
#include <sstream>
#include <string>
/// \endcond

#include "GGEMS/tools/GGEMSLogger.hh"

/*!
 * \struct GGEMSLocal
 * \brief Per-thread staging of log lines and their context.
 *
 * Each thread owns its instance (declared thread-local in \ref gglog::local),
 * which accumulates a single line into \c osstream_. The line is emitted by
 * the back-end when \ref gglog::endl is inserted.
 */
struct GGEMSLocal {
  /*!
   * \brief Construct with default severity \c gglog::Level::INFO.
   */
  GGEMSLocal() noexcept : level_{gglog::Level::INFO} {}

  /*!
   * \brief Destructor (no-op).
   */
  ~GGEMSLocal() = default;

  GGEMSLocal(GGEMSLocal const&)            = delete;
  GGEMSLocal(GGEMSLocal&&)                 = delete;
  GGEMSLocal& operator=(GGEMSLocal const&) = delete;
  GGEMSLocal& operator=(GGEMSLocal&&)      = delete;

  /*!
   * \brief Query global visibility policy for \p level.
   * \param level Candidate severity.
   * \return \c true if visible given the current logger policy.
   */
  [[nodiscard]] bool IsVisible(gglog::Level level) const noexcept;

  /*!
   * \brief Dispatch the staged message to the \ref GGEMSLogger sink.
   *
   * The function gathers \c level_, \c class_name_, \c method_name_ and the
   * accumulated payload, then calls \ref GGEMSLogger::LogMessage.
   * The per-thread buffer itself is reset by \ref gglog::endl after this call.
   */
  void WriteMessage() const noexcept;

  std::ostringstream osstream_; /*!< Per-thread line buffer */
  std::string        class_name_; /*!< Optional class context */
  std::string        method_name_; /*!< Optional method context */
  gglog::Level       level_; /*!< Severity associated with the message */
};

namespace gglog {
  /*!
   * \brief Thread-local staging area used by the front-end.
   *
   * This instance is written by \ref gglog::io and consumed by \ref gglog::endl.
   */
  extern thread_local GGEMSLocal local;
}
