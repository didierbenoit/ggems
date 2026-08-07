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
#include "GGEMS/core/sources/GGEMSSourceRunSnapshot.hh"

struct GLFWwindow;

namespace ggems::ui {
class GGEMSVulkanContext {
public:
  GGEMSVulkanContext() = default;
  ~GGEMSVulkanContext() noexcept;

  GGEMSVulkanContext(GGEMSVulkanContext const &) = delete;
  GGEMSVulkanContext(GGEMSVulkanContext &&) = delete;
  auto operator=(GGEMSVulkanContext const &) -> GGEMSVulkanContext & = delete;
  auto operator=(GGEMSVulkanContext &&) -> GGEMSVulkanContext & = delete;

  void Initialize(GLFWwindow *window,
                  detail::GGEMSVulkanDeviceSelector const &device_selector,
                  detail::GGEMSComputeStatus compute_status);

  [[nodiscard]] auto IsInitialized() const noexcept -> bool;

  auto RenderFrame(GLFWwindow *window, bool framebuffer_resized) -> void;

  auto SubmitSourceRunSnapshot(core::sources::GGEMSSourceRunSnapshot snapshot)
      -> void;

  auto SubmitParticleTraceSegments(
      std::vector<ggems::render::GGEMSParticleTraceSegment> segments) -> void;
  auto ClearParticleTraces() -> void;

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

  struct SwapchainSupportDetails {
    vk::SurfaceCapabilitiesKHR capabilities{};
    std::vector<vk::SurfaceFormatKHR> surface_formats;
    std::vector<vk::PresentModeKHR> present_modes;
  };

  void CreateInstance();
  void SetupDebugMessenger();
  void CreateSurface(GLFWwindow *window);

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
      detail::GGEMSVulkanDeviceSelector const &device_selector,
      std::optional<detail::GGEMSVulkanDisplayAdapter> const &display_adapter)
      -> void;

  [[nodiscard]] auto
  BuildPhysicalDeviceCandidate(vk::raii::PhysicalDevice const &physical_device,
                               std::uint32_t enumeration_index) const
      -> detail::GGEMSVulkanDeviceCandidate;

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
      std::optional<detail::GGEMSVulkanDisplayAdapter> const &display_adapter)
      const -> void;

  auto CreateLogicalDevice() -> void;

  auto CreateSwapchain(GLFWwindow *window) -> void;
  auto CreateSwapchainImageViews() -> void;

  [[nodiscard]] auto
  QuerySwapchainSupport(vk::raii::PhysicalDevice const &physical_device) const
      -> SwapchainSupportDetails;

  [[nodiscard]] static auto ChooseSwapchainSurfaceFormat(
      std::vector<vk::SurfaceFormatKHR> const &surface_formats)
      -> vk::SurfaceFormatKHR;

  [[nodiscard]] static auto ChooseSwapchainPresentMode(
      std::vector<vk::PresentModeKHR> const &present_modes)
      -> vk::PresentModeKHR;

  [[nodiscard]] static auto
  ChooseSwapchainExtent(vk::SurfaceCapabilitiesKHR const &capabilities,
                        GLFWwindow *window) -> vk::Extent2D;

  auto CreateCommandPool() -> void;
  auto AllocateCommandBuffers() -> void;

  auto CreateFrameSyncObjects() -> void;
  auto CreateSwapchainSyncObjects() -> void;

  auto RecordCommandBuffer(std::uint32_t image_index) -> void;

  auto TransitionSwapchainImageLayout(std::uint32_t image_index,
                                      vk::ImageLayout old_layout,
                                      vk::ImageLayout new_layout) -> void;

  auto CleanupSwapchain() -> void;
  auto RecreateSwapchain(GLFWwindow *window) -> void;

  auto CreateImGuiDescriptorPool() -> void;
  auto InitializeImGui(GLFWwindow *window) -> void;
  auto ShutdownImGui() noexcept -> void;
  auto BuildImGuiFrame() -> void;
  auto ApplyPendingSourceRunSnapshot() -> void;
  auto ApplyPendingParticleTraceSegments() -> void;

  auto LoadImGuiFonts() -> void;

  static auto CheckImGuiVkResult(VkResult result) noexcept -> void;

  auto InitializeSceneRenderer() -> void;
  auto ShutdownSceneRenderer() noexcept -> void;

  vk::raii::Context context_;
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

  VkFormat imgui_color_attachment_format_{VK_FORMAT_UNDEFINED};
  VkPipelineRenderingCreateInfo imgui_pipeline_rendering_create_info_{};

  GGEMSImGuiLayer imgui_layer_;
  GGEMSVulkanSceneRenderer scene_renderer_;
  detail::GGEMSDeviceStatusSnapshot device_status_{};

  std::mutex pending_source_run_snapshot_mutex_;
  std::optional<core::sources::GGEMSSourceRunSnapshot>
      pending_source_run_snapshot_;

  std::mutex pending_particle_trace_mutex_;
  std::vector<ggems::render::GGEMSParticleTraceSegment>
      pending_particle_trace_segments_;
  bool has_pending_particle_trace_segments_{false};
  bool pending_particle_trace_clear_{false};

  float imgui_ui_scale_{1.0F};
  float imgui_font_size_{15.0F};
  bool imgui_initialized_{false};

  QueueFamilyIndices queue_family_indices_{};
  detail::GGEMSVulkanDeviceCandidate selected_physical_device_candidate_{};

  bool initialized_{false};

  static constexpr std::uint32_t k_vulkan_api_version_{vk::ApiVersion13};
  static constexpr std::uint32_t k_max_frames_in_flight_{2U};
};
} // namespace ggems::ui
