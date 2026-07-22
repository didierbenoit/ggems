#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "GGEMS/render/GGEMSParticleTrace.hh"

struct GLFWwindow;

namespace ggems::core {
class GGEMSRun;
}

namespace ggems::core::observer {
class GGEMSTransportObserver;
}

namespace ggems::ui {

class GGEMSVulkanContext;

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

  void Initialise();
  void Run();

  auto SubmitLastRunSourceSnapshot(ggems::core::GGEMSRun const &run) -> void;

  void SubmitParticleTraceSegments(
      std::vector<ggems::render::GGEMSParticleTraceSegment> segments);
  void SubmitParticleTracesFromObserver(
      ggems::core::observer::GGEMSTransportObserver const &observer);
  void ClearParticleTraces();

  [[nodiscard]] auto IsInitialised() const noexcept -> bool;

  void SetVulkanDevice(std::string selection);
  void SetVulkanDevice(std::uint32_t enumeration_index);

private:
  void Shutdown() noexcept;

  static void FramebufferResizeCallback(GLFWwindow *window, int width,
                                        int height) noexcept;

  std::string title_;
  std::int32_t width_{0};
  std::int32_t height_{0};
  std::string vulkan_device_name_selector_{"auto"};
  std::optional<std::uint32_t> vulkan_device_index_selector_;
  GLFWwindow *window_{nullptr};
  std::unique_ptr<GGEMSVulkanContext> vk_context_;
  bool glfw_initialised_{false};
  bool framebuffer_resized_{false};
  bool missing_observer_warning_emitted_{false};
};

} // namespace ggems::ui
