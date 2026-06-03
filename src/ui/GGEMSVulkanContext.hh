#pragma once

#include <vector>

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
  void CreateInstance();
  void SetupDebugMessenger();
  void CreateSurface(GLFWwindow *window);

  [[nodiscard]] std::vector<char const *> GetRequiredInstanceExtensions() const;

  static VKAPI_ATTR VkBool32 VKAPI_CALL
  DebugVkCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
                  vk::DebugUtilsMessageTypeFlagsEXT type,
                  vk::DebugUtilsMessengerCallbackDataEXT const *callback_data,
                  void *user_data) noexcept;

private:
  vk::raii::Context context_{};
  vk::raii::Instance instance_{nullptr};
  vk::raii::DebugUtilsMessengerEXT debug_messenger_{nullptr};
  vk::raii::SurfaceKHR surface_{nullptr};

  bool initialised_{false};
};
} // namespace ggems::ui
