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
               "Vulkan scene colour targer created: {}x{}, format={}.",
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

void GGEMSVulkanSceneRenderer::RecordClearCommands(
    vk::raii::CommandBuffer const &command_buffer) {
  if (imgui_descriptor_set_ == VK_NULL_HANDLE ||
      *colour_image_ == vk::Image{}) {
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

  vk::ImageMemoryBarrier2 to_transfer{
      .srcStageMask = src_stage,
      .srcAccessMask = src_access,
      .dstStageMask = vk::PipelineStageFlagBits2::eTransfer,
      .dstAccessMask = vk::AccessFlagBits2::eTransferWrite,
      .oldLayout = colour_image_layout_,
      .newLayout = vk::ImageLayout::eTransferDstOptimal,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image = *colour_image_,
      .subresourceRange = colour_range};

  vk::DependencyInfo to_transfer_dependency{
      .imageMemoryBarrierCount = 1U, .pImageMemoryBarriers = &to_transfer};

  command_buffer.pipelineBarrier2(to_transfer_dependency);

  vk::ClearColorValue clear_colour{
      std::array<float, 4U>{0.025f, 0.030f, 0.032f, 1.0f}};

  command_buffer.clearColorImage(*colour_image_,
                                 vk::ImageLayout::eTransferDstOptimal,
                                 clear_colour, colour_range);

  vk::ImageMemoryBarrier2 to_shader_read{
      .srcStageMask = vk::PipelineStageFlagBits2::eTransfer,
      .srcAccessMask = vk::AccessFlagBits2::eTransferWrite,
      .dstStageMask = vk::PipelineStageFlagBits2::eFragmentShader,
      .dstAccessMask = vk::AccessFlagBits2::eShaderSampledRead,
      .oldLayout = vk::ImageLayout::eTransferDstOptimal,
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

} // namespace ggems::ui
