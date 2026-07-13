#pragma once

#include <vector>
#include <cstdint>
#include <mutex>
#include <optional>

#include <vulkan/vulkan_raii.hpp>

#include "GGEMSDeviceStatus.hh"
#include "GGEMSImGuiLayer.hh"
#include "GGEMSVulkanSceneRenderer.hh"
#include "GGEMSVulkanDeviceSelection.hh"

#include "GGEMS/render/GGEMSParticleTrace.hh"

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
  void Initialise(GLFWwindow *window,
                  detail::GGEMSVulkanDeviceSelector const &device_selector,
                  detail::GGEMSComputeStatus compute_status);

  [[nodiscard]] bool IsInitialised() const noexcept;

  void RenderFrame(GLFWwindow *window, bool framebuffer_resized);

  void SubmitParticleTraceSegments(
      std::vector<ggems::render::GGEMSParticleTraceSegment> segments);
  void ClearParticleTraces();

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

#if VK_HEADER_VERSION >= 304
  static VKAPI_ATTR VkBool32 VKAPI_CALL
  DebugVkCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
                  vk::DebugUtilsMessageTypeFlagsEXT type,
                  vk::DebugUtilsMessengerCallbackDataEXT const *callback_data,
                  void *user_data) noexcept;
#else
  static VKAPI_ATTR VkBool32 VKAPI_CALL
  DebugVkCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                  VkDebugUtilsMessageTypeFlagsEXT type,
                  VkDebugUtilsMessengerCallbackDataEXT const *callback_data,
                  void *user_data) noexcept;
#endif

  void SelectPhysicalDevice(
      detail::GGEMSVulkanDeviceSelector const &device_selector,
      std::optional<detail::GGEMSVulkanDisplayAdapter> const &display_adapter);

  [[nodiscard]] detail::GGEMSVulkanDeviceCandidate
  BuildPhysicalDeviceCandidate(vk::raii::PhysicalDevice const &physical_device,
                               std::uint32_t enumeration_index) const;

  [[nodiscard]] QueueFamilyIndices
  FindQueueFamilies(vk::raii::PhysicalDevice const &physical_device) const;

  [[nodiscard]] bool SupportsRequiredDeviceExtensions(
      vk::raii::PhysicalDevice const &physical_device) const;

  [[nodiscard]] bool SupportsRequiredFeatures(
      vk::raii::PhysicalDevice const &physical_device) const;

  [[nodiscard]] bool
  SupportsSwapchain(vk::raii::PhysicalDevice const &physical_device) const;

  void WarnIfCrossAdapterPresentation(
      GLFWwindow *window,
      std::optional<detail::GGEMSVulkanDisplayAdapter> const &display_adapter)
      const;

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

  void CreateFrameSyncObjects();
  void CreateSwapchainSyncObjects();

  void RecordCommandBuffer(std::uint32_t image_index);

  void TransitionSwapchainImageLayout(std::uint32_t image_index,
                                      vk::ImageLayout old_layout,
                                      vk::ImageLayout new_layout);

  void CleanupSwapchain();
  void RecreateSwapchain(GLFWwindow *window);

  void CreateImGuiDescriptorPool();
  void InitialiseImGui(GLFWwindow *window);
  void ShutdownImGui() noexcept;
  void BuildImGuiFrame();
  void ApplyPendingParticleTraceSegments();

  void LoadImGuiFonts();

  static void CheckImGuiVkResult(VkResult result) noexcept;

  void InitialiseSceneRenderer();
  void ShutdownSceneRenderer() noexcept;

private:
  vk::raii::Context context_{};
  vk::raii::Instance instance_{nullptr};
  vk::raii::DebugUtilsMessengerEXT debug_messenger_{nullptr};
  vk::raii::SurfaceKHR surface_{nullptr};
  vk::raii::PhysicalDevice physical_device_{nullptr};
  vk::raii::Device device_{nullptr};
  vk::raii::Queue graphics_queue_{nullptr};
  vk::raii::Queue presentation_queue_{nullptr};
  vk::raii::DescriptorPool imgui_descriptor_pool_{nullptr};
  vk::raii::CommandPool command_pool_{nullptr};
  vk::raii::SwapchainKHR swapchain_{nullptr};
  std::vector<vk::Image> swapchain_images_{};
  std::vector<vk::raii::ImageView> swapchain_image_views_{};
  std::vector<vk::raii::CommandBuffer> command_buffers_{};

  std::vector<vk::raii::Semaphore> image_available_semaphores_{};
  std::vector<vk::raii::Semaphore> render_finished_semaphores_{};
  std::vector<vk::raii::Fence> in_flight_fences_{};
  std::vector<vk::Fence> swapchain_image_in_flight_fences_{};
  std::uint32_t current_frame_{0U};

  vk::Format swapchain_image_format_{vk::Format::eUndefined};
  vk::Extent2D swapchain_extent_{};

  VkFormat imgui_colour_attachment_format_{VK_FORMAT_UNDEFINED};
  VkPipelineRenderingCreateInfo imgui_pipeline_rendering_create_info_{};

  GGEMSImGuiLayer imgui_layer_{};
  GGEMSVulkanSceneRenderer scene_renderer_{};
  detail::GGEMSDeviceStatusSnapshot device_status_{};

  std::mutex pending_particle_trace_mutex_{};
  std::vector<ggems::render::GGEMSParticleTraceSegment>
      pending_particle_trace_segments_{};
  bool has_pending_particle_trace_segments_{false};
  bool pending_particle_trace_clear_{false};

  float imgui_ui_scale_{1.0f};
  float imgui_font_size_{15.0f};
  bool imgui_initialised_{false};

  QueueFamilyIndices queue_family_indices_{};
  detail::GGEMSVulkanDeviceCandidate selected_physical_device_candidate_{};

  bool initialised_{false};

  static constexpr std::uint32_t k_vulkan_api_version_{vk::ApiVersion13};
  static constexpr std::uint32_t k_max_frames_in_flight_{2U};
};
} // namespace ggems::ui
