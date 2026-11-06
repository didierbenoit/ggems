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
 * \brief Displays physical grid intervals and Cartesian/polar cursor positions.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <array>
#include <cmath>
#include <numbers>
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
  auto const plane = static_cast<std::size_t>(grid.reference_plane);

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

    auto const first = plane == 2U ? 1U : 0U;
    auto const second = plane == 0U ? 1U : 2U;

    double const radius = std::hypot((*position)[first], (*position)[second]);
    auto const radius_quantity =
      ggems::units::MakeQuantity<ggems::units::PositionCoordinate>(radius, "m");
    bool const zero_radius = radius_quantity && radius_quantity->value == 0;

    std::string radius_text{"N/A (outside unit range)"};

    if (radius_quantity) {
      radius_text =
        zero_radius ? "0 mm" : ggems::units::HumanReadable(*radius_quantity, 3);
    }

    ImGui::Text("Cursor radius: %s", radius_text.c_str());

    if (zero_radius) {
      ImGui::TextUnformatted(
        "Cursor azimuth (deg): N/A (origin / below length resolution)");
    } else {
      double const degrees =
        std::atan2((*position)[second], (*position)[first]) *
        (180.0 / std::numbers::pi);

      // Wrap after rounding so the displayed value also stays below 360.
      double const azimuth =
        std::fmod((std::round(degrees * 1000.0) / 1000.0) + 360.0, 360.0);

      ImGui::Text("Cursor azimuth (deg): %.3f", azimuth);
    }
    ImGui::SetItemTooltip("From +%c toward +%c in [0,360) degrees; unavailable "
                          "when the radius rounds to zero picometers.",
                          "XYZ"[first], "XYZ"[second]);
  } else {
    ImGui::TextUnformatted("Cursor world: N/A");
    ImGui::SetItemTooltip("Requires a loaded World, a cursor over the current "
                          "viewport image, and a nonparallel ray intersecting "
                          "the selected plane inside the camera depth range.");
    ImGui::TextUnformatted("Cursor radius: N/A");
    ImGui::TextUnformatted("Cursor azimuth (deg): N/A");
  }

  if (input.cursor_pixels) {
    ImGui::Text("Viewport cursor (px): (%.1f, %.1f)", input.cursor_pixels->x,
                input.cursor_pixels->y);
  } else {
    ImGui::TextUnformatted("Viewport cursor (px): N/A");
  }

  auto const step = grid.GetMinorStepM();
  ImGui::Text("%s grids: Cartesian %s / Polar %s", k_plane_names[plane],
              grid.visible[plane] ? "on" : "off",
              grid.polar_visible[plane] ? "on" : "off");

  if (world_loaded && step > 0.0F) {
    auto const minor = FormatMeters(step, true);
    auto const major = FormatMeters(5.0F * step, true);
    ImGui::Text("Interval: minor %s / major %s", minor.c_str(), major.c_str());
  } else {
    ImGui::TextUnformatted("Interval: step N/A");
  }

  ImGui::PopTextWrapPos();
}

} // namespace ggems::ui::detail
