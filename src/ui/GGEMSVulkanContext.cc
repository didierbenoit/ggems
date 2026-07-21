#include "GGEMSVulkanContext.hh"

#include <array>
#include <cstdio>
#include <cmath>
#include <cstring>
#include <format>
#include <ranges>
#include <string>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <limits>
#include <cstdlib>
#include <filesystem>
#include <optional>
#include <utility>
#include <string_view>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMSImGuiTheme.hh"
#include "GGEMS/render/GGEMSColourNames.hh"

#if defined(_WIN32)
#include "GGEMSVulkanDisplayAdapterWin32.hh"
#endif

namespace {

// =============================================================================
// =============================================================================

#ifdef GGEMS_DEBUG_MODE
constexpr bool k_enable_validation_layers{true};
#else
constexpr bool k_enable_validation_layers{false};
#endif

constexpr std::array<char const *, 1> k_validation_layers{
    "VK_LAYER_KHRONOS_validation"};

constexpr std::array<char const *, 1> k_required_device_extensions{
    vk::KHRSwapchainExtensionName};

constexpr float k_imgui_min_ui_scale{1.0f};
constexpr float k_imgui_max_ui_scale{2.5f};
constexpr float k_imgui_base_font_size{15.0f};

// =============================================================================
// =============================================================================

[[nodiscard]] float NormaliseImGuiUIScale(float content_scale_x,
                                          float content_scale_y) noexcept {
  if (!std::isfinite(content_scale_x) || !std::isfinite(content_scale_y) ||
      content_scale_x <= 0.0f || content_scale_y <= 0.0f) {
    return k_imgui_min_ui_scale;
  }

  return std::clamp(std::max(content_scale_x, content_scale_y),
                    k_imgui_min_ui_scale, k_imgui_max_ui_scale);
}

// =============================================================================
// =============================================================================

void AppendRejectionReason(std::string &diagnostic, std::string_view reason) {
  if (!diagnostic.empty()) {
    diagnostic += "; ";
  }
  diagnostic += reason;
}

// =============================================================================
// =============================================================================

[[nodiscard]] std::string_view YesNo(bool value) noexcept {
  return value ? "yes" : "no";
}

// =============================================================================
// =============================================================================

[[nodiscard]] std::string_view DisplayMatchLabel(
    ggems::ui::detail::GGEMSVulkanDeviceCandidate const &candidate,
    std::optional<ggems::ui::detail::GGEMSVulkanDisplayAdapter> const
        &display_adapter) noexcept {
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

// =============================================================================
// =============================================================================

[[nodiscard]] std::optional<std::filesystem::path> FindFirstExistingFont() {
  std::vector<std::filesystem::path> candidates{};

  if (char const *local_app_data = std::getenv("LOCALAPPDATA");
      local_app_data != nullptr) {
    std::filesystem::path const user_fonts =
        std::filesystem::path{local_app_data} / "Microsoft/Windows/Fonts";

    candidates.emplace_back(user_fonts / "JetBrainsMono-Regular.ttf");
    candidates.emplace_back(user_fonts / "JetBrainsMonoNerdFont-Regular.ttf");
    candidates.emplace_back(user_fonts /
                            "JetBrainsMonoNerdFontMono-Regular.ttf");
    candidates.emplace_back(user_fonts / "JetBrainsMonoNLNerdFont-Regular.ttf");
    candidates.emplace_back(user_fonts /
                            "JetBrainsMonoNLNerdFontMono-Regular.ttf");
  }

  candidates.emplace_back("C:/Windows/Fonts/JetBrainsMono-Regular.ttf");
  candidates.emplace_back("C:/Windows/Fonts/JetBrainsMonoNerdFont-Regular.ttf");
  candidates.emplace_back(
      "C:/Windows/Fonts/JetBrainsMonoNerdFontMono-Regular.ttf");
  candidates.emplace_back("C:/Windows/Fonts/CascadiaMono.ttf");
  candidates.emplace_back("C:/Windows/Fonts/CascadiaCode.ttf");
  candidates.emplace_back("C:/Windows/Fonts/consola.ttf");

  if (char const *home = std::getenv("HOME"); home != nullptr) {
    std::filesystem::path const user_fonts =
        std::filesystem::path{home} / ".local/share/fonts";

    candidates.emplace_back(user_fonts / "JetBrainsMono-Regular.ttf");
    candidates.emplace_back(user_fonts / "JetBrainsMonoNerdFont-Regular.ttf");
    candidates.emplace_back(user_fonts /
                            "JetBrainsMonoNerdFontMono-Regular.ttf");
    candidates.emplace_back(user_fonts / "DejaVuSansMono.ttf");
  }

  candidates.emplace_back(
      "/usr/share/fonts/truetype/jetbrains-mono/JetBrainsMono-Regular.ttf");
  candidates.emplace_back(
      "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf");
  candidates.emplace_back("/usr/local/share/fonts/JetBrainsMono-Regular.ttf");

  for (std::filesystem::path const &candidate : candidates) {
    if (std::filesystem::exists(candidate)) {
      return candidate;
    }
  }

  return std::nullopt;
}

// =============================================================================
// =============================================================================

std::array<float, 4U>
ToVulkanClearColour(ggems::render::ColourKey const &colour) {
  ggems::render::RGB const rgb =
      ggems::render::GetColourRGB(colour.family, colour.shade, colour.variant);

  constexpr float k_inverse_255{1.0F / 255.0F};

  return {static_cast<float>(rgb.r) * k_inverse_255,
          static_cast<float>(rgb.g) * k_inverse_255,
          static_cast<float>(rgb.b) * k_inverse_255, 1.0F};
}

} // namespace

namespace ggems::ui {

// =============================================================================
// =============================================================================

GGEMSVulkanContext::~GGEMSVulkanContext() noexcept {
  if (*device_ == nullptr) {
    return;
  }

  try {
    device_.waitIdle();
  } catch (...) {
    std::fputs(
        "[GGEMS Vulkan] Failed to wait for device idle during shutdown.\n",
        stderr);
  }

  ShutdownSceneRenderer();
  ShutdownImGui();
}

// -----------------------------------------------------------------------------

void GGEMSVulkanContext::Initialise(
    GLFWwindow *window,
    detail::GGEMSVulkanDeviceSelector const &device_selector,
    detail::GGEMSComputeStatus compute_status) {
  if (initialised_) {
    return;
  }

  GGEMS_CHECK_INTERNAL(
      window != nullptr,
      "A valid GLFW window is required before initialising Vulkan GuiMode.");

  device_status_.compute = std::move(compute_status);

  try {
    std::optional<detail::GGEMSVulkanDisplayAdapter> display_adapter{};

    CreateInstance();
    SetupDebugMessenger();
    CreateSurface(window);

#if defined(_WIN32)
    auto display_resolution = detail::ResolveWin32DisplayAdapter(window);

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
    CreateSwapchain(window);
    WarnIfCrossAdapterPresentation(window, display_adapter);
    CreateSwapchainImageViews();
    CreateCommandPool();
    AllocateCommandBuffers();
    CreateFrameSyncObjects();
    CreateSwapchainSyncObjects();
    CreateImGuiDescriptorPool();
    InitialiseImGui(window);
    InitialiseSceneRenderer();
  } catch (vk::SystemError const &error) {
    GGEMS_RECOVERABLE(
        std::format("Unable to initialise Vulkan GuiMode: {}.", error.what()));
  }

  initialised_ = true;
  device_status_.renderer.initialised = true;

  GGEMS_INFO("Vulkan",
             "Vulkan swapchain command buffers, synchronisation objects and "
             "Dear ImGui backend initialised.");
}

// -----------------------------------------------------------------------------

bool GGEMSVulkanContext::IsInitialised() const noexcept { return initialised_; }

// -----------------------------------------------------------------------------

void GGEMSVulkanContext::SubmitParticleTraceSegments(
    std::vector<ggems::render::GGEMSParticleTraceSegment> segments) {
  std::scoped_lock lock{pending_particle_trace_mutex_};

  pending_particle_trace_segments_ = std::move(segments);
  has_pending_particle_trace_segments_ = true;
  pending_particle_trace_clear_ = false;
}

// -----------------------------------------------------------------------------

void GGEMSVulkanContext::ClearParticleTraces() {
  std::scoped_lock lock{pending_particle_trace_mutex_};

  pending_particle_trace_segments_.clear();
  has_pending_particle_trace_segments_ = false;
  pending_particle_trace_clear_ = true;
}

// -----------------------------------------------------------------------------

void GGEMSVulkanContext::CreateInstance() {
  constexpr vk::ApplicationInfo application_info{
      .pApplicationName = "GGEMS GuiMode",
      .applicationVersion = VK_MAKE_API_VERSION(0, 2, 0, 0),
      .pEngineName = "GGEMS",
      .engineVersion = VK_MAKE_API_VERSION(0, 2, 0, 0),
      .apiVersion = k_vulkan_api_version_};

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

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

#if VK_HEADER_VERSION >= 304
VKAPI_ATTR VkBool32 VKAPI_CALL GGEMSVulkanContext::DebugVkCallback(
    vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
    vk::DebugUtilsMessageTypeFlagsEXT type,
    vk::DebugUtilsMessengerCallbackDataEXT const *callback_data,
    void *) noexcept {
#else
VKAPI_ATTR VkBool32 VKAPI_CALL GGEMSVulkanContext::DebugVkCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    VkDebugUtilsMessengerCallbackDataEXT const *callback_data,
    void *) noexcept {
#endif
  if (callback_data == nullptr || callback_data->pMessage == nullptr) {
    return VK_FALSE;
  }

  std::uint32_t message_severity = static_cast<std::uint32_t>(severity);
  std::uint32_t message_type = static_cast<std::uint32_t>(type);

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

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

bool GGEMSVulkanContext::SupportsRequiredFeatures(
    vk::raii::PhysicalDevice const &physical_device) const {
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
         vulkan_11_features.shaderDrawParameters;
}

// -----------------------------------------------------------------------------

bool GGEMSVulkanContext::SupportsSwapchain(
    vk::raii::PhysicalDevice const &physical_device) const {
  SwapchainSupportDetails details = QuerySwapchainSupport(physical_device);

  return !details.surface_formats.empty() && !details.present_modes.empty();
}

// -----------------------------------------------------------------------------

GGEMSVulkanContext::SwapchainSupportDetails
GGEMSVulkanContext::QuerySwapchainSupport(
    vk::raii::PhysicalDevice const &physical_device) const {
  SwapchainSupportDetails details{};

  details.capabilities = physical_device.getSurfaceCapabilitiesKHR(*surface_);
  details.surface_formats = physical_device.getSurfaceFormatsKHR(*surface_);
  details.present_modes = physical_device.getSurfacePresentModesKHR(*surface_);

  return details;
}

// -----------------------------------------------------------------------------

detail::GGEMSVulkanDeviceCandidate
GGEMSVulkanContext::BuildPhysicalDeviceCandidate(
    vk::raii::PhysicalDevice const &physical_device,
    std::uint32_t enumeration_index) const {
  vk::PhysicalDeviceProperties properties = physical_device.getProperties();
  QueueFamilyIndices queue_family_indices = FindQueueFamilies(physical_device);

  detail::GGEMSVulkanDeviceCandidate candidate{
      .enumeration_index = enumeration_index,
      .physical_device = *physical_device,
      .name = properties.deviceName.data(),
      .type = properties.deviceType,
      .vendor_id = properties.vendorID,
      .device_id = properties.deviceID,
      .api_version = properties.apiVersion,
      .driver_version = properties.driverVersion,
      .graphics_queue_family = queue_family_indices.graphics,
      .presentation_queue_family = queue_family_indices.presentation};

#if defined(_WIN32)
  if (candidate.api_version >= vk::ApiVersion11) {
    candidate.platform_adapter_id =
        detail::QueryWin32VulkanAdapterId(physical_device);
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

void GGEMSVulkanContext::SelectPhysicalDevice(
    detail::GGEMSVulkanDeviceSelector const &device_selector,
    std::optional<detail::GGEMSVulkanDisplayAdapter> const &display_adapter) {
  std::vector<vk::raii::PhysicalDevice> physical_devices =
      instance_.enumeratePhysicalDevices();

  GGEMS_CHECK_RECOVERABLE(
      !physical_devices.empty(),
      "No Vulkan physical device is available for GGEMS GuiMode.");

  std::vector<detail::GGEMSVulkanDeviceCandidate> candidates{};
  candidates.reserve(physical_devices.size());

  for (std::uint32_t index = 0U;
       index < static_cast<std::uint32_t>(physical_devices.size()); ++index) {
    candidates.push_back(
        BuildPhysicalDeviceCandidate(physical_devices[index], index));
  }

  GGEMS_INFO("Vulkan", "Vulkan physical devices:");

  for (detail::GGEMSVulkanDeviceCandidate const &candidate : candidates) {
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
      detail::SelectVulkanDevice(device_selector, candidates, display_adapter);

  if (!selection.has_value()) {
    GGEMS_RECOVERABLE(selection.error());
  }

  auto selected_candidate = std::ranges::find_if(
      candidates,
      [&selection](detail::GGEMSVulkanDeviceCandidate const &candidate) {
        return candidate.enumeration_index == selection->enumeration_index;
      });

  GGEMS_CHECK_INTERNAL(
      selected_candidate != candidates.end(),
      std::format(
          "Selected Vulkan enumeration index {} is absent from the collected "
          "candidate set.",
          selection->enumeration_index));

  vk::PhysicalDevice selected_handle = selected_candidate->physical_device;

  auto selected_physical_device = std::ranges::find_if(
      physical_devices,
      [selected_handle](vk::raii::PhysicalDevice const &physical_device) {
        return *physical_device == selected_handle;
      });

  GGEMS_CHECK_INTERNAL(
      selected_physical_device != physical_devices.end(),
      std::format("Selected Vulkan candidate [{}] '{}' no longer refers to an "
                  "enumerated physical device.",
                  selected_candidate->enumeration_index,
                  selected_candidate->name));

  physical_device_ = *selected_physical_device;
  selected_physical_device_candidate_ = *selected_candidate;

  device_status_.renderer.enumeration_index =
      selected_candidate->enumeration_index;
  device_status_.renderer.name = selected_candidate->name;
  device_status_.renderer.type = vk::to_string(selected_candidate->type);
  device_status_.renderer.selection_reason = selection->reason;

  queue_family_indices_ = QueueFamilyIndices{
      .graphics = selected_candidate->graphics_queue_family,
      .presentation = selected_candidate->presentation_queue_family};

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

void GGEMSVulkanContext::WarnIfCrossAdapterPresentation(
    GLFWwindow *window,
    std::optional<detail::GGEMSVulkanDisplayAdapter> const &display_adapter)
    const {
  if (!display_adapter.has_value() ||
      !detail::IsVulkanDisplayAdapterMismatch(
          selected_physical_device_candidate_, display_adapter)) {
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

  vk::PhysicalDeviceVulkan11Features vulkan_11_features{
      .pNext = &vulkan_13_features, .shaderDrawParameters = vk::True};

  vk::DeviceCreateInfo create_info{
      .pNext = &vulkan_11_features,
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

  GGEMS_INFOEX("Vulkan", 2,
               "Vulkan logical device created: graphics queue family={}, "
               "presentation queue family={}, separate={}.",
               queue_family_indices_.graphics.value(),
               queue_family_indices_.presentation.value(),
               queue_family_indices_.UsesSeparateFamilies());
}

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

void GGEMSVulkanContext::CreateSwapchain(GLFWwindow *window) {
  GGEMS_CHECK_INTERNAL(
      *physical_device_ != nullptr && *device_ != nullptr,
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

  GGEMS_INFOEX("Vulkan", 1,
               "Vulkan swapchain created: images={}, format={}, extent={}x{}, "
               "present mode={}.",
               swapchain_images_.size(), vk::to_string(swapchain_image_format_),
               swapchain_extent_.width, swapchain_extent_.height,
               vk::to_string(present_mode));
}

// -----------------------------------------------------------------------------

void GGEMSVulkanContext::CreateSwapchainImageViews() {
  GGEMS_CHECK_INTERNAL(
      *swapchain_ != nullptr,
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

  GGEMS_INFOEX("Vulkan", 2, "Created {} Vulkan swapchain image views.",
               swapchain_image_views_.size());
}

// -----------------------------------------------------------------------------

void GGEMSVulkanContext::CreateCommandPool() {
  GGEMS_CHECK_INTERNAL(
      queue_family_indices_.graphics.has_value(),
      "A Vulkan graphics queue family is required before creating the"
      "GuiMode command pool.");

  vk::CommandPoolCreateInfo create_info{
      .flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
      .queueFamilyIndex = queue_family_indices_.graphics.value()};

  command_pool_ = vk::raii::CommandPool{device_, create_info};

  GGEMS_INFOEX("Vulkan", 2,
               "Vulkan command pool created for graphics family {}.",
               queue_family_indices_.graphics.value());
}

// -----------------------------------------------------------------------------

void GGEMSVulkanContext::AllocateCommandBuffers() {
  GGEMS_CHECK_INTERNAL(
      *command_pool_ != nullptr,
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

  GGEMS_INFOEX("Vulkan", 2, "Allocated {} Vulkan command buffers.",
               command_buffers_.size());
}

// -----------------------------------------------------------------------------

void GGEMSVulkanContext::CreateFrameSyncObjects() {
  vk::SemaphoreCreateInfo const semaphore_create_info{};

  vk::FenceCreateInfo const fence_create_info{
      .flags = vk::FenceCreateFlagBits::eSignaled};

  image_available_semaphores_.clear();
  in_flight_fences_.clear();

  image_available_semaphores_.reserve(k_max_frames_in_flight_);
  in_flight_fences_.reserve(k_max_frames_in_flight_);

  for (std::uint32_t i = 0U; i < k_max_frames_in_flight_; ++i) {
    image_available_semaphores_.emplace_back(device_, semaphore_create_info);
    in_flight_fences_.emplace_back(device_, fence_create_info);
  }

  GGEMS_INFOEX(
      "Vulkan", 2,
      "Created Vulkan frame synchronisation objects for {} frames in flight.",
      k_max_frames_in_flight_);
}

// -----------------------------------------------------------------------------

void GGEMSVulkanContext::CreateSwapchainSyncObjects() {
  GGEMS_CHECK_INTERNAL(
      !swapchain_images_.empty(),
      "Swapchain images are required before creating Vulkan swapchain "
      "synchronisation objects.");

  vk::SemaphoreCreateInfo const semaphore_create_info{};

  render_finished_semaphores_.clear();
  render_finished_semaphores_.reserve(swapchain_images_.size());

  for (std::size_t i = 0U; i < swapchain_images_.size(); ++i) {
    render_finished_semaphores_.emplace_back(device_, semaphore_create_info);
  }

  swapchain_image_in_flight_fences_.assign(swapchain_images_.size(),
                                           vk::Fence{nullptr});

  GGEMS_INFOEX(
      "Vulkan", 2,
      "Created {} Vulkan render-finished semaphores for swapchain images.",
      render_finished_semaphores_.size());
}

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

void GGEMSVulkanContext::RecordCommandBuffer(std::uint32_t image_index) {
  vk::raii::CommandBuffer &command_buffer = command_buffers_[image_index];

  command_buffer.reset();

  vk::CommandBufferBeginInfo begin_info{};
  command_buffer.begin(begin_info);

  scene_renderer_.RecordSceneCommands(command_buffers_[image_index]);

  TransitionSwapchainImageLayout(image_index, vk::ImageLayout::eUndefined,
                                 vk::ImageLayout::eColorAttachmentOptimal);

  vk::ClearValue clear_value =
      vk::ClearColorValue(ToVulkanClearColour(render::BLUE_Abyss));

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

  ImGui_ImplVulkan_RenderDrawData(
      ImGui::GetDrawData(), static_cast<VkCommandBuffer>(*command_buffer));

  command_buffer.endRendering();

  TransitionSwapchainImageLayout(image_index,
                                 vk::ImageLayout::eColorAttachmentOptimal,
                                 vk::ImageLayout::ePresentSrcKHR);

  command_buffer.end();
}

// -----------------------------------------------------------------------------

void GGEMSVulkanContext::RenderFrame(GLFWwindow *window,
                                     bool framebuffer_resized) {
  GGEMS_CHECK_INTERNAL(
      initialised_,
      "Vulkan GuiMode must be initialised before rendering a frame.");

  GGEMS_CHECK_INTERNAL(
      window != nullptr,
      "A valid GLFW window is required before rendering a Vulkan frame.");

  try {
    if (framebuffer_resized) {
      RecreateSwapchain(window);
    }

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

    GGEMS_CHECK_INTERNAL(image_index < swapchain_image_in_flight_fences_.size(),
                         "The acquired Vulkan swapchain image index exceeds "
                         "the number of tracked "
                         "in-flight image fences.");

    vk::Fence const image_in_flight_fence =
        swapchain_image_in_flight_fences_[image_index];

    if (image_in_flight_fence != vk::Fence{nullptr}) {
      vk::Result const wait_image_result =
          device_.waitForFences(image_in_flight_fence, vk::True,
                                std::numeric_limits<std::uint64_t>::max());

      GGEMS_CHECK_RECOVERABLE(
          wait_image_result == vk::Result::eSuccess,
          std::format(
              "Unable to wait for the Vulkan swapchain image fence: {}.",
              vk::to_string(wait_image_result)));
    }

    swapchain_image_in_flight_fences_[image_index] =
        *in_flight_fences_[current_frame_];

    BuildImGuiFrame();

    RecordCommandBuffer(image_index);

    device_.resetFences(*in_flight_fences_[current_frame_]);

    vk::Semaphore wait_semaphores[]{
        *image_available_semaphores_[current_frame_]};

    vk::PipelineStageFlags wait_stages[]{
        vk::PipelineStageFlagBits::eColorAttachmentOutput};

    vk::CommandBuffer command_buffers[]{*command_buffers_[image_index]};

    vk::Semaphore signal_semaphores[]{
        *render_finished_semaphores_[image_index]};

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

    if (present_result == vk::Result::eErrorOutOfDateKHR ||
        present_result == vk::Result::eSuboptimalKHR) {
      RecreateSwapchain(window);
    } else {
      GGEMS_CHECK_RECOVERABLE(
          present_result == vk::Result::eSuccess,
          std::format("Unable to present a Vulkan swapchain image: {}.",
                      vk::to_string(present_result)));
    }

    current_frame_ = (current_frame_ + 1U) % k_max_frames_in_flight_;
  } catch (vk::OutOfDateKHRError const &) {
    RecreateSwapchain(window);
  } catch (vk::SystemError const &error) {
    GGEMS_RECOVERABLE(std::format(
        "Unable to render a Vulkan GuiMode frame: {}.", error.what()));
  }
}

// -----------------------------------------------------------------------------

void GGEMSVulkanContext::CleanupSwapchain() {
  command_buffers_.clear();

  swapchain_image_views_.clear();
  swapchain_images_.clear();

  render_finished_semaphores_.clear();
  swapchain_image_in_flight_fences_.clear();

  swapchain_ = nullptr;
  swapchain_image_format_ = vk::Format::eUndefined;
  swapchain_extent_ = vk::Extent2D{};
}

// -----------------------------------------------------------------------------

void GGEMSVulkanContext::RecreateSwapchain(GLFWwindow *window) {
  GGEMS_CHECK_INTERNAL(window != nullptr,
                       "A valid GLFW window is required before recreating the "
                       "Vulkan swapchain.");

  int width{0};
  int height{0};

  glfwGetFramebufferSize(window, &width, &height);

  while (width == 0 || height == 0) {
    glfwWaitEvents();
    glfwGetFramebufferSize(window, &width, &height);
  }

  device_.waitIdle();

  CleanupSwapchain();

  CreateSwapchain(window);
  CreateSwapchainImageViews();
  AllocateCommandBuffers();
  CreateSwapchainSyncObjects();

  if (imgui_initialised_) {
    ImGui_ImplVulkan_SetMinImageCount(
        static_cast<std::uint32_t>(swapchain_images_.size()));
  }

  current_frame_ = 0U;

  GGEMS_INFOEX("Vulkan", 1,
               "Vulkan swapchain recreated after framebuffer resize.");
}

// -----------------------------------------------------------------------------

void GGEMSVulkanContext::CreateImGuiDescriptorPool() {
  std::array<vk::DescriptorPoolSize, 11> pool_sizes{
      vk::DescriptorPoolSize{vk::DescriptorType::eSampler, 1000U},
      vk::DescriptorPoolSize{vk::DescriptorType::eCombinedImageSampler, 1000U},
      vk::DescriptorPoolSize{vk::DescriptorType::eSampledImage, 1000U},
      vk::DescriptorPoolSize{vk::DescriptorType::eStorageImage, 1000U},
      vk::DescriptorPoolSize{vk::DescriptorType::eUniformTexelBuffer, 1000U},
      vk::DescriptorPoolSize{vk::DescriptorType::eStorageTexelBuffer, 1000U},
      vk::DescriptorPoolSize{vk::DescriptorType::eUniformBuffer, 1000U},
      vk::DescriptorPoolSize{vk::DescriptorType::eStorageBuffer, 1000U},
      vk::DescriptorPoolSize{vk::DescriptorType::eUniformBufferDynamic, 1000U},
      vk::DescriptorPoolSize{vk::DescriptorType::eStorageBufferDynamic, 1000U},
      vk::DescriptorPoolSize{vk::DescriptorType::eInputAttachment, 1000U}};

  vk::DescriptorPoolCreateInfo create_info{
      .flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
      .maxSets = 1000U * static_cast<std::uint32_t>(pool_sizes.size()),
      .poolSizeCount = static_cast<std::uint32_t>(pool_sizes.size()),
      .pPoolSizes = pool_sizes.data()};

  imgui_descriptor_pool_ = vk::raii::DescriptorPool{device_, create_info};

  GGEMS_INFOEX("Vulkan", 2, "Dear ImGui Vulkan descriptor pool created.");
}

// -----------------------------------------------------------------------------

void GGEMSVulkanContext::CheckImGuiVkResult(VkResult result) noexcept {
  if (result == VK_SUCCESS) {
    return;
  }

  try {
    GGEMS_ERROR("Vulkan", "Dear ImGui Vulkan backend error: {}.",
                vk::to_string(static_cast<vk::Result>(result)));
  } catch (...) {
    std::fputs("[GGEMS Vulkan] Dear ImGui backend reported an error.\n",
               stderr);
  }
}

// -----------------------------------------------------------------------------

void GGEMSVulkanContext::InitialiseImGui(GLFWwindow *window) {
  GGEMS_CHECK_INTERNAL(
      window != nullptr,
      "A valid GLFW window is required before initialising Dear ImGui.");

  GGEMS_CHECK_INTERNAL(
      *imgui_descriptor_pool_ != nullptr,
      "A Vulkan descriptor pool is required before initialising Dear ImGui.");

  float content_scale_x{1.0f};
  float content_scale_y{1.0f};

  glfwGetWindowContentScale(window, &content_scale_x, &content_scale_y);

  imgui_ui_scale_ = NormaliseImGuiUIScale(content_scale_x, content_scale_y);
  imgui_font_size_ = k_imgui_base_font_size * imgui_ui_scale_;

  IMGUI_CHECKVERSION();

  ImGui::CreateContext();

  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  ImGui::StyleColorsDark();
  ApplyGGEMSImGuiTheme();
  ImGui::GetStyle().ScaleAllSizes(imgui_ui_scale_);

  LoadImGuiFonts();

  GGEMS_INFO("Gui", "ImGui content scale: x={:.2f}, y={:.2f}.", content_scale_x,
             content_scale_y);
  GGEMS_INFO("Gui", "ImGui UI scale: {:.2f}.", imgui_ui_scale_);
  GGEMS_INFO("Gui", "ImGui font size: {:.2f} px.", imgui_font_size_);

  ImGui_ImplGlfw_InitForVulkan(window, true);

  imgui_colour_attachment_format_ =
      static_cast<VkFormat>(swapchain_image_format_);

  imgui_pipeline_rendering_create_info_ = VkPipelineRenderingCreateInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
      .pNext = nullptr,
      .viewMask = 0U,
      .colorAttachmentCount = 1U,
      .pColorAttachmentFormats = &imgui_colour_attachment_format_,
      .depthAttachmentFormat = VK_FORMAT_UNDEFINED,
      .stencilAttachmentFormat = VK_FORMAT_UNDEFINED};

  ImGui_ImplVulkan_InitInfo init_info{};
  init_info.ApiVersion = k_vulkan_api_version_;
  init_info.Instance = static_cast<VkInstance>(*instance_);
  init_info.PhysicalDevice = static_cast<VkPhysicalDevice>(*physical_device_);
  init_info.Device = static_cast<VkDevice>(*device_);
  init_info.QueueFamily = queue_family_indices_.graphics.value();
  init_info.Queue = static_cast<VkQueue>(*graphics_queue_);
  init_info.DescriptorPool =
      static_cast<VkDescriptorPool>(*imgui_descriptor_pool_);
  init_info.MinImageCount =
      static_cast<std::uint32_t>(swapchain_images_.size());
  init_info.ImageCount = static_cast<std::uint32_t>(swapchain_images_.size());
  init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
  init_info.CheckVkResultFn = &GGEMSVulkanContext::CheckImGuiVkResult;
  init_info.UseDynamicRendering = true;
  init_info.PipelineInfoMain.PipelineRenderingCreateInfo =
      imgui_pipeline_rendering_create_info_;

  ImGui_ImplVulkan_Init(&init_info);

  imgui_initialised_ = true;

  GGEMS_INFOEX("Gui", 1, "Dear ImGui context and Vulkan backend initialised.");
}

// -----------------------------------------------------------------------------

void GGEMSVulkanContext::ShutdownImGui() noexcept {
  if (!imgui_initialised_) {
    return;
  }

  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  imgui_initialised_ = false;
}

// -----------------------------------------------------------------------------

void GGEMSVulkanContext::ApplyPendingParticleTraceSegments() {
  std::vector<ggems::render::GGEMSParticleTraceSegment> segments{};
  bool apply_segments{false};
  bool clear_segments{false};

  {
    std::scoped_lock lock{pending_particle_trace_mutex_};

    if (has_pending_particle_trace_segments_) {
      segments = std::move(pending_particle_trace_segments_);
      pending_particle_trace_segments_.clear();
      has_pending_particle_trace_segments_ = false;
      apply_segments = true;
    }

    if (pending_particle_trace_clear_) {
      pending_particle_trace_clear_ = false;
      clear_segments = true;
    }
  }

  if (!apply_segments && !clear_segments) {
    return;
  }

  device_.waitIdle();

  if (clear_segments) {
    scene_renderer_.ClearParticleTraces();
  }

  if (apply_segments) {
    scene_renderer_.SetParticleTraceSegments(segments);
  }
}

// -----------------------------------------------------------------------------

void GGEMSVulkanContext::BuildImGuiFrame() {
  GGEMS_CHECK_INTERNAL(imgui_initialised_,
                       "Dear ImGui must be initialised before building a GUI "
                       "frame.");

  ApplyPendingParticleTraceSegments();

  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  GGEMSImGuiLayer::ViewportState viewport_state =
      imgui_layer_.GetViewportState();

  if (viewport_state.visible) {
    scene_renderer_.SetViewportExtent(viewport_state.extent);
    scene_renderer_.RecreateRenderTargetsIfNeeded();
  }

  imgui_layer_.BuildFrame(swapchain_extent_, scene_renderer_.GetTextureID(),
                          scene_renderer_.GetViewportExtent(), device_status_);

  GGEMSImGuiLayer::ViewportState updated_viewport_state =
      imgui_layer_.GetViewportState();

  scene_renderer_.OrbitCamera(updated_viewport_state.orbit_delta_x_pixels,
                              updated_viewport_state.orbit_delta_y_pixels);

  scene_renderer_.PanCamera(updated_viewport_state.pan_delta_x_pixels,
                            updated_viewport_state.pan_delta_y_pixels);

  scene_renderer_.ZoomCamera(updated_viewport_state.zoom_delta);

  if (imgui_layer_.ShouldResetCamera()) {
    scene_renderer_.ResetCamera();
  }

  scene_renderer_.SetShowAxes(imgui_layer_.ShouldShowAxes());
  scene_renderer_.SetShowParticleTraces(
      imgui_layer_.ShouldShowParticleTraces());

  ImGui::Render();
}

// -----------------------------------------------------------------------------

void GGEMSVulkanContext::LoadImGuiFonts() {
  ImGuiIO &io = ImGui::GetIO();

  std::optional<std::filesystem::path> const font_path =
      FindFirstExistingFont();

  if (font_path.has_value()) {
    io.Fonts->AddFontFromFileTTF(font_path->string().c_str(), imgui_font_size_);

    GGEMS_INFOEX("Gui", 2, "Loaded ImGui font '{}'.", font_path->string());
    return;
  }

  ImFontConfig font_config{};
  font_config.SizePixels = imgui_font_size_;
  io.Fonts->AddFontDefault(&font_config);

  GGEMS_WARN("Gui",
             "No preferred monospace ImGui font was found. Falling back to "
             "Dear ImGui default font.");
}

// -----------------------------------------------------------------------------

void GGEMSVulkanContext::InitialiseSceneRenderer() {
  scene_renderer_.Initialise(physical_device_, device_,
                             vk::Format::eR8G8B8A8Unorm);
}

// -----------------------------------------------------------------------------

void GGEMSVulkanContext::ShutdownSceneRenderer() noexcept {
  scene_renderer_.Shutdown();
}
} // namespace ggems::ui
