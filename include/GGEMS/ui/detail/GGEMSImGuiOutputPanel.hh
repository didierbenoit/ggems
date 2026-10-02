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
 * \brief Displays the GGEMS output state with local filtering controls.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstddef>
#include <array>

#include "GGEMS/logging/GGEMSLogger.hh"
#include "GGEMS/logging/GGEMSOutputState.hh"

namespace ggems::render {
struct WrappedLine;
} // namespace ggems::render

namespace ggems::ui {
/*! \brief Displays log snapshots with severity and depth filters. */
class GGEMSImGuiOutputPanel {
public:
  /*! \brief Creates an output panel with default display settings. */
  GGEMSImGuiOutputPanel() = default;

  /*! \brief Destroys the output-panel display state. */
  ~GGEMSImGuiOutputPanel() = default;

  /*! \brief Disallows copying the owned UI state. */
  GGEMSImGuiOutputPanel(GGEMSImGuiOutputPanel const &) = delete;

  /*! \brief Disallows moving the owned UI state. */
  GGEMSImGuiOutputPanel(GGEMSImGuiOutputPanel &&) = delete;

  /*! \brief Disallows copy assignment of the owned UI state. */
  auto operator=(GGEMSImGuiOutputPanel const &)
    -> GGEMSImGuiOutputPanel & = delete;

  /*! \brief Disallows move assignment of the owned UI state. */
  auto operator=(GGEMSImGuiOutputPanel &&) -> GGEMSImGuiOutputPanel & = delete;

  /*!
   * \brief Builds the output console and its filtering controls.
   *
   * \param[in,out] output_state Log state read for display and cleared on user
   *   request.
   */
  auto Render(core::GGEMSOutputState &output_state) -> void;

private:
  /*!
   * \brief Displays the colored segments of one banner line.
   *
   * \param[in] line Borrowed line with UTF-32 text and palette colors.
   */
  static auto RenderWrappedLine(render::WrappedLine const &line) -> void;

  /*!
   * \brief Applies the local severity and INFO-depth filters.
   *
   * \param[in] line Log record to classify.
   * \return True when the record passes the current filters.
   */
  [[nodiscard]] auto
  ShouldDisplay(core::RenderedLogLine const &line) const noexcept -> bool;

  /*! \brief Maximum number of recent log records fetched per frame. */
  std::size_t max_visible_lines_{2000U};

  /*! \brief Whether to follow new output when already at the bottom. */
  bool auto_scroll_{true};

  /*! \brief Whether to display log prefixes. */
  bool show_prefix_{true};

  /*! \brief Whether to display the GGEMS banner above the log. */
  bool show_banner_{true};

  /*! \brief Visibility for INFO depths zero through three and four-plus. */
  std::array<bool, 5> show_info_depth_{{true, true, true, true, true}};

  /*! \brief Whether debug records pass the display filter. */
  bool show_debug_{true};

  /*! \brief Whether warning records pass the display filter. */
  bool show_warn_{true};

  /*! \brief Whether error records pass the display filter. */
  bool show_error_{true};
};
} // namespace ggems::ui
