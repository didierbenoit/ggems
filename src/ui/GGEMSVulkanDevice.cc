// *****************************************************************************
// * This file is part of GGEMS.                                               *
// *                                                                           *
// * SPDX-License-Identifier: GPL-3.0-or-later                                 *
// * Copyright (C) 2017-2026 CHRU de Brest, Université de Bretagne Occidentale,*
// * Inserm.                                                                   *
// *                                                                           *
// * GGEMS is free software: you can redistribute it and/or modify             *
// * it under the terms of the GNU General Public License as published by      *
// * the Free Software Foundation, either version 3 of the License, or         *
// * (at your option) any later version.                                       *
// *                                                                           *
// * GGEMS is distributed in the hope that it will be useful,                  *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
// * GNU General Public License for more details.                              *
// *                                                                           *
// * You should have received a copy of the GNU General Public License         *
// * along with GGEMS. If not, see <https://www.gnu.org/licenses/>.            *
// *****************************************************************************

/*!
 * \file
 * \brief Initializes Vulkan and selects a device for the window surface.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <format>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

/*! \brief Requests Vulkan declarations through the GLFW public boundary. */
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "GGEMS/ui/detail/GGEMSVulkanDevice.hh"
#include "GGEMS/ui/detail/GGEMSDeviceStatus.hh"
#include "GGEMS/ui/detail/GGEMSVulkanDeviceSelection.hh"

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/logging/GGEMSLogMacros.hh"

#if defined(_WIN32)
#include "GGEMS/ui/detail/GGEMSVulkanDisplayAdapterWin32.hh"
#endif

namespace {

// =============================================================================
// =============================================================================

#ifdef GGEMS_DEBUG_MODE
/*! \brief Whether this build requests Vulkan validation layers. */
constexpr bool k_enable_validation_layers{true};
#else
/*! \brief Whether this build requests Vulkan validation layers. */
constexpr bool k_enable_validation_layers{false};
#endif

/*! \brief Validation layers required when debug validation is enabled. */
constexpr std::array<char const *, 1> k_validation_layers{
  "VK_LAYER_KHRONOS_validation",
};

#ifdef __APPLE__
/*! \brief Device extensions required for Apple presentation. */
constexpr std::array<char const *, 2> k_required_device_extensions{
  vk::KHRSwapchainExtensionName,
  "VK_KHR_portability_subset",
};
#else
/*! \brief Device extensions required for window presentation. */
constexpr std::array<char const *, 1> k_required_device_extensions{
  vk::KHRSwapchainExtensionName,
};
#endif

/*!
 * \brief Appends a capability failure to the device diagnostic.
 *
 * \param[in,out] diagnostic Accumulated device rejection reasons.
 * \param[in] reason Additional failed capability to report.
 */
auto AppendRejectionReason(std::string &diagnostic, std::string_view reason)
  -> void {
  if (!diagnostic.empty()) {
    diagnostic += "; ";
  }
  diagnostic += reason;
}

// =============================================================================
// =============================================================================

/*!
 * \brief Formats a capability flag for device diagnostics.
 *
 * \param[in] value Capability availability.
 * \return Static yes or no text.
 */
[[nodiscard]] auto YesNo(bool value) noexcept -> std::string_view {
  return value ? "yes" : "no";
}

// =============================================================================
// =============================================================================

/*!
 * \brief Classifies the known device-to-display identity match.
 *
 * \param[in] candidate Vulkan device candidate.
 * \param[in] display_adapter Resolved monitor adapter, when available.
 * \return Static yes, no, unavailable, or unknown label.
 */
[[nodiscard]] auto DisplayMatchLabel(
  ggems::ui::detail::GGEMSVulkanDeviceCandidate const &candidate,
  std::optional<ggems::ui::detail::GGEMSVulkanDisplayAdapter> const
    &display_adapter) noexcept -> std::string_view {
  if (!display_adapter.has_value()) {
    return "unavailable";
  }

  if (!candidate.platform_adapter_id.has_value()) {
    return "unknown";
  }

  return candidate.platform_adapter_id.value() == display_adapter->platform_id
           ? "yes"
           : "no";
}

} // namespace

namespace ggems::ui::detail {

// =============================================================================
// =============================================================================

GGEMSVulkanDevice::GGEMSVulkanDevice(
  GLFWwindow *window, GGEMSVulkanDeviceSelector const &device_selector) {
  std::optional<GGEMSVulkanDisplayAdapter> display_adapter{};

  CreateInstance();
  SetupDebugMessenger();
  CreateSurface(window);

#ifdef _WIN32
  auto display_resolution = ResolveWin32DisplayAdapter(window);

  if (display_resolution.has_value()) {
    display_adapter = display_resolution.value();

    GGEMS_INFO("Vulkan",
               "Display adapter resolved by Win32 HMONITOR/DXGI LUID: {}.",
               display_adapter->name);
  } else {
    GGEMS_INFO("Vulkan",
               "Display adapter identity is unavailable: {} "
               "Automatic selection will use the existing GGEMS fallback. "
               "An explicit Vulkan selector remains available.",
               display_resolution.error());
  }
#else
  GGEMS_INFO(
    "Vulkan",
    "Display adapter identity resolution is unavailable on this platform. "
    "Automatic selection will use the existing GGEMS fallback. "
    "An explicit Vulkan selector remains available.");
#endif

  SelectPhysicalDevice(device_selector, display_adapter);
  CreateLogicalDevice();
  WarnIfCrossAdapterPresentation(window, display_adapter);

  status_.initialized = true;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanDevice::GetInstance() const noexcept
  -> vk::raii::Instance const & {
  return instance_;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanDevice::GetPhysicalDevice() const noexcept
  -> vk::raii::PhysicalDevice const & {
  return physical_device_;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanDevice::GetDevice() const noexcept -> vk::raii::Device const & {
  return device_;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanDevice::GetSurface() const noexcept
  -> vk::raii::SurfaceKHR const & {
  return surface_;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanDevice::GetGraphicsQueue() const noexcept
  -> vk::raii::Queue const & {
  return graphics_queue_;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanDevice::GetPresentationQueue() const noexcept
  -> vk::raii::Queue const & {
  return presentation_queue_;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanDevice::GetGraphicsQueueFamily() const noexcept
  -> std::uint32_t {
  return queue_family_indices_.graphics.value();
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanDevice::GetPresentationQueueFamily() const noexcept
  -> std::uint32_t {
  return queue_family_indices_.presentation.value();
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanDevice::GetStatus() const noexcept
  -> GGEMSVulkanDeviceStatus const & {
  return status_;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanDevice::CreateInstance() -> void {
  constexpr vk::ApplicationInfo application_info{
    .pApplicationName = "GGEMS GuiMode",
    .applicationVersion = vk::makeApiVersion(0, 2, 0, 0),
    .pEngineName = "GGEMS",
    .engineVersion = vk::makeApiVersion(0, 2, 0, 0),
    .apiVersion = k_vulkan_api_version_,
  };

  std::vector<char const *> required_layers{};

  if (k_enable_validation_layers) {
    required_layers.assign(k_validation_layers.begin(),
                           k_validation_layers.end());
  }

  std::vector<vk::LayerProperties> available_layers =
    context_.enumerateInstanceLayerProperties();

  for (char const *required_layer : required_layers) {
    bool is_available = std::ranges::any_of(
      available_layers,
      [required_layer](vk::LayerProperties const &layer) -> bool {
        return std::strcmp(layer.layerName, required_layer) == 0;
      });

    if (!is_available) {
      throw ggems::core::GGEMSRecoverable(
        std::format("Required Vulkan validation layer '{}' is unavailable.",
                    required_layer));
    }
  }

  std::vector<char const *> required_extensions =
    GetRequiredInstanceExtensions();

  std::vector<vk::ExtensionProperties> available_extensions =
    context_.enumerateInstanceExtensionProperties();

  for (char const *required_extension : required_extensions) {
    bool is_available = std::ranges::any_of(
      available_extensions,
      [required_extension](vk::ExtensionProperties const &extension) -> bool {
        return std::strcmp(extension.extensionName, required_extension) == 0;
      });

    if (!is_available) {
      throw ggems::core::GGEMSRecoverable(
        std::format("Required Vulkan instance extension '{}' is unavailable.",
                    required_extension));
    }
  }

  vk::InstanceCreateFlags instance_flags{};

#ifdef __APPLE__
  instance_flags |= vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
#endif

  vk::InstanceCreateInfo create_info{
    .flags = instance_flags,
    .pApplicationInfo = &application_info,
    .enabledLayerCount = static_cast<std::uint32_t>(required_layers.size()),
    .ppEnabledLayerNames = required_layers.data(),
    .enabledExtensionCount =
      static_cast<std::uint32_t>(required_extensions.size()),
    .ppEnabledExtensionNames = required_extensions.data(),
  };

  instance_ = vk::raii::Instance{context_, create_info};
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanDevice::GetRequiredInstanceExtensions()
  -> std::vector<char const *> {
  std::uint32_t extension_count{0};

  char const **glfw_extensions =
    glfwGetRequiredInstanceExtensions(&extension_count);

  if (!(glfw_extensions != nullptr && extension_count > 0)) {
    throw ggems::core::GGEMSRecoverable(
      "GLFW did not report the Vulkan instance extensions required by "
      "GGEMS GuiMode.");
  }

  std::vector<char const *> extensions{glfw_extensions,
                                       glfw_extensions + extension_count};

#ifdef __APPLE__
  extensions.push_back(vk::KHRPortabilityEnumerationExtensionName);
#endif

  if (k_enable_validation_layers) {
    extensions.push_back(vk::EXTDebugUtilsExtensionName);
  }

  return extensions;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanDevice::SetupDebugMessenger() -> void {
  if (!k_enable_validation_layers) {
    return;
  }

  vk::DebugUtilsMessageSeverityFlagsEXT severity_flags{
    vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
      vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
  };

  vk::DebugUtilsMessageTypeFlagsEXT message_type_flags{
    vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
      vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
      vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation,
  };

  vk::DebugUtilsMessengerCreateInfoEXT create_info{
    .messageSeverity = severity_flags,
    .messageType = message_type_flags,
    .pfnUserCallback = &GGEMSVulkanDevice::DebugVkCallback,
  };

  debug_messenger_ = instance_.createDebugUtilsMessengerEXT(create_info);
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanDevice::CreateSurface(GLFWwindow *window) -> void {
  VkSurfaceKHR raw_surface{VK_NULL_HANDLE};

  VkResult result =
    glfwCreateWindowSurface(*instance_, window, nullptr, &raw_surface);

  if (!(result == VK_SUCCESS)) {
    throw ggems::core::GGEMSRecoverable(
      std::format("Unable to create the Vulkan GLFW surface: {}.",
                  vk::to_string(static_cast<vk::Result>(result))));
  }

  surface_ = vk::raii::SurfaceKHR{instance_, raw_surface};
}

// -----------------------------------------------------------------------------

#if VK_HEADER_VERSION >= 304
VKAPI_ATTR auto VKAPI_CALL GGEMSVulkanDevice::DebugVkCallback(
  vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
  vk::DebugUtilsMessageTypeFlagsEXT type,
  vk::DebugUtilsMessengerCallbackDataEXT const *callback_data, void *) noexcept
  -> VkBool32 {
#else
VKAPI_ATTR auto VKAPI_CALL GGEMSVulkanDevice::DebugVkCallback(
  VkDebugUtilsMessageSeverityFlagBitsEXT severity,
  VkDebugUtilsMessageTypeFlagsEXT type,
  VkDebugUtilsMessengerCallbackDataEXT const *callback_data, void *) noexcept
  -> VkBool32 {
#endif
  if (callback_data == nullptr || callback_data->pMessage == nullptr) {
    return VK_FALSE;
  }

  auto message_severity = static_cast<std::uint32_t>(severity);
  auto message_type = static_cast<std::uint32_t>(type);

  try {
    if (message_severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
      GGEMS_ERROR("Vulkan", "Validation layer [{}]: {}", message_type,
                  callback_data->pMessage);
    } else if (message_severity ==
               VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
      GGEMS_WARN("Vulkan", "Validation layer [{}]: {}", message_type,
                 callback_data->pMessage);
    }
  } catch (...) {
    std::fputs("[GGEMS Vulkan] Failed to record a validation message.\n",
               stderr);
  }

  return VK_FALSE;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanDevice::FindQueueFamilies(
  vk::raii::PhysicalDevice const &physical_device) const
  -> GGEMSVulkanDevice::QueueFamilyIndices {
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

// -----------------------------------------------------------------------------

auto GGEMSVulkanDevice::SupportsRequiredDeviceExtensions(
  vk::raii::PhysicalDevice const &physical_device) -> bool {
  std::vector<vk::ExtensionProperties> const available_extensions =
    physical_device.enumerateDeviceExtensionProperties();

  return std::ranges::all_of(
    k_required_device_extensions,
    [&available_extensions](char const *required_extension) -> bool {
      return std::ranges::any_of(
        available_extensions,
        [required_extension](vk::ExtensionProperties const &extension) -> bool {
          return std::strcmp(extension.extensionName, required_extension) == 0;
        });
    });
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanDevice::SupportsRequiredFeatures(
  vk::raii::PhysicalDevice const &physical_device) -> bool {
  auto features =
    physical_device.getFeatures2<vk::PhysicalDeviceFeatures2,
                                 vk::PhysicalDeviceVulkan11Features,
                                 vk::PhysicalDeviceVulkan13Features>();

  vk::PhysicalDeviceVulkan11Features const &vulkan_11_features =
    features.get<vk::PhysicalDeviceVulkan11Features>();

  vk::PhysicalDeviceVulkan13Features const &vulkan_13_features =
    features.get<vk::PhysicalDeviceVulkan13Features>();

  return vulkan_13_features.dynamicRendering == vk::True &&
         vulkan_13_features.synchronization2 == vk::True &&
         vulkan_11_features.shaderDrawParameters == vk::True;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanDevice::SupportsSwapchain(
  vk::raii::PhysicalDevice const &physical_device) const -> bool {
  return !physical_device.getSurfaceFormatsKHR(*surface_).empty() &&
         !physical_device.getSurfacePresentModesKHR(*surface_).empty();
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanDevice::BuildPhysicalDeviceCandidate(
  vk::raii::PhysicalDevice const &physical_device,
  std::uint32_t enumeration_index) const -> GGEMSVulkanDeviceCandidate {
  vk::PhysicalDeviceProperties properties = physical_device.getProperties();
  QueueFamilyIndices queue_family_indices = FindQueueFamilies(physical_device);

  GGEMSVulkanDeviceCandidate candidate{
    .enumeration_index = enumeration_index,
    .physical_device = *physical_device,
    .name = properties.deviceName.data(),
    .type = properties.deviceType,
    .vendor_id = properties.vendorID,
    .device_id = properties.deviceID,
    .api_version = properties.apiVersion,
    .driver_version = properties.driverVersion,
    .graphics_queue_family = queue_family_indices.graphics,
    .presentation_queue_family = queue_family_indices.presentation,
    .rejection_reason = {},
    .platform_adapter_id = {},
  };

#ifdef _WIN32
  if (candidate.api_version >= vk::ApiVersion11) {
    candidate.platform_adapter_id = QueryWin32VulkanAdapterId(physical_device);
  }
#endif

  bool supports_vulkan_1_3 = candidate.api_version >= vk::ApiVersion13;

  candidate.required_extensions_available =
    SupportsRequiredDeviceExtensions(physical_device);

  candidate.required_features_available =
    supports_vulkan_1_3 && SupportsRequiredFeatures(physical_device);

  candidate.swapchain_adequate =
    candidate.required_extensions_available &&
    candidate.presentation_queue_family.has_value() &&
    SupportsSwapchain(physical_device);

  candidate.suitable =
    supports_vulkan_1_3 && candidate.graphics_queue_family.has_value() &&
    candidate.presentation_queue_family.has_value() &&
    candidate.required_extensions_available &&
    candidate.required_features_available && candidate.swapchain_adequate;

  if (!supports_vulkan_1_3) {
    AppendRejectionReason(candidate.rejection_reason,
                          "Vulkan 1.3 is unavailable");
  }

  if (!candidate.graphics_queue_family.has_value()) {
    AppendRejectionReason(candidate.rejection_reason,
                          "no graphics queue family");
  }

  if (!candidate.presentation_queue_family.has_value()) {
    AppendRejectionReason(candidate.rejection_reason,
                          "no queue family supports the GLFW surface");
  }

  if (!candidate.required_extensions_available) {
    AppendRejectionReason(candidate.rejection_reason,
                          "VK_KHR_swapchain is unavailable");
  }

  if (supports_vulkan_1_3 && !candidate.required_features_available) {
    AppendRejectionReason(candidate.rejection_reason,
                          "required Vulkan 1.1/1.3 features are unavailable");
  }

  if (candidate.required_extensions_available &&
      candidate.presentation_queue_family.has_value() &&
      !candidate.swapchain_adequate) {
    AppendRejectionReason(
      candidate.rejection_reason,
      "surface formats or presentation modes are unavailable");
  }

  return candidate;
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanDevice::SelectPhysicalDevice(
  GGEMSVulkanDeviceSelector const &device_selector,
  std::optional<GGEMSVulkanDisplayAdapter> const &display_adapter) -> void {
  std::vector<vk::raii::PhysicalDevice> physical_devices =
    instance_.enumeratePhysicalDevices();

  if (physical_devices.empty()) {
    throw ggems::core::GGEMSRecoverable(
      "No Vulkan physical device is available for GGEMS GuiMode.");
  }

  std::vector<GGEMSVulkanDeviceCandidate> candidates{};
  candidates.reserve(physical_devices.size());

  for (std::uint32_t index = 0U;
       index < static_cast<std::uint32_t>(physical_devices.size()); ++index) {
    candidates.push_back(
      BuildPhysicalDeviceCandidate(physical_devices[index], index));
  }

  GGEMS_INFO("Vulkan", "Vulkan physical devices:");

  for (GGEMSVulkanDeviceCandidate const &candidate : candidates) {
    GGEMS_INFO("Vulkan", "[{}] {}", candidate.enumeration_index,
               candidate.name);
    GGEMS_INFO("Vulkan",
               "    type={} vendor=0x{:04x} device=0x{:04x} "
               "API={}.{}.{} driver=0x{:08x}",
               vk::to_string(candidate.type), candidate.vendor_id,
               candidate.device_id, VK_API_VERSION_MAJOR(candidate.api_version),
               VK_API_VERSION_MINOR(candidate.api_version),
               VK_API_VERSION_PATCH(candidate.api_version),
               candidate.driver_version);
    GGEMS_INFO(
      "Vulkan",
      "    graphics={} present={} extensions={} features={} swapchain={}",
      YesNo(candidate.graphics_queue_family.has_value()),
      YesNo(candidate.presentation_queue_family.has_value()),
      YesNo(candidate.required_extensions_available),
      YesNo(candidate.required_features_available),
      YesNo(candidate.swapchain_adequate));
    GGEMS_INFO("Vulkan", "    display_match={} suitable={}",
               DisplayMatchLabel(candidate, display_adapter),
               YesNo(candidate.suitable));

    if (!candidate.suitable) {
      GGEMS_INFO("Vulkan", "    rejection={}", candidate.rejection_reason);
    }
  }

  auto selection =
    SelectVulkanDevice(device_selector, candidates, display_adapter);

  if (!selection.has_value()) {
    throw ggems::core::GGEMSRecoverable(selection.error());
  }

  auto selected_candidate = std::ranges::find_if(
    candidates,
    [&selection](GGEMSVulkanDeviceCandidate const &candidate) -> bool {
      return candidate.enumeration_index == selection->enumeration_index;
    });

  if (!(selected_candidate != candidates.end())) {
    throw ggems::core::GGEMSInternal(std::format(
      "Selected Vulkan enumeration index {} is absent from the collected "
      "candidate set.",
      selection->enumeration_index));
  }

  vk::PhysicalDevice selected_handle = selected_candidate->physical_device;

  auto selected_physical_device = std::ranges::find_if(
    physical_devices,
    [selected_handle](vk::raii::PhysicalDevice const &physical_device) -> bool {
      return *physical_device == selected_handle;
    });

  if (!(selected_physical_device != physical_devices.end())) {
    throw ggems::core::GGEMSInternal(std::format(
      "Selected Vulkan candidate [{}] '{}' no longer refers to an "
      "enumerated physical device.",
      selected_candidate->enumeration_index, selected_candidate->name));
  }

  physical_device_ = *selected_physical_device;
  selected_physical_device_candidate_ = *selected_candidate;

  status_.enumeration_index = selected_candidate->enumeration_index;
  status_.name = selected_candidate->name;
  status_.type = vk::to_string(selected_candidate->type);
  status_.selection_reason = selection->reason;

  queue_family_indices_ = QueueFamilyIndices{
    .graphics = selected_candidate->graphics_queue_family,
    .presentation = selected_candidate->presentation_queue_family,
  };

  GGEMS_INFO("Vulkan", "Vulkan selected device: [{}] {}",
             selected_candidate->enumeration_index, selected_candidate->name);
  GGEMS_INFO("Vulkan", "Selection reason: {}.", selection->reason);

  GGEMS_INFOEX("Vulkan", 2,
               "Selected Vulkan queue families: graphics={}, presentation={}, "
               "separate={}.",
               queue_family_indices_.graphics.value(),
               queue_family_indices_.presentation.value(),
               queue_family_indices_.UsesSeparateFamilies());
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanDevice::WarnIfCrossAdapterPresentation(
  GLFWwindow *window,
  std::optional<GGEMSVulkanDisplayAdapter> const &display_adapter) const
  -> void {
  if (!display_adapter.has_value() ||
      !IsVulkanDisplayAdapterMismatch(selected_physical_device_candidate_,
                                      display_adapter)) {
    return;
  }

  int framebuffer_width{0};
  int framebuffer_height{0};
  glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);

  GGEMS_WARN(
    "Vulkan",
    "Vulkan rendering device and display adapter differ.\n"
    "Cross-adapter presentation may reduce performance or cause "
    "instability.\n"
    "Framebuffer: {}x{}.\n"
    "Selected Vulkan device: {}.\n"
    "Display adapter: {}.\n"
    "High framebuffer resolutions can amplify cross-adapter presentation "
    "costs.",
    framebuffer_width, framebuffer_height,
    selected_physical_device_candidate_.name, display_adapter->name);
}

// -----------------------------------------------------------------------------

auto GGEMSVulkanDevice::CreateLogicalDevice() -> void {
  float queue_priority{1.0F};

  std::vector<vk::DeviceQueueCreateInfo> queue_create_infos{};
  queue_create_infos.reserve(queue_family_indices_.UsesSeparateFamilies() ? 2U
                                                                          : 1U);
  queue_create_infos.push_back(vk::DeviceQueueCreateInfo{
    .queueFamilyIndex = queue_family_indices_.graphics.value(),
    .queueCount = 1U,
    .pQueuePriorities = &queue_priority,
  });

  if (queue_family_indices_.UsesSeparateFamilies()) {
    queue_create_infos.push_back(vk::DeviceQueueCreateInfo{
      .queueFamilyIndex = queue_family_indices_.presentation.value(),
      .queueCount = 1U,
      .pQueuePriorities = &queue_priority,
    });
  }

  vk::PhysicalDeviceFeatures device_features{};

  vk::PhysicalDeviceVulkan13Features vulkan_13_features{
    .synchronization2 = vk::True,
    .dynamicRendering = vk::True,
  };

  vk::PhysicalDeviceVulkan11Features vulkan_11_features{
    .pNext = &vulkan_13_features,
    .shaderDrawParameters = vk::True,
  };

  vk::DeviceCreateInfo create_info{
    .pNext = &vulkan_11_features,
    .queueCreateInfoCount =
      static_cast<std::uint32_t>(queue_create_infos.size()),
    .pQueueCreateInfos = queue_create_infos.data(),
    .enabledExtensionCount =
      static_cast<std::uint32_t>(k_required_device_extensions.size()),
    .ppEnabledExtensionNames = k_required_device_extensions.data(),
    .pEnabledFeatures = &device_features,
  };

  device_ = vk::raii::Device{physical_device_, create_info};

  graphics_queue_ =
    vk::raii::Queue{device_, queue_family_indices_.graphics.value(), 0U};

  presentation_queue_ =
    vk::raii::Queue{device_, queue_family_indices_.presentation.value(), 0U};

  GGEMS_INFOEX("Vulkan", 2,
               "Vulkan logical device created: graphics queue family={}, "
               "presentation queue family={}, separate={}.",
               queue_family_indices_.graphics.value(),
               queue_family_indices_.presentation.value(),
               queue_family_indices_.UsesSeparateFamilies());
}

} // namespace ggems::ui::detail
