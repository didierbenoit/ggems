#pragma once

#include <vulkan/vulkan.hpp>

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
  void BuildMainDockspace();
  void BuildMainMenuBar();
  void BuildStatusPanel(vk::Extent2D const &swapchain_extent);
  void BuildViewportPlaceholder();

private:
  GGEMSImGuiOutputPanel output_panel_{};
  bool show_output_panel_{true};
  bool show_status_panel_{true};
  bool show_viewport_placeholder_{true};
};
} // namespace ggems::ui
