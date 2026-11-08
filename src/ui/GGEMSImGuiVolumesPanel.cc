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
 * \brief Builds the physical volume tree and its immutable metadata readout.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <algorithm>
#include <cfloat>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>

#include <imgui.h>

#include "GGEMS/ui/detail/GGEMSImGuiVolumesPanel.hh"
#include "GGEMS/ui/detail/GGEMSImGuiLayout.hh"
#include "GGEMS/render/GGEMSGeometryScene.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"
#include "GGEMS/units/GGEMSUnitConversion.hh"
#include "GGEMS/units/GGEMSUnitFormatting.hh"

namespace {

// =============================================================================
// =============================================================================

/*!
 * \brief Visits visible branches using snapshot-built physical child links.
 *
 * \param[in] scene Immutable physical occurrences in VolumeID order.
 * \param[in] volume_id Current physical row.
 * \param[in,out] selected_volume_id Shared physical selection.
 * \param[out] focus Double-click framing request, if any.
 */
auto BuildVolumeNode(ggems::render::GGEMSGeometryScene const &scene,
                     std::uint32_t volume_id,
                     std::optional<std::uint32_t> &selected_volume_id,
                     std::optional<std::uint32_t> &focus) -> void {
  auto const &occurrence = scene.occurrences[volume_id];
  ImGui::PushID(static_cast<int>(volume_id));

  ImGuiTreeNodeFlags flags =
    ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

  if (volume_id == 0U) {
    flags |= ImGuiTreeNodeFlags_DefaultOpen;
  }

  if (occurrence.children.empty()) {
    flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
  }

  if (selected_volume_id == volume_id) {
    flags |= ImGuiTreeNodeFlags_Selected;
  }

  bool const opened =
    volume_id == 0U
      ? ImGui::TreeNodeEx("##physical_volume", flags, "%s",
                          occurrence.shape_name.c_str())
      : ImGui::TreeNodeEx("##physical_volume", flags, "%s #%u",
                          occurrence.shape_name.c_str(), volume_id);

  if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
    selected_volume_id = volume_id;
    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
      focus = volume_id;
    }
  }

  if (opened && !occurrence.children.empty()) {
    for (auto const child : occurrence.children) {
      BuildVolumeNode(scene, child, selected_volume_id, focus);
    }
    ImGui::TreePop();
  }
  ImGui::PopID();
}

// =============================================================================
// =============================================================================

/*!
 * \brief Displays captured authored dimensions, material, and composed pose.
 *
 * \param[in] occurrence Selected physical occurrence from the current scene.
 */
auto BuildVolumeMetadata(
  ggems::render::GGEMSGeometryOccurrence const &occurrence) -> void {
  ImGui::PushTextWrapPos(ImGui::GetCursorPosX() +
                         ImGui::GetContentRegionAvail().x);

  ImGui::Text("VolumeID: %u", occurrence.volume_id);

  if (occurrence.volume_id == 0U) {
    ImGui::TextUnformatted("Parent: none (World root)");
  } else {
    ImGui::Text("Parent VolumeID: %u", occurrence.parent_volume_id);
  }

  ImGui::Text("Type: %s", occurrence.shape_name.c_str());
  ImGui::SeparatorText("Dimensions");

  for (auto const &property : occurrence.dimensions) {
    ImGui::Text("%s: %s", property.name.c_str(), property.value.c_str());
  }

  ImGui::SeparatorText("World position");

  for (std::size_t axis = 0U; axis < 3U; ++axis) {
    auto const position =
      ggems::units::MakeQuantity<ggems::units::PositionCoordinate>(
        occurrence.position_m[axis], "m");
    std::string const text = position
                               ? ggems::units::HumanReadable(*position, 3)
                               : "N/A (outside unit range)";
    ImGui::Text("%c: %s", static_cast<int>('X' + axis), text.c_str());
  }

  ImGui::SeparatorText("World orientation");
  ImGui::TextDisabled("Local axes in World coordinates");

  for (std::size_t axis = 0U; axis < 3U; ++axis) {
    ImGui::Text("%c: (%.6g, %.6g, %.6g)", static_cast<int>('X' + axis),
                occurrence.orientation[axis], occurrence.orientation[3U + axis],
                occurrence.orientation[6U + axis]);
  }

  ImGui::SeparatorText("Material");
  ImGui::Text("Name: %s", occurrence.material_name.c_str());

  auto const density =
    ggems::units::HumanReadable(occurrence.material_density, 3);

  ImGui::Text("Density: %s", density.c_str());
  ImGui::PopTextWrapPos();
}

} // namespace

namespace ggems::ui::detail {

// =============================================================================
// =============================================================================
auto BuildVolumesPanel(bool &show_window,
                       std::optional<std::uint32_t> &selected_volume_id,
                       render::GGEMSGeometryScene const *scene)
  -> std::optional<std::uint32_t> {
  std::optional<std::uint32_t> focus;
  if (ImGui::Begin(k_volumes_window_name, &show_window)) {
    float const available = std::max(1.0F, ImGui::GetContentRegionAvail().y);
    float const minimum =
      std::min(3.0F * ImGui::GetTextLineHeightWithSpacing(), 0.3F * available);

    ImGui::SetNextWindowSizeConstraints(
      ImVec2{0.0F, minimum},
      ImVec2{FLT_MAX, std::max(minimum, available - minimum -
                                          ImGui::GetStyle().ItemSpacing.y)});
    if (ImGui::BeginChild("##volume_tree", ImVec2{0.0F, 0.5F * available},
                          ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeY)) {
      if (scene != nullptr && !scene->occurrences.empty()) {
        BuildVolumeNode(*scene, 0U, selected_volume_id, focus);
      } else {
        ImGui::TextDisabled("No World loaded yet");
      }
    }

    ImGui::EndChild();

    if (ImGui::BeginChild("##volume_metadata", ImVec2{0.0F, 0.0F},
                          ImGuiChildFlags_Borders)) {
      if (scene != nullptr && selected_volume_id) {
        BuildVolumeMetadata(scene->occurrences[*selected_volume_id]);
      } else {
        ImGui::TextDisabled("Select a physical volume to inspect it.");
        ImGui::TextWrapped(
          "Double-click a row to center and frame that occurrence.");
      }
    }

    ImGui::EndChild();
  }

  ImGui::End();
  return focus;
}

} // namespace ggems::ui::detail
