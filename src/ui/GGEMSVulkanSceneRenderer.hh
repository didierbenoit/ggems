#pragma once

#include <cstdint>
#include <filesystem>
#include <vector>

#include <imgui.h>
#include <vulkan/vulkan_raii.hpp>

#include "GGEMSVulkanCamera.hh"

namespace ggems::ui {

class GGEMSVulkanSceneRenderer {
private:
  using ScenePushConstants = GGEMSVulkanCamera::Matrix4Rows;

  struct TraceVertex {
    float position[3]{};
    float colour[4]{};
  };

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

  void SetShowParticleTraces(bool show_particle_traces) noexcept;
  [[nodiscard]] bool ShouldShowParticleTraces() const noexcept;
  [[nodiscard]] std::uint32_t GetParticleTraceVertexCount() const noexcept;

  void OrbitCamera(float delta_x_pixels, float delta_y_pixels) noexcept;
  void ZoomCamera(float wheel_delta) noexcept;
  void ResetCamera() noexcept;

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

  void CreateTraceShaderModules();
  void CreateTracePipeline();
  void CleanupTracePipeline() noexcept;
  void CleanupTraceResources() noexcept;
  void CreateDemoTraceVertices();
  void CreateTraceVertexBuffer();
  void CleanuTraceResources() noexcept;
  void RecordTraceCommands(vk::raii::CommandBuffer const &command_buffer);

  void CreateDepthTarget();
  [[nodiscard]] bool IsDepthFormatSupported(vk::Format format) const;

private:
  vk::raii::PhysicalDevice const *physical_device_{nullptr};
  vk::raii::Device const *device_{nullptr};

  vk::Extent2D viewport_extent_{};

  vk::raii::Sampler sampler_{nullptr};

  vk::Format colour_format_{vk::Format::eUndefined};
  vk::raii::Image colour_image_{nullptr};
  vk::raii::DeviceMemory colour_memory_{nullptr};
  vk::raii::ImageView colour_image_view_{nullptr};
  vk::ImageLayout colour_image_layout_{vk::ImageLayout::eUndefined};

  vk::Format depth_format_{vk::Format::eD32Sfloat};
  vk::raii::Image depth_image_{nullptr};
  vk::raii::DeviceMemory depth_memory_{nullptr};
  vk::raii::ImageView depth_image_view_{nullptr};
  vk::ImageLayout depth_image_layout_{vk::ImageLayout::eUndefined};

  bool initialised_{false};
  bool requires_resize_{false};

  VkDescriptorSet imgui_descriptor_set_{VK_NULL_HANDLE};

  vk::raii::ShaderModule axes_vertex_shader_module_{nullptr};
  vk::raii::ShaderModule axes_fragment_shader_module_{nullptr};
  vk::raii::ShaderModule trace_vertex_shader_module_{nullptr};
  vk::raii::ShaderModule trace_fragment_shader_module_{nullptr};

  vk::raii::PipelineLayout axes_pipeline_layout_{nullptr};
  vk::raii::Pipeline axes_pipeline_{nullptr};

  vk::raii::PipelineLayout trace_pipeline_layout_{nullptr};
  vk::raii::Pipeline trace_pipeline_{nullptr};
  vk::raii::Buffer trace_vertex_buffer_{nullptr};
  vk::raii::DeviceMemory trace_vertex_memory_{nullptr};
  std::vector<TraceVertex> trace_vertices_{};

  bool show_axes_{true};
  bool show_particle_traces_{true};
  GGEMSVulkanCamera camera_{};
};

} // namespace ggems::ui
