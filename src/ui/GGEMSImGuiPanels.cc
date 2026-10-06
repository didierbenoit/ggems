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
 * \brief Presents device status, completed sources, and scene-category
 *   controls.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include <imgui.h>

#include "GGEMS/ui/detail/GGEMSImGuiPanels.hh"
#include "GGEMS/ui/detail/GGEMSDeviceStatus.hh"
#include "GGEMS/ui/detail/GGEMSImGuiLayout.hh"
#include "GGEMS/ui/detail/GGEMSWorkbenchState.hh"

#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/radioactivity/GGEMSRadionuclideDefinition.hh"
#include "GGEMS/radioactivity/GGEMSRadionuclideEmission.hh"
#include "GGEMS/render/GGEMSParticleTrace.hh"
#include "GGEMS/sources/GGEMSEnergyDistributionRecord.hh"
#include "GGEMS/sources/GGEMSSourceRunSnapshot.hh"
#include "GGEMS/sources/GGEMSSourceTypes.hh"
#include "GGEMS/sources/GGEMSSourceValidation.hh"
#include "GGEMS/sources/GGEMSSourcePopulation.hh"
#include "GGEMS/sources/GGEMSSourcePopulationRecord.hh"
#include "GGEMS/units/GGEMSAngularUnits.hh"
#include "GGEMS/units/GGEMSEnergyUnits.hh"
#include "GGEMS/units/GGEMSLengthUnits.hh"
#include "GGEMS/units/GGEMSTimeUnits.hh"
#include "GGEMS/units/GGEMSUnitFormatting.hh"

namespace {

/*! \brief Scene hierarchy categories shared with the workbench. */
using SceneSelection = ggems::ui::detail::GGEMSWorkbenchState::SceneSelection;

// =============================================================================
// =============================================================================

/*!
 * \brief Returns the display label for a scene category.
 *
 * \param[in] selection Selected scene hierarchy category.
 * \return Static label, or Unknown for an unrecognized value.
 */
[[nodiscard]] auto GetSceneSelectionName(SceneSelection selection) noexcept
  -> char const * {
  switch (selection) {
  case SceneSelection::None:
    return "None";
  case SceneSelection::World:
    return "World";
  case SceneSelection::Sources:
    return "Sources";
  case SceneSelection::Volumes:
    return "Volumes";
  case SceneSelection::Materials:
    return "Materials";
  case SceneSelection::Tracks:
    return "Tracks";
  }

  return "Unknown";
}

// =============================================================================
// =============================================================================

/*!
 * \brief Displays one energy law with the compact source-panel summary.
 *
 * \param[in] energy_record Descriptor locating the law in the packed tables.
 * \param[in] energy_values Concatenated canonical energy tables in micro-eV.
 * \param[in] mono_energy_micro_eV Energy displayed by a monoenergetic law.
 */
auto BuildEnergyLawSummary(
  ggems::core::sources::GGEMSEnergyDistributionRecord const &energy_record,
  std::vector<std::uint64_t> const &energy_values,
  std::uint64_t mono_energy_micro_eV) -> void {
  namespace core = ggems::core;
  namespace units = ggems::units;

  auto const energy_distribution_type{
    core::sources::FromKernelEnergyDistributionType(
      energy_record.distribution_type),
  };

  std::string const energy_distribution{
    core::sources::ToLongName(energy_distribution_type)};

  ImGui::Text("Energy distribution: %s", energy_distribution.c_str());

  if (energy_distribution_type ==
      core::sources::GGEMSEnergyDistributionType::Mono) {
    std::string const energy =
      units::HumanReadable(units::Energy{mono_energy_micro_eV}, 3);
    ImGui::Text("Energy: %s", energy.c_str());
    return;
  }

  bool const table_offset_fits =
    energy_record.table_offset <=
    static_cast<std::uint64_t>(energy_values.size());

  std::size_t const table_offset =
    table_offset_fits ? static_cast<std::size_t>(energy_record.table_offset)
                      : 0U;

  auto const table_count = static_cast<std::size_t>(energy_record.table_count);

  bool const valid_table = table_offset_fits && table_count >= 2U &&
                           table_count <= energy_values.size() - table_offset;

  if (!valid_table) {
    ImGui::TextDisabled("Invalid energy table metadata");
    return;
  }

  std::string const first =
    units::HumanReadable(units::Energy{energy_values[table_offset]}, 3);

  std::string const last = units::HumanReadable(
    units::Energy{energy_values[table_offset + table_count - 1U]}, 3);

  if (energy_distribution_type ==
      core::sources::GGEMSEnergyDistributionType::DiscreteLines) {
    ImGui::Text("Line count: %zu", table_count);
    ImGui::Text("Line-energy range: %s - %s", first.c_str(), last.c_str());
  } else if (energy_distribution_type ==
             core::sources::GGEMSEnergyDistributionType::RegularSpectrum) {
    std::string const width = units::HumanReadable(
      units::Energy{energy_record.regular_bin_width_micro_eV}, 3);
    ImGui::Text("Bin count: %zu", table_count);
    ImGui::Text("Center range: %s - %s", first.c_str(), last.c_str());
    ImGui::Text("Bin width: %s", width.c_str());
  } else {
    ImGui::TextDisabled("Unknown energy distribution");
  }
}

// =============================================================================
// =============================================================================

/*!
 * \brief Displays the collapsed per-group summary of a radionuclide source.
 *
 * \param[in] source_run_snapshot Completed snapshot owning the packed groups.
 * \param[in] population Population record of the ActivityDriven source slot.
 * \param[in] definition Radionuclide definition retained for that slot.
 */
auto BuildEmissionGroupEntries(
  ggems::core::sources::GGEMSSourceRunSnapshot const &source_run_snapshot,
  ggems::core::sources::GGEMSSourcePopulationRecord const &population,
  ggems::core::radioactivity::GGEMSRadionuclideDefinition const &definition)
  -> void {
  namespace core = ggems::core;

  ImGuiTreeNodeFlags const flags =
    ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

  if (!ImGui::TreeNodeEx("##emission_groups", flags,
                         "Emission group details")) {
    return;
  }

  auto const &emission_records = source_run_snapshot.GetEmissionRecords();
  auto const &group_ranges = source_run_snapshot.GetGroupRanges();
  auto const &energy_records =
    source_run_snapshot.GetEnergyDistributionRecords();
  auto const &energy_values =
    source_run_snapshot.GetEnergyValuesMicroElectronVolt();
  auto const emissions = definition.GetEmissions();

  for (std::uint32_t offset = 0U; offset < population.emission_count;
       ++offset) {
    std::size_t const emission_index =
      static_cast<std::size_t>(population.first_emission_index) + offset;
    auto const &emission = emission_records[emission_index];

    std::string_view const particle_type{
      core::particles::ToLongName(
        core::particles::FromKernelParticleType(emission.particle_type)),
    };

    ImGui::Text("[%u] %.*s - %.8g per decay - %llu sampled",
                static_cast<unsigned int>(offset),
                static_cast<int>(particle_type.size()), particle_type.data(),
                static_cast<double>(emissions[offset].GetYieldPerDecay()),
                static_cast<unsigned long long>(
                  group_ranges[emission_index].primary_count));

    ImGui::Indent();
    BuildEnergyLawSummary(
      energy_records[emission.energy_distribution_record_index], energy_values,
      emission.mono_energy_micro_eV);
    ImGui::Unindent();
  }

  ImGui::TreePop();
}

// =============================================================================
// =============================================================================

/*!
 * \brief Displays completed source records and trace-visibility controls.
 *
 * \param[in,out] workbench Per-source visibility and source presentation
 *   revision.
 * \param[in] source_run_snapshot Borrowed completed snapshot, or null when
 *   absent.
 */
auto BuildSourceEntries(
  ggems::ui::detail::GGEMSWorkbenchState &workbench,
  ggems::core::sources::GGEMSSourceRunSnapshot const *source_run_snapshot)
  -> void {
  namespace core = ggems::core;
  namespace units = ggems::units;

  if (source_run_snapshot == nullptr) {
    ImGui::TextDisabled("No completed GGEMSRun source snapshot submitted yet");
    return;
  }

  auto const &records = source_run_snapshot->GetRecords();
  auto const &ranges = source_run_snapshot->GetRanges();
  auto const &population_records = source_run_snapshot->GetPopulationRecords();
  auto const &radionuclide_definitions =
    source_run_snapshot->GetRadionuclideDefinitions();
  auto const &energy_records =
    source_run_snapshot->GetEnergyDistributionRecords();
  auto const &energy_values =
    source_run_snapshot->GetEnergyValuesMicroElectronVolt();

  ggems::render::GGEMSParticleTraceVisibility &trace_visibility =
    workbench.trace_visibility;

  ImGui::PushID(static_cast<int>(workbench.source_presentation_revision));

  for (std::size_t source_index = 0U; source_index < records.size();
       ++source_index) {
    auto const &record = records[source_index];
    auto const &range = ranges[source_index];
    auto const &population = population_records[source_index];

    bool const activity_driven =
      population.population_mode ==
      core::sources::ToKernelSourcePopulationMode(
        core::sources::GGEMSSourcePopulationMode::ActivityDriven);

    core::radioactivity::GGEMSRadionuclideDefinition const *const definition =
      activity_driven ? radionuclide_definitions[source_index].get() : nullptr;

    std::string_view const source_type{
      core::sources::ToLongName(
        core::sources::FromKernelSourceType(record.source_type)),
    };

    std::string_view const particle_type{
      core::particles::ToLongName(
        core::particles::FromKernelParticleType(record.emitted_particle_type)),
    };

    std::string_view const label_kind =
      activity_driven ? std::string_view{"ActivityDriven"} : source_type;

    std::string_view const label_detail =
      activity_driven ? definition->GetCanonicalName() : particle_type;

    auto const emission_geometry_type{
      core::sources::FromKernelEmissionGeometryType(
        record.emission_geometry_type),
    };

    auto const angular_distribution_type{
      core::sources::FromKernelAngularDistributionType(
        record.angular_distribution_type),
    };

    std::string const emission_geometry{
      core::sources::ToLongName(emission_geometry_type)};

    std::string const angular_distribution{
      core::sources::ToLongName(angular_distribution_type)};

    std::uint64_t const projection_primary_end =
      range.projection_primary_begin + range.primary_count;

    ImGui::PushID(static_cast<int>(source_index));

    bool source_visible = trace_visibility.IsSourceVisible(
      static_cast<std::uint32_t>(source_index));

    if (ImGui::Checkbox("##trajectory_visibility", &source_visible)) {
      trace_visibility.SetSourceVisible(source_index, source_visible);
    }

    if (ImGui::IsItemHovered()) {
      ImGui::BeginTooltip();
      ImGui::TextUnformatted(
        "Controls trajectory visibility only; source state is unchanged.");
      ImGui::EndTooltip();
    }

    ImGui::SameLine();

    ImGuiTreeNodeFlags const flags =
      ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

    bool const opened = ImGui::TreeNodeEx(
      "##source_details", flags, "Source %zu - %.*s - %.*s", source_index,
      static_cast<int>(label_kind.size()), label_kind.data(),
      static_cast<int>(label_detail.size()), label_detail.data());

    if (opened) {
      ImGui::Text("Source index: %zu", source_index);
      ImGui::Text("Type: %.*s", static_cast<int>(source_type.size()),
                  source_type.data());

      if (activity_driven) {
        std::string_view const radionuclide_name =
          definition->GetCanonicalName();

        ImGui::TextUnformatted("Population: ActivityDriven");
        ImGui::Text("Radionuclide: %.*s",
                    static_cast<int>(radionuclide_name.size()),
                    radionuclide_name.data());
        ImGui::Text("Half-life: %.8g s",
                    static_cast<double>(definition->GetHalfLifeSeconds()));
        ImGui::Text("Emission groups: %u",
                    static_cast<unsigned int>(population.emission_count));
        ImGui::Text("Total yield: %.8g particles per decay",
                    static_cast<double>(definition->GetTotalYieldPerDecay()));
      } else {
        ImGui::Text("Particle: %.*s", static_cast<int>(particle_type.size()),
                    particle_type.data());
      }

      ImGui::Text("Primary count: %llu",
                  static_cast<unsigned long long>(range.primary_count));
      ImGui::Text(
        "Range: [%llu, %llu)",
        static_cast<unsigned long long>(range.projection_primary_begin),
        static_cast<unsigned long long>(projection_primary_end));

      ImGui::Separator();

      ImGui::Text("Emission: %s", emission_geometry.c_str());

      if (emission_geometry_type ==
          core::sources::GGEMSEmissionGeometryType::Rectangle) {
        std::string const size_x =
          units::HumanReadable(units::Length{record.geometry_size_x_pm}, 3);
        std::string const size_y =
          units::HumanReadable(units::Length{record.geometry_size_y_pm}, 3);

        ImGui::Text("Size: %s x %s", size_x.c_str(), size_y.c_str());
      } else if (emission_geometry_type ==
                 core::sources::GGEMSEmissionGeometryType::Ellipse) {
        std::string const diameter_x =
          units::HumanReadable(units::Length{record.geometry_size_x_pm}, 3);
        std::string const diameter_y =
          units::HumanReadable(units::Length{record.geometry_size_y_pm}, 3);

        ImGui::Text("Diameter: %s x %s", diameter_x.c_str(),
                    diameter_y.c_str());
      } else if (emission_geometry_type ==
                 core::sources::GGEMSEmissionGeometryType::Box) {
        std::string const width =
          units::HumanReadable(units::Length{record.geometry_size_x_pm}, 3);
        std::string const height =
          units::HumanReadable(units::Length{record.geometry_size_y_pm}, 3);
        std::string const depth =
          units::HumanReadable(units::Length{record.geometry_size_z_pm}, 3);

        ImGui::Text("Size: %s x %s x %s", width.c_str(), height.c_str(),
                    depth.c_str());
      } else if (emission_geometry_type ==
                 core::sources::GGEMSEmissionGeometryType::Sphere) {
        std::string const diameter =
          units::HumanReadable(units::Length{record.geometry_size_x_pm}, 3);
        ImGui::Text("Diameter: %s", diameter.c_str());
      } else if (emission_geometry_type ==
                 core::sources::GGEMSEmissionGeometryType::Cylinder) {
        std::string const diameter =
          units::HumanReadable(units::Length{record.geometry_size_x_pm}, 3);
        std::string const height =
          units::HumanReadable(units::Length{record.geometry_size_z_pm}, 3);
        ImGui::Text("Diameter: %s", diameter.c_str());
        ImGui::Text("Height: %s", height.c_str());
      }

      ImGui::Text("Angular: %s", angular_distribution.c_str());

      if (angular_distribution_type ==
          core::sources::GGEMSAngularDistributionType::Isotropic) {
        if (core::sources::IsDefaultFullSphereIsotropicDomain(record)) {
          ImGui::TextUnformatted("Domain: Full sphere");
        } else {
          long double const theta_min_rad = std::acos(std::clamp(
            static_cast<long double>(record.isotropic_cos_theta_upper), -1.0L,
            1.0L));
          long double const theta_max_rad = std::acos(std::clamp(
            static_cast<long double>(record.isotropic_cos_theta_lower), -1.0L,
            1.0L));
          std::string const theta_min =
            units::HumanReadable(units::MakeRadians(theta_min_rad), 3);
          std::string const theta_max =
            units::HumanReadable(units::MakeRadians(theta_max_rad), 3);
          std::string const phi_min =
            units::HumanReadable(units::MakeRadians(static_cast<long double>(
                                   record.isotropic_phi_min_rad)),
                                 3);
          std::string const phi_max =
            units::HumanReadable(units::MakeRadians(static_cast<long double>(
                                   record.isotropic_phi_max_rad)),
                                 3);

          ImGui::Text("Theta: %s to %s", theta_min.c_str(), theta_max.c_str());
          ImGui::Text("Phi: %s to %s", phi_min.c_str(), phi_max.c_str());
        }
      } else if (angular_distribution_type ==
                 core::sources::GGEMSAngularDistributionType::Focused) {
        std::string const focus_x =
          units::HumanReadableSignedLength(record.focus_position_x_pm, 3);
        std::string const focus_y =
          units::HumanReadableSignedLength(record.focus_position_y_pm, 3);
        std::string const focus_z =
          units::HumanReadableSignedLength(record.focus_position_z_pm, 3);

        ImGui::Text("Focus: (%s, %s, %s)", focus_x.c_str(), focus_y.c_str(),
                    focus_z.c_str());
      }

      ImGui::Separator();

      std::string const position_x =
        units::HumanReadableSignedLength(record.position_x_pm, 3);
      std::string const position_y =
        units::HumanReadableSignedLength(record.position_y_pm, 3);
      std::string const position_z =
        units::HumanReadableSignedLength(record.position_z_pm, 3);

      std::string const time_start =
        units::HumanReadable(units::Time{record.time_start_ps}, 3);
      std::string const time_stop =
        units::HumanReadable(units::Time{record.time_stop_ps}, 3);

      ImGui::Text("Position: (%s, %s, %s)", position_x.c_str(),
                  position_y.c_str(), position_z.c_str());
      ImGui::Text("Axis Z: (%.6g, %.6g, %.6g)",
                  static_cast<double>(record.axis_z_x),
                  static_cast<double>(record.axis_z_y),
                  static_cast<double>(record.axis_z_z));

      if (activity_driven) {
        BuildEmissionGroupEntries(*source_run_snapshot, population,
                                  *definition);
      } else {
        BuildEnergyLawSummary(energy_records[source_index], energy_values,
                              record.energy_micro_eV);
      }

      if (record.time_stop_ps <= record.time_start_ps) {
        ImGui::Text("Time: %s (fixed)", time_start.c_str());
      } else {
        ImGui::Text("Time window: [%s, %s)", time_start.c_str(),
                    time_stop.c_str());
      }

      ImGui::TreePop();
    }

    ImGui::PopID();
  }

  ImGui::PopID();
}

// =============================================================================
// =============================================================================

/*!
 * \brief Builds one selectable scene category and its available contents.
 *
 * \param[in,out] workbench Selected category and trace presentation controls.
 * \param[in] source_run_snapshot Borrowed source snapshot used by the source
 *   category.
 * \param[in] label Category display label and ImGui identity.
 * \param[in] selection Category associated with this node.
 * \param[in] extra_flags Additional tree-node presentation flags.
 */
auto BuildSceneNode(
  ggems::ui::detail::GGEMSWorkbenchState &workbench,
  ggems::core::sources::GGEMSSourceRunSnapshot const *source_run_snapshot,
  char const *label, SceneSelection selection,
  ImGuiTreeNodeFlags extra_flags = 0) -> void {
  ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow |
                             ImGuiTreeNodeFlags_SpanAvailWidth | extra_flags;

  if (workbench.selected_scene_item == selection) {
    flags |= ImGuiTreeNodeFlags_Selected;
  }

  bool const opened = ImGui::TreeNodeEx(label, flags);

  if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
    workbench.selected_scene_item = selection;
  }

  if (opened) {
    switch (selection) {
    case SceneSelection::World:
      ImGui::TextDisabled("World volume: not loaded yet");
      break;
    case SceneSelection::Sources:
      BuildSourceEntries(workbench, source_run_snapshot);
      break;
    case SceneSelection::Volumes:
      ImGui::TextDisabled("No geometry volume loaded yet");
      break;
    case SceneSelection::Materials:
      ImGui::TextDisabled("No material table loaded yet");
      break;
    case SceneSelection::Tracks: {
      ImGui::TextDisabled(
        "Particle traces can be submitted from observer records.");
      ImGui::TextDisabled("Camera pan: middle mouse or arrow keys.");
      ImGui::TextDisabled("Shift + arrows: faster, Ctrl + arrows: precise.");
      bool show_particle_traces = workbench.trace_visibility.IsGlobalVisible();
      if (ImGui::Checkbox("Show trajectories", &show_particle_traces)) {
        workbench.trace_visibility.SetGlobalVisible(show_particle_traces);
      }
      break;
    }
    case SceneSelection::None:
      break;
    }

    ImGui::TreePop();
  }
}

} // namespace

namespace ggems::ui::detail {

// =============================================================================
// =============================================================================

auto BuildStatusPanel(bool &show_window,
                      GGEMSDeviceStatusSnapshot const &device_status,
                      std::uint32_t swapchain_width,
                      std::uint32_t swapchain_height, bool show_axes,
                      bool particle_traces_visible) -> void {
  ImGui::Begin(k_status_window_name, &show_window);

  ImGui::TextUnformatted("GuiMode bootstrap");
  ImGui::Separator();

  ImGui::TextUnformatted("Renderer");
  ImGui::Separator();
  ImGui::Indent();

  if (!device_status.renderer.initialized) {
    ImGui::TextDisabled("Vulkan renderer: not initialized");
    ImGui::TextDisabled("Vulkan device: not initialized");
  } else {
    ImGui::TextUnformatted("Vulkan renderer: initialized");
    ImGui::Text(
      "Vulkan device: [%u] %s",
      static_cast<unsigned int>(device_status.renderer.enumeration_index),
      device_status.renderer.name.c_str());
    ImGui::Text("Device type: %s", device_status.renderer.type.c_str());
    ImGui::Text("Selection: %s",
                device_status.renderer.selection_reason.c_str());
  }

  ImGui::Text("Swapchain extent: %u x %u", swapchain_width, swapchain_height);

  ImGui::Unindent();
  ImGui::Separator();

  ImGui::TextUnformatted("Compute");
  ImGui::Separator();
  ImGui::Indent();

  if (!device_status.compute.initialized) {
    ImGui::TextDisabled("OpenCL devices: not initialized");
  } else {
    ImGui::Text("OpenCL devices: %zu", device_status.compute.devices.size());

    for (GGEMSComputeDeviceStatus const &device :
         device_status.compute.devices) {
      ImGui::Text("[%zu] %s - %s", device.context_index, device.name.c_str(),
                  device.type.c_str());

      if (device.show_platform && !device.platform.empty()) {
        ImGui::Indent();
        ImGui::TextDisabled("Platform: %s", device.platform.c_str());
        ImGui::Unindent();
      }
    }
  }

  ImGui::TextUnformatted("Scene renderer: connected");
  ImGui::Text("Axes: %s", show_axes ? "visible" : "hidden");
  ImGui::Text("Particle traces: %s",
              particle_traces_visible ? "visible" : "hidden");
  ImGui::TextUnformatted("GGEMSWorld: not loaded yet");
  ImGui::TextUnformatted("Output console: connected");

  ImGui::End();
}

// -----------------------------------------------------------------------------

auto BuildScenePanel(
  bool &show_window, GGEMSWorkbenchState &workbench,
  core::sources::GGEMSSourceRunSnapshot const *source_run_snapshot) -> void {
  ImGui::Begin(k_scene_window_name, &show_window);

  ImGui::TextUnformatted("Scene hierarchy");
  ImGui::Separator();

  BuildSceneNode(workbench, source_run_snapshot, "GGEMSWorld",
                 SceneSelection::World, ImGuiTreeNodeFlags_DefaultOpen);

  BuildSceneNode(workbench, source_run_snapshot, "Sources",
                 SceneSelection::Sources, ImGuiTreeNodeFlags_DefaultOpen);

  BuildSceneNode(workbench, source_run_snapshot, "Volumes",
                 SceneSelection::Volumes, ImGuiTreeNodeFlags_DefaultOpen);

  BuildSceneNode(workbench, source_run_snapshot, "Materials",
                 SceneSelection::Materials);

  BuildSceneNode(workbench, source_run_snapshot, "Tracks / Replay",
                 SceneSelection::Tracks);

  ImGui::End();
}

// -----------------------------------------------------------------------------

auto BuildInspectorPanel(bool &show_window,
                         GGEMSWorkbenchState::SceneSelection selection)
  -> void {
  ImGui::Begin(k_inspector_window_name, &show_window);

  ImGui::TextUnformatted("Selection");
  ImGui::Separator();

  if (selection == SceneSelection::None) {
    ImGui::TextDisabled("No GGEMS object selected yet.");
  } else {
    ImGui::Text("Selected: %s", GetSceneSelectionName(selection));
  }

  ImGui::Spacing();

  if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::TextUnformatted("Position: not available yet");
    ImGui::TextUnformatted("Rotation: not available yet");
    ImGui::TextUnformatted("Scale:    not available yet");
  }

  if (ImGui::CollapsingHeader("Geometry", ImGuiTreeNodeFlags_DefaultOpen)) {
    switch (selection) {
    case SceneSelection::World:
      ImGui::TextUnformatted("Type: GGEMSWorld boundary");
      ImGui::TextUnformatted("Bounds: not loaded yet");
      break;
    case SceneSelection::Volumes:
      ImGui::TextUnformatted("Type: geometry collection");
      ImGui::TextUnformatted("Bounds: not available yet");
      break;
    default:
      ImGui::TextUnformatted("Type: not available yet");
      ImGui::TextUnformatted("Bounds: not available yet");
      break;
    }
  }

  if (ImGui::CollapsingHeader("Material")) {
    ImGui::TextUnformatted("Material: not available yet");
    ImGui::TextUnformatted("Density:  not available yet");
  }

  if (ImGui::CollapsingHeader("Physics")) {
    ImGui::TextUnformatted("Processes: not available yet");
    ImGui::TextUnformatted("Cross sections: not available yet");
  }

  ImGui::End();
}

} // namespace ggems::ui::detail
