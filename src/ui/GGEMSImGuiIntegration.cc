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
 * \brief Manages Dear ImGui backends, scaling, fonts, and persisted settings.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

/*! \brief Keeps GLFW from selecting a graphics API include boundary. */
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include "GGEMS/ui/detail/GGEMSImGuiIntegration.hh"

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/logging/GGEMSLogMacros.hh"
#include "GGEMS/ui/detail/GGEMSImGuiSettingsFile.hh"
#include "GGEMS/ui/detail/GGEMSImGuiTheme.hh"
#include "GGEMS/ui/detail/GGEMSPresentationScale.hh"

namespace {

/*! \brief Descriptor-pool capacity requested from the Vulkan backend. */
constexpr std::uint32_t k_sampled_image_pool_size{16U};

/*! \brief Minimum swapchain image count admitted by the backend. */
constexpr std::uint32_t k_min_backend_image_count{2U};

/*! \brief Vulkan API version supplied to Dear ImGui. */
constexpr std::uint32_t k_vulkan_api_version{vk::ApiVersion13};

// =============================================================================
// =============================================================================

/*!
 * \brief Finds the first installed font in the preferred monospace list.
 *
 * \return Existing font path, or no value when no candidate is present.
 */
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

    std::filesystem::path const jetbrains_nerd =
      user_fonts / "JetBrainsMonoNerd";

    candidates.emplace_back(jetbrains_nerd /
                            "JetBrainsMonoNerdFont-Regular.ttf");
    candidates.emplace_back(jetbrains_nerd /
                            "JetBrainsMonoNerdFontMono-Regular.ttf");
    candidates.emplace_back(jetbrains_nerd /
                            "JetBrainsMonoNLNerdFont-Regular.ttf");
    candidates.emplace_back(jetbrains_nerd /
                            "JetBrainsMonoNLNerdFontMono-Regular.ttf");
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
    IMGUI_CHECKVERSION();

    context_ = ImGui::CreateContext();

    ImGuiIO &imgui_io = ImGui::GetIO();
    imgui_io.IniFilename = nullptr;
    imgui_io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    imgui_io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    imgui_io.UserData = this;

    LoadSettings();
    LoadFonts();

    AttachGlfwBackend();
    ApplyContentScale(
      NormalizeContentScale(ImGui_ImplGlfw_GetContentScaleForWindow(window_)));
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
    try {
      if (ImGui::GetFrameCount() > 0) {
        SaveSettings();
      }
    } catch (...) {
      std::fputs("[GGEMS Gui] Failed to save the Dear ImGui settings during "
                 "shutdown.\n",
                 stderr);
    }

    ImGui::DestroyContext(context_);
    context_ = nullptr;
  }

  scene_image_view_ = vk::ImageView{};
  applied_content_scale_ = 0.0F;
  settings_path_.reset();
  persisted_settings_.clear();
  settings_write_failed_ = false;
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

auto GGEMSImGuiIntegration::BeginFrame() -> void {
  if (!vulkan_backend_attached_) {
    throw ggems::core::GGEMSInternal(
      "Dear ImGui must be initialized before beginning a GUI frame.");
  }

  ThrowIfBackendFailed();

  if (ImGui::GetIO().WantSaveIniSettings) {
    SaveSettings();
  }

  float const content_scale =
    NormalizeContentScale(ImGui_ImplGlfw_GetContentScaleForWindow(window_));

  if (content_scale != applied_content_scale_) {
    ApplyContentScale(content_scale);
  }

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

auto GGEMSImGuiIntegration::GetFramebufferDensity() noexcept
  -> FramebufferDensity {
  ImVec2 const scale = ImGui::GetIO().DisplayFramebufferScale;

  auto const Admit = [](float density) noexcept -> float {
    return std::isfinite(density) && density > 0.0F ? density : 1.0F;
  };

  return FramebufferDensity{.x = Admit(scale.x), .y = Admit(scale.y)};
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

auto GGEMSImGuiIntegration::LoadSettings() -> void {
  settings_path_ = BuildImGuiSettingsPath(ReadUserDirectories());

  if (!settings_path_.has_value()) {
    GGEMS_WARN("Gui", "No per-user configuration directory is available: the "
                      "Dear ImGui layout will not be saved.");
    return;
  }

  GGEMS_INFO("Gui", "Dear ImGui settings file: '{}'.", ToUtf8(*settings_path_));

  GGEMSSettingsFileRead const settings = ReadImGuiSettingsFile(*settings_path_);

  switch (settings.status) {
  case GGEMSSettingsFileRead::Status::Missing:
    return;

  case GGEMSSettingsFileRead::Status::Failed:
    GGEMS_WARN("Gui",
               "Unable to read the Dear ImGui settings file '{}' ({}): using "
               "the default layout; layout changes will not be saved during "
               "this session.",
               ToUtf8(*settings_path_), settings.error);
    settings_path_.reset();
    return;

  case GGEMSSettingsFileRead::Status::Loaded:
    break;
  }

  if (!settings.text.empty()) {
    ImGui::LoadIniSettingsFromMemory(settings.text.data(),
                                     settings.text.size());
  }

  persisted_settings_ = settings.text;
}

// -----------------------------------------------------------------------------

auto GGEMSImGuiIntegration::SaveSettings() -> void {
  ImGui::GetIO().WantSaveIniSettings = false;

  if (!settings_path_.has_value()) {
    return;
  }

  std::size_t size{0U};
  char const *data = ImGui::SaveIniSettingsToMemory(&size);
  std::string_view const settings{data, size};

  if (settings == persisted_settings_) {
    return;
  }

  std::optional<std::string> const error =
    WriteImGuiSettingsFile(*settings_path_, settings);

  if (error.has_value()) {
    if (!settings_write_failed_) {
      GGEMS_WARN("Gui",
                 "Unable to save the Dear ImGui settings ({}): the current "
                 "layout remains usable but is not persisted.",
                 *error);
      settings_write_failed_ = true;
    }

    return;
  }

  persisted_settings_ = settings;
  settings_write_failed_ = false;
}

// -----------------------------------------------------------------------------

auto GGEMSImGuiIntegration::ApplyContentScale(float content_scale) -> void {
  ImGui::GetStyle() = BuildGGEMSStyle(k_default_user_ui_scale, content_scale);
  applied_content_scale_ = content_scale;

  GGEMS_INFO("Gui",
             "ImGui UI scale: {:.2f} (user {:.2f}, content {:.2f}); font size "
             "{:.2f} px.",
             k_default_user_ui_scale * content_scale, k_default_user_ui_scale,
             content_scale,
             k_ggems_base_font_size * k_default_user_ui_scale * content_scale);
}

// -----------------------------------------------------------------------------

auto GGEMSImGuiIntegration::LoadFonts() -> void {
  ImGuiIO &imgui_io = ImGui::GetIO();

  std::optional<std::filesystem::path> const font_path =
    FindFirstExistingFont();

  if (font_path.has_value()) {
    imgui_io.Fonts->AddFontFromFileTTF(ToUtf8(*font_path).c_str(),
                                       k_ggems_base_font_size);

    GGEMS_INFOEX("Gui", 2, "Loaded ImGui font '{}'.", ToUtf8(*font_path));
    return;
  }

  ImFontConfig font_config{};
  font_config.SizePixels = k_ggems_base_font_size;
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
