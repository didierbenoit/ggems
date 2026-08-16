#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <algorithm>
#include <cmath>

#include <vulkan/vulkan.hpp>

#include <imgui.h>
#include <imgui_internal.h>

#include "GGEMS/ui/GGEMSImGuiLayer.hh"
#include "GGEMS/ui/GGEMSDeviceStatus.hh"
#include "GGEMS/core/GGEMSOutputState.hh"
#include "GGEMS/core/GGEMSOutputMode.hh"
#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/sources/GGEMSSourceTypes.hh"
#include "GGEMS/core/sources/GGEMSEnergyDistributionRecord.hh"
#include "GGEMS/core/sources/GGEMSSourceValidation.hh"
#include "GGEMS/core/sources/GGEMSSourceRunSnapshot.hh"
#include "GGEMS/render/GGEMSParticleTrace.hh"
#include "GGEMS/core/units/GGEMSLengthUnits.hh"
#include "GGEMS/core/units/GGEMSEnergyUnits.hh"
#include "GGEMS/core/units/GGEMSTimeUnits.hh"
#include "GGEMS/core/units/GGEMSAngularUnits.hh"
#include "GGEMS/core/units/GGEMSUnitFormatting.hh"

namespace ggems::ui {

// =============================================================================
// =============================================================================

auto GGEMSImGuiLayer::GetSceneSelectionName() const noexcept -> char const * {
  switch (selected_scene_item_) {
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
  case SceneSelection::Particles:
    return "Particles";
  case SceneSelection::Processes:
    return "Processes";
  }

  return "Unknown";
}

// =============================================================================
// =============================================================================

auto GGEMSImGuiLayer::BuildFrame(
    vk::Extent2D const &swapchain_extent, ImTextureID scene_texture_id,
    vk::Extent2D const &scene_texture_extent,
    detail::GGEMSDeviceStatusSnapshot const &device_status) -> void {
  reset_camera_requested_ = false;

  BuildMainDockspace();

  if (show_output_panel_) {
    core::GGEMSOutputState &output_state = core::GetOutputState();
    output_panel_.Render(output_state);
  }

  if (show_status_panel_) {
    BuildStatusPanel(swapchain_extent, device_status);
  }

  if (show_scene_panel_) {
    BuildScenePanel();
  }

  if (show_viewport_placeholder_) {
    BuildViewportPlaceholder(scene_texture_id, scene_texture_extent);
  }

  if (show_inspector_panel_) {
    BuildInspectorPanel();
  }
}

// -----------------------------------------------------------------------------

auto GGEMSImGuiLayer::SetSourceRunSnapshot(
    core::sources::GGEMSSourceRunSnapshot snapshot) -> void {
  particle_trace_visibility_.ReconcileSourceCount(snapshot.GetRecords().size());
  source_run_snapshot_ = std::move(snapshot);
}

// -----------------------------------------------------------------------------

auto GGEMSImGuiLayer::ShouldShowAxes() const noexcept -> bool {
  return show_axes_;
}

// -----------------------------------------------------------------------------

auto GGEMSImGuiLayer::ShouldShowParticleTraces() const noexcept -> bool {
  return particle_trace_visibility_.IsGlobalVisible();
}

// -----------------------------------------------------------------------------

auto GGEMSImGuiLayer::ShouldResetCamera() const noexcept -> bool {
  return reset_camera_requested_;
}

// -----------------------------------------------------------------------------

auto GGEMSImGuiLayer::GetViewportState() const noexcept
    -> GGEMSImGuiLayer::ViewportState const & {
  return viewport_state_;
}

// -----------------------------------------------------------------------------

auto GGEMSImGuiLayer::BuildMainDockspace() -> void {
  ImGuiViewport const *viewport = ImGui::GetMainViewport();

  ImGui::SetNextWindowPos(viewport->WorkPos);
  ImGui::SetNextWindowSize(viewport->WorkSize);
  ImGui::SetNextWindowViewport(viewport->ID);

  ImGuiWindowFlags const window_flags =
      ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking |
      ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
      ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus |
      ImGuiWindowFlags_NoBackground;

  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0F);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0F);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0.0F, 0.0F});

  bool dockspace_open{true};
  ImGui::Begin("GGEMS Main Dockspace", &dockspace_open, window_flags);

  ImGui::PopStyleVar(3);

  BuildMainMenuBar();

  ImGuiID dockspace_id = ImGui::GetID("GGEMS_Dockspace");

  ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

  ImGui::DockSpace(dockspace_id, ImVec2{0.0F, 0.0F}, dockspace_flags);

  if (!dockspace_layout_built_) {
    BuildDefaultDockspaceLayout(dockspace_id, viewport->WorkSize);
    dockspace_layout_built_ = true;
  }

  ImGui::End();
}

// -----------------------------------------------------------------------------

auto GGEMSImGuiLayer::GetParticleTraceVisibility() const noexcept
    -> render::GGEMSParticleTraceVisibility const & {
  return particle_trace_visibility_;
}

// -----------------------------------------------------------------------------

auto GGEMSImGuiLayer::BuildMainMenuBar() -> void {
  if (!ImGui::BeginMainMenuBar()) {
    return;
  }

  if (ImGui::BeginMenu("File")) {
    ImGui::MenuItem("Open result...", nullptr, false, false);
    ImGui::MenuItem("Save session...", nullptr, false, false);
    ImGui::Separator();
    ImGui::MenuItem("Exit", nullptr, false, false);
    ImGui::EndMenu();
  }

  if (ImGui::BeginMenu("View")) {
    ImGui::MenuItem("Output", nullptr, &show_output_panel_);
    ImGui::MenuItem("Status", nullptr, &show_status_panel_);
    ImGui::MenuItem("Scene", nullptr, &show_scene_panel_);
    ImGui::MenuItem("Viewport", nullptr, &show_viewport_placeholder_);
    ImGui::MenuItem("Inspector", nullptr, &show_inspector_panel_);

    ImGui::Separator();
    ImGui::MenuItem("Axes", nullptr, &show_axes_);

    bool show_particle_traces = particle_trace_visibility_.IsGlobalVisible();
    if (ImGui::MenuItem("Particle traces", nullptr, &show_particle_traces)) {
      particle_trace_visibility_.SetGlobalVisible(show_particle_traces);
    }

    ImGui::MenuItem("Step points", nullptr, &show_step_points_, false);
    ImGui::MenuItem("Interaction points", nullptr, &show_interaction_points_,
                    false);

    if (ImGui::MenuItem("Reset Camera")) {
      reset_camera_requested_ = true;
    }

    ImGui::EndMenu();
  }

  if (ImGui::BeginMenu("Simulation")) {
    ImGui::MenuItem("Start", nullptr, false, false);
    ImGui::MenuItem("Pause", nullptr, false, false);
    ImGui::MenuItem("Stop", nullptr, false, false);
    ImGui::EndMenu();
  }

  if (ImGui::BeginMenu("Debug")) {
    ImGui::MenuItem("Particle tracking", nullptr, false, false);
    ImGui::MenuItem("Geometry inspection", nullptr, false, false);
    ImGui::EndMenu();
  }

  if (ImGui::BeginMenu("Help")) {
    ImGui::MenuItem("About GGEMS", nullptr, false, false);
    ImGui::EndMenu();
  }

  ImGui::EndMainMenuBar();
}

// -----------------------------------------------------------------------------

auto GGEMSImGuiLayer::BuildStatusPanel(
    vk::Extent2D const &swapchain_extent,
    detail::GGEMSDeviceStatusSnapshot const &device_status) -> void {
  ImGui::Begin("GGEMS Status", &show_status_panel_);

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

  ImGui::Text("Swapchain extent: %u x %u", swapchain_extent.width,
              swapchain_extent.height);

  ImGui::Unindent();
  ImGui::Separator();

  ImGui::TextUnformatted("Compute");
  ImGui::Separator();
  ImGui::Indent();

  if (!device_status.compute.initialized) {
    ImGui::TextDisabled("OpenCL devices: not initialized");
  } else {
    ImGui::Text("OpenCL devices: %zu", device_status.compute.devices.size());

    for (detail::GGEMSComputeDeviceStatus const &device :
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
  ImGui::Text("Axes: %s", show_axes_ ? "visible" : "hidden");
  ImGui::Text("Particle traces: %s",
              particle_trace_visibility_.IsGlobalVisible() ? "visible"
                                                           : "hidden");
  ImGui::TextUnformatted("GGEMSWorld: not loaded yet");
  ImGui::TextUnformatted("Output console: connected");

  ImGui::End();
}

// -----------------------------------------------------------------------------

auto GGEMSImGuiLayer::BuildViewportPlaceholder(
    ImTextureID scene_texture_id, vk::Extent2D const &scene_texture_extent)
    -> void {
  ImGuiWindowFlags window_flags =
      ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

  viewport_state_.orbit_delta_x_pixels = 0.0F;
  viewport_state_.orbit_delta_y_pixels = 0.0F;
  viewport_state_.pan_delta_x_pixels = 0.0F;
  viewport_state_.pan_delta_y_pixels = 0.0F;
  viewport_state_.zoom_delta = 0.0F;
  viewport_state_.hovered = false;
  viewport_state_.focused = false;

  ImGui::Begin("GGEMS Viewport", &show_viewport_placeholder_, window_flags);

  ImVec2 available_size = ImGui::GetContentRegionAvail();

  std::uint32_t width = available_size.x > 1.0F
                            ? static_cast<std::uint32_t>(available_size.x)
                            : 1U;

  std::uint32_t height = available_size.y > 1.0F
                             ? static_cast<std::uint32_t>(available_size.y)
                             : 1U;

  bool texture_matches_viewport = scene_texture_id != ImTextureID{} &&
                                  scene_texture_extent.width == width &&
                                  scene_texture_extent.height == height;

  viewport_state_.extent = vk::Extent2D{.width = width, .height = height};
  viewport_state_.visible = show_viewport_placeholder_;
  viewport_state_.focused =
      ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

  if (texture_matches_viewport) {
    ImGui::Image(scene_texture_id,
                 ImVec2{static_cast<float>(width), static_cast<float>(height)});

    viewport_state_.hovered = ImGui::IsItemHovered();

    ImGuiIO const &imgui_io = ImGui::GetIO();

    if (viewport_state_.hovered) {

      if (ImGui::IsMouseDragging(ImGuiMouseButton_Left, 0.0F)) {
        viewport_state_.orbit_delta_x_pixels = imgui_io.MouseDelta.x;
        viewport_state_.orbit_delta_y_pixels = imgui_io.MouseDelta.y;
      }

      if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle, 0.0F)) {
        viewport_state_.pan_delta_x_pixels = imgui_io.MouseDelta.x;
        viewport_state_.pan_delta_y_pixels = imgui_io.MouseDelta.y;
      }

      if (imgui_io.MouseWheel != 0.0F) {
        viewport_state_.zoom_delta = imgui_io.MouseWheel;
      }

      bool allow_keyboard_pan =
          (viewport_state_.hovered || viewport_state_.focused) &&
          !imgui_io.WantTextInput;

      if (allow_keyboard_pan) {
        float keyboard_pan_speed_pixels_per_second = 360.0F;

        if (imgui_io.KeyShift) {
          keyboard_pan_speed_pixels_per_second *= 3.0F;
        }

        if (imgui_io.KeyCtrl) {
          keyboard_pan_speed_pixels_per_second *= 0.25F;
        }

        float keyboard_pan_delta_pixels =
            keyboard_pan_speed_pixels_per_second * imgui_io.DeltaTime;

        if (ImGui::IsKeyDown(ImGuiKey_LeftArrow)) {
          viewport_state_.pan_delta_x_pixels -= keyboard_pan_delta_pixels;
        }

        if (ImGui::IsKeyDown(ImGuiKey_RightArrow)) {
          viewport_state_.pan_delta_x_pixels += keyboard_pan_delta_pixels;
        }

        if (ImGui::IsKeyDown(ImGuiKey_UpArrow)) {
          viewport_state_.pan_delta_y_pixels -= keyboard_pan_delta_pixels;
        }

        if (ImGui::IsKeyDown(ImGuiKey_DownArrow)) {
          viewport_state_.pan_delta_y_pixels += keyboard_pan_delta_pixels;
        }
      }
    }
  } else {
    ImGui::TextDisabled("Vulkan scene renderer: preparing render target...");
    ImGui::TextDisabled("Viewport extent: %u x %u", width, height);

    viewport_state_.hovered =
        ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows);
  }

  ImGui::End();
}

// -----------------------------------------------------------------------------

auto GGEMSImGuiLayer::BuildDefaultDockspaceLayout(ImGuiID dockspace_id,
                                                  ImVec2 const &dockspace_size)
    -> void {
  ImGui::DockBuilderRemoveNode(dockspace_id);

  ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
  ImGui::DockBuilderSetNodeSize(dockspace_id, dockspace_size);

  ImGuiID dockspace_main_id = dockspace_id;

  ImGuiID dock_bottom_id = ImGui::DockBuilderSplitNode(
      dockspace_main_id, ImGuiDir_Down, 0.30F, nullptr, &dockspace_main_id);

  ImGuiID dock_left_id = ImGui::DockBuilderSplitNode(
      dockspace_main_id, ImGuiDir_Left, 0.24F, nullptr, &dockspace_main_id);

  ImGuiID dock_right_id = ImGui::DockBuilderSplitNode(
      dockspace_main_id, ImGuiDir_Right, 0.26F, nullptr, &dockspace_main_id);

  ImGui::DockBuilderDockWindow("GGEMS Output", dock_bottom_id);
  ImGui::DockBuilderDockWindow("GGEMS Status", dock_left_id);
  ImGui::DockBuilderDockWindow("GGEMS Scene", dock_left_id);
  ImGui::DockBuilderDockWindow("GGEMS Viewport", dockspace_main_id);
  ImGui::DockBuilderDockWindow("GGEMS Inspector", dock_right_id);

  ImGui::DockBuilderFinish(dockspace_id);
}

// -----------------------------------------------------------------------------

auto GGEMSImGuiLayer::BuildInspectorPanel() -> void {
  ImGui::Begin("GGEMS Inspector", &show_inspector_panel_);

  ImGui::TextUnformatted("Selection");
  ImGui::Separator();

  if (selected_scene_item_ == SceneSelection::None) {
    ImGui::TextDisabled("No GGEMS object selected yet.");
  } else {
    ImGui::Text("Selected: %s", GetSceneSelectionName());
  }

  ImGui::Spacing();

  if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
    if (selected_scene_item_ == SceneSelection::World) {
      ImGui::TextUnformatted("Position: world origin");
      ImGui::TextUnformatted("Rotation: identity");
      ImGui::TextUnformatted("Scale:    1.0");
    } else {
      ImGui::TextUnformatted("Position: not available yet");
      ImGui::TextUnformatted("Rotation: not available yet");
      ImGui::TextUnformatted("Scale:    not available yet");
    }
  }

  if (ImGui::CollapsingHeader("Geometry", ImGuiTreeNodeFlags_DefaultOpen)) {
    switch (selected_scene_item_) {
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

// -----------------------------------------------------------------------------

auto GGEMSImGuiLayer::BuildScenePanel() -> void {
  ImGui::Begin("GGEMS Scene", &show_scene_panel_);

  ImGui::TextUnformatted("Scene hierarchy");
  ImGui::Separator();

  BuildSceneNode("GGEMSWorld", SceneSelection::World,
                 ImGuiTreeNodeFlags_DefaultOpen);

  BuildSceneNode("Sources", SceneSelection::Sources,
                 ImGuiTreeNodeFlags_DefaultOpen);

  BuildSceneNode("Volumes", SceneSelection::Volumes,
                 ImGuiTreeNodeFlags_DefaultOpen);

  BuildSceneNode("Materials", SceneSelection::Materials);

  BuildSceneNode("Tracks / Replay", SceneSelection::Tracks);

  ImGui::End();
}

// -----------------------------------------------------------------------------

auto GGEMSImGuiLayer::BuildSourceEntries() -> void {
  if (!source_run_snapshot_.has_value()) {
    ImGui::TextDisabled("No completed GGEMSRun source snapshot submitted yet");
    return;
  }

  auto const &records = source_run_snapshot_->GetRecords();
  auto const &ranges = source_run_snapshot_->GetRanges();
  auto const &energy_records =
      source_run_snapshot_->GetEnergyDistributionRecords();
  auto const &energy_values =
      source_run_snapshot_->GetEnergyValuesMilliElectronVolt();

  if (records.size() != ranges.size() ||
      records.size() != energy_records.size()) {
    ImGui::TextDisabled("Invalid source snapshot");
    return;
  }

  for (std::size_t source_index = 0U; source_index < records.size();
       ++source_index) {
    auto const &record = records[source_index];
    auto const &range = ranges[source_index];
    auto const &energy_record = energy_records[source_index];

    std::string const source_type{core::sources::ToLongName(
        core::sources::FromKernelSourceType(record.source_type))};

    std::string const particle_type = core::particles::ToLongName(
        core::particles::FromKernelParticleType(record.emitted_particle_type));

    auto const emission_geometry_type =
        core::sources::FromKernelEmissionGeometryType(
            record.emission_geometry_type);

    auto const angular_distribution_type =
        core::sources::FromKernelAngularDistributionType(
            record.angular_distribution_type);

    auto const energy_distribution_type =
        core::sources::FromKernelEnergyDistributionType(
            energy_record.distribution_type);
    std::string const energy_distribution{
        core::sources::ToLongName(energy_distribution_type)};

    std::string const emission_geometry{
        core::sources::ToLongName(emission_geometry_type)};

    std::string const angular_distribution{
        core::sources::ToLongName(angular_distribution_type)};

    std::uint64_t const projection_primary_end =
        range.projection_primary_begin + range.primary_count;

    ImGui::PushID(static_cast<void const *>(&record));

    bool source_visible = particle_trace_visibility_.IsSourceVisible(
        static_cast<std::uint32_t>(source_index));

    if (ImGui::Checkbox("##trajectory_visibility", &source_visible)) {
      particle_trace_visibility_.SetSourceVisible(source_index, source_visible);
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
        "##source_details", flags, "Source %zu - %s - %s", source_index,
        source_type.c_str(), particle_type.c_str());

    if (opened) {
      ImGui::Text("Source index: %zu", source_index);
      ImGui::Text("Type: %s", source_type.c_str());
      ImGui::Text("Particle: %s", particle_type.c_str());
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

      ImGui::Text("Energy distribution: %s", energy_distribution.c_str());

      if (energy_distribution_type ==
          core::sources::GGEMSEnergyDistributionType::Mono) {
        std::string const energy =
            units::HumanReadable(units::Energy{record.energy_milli_eV}, 3);
        ImGui::Text("Energy: %s", energy.c_str());
      } else {
        bool const table_offset_fits =
            energy_record.table_offset <=
            static_cast<std::uint64_t>(energy_values.size());
        std::size_t const table_offset =
            table_offset_fits
                ? static_cast<std::size_t>(energy_record.table_offset)
                : 0U;
        auto const table_count =
            static_cast<std::size_t>(energy_record.table_count);
        bool const valid_table =
            table_offset_fits && table_count >= 2U &&
            table_count <= energy_values.size() - table_offset;

        if (!valid_table) {
          ImGui::TextDisabled("Invalid energy table metadata");
        } else {
          std::string const first = units::HumanReadable(
              units::Energy{energy_values[table_offset]}, 3);
          std::string const last = units::HumanReadable(
              units::Energy{energy_values[table_offset + table_count - 1U]}, 3);

          if (energy_distribution_type ==
              core::sources::GGEMSEnergyDistributionType::DiscreteLines) {
            ImGui::Text("Line count: %zu", table_count);
            ImGui::Text("Line-energy range: %s - %s", first.c_str(),
                        last.c_str());
          } else if (energy_distribution_type ==
                     core::sources::GGEMSEnergyDistributionType::
                         RegularSpectrum) {
            std::string const width = units::HumanReadable(
                units::Energy{energy_record.regular_bin_width_milli_eV}, 3);
            ImGui::Text("Bin count: %zu", table_count);
            ImGui::Text("Center range: %s - %s", first.c_str(), last.c_str());
            ImGui::Text("Bin width: %s", width.c_str());
          } else {
            ImGui::TextDisabled("Unknown energy distribution");
          }
        }
      }

      if (record.time_stop_ps <= record.time_start_ps) {
        ImGui::Text("Time: %s (fixed)", time_start.c_str());
      } else {
        ImGui::Text("Time window: [%s, %s)", time_start.c_str(),
                    time_stop.c_str());
      }

      ImGui::Text("Weight: %.9g", static_cast<double>(record.weight));
      ImGui::TreePop();
    }

    ImGui::PopID();
  }
}

// -----------------------------------------------------------------------------

auto GGEMSImGuiLayer::BuildSceneNode(char const *label,
                                     SceneSelection selection,
                                     ImGuiTreeNodeFlags extra_flags) -> void {
  ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow |
                             ImGuiTreeNodeFlags_SpanAvailWidth | extra_flags;

  if (selected_scene_item_ == selection) {
    flags |= ImGuiTreeNodeFlags_Selected;
  }

  bool const opened = ImGui::TreeNodeEx(label, flags);

  if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
    selected_scene_item_ = selection;
  }

  if (opened) {
    switch (selection) {
    case SceneSelection::World:
      ImGui::TextDisabled("World volume: not loaded yet");
      break;
    case SceneSelection::Sources:
      BuildSourceEntries();
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
      bool show_particle_traces = particle_trace_visibility_.IsGlobalVisible();
      if (ImGui::Checkbox("Show trajectories", &show_particle_traces)) {
        particle_trace_visibility_.SetGlobalVisible(show_particle_traces);
      }
      ImGui::Checkbox("Show step points", &show_step_points_);
      ImGui::SameLine();
      ImGui::TextDisabled("soon");
      ImGui::Checkbox("Show interaction points", &show_interaction_points_);
      ImGui::SameLine();
      ImGui::TextDisabled("soon");
      break;
    }
    case SceneSelection::Particles:
      ImGui::TextDisabled("No particle loaded yet");
      break;
    case SceneSelection::Processes:
      ImGui::TextDisabled("No process loaded yet");
      break;
    case SceneSelection::None:
      break;
    }

    ImGui::TreePop();
  }
}

} // namespace ggems::ui
