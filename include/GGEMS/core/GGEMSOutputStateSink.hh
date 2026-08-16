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
 * \brief Declares the logger sink that feeds the GGEMS GUI output state.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <utility>

/// \endcond
#include "GGEMS/core/GGEMSOutputState.hh"
#include "GGEMS/core/GGEMSLogger.hh"

namespace ggems::core {
/*!
 * \brief Forwards rendered logger output into a GGEMSOutputState.
 *
 * The sink stores a non-owning reference; the referenced output state must
 * outlive the sink.
 */
class GGEMSOutputStateSink final : public LogSink {
public:
  /*!
   * \brief Constructs a sink forwarding to an existing output state.
   *
   * \param[in,out] state Output state receiving rendered log lines.
   */
  explicit GGEMSOutputStateSink(GGEMSOutputState &state) noexcept
      : state_(state) {}

  /*!
   * \brief Transfers one rendered line into the output state.
   *
   * \param[in] log_line Rendered line to retain.
   */
  auto Write(RenderedLogLine &&log_line) -> void override {
    state_.PushLogLine(std::move(log_line));
  }

private:
  /*! \brief Non-owning reference to the destination output state. */
  GGEMSOutputState &state_;
};
} // namespace ggems::core
