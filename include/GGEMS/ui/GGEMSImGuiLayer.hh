#pragma once

#include <cstdint>
#include <optional>

#include <vulkan/vulkan.hpp>
#include <imgui.h>

#include "GGEMSImGuiOutputPanel.hh"
#include "GGEMSDeviceStatus.hh"
#include "GGEMS/sources/GGEMSSourceRunSnapshot.hh"
#include "GGEMS/render/GGEMSParticleTrace.hh"

namespace ggems::ui {
class GGEMSImGuiLayer {
public:
  struct ViewportState {
    vk::Extent2D extent{};
    bool visible{false};
    bool hovered{false};
    bool focused{false};
    float orbit_delta_x_pixels{0.0F};
    float orbit_delta_y_pixels{0.0F};
    float pan_delta_x_pixels{0.0F};
    float pan_delta_y_pixels{0.0F};
    float zoom_delta{0.0F};
  };

  GGEMSImGuiLayer() = default;
  ~GGEMSImGuiLayer() = default;

  GGEMSImGuiLayer(GGEMSImGuiLayer const &) = delete;
  GGEMSImGuiLayer(GGEMSImGuiLayer &&) = delete;
  auto operator=(GGEMSImGuiLayer const &) -> GGEMSImGuiLayer & = delete;
  auto operator=(GGEMSImGuiLayer &&) -> GGEMSImGuiLayer & = delete;

  auto BuildFrame(vk::Extent2D const &swapchain_extent,
                  ImTextureID scene_texture_id,
                  vk::Extent2D const &scene_texture_extent,
                  detail::GGEMSDeviceStatusSnapshot const &device_status)
      -> void;

  auto SetSourceRunSnapshot(core::sources::GGEMSSourceRunSnapshot snapshot)
      -> void;

  [[nodiscard]] auto GetViewportState() const noexcept -> ViewportState const &;
  [[nodiscard]] auto ShouldShowAxes() const noexcept -> bool;
  [[nodiscard]] auto ShouldShowParticleTraces() const noexcept -> bool;
  [[nodiscard]] auto ShouldResetCamera() const noexcept -> bool;
  [[nodiscard]] auto GetParticleTraceVisibility() const noexcept
      -> render::GGEMSParticleTraceVisibility const &;

private:
  enum class SceneSelection : std::uint8_t {
    None,
    World,
    Sources,
    Volumes,
    Materials,
    Particles,
    Processes,
    Tracks
  };

  auto BuildMainDockspace() -> void;
  auto BuildMainMenuBar() -> void;
  static auto BuildDefaultDockspaceLayout(ImGuiID dockspace_id,
                                          ImVec2 const &dockspace_size) -> void;
  auto BuildStatusPanel(vk::Extent2D const &swapchain_extent,
                        detail::GGEMSDeviceStatusSnapshot const &device_status)
      -> void;

  auto BuildViewportPlaceholder(ImTextureID scene_texture_id,
                                vk::Extent2D const &scene_texture_extent)
      -> void;
  auto BuildInspectorPanel() -> void;
  auto BuildScenePanel() -> void;

  auto BuildSourceEntries() -> void;
  [[nodiscard]] auto GetSceneSelectionName() const noexcept -> char const *;
  auto BuildSceneNode(char const *label, SceneSelection selection,
                      ImGuiTreeNodeFlags extra_flags = 0) -> void;

  GGEMSImGuiOutputPanel output_panel_;
  bool show_output_panel_{true};
  bool show_status_panel_{true};
  bool show_viewport_placeholder_{true};
  bool show_inspector_panel_{true};
  bool dockspace_layout_built_{false};
  bool show_scene_panel_{true};
  bool show_axes_{true};
  bool show_step_points_{false};
  bool show_interaction_points_{false};
  std::optional<core::sources::GGEMSSourceRunSnapshot> source_run_snapshot_;
  render::GGEMSParticleTraceVisibility particle_trace_visibility_{};
  SceneSelection selected_scene_item_{SceneSelection::World};
  ViewportState viewport_state_{};
  bool reset_camera_requested_{false};
};
} // namespace ggems::ui
