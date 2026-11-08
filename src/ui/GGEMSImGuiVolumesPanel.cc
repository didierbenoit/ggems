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
 * \brief Builds physical volume rows, display controls, and metadata.
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
#include <array>
#include <span>
#include <format>

#include <imgui.h>

#include "GGEMS/ui/detail/GGEMSImGuiVolumesPanel.hh"
#include "GGEMS/ui/detail/GGEMSImGuiLayout.hh"
#include "GGEMS/render/GGEMSGeometryScene.hh"
#include "GGEMS/render/GGEMSGeometryAppearance.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"
#include "GGEMS/units/GGEMSUnitConversion.hh"
#include "GGEMS/units/GGEMSUnitFormatting.hh"

namespace {

// =============================================================================
// =============================================================================

/*! \brief Display modes shared with the renderer. */
using Mode = ggems::render::GGEMSGeometryDisplayMode;

/*! \brief Readable labels in display-mode order. */
constexpr std::array<char const *, 3U> k_mode_names{
  "None",
  "Wireframe",
  "Solid",
};

/*! \brief Compact row labels in the same display-mode order. */
constexpr std::array<char const *, 3U> k_mode_letters{"N", "W", "S"};

/*!
 * \brief Edits one occurrence's display override independently of its row.
 *
 * \param[in] occurrence Physical identity and default-mode hierarchy.
 * \param[in,out] appearance Presentation overrides for this occurrence only.
 */
auto BuildDisplayMode(ggems::render::GGEMSGeometryOccurrence const &occurrence,
                      ggems::render::GGEMSGeometryAppearance &appearance)
  -> void {
  auto const mode =
    ggems::render::ResolveGeometryDisplayMode(occurrence, appearance);

  auto const index = static_cast<std::size_t>(mode);
  ImGui::SetNextItemWidth(ImGui::GetFontSize() +
                          (2.0F * ImGui::GetStyle().FramePadding.x));

  bool const opened = ImGui::BeginCombo("##display_mode", k_mode_letters[index],
                                        ImGuiComboFlags_NoArrowButton);

  ImGui::SetItemTooltip("Display: %s. Changes only this occurrence; "
                        "daughters stay independent.",
                        k_mode_names[index]);
  if (opened) {
    std::size_t const count = occurrence.volume_id == 0U ? 2U : 3U;
    for (std::size_t choice = 0U; choice < count; ++choice) {
      auto const choice_mode = static_cast<Mode>(choice);
      if (ImGui::Selectable(k_mode_names[choice], mode == choice_mode)) {
        appearance.mode_override = choice_mode;
      }
    }

    ImGui::EndCombo();
  }
}

/*!
 * \brief Visits visible branches with separate arrow, mode, and label inputs.
 *
 * \param[in] scene Immutable physical occurrences in VolumeID order.
 * \param[in] volume_id Current physical row.
 * \param[in,out] selected_volume_id Shared physical selection.
 * \param[in,out] appearance Per-occurrence presentation overrides.
 * \param[out] focus Double-click framing request, if any.
 */
auto BuildVolumeNode(
  ggems::render::GGEMSGeometryScene const &scene, std::uint32_t volume_id,
  std::optional<std::uint32_t> &selected_volume_id,
  std::span<ggems::render::GGEMSGeometryAppearance> appearance,
  std::optional<std::uint32_t> &focus) -> void {
  auto const &occurrence = scene.occurrences[volume_id];
  ImGui::PushID(static_cast<int>(volume_id));

  ImGuiTreeNodeFlags flags =
    ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_NoTreePushOnOpen |
    ImGuiTreeNodeFlags_SpanLabelWidth | ImGuiTreeNodeFlags_FramePadding;

  if (volume_id == 0U) {
    flags |= ImGuiTreeNodeFlags_DefaultOpen;
  }

  if (occurrence.children.empty()) {
    flags |= ImGuiTreeNodeFlags_Leaf;
  }

  bool const opened = ImGui::TreeNodeEx("##physical_branch", flags);

  ImGui::SameLine();
  BuildDisplayMode(occurrence, appearance[volume_id]);
  ImGui::SameLine();

  auto const label =
    volume_id == 0U ? occurrence.shape_name
                    : std::format("{} #{}", occurrence.shape_name, volume_id);

  ImGui::Selectable(label.c_str(), selected_volume_id == volume_id,
                    ImGuiSelectableFlags_AllowDoubleClick);

  if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
    selected_volume_id = volume_id;
    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
      focus = volume_id;
    }
  }

  if (opened && !occurrence.children.empty()) {
    ImGui::TreePush("##children");
    for (auto const child : occurrence.children) {
      BuildVolumeNode(scene, child, selected_volume_id, appearance, focus);
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
  ImGui::TextWrapped("Production Cuts: not available in this scene snapshot");
  ImGui::PopTextWrapPos();
}

} // namespace

namespace ggems::ui::detail {

// =============================================================================
// =============================================================================
auto BuildVolumesPanel(bool &show_window,
                       std::optional<std::uint32_t> &selected_volume_id,
                       render::GGEMSGeometryScene const *scene,
                       std::span<render::GGEMSGeometryAppearance> appearance)
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
        BuildVolumeNode(*scene, 0U, selected_volume_id, appearance, focus);
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
