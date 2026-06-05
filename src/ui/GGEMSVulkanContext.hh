#pragma once

#include <vector>
#include <cstdint>
#include <optional>

#include <vulkan/vulkan_raii.hpp>

struct GLFWwindow;

namespace ggems::ui {
class GGEMSVulkanContext {
public:
  GGEMSVulkanContext() = default;
  ~GGEMSVulkanContext() noexcept;

  GGEMSVulkanContext(GGEMSVulkanContext const &) = delete;
  GGEMSVulkanContext(GGEMSVulkanContext &&) = delete;
  GGEMSVulkanContext &operator=(GGEMSVulkanContext const &) = delete;
  GGEMSVulkanContext &operator=(GGEMSVulkanContext &&) = delete;

public:
  void Initialise(GLFWwindow *window);
  [[nodiscard]] bool IsInitialised() const noexcept;

  void RenderFrame();

private:
  struct QueueFamilyIndices {
    std::optional<std::uint32_t> graphics{};
    std::optional<std::uint32_t> presentation{};

    [[nodiscard]] bool IsComplete() const noexcept {
      return graphics.has_value() && presentation.has_value();
    }

    [[nodiscard]] bool UsesSeparateFamilies() const noexcept {
      return IsComplete() && graphics.value() != presentation.value();
    }
  };

  struct SwapchainSupportDetails {
    vk::SurfaceCapabilitiesKHR capabilities{};
    std::vector<vk::SurfaceFormatKHR> surface_formats{};
    std::vector<vk::PresentModeKHR> present_modes{};
  };

private:
  void CreateInstance();
  void SetupDebugMessenger();
  void CreateSurface(GLFWwindow *window);

  [[nodiscard]] std::vector<char const *> GetRequiredInstanceExtensions() const;

  static VKAPI_ATTR VkBool32 VKAPI_CALL
  DebugVkCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
                  vk::DebugUtilsMessageTypeFlagsEXT type,
                  vk::DebugUtilsMessengerCallbackDataEXT const *callback_data,
                  void *user_data) noexcept;

  void SelectPhysicalDevice();

  [[nodiscard]] bool IsPhysicalDeviceSuitable(
      vk::raii::PhysicalDevice const &physical_device) const;

  [[nodiscard]] QueueFamilyIndices
  FindQueueFamilies(vk::raii::PhysicalDevice const &physical_device) const;

  [[nodiscard]] bool SupportsRequiredDeviceExtensions(
      vk::raii::PhysicalDevice const &physical_device) const;

  [[nodiscard]] bool SupportsRequiredFeatures(
      vk::raii::PhysicalDevice const &physical_device) const;

  [[nodiscard]] bool
  SupportsSwapchain(vk::raii::PhysicalDevice const &physical_device) const;

  [[nodiscard]] std::uint32_t
  ScorePhysicalDevice(vk::raii::PhysicalDevice const &physical_device) const;

  void CreateLogicalDevice();

  void CreateSwapchain(GLFWwindow *window);
  void CreateSwapchainImageViews();

  [[nodiscard]] SwapchainSupportDetails
  QuerySwapchainSupport(vk::raii::PhysicalDevice const &physical_device) const;

  [[nodiscard]] vk::SurfaceFormatKHR ChooseSwapchainSurfaceFormat(
      std::vector<vk::SurfaceFormatKHR> const &surface_formats) const;

  [[nodiscard]] vk::PresentModeKHR ChooseSwapchainPresentMode(
      std::vector<vk::PresentModeKHR> const &present_modes) const;

  [[nodiscard]] vk::Extent2D
  ChooseSwapchainExtent(vk::SurfaceCapabilitiesKHR const &capabilities,
                        GLFWwindow *window) const;

  void CreateCommandPool();
  void AllocateCommandBuffers();

  void CreateSyncObjects();

  void RecordCommandBuffer(std::uint32_t image_index);

  void TransitionSwapchainImageLayout(std::uint32_t image_index,
                                      vk::ImageLayout old_layout,
                                      vk::ImageLayout new_layout);

private:
  vk::raii::Context context_{};
  vk::raii::Instance instance_{nullptr};
  vk::raii::DebugUtilsMessengerEXT debug_messenger_{nullptr};
  vk::raii::SurfaceKHR surface_{nullptr};
  vk::raii::PhysicalDevice physical_device_{nullptr};
  vk::raii::Device device_{nullptr};
  vk::raii::Queue graphics_queue_{nullptr};
  vk::raii::Queue presentation_queue_{nullptr};
  vk::raii::CommandPool command_pool_{nullptr};
  vk::raii::SwapchainKHR swapchain_{nullptr};
  std::vector<vk::Image> swapchain_images_{};
  std::vector<vk::raii::ImageView> swapchain_image_views_{};
  std::vector<vk::raii::CommandBuffer> command_buffers_{};
  std::vector<vk::raii::Semaphore> image_available_semaphores_{};
  std::vector<vk::raii::Semaphore> render_finished_semaphores_{};
  std::vector<vk::raii::Fence> in_flight_fences_{};
  std::uint32_t current_frame_{0U};
  vk::Format swapchain_image_format_{vk::Format::eUndefined};
  vk::Extent2D swapchain_extent_{};

  QueueFamilyIndices queue_family_indices_{};

  bool initialised_{false};

  static constexpr std::uint32_t k_max_frames_in_flight_{2U};
};
} // namespace ggems::ui
