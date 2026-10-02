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
 * \brief Displays the scene texture and collects camera gestures.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>

#include <imgui.h>

namespace ggems::ui::detail {

/*! \brief Owns mouse gesture state for the scene viewport. */
class GGEMSImGuiViewportPanel {
public:
  /*! \brief Reports the requested extent and camera input for one frame. */
  struct Result {
    /*! \brief Whether the viewport has visible contents. */
    bool visible{false};

    /*! \brief Requested width in logical UI pixels. */
    std::uint32_t width{0U};

    /*! \brief Requested height in logical UI pixels. */
    std::uint32_t height{0U};

    /*! \brief Horizontal orbit drag in logical UI pixels. */
    float orbit_delta_x_pixels{0.0F};

    /*! \brief Vertical orbit drag in logical UI pixels. */
    float orbit_delta_y_pixels{0.0F};

    /*! \brief Horizontal mouse and keyboard pan in logical pixels. */
    float pan_delta_x_pixels{0.0F};

    /*! \brief Vertical mouse and keyboard pan in logical pixels. */
    float pan_delta_y_pixels{0.0F};

    /*! \brief Vertical mouse-wheel delta for multiplicative zoom. */
    float zoom_delta{0.0F};
  };

  /*!
   * \brief Builds the viewport and collects its routed input.
   *
   * \param[in,out] show_window Panel visibility, updated when the window
   *   closes.
   * \param[in] scene_texture_id Borrowed ImGui texture ID, or zero while
   *   unavailable.
   * \param[in] texture_width Logical width represented by the current texture.
   * \param[in] texture_height Logical height represented by the current
   *   texture.
   * \return Visible extent and camera deltas; gestures stop while the texture
   *   is stale.
   */
  auto Build(bool &show_window, ImTextureID scene_texture_id,
             std::uint32_t texture_width, std::uint32_t texture_height)
    -> Result;

  /*! \brief Releases the current mouse gesture. */
  auto CancelGesture() noexcept -> void;

private:
  /*! \brief Identifies the currently owned mouse drag. */
  enum class Gesture : std::uint8_t { None, Orbit, Pan };

  /*!
   * \var ggems::ui::detail::GGEMSImGuiViewportPanel::Gesture::None
   * \brief No drag is active.
   */

  /*!
   * \var ggems::ui::detail::GGEMSImGuiViewportPanel::Gesture::Orbit
   * \brief Left-button orbit drag.
   */

  /*!
   * \var ggems::ui::detail::GGEMSImGuiViewportPanel::Gesture::Pan
   * \brief Middle-button pan drag.
   */

  /*! \brief Drag retained while the scene image item remains active. */
  Gesture gesture_{Gesture::None};
};

} // namespace ggems::ui::detail
