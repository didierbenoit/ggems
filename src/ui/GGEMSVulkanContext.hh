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
  ~GGEMSVulkanContext() noexcept = default;

  GGEMSVulkanContext(GGEMSVulkanContext const &) = delete;
  GGEMSVulkanContext(GGEMSVulkanContext &&) = delete;
  GGEMSVulkanContext &operator=(GGEMSVulkanContext const &) = delete;
  GGEMSVulkanContext &operator=(GGEMSVulkanContext &&) = delete;

public:
  void Initialise(GLFWwindow *window);
  [[nodiscard]] bool IsInitialised() const noexcept;

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

private:
  vk::raii::Context context_{};
  vk::raii::Instance instance_{nullptr};
  vk::raii::DebugUtilsMessengerEXT debug_messenger_{nullptr};
  vk::raii::SurfaceKHR surface_{nullptr};
  vk::raii::PhysicalDevice physical_device_{nullptr};

  QueueFamilyIndices queue_family_indices_{};

  bool initialised_{false};
};
} // namespace ggems::ui
