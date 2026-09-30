#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <optional>
#include <vector>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include "GGEMS/ui/detail/GGEMSImGuiIntegration.hh"

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/logging/GGEMSLogMacros.hh"
#include "GGEMS/ui/GGEMSImGuiTheme.hh"

namespace {

constexpr float k_min_ui_scale{1.0F};
constexpr float k_max_ui_scale{2.5F};
constexpr float k_base_font_size{15.0F};
constexpr std::uint32_t k_sampled_image_pool_size{16U};
constexpr std::uint32_t k_min_backend_image_count{2U};
constexpr std::uint32_t k_vulkan_api_version{vk::ApiVersion13};

// =============================================================================
// =============================================================================

[[nodiscard]] auto NormalizeUIScale(float content_scale_x,
                                    float content_scale_y) noexcept -> float {
  return std::clamp(std::max(content_scale_x, content_scale_y), k_min_ui_scale,
                    k_max_ui_scale);
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto FindFirstExistingFont()
  -> std::optional<std::filesystem::path> {
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

#ifdef __APPLE__
  if (char const *home = std::getenv("HOME"); home != nullptr) {
    std::filesystem::path const user_fonts =
      std::filesystem::path{home} / "Library/Fonts";

    candidates.emplace_back(user_fonts / "JetBrainsMono-Regular.ttf");
    candidates.emplace_back(user_fonts / "JetBrainsMonoNerdFont-Regular.ttf");
    candidates.emplace_back(user_fonts /
                            "JetBrainsMonoNerdFontMono-Regular.ttf");
  }

  candidates.emplace_back("/System/Library/Fonts/Menlo.ttc");
  candidates.emplace_back("/System/Library/Fonts/Monaco.dfont");
#endif

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

} // namespace

namespace ggems::ui::detail {

// =============================================================================
// =============================================================================

GGEMSImGuiIntegration::~GGEMSImGuiIntegration() noexcept { Shutdown(); }

// -----------------------------------------------------------------------------

auto GGEMSImGuiIntegration::Initialize(GLFWwindow *window,
                                       VulkanHandles const &handles,
                                       VulkanBackendEpoch const &epoch)
  -> void {
  if (context_ != nullptr) {
    return;
  }

  if (!(window != nullptr)) {
    throw ggems::core::GGEMSInternal(
      "A valid GLFW window is required before initializing Dear ImGui.");
  }

  window_ = window;
  handles_ = handles;

  try {
    float content_scale_x{1.0F};
    float content_scale_y{1.0F};

    glfwGetWindowContentScale(window, &content_scale_x, &content_scale_y);

    float const ui_scale = NormalizeUIScale(content_scale_x, content_scale_y);
    float const font_size = k_base_font_size * ui_scale;

    IMGUI_CHECKVERSION();

    context_ = ImGui::CreateContext();

    ImGuiIO &imgui_io = ImGui::GetIO();
    imgui_io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    imgui_io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    imgui_io.UserData = this;

    ImGui::StyleColorsDark();
    ApplyGGEMSImGuiTheme();
    ImGui::GetStyle().ScaleAllSizes(ui_scale);

    LoadFonts(font_size);

    GGEMS_INFO("Gui", "ImGui content scale: x={:.2f}, y={:.2f}.",
               content_scale_x, content_scale_y);
    GGEMS_INFO("Gui", "ImGui UI scale: {:.2f}.", ui_scale);
    GGEMS_INFO("Gui", "ImGui font size: {:.2f} px.", font_size);

    AttachGlfwBackend();
    AttachVulkanBackend(epoch);
  } catch (...) {
    Shutdown();
    throw;
  }

  GGEMS_INFOEX("Gui", 1, "Dear ImGui context and Vulkan backend initialized.");
}

// -----------------------------------------------------------------------------

auto GGEMSImGuiIntegration::Shutdown() noexcept -> void {
  DetachVulkanBackend();
  DetachGlfwBackend();

  if (context_ != nullptr) {
    ImGui::DestroyContext(context_);
    context_ = nullptr;
  }

  scene_image_view_ = vk::ImageView{};
  backend_failure_.reset();
  handles_ = VulkanHandles{};
  window_ = nullptr;
}

// -----------------------------------------------------------------------------

auto GGEMSImGuiIntegration::UpdateVulkanBackend(VulkanBackendEpoch const &epoch)
  -> void {
  if (!vulkan_backend_attached_) {
    throw ggems::core::GGEMSInternal(
      "The Dear ImGui Vulkan backend must be attached before updating "
      "its epoch.");
  }

  ThrowIfBackendFailed();

  if (epoch == epoch_) {
    return;
  }

  DetachVulkanBackend();
  DetachGlfwBackend();
  AttachGlfwBackend();
  AttachVulkanBackend(epoch);

  if (scene_image_view_ != vk::ImageView{}) {
    RegisterSceneTexture(scene_image_view_);
  }

  GGEMS_INFOEX("Gui", 1,
               "Dear ImGui Vulkan backend replaced: image count {}, "
               "color format {}.",
               epoch.image_count, vk::to_string(epoch.color_format));
}

// -----------------------------------------------------------------------------

auto GGEMSImGuiIntegration::RegisterSceneTexture(vk::ImageView image_view)
  -> void {
  if (!vulkan_backend_attached_) {
    throw ggems::core::GGEMSInternal(
      "The Dear ImGui Vulkan backend must be attached before registering "
      "the scene texture.");
  }

  ThrowIfBackendFailed();

  ReleaseSceneDescriptor();

  scene_image_view_ = image_view;

  if (image_view == vk::ImageView{}) {
    return;
  }

  scene_descriptor_set_ =
    ImGui_ImplVulkan_AddTexture(static_cast<VkImageView>(image_view),
                                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  ThrowIfBackendFailed();
}

// -----------------------------------------------------------------------------

auto GGEMSImGuiIntegration::UnregisterSceneTexture() noexcept -> void {
  ReleaseSceneDescriptor();
  scene_image_view_ = vk::ImageView{};
}

// -----------------------------------------------------------------------------

auto GGEMSImGuiIntegration::GetSceneTextureID() const noexcept -> ImTextureID {
  return reinterpret_cast<ImTextureID>(scene_descriptor_set_);
}

// -----------------------------------------------------------------------------

auto GGEMSImGuiIntegration::BeginFrame() const -> void {
  if (!vulkan_backend_attached_) {
    throw ggems::core::GGEMSInternal(
      "Dear ImGui must be initialized before beginning a GUI frame.");
  }

  ThrowIfBackendFailed();

  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
}

// -----------------------------------------------------------------------------

auto GGEMSImGuiIntegration::RenderDrawData(
  vk::CommandBuffer command_buffer) const -> void {
  ThrowIfBackendFailed();

  ImGui::Render();

  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(),
                                  static_cast<VkCommandBuffer>(command_buffer));

  ThrowIfBackendFailed();
}

// -----------------------------------------------------------------------------

auto GGEMSImGuiIntegration::ThrowIfBackendFailed() const -> void {
  if (!backend_failure_.has_value()) {
    return;
  }

  throw ggems::core::GGEMSRecoverable(
    std::format("The Dear ImGui Vulkan backend failed: {}.",
                vk::to_string(*backend_failure_)));
}

// -----------------------------------------------------------------------------

auto GGEMSImGuiIntegration::AttachGlfwBackend() -> void {
  if (!ImGui_ImplGlfw_InitForVulkan(window_, true)) {
    throw ggems::core::GGEMSRecoverable(
      "Unable to initialize the Dear ImGui GLFW backend.");
  }

  glfw_backend_attached_ = true;
}

// -----------------------------------------------------------------------------

auto GGEMSImGuiIntegration::DetachGlfwBackend() noexcept -> void {
  if (glfw_backend_attached_) {
    ImGui_ImplGlfw_Shutdown();
    glfw_backend_attached_ = false;
  }
}

// -----------------------------------------------------------------------------

auto GGEMSImGuiIntegration::AttachVulkanBackend(VulkanBackendEpoch const &epoch)
  -> void {
  if (epoch.min_image_count < k_min_backend_image_count ||
      epoch.image_count < epoch.min_image_count) {
    throw ggems::core::GGEMSRecoverable(std::format(
      "The Dear ImGui Vulkan backend requires at least {} swapchain images "
      "(min {}, actual {}).",
      k_min_backend_image_count, epoch.min_image_count, epoch.image_count));
  }

  color_attachment_format_ = static_cast<VkFormat>(epoch.color_format);

  ImGui_ImplVulkan_InitInfo init_info{};
  init_info.ApiVersion = k_vulkan_api_version;
  init_info.Instance = static_cast<VkInstance>(handles_.instance);
  init_info.PhysicalDevice =
    static_cast<VkPhysicalDevice>(handles_.physical_device);
  init_info.Device = static_cast<VkDevice>(handles_.device);
  init_info.QueueFamily = handles_.graphics_queue_family;
  init_info.Queue = static_cast<VkQueue>(handles_.graphics_queue);
  init_info.DescriptorPoolSize = k_sampled_image_pool_size;
  init_info.MinImageCount = epoch.min_image_count;
  init_info.ImageCount = epoch.image_count;
  init_info.PipelineInfoMain.MSAASamples =
    static_cast<VkSampleCountFlagBits>(epoch.sample_count);
  init_info.UseDynamicRendering = true;
  init_info.PipelineInfoMain.PipelineRenderingCreateInfo =
    VkPipelineRenderingCreateInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
      .pNext = nullptr,
      .viewMask = epoch.view_mask,
      .colorAttachmentCount = 1U,
      .pColorAttachmentFormats = &color_attachment_format_,
      .depthAttachmentFormat = static_cast<VkFormat>(epoch.depth_format),
      .stencilAttachmentFormat = static_cast<VkFormat>(epoch.stencil_format),
  };
  init_info.CheckVkResultFn = &GGEMSImGuiIntegration::RecordBackendResult;

  if (!ImGui_ImplVulkan_Init(&init_info)) {
    throw ggems::core::GGEMSRecoverable(
      "Unable to initialize the Dear ImGui Vulkan backend.");
  }

  vulkan_backend_attached_ = true;
  epoch_ = epoch;

  ThrowIfBackendFailed();
}

// -----------------------------------------------------------------------------

auto GGEMSImGuiIntegration::DetachVulkanBackend() noexcept -> void {
  ReleaseSceneDescriptor();

  if (vulkan_backend_attached_) {
    ImGui_ImplVulkan_Shutdown();
    vulkan_backend_attached_ = false;
  }

  epoch_ = VulkanBackendEpoch{};
}

// -----------------------------------------------------------------------------

auto GGEMSImGuiIntegration::ReleaseSceneDescriptor() noexcept -> void {
  if (scene_descriptor_set_ == VK_NULL_HANDLE) {
    return;
  }

  if (vulkan_backend_attached_) {
    ImGui_ImplVulkan_RemoveTexture(scene_descriptor_set_);
  }

  scene_descriptor_set_ = VK_NULL_HANDLE;
}

// -----------------------------------------------------------------------------

auto GGEMSImGuiIntegration::LoadFonts(float font_size) -> void {
  ImGuiIO &imgui_io = ImGui::GetIO();

  std::optional<std::filesystem::path> const font_path =
    FindFirstExistingFont();

  if (font_path.has_value()) {
    imgui_io.Fonts->AddFontFromFileTTF(font_path->string().c_str(), font_size);

    GGEMS_INFOEX("Gui", 2, "Loaded ImGui font '{}'.", font_path->string());
    return;
  }

  ImFontConfig font_config{};
  font_config.SizePixels = font_size;
  imgui_io.Fonts->AddFontDefault(&font_config);

  GGEMS_WARN("Gui",
             "No preferred monospace ImGui font was found. Falling back to "
             "Dear ImGui default font.");
}

// -----------------------------------------------------------------------------

auto GGEMSImGuiIntegration::RecordBackendResult(VkResult result) noexcept
  -> void {
  if (result >= VK_SUCCESS) {
    return;
  }

  auto *integration =
    static_cast<GGEMSImGuiIntegration *>(ImGui::GetIO().UserData);

  if (integration != nullptr && !integration->backend_failure_.has_value()) {
    integration->backend_failure_ = static_cast<vk::Result>(result);
  }
}

} // namespace ggems::ui::detail
