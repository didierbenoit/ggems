#include <filesystem>
#include <fstream>
#include <vector>
#include <format>
#include <array>
#include <cstddef>
#include <cstring>
#include <cstdint>
#include <limits>
#include <utility>
#include <ios>
#include <span>

#include <backends/imgui_impl_vulkan.h>

#include "GGEMS/ui/GGEMSVulkanSceneRenderer.hh"
#include "GGEMS/ui/GGEMSVulkanColorConversion.hh"

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSLogMacros.hh"
#include "GGEMS/render/GGEMSColorNames.hh"
#include "GGEMS/render/GGEMSParticleTrace.hh"
#include "GGEMS/core/particles/GGEMSParticleTypes.hh"

namespace {

constexpr std::uint32_t k_axis_count{3U};
constexpr std::uint32_t k_vertices_per_axis{2U};
constexpr std::uint32_t k_axes_vertex_count{k_axis_count * k_vertices_per_axis};
constexpr float k_orbit_degrees_per_pixel{0.20F};

} // namespace

namespace ggems::ui {

// =============================================================================
// =============================================================================

auto GGEMSVulkanSceneRenderer::Initialize(
    vk::raii::PhysicalDevice const &physical_device,
    vk::raii::Device const &device, vk::Format color_format) -> void {
  physical_device_ = &physical_device;
  device_ = &device;
  color_format_ = color_format;

  CreateAxesShaderModules();
  CreateTraceShaderModules();
  CreateAxesPipeline();
  CreateTracePipeline();

  initialized_ = true;
  requires_resize_ = true;

  GGEMS_INFOEX("Vulkan", 1, "Vulkan scene renderer initialized.");
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::Shutdown() -> void {
  if (!initialized_) {
    return;
  }

  CleanupRenderTargets();
  CleanupTraceResources();
  CleanupAxesPipeline();
  CleanupShaderModules();

  physical_device_ = nullptr;
  device_ = nullptr;
  color_format_ = vk::Format::eUndefined;
  initialized_ = false;
  requires_resize_ = false;
  viewport_extent_ = vk::Extent2D{};

  GGEMS_INFOEX("Vulkan", 2, "Vulkan scene renderer shut down.");
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::SetViewportExtent(vk::Extent2D const &extent)
    -> void {
  if (extent.width == 0U || extent.height == 0U) {
    return;
  }

  if (viewport_extent_.width == extent.width &&
      viewport_extent_.height == extent.height) {
    return;
  }

  viewport_extent_ = extent;
  requires_resize_ = true;

  GGEMS_INFOEX("Vulkan", 2, "Vulkan scene viewport resized to {}x{}",
               viewport_extent_.width, viewport_extent_.height);
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::RecreateRenderTargetsIfNeeded() -> void {
  if (!initialized_ || !requires_resize_) {
    return;
  }

  if (!(device_ != nullptr)) {
    throw ggems::core::GGEMSInternal(
        "A Vulkan device is required before recreating scene render targets.");
  }

  if (!(viewport_extent_.width > 0U &&
                           viewport_extent_.height > 0U)) {
    throw ggems::core::GGEMSInternal("A valid scene viewport extent is required before "
                       "recreating scene render targets.");
  }

  device_->waitIdle();
  CleanupRenderTargets();
  CreateColorTarget();
  CreateDepthTarget();

  requires_resize_ = false;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::IsInitialized() const noexcept -> bool {
  return initialized_;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::RequiresResize() const noexcept -> bool {
  return requires_resize_;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::GetViewportExtent() const noexcept
    -> vk::Extent2D const & {
  return viewport_extent_;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::GetColorFormat() const noexcept -> vk::Format {
  return color_format_;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::GetColorImageView() const noexcept
    -> vk::ImageView {
  return *color_image_view_;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::GetSampler() const noexcept -> vk::Sampler {
  return *sampler_;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::IsDepthFormatSupported(vk::Format format) const
    -> bool {
  if (!(physical_device_ != nullptr)) {
    throw ggems::core::GGEMSInternal("A Vulkan physical device is required before checking "
                       "depth format support.");
  }

  vk::FormatProperties properties =
      physical_device_->getFormatProperties(format);

  return (properties.optimalTilingFeatures &
          vk::FormatFeatureFlagBits::eDepthStencilAttachment) ==
         vk::FormatFeatureFlagBits::eDepthStencilAttachment;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::CreateColorTarget() -> void {
  if (!(device_ != nullptr)) {
    throw ggems::core::GGEMSInternal("A Vulkan device is required before creating a scene "
                       "color target.");
  }

  vk::ImageCreateInfo image_create_info{
      .imageType = vk::ImageType::e2D,
      .format = color_format_,
      .extent = vk::Extent3D{.width = viewport_extent_.width,
                             .height = viewport_extent_.height,
                             .depth = 1U},
      .mipLevels = 1U,
      .arrayLayers = 1U,
      .samples = vk::SampleCountFlagBits::e1,
      .tiling = vk::ImageTiling::eOptimal,
      .usage = vk::ImageUsageFlagBits::eColorAttachment |
               vk::ImageUsageFlagBits::eSampled |
               vk::ImageUsageFlagBits::eTransferDst,
      .sharingMode = vk::SharingMode::eExclusive,
      .initialLayout = vk::ImageLayout::eUndefined};

  color_image_ = vk::raii::Image{*device_, image_create_info};

  vk::MemoryRequirements memory_requirements =
      color_image_.getMemoryRequirements();

  vk::MemoryAllocateInfo memory_allocate_info{
      .allocationSize = memory_requirements.size,
      .memoryTypeIndex =
          FindMemoryType(memory_requirements.memoryTypeBits,
                         vk::MemoryPropertyFlagBits::eDeviceLocal)};

  color_memory_ = vk::raii::DeviceMemory{*device_, memory_allocate_info};
  color_image_.bindMemory(*color_memory_, 0U);

  vk::ImageViewCreateInfo image_view_create_info{
      .image = *color_image_,
      .viewType = vk::ImageViewType::e2D,
      .format = color_format_,
      .subresourceRange = vk::ImageSubresourceRange{
          .aspectMask = vk::ImageAspectFlagBits::eColor,
          .baseMipLevel = 0U,
          .levelCount = 1U,
          .baseArrayLayer = 0U,
          .layerCount = 1U}};

  color_image_view_ = vk::raii::ImageView{*device_, image_view_create_info};

  vk::SamplerCreateInfo sampler_create_info{
      .magFilter = vk::Filter::eLinear,
      .minFilter = vk::Filter::eLinear,
      .mipmapMode = vk::SamplerMipmapMode::eNearest,
      .addressModeU = vk::SamplerAddressMode::eClampToEdge,
      .addressModeV = vk::SamplerAddressMode::eClampToEdge,
      .addressModeW = vk::SamplerAddressMode::eClampToEdge,
      .mipLodBias = 0.0F,
      .anisotropyEnable = vk::False,
      .maxAnisotropy = 1.0F,
      .compareEnable = vk::False,
      .compareOp = vk::CompareOp::eAlways,
      .minLod = 0.0F,
      .maxLod = 0.0F,
      .borderColor = vk::BorderColor::eFloatOpaqueBlack,
      .unnormalizedCoordinates = vk::False};

  sampler_ = vk::raii::Sampler{*device_, sampler_create_info};

  imgui_descriptor_set_ =
      ImGui_ImplVulkan_AddTexture(static_cast<VkSampler>(*sampler_),
                                  static_cast<VkImageView>(*color_image_view_),
                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  color_image_layout_ = vk::ImageLayout::eUndefined;

  GGEMS_INFOEX("Vulkan", 2,
               "Vulkan scene color target created: {}x{}, format={}.",
               viewport_extent_.width, viewport_extent_.height,
               vk::to_string(color_format_));
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::CreateDepthTarget() -> void {
  if (!(device_ != nullptr)) {
    throw ggems::core::GGEMSInternal("A Vulkan device is required before creating a scene "
                       "depth target.");
  }

  if (!(IsDepthFormatSupported(depth_format_))) {
    throw ggems::core::GGEMSInternal(
        std::format("Vulkan depth format '{}' is not supported as a depth "
                  "attachment.",
                  vk::to_string(depth_format_)));
  }

  vk::ImageCreateInfo depth_image_create_info{
      .imageType = vk::ImageType::e2D,
      .format = depth_format_,
      .extent = vk::Extent3D{.width = viewport_extent_.width,
                             .height = viewport_extent_.height,
                             .depth = 1U},
      .mipLevels = 1U,
      .arrayLayers = 1U,
      .samples = vk::SampleCountFlagBits::e1,
      .tiling = vk::ImageTiling::eOptimal,
      .usage = vk::ImageUsageFlagBits::eDepthStencilAttachment,
      .sharingMode = vk::SharingMode::eExclusive,
      .initialLayout = vk::ImageLayout::eUndefined};

  depth_image_ = vk::raii::Image{*device_, depth_image_create_info};

  vk::MemoryRequirements memory_requirements =
      depth_image_.getMemoryRequirements();

  vk::MemoryAllocateInfo memory_allocate_info{
      .allocationSize = memory_requirements.size,
      .memoryTypeIndex =
          FindMemoryType(memory_requirements.memoryTypeBits,
                         vk::MemoryPropertyFlagBits::eDeviceLocal)};

  depth_memory_ = vk::raii::DeviceMemory{*device_, memory_allocate_info};
  depth_image_.bindMemory(*depth_memory_, 0U);

  vk::ImageViewCreateInfo depth_view_create_info{
      .image = *depth_image_,
      .viewType = vk::ImageViewType::e2D,
      .format = depth_format_,
      .subresourceRange = vk::ImageSubresourceRange{
          .aspectMask = vk::ImageAspectFlagBits::eDepth,
          .baseMipLevel = 0U,
          .levelCount = 1U,
          .baseArrayLayer = 0U,
          .layerCount = 1U}};

  depth_image_view_ = vk::raii::ImageView{*device_, depth_view_create_info};

  depth_image_layout_ = vk::ImageLayout::eUndefined;

  GGEMS_INFOEX("Vulkan", 2,
               "Vulkan scene depth target created: {}x{}, format={}.",
               viewport_extent_.width, viewport_extent_.height,
               vk::to_string(depth_format_));
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::CleanupRenderTargets() noexcept -> void {
  if (imgui_descriptor_set_ != VK_NULL_HANDLE) {
    ImGui_ImplVulkan_RemoveTexture(imgui_descriptor_set_);
    imgui_descriptor_set_ = VK_NULL_HANDLE;
  }

  sampler_ = nullptr;
  color_image_view_ = nullptr;
  color_memory_ = nullptr;
  color_image_ = nullptr;
  color_image_layout_ = vk::ImageLayout::eUndefined;

  depth_image_view_ = nullptr;
  depth_memory_ = nullptr;
  depth_image_ = nullptr;
  depth_image_layout_ = vk::ImageLayout::eUndefined;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::FindMemoryType(
    std::uint32_t type_filter, vk::MemoryPropertyFlags properties) const
    -> std::uint32_t {
  if (!(physical_device_ != nullptr)) {
    throw ggems::core::GGEMSInternal("A Vulkan physical device is required before selecting "
                       "a memory type.");
  }

  vk::PhysicalDeviceMemoryProperties memory_properties =
      physical_device_->getMemoryProperties();

  for (std::uint32_t i = 0U; i < memory_properties.memoryTypeCount; ++i) {
    bool type_supported = (type_filter & (1U << i)) != 0U;
    bool properties_supported =
        (memory_properties.memoryTypes[i].propertyFlags & properties) ==
        properties;

    if (type_supported && properties_supported) {
      return i;
    }
  }

  if (!(false)) {
    throw ggems::core::GGEMSInternal("No suitable Vulkan memory type was found for the scene "
                       "renderer color target.");
  }

  return 0U;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::GetTextureID() const noexcept -> ImTextureID {
  return reinterpret_cast<ImTextureID>(imgui_descriptor_set_);
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::RecordSceneCommands(
    vk::raii::CommandBuffer const &command_buffer,
    ggems::render::GGEMSParticleTraceVisibility const &visibility) -> void {
  if (imgui_descriptor_set_ == VK_NULL_HANDLE ||
      *color_image_ == vk::Image{} || *color_image_view_ == vk::ImageView{}) {
    return;
  }

  vk::ImageSubresourceRange color_range{.aspectMask =
                                             vk::ImageAspectFlagBits::eColor,
                                         .baseMipLevel = 0U,
                                         .levelCount = 1U,
                                         .baseArrayLayer = 0U,
                                         .layerCount = 1U};

  vk::PipelineStageFlags2 src_stage =
      color_image_layout_ == vk::ImageLayout::eUndefined
          ? vk::PipelineStageFlagBits2::eNone
          : vk::PipelineStageFlagBits2::eFragmentShader;

  vk::AccessFlags2 src_access =
      color_image_layout_ == vk::ImageLayout::eUndefined
          ? vk::AccessFlagBits2::eNone
          : vk::AccessFlagBits2::eShaderSampledRead;

  vk::ImageMemoryBarrier2 to_color_attachment{
      .srcStageMask = src_stage,
      .srcAccessMask = src_access,
      .dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
      .dstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
      .oldLayout = color_image_layout_,
      .newLayout = vk::ImageLayout::eColorAttachmentOptimal,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = *color_image_,
      .subresourceRange = color_range};

  vk::DependencyInfo to_color_attachment_dependency{
      .imageMemoryBarrierCount = 1U,
      .pImageMemoryBarriers = &to_color_attachment};

  command_buffer.pipelineBarrier2(to_color_attachment_dependency);

  vk::ClearValue clear_value{detail::ToVulkanClearColor(render::BLUE_Abyss)};

  vk::RenderingAttachmentInfo color_attachment{
      .imageView = *color_image_view_,
      .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eStore,
      .clearValue = clear_value};

  vk::ImageSubresourceRange depth_range{.aspectMask =
                                            vk::ImageAspectFlagBits::eDepth,
                                        .baseMipLevel = 0U,
                                        .levelCount = 1U,
                                        .baseArrayLayer = 0U,
                                        .layerCount = 1U};

  vk::PipelineStageFlags2 depth_src_stage =
      depth_image_layout_ == vk::ImageLayout::eUndefined
          ? vk::PipelineStageFlagBits2::eNone
          : vk::PipelineStageFlagBits2::eLateFragmentTests;

  vk::AccessFlags2 depth_src_access =
      depth_image_layout_ == vk::ImageLayout::eUndefined
          ? vk::AccessFlagBits2::eNone
          : vk::AccessFlagBits2::eDepthStencilAttachmentWrite;

  vk::ImageMemoryBarrier2 to_depth_attachment{
      .srcStageMask = depth_src_stage,
      .srcAccessMask = depth_src_access,
      .dstStageMask = vk::PipelineStageFlagBits2::eEarlyFragmentTests |
                      vk::PipelineStageFlagBits2::eLateFragmentTests,
      .dstAccessMask = vk::AccessFlagBits2::eDepthStencilAttachmentRead |
                       vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
      .oldLayout = depth_image_layout_,
      .newLayout = vk::ImageLayout::eDepthAttachmentOptimal,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = *depth_image_,
      .subresourceRange = depth_range};

  vk::DependencyInfo to_depth_attachment_dependency{
      .imageMemoryBarrierCount = 1U,
      .pImageMemoryBarriers = &to_depth_attachment};

  command_buffer.pipelineBarrier2(to_depth_attachment_dependency);

  depth_image_layout_ = vk::ImageLayout::eDepthAttachmentOptimal;

  vk::ClearValue depth_clear_value{};
  depth_clear_value.depthStencil =
      vk::ClearDepthStencilValue{.depth = 1.0F, .stencil = 0U};

  vk::RenderingAttachmentInfo depth_attachment{
      .imageView = *depth_image_view_,
      .imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eDontCare,
      .clearValue = depth_clear_value};

  vk::RenderingInfo const rendering_info{
      .renderArea = vk::Rect2D{.offset = vk::Offset2D{.x = 0, .y = 0},
                               .extent = viewport_extent_},
      .layerCount = 1U,
      .colorAttachmentCount = 1U,
      .pColorAttachments = &color_attachment,
      .pDepthAttachment = &depth_attachment};

  command_buffer.beginRendering(rendering_info);

  if (show_axes_) {
    RecordAxesCommands(command_buffer);
  }

  RecordTraceCommands(command_buffer, visibility);

  command_buffer.endRendering();

  vk::ImageMemoryBarrier2 to_shader_read{
      .srcStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
      .srcAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
      .dstStageMask = vk::PipelineStageFlagBits2::eFragmentShader,
      .dstAccessMask = vk::AccessFlagBits2::eShaderSampledRead,
      .oldLayout = vk::ImageLayout::eColorAttachmentOptimal,
      .newLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = *color_image_,
      .subresourceRange = color_range};

  vk::DependencyInfo const to_shader_read_dependency{
      .imageMemoryBarrierCount = 1U, .pImageMemoryBarriers = &to_shader_read};

  command_buffer.pipelineBarrier2(to_shader_read_dependency);

  color_image_layout_ = vk::ImageLayout::eShaderReadOnlyOptimal;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::ReadSPIRVFile(std::filesystem::path const &path)
    -> std::vector<std::uint32_t> {
  std::ifstream file{path, std::ios::binary | std::ios::ate};

  if (!(file.is_open())) {
    throw ggems::core::GGEMSInternal(
        std::format("Unable to open SPIR-V shader file '{}'.", path.string()));
  }

  std::streamsize file_size = file.tellg();

  if (!(file_size > 0)) {
    throw ggems::core::GGEMSInternal(
        std::format("SPIR-V shader file '{}' is empty.", path.string()));
  }

  if (!(file_size % static_cast<std::streamsize>(sizeof(std::uint32_t)) == 0)) {
    throw ggems::core::GGEMSInternal(
        std::format("SPIR-V shader file '{}' has an invalid byte size.",
                  path.string()));
  }

  file.seekg(0, std::ios::beg);

  std::size_t word_count =
      static_cast<std::size_t>(file_size) / sizeof(std::uint32_t);

  std::vector<std::uint32_t> code(word_count);

  file.read(reinterpret_cast<char *>(code.data()), file_size);

  if (!(file.good())) {
    throw ggems::core::GGEMSInternal(
        std::format("Unable to read SPIR-V shader file '{}'.", path.string()));
  }

  return code;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::CreateAxesShaderModules() -> void {
  if (!(device_ != nullptr)) {
    throw ggems::core::GGEMSInternal("A Vulkan device is required before creating scene "
                       "shader modules.");
  }

#ifndef GGEMS_UI_SHADER_DIRECTORY
  if (!(false)) {
    throw ggems::core::GGEMSInternal("GGEMS_UI_SHADER_DIRECTORY is not defined.");
  }
#endif

  std::filesystem::path shader_directory{GGEMS_UI_SHADER_DIRECTORY};

  std::filesystem::path vertex_shader_path =
      shader_directory / "GGEMSAxes.vert.spv";
  std::filesystem::path fragment_shader_path =
      shader_directory / "GGEMSAxes.frag.spv";

  std::vector<std::uint32_t> vertex_code = ReadSPIRVFile(vertex_shader_path);
  std::vector<std::uint32_t> fragment_code =
      ReadSPIRVFile(fragment_shader_path);

  vk::ShaderModuleCreateInfo vertex_create_info{
      .codeSize = vertex_code.size() * sizeof(std::uint32_t),
      .pCode = vertex_code.data()};

  vk::ShaderModuleCreateInfo fragment_create_info{
      .codeSize = fragment_code.size() * sizeof(std::uint32_t),
      .pCode = fragment_code.data()};

  axes_vertex_shader_module_ =
      vk::raii::ShaderModule{*device_, vertex_create_info};
  axes_fragment_shader_module_ =
      vk::raii::ShaderModule{*device_, fragment_create_info};

  GGEMS_INFOEX("Vulkan", 2,
               "Vulkan scene axes shader modules created from '{}'.",
               shader_directory.string());
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::CreateAxesPipeline() -> void {
  if (!(device_ != nullptr)) {
    throw ggems::core::GGEMSInternal(
        "A Vulkan device is required before creating the axes pipeline.");
  }

  if (!(*axes_vertex_shader_module_ != vk::ShaderModule{})) {
    throw ggems::core::GGEMSInternal(
        "A Vulkan vertex shader module is required before creating the axes "
      "pipeline.");
  }

  if (!(*axes_fragment_shader_module_ != vk::ShaderModule{})) {
    throw ggems::core::GGEMSInternal(
        "A Vulkan fragment shader module is required before creating the axes "
      "pipeline.");
  }

  vk::PipelineShaderStageCreateInfo vertex_stage{
      .stage = vk::ShaderStageFlagBits::eVertex,
      .module = *axes_vertex_shader_module_,
      .pName = "VertexMain"};

  vk::PipelineShaderStageCreateInfo fragment_stage{
      .stage = vk::ShaderStageFlagBits::eFragment,
      .module = *axes_fragment_shader_module_,
      .pName = "FragmentMain"};

  std::array<vk::PipelineShaderStageCreateInfo, 2U> shader_stages{
      vertex_stage, fragment_stage};

  vk::PipelineVertexInputStateCreateInfo vertex_input_state{};

  vk::PipelineInputAssemblyStateCreateInfo input_assembly_state{
      .topology = vk::PrimitiveTopology::eLineList,
      .primitiveRestartEnable = vk::False};

  vk::PipelineViewportStateCreateInfo viewport_state{.viewportCount = 1U,
                                                     .scissorCount = 1U};

  vk::PipelineRasterizationStateCreateInfo rasterization_state{
      .depthClampEnable = vk::False,
      .rasterizerDiscardEnable = vk::False,
      .polygonMode = vk::PolygonMode::eFill,
      .cullMode = vk::CullModeFlagBits::eNone,
      .frontFace = vk::FrontFace::eCounterClockwise,
      .depthBiasEnable = vk::False,
      .lineWidth = 1.0F};

  vk::PipelineMultisampleStateCreateInfo multisample_state{
      .rasterizationSamples = vk::SampleCountFlagBits::e1,
      .sampleShadingEnable = vk::False};

  vk::PipelineColorBlendAttachmentState color_blend_attachment{
      .blendEnable = vk::False,
      .colorWriteMask =
          vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
          vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA};

  vk::PipelineColorBlendStateCreateInfo color_blend_state{
      .logicOpEnable = vk::False,
      .attachmentCount = 1U,
      .pAttachments = &color_blend_attachment};

  std::array<vk::DynamicState, 2U> dynamic_states{vk::DynamicState::eViewport,
                                                  vk::DynamicState::eScissor};

  vk::PipelineDynamicStateCreateInfo dynamic_state{
      .dynamicStateCount = static_cast<std::uint32_t>(dynamic_states.size()),
      .pDynamicStates = dynamic_states.data()};

  vk::PipelineDepthStencilStateCreateInfo depth_stencil_state{
      .depthTestEnable = vk::True,
      .depthWriteEnable = vk::True,
      .depthCompareOp = vk::CompareOp::eLessOrEqual,
      .depthBoundsTestEnable = vk::False,
      .stencilTestEnable = vk::False,
      .minDepthBounds = 0.0F,
      .maxDepthBounds = 1.0F};

  vk::PushConstantRange axes_push_constant_range{
      .stageFlags = vk::ShaderStageFlagBits::eVertex,
      .offset = 0U,
      .size = sizeof(ScenePushConstants)};

  vk::PipelineLayoutCreateInfo pipeline_layout_create_info{
      .pushConstantRangeCount = 1U,
      .pPushConstantRanges = &axes_push_constant_range};

  axes_pipeline_layout_ =
      vk::raii::PipelineLayout{*device_, pipeline_layout_create_info};

  vk::PipelineRenderingCreateInfo rendering_create_info{
      .colorAttachmentCount = 1U,
      .pColorAttachmentFormats = &color_format_,
      .depthAttachmentFormat = depth_format_};

  vk::GraphicsPipelineCreateInfo pipeline_create_info{
      .pNext = &rendering_create_info,
      .stageCount = static_cast<std::uint32_t>(shader_stages.size()),
      .pStages = shader_stages.data(),
      .pVertexInputState = &vertex_input_state,
      .pInputAssemblyState = &input_assembly_state,
      .pViewportState = &viewport_state,
      .pRasterizationState = &rasterization_state,
      .pMultisampleState = &multisample_state,
      .pDepthStencilState = &depth_stencil_state,
      .pColorBlendState = &color_blend_state,
      .pDynamicState = &dynamic_state,
      .layout = *axes_pipeline_layout_,
      .renderPass = nullptr,
      .subpass = 0U};

  axes_pipeline_ = vk::raii::Pipeline{*device_, nullptr, pipeline_create_info};

  GGEMS_INFOEX("Vulkan", 2, "Vulkan scene axes pipeline created.");
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::CreateTraceShaderModules() -> void {
  if (!(device_ != nullptr)) {
    throw ggems::core::GGEMSInternal(
        "A Vulkan device is required before creating trace shader modules.");
  }

#ifndef GGEMS_UI_SHADER_DIRECTORY
  if (!(false)) {
    throw ggems::core::GGEMSInternal("GGEMS_UI_SHADER_DIRECTORY is not defined.");
  }
#endif

  std::filesystem::path shader_directory(GGEMS_UI_SHADER_DIRECTORY);

  std::filesystem::path vertex_shader_path =
      shader_directory / "GGEMSTraces.vert.spv";
  std::filesystem::path fragment_shader_path =
      shader_directory / "GGEMSTraces.frag.spv";

  std::vector<std::uint32_t> vertex_code = ReadSPIRVFile(vertex_shader_path);
  std::vector<std::uint32_t> fragment_code =
      ReadSPIRVFile(fragment_shader_path);

  vk::ShaderModuleCreateInfo vertex_create_info{
      .codeSize = vertex_code.size() * sizeof(std::uint32_t),
      .pCode = vertex_code.data()};

  vk::ShaderModuleCreateInfo fragment_create_info{
      .codeSize = fragment_code.size() * sizeof(std::uint32_t),
      .pCode = fragment_code.data()};

  trace_vertex_shader_module_ =
      vk::raii::ShaderModule{*device_, vertex_create_info};
  trace_fragment_shader_module_ =
      vk::raii::ShaderModule{*device_, fragment_create_info};

  GGEMS_INFOEX("Vulkan", 2,
               "Vulkan scene trace shader modules created from '{}'.",
               shader_directory.string());
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::RecordAxesCommands(
    vk::raii::CommandBuffer const &command_buffer) -> void {
  if (*axes_pipeline_ == vk::Pipeline{}) {
    return;
  }

  vk::Viewport viewport{.x = 0.0F,
                        .y = 0.0F,
                        .width = static_cast<float>(viewport_extent_.width),
                        .height = static_cast<float>(viewport_extent_.height),
                        .minDepth = 0.0F,
                        .maxDepth = 1.0F};

  vk::Rect2D const scissor{.offset = vk::Offset2D{.x = 0, .y = 0},
                           .extent = viewport_extent_};

  command_buffer.setViewport(0U, viewport);
  command_buffer.setScissor(0U, scissor);

  command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
                              *axes_pipeline_);

  camera_.SetViewportExtent(viewport_extent_);

  ScenePushConstants push_constants = camera_.BuildWorldToClipMatrix();

  command_buffer.pushConstants(
      *axes_pipeline_layout_, vk::ShaderStageFlagBits::eVertex, 0U,
      vk::ArrayProxy<const ScenePushConstants>{1U, &push_constants});

  command_buffer.draw(k_axes_vertex_count, 1U, 0U, 0U);
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::CreateTracePipeline() -> void {
  if (!(device_ != nullptr)) {
    throw ggems::core::GGEMSInternal(
        "A Vulkan device is required before creating the trace pipeline.");
  }

  if (!(*trace_vertex_shader_module_ != vk::ShaderModule{})) {
    throw ggems::core::GGEMSInternal("A Vulkan vertex shader module is required before "
                       "creating the trace pipeline.");
  }

  if (!(*trace_fragment_shader_module_ != vk::ShaderModule{})) {
    throw ggems::core::GGEMSInternal("A Vulkan fragment shader module is required before "
                       "creating the trace pipeline.");
  }

  vk::PipelineShaderStageCreateInfo vertex_stage{
      .stage = vk::ShaderStageFlagBits::eVertex,
      .module = *trace_vertex_shader_module_,
      .pName = "VertexMain"};

  vk::PipelineShaderStageCreateInfo fragment_stage{
      .stage = vk::ShaderStageFlagBits::eFragment,
      .module = *trace_fragment_shader_module_,
      .pName = "FragmentMain"};

  std::array<vk::PipelineShaderStageCreateInfo, 2U> shader_stages{
      vertex_stage, fragment_stage};

  vk::VertexInputBindingDescription vertex_binding_description{
      .binding = 0U,
      .stride = sizeof(TraceVertex),
      .inputRate = vk::VertexInputRate::eVertex};

  std::array<vk::VertexInputAttributeDescription, 2U>
      vertex_attribute_descriptions{
          {vk::VertexInputAttributeDescription{
               .location = 0U,
               .binding = 0U,
               .format = vk::Format::eR32G32B32Sfloat,
               .offset = offsetof(TraceVertex, position)},
           vk::VertexInputAttributeDescription{
               .location = 1U,
               .binding = 0U,
               .format = vk::Format::eR32G32B32A32Sfloat,
               .offset = offsetof(TraceVertex, color)}}};

  vk::PipelineVertexInputStateCreateInfo vertex_input_state{
      .vertexBindingDescriptionCount = 1U,
      .pVertexBindingDescriptions = &vertex_binding_description,
      .vertexAttributeDescriptionCount =
          static_cast<std::uint32_t>(vertex_attribute_descriptions.size()),
      .pVertexAttributeDescriptions = vertex_attribute_descriptions.data()};

  vk::PipelineInputAssemblyStateCreateInfo input_assembly_state{
      .topology = vk::PrimitiveTopology::eLineList,
      .primitiveRestartEnable = vk::False};

  vk::PipelineViewportStateCreateInfo viewport_state{.viewportCount = 1U,
                                                     .scissorCount = 1U};

  vk::PipelineRasterizationStateCreateInfo rasterization_state{
      .depthClampEnable = vk::False,
      .rasterizerDiscardEnable = vk::False,
      .polygonMode = vk::PolygonMode::eFill,
      .cullMode = vk::CullModeFlagBits::eNone,
      .frontFace = vk::FrontFace::eCounterClockwise,
      .depthBiasEnable = vk::False,
      .lineWidth = 1.0F};

  vk::PipelineMultisampleStateCreateInfo multisample_state{
      .rasterizationSamples = vk::SampleCountFlagBits::e1,
      .sampleShadingEnable = vk::False};

  vk::PipelineColorBlendAttachmentState color_blend_attachment{
      .blendEnable = vk::False,
      .colorWriteMask =
          vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
          vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA};

  vk::PipelineColorBlendStateCreateInfo color_blend_state{
      .logicOpEnable = vk::False,
      .attachmentCount = 1U,
      .pAttachments = &color_blend_attachment};

  std::array<vk::DynamicState, 2U> dynamic_states{vk::DynamicState::eViewport,
                                                  vk::DynamicState::eScissor};

  vk::PipelineDynamicStateCreateInfo dynamic_state{
      .dynamicStateCount = static_cast<std::uint32_t>(dynamic_states.size()),
      .pDynamicStates = dynamic_states.data()};

  vk::PipelineDepthStencilStateCreateInfo depth_stencil_state{
      .depthTestEnable = vk::True,
      .depthWriteEnable = vk::True,
      .depthCompareOp = vk::CompareOp::eLessOrEqual,
      .depthBoundsTestEnable = vk::False,
      .stencilTestEnable = vk::False,
      .minDepthBounds = 0.0F,
      .maxDepthBounds = 1.0F};

  vk::PushConstantRange trace_push_constant_range{
      .stageFlags = vk::ShaderStageFlagBits::eVertex,
      .offset = 0U,
      .size = sizeof(ScenePushConstants)};

  vk::PipelineLayoutCreateInfo pipeline_layout_create_info{
      .pushConstantRangeCount = 1U,
      .pPushConstantRanges = &trace_push_constant_range};

  trace_pipeline_layout_ =
      vk::raii::PipelineLayout{*device_, pipeline_layout_create_info};

  vk::PipelineRenderingCreateInfo rendering_create_info{
      .colorAttachmentCount = 1U,
      .pColorAttachmentFormats = &color_format_,
      .depthAttachmentFormat = depth_format_};

  vk::GraphicsPipelineCreateInfo pipeline_create_info{
      .pNext = &rendering_create_info,
      .stageCount = static_cast<std::uint32_t>(shader_stages.size()),
      .pStages = shader_stages.data(),
      .pVertexInputState = &vertex_input_state,
      .pInputAssemblyState = &input_assembly_state,
      .pViewportState = &viewport_state,
      .pRasterizationState = &rasterization_state,
      .pMultisampleState = &multisample_state,
      .pDepthStencilState = &depth_stencil_state,
      .pColorBlendState = &color_blend_state,
      .pDynamicState = &dynamic_state,
      .layout = *trace_pipeline_layout_,
      .renderPass = nullptr,
      .subpass = 0U};

  trace_pipeline_ = vk::raii::Pipeline{*device_, nullptr, pipeline_create_info};

  GGEMS_INFOEX("Vulkan", 2, "Vulkan scene trace pipeline created.");
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::CreateDemoTraceVertices() -> void {
  using core::particles::GGEMSParticleType;

  std::vector<ggems::render::GGEMSParticleTraceSegment> demo_segments{};
  demo_segments.reserve(10);

  auto append_segment =
      [&demo_segments](GGEMSParticleType particle_type,
                       std::array<float, 3U> const &first,
                       std::array<float, 3U> const &second) -> void {
    demo_segments.push_back(ggems::render::GGEMSParticleTraceSegment{
        .particle_type = particle_type,
        .begin = ggems::render::GGEMSParticleTracePoint{.x_m = first[0],
                                                        .y_m = first[1],
                                                        .z_m = first[2]},
        .end = ggems::render::GGEMSParticleTracePoint{
            .x_m = second[0], .y_m = second[1], .z_m = second[2]}});
  };

  append_segment(GGEMSParticleType::Aionino, {0.0F, 0.0F, -0.90F},
                 {0.0F, 0.0F, -0.45F});

  append_segment(GGEMSParticleType::Gamma, {0.0F, 0.0F, -0.45F},
                 {0.0F, 0.0F, 0.10F});
  append_segment(GGEMSParticleType::Gamma, {0.0F, 0.0F, 0.10F},
                 {0.0F, 0.0F, 0.82F});

  append_segment(GGEMSParticleType::Electron, {0.0F, 0.0F, 0.10F},
                 {0.38F, 0.10F, 0.30F});
  append_segment(GGEMSParticleType::Electron, {0.38F, 0.10F, 0.30F},
                 {0.72F, 0.20F, 0.46F});

  append_segment(GGEMSParticleType::Positron, {0.0F, 0.0F, -0.04F},
                 {-0.32F, 0.18F, 0.18F});
  append_segment(GGEMSParticleType::Positron, {-0.32F, 0.18F, 0.18F},
                 {-0.56F, 0.32F, 0.42F});

  append_segment(GGEMSParticleType::Proton, {-0.50F, -0.36F, -0.55F},
                 {-0.08F, -0.24F, -0.18F});
  append_segment(GGEMSParticleType::Neutron, {0.42F, -0.36F, -0.55F},
                 {0.06F, -0.18F, -0.20F});
  append_segment(GGEMSParticleType::Alpha, {-0.22F, 0.42F, -0.42F},
                 {0.28F, 0.34F, -0.02F});

  auto draw_data = ggems::render::BuildParticleTraceDrawData(demo_segments);
  trace_vertices_ = std::move(draw_data.vertices);
  trace_draw_ranges_ = std::move(draw_data.draw_ranges);

  GGEMS_INFOEX("Vulkan", 2, "Created {} demo particle trace vertex/vertices.",
               trace_vertices_.size());
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::DestroyTraceVertexBuffer() noexcept -> void {
  trace_vertex_buffer_ = nullptr;
  trace_vertex_memory_ = nullptr;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::CreateTraceVertexBuffer() -> void {
  if (!(device_ != nullptr)) {
    throw ggems::core::GGEMSInternal("A Vulkan device is required before creating the trace "
                       "vertex buffer.");
  }

  if (trace_vertices_.empty()) {
    return;
  }

  auto const buffer_size =
      static_cast<vk::DeviceSize>(trace_vertices_.size() * sizeof(TraceVertex));

  vk::BufferCreateInfo buffer_create_info{
      .size = buffer_size,
      .usage = vk::BufferUsageFlagBits::eVertexBuffer,
      .sharingMode = vk::SharingMode::eExclusive};

  trace_vertex_buffer_ = vk::raii::Buffer{*device_, buffer_create_info};

  vk::MemoryRequirements memory_requirements =
      trace_vertex_buffer_.getMemoryRequirements();

  vk::MemoryAllocateInfo memory_allocate_info{
      .allocationSize = memory_requirements.size,
      .memoryTypeIndex =
          FindMemoryType(memory_requirements.memoryTypeBits,
                         vk::MemoryPropertyFlagBits::eHostVisible |
                             vk::MemoryPropertyFlagBits::eHostCoherent)};

  trace_vertex_memory_ = vk::raii::DeviceMemory{*device_, memory_allocate_info};
  trace_vertex_buffer_.bindMemory(*trace_vertex_memory_, 0U);

  void *mapped_memory = trace_vertex_memory_.mapMemory(0, buffer_size);

  std::memcpy(mapped_memory, trace_vertices_.data(),
              static_cast<std::size_t>(buffer_size));

  trace_vertex_memory_.unmapMemory();

  GGEMS_INFOEX("Vulkan", 2,
               "Vulkan trace vertex buffer "
               "uploaded: {} vertex/vertices.",
               trace_vertices_.size());
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::RecordTraceCommands(
    vk::raii::CommandBuffer const &command_buffer,
    ggems::render::GGEMSParticleTraceVisibility const &visibility) -> void {
  if (!visibility.IsGlobalVisible()) {
    return;
  }

  if (*trace_pipeline_ == vk::Pipeline{} ||
      *trace_vertex_buffer_ == vk::Buffer{} || trace_vertices_.empty() ||
      trace_draw_ranges_.empty()) {
    return;
  }

  vk::Viewport viewport{.x = 0.0F,
                        .y = 0.0F,
                        .width = static_cast<float>(viewport_extent_.width),
                        .height = static_cast<float>(viewport_extent_.height),
                        .minDepth = 0.0F,
                        .maxDepth = 1.0F};

  vk::Rect2D const scissor{.offset = vk::Offset2D{.x = 0, .y = 0},
                           .extent = viewport_extent_};

  command_buffer.setViewport(0U, viewport);
  command_buffer.setScissor(0U, scissor);

  command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
                              *trace_pipeline_);

  std::array<vk::Buffer, 1U> vertex_buffers{*trace_vertex_buffer_};
  std::array<vk::DeviceSize, 1U> vertex_offsets{0U};

  command_buffer.bindVertexBuffers(0U, vertex_buffers, vertex_offsets);

  camera_.SetViewportExtent(viewport_extent_);

  ScenePushConstants push_constants = camera_.BuildWorldToClipMatrix();

  command_buffer.pushConstants(
      *trace_pipeline_layout_, vk::ShaderStageFlagBits::eVertex, 0U,
      vk::ArrayProxy<const ScenePushConstants>{1U, &push_constants});

  for (TraceDrawRange const &draw_range : trace_draw_ranges_) {
    if (!visibility.ShouldDraw(draw_range.source_index)) {
      continue;
    }

    command_buffer.draw(static_cast<std::uint32_t>(draw_range.vertex_count), 1U,
                        static_cast<std::uint32_t>(draw_range.first_vertex),
                        0U);
  }
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::CleanupShaderModules() noexcept -> void {
  trace_fragment_shader_module_ = nullptr;
  trace_vertex_shader_module_ = nullptr;
  axes_fragment_shader_module_ = nullptr;
  axes_vertex_shader_module_ = nullptr;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::CleanupTracePipeline() noexcept -> void {
  trace_pipeline_ = nullptr;
  trace_pipeline_layout_ = nullptr;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::CleanupTraceResources() noexcept -> void {
  DestroyTraceVertexBuffer();
  trace_vertices_.clear();
  trace_draw_ranges_.clear();
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::CleanupAxesPipeline() noexcept -> void {
  axes_pipeline_ = nullptr;
  axes_pipeline_layout_ = nullptr;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::SetShowAxes(bool show_axes) noexcept -> void {
  show_axes_ = show_axes;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::GetParticleTraceVertexCount() const noexcept
    -> std::uint32_t {
  return static_cast<std::uint32_t>(trace_vertices_.size());
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::ShouldShowAxes() const noexcept -> bool {
  return show_axes_;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::OrbitCamera(float delta_x_pixels,
                                           float delta_y_pixels) noexcept
    -> void {
  if (delta_x_pixels == 0.0F && delta_y_pixels == 0.0F) {
    return;
  }

  camera_.Orbit(delta_x_pixels * k_orbit_degrees_per_pixel,
                delta_y_pixels * k_orbit_degrees_per_pixel);
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::PanCamera(float delta_x_pixels,
                                         float delta_y_pixels) noexcept
    -> void {
  if (delta_x_pixels == 0.0F && delta_y_pixels == 0.0F) {
    return;
  }

  camera_.Pan(delta_x_pixels, delta_y_pixels);
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::ZoomCamera(float wheel_delta) noexcept -> void {
  camera_.ZoomBy(wheel_delta);
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::ResetCamera() noexcept -> void {
  camera_.Reset();
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::SetParticleTraceSegments(
    std::span<ggems::render::GGEMSParticleTraceSegment const> segments)
    -> void {
  if (!(device_ != nullptr)) {
    throw ggems::core::GGEMSInternal(
        "A Vulkan device is required before setting particle trace segments.");
  }

  auto draw_data = ggems::render::BuildParticleTraceDrawData(segments);

  if (!(draw_data.vertices.size() <=
          static_cast<std::size_t>(std::numeric_limits<std::uint32_t>::max()))) {
    throw ggems::core::GGEMSInternal(
        "The particle trace vertex count exceeds the Vulkan uint32_t draw "
      "range.");
  }

  DestroyTraceVertexBuffer();

  trace_vertices_ = std::move(draw_data.vertices);
  trace_draw_ranges_ = std::move(draw_data.draw_ranges);

  if (!trace_vertices_.empty()) {
    CreateTraceVertexBuffer();
  }

  GGEMS_INFOEX(
      "Vulkan", 1,
      "Loaded {} particle trace segment(s) into Vulkan: {} vertex/vertices.",
      segments.size(), trace_vertices_.size());
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanSceneRenderer::ClearParticleTraces() -> void {
  trace_vertices_.clear();
  trace_draw_ranges_.clear();
  DestroyTraceVertexBuffer();

  GGEMS_INFOEX("Vulkan", 1, "Cleared Vulkan particle traces.");
}
} // namespace ggems::ui
