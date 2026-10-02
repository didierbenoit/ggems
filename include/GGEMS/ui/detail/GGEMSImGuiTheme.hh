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
 * \brief Maps UI color roles and scaling to the GGEMS Dear ImGui style.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>

#include <imgui.h>

#include "GGEMS/render/GGEMSColorTypes.hh"

namespace ggems::ui {

/*! \brief Identifies a semantic color role in the GGEMS interface. */
enum class GGEMSThemeRole : std::uint8_t {
  /*! \brief Background of titled windows and table headers. */
  WindowBackground,

  /*! \brief Recessed controls and dimmed surfaces. */
  RecessedBackground,

  /*! \brief Selected and active controls. */
  Selection,

  /*! \brief Inactive tab background. */
  TabBackground,

  /*! \brief Selected tab overline. */
  TabAccent,

  /*! \brief Normal readable foreground text. */
  PrimaryText,

  /*! \brief Disabled or secondary text. */
  MutedText,

  /*! \brief Interactive highlights and plot lines. */
  Accent,

  /*! \brief Emphasized plot-line highlight. */
  AccentStrong,

  /*! \brief Attention markers and histogram bars. */
  Attention,

  /*! \brief Emphasized histogram highlight. */
  AttentionStrong,

  /*! \brief Clear color of the offscreen scene image. */
  SceneBackground,

  /*! \brief Output console background. */
  OutputBackground,

  /*! \brief Divider between UI regions. */
  Separator,

  /*! \brief Window and child-region borders. */
  Border,

  /*! \brief Hovered and active scrollbar highlight. */
  ScrollbarAccent,

  /*! \brief Main application and dockspace background. */
  ApplicationBackground,

  /*! \brief Popup menu background. */
  PopupBackground,
};

/*! \brief Unscaled font size in logical UI pixels. */
inline constexpr float k_ggems_base_font_size{16.5F};

/*!
 * \brief Resolves a UI role to its GGEMS palette entry.
 *
 * \param[in] role Semantic interface color role.
 * \return Palette key; unrecognized roles use the fallback foreground color.
 */
[[nodiscard]] auto GetThemeColorKey(GGEMSThemeRole role) noexcept
  -> render::ColorKey;

/*!
 * \brief Returns an opaque linear color for a UI role.
 *
 * \param[in] role Semantic interface color role.
 * \return Linear RGBA value for Dear ImGui.
 */
[[nodiscard]] auto GetThemeColor(GGEMSThemeRole role) noexcept -> ImVec4;

/*!
 * \brief Converts a GGEMS palette color to opaque linear RGBA.
 *
 * \param[in] color Palette family, shade, and variant.
 * \return Linear RGB channels with alpha equal to one.
 */
[[nodiscard]] auto ToImGuiColor(render::ColorKey const &color) noexcept
  -> ImVec4;

/*!
 * \brief Builds the GGEMS color and metric style at the requested scale.
 *
 * \param[in] user_scale User-selected metric and font multiplier.
 * \param[in] content_scale Window content-density multiplier.
 * \return Style with sizes scaled by both factors and separate font scales.
 */
[[nodiscard]] auto BuildGGEMSStyle(float user_scale, float content_scale)
  -> ImGuiStyle;

} // namespace ggems::ui
