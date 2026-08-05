#pragma once

#include <cstdint>
#include <filesystem>
#include <span>
#include <vector>

#include <imgui.h>
#include <vulkan/vulkan_raii.hpp>

#include "GGEMSVulkanCamera.hh"
#include "GGEMS/render/GGEMSParticleTrace.hh"

namespace ggems::ui {

class GGEMSVulkanSceneRenderer {
private:
  using ScenePushConstants = GGEMSVulkanCamera::Matrix4Rows;
  using TraceVertex = ggems::render::GGEMSParticleTraceVertex;
  using TraceDrawRange = ggems::render::GGEMSParticleTraceDrawRange;

public:
  GGEMSVulkanSceneRenderer() = default;
  ~GGEMSVulkanSceneRenderer() = default;

  GGEMSVulkanSceneRenderer(GGEMSVulkanSceneRenderer const &) = delete;
  GGEMSVulkanSceneRenderer(GGEMSVulkanSceneRenderer &&) = delete;
  auto operator=(GGEMSVulkanSceneRenderer const &)
      -> GGEMSVulkanSceneRenderer & = delete;
  auto operator=(GGEMSVulkanSceneRenderer &&)
      -> GGEMSVulkanSceneRenderer & = delete;

  auto Initialise(vk::raii::PhysicalDevice const &physical_device,
                  vk::raii::Device const &device, vk::Format colour_format)
      -> void;

  auto Shutdown() noexcept -> void;

  auto SetViewportExtent(vk::Extent2D const &extent) -> void;
  auto RecreateRenderTargetsIfNeeded() -> void;

  [[nodiscard]] auto IsInitialised() const noexcept -> bool;
  [[nodiscard]] auto RequiresResize() const noexcept -> bool;
  [[nodiscard]] auto GetViewportExtent() const noexcept -> vk::Extent2D const &;
  [[nodiscard]] auto GetColourFormat() const noexcept -> vk::Format;
  [[nodiscard]] auto GetColourImageView() const noexcept -> vk::ImageView;
  [[nodiscard]] auto GetSampler() const noexcept -> vk::Sampler;

  [[nodiscard]] auto GetTextureID() const noexcept -> ImTextureID;
  auto RecordSceneCommands(
      vk::raii::CommandBuffer const &command_buffer,
      ggems::render::GGEMSParticleTraceVisibility const &visibility) -> void;

  auto SetShowAxes(bool show_axes) noexcept -> void;
  [[nodiscard]] auto ShouldShowAxes() const noexcept -> bool;

  auto SetParticleTraceSegments(
      std::span<ggems::render::GGEMSParticleTraceSegment const> segments)
      -> void;
  auto ClearParticleTraces() -> void;
  [[nodiscard]] auto GetParticleTraceVertexCount() const noexcept
      -> std::uint32_t;

  auto OrbitCamera(float delta_x_pixels, float delta_y_pixels) noexcept -> void;
  auto PanCamera(float delta_x_pixels, float delta_y_pixels) noexcept -> void;
  auto ZoomCamera(float wheel_delta) noexcept -> void;
  auto ResetCamera() noexcept -> void;

private:
  auto CreateColourTarget() -> void;
  auto CleanupRenderTargets() noexcept -> void;

  [[nodiscard]] auto FindMemoryType(std::uint32_t type_filter,
                                    vk::MemoryPropertyFlags properties) const
      -> std::uint32_t;

  auto CreateAxesShaderModules() -> void;
  auto CleanupShaderModules() noexcept -> void;

  [[nodiscard]] static auto ReadSPIRVFile(std::filesystem::path const &path)
      -> std::vector<std::uint32_t>;

  auto CreateAxesPipeline() -> void;
  auto CleanupAxesPipeline() noexcept -> void;
  auto RecordAxesCommands(vk::raii::CommandBuffer const &command_buffer)
      -> void;

  auto CreateTraceShaderModules() -> void;
  auto CreateTracePipeline() -> void;
  auto CleanupTracePipeline() noexcept -> void;
  auto CleanupTraceResources() noexcept -> void;
  auto CreateDemoTraceVertices() -> void;
  auto CreateTraceVertexBuffer() -> void;
  auto DestroyTraceVertexBuffer() noexcept -> void;
  auto RecordTraceCommands(
      vk::raii::CommandBuffer const &command_buffer,
      ggems::render::GGEMSParticleTraceVisibility const &visibility) -> void;

  auto CreateDepthTarget() -> void;
  [[nodiscard]] auto IsDepthFormatSupported(vk::Format format) const -> bool;

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
  std::vector<TraceVertex> trace_vertices_;
  std::vector<TraceDrawRange> trace_draw_ranges_;

  bool show_axes_{true};
  GGEMSVulkanCamera camera_;
};

} // namespace ggems::ui
