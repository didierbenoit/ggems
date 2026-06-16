#include <filesystem>
#include <fstream>
#include <vector>
#include <format>
#include <array>

#include <backends/imgui_impl_vulkan.h>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMSVulkanSceneRenderer.hh"

namespace ggems::ui {

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSVulkanSceneRenderer::Initialise(
    vk::raii::PhysicalDevice const &physical_device,
    vk::raii::Device const &device, vk::Format colour_format) {
  physical_device_ = &physical_device;
  device_ = &device;
  colour_format_ = colour_format;

  CreateAxesShaderModules();
  CreateAxesPipeline();

  initialised_ = true;
  requires_resize_ = true;

  GGEMS_INFOEX("Vulkan", 1, "Vulkan scene renderer initialised.");
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSVulkanSceneRenderer::Shutdown() noexcept {
  if (!initialised_) {
    return;
  }

  CleanupRenderTargets();
  CleanupAxesPipeline();
  CleanupShaderModules();

  physical_device_ = nullptr;
  device_ = nullptr;
  colour_format_ = vk::Format::eUndefined;
  initialised_ = false;
  requires_resize_ = false;
  viewport_extent_ = vk::Extent2D{};

  GGEMS_INFOEX("Vulkan", 2, "Vulkan scene renderer shut down.");
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSVulkanSceneRenderer::SetViewportExtent(vk::Extent2D const &extent) {
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

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSVulkanSceneRenderer::RecreateRenderTargetsIfNeeded() {
  if (!initialised_ || !requires_resize_) {
    return;
  }

  GGEMS_CHECK_INTERNAL(
      device_ != nullptr,
      "A Vulkan device is required before recreating scene render targets.");

  GGEMS_CHECK_INTERNAL(viewport_extent_.width > 0U &&
                           viewport_extent_.height > 0U,
                       "A valid scene viewport extent is required before "
                       "recreating scene render targets.");

  device_->waitIdle();
  CleanupRenderTargets();
  CreateColourTarget();

  requires_resize_ = false;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

bool GGEMSVulkanSceneRenderer::IsInitialised() const noexcept {
  return initialised_;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

bool GGEMSVulkanSceneRenderer::RequiresResize() const noexcept {
  return requires_resize_;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

vk::Extent2D const &
GGEMSVulkanSceneRenderer::GetViewportExtent() const noexcept {
  return viewport_extent_;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

vk::Format GGEMSVulkanSceneRenderer::GetColourFormat() const noexcept {
  return colour_format_;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

vk::ImageView GGEMSVulkanSceneRenderer::GetColourImageView() const noexcept {
  return *colour_image_view_;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

vk::Sampler GGEMSVulkanSceneRenderer::GetSampler() const noexcept {
  return *sampler_;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSVulkanSceneRenderer::CreateColourTarget() {
  GGEMS_CHECK_INTERNAL(device_ != nullptr,
                       "A Vulkan device is required before creating a scene "
                       "colour target.");

  vk::ImageCreateInfo image_create_info{
      .imageType = vk::ImageType::e2D,
      .format = colour_format_,
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

  colour_image_ = vk::raii::Image{*device_, image_create_info};

  vk::MemoryRequirements memory_requirements =
      colour_image_.getMemoryRequirements();

  vk::MemoryAllocateInfo memory_allocate_info{
      .allocationSize = memory_requirements.size,
      .memoryTypeIndex =
          FindMemoryType(memory_requirements.memoryTypeBits,
                         vk::MemoryPropertyFlagBits::eDeviceLocal)};

  colour_memory_ = vk::raii::DeviceMemory{*device_, memory_allocate_info};
  colour_image_.bindMemory(*colour_memory_, 0U);

  vk::ImageViewCreateInfo image_view_create_info{
      .image = *colour_image_,
      .viewType = vk::ImageViewType::e2D,
      .format = colour_format_,
      .subresourceRange = vk::ImageSubresourceRange{
          .aspectMask = vk::ImageAspectFlagBits::eColor,
          .baseMipLevel = 0U,
          .levelCount = 1U,
          .baseArrayLayer = 0U,
          .layerCount = 1U}};

  colour_image_view_ = vk::raii::ImageView{*device_, image_view_create_info};

  vk::SamplerCreateInfo sampler_create_info{
      .magFilter = vk::Filter::eLinear,
      .minFilter = vk::Filter::eLinear,
      .mipmapMode = vk::SamplerMipmapMode::eNearest,
      .addressModeU = vk::SamplerAddressMode::eClampToEdge,
      .addressModeV = vk::SamplerAddressMode::eClampToEdge,
      .addressModeW = vk::SamplerAddressMode::eClampToEdge,
      .mipLodBias = 0.0f,
      .anisotropyEnable = vk::False,
      .maxAnisotropy = 1.0f,
      .compareEnable = vk::False,
      .compareOp = vk::CompareOp::eAlways,
      .minLod = 0.0f,
      .maxLod = 0.0f,
      .borderColor = vk::BorderColor::eFloatOpaqueBlack,
      .unnormalizedCoordinates = vk::False};

  sampler_ = vk::raii::Sampler{*device_, sampler_create_info};

  imgui_descriptor_set_ =
      ImGui_ImplVulkan_AddTexture(static_cast<VkSampler>(*sampler_),
                                  static_cast<VkImageView>(*colour_image_view_),
                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  colour_image_layout_ = vk::ImageLayout::eUndefined;

  GGEMS_INFOEX("Vulkan", 2,
               "Vulkan scene colour target created: {}x{}, format={}.",
               viewport_extent_.width, viewport_extent_.height,
               vk::to_string(colour_format_));
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSVulkanSceneRenderer::CleanupRenderTargets() noexcept {
  if (imgui_descriptor_set_ != VK_NULL_HANDLE) {
    ImGui_ImplVulkan_RemoveTexture(imgui_descriptor_set_);
    imgui_descriptor_set_ = VK_NULL_HANDLE;
  }

  sampler_ = nullptr;
  colour_image_view_ = nullptr;
  colour_memory_ = nullptr;
  colour_image_ = nullptr;
  colour_image_layout_ = vk::ImageLayout::eUndefined;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::uint32_t GGEMSVulkanSceneRenderer::FindMemoryType(
    std::uint32_t type_filter, vk::MemoryPropertyFlags properties) const {
  GGEMS_CHECK_INTERNAL(physical_device_ != nullptr,
                       "A Vulkan physical device is required before selecting "
                       "a memory type.");

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

  GGEMS_CHECK_INTERNAL(false,
                       "No suitable Vulkan memory type was found for the scene "
                       "renderer colour target.");

  return 0U;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

ImTextureID GGEMSVulkanSceneRenderer::GetTextureID() const noexcept {
  return reinterpret_cast<ImTextureID>(imgui_descriptor_set_);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSVulkanSceneRenderer::RecordSceneCommands(
    vk::raii::CommandBuffer const &command_buffer) {
  if (imgui_descriptor_set_ == VK_NULL_HANDLE ||
      *colour_image_ == vk::Image{} || *colour_image_view_ == vk::ImageView{}) {
    return;
  }

  vk::ImageSubresourceRange colour_range{.aspectMask =
                                             vk::ImageAspectFlagBits::eColor,
                                         .baseMipLevel = 0U,
                                         .levelCount = 1U,
                                         .baseArrayLayer = 0U,
                                         .layerCount = 1U};

  vk::PipelineStageFlags2 src_stage =
      colour_image_layout_ == vk::ImageLayout::eUndefined
          ? vk::PipelineStageFlagBits2::eNone
          : vk::PipelineStageFlagBits2::eFragmentShader;

  vk::AccessFlags2 src_access =
      colour_image_layout_ == vk::ImageLayout::eUndefined
          ? vk::AccessFlagBits2::eNone
          : vk::AccessFlagBits2::eShaderSampledRead;

  vk::ImageMemoryBarrier2 to_colour_attachment{
      .srcStageMask = src_stage,
      .srcAccessMask = src_access,
      .dstStageMask = vk::PipelineStageFlagBits2::eColorAttachmentOutput,
      .dstAccessMask = vk::AccessFlagBits2::eColorAttachmentWrite,
      .oldLayout = colour_image_layout_,
      .newLayout = vk::ImageLayout::eColorAttachmentOptimal,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = *colour_image_,
      .subresourceRange = colour_range};

  vk::DependencyInfo to_colour_attachment_dependency{
      .imageMemoryBarrierCount = 1U,
      .pImageMemoryBarriers = &to_colour_attachment};

  command_buffer.pipelineBarrier2(to_colour_attachment_dependency);

  vk::ClearValue clear_value{
      std::array<float, 4U>{0.025f, 0.030f, 0.032f, 1.0f}};

  vk::RenderingAttachmentInfo colour_attachment{
      .imageView = *colour_image_view_,
      .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eStore,
      .clearValue = clear_value};

  vk::RenderingInfo const rendering_info{
      .renderArea = vk::Rect2D{.offset = vk::Offset2D{.x = 0, .y = 0},
                               .extent = viewport_extent_},
      .layerCount = 1U,
      .colorAttachmentCount = 1U,
      .pColorAttachments = &colour_attachment};

  command_buffer.beginRendering(rendering_info);

  if (show_axes_) {
    RecordAxesCommands(command_buffer);
  }

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
      .image = *colour_image_,
      .subresourceRange = colour_range};

  vk::DependencyInfo const to_shader_read_dependency{
      .imageMemoryBarrierCount = 1U, .pImageMemoryBarriers = &to_shader_read};

  command_buffer.pipelineBarrier2(to_shader_read_dependency);

  colour_image_layout_ = vk::ImageLayout::eShaderReadOnlyOptimal;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::vector<std::uint32_t>
GGEMSVulkanSceneRenderer::ReadSPIRVFile(std::filesystem::path const &path) {
  std::ifstream file{path, std::ios::binary | std::ios::ate};

  GGEMS_CHECK_INTERNAL(
      file.is_open(),
      std::format("Unable to open SPIR-V shader file '{}'.", path.string()));

  std::streamsize file_size = file.tellg();

  GGEMS_CHECK_INTERNAL(
      file_size > 0,
      std::format("SPIR-V shader file '{}' is empty.", path.string()));

  GGEMS_CHECK_INTERNAL(
      file_size % static_cast<std::streamsize>(sizeof(std::uint32_t)) == 0,
      std::format("SPIR-V shader file '{}' has an invalid byte size.",
                  path.string()));

  file.seekg(0, std::ios::beg);

  std::size_t word_count =
      static_cast<std::size_t>(file_size) / sizeof(std::uint32_t);

  std::vector<std::uint32_t> code(word_count);

  file.read(reinterpret_cast<char *>(code.data()), file_size);

  GGEMS_CHECK_INTERNAL(
      file.good(),
      std::format("Unable to read SPIR-V shader file '{}'.", path.string()));

  return code;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSVulkanSceneRenderer::CreateAxesShaderModules() {
  GGEMS_CHECK_INTERNAL(device_ != nullptr,
                       "A Vulkan device is required before creating scene "
                       "shader modules.");

#ifndef GGEMS_UI_SHADER_DIRECTORY
  GGEMS_CHECK_INTERNAL(false, "GGEMS_UI_SHADER_DIRECTORY is not defined.");
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

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSVulkanSceneRenderer::CreateAxesPipeline() {
  GGEMS_CHECK_INTERNAL(
      device_ != nullptr,
      "A Vulkan device is required before creating the axes pipeline.");

  GGEMS_CHECK_INTERNAL(
      *axes_vertex_shader_module_ != vk::ShaderModule{},
      "A Vulkan vertex shader module is required before creating the axes "
      "pipeline.");

  GGEMS_CHECK_INTERNAL(
      *axes_fragment_shader_module_ != vk::ShaderModule{},
      "A Vulkan fragment shader module is required before creating the axes "
      "pipeline.");

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

  vk::PipelineColorBlendAttachmentState colour_blend_attachment{
      .blendEnable = vk::False,
      .colorWriteMask =
          vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
          vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA};

  vk::PipelineColorBlendStateCreateInfo colour_blend_state{
      .logicOpEnable = vk::False,
      .attachmentCount = 1U,
      .pAttachments = &colour_blend_attachment};

  std::array<vk::DynamicState, 2U> dynamic_states{vk::DynamicState::eViewport,
                                                  vk::DynamicState::eScissor};

  vk::PipelineDynamicStateCreateInfo dynamic_state{
      .dynamicStateCount = static_cast<std::uint32_t>(dynamic_states.size()),
      .pDynamicStates = dynamic_states.data()};

  vk::PipelineLayoutCreateInfo pipeline_layout_create_info{};

  axes_pipeline_layout_ =
      vk::raii::PipelineLayout{*device_, pipeline_layout_create_info};

  vk::PipelineRenderingCreateInfo rendering_create_info{
      .colorAttachmentCount = 1U, .pColorAttachmentFormats = &colour_format_};

  vk::GraphicsPipelineCreateInfo pipeline_create_info{
      .pNext = &rendering_create_info,
      .stageCount = static_cast<std::uint32_t>(shader_stages.size()),
      .pStages = shader_stages.data(),
      .pVertexInputState = &vertex_input_state,
      .pInputAssemblyState = &input_assembly_state,
      .pViewportState = &viewport_state,
      .pRasterizationState = &rasterization_state,
      .pMultisampleState = &multisample_state,
      .pColorBlendState = &colour_blend_state,
      .pDynamicState = &dynamic_state,
      .layout = *axes_pipeline_layout_,
      .renderPass = nullptr,
      .subpass = 0U};

  axes_pipeline_ = vk::raii::Pipeline{*device_, nullptr, pipeline_create_info};

  GGEMS_INFOEX("Vulkan", 2, "Vulkan scene axes pipeline created.");
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSVulkanSceneRenderer::RecordAxesCommands(
    vk::raii::CommandBuffer const &command_buffer) {
  if (*axes_pipeline_ == vk::Pipeline{}) {
    return;
  }

  vk::Viewport viewport{.x = 0.0f,
                        .y = 0.0f,
                        .width = static_cast<float>(viewport_extent_.width),
                        .height = static_cast<float>(viewport_extent_.height),
                        .minDepth = 0.0f,
                        .maxDepth = 1.0f};

  vk::Rect2D const scissor{.offset = vk::Offset2D{.x = 0, .y = 0},
                           .extent = viewport_extent_};

  command_buffer.setViewport(0U, viewport);
  command_buffer.setScissor(0U, scissor);

  command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
                              *axes_pipeline_);

  command_buffer.draw(6U, 1U, 0U, 0U);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSVulkanSceneRenderer::CleanupShaderModules() noexcept {
  axes_fragment_shader_module_ = nullptr;
  axes_vertex_shader_module_ = nullptr;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSVulkanSceneRenderer::CleanupAxesPipeline() noexcept {
  axes_pipeline_ = nullptr;
  axes_pipeline_layout_ = nullptr;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSVulkanSceneRenderer::SetShowAxes(bool show_axes) noexcept {
  show_axes_ = show_axes;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

bool GGEMSVulkanSceneRenderer::ShouldShowAxes() const noexcept {
  return show_axes_;
}

} // namespace ggems::ui
