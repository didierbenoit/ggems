#include "GGEMSVulkanContext.hh"

#include <array>
#include <cstdio>
#include <cstring>
#include <format>
#include <ranges>
#include <string>
#include <vector>

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

} // namespace ggems::ui
