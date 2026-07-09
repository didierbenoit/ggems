#pragma once

#include <vulkan/vulkan.hpp>
#include <imgui.h>

#include "GGEMSImGuiOutputPanel.hh"

namespace ggems::ui {
class GGEMSImGuiLayer {
public:
  struct ViewportState {
    vk::Extent2D extent{};
    bool visible{false};
    bool hovered{false};
    bool focused{false};
    float orbit_delta_x_pixels{0.0f};
    float orbit_delta_y_pixels{0.0f};
    float zoom_delta{0.0f};
  };

public:
  GGEMSImGuiLayer() = default;
  ~GGEMSImGuiLayer() = default;

  GGEMSImGuiLayer(GGEMSImGuiLayer const &) = delete;
  GGEMSImGuiLayer(GGEMSImGuiLayer &&) = delete;
  GGEMSImGuiLayer &operator=(GGEMSImGuiLayer const &) = delete;
  GGEMSImGuiLayer &operator=(GGEMSImGuiLayer &&) = delete;

public:
  void BuildFrame(vk::Extent2D const &swapchain_extent,
                  ImTextureID scene_texture_id,
                  vk::Extent2D const &scene_texture_extent);

  [[nodiscard]] ViewportState const &GetViewportState() const noexcept;
  [[nodiscard]] bool ShouldShowAxes() const noexcept;
  [[nodiscard]] bool ShouldShowParticleTraces() const noexcept;
  [[nodiscard]] bool ShouldResetCamera() const noexcept;

private:
  enum class SceneSelection {
    None,
    World,
    Sources,
    Volumes,
    Materials,
    Particles,
    Processes,
    Tracks
  };

private:
  void BuildMainDockspace();
  void BuildMainMenuBar();
  void BuildDefaultDockspaceLayout(ImGuiID dockspace_id,
                                   ImVec2 const &dockspace_size);
  void BuildStatusPanel(vk::Extent2D const &swapchain_extent);
  void BuildViewportPlaceholder(ImTextureID scene_texture_id,
                                vk::Extent2D const &scene_texture_extent);
  void BuildInspectorPanel();
  void BuildScenePanel();

  [[nodiscard]] char const *GetSceneSelectionName() const noexcept;
  void BuildSceneNode(char const *label, SceneSelection selection,
                      ImGuiTreeNodeFlags extra_flags = 0);

private:
  GGEMSImGuiOutputPanel output_panel_{};
  bool show_output_panel_{true};
  bool show_status_panel_{true};
  bool show_viewport_placeholder_{true};
  bool show_inspector_panel_{true};
  bool dockspace_layout_built_{false};
  bool show_scene_panel_{true};
  bool show_axes_{true};
  bool show_particle_traces_{true};
  bool show_step_points_{false};
  bool show_interaction_points_{false};
  SceneSelection selected_scene_item_{SceneSelection::World};
  ViewportState viewport_state_{};
  bool reset_camera_requested_{false};
};
} // namespace ggems::ui
