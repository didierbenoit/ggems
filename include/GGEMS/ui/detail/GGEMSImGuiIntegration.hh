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
 * \brief Owns Dear ImGui backends, scene texture binding, and settings.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>
#include <optional>
#include <filesystem>
#include <string>

#include <imgui.h>
#include <vulkan/vulkan.hpp>

struct GLFWwindow;
struct ImGuiContext;

namespace ggems::ui::detail {

/*!
 * \brief Owns the Dear ImGui context and GLFW/Vulkan backend lifecycle.
 *
 * Window and Vulkan handles are borrowed and must outlive the backends. The
 *   application synchronizes GPU work before backend or texture replacement.
 */
class GGEMSImGuiIntegration {
public:
  /*! \brief Borrows the Vulkan objects required by the ImGui backend. */
  struct VulkanHandles {
    /*! \brief Borrowed Vulkan instance. */
    vk::Instance instance{};

    /*! \brief Borrowed selected physical device. */
    vk::PhysicalDevice physical_device{};

    /*! \brief Borrowed logical device. */
    vk::Device device{};

    /*! \brief Borrowed queue receiving ImGui rendering commands. */
    vk::Queue graphics_queue{};

    /*! \brief Queue-family index associated with the graphics queue. */
    std::uint32_t graphics_queue_family{0U};
  };

  /*! \brief Describes swapchain compatibility for an ImGui backend. */
  struct VulkanBackendEpoch {
    /*! \brief Actual number of swapchain images. */
    std::uint32_t image_count{0U};

    /*! \brief Backend minimum image count, at least two. */
    std::uint32_t min_image_count{0U};

    /*! \brief Main-pass color attachment format. */
    vk::Format color_format{vk::Format::eUndefined};

    /*! \brief Main-pass multisample count. */
    vk::SampleCountFlagBits sample_count{vk::SampleCountFlagBits::e1};

    /*! \brief Main-pass dynamic-rendering multiview mask. */
    std::uint32_t view_mask{0U};

    /*! \brief Main-pass depth format, or undefined when unused. */
    vk::Format depth_format{vk::Format::eUndefined};

    /*! \brief Main-pass stencil format, or undefined when unused. */
    vk::Format stencil_format{vk::Format::eUndefined};

    /*!
     * \brief Compares all backend compatibility fields.
     *
     * \return True when all backend compatibility fields are equal.
     */
    [[nodiscard]] auto operator==(VulkanBackendEpoch const &) const noexcept
      -> bool = default;
  };

  /*! \brief Creates an inactive UI component. */
  GGEMSImGuiIntegration() = default;

  /*! \brief Saves settings and releases the context and backends. */
  ~GGEMSImGuiIntegration() noexcept;

  /*! \brief Disallows copying the owned UI state. */
  GGEMSImGuiIntegration(GGEMSImGuiIntegration const &) = delete;

  /*! \brief Disallows moving the owned UI state. */
  GGEMSImGuiIntegration(GGEMSImGuiIntegration &&) = delete;

  /*! \brief Disallows copy assignment of the owned UI state. */
  auto operator=(GGEMSImGuiIntegration const &)
    -> GGEMSImGuiIntegration & = delete;

  /*! \brief Disallows move assignment of the owned UI state. */
  auto operator=(GGEMSImGuiIntegration &&) -> GGEMSImGuiIntegration & = delete;

  /*!
   * \brief Creates the context and attaches GLFW and Vulkan backends.
   *
   * An existing context makes this call a no-op; failed initialization releases
   *   the created state.
   *
   * \param[in] window Borrowed non-null GLFW window.
   * \param[in] handles Borrowed Vulkan objects valid until shutdown.
   * \param[in] epoch Initial swapchain rendering compatibility.
   * \throws core::GGEMSInternal If the window is null.
   * \throws core::GGEMSRecoverable If a backend cannot initialize.
   */
  auto Initialize(GLFWwindow *window, VulkanHandles const &handles,
                  VulkanBackendEpoch const &epoch) -> void;

  /*!
   * \brief Releases backends and the context and attempts to save settings.
   *
   * The caller must complete outstanding GPU use before shutdown.
   */
  auto Shutdown() noexcept -> void;

  /*!
   * \brief Replaces the backends when rendering compatibility changes.
   *
   * Equal parameters leave the backends intact. A retained scene view is
   *   registered again after replacement; GPU use must already be complete.
   *
   * \param[in] epoch New swapchain compatibility parameters.
   * \throws core::GGEMSInternal If no Vulkan backend is attached.
   * \throws core::GGEMSRecoverable If backend initialization or rendering has
   *   failed.
   */
  auto UpdateVulkanBackend(VulkanBackendEpoch const &epoch) -> void;

  /*!
   * \brief Replaces the descriptor used to sample the scene image.
   *
   * The view must remain valid until unregistered and use shader-read-only
   *   layout when sampled.
   *
   * \param[in] image_view Borrowed sampled view, or a null view to release the
   *   binding.
   * \throws core::GGEMSInternal If no Vulkan backend is attached.
   * \throws core::GGEMSRecoverable If the Vulkan backend has failed.
   */
  auto RegisterSceneTexture(vk::ImageView image_view) -> void;

  /*! \brief Releases the descriptor and forgets the borrowed scene view. */
  auto UnregisterSceneTexture() noexcept -> void;

  /*!
   * \brief Returns the current scene descriptor as an ImGui texture ID.
   *
   * \return Borrowed texture ID, or zero when no scene is registered.
   */
  [[nodiscard]] auto GetSceneTextureID() const noexcept -> ImTextureID;

  /*!
   * \brief Starts a backend frame and refreshes settings and content scale.
   *
   * \throws core::GGEMSInternal If no Vulkan backend is attached.
   * \throws core::GGEMSRecoverable If the Vulkan backend has failed.
   */
  auto BeginFrame() -> void;

  /*!
   * \brief Finalizes the ImGui frame and records its draw commands.
   *
   * \param[in] command_buffer Recording command buffer inside a compatible main
   *   rendering pass.
   * \throws core::GGEMSRecoverable If the Vulkan backend has failed.
   */
  auto RenderDrawData(vk::CommandBuffer command_buffer) const -> void;

  /*! \brief Stores physical framebuffer pixels per logical UI pixel. */
  struct FramebufferDensity {
    /*! \brief Horizontal framebuffer density. */
    float x{1.0F};

    /*! \brief Vertical framebuffer density. */
    float y{1.0F};
  };

  /*!
   * \brief Reads usable framebuffer density from the current ImGui context.
   *
   * \return Per-axis densities, substituting one for nonfinite or nonpositive
   *   values.
   */
  [[nodiscard]] static auto GetFramebufferDensity() noexcept
    -> FramebufferDensity;

  /*!
   * \brief Reports the first retained Vulkan backend failure.
   *
   * \throws core::GGEMSRecoverable If a negative backend result was recorded.
   */
  auto ThrowIfBackendFailed() const -> void;

private:
  /*!
   * \brief Attaches the GLFW backend and installs its window callbacks.
   *
   * \throws core::GGEMSRecoverable If GLFW backend initialization fails.
   */
  auto AttachGlfwBackend() -> void;

  /*! \brief Shuts down an attached GLFW backend. */
  auto DetachGlfwBackend() noexcept -> void;

  /*!
   * \brief Attaches a backend compatible with the supplied rendering epoch.
   *
   * \param[in] epoch Swapchain image count and attachment compatibility.
   * \throws core::GGEMSRecoverable If image counts are invalid or
   *   initialization fails.
   */
  auto AttachVulkanBackend(VulkanBackendEpoch const &epoch) -> void;

  /*! \brief Releases the scene descriptor and attached Vulkan backend. */
  auto DetachVulkanBackend() noexcept -> void;

  /*! \brief Releases only the scene descriptor while retaining its view. */
  auto ReleaseSceneDescriptor() noexcept -> void;

  /*! \brief Loads user settings and disables persistence after read failure. */
  auto LoadSettings() -> void;

  /*! \brief Persists changed layout text and reports write failures. */
  auto SaveSettings() -> void;

  /*!
   * \brief Applies window content scale to the current GGEMS style.
   *
   * \param[in] content_scale Admitted content-density multiplier.
   */
  auto ApplyContentScale(float content_scale) -> void;

  /*! \brief Loads an available monospace font or ImGui default font. */
  static auto LoadFonts() -> void;

  /*!
   * \brief Retains the first backend error without throwing through Vulkan.
   *
   * \param[in] result Backend Vulkan result; nonnegative values are ignored.
   */
  static auto RecordBackendResult(VkResult result) noexcept -> void;

  /*! \brief Borrowed GLFW window used by the platform backend. */
  GLFWwindow *window_{nullptr};

  /*! \brief Borrowed Vulkan objects retained for backend replacement. */
  VulkanHandles handles_{};

  /*! \brief Compatibility parameters of the attached Vulkan backend. */
  VulkanBackendEpoch epoch_{};

  /*! \brief Stable format storage referenced by backend initialization. */
  VkFormat color_attachment_format_{VK_FORMAT_UNDEFINED};

  /*! \brief Owned Dear ImGui context, or null before initialization. */
  ImGuiContext *context_{nullptr};

  /*! \brief Whether the GLFW backend requires shutdown. */
  bool glfw_backend_attached_{false};

  /*! \brief Whether the Vulkan backend requires shutdown. */
  bool vulkan_backend_attached_{false};

  /*! \brief Borrowed scene view retained across backend replacement. */
  vk::ImageView scene_image_view_{};

  /*! \brief Owned backend descriptor for sampling the scene image. */
  VkDescriptorSet scene_descriptor_set_{VK_NULL_HANDLE};

  /*! \brief Content scale currently applied to style and fonts. */
  float applied_content_scale_{0.0F};

  /*!
   * \brief Writable settings destination, absent when persistence is disabled.
   */
  std::optional<std::filesystem::path> settings_path_;

  /*! \brief Last successfully loaded or written settings text. */
  std::string persisted_settings_;

  /*! \brief Suppresses repeated warnings during a write failure episode. */
  bool settings_write_failed_{false};

  /*! \brief First negative Vulkan result reported by the backend callback. */
  std::optional<vk::Result> backend_failure_;
};
} // namespace ggems::ui::detail
