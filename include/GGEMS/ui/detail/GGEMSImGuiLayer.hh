#pragma once

#include <cstdint>

#include <vulkan/vulkan.hpp>
#include <imgui.h>

#include "GGEMSDeviceStatus.hh"
#include "GGEMSImGuiOutputPanel.hh"
#include "GGEMSImGuiViewportPanel.hh"
#include "GGEMSWorkbenchState.hh"
#include "GGEMS/logging/GGEMSOutputState.hh"
#include "GGEMS/sources/GGEMSSourceRunSnapshot.hh"

namespace ggems::ui {
class GGEMSImGuiLayer {
public:
  struct FrameInputs {
    ImTextureID scene_texture_id{};
    std::uint32_t scene_texture_width{0U};
    std::uint32_t scene_texture_height{0U};
    std::uint32_t swapchain_width{0U};
    std::uint32_t swapchain_height{0U};
    core::sources::GGEMSSourceRunSnapshot const *source_run_snapshot{nullptr};
    core::GGEMSOutputState *output_state{nullptr};
  };

  GGEMSImGuiLayer() = default;
  ~GGEMSImGuiLayer() = default;

  GGEMSImGuiLayer(GGEMSImGuiLayer const &) = delete;
  GGEMSImGuiLayer(GGEMSImGuiLayer &&) = delete;
  auto operator=(GGEMSImGuiLayer const &) -> GGEMSImGuiLayer & = delete;
  auto operator=(GGEMSImGuiLayer &&) -> GGEMSImGuiLayer & = delete;

  auto BuildFrame(FrameInputs const &inputs,
                  detail::GGEMSDeviceStatusSnapshot const &device_status,
                  detail::GGEMSWorkbenchState &workbench) -> void;

private:
  [[nodiscard]] auto BuildMainDockspace(detail::GGEMSWorkbenchState &workbench)
    -> bool;
  [[nodiscard]] static auto
  BuildMainMenuBar(detail::GGEMSWorkbenchState &workbench) -> bool;
  static auto BuildDefaultDockspaceLayout(ImGuiID dockspace_id,
                                          ImVec2 const &dockspace_size) -> void;

  GGEMSImGuiOutputPanel output_panel_;
  detail::GGEMSImGuiViewportPanel viewport_panel_;
  bool dockspace_layout_built_{false};
};
} // namespace ggems::ui
