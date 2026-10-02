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
 * \brief Composes the dockspace and diagnostic panels for each UI frame.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>

#include <imgui.h>

#include "GGEMSDeviceStatus.hh"
#include "GGEMSImGuiOutputPanel.hh"
#include "GGEMSImGuiViewportPanel.hh"
#include "GGEMSWorkbenchState.hh"
#include "GGEMS/logging/GGEMSOutputState.hh"
#include "GGEMS/sources/GGEMSSourceRunSnapshot.hh"

namespace ggems::ui {
/*! \brief Coordinates panels and applies their presentation controls. */
class GGEMSImGuiLayer {
public:
  /*! \brief Borrows the image and diagnostic data needed by one frame. */
  struct FrameInputs {
    /*! \brief Borrowed scene descriptor, or zero while unavailable. */
    ImTextureID scene_texture_id{};

    /*! \brief Logical width represented by the scene texture. */
    std::uint32_t scene_texture_logical_width{0U};

    /*! \brief Logical height represented by the scene texture. */
    std::uint32_t scene_texture_logical_height{0U};

    /*! \brief Swapchain width in physical framebuffer pixels. */
    std::uint32_t swapchain_width{0U};

    /*! \brief Swapchain height in physical framebuffer pixels. */
    std::uint32_t swapchain_height{0U};

    /*! \brief Borrowed completed source snapshot, or null when absent. */
    core::sources::GGEMSSourceRunSnapshot const *source_run_snapshot{nullptr};

    /*! \brief Borrowed log state, required when the output panel is shown. */
    core::GGEMSOutputState *output_state{nullptr};
  };

  /*! \brief Creates panel state with docking initialization pending. */
  GGEMSImGuiLayer() = default;

  /*! \brief Destroys the panel interaction state. */
  ~GGEMSImGuiLayer() = default;

  /*! \brief Disallows copying the owned UI state. */
  GGEMSImGuiLayer(GGEMSImGuiLayer const &) = delete;

  /*! \brief Disallows moving the owned UI state. */
  GGEMSImGuiLayer(GGEMSImGuiLayer &&) = delete;

  /*! \brief Disallows copy assignment of the owned UI state. */
  auto operator=(GGEMSImGuiLayer const &) -> GGEMSImGuiLayer & = delete;

  /*! \brief Disallows move assignment of the owned UI state. */
  auto operator=(GGEMSImGuiLayer &&) -> GGEMSImGuiLayer & = delete;

  /*!
   * \brief Builds panels and applies camera and visibility input.
   *
   * \param[in] inputs Borrowed frame resources, valid throughout this call.
   * \param[in] device_status Device information shown by the status panel.
   * \param[in,out] workbench Shared presentation state updated by panel
   *   interactions.
   */
  auto BuildFrame(FrameInputs const &inputs,
                  detail::GGEMSDeviceStatusSnapshot const &device_status,
                  detail::GGEMSWorkbenchState &workbench) -> void;

private:
  /*! \brief Records one-frame reset requests from the main menu. */
  struct MenuActions {
    /*! \brief Requests restoration of the default camera view. */
    bool reset_camera{false};

    /*! \brief Requests restoration of default docking and panel visibility. */
    bool reset_layout{false};
  };

  /*!
   * \brief Builds the dockspace and handles layout initialization.
   *
   * \param[in,out] workbench Panel visibility and scene controls.
   * \return Menu reset requests for this frame.
   */
  [[nodiscard]] auto BuildMainDockspace(detail::GGEMSWorkbenchState &workbench)
    -> MenuActions;

  /*!
   * \brief Builds view controls and collects reset requests.
   *
   * \param[in,out] workbench Panel, axis, and trace visibility controls.
   * \return Camera and layout reset requests.
   */
  [[nodiscard]] static auto
  BuildMainMenuBar(detail::GGEMSWorkbenchState &workbench) -> MenuActions;

  /*! \brief Output console filtering and display state. */
  GGEMSImGuiOutputPanel output_panel_;

  /*! \brief Viewport mouse-gesture state. */
  detail::GGEMSImGuiViewportPanel viewport_panel_;

  /*! \brief Whether saved or default docking has been admitted. */
  bool layout_initialized_{false};
};
} // namespace ggems::ui
