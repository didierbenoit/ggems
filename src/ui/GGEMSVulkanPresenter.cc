#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <format>
#include <limits>
#include <utility>
#include <vector>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "GGEMS/ui/detail/GGEMSVulkanPresenter.hh"
#include "GGEMS/ui/detail/GGEMSVulkanColorConversion.hh"
#include "GGEMS/ui/detail/GGEMSVulkanDevice.hh"

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/logging/GGEMSLogMacros.hh"
#include "GGEMS/render/GGEMSColorNames.hh"

namespace ggems::ui::detail {

// =============================================================================
// =============================================================================

GGEMSVulkanPresenter::GGEMSVulkanPresenter(
  GGEMSVulkanDevice const &device, vk::Extent2D const &framebuffer_extent)
    : device_{device} {
  CreateCommandPool();
  CreateFrameSlots();

  SwapchainSupportDetails const support_details = QuerySwapchainSupport();

  generation_ = CreateSwapchainGeneration(
    support_details,
    ChooseSwapchainExtent(support_details.capabilities, framebuffer_extent),
    vk::SwapchainKHR{});
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanPresenter::GetImageCount() const noexcept -> std::uint32_t {
  return static_cast<std::uint32_t>(generation_.images.size());
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanPresenter::GetImageFormat() const noexcept -> vk::Format {
  return generation_.image_format;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanPresenter::GetExtent() const noexcept -> vk::Extent2D const & {
  return generation_.extent;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanPresenter::ChooseSwapchainSurfaceFormat(
  std::vector<vk::SurfaceFormatKHR> const &surface_formats)
  -> vk::SurfaceFormatKHR {
  if (surface_formats.empty()) {
    throw ggems::core::GGEMSInternal(
      "No Vulkan surface format is available for GuiMode.");
  }

  for (vk::SurfaceFormatKHR const &surface_format : surface_formats) {
    if (surface_format.format == vk::Format::eB8G8R8A8Srgb &&
        surface_format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
      return surface_format;
    }
  }

  return surface_formats.front();
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanPresenter::ChooseSwapchainPresentMode(
  std::vector<vk::PresentModeKHR> const &present_modes) -> vk::PresentModeKHR {
  if (present_modes.empty()) {
    throw ggems::core::GGEMSInternal(
      "No Vulkan present mode is available for GuiMode.");
  }

  for (vk::PresentModeKHR const &present_mode : present_modes) {
    if (present_mode == vk::PresentModeKHR::eFifo) {
      return present_mode;
    }
  }

  return present_modes.front();
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanPresenter::ChooseSwapchainExtent(
  vk::SurfaceCapabilitiesKHR const &capabilities,
  vk::Extent2D const &framebuffer_extent) -> vk::Extent2D {
  if (capabilities.currentExtent.width !=
      std::numeric_limits<std::uint32_t>::max()) {
    return capabilities.currentExtent;
  }

  return vk::Extent2D{
    .width =
      std::clamp(framebuffer_extent.width, capabilities.minImageExtent.width,
                 capabilities.maxImageExtent.width),
    .height =
      std::clamp(framebuffer_extent.height, capabilities.minImageExtent.height,
                 capabilities.maxImageExtent.height),
  };
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanPresenter::QuerySwapchainSupport() const
  -> GGEMSVulkanPresenter::SwapchainSupportDetails {
  vk::raii::PhysicalDevice const &physical_device = device_.GetPhysicalDevice();
  vk::SurfaceKHR const surface = *device_.GetSurface();

  SwapchainSupportDetails details{};

  details.capabilities = physical_device.getSurfaceCapabilitiesKHR(surface);
  details.surface_formats = physical_device.getSurfaceFormatsKHR(surface);
  details.present_modes = physical_device.getSurfacePresentModesKHR(surface);

  return details;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanPresenter::CreateSwapchainGeneration(
  SwapchainSupportDetails const &support_details, vk::Extent2D const &extent,
  vk::SwapchainKHR old_swapchain) -> GGEMSVulkanPresenter::SwapchainGeneration {
  vk::SurfaceFormatKHR surface_format =
    ChooseSwapchainSurfaceFormat(support_details.surface_formats);

  vk::PresentModeKHR present_mode =
    ChooseSwapchainPresentMode(support_details.present_modes);

  std::uint32_t image_count = support_details.capabilities.minImageCount + 1U;

  if (support_details.capabilities.maxImageCount > 0U &&
      image_count > support_details.capabilities.maxImageCount) {
    image_count = support_details.capabilities.maxImageCount;
  }

  std::array<std::uint32_t, 2> queue_family_indices{
    device_.GetGraphicsQueueFamily(),
    device_.GetPresentationQueueFamily(),
  };

  vk::SharingMode image_sharing_mode{vk::SharingMode::eExclusive};
  std::uint32_t queue_family_index_count{0};
  std::uint32_t *queue_family_index_data{nullptr};

  if (queue_family_indices[0] != queue_family_indices[1]) {
    image_sharing_mode = vk::SharingMode::eConcurrent;
    queue_family_index_count =
      static_cast<std::uint32_t>(queue_family_indices.size());
    queue_family_index_data = queue_family_indices.data();
  }

  constexpr std::array k_composite_alpha_modes{
    vk::CompositeAlphaFlagBitsKHR::eOpaque,
    vk::CompositeAlphaFlagBitsKHR::eInherit,
    vk::CompositeAlphaFlagBitsKHR::ePreMultiplied,
    vk::CompositeAlphaFlagBitsKHR::ePostMultiplied,
  };

  auto const composite_alpha = std::ranges::find_if(
    k_composite_alpha_modes,
    [&support_details](vk::CompositeAlphaFlagBitsKHR mode) -> bool {
      return static_cast<bool>(
        support_details.capabilities.supportedCompositeAlpha & mode);
    });

  if (composite_alpha == k_composite_alpha_modes.end()) {
    throw ggems::core::GGEMSRecoverable(
      "No supported Vulkan swapchain composite alpha mode is available for "
      "GuiMode.");
  }

  vk::SwapchainCreateInfoKHR create_info{
    .surface = *device_.GetSurface(),
    .minImageCount = image_count,
    .imageFormat = surface_format.format,
    .imageColorSpace = surface_format.colorSpace,
    .imageExtent = extent,
    .imageArrayLayers = 1U,
    .imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
    .imageSharingMode = image_sharing_mode,
    .queueFamilyIndexCount = queue_family_index_count,
    .pQueueFamilyIndices = queue_family_index_data,
    .preTransform = support_details.capabilities.currentTransform,
    .compositeAlpha = *composite_alpha,
    .presentMode = present_mode,
    .clipped = vk::True,
    .oldSwapchain = old_swapchain,
  };

  vk::raii::Device const &device = device_.GetDevice();

  vk::raii::SwapchainKHR swapchain{device, create_info};
  std::vector<vk::Image> images = swapchain.getImages();

  std::vector<vk::raii::ImageView> image_views;
  std::vector<vk::raii::Semaphore> render_finished_semaphores;

  image_views.reserve(images.size());
  render_finished_semaphores.reserve(images.size());

  vk::SemaphoreCreateInfo const semaphore_create_info{};

  for (vk::Image image : images) {
    vk::ImageViewCreateInfo view_create_info{
      .image = image,
      .viewType = vk::ImageViewType::e2D,
      .format = surface_format.format,
      .components =
        vk::ComponentMapping{
          .r = vk::ComponentSwizzle::eIdentity,
          .g = vk::ComponentSwizzle::eIdentity,
          .b = vk::ComponentSwizzle::eIdentity,
          .a = vk::ComponentSwizzle::eIdentity,
        },
      .subresourceRange =
        vk::ImageSubresourceRange{
          .aspectMask = vk::ImageAspectFlagBits::eColor,
          .baseMipLevel = 0U,
          .levelCount = 1U,
          .baseArrayLayer = 0U,
          .layerCount = 1U,
        },
    };

    image_views.emplace_back(device, view_create_info);
    render_finished_semaphores.emplace_back(device, semaphore_create_info);
  }

  GGEMS_INFOEX("Vulkan", 1,
               "Vulkan swapchain created: images={}, format={}, extent={}x{}, "
               "present mode={}.",
               images.size(), vk::to_string(surface_format.format),
               extent.width, extent.height, vk::to_string(present_mode));

  return SwapchainGeneration{
    .swapchain = std::move(swapchain),
    .images = std::move(images),
    .image_views = std::move(image_views),
    .render_finished_semaphores = std::move(render_finished_semaphores),
    .image_format = surface_format.format,
    .extent = extent,
  };
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanPresenter::CreateCommandPool() -> void {
  vk::CommandPoolCreateInfo create_info{
    .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
    .queueFamilyIndex = device_.GetGraphicsQueueFamily(),
  };

  command_pool_ = vk::raii::CommandPool{device_.GetDevice(), create_info};

  GGEMS_INFOEX("Vulkan", 2,
               "Vulkan command pool created for graphics family {}.",
               device_.GetGraphicsQueueFamily());
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanPresenter::CreateFrameSlots() -> void {
  vk::raii::Device const &device = device_.GetDevice();

  vk::CommandBufferAllocateInfo allocate_info{
    .commandPool = *command_pool_,
    .level = vk::CommandBufferLevel::ePrimary,
    .commandBufferCount = k_frame_slot_count_,
  };

  std::vector<vk::raii::CommandBuffer> command_buffers =
    device.allocateCommandBuffers(allocate_info);

  vk::SemaphoreCreateInfo const semaphore_create_info{};

  vk::FenceCreateInfo const fence_create_info{
    .flags = vk::FenceCreateFlagBits::eSignaled};

  for (std::size_t i = 0U; i < frame_slots_.size(); ++i) {
    frame_slots_[i] = FrameSlot{
      .command_buffer = std::move(command_buffers[i]),
      .image_available_semaphore =
        vk::raii::Semaphore{device, semaphore_create_info},
      .submit_fence = vk::raii::Fence{device, fence_create_info},
    };
  }

  GGEMS_INFOEX("Vulkan", 2, "Created {} Vulkan frame slots.",
               frame_slots_.size());
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanPresenter::TransitionSwapchainImageLayout(
  vk::ImageLayout old_layout, vk::ImageLayout new_layout) -> void {
  vk::PipelineStageFlags2 source_stage{
    vk::PipelineStageFlagBits2::eColorAttachmentOutput};
  vk::AccessFlags2 source_access{vk::AccessFlagBits2::eNone};

  vk::PipelineStageFlags2 destination_stage{
    vk::PipelineStageFlagBits2::eColorAttachmentOutput};
  vk::AccessFlags2 destination_access{
    vk::AccessFlagBits2::eColorAttachmentWrite};

  if (new_layout == vk::ImageLayout::ePresentSrcKHR) {
    source_access = vk::AccessFlagBits2::eColorAttachmentWrite;
    destination_stage = vk::PipelineStageFlagBits2::eNone;
    destination_access = vk::AccessFlagBits2::eNone;
  }

  vk::ImageMemoryBarrier2 image_barrier{
    .srcStageMask = source_stage,
    .srcAccessMask = source_access,
    .dstStageMask = destination_stage,
    .dstAccessMask = destination_access,
    .oldLayout = old_layout,
    .newLayout = new_layout,
    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .image = generation_.images[acquired_image_->index],
    .subresourceRange =
      vk::ImageSubresourceRange{
        .aspectMask = vk::ImageAspectFlagBits::eColor,
        .baseMipLevel = 0U,
        .levelCount = 1U,
        .baseArrayLayer = 0U,
        .layerCount = 1U,
      },
  };

  vk::DependencyInfo dependency_info{
    .imageMemoryBarrierCount = 1U,
    .pImageMemoryBarriers = &image_barrier,
  };

  frame_slots_[frame_slot_index_].command_buffer.pipelineBarrier2(
    dependency_info);
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanPresenter::AcquireImage() -> void {
  if (acquired_image_.has_value()) {
    throw ggems::core::GGEMSRecoverable(
      "A previous GuiMode frame was abandoned after acquiring a Vulkan "
      "swapchain image; rendering cannot continue.");
  }

  FrameSlot const &slot = frame_slots_[frame_slot_index_];

  vk::Result wait_result = device_.GetDevice().waitForFences(
    *slot.submit_fence, vk::True, std::numeric_limits<std::uint64_t>::max());

  if (!(wait_result == vk::Result::eSuccess)) {
    throw ggems::core::GGEMSRecoverable(
      std::format("Unable to wait for the Vulkan frame slot fence: {}.",
                  vk::to_string(wait_result)));
  }

  auto [result, image_index] = generation_.swapchain.acquireNextImage(
    std::numeric_limits<std::uint64_t>::max(), *slot.image_available_semaphore,
    nullptr);

  if (!(result == vk::Result::eSuccess ||
        result == vk::Result::eSuboptimalKHR)) {
    throw ggems::core::GGEMSRecoverable(
      std::format("Unable to acquire a Vulkan swapchain image: {}.",
                  vk::to_string(result)));
  }

  acquired_image_ = AcquiredImage{
    .index = image_index,
    .suboptimal = result == vk::Result::eSuboptimalKHR,
  };
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanPresenter::BeginRecording() -> vk::raii::CommandBuffer const & {
  vk::raii::CommandBuffer const &command_buffer =
    frame_slots_[frame_slot_index_].command_buffer;

  command_buffer.reset();

  vk::CommandBufferBeginInfo begin_info{};
  command_buffer.begin(begin_info);

  return command_buffer;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanPresenter::BeginMainPass() -> void {
  TransitionSwapchainImageLayout(vk::ImageLayout::eUndefined,
                                 vk::ImageLayout::eColorAttachmentOptimal);

  vk::ClearValue clear_value =
    vk::ClearColorValue(ToVulkanClearColor(render::BLUE_Abyss));

  vk::RenderingAttachmentInfo color_attachment{
    .imageView = *generation_.image_views[acquired_image_->index],
    .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
    .loadOp = vk::AttachmentLoadOp::eClear,
    .storeOp = vk::AttachmentStoreOp::eStore,
    .clearValue = clear_value,
  };

  vk::RenderingInfo rendering_info{
    .renderArea =
      vk::Rect2D{
        .offset = vk::Offset2D{.x = 0, .y = 0},
        .extent = generation_.extent,
      },
    .layerCount = 1U,
    .colorAttachmentCount = 1U,
    .pColorAttachments = &color_attachment,
  };

  frame_slots_[frame_slot_index_].command_buffer.beginRendering(rendering_info);
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanPresenter::EndFrame() -> bool {
  FrameSlot const &slot = frame_slots_[frame_slot_index_];
  AcquiredImage const acquired = *acquired_image_;

  slot.command_buffer.endRendering();

  TransitionSwapchainImageLayout(vk::ImageLayout::eColorAttachmentOptimal,
                                 vk::ImageLayout::ePresentSrcKHR);

  slot.command_buffer.end();

  std::array<vk::Semaphore, 1U> const wait_semaphores{
    *slot.image_available_semaphore};

  std::array<vk::PipelineStageFlags, 1U> const wait_stages{
    vk::PipelineStageFlagBits::eColorAttachmentOutput};

  std::array<vk::CommandBuffer, 1U> const command_buffers{*slot.command_buffer};

  std::array<vk::Semaphore, 1U> const signal_semaphores{
    *generation_.render_finished_semaphores[acquired.index]};

  vk::SubmitInfo submit_info{
    .waitSemaphoreCount = 1U,
    .pWaitSemaphores = wait_semaphores.data(),
    .pWaitDstStageMask = wait_stages.data(),
    .commandBufferCount = 1U,
    .pCommandBuffers = command_buffers.data(),
    .signalSemaphoreCount = 1U,
    .pSignalSemaphores = signal_semaphores.data(),
  };

  device_.GetDevice().resetFences(*slot.submit_fence);
  device_.GetGraphicsQueue().submit(submit_info, *slot.submit_fence);

  // The slot's work is fenced from here on: a failed presentation must not
  // hide the submission.
  acquired_image_.reset();
  frame_slot_index_ = (frame_slot_index_ + 1U) % k_frame_slot_count_;

  std::array<vk::SwapchainKHR, 1U> const swapchains{*generation_.swapchain};

  vk::PresentInfoKHR present_info{
    .waitSemaphoreCount = 1U,
    .pWaitSemaphores = signal_semaphores.data(),
    .swapchainCount = 1U,
    .pSwapchains = swapchains.data(),
    .pImageIndices = &acquired.index,
  };

  vk::Result present_result =
    device_.GetPresentationQueue().presentKHR(present_info);

  return acquired.suboptimal || present_result == vk::Result::eSuboptimalKHR;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanPresenter::RecreateSwapchain(
  vk::Extent2D const &framebuffer_extent) -> void {
  SwapchainSupportDetails const support_details = QuerySwapchainSupport();

  vk::Extent2D const extent =
    ChooseSwapchainExtent(support_details.capabilities, framebuffer_extent);

  if (extent.width == 0U || extent.height == 0U) {
    return;
  }

  device_.GetDevice().waitIdle();

  SwapchainGeneration generation =
    CreateSwapchainGeneration(support_details, extent, *generation_.swapchain);

  std::swap(generation_, generation);

  GGEMS_INFOEX("Vulkan", 1,
               "Vulkan swapchain recreated after framebuffer resize.");
}

} // namespace ggems::ui::detail
