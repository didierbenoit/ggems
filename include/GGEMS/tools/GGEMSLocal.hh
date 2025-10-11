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
 * \brief Definition of GGEMSLocal class
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
 * \brief Structure storing infos in ostringstream to print on the terminal
 */
struct GGEMSLocal final {
  /*!
   * \brief Constructor of GGEMSLocal
   * \fn GGEMSLocal(void)
   */
  GGEMSLocal(void) : level_(gglog::Level::INFO) {}

  /*!
   * \brief Destructor of GGEMSLocal
   * \fn ~GGEMSLocal(void)
   */
   ~GGEMSLocal(void) = default;

  /*!
   * \fn GGEMSLocal(GGEMSLocal const& local) = delete
   * \param local - Reference on GGEMSLocal
   * \brief Avoid copy of GGEMSLocal by reference
   */
  GGEMSLocal(GGEMSLocal const& local) = delete;

  /*!
   * \fn GGEMSLocal(GGEMSLocal const&& local) = delete
   * \param local - RValue reference on GGEMSLocal
   * \brief Avoid copy of GGEMSLocal by rvalue reference
   */
  GGEMSLocal(GGEMSLocal const&& local) = delete;

  /*!
   * \fn GGEMSLocal& operator=(GGEMSLocal const& local) = delete
   * \param local - Reference on GGEMSLocal
   * \brief Avoid assignement of GGEMSLocal by reference
   */
  GGEMSLocal& operator=(GGEMSLocal const& local) = delete;

  /*!
   * \fn GGEMSLocal& operator=(GGEMSLocal const&& local) = delete
   * \param local - RValue reference on GGEMSLocal
   * \brief Avoid copy of GGEMSLocal by rvalue reference
   */
  GGEMSLocal& operator=(GGEMSLocal const&& local) = delete;

  /*!
   * \fn bool IsValidLogLevel(gglog::Level const& level) const
   * \param level - Level of message output
   * \brief Check if the message should be print
   * \return True if it a valid log
   */
  bool IsValidLogLevel(gglog::Level const& level) const;

  /*!
   * \fn void WriteMessage(void) const
   * \brief Give message to the logger manager
   */
  void WriteMessage(void) const;

  std::ostringstream                    osstream_; /*!< Output string stream storing infos to print on the terminal */
  std::string                           class_name_; /*!< Store the class name */
  std::string                           method_name_; /*!< Store the method name */
  gglog::Level                          level_; /*!< level of log */
}; // struct GGEMSLocal

namespace gglog {
  extern thread_local GGEMSLocal local; /*!< Seemingly global or static storage duration but one copy per thread */
} // namespace gglog
