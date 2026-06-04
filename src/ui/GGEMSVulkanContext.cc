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
  } catch (vk::SystemError const &error) {
    GGEMS_RECOVERABLE(
        std::format("Unable to initialise Vulkan GuiMode: {}.", error.what()));
  }

  initialised_ = true;

  GGEMS_INFO("Vulkan",
             "Vulkan instance and GLFW presentation surface initialised.");
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
  std::vector<vk::SurfaceFormatKHR> surface_formats =
      physical_device.getSurfaceFormatsKHR(*surface_);

  std::vector<vk::PresentModeKHR> present_modes =
      physical_device.getSurfacePresentModesKHR(*surface_);

  return !surface_formats.empty() && !present_modes.empty();
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

} // namespace ggems::ui
