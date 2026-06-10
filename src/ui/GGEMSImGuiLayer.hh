#pragma once

#include <vulkan/vulkan.hpp>
#include <imgui.h>

#include "GGEMSImGuiOutputPanel.hh"

namespace ggems::ui {
class GGEMSImGuiLayer {
public:
  GGEMSImGuiLayer() = default;
  ~GGEMSImGuiLayer() = default;

  GGEMSImGuiLayer(GGEMSImGuiLayer const &) = delete;
  GGEMSImGuiLayer(GGEMSImGuiLayer &&) = delete;
  GGEMSImGuiLayer &operator=(GGEMSImGuiLayer const &) = delete;
  GGEMSImGuiLayer &operator=(GGEMSImGuiLayer &&) = delete;

public:
  void BuildFrame(vk::Extent2D const &swapchain_extent);

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
  void BuildViewportPlaceholder();
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
  SceneSelection selected_scene_item_{SceneSelection::World};
};
} // namespace ggems::ui
