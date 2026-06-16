#pragma once

#include <cstdint>
#include <filesystem>
#include <vector>

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
  void RecordSceneCommands(vk::raii::CommandBuffer const &command_buffer);

  void SetShowAxes(bool show_axes) noexcept;
  [[nodiscard]] bool ShouldShowAxes() const noexcept;

private:
  void CreateColourTarget();
  void CleanupRenderTargets() noexcept;

  [[nodiscard]] std::uint32_t
  FindMemoryType(std::uint32_t type_filter,
                 vk::MemoryPropertyFlags properties) const;

  void CreateAxesShaderModules();
  void CleanupShaderModules() noexcept;

  [[nodiscard]] static std::vector<std::uint32_t>
  ReadSPIRVFile(std::filesystem::path const &path);

  void CreateAxesPipeline();
  void CleanupAxesPipeline() noexcept;
  void RecordAxesCommands(vk::raii::CommandBuffer const &command_buffer);

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

  vk::raii::ShaderModule axes_vertex_shader_module_{nullptr};
  vk::raii::ShaderModule axes_fragment_shader_module_{nullptr};

  vk::raii::PipelineLayout axes_pipeline_layout_{nullptr};
  vk::raii::Pipeline axes_pipeline_{nullptr};

  bool show_axes_{true};
};

} // namespace ggems::ui
