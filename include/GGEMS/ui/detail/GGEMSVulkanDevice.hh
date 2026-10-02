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
 * \brief Owns Vulkan initialization, device selection, and window queues.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

#include "GGEMSDeviceStatus.hh"
#include "GGEMSVulkanDeviceSelection.hh"

struct GLFWwindow;

namespace ggems::ui::detail {

/*!
 * \brief Owns the Vulkan instance, window surface, device, and queues.
 *
 * The GLFW window outlives its surface. Users must finish GPU work and release
 *   dependent resources before this owner is destroyed.
 */
class GGEMSVulkanDevice {
public:
  /*!
   * \brief Creates Vulkan objects for a compatible rendering device.
   *
   * \param[in] window Borrowed GLFW window used to create the presentation
   *   surface.
   * \param[in] device_selector Automatic or explicit physical-device request.
   * \throws core::GGEMSRecoverable If required capabilities or device selection
   *   fail.
   * \throws vk::SystemError If Vulkan object creation or capability queries
   *   fail.
   */
  GGEMSVulkanDevice(GLFWwindow *window,
                    GGEMSVulkanDeviceSelector const &device_selector);

  /*! \brief Releases the owned resources. */
  ~GGEMSVulkanDevice() = default;

  /*! \brief Disallows copying the owned UI state. */
  GGEMSVulkanDevice(GGEMSVulkanDevice const &) = delete;

  /*! \brief Disallows moving the owned UI state. */
  GGEMSVulkanDevice(GGEMSVulkanDevice &&) = delete;

  /*! \brief Disallows copy assignment of the owned UI state. */
  auto operator=(GGEMSVulkanDevice const &) -> GGEMSVulkanDevice & = delete;

  /*! \brief Disallows move assignment of the owned UI state. */
  auto operator=(GGEMSVulkanDevice &&) -> GGEMSVulkanDevice & = delete;

  /*!
   * \brief Returns the Vulkan instance.
   *
   * \return Borrowed reference valid for this device owner's lifetime.
   */
  [[nodiscard]] auto GetInstance() const noexcept -> vk::raii::Instance const &;

  /*!
   * \brief Returns the selected physical device.
   *
   * \return Borrowed reference valid for this device owner's lifetime.
   */
  [[nodiscard]] auto GetPhysicalDevice() const noexcept
    -> vk::raii::PhysicalDevice const &;

  /*!
   * \brief Returns the logical rendering device.
   *
   * \return Borrowed reference valid for this device owner's lifetime.
   */
  [[nodiscard]] auto GetDevice() const noexcept -> vk::raii::Device const &;

  /*!
   * \brief Returns the window presentation surface.
   *
   * \return Borrowed reference valid for this device owner's lifetime.
   */
  [[nodiscard]] auto GetSurface() const noexcept
    -> vk::raii::SurfaceKHR const &;

  /*!
   * \brief Returns the graphics submission queue.
   *
   * \return Borrowed reference valid for this device owner's lifetime.
   */
  [[nodiscard]] auto GetGraphicsQueue() const noexcept
    -> vk::raii::Queue const &;

  /*!
   * \brief Returns the presentation queue.
   *
   * \return Borrowed reference valid for this device owner's lifetime.
   */
  [[nodiscard]] auto GetPresentationQueue() const noexcept
    -> vk::raii::Queue const &;

  /*!
   * \brief Returns the selected graphics queue-family index.
   *
   * \return Queue family used for graphics command submission.
   */
  [[nodiscard]] auto GetGraphicsQueueFamily() const noexcept -> std::uint32_t;

  /*!
   * \brief Returns the selected presentation queue-family index.
   *
   * \return Queue family supporting the window surface.
   */
  [[nodiscard]] auto GetPresentationQueueFamily() const noexcept
    -> std::uint32_t;

  /*!
   * \brief Returns the selected device description for UI display.
   *
   * \return Borrowed status record owned by this device.
   */
  [[nodiscard]] auto GetStatus() const noexcept
    -> GGEMSVulkanDeviceStatus const &;

private:
  /*! \brief Stores available graphics and surface-presentation families. */
  struct QueueFamilyIndices {
    /*! \brief Selected graphics family, absent when unsupported. */
    std::optional<std::uint32_t> graphics;

    /*! \brief Selected surface-presentation family, absent when unsupported. */
    std::optional<std::uint32_t> presentation;

    /*!
     * \brief Checks that graphics and presentation families were found.
     *
     * \return True when both family indices are present.
     */
    [[nodiscard]] auto IsComplete() const noexcept -> bool {
      return graphics.has_value() && presentation.has_value();
    }

    /*!
     * \brief Checks whether rendering and presentation use distinct families.
     *
     * \return True when both indices exist and differ.
     */
    [[nodiscard]] auto UsesSeparateFamilies() const noexcept -> bool {
      return IsComplete() && graphics.value() != presentation.value();
    }
  };

  /*!
   * \brief Creates an instance with required extensions and debug layers.
   *
   * \throws core::GGEMSRecoverable If a required layer or extension is
   *   unavailable.
   */
  auto CreateInstance() -> void;

  /*! \brief Enables warning and error logging when validation is requested. */
  auto SetupDebugMessenger() -> void;

  /*!
   * \brief Creates the Vulkan presentation surface for the GLFW window.
   *
   * \param[in] window Borrowed GLFW window.
   * \throws core::GGEMSRecoverable If GLFW cannot create the surface.
   */
  auto CreateSurface(GLFWwindow *window) -> void;

  /*!
   * \brief Collects window-system and optional validation extensions.
   *
   * \return Borrowed extension-name pointers used during instance creation.
   * \throws core::GGEMSRecoverable If GLFW reports no required extensions.
   */
  [[nodiscard]] static auto GetRequiredInstanceExtensions()
    -> std::vector<char const *>;

#if VK_HEADER_VERSION >= 304
  /*!
   * \brief Logs validation warnings and errors without aborting Vulkan calls.
   *
   * \param[in] severity Vulkan validation-message severity.
   * \param[in] type Validation-message category flags.
   * \param[in] callback_data Borrowed validation diagnostic; may be null.
   * \param[in] user_data Unused Vulkan callback payload.
   * \return VK_FALSE so the callback does not request call abortion.
   */
  static VKAPI_ATTR auto VKAPI_CALL
  DebugVkCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
                  vk::DebugUtilsMessageTypeFlagsEXT type,
                  vk::DebugUtilsMessengerCallbackDataEXT const *callback_data,
                  void *user_data) noexcept -> VkBool32;
#else
  /*!
   * \brief Logs validation warnings and errors without aborting Vulkan calls.
   *
   * \param[in] severity Vulkan validation-message severity.
   * \param[in] type Validation-message category flags.
   * \param[in] callback_data Borrowed validation diagnostic; may be null.
   * \param[in] user_data Unused Vulkan callback payload.
   * \return VK_FALSE so the callback does not request call abortion.
   */
  static VKAPI_ATTR auto VKAPI_CALL
  DebugVkCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                  VkDebugUtilsMessageTypeFlagsEXT type,
                  VkDebugUtilsMessengerCallbackDataEXT const *callback_data,
                  void *user_data) noexcept -> VkBool32;
#endif

  /*!
   * \brief Selects and retains a device meeting the current GUI requirements.
   *
   * \param[in] device_selector Automatic or explicit device request.
   * \param[in] display_adapter Optional adapter identity for the window
   *   monitor.
   * \throws core::GGEMSRecoverable If no acceptable selection is available.
   */
  auto SelectPhysicalDevice(
    GGEMSVulkanDeviceSelector const &device_selector,
    std::optional<GGEMSVulkanDisplayAdapter> const &display_adapter) -> void;

  /*!
   * \brief Collects surface suitability and identity for one Vulkan device.
   *
   * \param[in] physical_device Enumerated physical device to inspect.
   * \param[in] enumeration_index Index retained for user selection and
   *   diagnostics.
   * \return Capability record including any rejection reasons.
   */
  [[nodiscard]] auto
  BuildPhysicalDeviceCandidate(vk::raii::PhysicalDevice const &physical_device,
                               std::uint32_t enumeration_index) const
    -> GGEMSVulkanDeviceCandidate;

  /*!
   * \brief Finds graphics and window-presentation queue families.
   *
   * \param[in] physical_device Physical device whose queue families are
   *   inspected.
   * \return First available family for each required role.
   */
  [[nodiscard]] auto
  FindQueueFamilies(vk::raii::PhysicalDevice const &physical_device) const
    -> QueueFamilyIndices;

  /*!
   * \brief Checks the GUI device-extension requirements.
   *
   * \param[in] physical_device Physical device to inspect.
   * \return True when every platform-required device extension is available.
   */
  [[nodiscard]] static auto SupportsRequiredDeviceExtensions(
    vk::raii::PhysicalDevice const &physical_device) -> bool;

  /*!
   * \brief Checks dynamic rendering, synchronization, and draw parameters.
   *
   * \param[in] physical_device Physical device admitting Vulkan feature
   *   queries.
   * \return True when the required Vulkan 1.1 and 1.3 features are supported.
   */
  [[nodiscard]] static auto
  SupportsRequiredFeatures(vk::raii::PhysicalDevice const &physical_device)
    -> bool;

  /*!
   * \brief Checks available formats and presentation modes for the surface.
   *
   * \param[in] physical_device Physical device with surface-presentation
   *   support.
   * \return True when both surface formats and presentation modes are
   *   available.
   */
  [[nodiscard]] auto
  SupportsSwapchain(vk::raii::PhysicalDevice const &physical_device) const
    -> bool;

  /*!
   * \brief Warns when known renderer and monitor adapter identities differ.
   *
   * \param[in] window Window used to report physical framebuffer dimensions.
   * \param[in] display_adapter Resolved monitor adapter, when available.
   */
  auto WarnIfCrossAdapterPresentation(
    GLFWwindow *window,
    std::optional<GGEMSVulkanDisplayAdapter> const &display_adapter) const
    -> void;

  /*! \brief Creates the selected device and its required queue handles. */
  auto CreateLogicalDevice() -> void;

  /*! \brief Vulkan loader context used to create the instance. */
  vk::raii::Context context_;

  /*! \brief Owned Vulkan instance. */
  vk::raii::Instance instance_{nullptr};

  /*! \brief Optional validation-message subscription. */
  vk::raii::DebugUtilsMessengerEXT debug_messenger_{nullptr};

  /*! \brief Owned presentation surface for the borrowed GLFW window. */
  vk::raii::SurfaceKHR surface_{nullptr};

  /*! \brief Selected physical-device wrapper tied to the instance. */
  vk::raii::PhysicalDevice physical_device_{nullptr};

  /*! \brief Owned logical device supporting the GUI feature set. */
  vk::raii::Device device_{nullptr};

  /*! \brief Graphics queue obtained from the logical device. */
  vk::raii::Queue graphics_queue_{nullptr};

  /*! \brief Presentation queue obtained from the logical device. */
  vk::raii::Queue presentation_queue_{nullptr};

  /*! \brief Queue families selected for graphics and presentation. */
  QueueFamilyIndices queue_family_indices_{};

  /*! \brief Capabilities and identity retained for device diagnostics. */
  GGEMSVulkanDeviceCandidate selected_physical_device_candidate_{};

  /*! \brief Device description exposed to the status panel. */
  GGEMSVulkanDeviceStatus status_{};

  /*! \brief Vulkan API version requested by the GUI instance. */
  static constexpr std::uint32_t k_vulkan_api_version_{vk::ApiVersion13};
};

} // namespace ggems::ui::detail
