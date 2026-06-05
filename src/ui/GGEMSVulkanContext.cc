#include "GGEMSVulkanContext.hh"

#include <array>
#include <cstdio>
#include <cstring>
#include <format>
#include <ranges>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <limits>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSMacros.hh"

namespace {

#ifdef GGEMS_DEBUG_MODE
constexpr bool k_enable_validation_layers{true};
#else
constexpr bool k_enable_validation_layers{false};
#endif

constexpr std::array<char const *, 1> k_validation_layers{
    "VK_LAYER_KHRONOS_validation"};

constexpr std::array<char const *, 1> k_required_device_extensions{
    vk::KHRSwapchainExtensionName};
} // namespace

namespace ggems::ui {
/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSVulkanContext::~GGEMSVulkanContext() noexcept {
  if (device_ == nullptr) {
    return;
  }

  try {
    device_.waitIdle();
  } catch (...) {
    std::fputs(
        "[GGEMS Vulkan] Failed to wait for device idle during shutdown.\n",
        stderr);
  }
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSVulkanContext::Initialise(GLFWwindow *window) {
  if (initialised_) {
    return;
  }

  GGEMS_CHECK_INTERNAL(
      window != nullptr,
      "A valid GLFW window is required before initialising Vulkan GuiMode.");

  try {
    CreateInstance();
    SetupDebugMessenger();
    CreateSurface(window);
    SelectPhysicalDevice();
    CreateLogicalDevice();
    CreateSwapchain(window);
    CreateSwapchainImageViews();
    CreateCommandPool();
    AllocateCommandBuffers();
    CreateSyncObjects();
  } catch (vk::SystemError const &error) {
    GGEMS_RECOVERABLE(
        std::format("Unable to initialise Vulkan GuiMode: {}.", error.what()));
  }

  initialised_ = true;

  GGEMS_INFO("Vulkan",
             "Vulkan swapchain command buffers and synchronisation objects "
             "initialised.");
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

bool GGEMSVulkanContext::IsInitialised() const noexcept { return initialised_; }

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSVulkanContext::CreateInstance() {
  constexpr vk::ApplicationInfo application_info{
      .pApplicationName = "GGEMS GuiMode",
      .applicationVersion = VK_MAKE_API_VERSION(0, 2, 0, 0),
      .pEngineName = "GGEMS",
      .engineVersion = VK_MAKE_API_VERSION(0, 2, 0, 0),
      .apiVersion = vk::ApiVersion13};

  std::vector<char const *> required_layers{};

  if (k_enable_validation_layers) {
    required_layers.assign(k_validation_layers.begin(),
                           k_validation_layers.end());
  }

  std::vector<vk::LayerProperties> available_layers =
      context_.enumerateInstanceLayerProperties();

  for (char const *required_layer : required_layers) {
    bool is_available = std::ranges::any_of(
        available_layers, [required_layer](vk::LayerProperties const &layer) {
          return std::strcmp(layer.layerName, required_layer) == 0;
        });

    GGEMS_CHECK_RECOVERABLE(
        is_available,
        std::format("Required Vulkan validation layer '{}' is unavailable.",
                    required_layer));
  }

  std::vector<char const *> required_extensions =
      GetRequiredInstanceExtensions();

  std::vector<vk::ExtensionProperties> available_extensions =
      context_.enumerateInstanceExtensionProperties();

  for (char const *required_extension : required_extensions) {
    bool is_available = std::ranges::any_of(
        available_extensions,
        [required_extension](vk::ExtensionProperties const &extension) {
          return std::strcmp(extension.extensionName, required_extension) == 0;
        });

    GGEMS_CHECK_RECOVERABLE(
        is_available,
        std::format("Required Vulkan instance extension '{}' is unavailable.",
                    required_extension));
  }

  vk::InstanceCreateInfo create_info{
      .pApplicationInfo = &application_info,
      .enabledLayerCount = static_cast<std::uint32_t>(required_layers.size()),
      .ppEnabledLayerNames = required_layers.data(),
      .enabledExtensionCount =
          static_cast<std::uint32_t>(required_extensions.size()),
      .ppEnabledExtensionNames = required_extensions.data()};

  instance_ = vk::raii::Instance{context_, create_info};
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::vector<char const *>
GGEMSVulkanContext::GetRequiredInstanceExtensions() const {
  std::uint32_t extension_count{0};

  char const **glfw_extensions =
      glfwGetRequiredInstanceExtensions(&extension_count);

  GGEMS_CHECK_RECOVERABLE(
      glfw_extensions != nullptr && extension_count > 0,
      "GLFW did not report the Vulkan instance extensions required by "
      "GGEMS GuiMode.");

  std::vector<char const *> extensions{glfw_extensions,
                                       glfw_extensions + extension_count};

  if (k_enable_validation_layers) {
    extensions.push_back(vk::EXTDebugUtilsExtensionName);
  }

  return extensions;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSVulkanContext::SetupDebugMessenger() {
  if (!k_enable_validation_layers) {
    return;
  }

  vk::DebugUtilsMessageSeverityFlagsEXT severity_flags{
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eError};

  vk::DebugUtilsMessageTypeFlagsEXT message_type_flags{
      vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
      vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
      vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation};

  vk::DebugUtilsMessengerCreateInfoEXT create_info{
      .messageSeverity = severity_flags,
      .messageType = message_type_flags,
      .pfnUserCallback = &GGEMSVulkanContext::DebugVkCallback};

  debug_messenger_ = instance_.createDebugUtilsMessengerEXT(create_info);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSVulkanContext::CreateSurface(GLFWwindow *window) {
  VkSurfaceKHR raw_surface{VK_NULL_HANDLE};

  VkResult result =
      glfwCreateWindowSurface(*instance_, window, nullptr, &raw_surface);

  GGEMS_CHECK_RECOVERABLE(
      result == VK_SUCCESS,
      std::format("Unable to create the Vulkan GLFW surface: {}.",
                  vk::to_string(static_cast<vk::Result>(result))));

  surface_ = vk::raii::SurfaceKHR{instance_, raw_surface};
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

VKAPI_ATTR VkBool32 VKAPI_CALL GGEMSVulkanContext::DebugVkCallback(
    vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
    vk::DebugUtilsMessageTypeFlagsEXT type,
    vk::DebugUtilsMessengerCallbackDataEXT const *callback_data,
    void *) noexcept {
  if (callback_data == nullptr || callback_data->pMessage == nullptr) {
    return vk::False;
  }

  try {
    if (severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eError) {
      GGEMS_ERROR("Vulkan", "Validation layer [{}]: {}", vk::to_string(type),
                  callback_data->pMessage);
    } else if (severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning) {
      GGEMS_WARN("Vulkan", "Validation layer [{}]: {}", vk::to_string(type),
                 callback_data->pMessage);
    }
  } catch (...) {
    std::fputs("[GGEMS Vulkan] Failed to record a validation message.\n",
               stderr);
  }

  return vk::False;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSVulkanContext::QueueFamilyIndices GGEMSVulkanContext::FindQueueFamilies(
    vk::raii::PhysicalDevice const &physical_device) const {
  QueueFamilyIndices indices{};

  std::vector<vk::QueueFamilyProperties> const queue_family_properties =
      physical_device.getQueueFamilyProperties();

  for (std::uint32_t index = 0;
       index < static_cast<std::uint32_t>(queue_family_properties.size());
       ++index) {
    vk::QueueFamilyProperties const &properties =
        queue_family_properties[index];

    if (!indices.graphics.has_value() &&
        static_cast<bool>(properties.queueFlags &
                          vk::QueueFlagBits::eGraphics)) {
      indices.graphics = index;
    }

    if (!indices.presentation.has_value() &&
        physical_device.getSurfaceSupportKHR(index, *surface_) == vk::True) {
      indices.presentation = index;
    }

    if (indices.IsComplete()) {
      break;
    }
  }

  return indices;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

bool GGEMSVulkanContext::SupportsRequiredDeviceExtensions(
    vk::raii::PhysicalDevice const &physical_device) const {
  std::vector<vk::ExtensionProperties> const available_extensions =
      physical_device.enumerateDeviceExtensionProperties();

  return std::ranges::all_of(
      k_required_device_extensions,
      [&available_extensions](char const *required_extension) {
        return std::ranges::any_of(
            available_extensions,
            [required_extension](vk::ExtensionProperties const &extension) {
              return std::strcmp(extension.extensionName, required_extension) ==
                     0;
            });
      });
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

bool GGEMSVulkanContext::SupportsRequiredFeatures(
    vk::raii::PhysicalDevice const &physical_device) const {
  auto features =
      physical_device.getFeatures2<vk::PhysicalDeviceFeatures2,
                                   vk::PhysicalDeviceVulkan13Features>();

  vk::PhysicalDeviceVulkan13Features const &vulkan_13_features =
      features.get<vk::PhysicalDeviceVulkan13Features>();

  return vulkan_13_features.dynamicRendering == vk::True &&
         vulkan_13_features.synchronization2 == vk::True;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

bool GGEMSVulkanContext::SupportsSwapchain(
    vk::raii::PhysicalDevice const &physical_device) const {
  SwapchainSupportDetails details = QuerySwapchainSupport(physical_device);

  return !details.surface_formats.empty() && !details.present_modes.empty();
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

GGEMSVulkanContext::SwapchainSupportDetails
GGEMSVulkanContext::QuerySwapchainSupport(
    vk::raii::PhysicalDevice const &physical_device) const {
  SwapchainSupportDetails details{};

  details.capabilities = physical_device.getSurfaceCapabilitiesKHR(*surface_);
  details.surface_formats = physical_device.getSurfaceFormatsKHR(*surface_);
  details.present_modes = physical_device.getSurfacePresentModesKHR(*surface_);

  return details;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

bool GGEMSVulkanContext::IsPhysicalDeviceSuitable(
    vk::raii::PhysicalDevice const &physical_device) const {
  vk::PhysicalDeviceProperties properties = physical_device.getProperties();

  bool supports_vulkan_1_3 = properties.apiVersion >= vk::ApiVersion13;

  QueueFamilyIndices queue_family_indices = FindQueueFamilies(physical_device);

  return supports_vulkan_1_3 && queue_family_indices.IsComplete() &&
         SupportsRequiredDeviceExtensions(physical_device) &&
         SupportsRequiredFeatures(physical_device) &&
         SupportsSwapchain(physical_device);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

std::uint32_t GGEMSVulkanContext::ScorePhysicalDevice(
    vk::raii::PhysicalDevice const &physical_device) const {
  switch (physical_device.getProperties().deviceType) {
  case vk::PhysicalDeviceType::eIntegratedGpu:
    return 400;

  case vk::PhysicalDeviceType::eDiscreteGpu:
    return 300;

  case vk::PhysicalDeviceType::eVirtualGpu:
    return 200;

  case vk::PhysicalDeviceType::eCpu:
    return 100;

  case vk::PhysicalDeviceType::eOther:
  default:
    return 0;
  }
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSVulkanContext::SelectPhysicalDevice() {
  std::vector<vk::raii::PhysicalDevice> physical_devices =
      instance_.enumeratePhysicalDevices();

  GGEMS_CHECK_RECOVERABLE(
      !physical_devices.empty(),
      "No Vulkan physical device is available for GGEMS GuiMode.");

  std::uint32_t best_score{std::numeric_limits<std::uint32_t>::min()};
  bool device_selected{false};

  for (vk::raii::PhysicalDevice const &physical_device : physical_devices) {
    vk::PhysicalDeviceProperties const properties =
        physical_device.getProperties();

    if (!IsPhysicalDeviceSuitable(physical_device)) {
      GGEMS_DEBUG("Vulkan",
                  "Rejected Vulkan display device '{}': incompatible with "
                  "GGEMS GuiMode requirements.",
                  properties.deviceName.data());
      continue;
    }

    std::uint32_t score = ScorePhysicalDevice(physical_device);

    GGEMS_DEBUG("Vulkan",
                "Compatible Vulkan display device '{}': type={}, score={}.",
                properties.deviceName.data(),
                vk::to_string(properties.deviceType), score);

    if (!device_selected || score > best_score) {
      physical_device_ = physical_device;
      queue_family_indices_ = FindQueueFamilies(physical_device);
      best_score = score;
      device_selected = true;
    }
  }

  GGEMS_CHECK_RECOVERABLE(
      device_selected,
      "No Vulkan physical device satisfies the GGEMS GuiMode requirements.");

  vk::PhysicalDeviceProperties const selected_properties =
      physical_device_.getProperties();

  GGEMS_INFO("Vulkan",
             "Selected Vulkan display device '{}': type={}, API={}.{}.{}.",
             selected_properties.deviceName.data(),
             vk::to_string(selected_properties.deviceType),
             VK_API_VERSION_MAJOR(selected_properties.apiVersion),
             VK_API_VERSION_MINOR(selected_properties.apiVersion),
             VK_API_VERSION_PATCH(selected_properties.apiVersion));

  GGEMS_INFO("Vulkan",
             "Selected Vulkan queue families: graphics={}, presentation={}, "
             "separate={}.",
             queue_family_indices_.graphics.value(),
             queue_family_indices_.presentation.value(),
             queue_family_indices_.UsesSeparateFamilies());
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSVulkanContext::CreateLogicalDevice() {
  GGEMS_CHECK_INTERNAL(
      queue_family_indices_.IsComplete(),
      "Vulkan queue families must be identified before creating the logical "
      "device.");

  float queue_priority{1.0f};

  std::vector<vk::DeviceQueueCreateInfo> queue_create_infos{};
  queue_create_infos.reserve(queue_family_indices_.UsesSeparateFamilies() ? 2U
                                                                          : 1U);
  queue_create_infos.push_back(vk::DeviceQueueCreateInfo{
      .queueFamilyIndex = queue_family_indices_.graphics.value(),
      .queueCount = 1U,
      .pQueuePriorities = &queue_priority});

  if (queue_family_indices_.UsesSeparateFamilies()) {
    queue_create_infos.push_back(vk::DeviceQueueCreateInfo{
        .queueFamilyIndex = queue_family_indices_.presentation.value(),
        .queueCount = 1U,
        .pQueuePriorities = &queue_priority});
  }

  vk::PhysicalDeviceFeatures device_features{};

  vk::PhysicalDeviceVulkan13Features vulkan_13_features{
      .synchronization2 = vk::True, .dynamicRendering = vk::True};

  vk::DeviceCreateInfo create_info{
      .pNext = &vulkan_13_features,
      .queueCreateInfoCount =
          static_cast<std::uint32_t>(queue_create_infos.size()),
      .pQueueCreateInfos = queue_create_infos.data(),
      .enabledExtensionCount =
          static_cast<std::uint32_t>(k_required_device_extensions.size()),
      .ppEnabledExtensionNames = k_required_device_extensions.data(),
      .pEnabledFeatures = &device_features};

  device_ = vk::raii::Device{physical_device_, create_info};

  graphics_queue_ =
      vk::raii::Queue{device_, queue_family_indices_.graphics.value(), 0U};

  presentation_queue_ =
      vk::raii::Queue{device_, queue_family_indices_.presentation.value(), 0U};

  GGEMS_INFO("Vulkan",
             "Vulkan logical device created: graphics queue family={}, "
             "presentation queue family={}, separate={}.",
             queue_family_indices_.graphics.value(),
             queue_family_indices_.presentation.value(),
             queue_family_indices_.UsesSeparateFamilies());
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

vk::SurfaceFormatKHR GGEMSVulkanContext::ChooseSwapchainSurfaceFormat(
    std::vector<vk::SurfaceFormatKHR> const &surface_formats) const {
  GGEMS_CHECK_INTERNAL(!surface_formats.empty(),
                       "No Vulkan surface format is available for GuiMode.");

  for (vk::SurfaceFormatKHR const &surface_format : surface_formats) {
    if (surface_format.format == vk::Format::eB8G8R8A8Srgb &&
        surface_format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
      return surface_format;
    }
  }

  return surface_formats.front();
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

vk::PresentModeKHR GGEMSVulkanContext::ChooseSwapchainPresentMode(
    std::vector<vk::PresentModeKHR> const &present_modes) const {
  GGEMS_CHECK_INTERNAL(!present_modes.empty(),
                       "No Vulkan present mode is available for GuiMode.");

  for (vk::PresentModeKHR const &present_mode : present_modes) {
    if (present_mode == vk::PresentModeKHR::eFifo) {
      return present_mode;
    }
  }

  return present_modes.front();
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

vk::Extent2D GGEMSVulkanContext::ChooseSwapchainExtent(
    vk::SurfaceCapabilitiesKHR const &capabilities, GLFWwindow *window) const {
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

  vk::Extent2D actual_extent{.width = static_cast<std::uint32_t>(width),
                             .height = static_cast<std::uint32_t>(height)};

  actual_extent.width =
      std::clamp(actual_extent.width, capabilities.minImageExtent.width,
                 capabilities.maxImageExtent.width);

  actual_extent.height =
      std::clamp(actual_extent.height, capabilities.minImageExtent.height,
                 capabilities.maxImageExtent.height);

  return actual_extent;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSVulkanContext::CreateSwapchain(GLFWwindow *window) {
  GGEMS_CHECK_INTERNAL(
      physical_device_ != nullptr && device_ != nullptr,
      "A Vulkan physical device and logical device are required before"
      "creating the GuiMode swapchain.");

  SwapchainSupportDetails support_details =
      QuerySwapchainSupport(physical_device_);

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
      queue_family_indices_.graphics.value(),
      queue_family_indices_.presentation.value()};

  vk::SharingMode image_sharing_mode{vk::SharingMode::eExclusive};
  std::uint32_t queue_family_index_count{0};
  std::uint32_t *queue_family_index_data{nullptr};

  if (queue_family_indices_.UsesSeparateFamilies()) {
    image_sharing_mode = vk::SharingMode::eConcurrent;
    queue_family_index_count =
        static_cast<std::uint32_t>(queue_family_indices.size());
    queue_family_index_data = queue_family_indices.data();
  }

  vk::SwapchainCreateInfoKHR create_info{
      .surface = *surface_,
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
      .compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
      .presentMode = present_mode,
      .clipped = vk::True};

  swapchain_ = vk::raii::SwapchainKHR{device_, create_info};
  swapchain_images_ = swapchain_.getImages();
  swapchain_image_format_ = surface_format.format;
  swapchain_extent_ = extent;

  GGEMS_INFO("Vulkan",
             "Vulkan swapchain created: images={}, format={}, extent={}x{}, "
             "present mode={}.",
             swapchain_images_.size(), vk::to_string(swapchain_image_format_),
             swapchain_extent_.width, swapchain_extent_.height,
             vk::to_string(present_mode));
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSVulkanContext::CreateSwapchainImageViews() {
  GGEMS_CHECK_INTERNAL(
      swapchain_ != nullptr,
      "A Vulkan swapchain is required before creating swapchain image views.");

  GGEMS_CHECK_INTERNAL(
      swapchain_image_format_ != vk::Format::eUndefined,
      "A valid Vulkan swapchain image format is required before creating "
      "swapchain image views.");

  swapchain_image_views_.clear();
  swapchain_image_views_.reserve(swapchain_images_.size());

  for (vk::Image image : swapchain_images_) {
    vk::ImageViewCreateInfo create_info{
        .image = image,
        .viewType = vk::ImageViewType::e2D,
        .format = swapchain_image_format_,
        .components =
            vk::ComponentMapping{.r = vk::ComponentSwizzle::eIdentity,
                                 .g = vk::ComponentSwizzle::eIdentity,
                                 .b = vk::ComponentSwizzle::eIdentity,
                                 .a = vk::ComponentSwizzle::eIdentity},
        .subresourceRange = vk::ImageSubresourceRange{
            .aspectMask = vk::ImageAspectFlagBits::eColor,
            .baseMipLevel = 0U,
            .levelCount = 1U,
            .baseArrayLayer = 0U,
            .layerCount = 1U}};

    swapchain_image_views_.emplace_back(device_, create_info);
  }

  GGEMS_INFO("Vulkan", "Created {} Vulkan swapchain image views.",
             swapchain_image_views_.size());
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSVulkanContext::CreateCommandPool() {
  GGEMS_CHECK_INTERNAL(
      queue_family_indices_.graphics.has_value(),
      "A Vulkan graphics queue family is required before creating the"
      "GuiMode command pool.");

  vk::CommandPoolCreateInfo create_info{
      .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
      .queueFamilyIndex = queue_family_indices_.graphics.value()};

  command_pool_ = vk::raii::CommandPool{device_, create_info};

  GGEMS_INFO("Vulkan", "Vulkan command pool created for graphics family {}.",
             queue_family_indices_.graphics.value());
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSVulkanContext::AllocateCommandBuffers() {
  GGEMS_CHECK_INTERNAL(
      command_pool_ != nullptr,
      "A Vulkan command pool is required before allocating GuiMode command "
      "buffers.");

  GGEMS_CHECK_INTERNAL(
      !swapchain_images_.empty(),
      "Swapchain_images_are_required_before_allocating GuiMode command "
      "buffers.");

  vk::CommandBufferAllocateInfo allocate_info{
      .commandPool = *command_pool_,
      .level = vk::CommandBufferLevel::ePrimary,
      .commandBufferCount =
          static_cast<std::uint32_t>(swapchain_images_.size())};

  command_buffers_ = device_.allocateCommandBuffers(allocate_info);

  GGEMS_INFO("Vulkan", "Allocated {} Vulkan command buffers.",
             command_buffers_.size());
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSVulkanContext::CreateSyncObjects() {
  vk::SemaphoreCreateInfo semaphore_create_info{};

  vk::FenceCreateInfo fence_create_info{.flags =
                                            vk::FenceCreateFlagBits::eSignaled};

  image_available_semaphores_.clear();
  render_finished_semaphores_.clear();
  in_flight_fences_.clear();

  image_available_semaphores_.reserve(k_max_frames_in_flight_);
  render_finished_semaphores_.reserve(k_max_frames_in_flight_);
  in_flight_fences_.reserve(k_max_frames_in_flight_);

  for (std::uint32_t i = 0U; i < k_max_frames_in_flight_; ++i) {
    image_available_semaphores_.emplace_back(device_, semaphore_create_info);
    render_finished_semaphores_.emplace_back(device_, semaphore_create_info);
    in_flight_fences_.emplace_back(device_, fence_create_info);
  }

  GGEMS_INFO("Vulkan",
             "Create Vulkan synchronisation objects for {} frames in flight.",
             k_max_frames_in_flight_);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSVulkanContext::TransitionSwapchainImageLayout(
    std::uint32_t image_index, vk::ImageLayout old_layout,
    vk::ImageLayout new_layout) {
  vk::PipelineStageFlags2 source_stage{vk::PipelineStageFlagBits2::eNone};
  vk::AccessFlags2 source_access{vk::AccessFlagBits2::eNone};

  vk::PipelineStageFlags2 destination_stage{
      vk::PipelineStageFlagBits2::eColorAttachmentOutput};
  vk::AccessFlags2 destination_access{
      vk::AccessFlagBits2::eColorAttachmentWrite};

  if (new_layout == vk::ImageLayout::ePresentSrcKHR) {
    source_stage = vk::PipelineStageFlagBits2::eColorAttachmentOutput;
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
      .image = swapchain_images_[image_index],
      .subresourceRange = vk::ImageSubresourceRange{
          .aspectMask = vk::ImageAspectFlagBits::eColor,
          .baseMipLevel = 0U,
          .levelCount = 1U,
          .baseArrayLayer = 0U,
          .layerCount = 1U}};

  vk::DependencyInfo dependency_info{.imageMemoryBarrierCount = 1U,
                                     .pImageMemoryBarriers = &image_barrier};

  command_buffers_[image_index].pipelineBarrier2(dependency_info);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSVulkanContext::RecordCommandBuffer(std::uint32_t image_index) {
  vk::raii::CommandBuffer &command_buffer = command_buffers_[image_index];

  command_buffer.reset();

  vk::CommandBufferBeginInfo begin_info{};
  command_buffer.begin(begin_info);

  TransitionSwapchainImageLayout(image_index, vk::ImageLayout::eUndefined,
                                 vk::ImageLayout::eColorAttachmentOptimal);

  vk::ClearValue clear_value = vk::ClearColorValue(0.08f, 0.09f, 0.11f, 1.0f);

  vk::RenderingAttachmentInfo colour_attachment{
      .imageView = *swapchain_image_views_[image_index],
      .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
      .loadOp = vk::AttachmentLoadOp::eClear,
      .storeOp = vk::AttachmentStoreOp::eStore,
      .clearValue = clear_value};

  vk::RenderingInfo rendering_info{
      .renderArea = vk::Rect2D{.offset = vk::Offset2D{.x = 0, .y = 0},
                               .extent = swapchain_extent_},
      .layerCount = 1U,
      .colorAttachmentCount = 1U,
      .pColorAttachments = &colour_attachment};

  command_buffer.beginRendering(rendering_info);
  command_buffer.endRendering();

  TransitionSwapchainImageLayout(image_index,
                                 vk::ImageLayout::eColorAttachmentOptimal,
                                 vk::ImageLayout::ePresentSrcKHR);

  command_buffer.end();
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void GGEMSVulkanContext::RenderFrame() {
  GGEMS_CHECK_INTERNAL(
      initialised_,
      "Vulkan GuiMode must be initialised before rendering a frame.");

  vk::Result wait_result =
      device_.waitForFences(*in_flight_fences_[current_frame_], vk::True,
                            std::numeric_limits<std::uint64_t>::max());

  GGEMS_CHECK_RECOVERABLE(
      wait_result == vk::Result::eSuccess,
      std::format("Unable to wait for the Vulkan in-flight fence: {}.",
                  vk::to_string(wait_result)));

  auto [result, image_index] = swapchain_.acquireNextImage(
      std::numeric_limits<std::uint64_t>::max(),
      *image_available_semaphores_[current_frame_], nullptr);

  GGEMS_CHECK_RECOVERABLE(
      result == vk::Result::eSuccess || result == vk::Result::eSuboptimalKHR,
      std::format("Unable to acquire a Vulkan swapchain image: {}.",
                  vk::to_string(result)));

  device_.resetFences(*in_flight_fences_[current_frame_]);

  RecordCommandBuffer(image_index);

  vk::Semaphore wait_semaphores[]{*image_available_semaphores_[current_frame_]};

  vk::PipelineStageFlags wait_stages[]{
      vk::PipelineStageFlagBits::eColorAttachmentOutput};

  vk::CommandBuffer command_buffers[]{*command_buffers_[image_index]};

  vk::Semaphore signal_semaphores[]{
      *render_finished_semaphores_[current_frame_]};

  vk::SubmitInfo submit_info{.waitSemaphoreCount = 1U,
                             .pWaitSemaphores = wait_semaphores,
                             .pWaitDstStageMask = wait_stages,
                             .commandBufferCount = 1U,
                             .pCommandBuffers = command_buffers,
                             .signalSemaphoreCount = 1U,
                             .pSignalSemaphores = signal_semaphores};

  graphics_queue_.submit(submit_info, *in_flight_fences_[current_frame_]);

  vk::SwapchainKHR swapchains[]{*swapchain_};

  vk::PresentInfoKHR present_info{.waitSemaphoreCount = 1U,
                                  .pWaitSemaphores = signal_semaphores,
                                  .swapchainCount = 1U,
                                  .pSwapchains = swapchains,
                                  .pImageIndices = &image_index};

  vk::Result present_result = presentation_queue_.presentKHR(present_info);

  GGEMS_CHECK_RECOVERABLE(
      present_result == vk::Result::eSuccess ||
          present_result == vk::Result::eSuboptimalKHR,
      std::format("Unable to present a Vulkan swapchain image: {}.",
                  vk::to_string(present_result)));

  current_frame_ = (current_frame_ + 1U) % k_max_frames_in_flight_;
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

} // namespace ggems::ui
