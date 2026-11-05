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
 * \brief Displays Cartesian grid diagnostics and reference-plane positions.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <array>
#include <cstddef>
#include <optional>
#include <string>

#include <imgui.h>

#include "GGEMS/ui/detail/GGEMSImGuiCartesianGrid.hh"
#include "GGEMS/ui/detail/GGEMSCartesianGrid.hh"
#include "GGEMS/ui/detail/GGEMSImGuiViewportPanel.hh"
#include "GGEMS/ui/detail/GGEMSSceneCamera.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"
#include "GGEMS/units/GGEMSUnitConversion.hh"
#include "GGEMS/units/GGEMSUnitFormatting.hh"

namespace {

/*! \brief Stable ordering shared with the camera's reference planes. */
constexpr std::array<char const *, 3U> k_plane_names{"XY", "XZ", "YZ"};

/*! \brief Explicit physical equations for the three selectable planes. */
constexpr std::array<char const *, 3U> k_plane_labels{
  "XY (Z = 0)",
  "XZ (Y = 0)",
  "YZ (X = 0)",
};

/*!
 * \brief Formats a diagnostic length with the existing GGEMS unit policy.
 *
 * \param[in] meters Signed position or nonnegative spacing in scene meters.
 * \param[in] spacing Whether a nonzero interval must not be displayed as zero.
 * \return Readable length, or an explicit indication of conversion failure.
 */
[[nodiscard]] auto FormatMeters(double meters, bool spacing = false)
  -> std::string {
  auto const quantity =
    ggems::units::MakeQuantity<ggems::units::PositionCoordinate>(meters, "m");

  if (!quantity) {
    return "N/A (outside unit range)";
  }

  if (quantity->value == 0) {
    return spacing ? "<1 pm" : "0 mm";
  }

  return ggems::units::HumanReadable(*quantity, 3);
}

} // namespace

namespace ggems::ui::detail {

// -----------------------------------------------------------------------------

auto BuildCartesianGridReadout(GGEMSSceneCamera const &camera,
                               GGEMSCartesianGrid const &grid,
                               GGEMSImGuiViewportPanel::Result const &input,
                               bool world_loaded) -> void {
  ImGui::SeparatorText("World reference");
  ImGui::PushTextWrapPos(ImGui::GetCursorPosX() +
                         ImGui::GetContentRegionAvail().x);
  ImGui::Text("Cursor plane: %s",
              k_plane_labels[static_cast<std::size_t>(grid.reference_plane)]);
  ImGui::TextDisabled("Ray-plane intersection (not a surface hit)");

  auto const position =
    world_loaded && input.cursor_pixels
      ? camera.IntersectReferencePlane(
          input.cursor_pixels->x, input.cursor_pixels->y, grid.reference_plane)
      : std::nullopt;

  if (position) {
    auto const x = FormatMeters((*position)[0]);
    auto const y = FormatMeters((*position)[1]);
    auto const z = FormatMeters((*position)[2]);
    ImGui::Text("Cursor world: (%s, %s, %s)", x.c_str(), y.c_str(), z.c_str());
  } else {
    ImGui::TextUnformatted("Cursor world: N/A");
    ImGui::SetItemTooltip("Requires a loaded World, a cursor over the current "
                          "viewport image, and a nonparallel ray intersecting "
                          "the selected plane inside the camera depth range.");
  }

  if (input.cursor_pixels) {
    ImGui::Text("Viewport cursor (px): (%.1f, %.1f)", input.cursor_pixels->x,
                input.cursor_pixels->y);
  } else {
    ImGui::TextUnformatted("Viewport cursor (px): N/A");
  }

  auto const plane = static_cast<std::size_t>(grid.reference_plane);
  auto const step = grid.GetMinorStepM();

  if (world_loaded && step > 0.0F) {
    auto const minor = FormatMeters(step, true);
    auto const major = FormatMeters(5.0F * step, true);
    ImGui::Text("%s grid: %s | minor %s / major %s", k_plane_names[plane],
                grid.visible[plane] ? "on" : "off", minor.c_str(),
                major.c_str());
  } else {
    ImGui::Text("%s grid: %s | step N/A", k_plane_names[plane],
                grid.visible[plane] ? "on" : "off");
  }

  ImGui::PopTextWrapPos();
}

} // namespace ggems::ui::detail
