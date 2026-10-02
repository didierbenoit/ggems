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
 * \brief Stores presentation controls shared by the UI panels.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>

#include "GGEMSSceneCamera.hh"
#include "GGEMS/render/GGEMSParticleTrace.hh"

namespace ggems::ui::detail {

/*! \brief Owns camera, panel visibility, and scene selection state. */
struct GGEMSWorkbenchState {
  /*! \brief Identifies the selected scene hierarchy category. */
  enum class SceneSelection : std::uint8_t {
    /*! \brief No scene category is selected. */
    None,

    /*! \brief World entry in the scene hierarchy. */
    World,

    /*! \brief Submitted source snapshot entries. */
    Sources,

    /*! \brief Geometry collection entry. */
    Volumes,

    /*! \brief Material collection entry. */
    Materials,

    /*! \brief Diagnostic particle trace controls. */
    Tracks,
  };

  /*! \brief Requests a scene target size for the next frame. */
  struct ViewportRequest {
    /*! \brief Whether the scene viewport is displayed this frame. */
    bool visible{false};

    /*! \brief Requested width in logical UI pixels. */
    std::uint32_t width{0U};

    /*! \brief Requested height in logical UI pixels. */
    std::uint32_t height{0U};
  };

  /*! \brief Presentation camera independent of simulation state. */
  GGEMSSceneCamera camera;

  /*! \brief Whether to show the output console. */
  bool show_output_panel{true};

  /*! \brief Whether to show renderer and compute status. */
  bool show_status_panel{true};

  /*! \brief Whether to show the scene hierarchy. */
  bool show_scene_panel{true};

  /*! \brief Whether to show the scene image and camera controls. */
  bool show_viewport_panel{true};

  /*! \brief Whether to show the selected category inspector. */
  bool show_inspector_panel{true};

  /*! \brief Whether to draw the reference axes. */
  bool show_axes{true};

  /*! \brief Global and per-source diagnostic trace visibility. */
  render::GGEMSParticleTraceVisibility trace_visibility{};

  /*! \brief Scene category selected for inspection. */
  SceneSelection selected_scene_item{SceneSelection::World};

  /*! \brief Latest requested logical scene extent. */
  ViewportRequest viewport{};

  /*! \brief Revision separating widget IDs across source snapshots. */
  std::uint32_t source_presentation_revision{0U};
};

} // namespace ggems::ui::detail
