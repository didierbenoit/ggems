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
#include <limits>

#include <imgui.h>

#include "GGEMS/ui/detail/GGEMSImGuiVolumesPanel.hh"
#include "GGEMS/ui/detail/GGEMSImGuiLayout.hh"
#include "GGEMS/ui/detail/GGEMSImGuiTheme.hh"
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

/*!
 * \brief Draws aligned full-label mode buttons without selecting a volume.
 * \param[in] mode Current effective choice, or no choice for a mixed group.
 * \param[in] world Whether to omit Solid for the World envelope.
 * \return Clicked choice, if any.
 */
auto BuildModeButtons(std::optional<Mode> mode, bool world = false)
  -> std::optional<Mode> {
  ImGui::TableSetColumnIndex(1);
  std::optional<Mode> changed;
  std::size_t const count = world ? 2U : k_mode_names.size();

  for (std::size_t index = 0U; index < count; ++index) {
    if (index != 0U) {
      ImGui::SameLine();
    }

    auto const choice = static_cast<Mode>(index);
    bool const selected = mode == choice;

    if (selected) {
      ImGui::PushStyleColor(
        ImGuiCol_Button,
        ggems::ui::GetThemeColor(ggems::ui::GGEMSThemeRole::Accent));
      ImGui::PushStyleColor(
        ImGuiCol_ButtonHovered,
        ggems::ui::GetThemeColor(ggems::ui::GGEMSThemeRole::AccentStrong));
      ImGui::PushStyleColor(
        ImGuiCol_ButtonActive,
        ggems::ui::GetThemeColor(ggems::ui::GGEMSThemeRole::Attention));
      ImGui::PushStyleColor(ImGuiCol_Text,
                            ggems::ui::GetThemeColor(
                              ggems::ui::GGEMSThemeRole::RecessedBackground));
    }

    if (ImGui::Button(k_mode_names[index])) {
      changed = choice;
    }

    if (selected) {
      ImGui::PopStyleColor(4);
    }
  }
  return changed;
}

/*!
 * \brief Reserves the full labels actually offered by the current scene.
 * \param[in] world_only Whether no physical child needs a Solid button.
 * \return Control-column width including button padding and spacing
 */
auto ModeControlsWidth(bool world_only) -> float {
  auto const count = world_only ? 2U : k_mode_names.size();

  float width =
    static_cast<float>(count - 1U) * ImGui::GetStyle().ItemSpacing.x;

  for (auto const *label : std::span{k_mode_names}.first(count)) {
    width +=
      ImGui::CalcTextSize(label).x + (2.0F * ImGui::GetStyle().FramePadding.x);
  }

  return width;
}

/*!
 * \brief Starts a table row with a stable disclosure ID independent of text.
 * \param[in] leaf Whether there is no expandable content.
 * \param[in] default_open Whether to open this row in a new snapshot.
 * \return Whether its children should be submitted.
 */
auto BeginTreeRow(bool leaf, bool default_open = false) -> bool {
  ImGui::TableNextRow();
  ImGui::TableSetColumnIndex(0);
  ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow |
                             ImGuiTreeNodeFlags_NoTreePushOnOpen |
                             ImGuiTreeNodeFlags_FramePadding;
  if (leaf) {
    flags |= ImGuiTreeNodeFlags_Leaf;
  }

  if (default_open) {
    flags |= ImGuiTreeNodeFlags_DefaultOpen;
  }
  bool const opened = ImGui::TreeNodeEx("##branch", flags);
  ImGui::SameLine();
  return opened;
}

/*!
 * \brief Borrows one snapshot and its presentation state for the current tree.
 *
 * Membership and mixed-state counts are prepared outside the frame path.
 * Only opened physical branches submit their descendants.
 */
struct VolumeTree {
  /*! \brief Immutable presentation metadata and prepared memberships. */
  ggems::render::GGEMSGeometryScene const &scene;

  /*! \brief Viewer-owned modes and cached effective group counts. */
  ggems::render::GGEMSGeometryAppearanceState &appearance;

  /*! \brief Physical selection shared with the existing inspector. */
  std::optional<std::uint32_t> &selected_volume_id;

  /*! \brief Viewer-only named group selection, independent of VolumeID. */
  std::optional<std::uint32_t> &selected_geometry_group;

  /*! \brief Double-click request for the caller's camera. */
  std::optional<std::uint32_t> &focus;

  /*!
   * \brief Displays independent original mapping entries under a voxel row.
   * \param[in] volume_id Physical voxel occurrence whose mappings are edited.
   */
  auto Mappings(std::uint32_t volume_id) -> void {
    auto const &mappings = scene.occurrences[volume_id].voxel_slices->mappings;
    ImGui::PushID("##mappings");

    for (std::uint32_t index = 0U; index < mappings.size(); ++index) {
      auto const &mapping = mappings[index];

      ImGui::PushID(static_cast<int>(index));
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::AlignTextToFramePadding();

      auto const label =
        mapping.end_label == mapping.first_label + 1U
          ? std::format("{} [{}]", mapping.material_name, mapping.first_label)
          : std::format("{} [{}, {})", mapping.material_name,
                        mapping.first_label, mapping.end_label);

      ImGui::TextUnformatted(label.c_str());
      ImGui::SetItemTooltip("Only this original label mapping; colors and "
                            "scientific Materials stay unchanged.");

      if (auto const mode = BuildModeButtons(
            appearance.GetVoxelMappingMode(volume_id, index))) {
        appearance.SetVoxelMappingMode(volume_id, index, *mode);
      }

      ImGui::PopID();
    }

    ImGui::PopID();
  }

  /*!
   * \brief Displays one occurrence without inheriting its parent's mode.
   * \param[in] volume_id Physical occurrence to select, frame, or edit.
   * \param[in] controls Whether this row owns the occurrence mode buttons.
   * \param[in] children Whether to expose its physical children or mappings.
   */
  auto Physical(std::uint32_t volume_id, bool controls = true,
                bool children = true) -> void {
    auto const &occurrence = scene.occurrences[volume_id];

    ImGui::PushID("##physical");
    ImGui::PushID(static_cast<int>(volume_id));

    bool const leaf =
      !children || (occurrence.children.empty() && !occurrence.voxel_slices);
    bool const opened = BeginTreeRow(leaf, volume_id == 0U && children);
    auto const label =
      volume_id == 0U ? occurrence.shape_name
                      : std::format("{} #{}", occurrence.shape_name, volume_id);

    ImGui::Selectable(label.c_str(), selected_volume_id == volume_id,
                      ImGuiSelectableFlags_AllowDoubleClick);

    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
      selected_volume_id = volume_id;
      selected_geometry_group.reset();
      if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        focus = volume_id;
      }
    }

    if (controls && !occurrence.voxel_slices) {
      if (auto const mode = BuildModeButtons(
            appearance.GetOccurrenceMode(scene, volume_id), volume_id == 0U)) {
        appearance.SetOccurrenceMode(scene, volume_id, *mode);
      }
    }

    if (opened && !leaf) {
      ImGui::TableSetColumnIndex(0);
      ImGui::TreePush("##children");

      if (volume_id == 0U) {
        Roots();
      } else if (occurrence.voxel_slices) {
        Mappings(volume_id);
      } else {
        for (auto const &branch : occurrence.child_branches) {
          Branch(branch);
        }
      }

      ImGui::TreePop();
    }

    ImGui::PopID();
    ImGui::PopID();
  }

  /*!
   * \brief Offers a collapsed browsing row for exact local repeated roots.
   * \param[in] members Prepared physical roots in original occurrence order.
   */
  auto Branch(std::span<std::uint32_t const> members) -> void {
    if (members.size() == 1U) {
      Physical(members.front());
      return;
    }

    ImGui::PushID("##copies");
    ImGui::PushID(static_cast<int>(members.front()));

    bool const opened = BeginTreeRow(false);
    auto const label = std::format(
      "{} copies ({})", scene.occurrences[members.front()].shape_name,
      members.size());

    ImGui::TextUnformatted(label.c_str());
    ImGui::SetItemTooltip("Browse physical occurrences; each row controls "
                          "only that occurrence.");

    if (opened) {
      ImGui::TreePush("##members");

      for (auto const member : members) {
        Physical(member);
      }

      ImGui::TableSetColumnIndex(0);
      ImGui::TreePop();
    }

    ImGui::PopID();
    ImGui::PopID();
  }

  /*!
   * \brief Displays a prepared authored group and its independent controls.
   * \param[in] group_index Snapshot-local named group index.
   */
  auto Group(std::uint32_t group_index) -> void {
    auto const &group = scene.groups[group_index];

    ImGui::PushID("##named");
    ImGui::PushID(static_cast<int>(group_index));

    bool const expandable =
      group.subtree ||
      scene.occurrences[group.physical_roots.front()].voxel_slices.has_value();
    bool opened = false;
    if (expandable) {
      opened = BeginTreeRow(false, true);
    } else {
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::SetCursorPosX(ImGui::GetCursorPosX() +
                           ImGui::GetTreeNodeToLabelSpacing() +
                           ImGui::GetStyle().ItemSpacing.x);
    }
    auto const mode = appearance.GetGroupMode(group_index);
    auto const label = std::format("{} ({}){}", group.name,
                                   group.members.size(), mode ? "" : " Mixed");
    auto const position = ImGui::GetCursorScreenPos();
    if (ImGui::Selectable(
          "##group_selection", selected_geometry_group == group_index,
          ImGuiSelectableFlags_None, ImVec2{0.0F, ImGui::GetFrameHeight()})) {
      selected_geometry_group = group_index;
      selected_volume_id.reset();
    }
    ImGui::SetItemTooltip(group.subtree
                            ? "Entire analytic subtree: %zu occurrences."
                            : "Only this authored group: %zu occurrences.",
                          group.members.size());
    ImGui::SetCursorScreenPos(
      ImVec2{position.x, position.y + ImGui::GetStyle().FramePadding.y});
    ImGui::TextUnformatted(label.c_str());

    if (auto const changed = BuildModeButtons(mode)) {
      appearance.SetGroupMode(scene, group_index, *changed);
    }

    if (opened) {
      ImGui::TableSetColumnIndex(0);
      ImGui::TreePush("##contents");

      for (auto const child : group.child_groups) {
        Group(child);
      }

      if (group.physical_leaves &&
          group.physical_roots.size() <=
            static_cast<std::size_t>(std::numeric_limits<int>::max())) {
        ImGuiListClipper clipper;

        clipper.Begin(static_cast<int>(group.physical_roots.size()),
                      ImGui::GetFrameHeight() +
                        (2.0F * ImGui::GetStyle().CellPadding.y));

        while (clipper.Step()) {
          for (int index = clipper.DisplayStart; index < clipper.DisplayEnd;
               ++index) {
            Physical(group.physical_roots[static_cast<std::size_t>(index)]);
          }
        }
      } else {
        for (auto const root : group.physical_roots) {
          Physical(root);
        }
      }

      // The clipper may have ended the row; only restore indentation.
      ImGui::TreePop();
    }

    ImGui::PopID();
    ImGui::PopID();
  }

  /*! \brief Visits prepared World children without rediscovering groups. */
  auto Roots() -> void {
    for (auto const &entry : scene.root_entries) {
      if (entry.group_index) {
        Group(*entry.group_index);
      } else {
        Branch(entry.volumes);
      }
    }
  }

  /*! \brief Displays the actual World name without adding a project node. */
  auto Build() -> void {
    auto const &world = scene.occurrences.front();

    if (world.name.empty()) {
      Physical(0U);
      return;
    }

    ImGui::PushID("##world_name");

    bool const opened = BeginTreeRow(false, true);

    ImGui::TextUnformatted(world.name.c_str());

    if (auto const mode =
          BuildModeButtons(appearance.GetOccurrenceMode(scene, 0U), true)) {
      appearance.SetOccurrenceMode(scene, 0U, *mode);
    }

    if (opened) {
      ImGui::TableSetColumnIndex(0);
      ImGui::TreePush("##contents");

      Physical(0U, false, false);
      Roots();

      ImGui::TreePop();
    }

    ImGui::PopID();
  }
};

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

  if (!occurrence.name.empty()) {
    ImGui::Text("Authored name: %s", occurrence.name.c_str());
  }

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

  if (occurrence.voxel_slices) {
    ImGui::Text("Material palette: %zu entries",
                occurrence.voxel_slices->palette_size);
  } else {
    ImGui::Text("Name: %s", occurrence.material_name.c_str());

    auto const density =
      ggems::units::HumanReadable(occurrence.material_density, 3);

    ImGui::Text("Density: %s", density.c_str());
  }

  ImGui::TextWrapped("Production Cuts: not available in this scene snapshot");
  ImGui::PopTextWrapPos();
}

} // namespace

namespace ggems::ui::detail {

// =============================================================================
// =============================================================================
auto BuildVolumesPanel(bool &show_window,
                       std::optional<std::uint32_t> &selected_volume_id,
                       std::optional<std::uint32_t> &selected_geometry_group,
                       render::GGEMSGeometryScene const *scene,
                       render::GGEMSGeometryAppearanceState &appearance,
                       std::uint32_t presentation_revision)
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
        ImGui::PushID(static_cast<int>(presentation_revision));

        auto const controls_width =
          ModeControlsWidth(scene->occurrences.size() == 1U);

        if (ImGui::BeginTable("##volume_rows", 2,
                              ImGuiTableFlags_RowBg |
                                ImGuiTableFlags_SizingStretchProp |
                                ImGuiTableFlags_NoSavedSettings)) {
          ImGui::TableSetupColumn("Volume",
                                  ImGuiTableColumnFlags_WidthStretch |
                                    ImGuiTableColumnFlags_IndentEnable);
          ImGui::TableSetupColumn("Display", ImGuiTableColumnFlags_WidthFixed,
                                  controls_width);

          VolumeTree tree{
            .scene = *scene,
            .appearance = appearance,
            .selected_volume_id = selected_volume_id,
            .selected_geometry_group = selected_geometry_group,
            .focus = focus,
          };

          tree.Build();
          ImGui::EndTable();
        }

        ImGui::PopID();
      } else {
        ImGui::TextDisabled("No World loaded yet");
      }
    }

    ImGui::EndChild();

    if (ImGui::BeginChild("##volume_metadata", ImVec2{0.0F, 0.0F},
                          ImGuiChildFlags_Borders)) {
      if (scene != nullptr && selected_geometry_group) {
        auto const &group = scene->groups[*selected_geometry_group];
        ImGui::PushTextWrapPos(ImGui::GetCursorPosX() +
                               ImGui::GetContentRegionAvail().x);
        ImGui::TextUnformatted(group.name.c_str());
        ImGui::SeparatorText("Group properties");
        for (auto const &property : group.inspection) {
          ImGui::Text("%s: %s", property.name.c_str(), property.value.c_str());
        }
        ImGui::TextWrapped(
          "Production Cuts: not available in this scene snapshot");
        ImGui::PopTextWrapPos();
      } else if (scene != nullptr && selected_volume_id) {
        auto const &occurrence = scene->occurrences[*selected_volume_id];
        BuildVolumeMetadata(occurrence);
      } else {
        ImGui::TextDisabled("Select a group or physical volume to inspect it.");
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
