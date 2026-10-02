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
 * \brief Owns swapchain generations and synchronized frame submission.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <vector>

#include <vulkan/vulkan_raii.hpp>

namespace ggems::ui::detail {

class GGEMSVulkanDevice;

/*!
 * \brief Owns presentation images and reusable frame synchronization.
 *
 * The borrowed device must outlive this object. Each frame acquires an image,
 *   begins recording, begins the main pass, and ends with submission and
 *   presentation. GPU use must finish before destruction.
 */
class GGEMSVulkanPresenter {
public:
  /*!
   * \brief Creates frame resources and the initial window swapchain.
   *
   * \param[in] device Borrowed device and surface owner.
   * \param[in] framebuffer_extent Requested presentation size in physical
   *   pixels.
   */
  GGEMSVulkanPresenter(GGEMSVulkanDevice const &device,
                       vk::Extent2D const &framebuffer_extent);

  /*! \brief Releases the owned resources. */
  ~GGEMSVulkanPresenter() = default;

  /*! \brief Disallows copying the owned UI state. */
  GGEMSVulkanPresenter(GGEMSVulkanPresenter const &) = delete;

  /*! \brief Disallows moving the owned UI state. */
  GGEMSVulkanPresenter(GGEMSVulkanPresenter &&) = delete;

  /*! \brief Disallows copy assignment of the owned UI state. */
  auto operator=(GGEMSVulkanPresenter const &)
    -> GGEMSVulkanPresenter & = delete;

  /*! \brief Disallows move assignment of the owned UI state. */
  auto operator=(GGEMSVulkanPresenter &&) -> GGEMSVulkanPresenter & = delete;

  /*!
   * \brief Waits for device idle and replaces the window swapchain.
   *
   * A zero resulting extent leaves the current generation unchanged.
   *
   * \param[in] framebuffer_extent Requested physical pixel extent, constrained
   *   by the surface.
   */
  auto RecreateSwapchain(vk::Extent2D const &framebuffer_extent) -> void;

  /*!
   * \brief Returns the active swapchain image count.
   *
   * \return Number of images in the current generation.
   */
  [[nodiscard]] auto GetImageCount() const noexcept -> std::uint32_t;

  /*!
   * \brief Returns the active swapchain color format.
   *
   * \return Format used by the main rendering pass.
   */
  [[nodiscard]] auto GetImageFormat() const noexcept -> vk::Format;

  /*!
   * \brief Returns the active physical presentation extent.
   *
   * \return Borrowed extent record; its values change on recreation.
   */
  [[nodiscard]] auto GetExtent() const noexcept -> vk::Extent2D const &;

  /*!
   * \brief Waits for a reusable frame slot and acquires a swapchain image.
   *
   * \throws core::GGEMSRecoverable If a previous acquisition remains
   *   outstanding or synchronization fails.
   * \throws vk::OutOfDateKHRError If the swapchain requires recreation.
   */
  auto AcquireImage() -> void;

  /*!
   * \brief Begins recording commands in the current reusable frame slot.
   *
   * \return Borrowed recording command buffer for the acquired frame.
   */
  [[nodiscard]] auto BeginRecording() -> vk::raii::CommandBuffer const &;

  /*!
   * \brief Begins dynamic rendering on the acquired swapchain image.
   *
   * Requires an acquired image and a recording frame command buffer.
   */
  auto BeginMainPass() -> void;

  /*!
   * \brief Ends rendering, submits the frame, and presents its image.
   *
   * Requires the main pass to be active. Submission remains fenced even if
   *   presentation throws.
   *
   * \return True when acquisition or presentation reports a suboptimal
   *   swapchain.
   */
  [[nodiscard]] auto EndFrame() -> bool;

private:
  /*! \brief Maximum number of reusable CPU submission slots. */
  static constexpr std::uint32_t k_frame_slot_count_{2U};

  /*! \brief Collects surface limits and available presentation choices. */
  struct SwapchainSupportDetails {
    /*! \brief Current surface image-count and extent constraints. */
    vk::SurfaceCapabilitiesKHR capabilities{};

    /*! \brief Available color formats and color spaces. */
    std::vector<vk::SurfaceFormatKHR> surface_formats;

    /*! \brief Available presentation scheduling modes. */
    std::vector<vk::PresentModeKHR> present_modes;
  };

  /*! \brief Owns the resources associated with one swapchain generation. */
  struct SwapchainGeneration {
    /*! \brief Owned swapchain for this generation. */
    vk::raii::SwapchainKHR swapchain{nullptr};

    /*! \brief Image handles owned by the swapchain. */
    std::vector<vk::Image> images;

    /*! \brief Owned color views corresponding to the swapchain images. */
    std::vector<vk::raii::ImageView> image_views;

    /*! \brief Per-image semaphores waited on by presentation. */
    std::vector<vk::raii::Semaphore> render_finished_semaphores;

    /*! \brief Color format shared by the generation images. */
    vk::Format image_format{vk::Format::eUndefined};

    /*! \brief Physical pixel size shared by the generation images. */
    vk::Extent2D extent{};
  };

  /*! \brief Owns the resources reused after one submission completes. */
  struct FrameSlot {
    /*! \brief Primary command buffer used by this submission slot. */
    vk::raii::CommandBuffer command_buffer{nullptr};

    /*! \brief Acquisition signal waited on by graphics submission. */
    vk::raii::Semaphore image_available_semaphore{nullptr};

    /*! \brief Fence protecting reuse of this slot after submission. */
    vk::raii::Fence submit_fence{nullptr};
  };

  /*! \brief Tracks the image owned by the current unfinished frame. */
  struct AcquiredImage {
    /*! \brief Index in the active swapchain generation. */
    std::uint32_t index{0U};

    /*! \brief Whether acquisition recommended swapchain recreation. */
    bool suboptimal{false};
  };

  /*! \brief Creates a resettable graphics-family command pool. */
  auto CreateCommandPool() -> void;

  /*! \brief Allocates submission buffers, acquisition signals, and fences. */
  auto CreateFrameSlots() -> void;

  /*!
   * \brief Creates a swapchain and the resources tied to its images.
   *
   * \param[in] support_details Queried surface capabilities and supported
   *   choices.
   * \param[in] extent Admitted physical pixel extent.
   * \param[in] old_swapchain Previous generation handle, or null for initial
   *   creation.
   * \return New owning generation ready for presentation.
   */
  [[nodiscard]] auto CreateSwapchainGeneration(
    SwapchainSupportDetails const &support_details, vk::Extent2D const &extent,
    vk::SwapchainKHR old_swapchain) -> SwapchainGeneration;

  /*!
   * \brief Queries the selected device for the current window surface.
   *
   * \return Surface capabilities, formats, and presentation modes.
   */
  [[nodiscard]] auto QuerySwapchainSupport() const -> SwapchainSupportDetails;

  /*!
   * \brief Prefers BGRA sRGB with nonlinear sRGB presentation.
   *
   * \param[in] surface_formats Nonempty surface-format list.
   * \return Preferred format when available, otherwise the first format.
   * \throws core::GGEMSInternal If the list is empty.
   */
  [[nodiscard]] static auto ChooseSwapchainSurfaceFormat(
    std::vector<vk::SurfaceFormatKHR> const &surface_formats)
    -> vk::SurfaceFormatKHR;

  /*!
   * \brief Prefers FIFO presentation from the available modes.
   *
   * \param[in] present_modes Nonempty presentation-mode list.
   * \return FIFO when available, otherwise the first mode.
   * \throws core::GGEMSInternal If the list is empty.
   */
  [[nodiscard]] static auto ChooseSwapchainPresentMode(
    std::vector<vk::PresentModeKHR> const &present_modes) -> vk::PresentModeKHR;

  /*!
   * \brief Resolves the surface-fixed or constrained framebuffer extent.
   *
   * \param[in] capabilities Surface extent limits and current fixed extent.
   * \param[in] framebuffer_extent Requested physical pixel extent.
   * \return Extent admitted by the window surface.
   */
  [[nodiscard]] static auto
  ChooseSwapchainExtent(vk::SurfaceCapabilitiesKHR const &capabilities,
                        vk::Extent2D const &framebuffer_extent) -> vk::Extent2D;

  /*!
   * \brief Records a layout transition for the acquired image.
   *
   * \param[in] old_layout Current image layout.
   * \param[in] new_layout Color-attachment or presentation layout to enter.
   */
  auto TransitionSwapchainImageLayout(vk::ImageLayout old_layout,
                                      vk::ImageLayout new_layout) -> void;

  /*! \brief Borrowed device and surface owner. */
  GGEMSVulkanDevice const &device_;

  /*! \brief Owned command pool for all reusable frame slots. */
  vk::raii::CommandPool command_pool_{nullptr};

  /*! \brief Owned frame resources reused only after their fences signal. */
  std::array<FrameSlot, k_frame_slot_count_> frame_slots_{};

  /*! \brief Current swapchain and its per-image resources. */
  SwapchainGeneration generation_{};

  /*! \brief Submission slot selected for the next frame. */
  std::uint32_t frame_slot_index_{0U};

  /*! \brief Acquired image awaiting submission, when present. */
  std::optional<AcquiredImage> acquired_image_;
};

} // namespace ggems::ui::detail
