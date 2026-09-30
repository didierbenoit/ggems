#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "GGEMSDeviceStatus.hh"
#include "GGEMSVulkanDeviceSelection.hh"

struct GLFWwindow;

namespace ggems::ui::detail {

class GGEMSVulkanDevice {
public:
  GGEMSVulkanDevice(GLFWwindow *window,
                    GGEMSVulkanDeviceSelector const &device_selector);
  ~GGEMSVulkanDevice() = default;

  GGEMSVulkanDevice(GGEMSVulkanDevice const &) = delete;
  GGEMSVulkanDevice(GGEMSVulkanDevice &&) = delete;
  auto operator=(GGEMSVulkanDevice const &) -> GGEMSVulkanDevice & = delete;
  auto operator=(GGEMSVulkanDevice &&) -> GGEMSVulkanDevice & = delete;

  [[nodiscard]] auto GetInstance() const noexcept -> vk::raii::Instance const &;
  [[nodiscard]] auto GetPhysicalDevice() const noexcept
    -> vk::raii::PhysicalDevice const &;
  [[nodiscard]] auto GetDevice() const noexcept -> vk::raii::Device const &;
  [[nodiscard]] auto GetSurface() const noexcept
    -> vk::raii::SurfaceKHR const &;
  [[nodiscard]] auto GetGraphicsQueue() const noexcept
    -> vk::raii::Queue const &;
  [[nodiscard]] auto GetPresentationQueue() const noexcept
    -> vk::raii::Queue const &;
  [[nodiscard]] auto GetGraphicsQueueFamily() const noexcept -> std::uint32_t;
  [[nodiscard]] auto GetPresentationQueueFamily() const noexcept
    -> std::uint32_t;
  [[nodiscard]] auto GetStatus() const noexcept
    -> GGEMSVulkanDeviceStatus const &;

private:
  struct QueueFamilyIndices {
    std::optional<std::uint32_t> graphics;
    std::optional<std::uint32_t> presentation;

    [[nodiscard]] auto IsComplete() const noexcept -> bool {
      return graphics.has_value() && presentation.has_value();
    }

    [[nodiscard]] auto UsesSeparateFamilies() const noexcept -> bool {
      return IsComplete() && graphics.value() != presentation.value();
    }
  };

  auto CreateInstance() -> void;
  auto SetupDebugMessenger() -> void;
  auto CreateSurface(GLFWwindow *window) -> void;

  [[nodiscard]] static auto GetRequiredInstanceExtensions()
    -> std::vector<char const *>;

#if VK_HEADER_VERSION >= 304
  static VKAPI_ATTR auto VKAPI_CALL
  DebugVkCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
                  vk::DebugUtilsMessageTypeFlagsEXT type,
                  vk::DebugUtilsMessengerCallbackDataEXT const *callback_data,
                  void *user_data) noexcept -> VkBool32;
#else
  static VKAPI_ATTR auto VKAPI_CALL
  DebugVkCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                  VkDebugUtilsMessageTypeFlagsEXT type,
                  VkDebugUtilsMessengerCallbackDataEXT const *callback_data,
                  void *user_data) noexcept -> VkBool32;
#endif

  auto SelectPhysicalDevice(
    GGEMSVulkanDeviceSelector const &device_selector,
    std::optional<GGEMSVulkanDisplayAdapter> const &display_adapter) -> void;

  [[nodiscard]] auto
  BuildPhysicalDeviceCandidate(vk::raii::PhysicalDevice const &physical_device,
                               std::uint32_t enumeration_index) const
    -> GGEMSVulkanDeviceCandidate;

  [[nodiscard]] auto
  FindQueueFamilies(vk::raii::PhysicalDevice const &physical_device) const
    -> QueueFamilyIndices;

  [[nodiscard]] static auto SupportsRequiredDeviceExtensions(
    vk::raii::PhysicalDevice const &physical_device) -> bool;

  [[nodiscard]] static auto
  SupportsRequiredFeatures(vk::raii::PhysicalDevice const &physical_device)
    -> bool;

  [[nodiscard]] auto
  SupportsSwapchain(vk::raii::PhysicalDevice const &physical_device) const
    -> bool;

  auto WarnIfCrossAdapterPresentation(
    GLFWwindow *window,
    std::optional<GGEMSVulkanDisplayAdapter> const &display_adapter) const
    -> void;

  auto CreateLogicalDevice() -> void;

  vk::raii::Context context_;
  vk::raii::Instance instance_{nullptr};
  vk::raii::DebugUtilsMessengerEXT debug_messenger_{nullptr};
  vk::raii::SurfaceKHR surface_{nullptr};
  vk::raii::PhysicalDevice physical_device_{nullptr};
  vk::raii::Device device_{nullptr};
  vk::raii::Queue graphics_queue_{nullptr};
  vk::raii::Queue presentation_queue_{nullptr};

  QueueFamilyIndices queue_family_indices_{};
  GGEMSVulkanDeviceCandidate selected_physical_device_candidate_{};
  GGEMSVulkanDeviceStatus status_{};

  static constexpr std::uint32_t k_vulkan_api_version_{vk::ApiVersion13};
};

} // namespace ggems::ui::detail
