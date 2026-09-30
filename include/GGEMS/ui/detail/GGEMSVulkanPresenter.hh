#pragma once

#include <cstdint>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

struct GLFWwindow;

namespace ggems::ui::detail {

class GGEMSVulkanDevice;

class GGEMSVulkanPresenter {
public:
  GGEMSVulkanPresenter(GGEMSVulkanDevice const &device, GLFWwindow *window);
  ~GGEMSVulkanPresenter() = default;

  GGEMSVulkanPresenter(GGEMSVulkanPresenter const &) = delete;
  GGEMSVulkanPresenter(GGEMSVulkanPresenter &&) = delete;
  auto operator=(GGEMSVulkanPresenter const &)
    -> GGEMSVulkanPresenter & = delete;
  auto operator=(GGEMSVulkanPresenter &&) -> GGEMSVulkanPresenter & = delete;

  auto RecreateSwapchain(GLFWwindow *window) -> void;

  [[nodiscard]] auto GetImageCount() const noexcept -> std::uint32_t;
  [[nodiscard]] auto GetImageFormat() const noexcept -> vk::Format;
  [[nodiscard]] auto GetExtent() const noexcept -> vk::Extent2D const &;

  auto AcquireImage() -> void;
  [[nodiscard]] auto BeginRecording() -> vk::raii::CommandBuffer const &;
  auto BeginMainPass() -> void;

  [[nodiscard]] auto EndFrame() -> bool;

private:
  struct SwapchainSupportDetails {
    vk::SurfaceCapabilitiesKHR capabilities{};
    std::vector<vk::SurfaceFormatKHR> surface_formats;
    std::vector<vk::PresentModeKHR> present_modes;
  };

  auto CreateSwapchain(GLFWwindow *window) -> void;
  auto CreateSwapchainImageViews() -> void;

  [[nodiscard]] auto QuerySwapchainSupport() const -> SwapchainSupportDetails;

  [[nodiscard]] static auto ChooseSwapchainSurfaceFormat(
    std::vector<vk::SurfaceFormatKHR> const &surface_formats)
    -> vk::SurfaceFormatKHR;

  [[nodiscard]] static auto ChooseSwapchainPresentMode(
    std::vector<vk::PresentModeKHR> const &present_modes) -> vk::PresentModeKHR;

  [[nodiscard]] static auto
  ChooseSwapchainExtent(vk::SurfaceCapabilitiesKHR const &capabilities,
                        GLFWwindow *window) -> vk::Extent2D;

  auto CreateCommandPool() -> void;
  auto AllocateCommandBuffers() -> void;

  auto CreateFrameSyncObjects() -> void;
  auto CreateSwapchainSyncObjects() -> void;

  auto TransitionSwapchainImageLayout(vk::ImageLayout old_layout,
                                      vk::ImageLayout new_layout) -> void;

  auto CleanupSwapchain() -> void;

  GGEMSVulkanDevice const &device_;

  vk::raii::CommandPool command_pool_{nullptr};
  vk::raii::SwapchainKHR swapchain_{nullptr};
  std::vector<vk::Image> swapchain_images_;
  std::vector<vk::raii::ImageView> swapchain_image_views_;
  std::vector<vk::raii::CommandBuffer> command_buffers_;

  std::vector<vk::raii::Semaphore> image_available_semaphores_;
  std::vector<vk::raii::Semaphore> render_finished_semaphores_;
  std::vector<vk::raii::Fence> in_flight_fences_;
  std::vector<vk::Fence> swapchain_image_in_flight_fences_;
  std::uint32_t current_frame_{0U};

  vk::Format swapchain_image_format_{vk::Format::eUndefined};
  vk::Extent2D swapchain_extent_{};

  std::uint32_t acquired_image_index_{0U};
  bool acquire_suboptimal_{false};

  static constexpr std::uint32_t k_max_frames_in_flight_{2U};
};

} // namespace ggems::ui::detail
