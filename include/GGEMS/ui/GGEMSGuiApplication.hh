#pragma once

#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "GGEMS/render/GGEMSParticleTrace.hh"
#include "GGEMS/sources/GGEMSSourceRunSnapshot.hh"
#include "GGEMS/ui/detail/GGEMSDeviceStatus.hh"
#include "GGEMS/ui/detail/GGEMSWorkbenchState.hh"

struct GLFWwindow;

namespace ggems::core {
class GGEMSRun;
}

namespace ggems::core::observer {
class GGEMSTransportObserver;
}

namespace ggems::ui {

class GGEMSImGuiLayer;
class GGEMSVulkanSceneRenderer;

namespace detail {
class GGEMSImGuiIntegration;
class GGEMSVulkanDevice;
class GGEMSVulkanPresenter;
} // namespace detail

class GGEMSGuiApplication {
public:
  explicit GGEMSGuiApplication(std::string title = "GGEMS GuiMode",
                               std::int32_t width = 1600,
                               std::int32_t height = 900);
  ~GGEMSGuiApplication() noexcept;

  GGEMSGuiApplication(GGEMSGuiApplication const &) = delete;
  GGEMSGuiApplication(GGEMSGuiApplication &&) = delete;
  auto operator=(GGEMSGuiApplication const &) -> GGEMSGuiApplication & = delete;
  auto operator=(GGEMSGuiApplication &&) -> GGEMSGuiApplication & = delete;

  void Initialize();
  void Run();

  auto SubmitLastRunSourceSnapshot(ggems::core::GGEMSRun const &run) -> void;

  void SubmitParticleTraceSegments(
    std::vector<ggems::render::GGEMSParticleTraceSegment> segments);
  void SubmitParticleTracesFromObserver(
    ggems::core::observer::GGEMSTransportObserver const &observer);
  void ClearParticleTraces();

  [[nodiscard]] auto IsInitialized() const noexcept -> bool;

  void SetVulkanDevice(std::string selection);
  void SetVulkanDevice(std::uint32_t enumeration_index);

private:
  void Shutdown() noexcept;

  static void FramebufferResizeCallback(GLFWwindow *window, int width,
                                        int height) noexcept;

  auto RenderFrame(bool framebuffer_resized) -> void;
  auto RecreateSwapchain() -> void;
  auto BuildImGuiFrame() -> void;
  auto RecreateSceneRenderTargets() -> void;
  auto ApplyPendingSourceRunSnapshot() -> void;
  auto ApplyPendingParticleTraceSegments() -> void;

  std::string title_;
  std::int32_t width_{0};
  std::int32_t height_{0};
  std::string vulkan_device_name_selector_{"auto"};
  std::optional<std::uint32_t> vulkan_device_index_selector_;
  GLFWwindow *window_{nullptr};

  std::unique_ptr<detail::GGEMSVulkanDevice> device_;
  std::unique_ptr<detail::GGEMSVulkanPresenter> presenter_;
  std::unique_ptr<detail::GGEMSImGuiIntegration> imgui_integration_;
  std::unique_ptr<GGEMSVulkanSceneRenderer> scene_renderer_;
  std::unique_ptr<GGEMSImGuiLayer> imgui_layer_;
  detail::GGEMSDeviceStatusSnapshot device_status_{};
  detail::GGEMSWorkbenchState workbench_{};
  std::uint32_t scene_target_logical_width_{0U};
  std::uint32_t scene_target_logical_height_{0U};

  std::optional<core::sources::GGEMSSourceRunSnapshot>
    displayed_source_run_snapshot_;

  std::mutex pending_source_run_snapshot_mutex_;
  std::optional<core::sources::GGEMSSourceRunSnapshot>
    pending_source_run_snapshot_;

  std::mutex pending_particle_trace_mutex_;
  std::vector<ggems::render::GGEMSParticleTraceSegment>
    pending_particle_trace_segments_;
  bool has_pending_particle_trace_segments_{false};
  bool pending_particle_trace_clear_{false};

  bool glfw_initialized_{false};
  bool framebuffer_resized_{false};
  bool rendering_failed_{false};
  bool missing_observer_warning_emitted_{false};
};

} // namespace ggems::ui
