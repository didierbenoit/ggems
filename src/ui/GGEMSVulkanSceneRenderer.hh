#pragma once

#include <cstdint>

#include <imgui.h>
#include <vulkan/vulkan_raii.hpp>

namespace ggems::ui {

class GGEMSVulkanSceneRenderer {
public:
  GGEMSVulkanSceneRenderer() = default;
  ~GGEMSVulkanSceneRenderer() = default;

  GGEMSVulkanSceneRenderer(GGEMSVulkanSceneRenderer const &) = delete;
  GGEMSVulkanSceneRenderer(GGEMSVulkanSceneRenderer &&) = delete;
  GGEMSVulkanSceneRenderer &
  operator=(GGEMSVulkanSceneRenderer const &) = delete;
  GGEMSVulkanSceneRenderer &operator=(GGEMSVulkanSceneRenderer &&) = delete;

public:
  void Initialise(vk::raii::PhysicalDevice const &physical_device,
                  vk::raii::Device const &device, vk::Format colour_format);

  void Shutdown() noexcept;

  void SetViewportExtent(vk::Extent2D const &extent);
  void RecreateRenderTargetsIfNeeded();

  [[nodiscard]] bool IsInitialised() const noexcept;
  [[nodiscard]] bool RequiresResize() const noexcept;
  [[nodiscard]] vk::Extent2D const &GetViewportExtent() const noexcept;
  [[nodiscard]] vk::Format GetColourFormat() const noexcept;
  [[nodiscard]] vk::ImageView GetColourImageView() const noexcept;
  [[nodiscard]] vk::Sampler GetSampler() const noexcept;

  [[nodiscard]] ImTextureID GetTextureID() const noexcept;
  void RecordClearCommands(vk::raii::CommandBuffer const &command_buffer);

private:
  void CreateColourTarget();
  void CleanupRenderTargets() noexcept;

  [[nodiscard]] std::uint32_t
  FindMemoryType(std::uint32_t type_filter,
                 vk::MemoryPropertyFlags properties) const;

private:
  vk::raii::PhysicalDevice const *physical_device_{nullptr};
  vk::raii::Device const *device_{nullptr};

  vk::Format colour_format_{vk::Format::eUndefined};
  vk::Extent2D viewport_extent_{};

  vk::raii::Image colour_image_{nullptr};
  vk::raii::DeviceMemory colour_memory_{nullptr};
  vk::raii::ImageView colour_image_view_{nullptr};
  vk::raii::Sampler sampler_{nullptr};

  bool initialised_{false};
  bool requires_resize_{false};

  VkDescriptorSet imgui_descriptor_set_{VK_NULL_HANDLE};
  vk::ImageLayout colour_image_layout_{vk::ImageLayout::eUndefined};
};

} // namespace ggems::ui
