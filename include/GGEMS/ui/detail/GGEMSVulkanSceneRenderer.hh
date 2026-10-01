#pragma once

#include <array>
#include <cstdint>
#include <filesystem>
#include <span>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "GGEMS/render/GGEMSParticleTrace.hh"

namespace ggems::ui {

class GGEMSVulkanSceneRenderer {
private:
  using ScenePushConstants = std::array<std::array<float, 4U>, 4U>;
  using TraceVertex = ggems::render::GGEMSParticleTraceVertex;
  using TraceDrawRange = ggems::render::GGEMSParticleTraceDrawRange;

public:
  struct RenderParameters {
    std::array<std::array<float, 4U>, 4U> world_to_clip{};
    bool show_axes{true};
    ggems::render::GGEMSParticleTraceVisibility const &trace_visibility;
  };

  GGEMSVulkanSceneRenderer() = default;
  ~GGEMSVulkanSceneRenderer() = default;

  GGEMSVulkanSceneRenderer(GGEMSVulkanSceneRenderer const &) = delete;
  GGEMSVulkanSceneRenderer(GGEMSVulkanSceneRenderer &&) = delete;
  auto operator=(GGEMSVulkanSceneRenderer const &)
    -> GGEMSVulkanSceneRenderer & = delete;
  auto operator=(GGEMSVulkanSceneRenderer &&)
    -> GGEMSVulkanSceneRenderer & = delete;

  auto Initialize(vk::raii::PhysicalDevice const &physical_device,
                  vk::raii::Device const &device, vk::Format color_format)
    -> void;

  auto Shutdown() -> void;

  auto SetViewportExtent(vk::Extent2D const &extent) -> void;
  auto RecreateRenderTargets() -> void;

  [[nodiscard]] auto IsInitialized() const noexcept -> bool;
  [[nodiscard]] auto RequiresResize() const noexcept -> bool;
  [[nodiscard]] auto GetViewportExtent() const noexcept -> vk::Extent2D const &;
  [[nodiscard]] auto GetColorFormat() const noexcept -> vk::Format;
  [[nodiscard]] auto GetColorImageView() const noexcept -> vk::ImageView;

  auto RecordSceneCommands(vk::raii::CommandBuffer const &command_buffer,
                           RenderParameters const &parameters) -> void;

  auto SetParticleTraceSegments(
    std::span<ggems::render::GGEMSParticleTraceSegment const> segments) -> void;
  auto ClearParticleTraces() -> void;
  [[nodiscard]] auto GetParticleTraceVertexCount() const noexcept
    -> std::uint32_t;

private:
  auto CreateColorTarget() -> void;
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
  auto RecordAxesCommands(vk::raii::CommandBuffer const &command_buffer,
                          ScenePushConstants const &push_constants) -> void;

  auto CreateTraceShaderModules() -> void;
  auto CreateTracePipeline() -> void;
  auto CleanupTracePipeline() noexcept -> void;
  auto CleanupTraceResources() noexcept -> void;
  auto CreateTraceVertexBuffer() -> void;
  auto DestroyTraceVertexBuffer() noexcept -> void;
  auto RecordTraceCommands(
    vk::raii::CommandBuffer const &command_buffer,
    ScenePushConstants const &push_constants,
    ggems::render::GGEMSParticleTraceVisibility const &visibility) -> void;

  auto CreateDepthTarget() -> void;
  [[nodiscard]] auto IsDepthFormatSupported(vk::Format format) const -> bool;

  vk::raii::PhysicalDevice const *physical_device_{nullptr};
  vk::raii::Device const *device_{nullptr};

  vk::Extent2D viewport_extent_{};

  vk::Format color_format_{vk::Format::eUndefined};
  vk::raii::Image color_image_{nullptr};
  vk::raii::DeviceMemory color_memory_{nullptr};
  vk::raii::ImageView color_image_view_{nullptr};
  vk::ImageLayout color_image_layout_{vk::ImageLayout::eUndefined};

  vk::Format depth_format_{vk::Format::eD32Sfloat};
  vk::raii::Image depth_image_{nullptr};
  vk::raii::DeviceMemory depth_memory_{nullptr};
  vk::raii::ImageView depth_image_view_{nullptr};
  vk::ImageLayout depth_image_layout_{vk::ImageLayout::eUndefined};

  bool initialized_{false};
  bool requires_resize_{false};

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
};

} // namespace ggems::ui
