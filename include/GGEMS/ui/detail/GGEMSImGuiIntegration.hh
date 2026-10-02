#pragma once

#include <cstdint>
#include <optional>
#include <filesystem>
#include <string>

#include <imgui.h>
#include <vulkan/vulkan.hpp>

struct GLFWwindow;
struct ImGuiContext;

namespace ggems::ui::detail {

class GGEMSImGuiIntegration {
public:
  struct VulkanHandles {
    vk::Instance instance{};
    vk::PhysicalDevice physical_device{};
    vk::Device device{};
    vk::Queue graphics_queue{};
    std::uint32_t graphics_queue_family{0U};
  };

  struct VulkanBackendEpoch {
    std::uint32_t image_count{0U};
    std::uint32_t min_image_count{0U};
    vk::Format color_format{vk::Format::eUndefined};
    vk::SampleCountFlagBits sample_count{vk::SampleCountFlagBits::e1};
    std::uint32_t view_mask{0U};
    vk::Format depth_format{vk::Format::eUndefined};
    vk::Format stencil_format{vk::Format::eUndefined};

    [[nodiscard]] auto operator==(VulkanBackendEpoch const &) const noexcept
      -> bool = default;
  };

  GGEMSImGuiIntegration() = default;
  ~GGEMSImGuiIntegration() noexcept;

  GGEMSImGuiIntegration(GGEMSImGuiIntegration const &) = delete;
  GGEMSImGuiIntegration(GGEMSImGuiIntegration &&) = delete;
  auto operator=(GGEMSImGuiIntegration const &)
    -> GGEMSImGuiIntegration & = delete;
  auto operator=(GGEMSImGuiIntegration &&) -> GGEMSImGuiIntegration & = delete;

  auto Initialize(GLFWwindow *window, VulkanHandles const &handles,
                  VulkanBackendEpoch const &epoch) -> void;
  auto Shutdown() noexcept -> void;

  auto UpdateVulkanBackend(VulkanBackendEpoch const &epoch) -> void;

  auto RegisterSceneTexture(vk::ImageView image_view) -> void;
  auto UnregisterSceneTexture() noexcept -> void;
  [[nodiscard]] auto GetSceneTextureID() const noexcept -> ImTextureID;

  auto BeginFrame() -> void;
  auto RenderDrawData(vk::CommandBuffer command_buffer) const -> void;

  struct FramebufferDensity {
    float x{1.0F};
    float y{1.0F};
  };

  [[nodiscard]] static auto GetFramebufferDensity() noexcept
    -> FramebufferDensity;

  auto ThrowIfBackendFailed() const -> void;

private:
  auto AttachGlfwBackend() -> void;
  auto DetachGlfwBackend() noexcept -> void;
  auto AttachVulkanBackend(VulkanBackendEpoch const &epoch) -> void;
  auto DetachVulkanBackend() noexcept -> void;
  auto ReleaseSceneDescriptor() noexcept -> void;

  auto LoadSettings() -> void;
  auto SaveSettings() -> void;
  auto ApplyContentScale(float content_scale) -> void;

  static auto LoadFonts() -> void;
  static auto RecordBackendResult(VkResult result) noexcept -> void;

  GLFWwindow *window_{nullptr};
  VulkanHandles handles_{};
  VulkanBackendEpoch epoch_{};
  VkFormat color_attachment_format_{VK_FORMAT_UNDEFINED};

  ImGuiContext *context_{nullptr};
  bool glfw_backend_attached_{false};
  bool vulkan_backend_attached_{false};

  vk::ImageView scene_image_view_{};
  VkDescriptorSet scene_descriptor_set_{VK_NULL_HANDLE};

  float applied_content_scale_{0.0F};

  std::optional<std::filesystem::path> settings_path_;
  std::string persisted_settings_;
  bool settings_write_failed_{false};

  std::optional<vk::Result> backend_failure_;
};
} // namespace ggems::ui::detail
