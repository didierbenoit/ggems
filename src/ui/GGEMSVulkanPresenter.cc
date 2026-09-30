#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <format>
#include <limits>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

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

GGEMSVulkanPresenter::GGEMSVulkanPresenter(GGEMSVulkanDevice const &device,
                                           GLFWwindow *window)
    : device_{device} {
  CreateSwapchain(window);
  CreateSwapchainImageViews();
  CreateCommandPool();
  AllocateCommandBuffers();
  CreateFrameSyncObjects();
  CreateSwapchainSyncObjects();
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanPresenter::GetImageCount() const noexcept -> std::uint32_t {
  return static_cast<std::uint32_t>(swapchain_images_.size());
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanPresenter::GetImageFormat() const noexcept -> vk::Format {
  return swapchain_image_format_;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanPresenter::GetExtent() const noexcept -> vk::Extent2D const & {
  return swapchain_extent_;
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
  vk::SurfaceCapabilitiesKHR const &capabilities, GLFWwindow *window)
  -> vk::Extent2D {
  if (capabilities.currentExtent.width !=
      std::numeric_limits<std::uint32_t>::max()) {
    return capabilities.currentExtent;
  }

  int width{0};
  int height{0};

  glfwGetFramebufferSize(window, &width, &height);

  while (width == 0 || height == 0) {
    glfwWaitEvents();
    glfwGetFramebufferSize(window, &width, &height);
  }

  vk::Extent2D actual_extent{
    .width = static_cast<std::uint32_t>(width),
    .height = static_cast<std::uint32_t>(height),
  };

  actual_extent.width =
    std::clamp(actual_extent.width, capabilities.minImageExtent.width,
               capabilities.maxImageExtent.width);

  actual_extent.height =
    std::clamp(actual_extent.height, capabilities.minImageExtent.height,
               capabilities.maxImageExtent.height);

  return actual_extent;
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

auto GGEMSVulkanPresenter::CreateSwapchain(GLFWwindow *window) -> void {
  SwapchainSupportDetails support_details = QuerySwapchainSupport();

  vk::SurfaceFormatKHR surface_format =
    ChooseSwapchainSurfaceFormat(support_details.surface_formats);

  vk::PresentModeKHR present_mode =
    ChooseSwapchainPresentMode(support_details.present_modes);

  vk::Extent2D extent =
    ChooseSwapchainExtent(support_details.capabilities, window);

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
  };

  swapchain_ = vk::raii::SwapchainKHR{device_.GetDevice(), create_info};
  swapchain_images_ = swapchain_.getImages();
  swapchain_image_format_ = surface_format.format;
  swapchain_extent_ = extent;

  GGEMS_INFOEX("Vulkan", 1,
               "Vulkan swapchain created: images={}, format={}, extent={}x{}, "
               "present mode={}.",
               swapchain_images_.size(), vk::to_string(swapchain_image_format_),
               swapchain_extent_.width, swapchain_extent_.height,
               vk::to_string(present_mode));
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanPresenter::CreateSwapchainImageViews() -> void {
  swapchain_image_views_.clear();
  swapchain_image_views_.reserve(swapchain_images_.size());

  for (vk::Image image : swapchain_images_) {
    vk::ImageViewCreateInfo create_info{
      .image = image,
      .viewType = vk::ImageViewType::e2D,
      .format = swapchain_image_format_,
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

    swapchain_image_views_.emplace_back(device_.GetDevice(), create_info);
  }

  GGEMS_INFOEX("Vulkan", 2, "Created {} Vulkan swapchain image views.",
               swapchain_image_views_.size());
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

auto GGEMSVulkanPresenter::AllocateCommandBuffers() -> void {
  vk::CommandBufferAllocateInfo allocate_info{
    .commandPool = *command_pool_,
    .level = vk::CommandBufferLevel::ePrimary,
    .commandBufferCount = static_cast<std::uint32_t>(swapchain_images_.size()),
  };

  command_buffers_ = device_.GetDevice().allocateCommandBuffers(allocate_info);

  GGEMS_INFOEX("Vulkan", 2, "Allocated {} Vulkan command buffers.",
               command_buffers_.size());
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanPresenter::CreateFrameSyncObjects() -> void {
  vk::SemaphoreCreateInfo const semaphore_create_info{};

  vk::FenceCreateInfo const fence_create_info{
    .flags = vk::FenceCreateFlagBits::eSignaled};

  image_available_semaphores_.clear();
  in_flight_fences_.clear();

  image_available_semaphores_.reserve(k_max_frames_in_flight_);
  in_flight_fences_.reserve(k_max_frames_in_flight_);

  for (std::uint32_t i = 0U; i < k_max_frames_in_flight_; ++i) {
    image_available_semaphores_.emplace_back(device_.GetDevice(),
                                             semaphore_create_info);
    in_flight_fences_.emplace_back(device_.GetDevice(), fence_create_info);
  }

  GGEMS_INFOEX(
    "Vulkan", 2,
    "Created Vulkan frame synchronization objects for {} frames in flight.",
    k_max_frames_in_flight_);
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanPresenter::CreateSwapchainSyncObjects() -> void {
  vk::SemaphoreCreateInfo const semaphore_create_info{};

  render_finished_semaphores_.clear();
  render_finished_semaphores_.reserve(swapchain_images_.size());

  for (std::size_t i = 0U; i < swapchain_images_.size(); ++i) {
    render_finished_semaphores_.emplace_back(device_.GetDevice(),
                                             semaphore_create_info);
  }

  swapchain_image_in_flight_fences_.assign(swapchain_images_.size(),
                                           vk::Fence{nullptr});

  GGEMS_INFOEX(
    "Vulkan", 2,
    "Created {} Vulkan render-finished semaphores for swapchain images.",
    render_finished_semaphores_.size());
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
    .image = swapchain_images_[acquired_image_index_],
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

  command_buffers_[acquired_image_index_].pipelineBarrier2(dependency_info);
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanPresenter::AcquireImage() -> void {
  vk::raii::Device const &device = device_.GetDevice();

  vk::Result wait_result =
    device.waitForFences(*in_flight_fences_[current_frame_], vk::True,
                         std::numeric_limits<std::uint64_t>::max());

  if (!(wait_result == vk::Result::eSuccess)) {
    throw ggems::core::GGEMSRecoverable(
      std::format("Unable to wait for the Vulkan in-flight fence: {}.",
                  vk::to_string(wait_result)));
  }

  auto [result, image_index] = swapchain_.acquireNextImage(
    std::numeric_limits<std::uint64_t>::max(),
    *image_available_semaphores_[current_frame_], nullptr);

  if (!(result == vk::Result::eSuccess ||
        result == vk::Result::eSuboptimalKHR)) {
    throw ggems::core::GGEMSRecoverable(
      std::format("Unable to acquire a Vulkan swapchain image: {}.",
                  vk::to_string(result)));
  }

  if (!(image_index < swapchain_image_in_flight_fences_.size())) {
    throw ggems::core::GGEMSInternal(
      "The acquired Vulkan swapchain image index exceeds "
      "the number of tracked "
      "in-flight image fences.");
  }

  vk::Fence const image_in_flight_fence =
    swapchain_image_in_flight_fences_[image_index];

  if (image_in_flight_fence != vk::Fence{nullptr}) {
    vk::Result const wait_image_result =
      device.waitForFences(image_in_flight_fence, vk::True,
                           std::numeric_limits<std::uint64_t>::max());

    if (!(wait_image_result == vk::Result::eSuccess)) {
      throw ggems::core::GGEMSRecoverable(
        std::format("Unable to wait for the Vulkan swapchain image fence: {}.",
                    vk::to_string(wait_image_result)));
    }
  }

  swapchain_image_in_flight_fences_[image_index] =
    *in_flight_fences_[current_frame_];

  acquired_image_index_ = image_index;
  acquire_suboptimal_ = result == vk::Result::eSuboptimalKHR;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanPresenter::BeginRecording() -> vk::raii::CommandBuffer const & {
  vk::raii::CommandBuffer const &command_buffer =
    command_buffers_[acquired_image_index_];

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
    .imageView = *swapchain_image_views_[acquired_image_index_],
    .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
    .loadOp = vk::AttachmentLoadOp::eClear,
    .storeOp = vk::AttachmentStoreOp::eStore,
    .clearValue = clear_value,
  };

  vk::RenderingInfo rendering_info{
    .renderArea =
      vk::Rect2D{
        .offset = vk::Offset2D{.x = 0, .y = 0},
        .extent = swapchain_extent_,
      },
    .layerCount = 1U,
    .colorAttachmentCount = 1U,
    .pColorAttachments = &color_attachment,
  };

  command_buffers_[acquired_image_index_].beginRendering(rendering_info);
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanPresenter::EndFrame() -> bool {
  vk::raii::CommandBuffer const &command_buffer =
    command_buffers_[acquired_image_index_];

  command_buffer.endRendering();

  TransitionSwapchainImageLayout(vk::ImageLayout::eColorAttachmentOptimal,
                                 vk::ImageLayout::ePresentSrcKHR);

  command_buffer.end();

  device_.GetDevice().resetFences(*in_flight_fences_[current_frame_]);

  std::array<vk::Semaphore, 1U> const wait_semaphores{
    *image_available_semaphores_[current_frame_]};

  std::array<vk::PipelineStageFlags, 1U> const wait_stages{
    vk::PipelineStageFlagBits::eColorAttachmentOutput};

  std::array<vk::CommandBuffer, 1U> const command_buffers{
    *command_buffers_[acquired_image_index_]};

  std::array<vk::Semaphore, 1U> signal_semaphores{
    *render_finished_semaphores_[acquired_image_index_]};

  vk::SubmitInfo submit_info{
    .waitSemaphoreCount = 1U,
    .pWaitSemaphores = wait_semaphores.data(),
    .pWaitDstStageMask = wait_stages.data(),
    .commandBufferCount = 1U,
    .pCommandBuffers = command_buffers.data(),
    .signalSemaphoreCount = 1U,
    .pSignalSemaphores = signal_semaphores.data(),
  };

  device_.GetGraphicsQueue().submit(submit_info,
                                    *in_flight_fences_[current_frame_]);

  std::array<vk::SwapchainKHR, 1U> swapchains{*swapchain_};

  vk::PresentInfoKHR present_info{
    .waitSemaphoreCount = 1U,
    .pWaitSemaphores = signal_semaphores.data(),
    .swapchainCount = 1U,
    .pSwapchains = swapchains.data(),
    .pImageIndices = &acquired_image_index_,
  };

  vk::Result present_result =
    device_.GetPresentationQueue().presentKHR(present_info);

  current_frame_ = (current_frame_ + 1U) % k_max_frames_in_flight_;

  return acquire_suboptimal_ || present_result == vk::Result::eSuboptimalKHR;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanPresenter::CleanupSwapchain() -> void {
  command_buffers_.clear();

  swapchain_image_views_.clear();
  swapchain_images_.clear();

  render_finished_semaphores_.clear();
  swapchain_image_in_flight_fences_.clear();

  swapchain_ = nullptr;
  swapchain_image_format_ = vk::Format::eUndefined;
  swapchain_extent_ = vk::Extent2D{};
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanPresenter::RecreateSwapchain(GLFWwindow *window) -> void {
  int width{0};
  int height{0};

  glfwGetFramebufferSize(window, &width, &height);

  while ((width == 0 || height == 0) &&
         glfwWindowShouldClose(window) == GLFW_FALSE) {
    glfwWaitEvents();
    glfwGetFramebufferSize(window, &width, &height);
  }

  if (glfwWindowShouldClose(window) == GLFW_TRUE) {
    return;
  }

  device_.GetDevice().waitIdle();

  CleanupSwapchain();

  CreateSwapchain(window);
  CreateSwapchainImageViews();
  AllocateCommandBuffers();
  CreateSwapchainSyncObjects();

  current_frame_ = 0U;

  GGEMS_INFOEX("Vulkan", 1,
               "Vulkan swapchain recreated after framebuffer resize.");
}

} // namespace ggems::ui::detail
