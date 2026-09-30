#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

namespace ggems::ui::detail {

class GGEMSVulkanDevice;

class GGEMSVulkanPresenter {
public:
  GGEMSVulkanPresenter(GGEMSVulkanDevice const &device,
                       vk::Extent2D const &framebuffer_extent);
  ~GGEMSVulkanPresenter() = default;

  GGEMSVulkanPresenter(GGEMSVulkanPresenter const &) = delete;
  GGEMSVulkanPresenter(GGEMSVulkanPresenter &&) = delete;
  auto operator=(GGEMSVulkanPresenter const &)
    -> GGEMSVulkanPresenter & = delete;
  auto operator=(GGEMSVulkanPresenter &&) -> GGEMSVulkanPresenter & = delete;

  auto RecreateSwapchain(vk::Extent2D const &framebuffer_extent) -> void;

  [[nodiscard]] auto GetImageCount() const noexcept -> std::uint32_t;
  [[nodiscard]] auto GetImageFormat() const noexcept -> vk::Format;
  [[nodiscard]] auto GetExtent() const noexcept -> vk::Extent2D const &;

  auto AcquireImage() -> void;
  [[nodiscard]] auto BeginRecording() -> vk::raii::CommandBuffer const &;
  auto BeginMainPass() -> void;

  [[nodiscard]] auto EndFrame() -> bool;

private:
  static constexpr std::uint32_t k_frame_slot_count_{2U};

  struct SwapchainSupportDetails {
    vk::SurfaceCapabilitiesKHR capabilities{};
    std::vector<vk::SurfaceFormatKHR> surface_formats;
    std::vector<vk::PresentModeKHR> present_modes;
  };

  struct SwapchainGeneration {
    vk::raii::SwapchainKHR swapchain{nullptr};
    std::vector<vk::Image> images;
    std::vector<vk::raii::ImageView> image_views;
    std::vector<vk::raii::Semaphore> render_finished_semaphores;
    vk::Format image_format{vk::Format::eUndefined};
    vk::Extent2D extent{};
  };

  struct FrameSlot {
    vk::raii::CommandBuffer command_buffer{nullptr};
    vk::raii::Semaphore image_available_semaphore{nullptr};
    vk::raii::Fence submit_fence{nullptr};
  };

  struct AcquiredImage {
    std::uint32_t index{0U};
    bool suboptimal{false};
  };

  auto CreateCommandPool() -> void;
  auto CreateFrameSlots() -> void;

  [[nodiscard]] auto CreateSwapchainGeneration(
    SwapchainSupportDetails const &support_details, vk::Extent2D const &extent,
    vk::SwapchainKHR old_swapchain) -> SwapchainGeneration;

  [[nodiscard]] auto QuerySwapchainSupport() const -> SwapchainSupportDetails;

  [[nodiscard]] static auto ChooseSwapchainSurfaceFormat(
    std::vector<vk::SurfaceFormatKHR> const &surface_formats)
    -> vk::SurfaceFormatKHR;

  [[nodiscard]] static auto ChooseSwapchainPresentMode(
    std::vector<vk::PresentModeKHR> const &present_modes) -> vk::PresentModeKHR;

  [[nodiscard]] static auto
  ChooseSwapchainExtent(vk::SurfaceCapabilitiesKHR const &capabilities,
                        vk::Extent2D const &framebuffer_extent) -> vk::Extent2D;

  auto TransitionSwapchainImageLayout(vk::ImageLayout old_layout,
                                      vk::ImageLayout new_layout) -> void;

  GGEMSVulkanDevice const &device_;

  vk::raii::CommandPool command_pool_{nullptr};

  std::array<FrameSlot, k_frame_slot_count_> frame_slots_{};
  SwapchainGeneration generation_{};

  std::uint32_t frame_slot_index_{0U};
  std::optional<AcquiredImage> acquired_image_;
};

} // namespace ggems::ui::detail
