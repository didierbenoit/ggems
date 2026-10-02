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
 * \brief Owns the GUI event loop and submits diagnostic scene data.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "GGEMS/render/GGEMSParticleTrace.hh"
#include "GGEMS/sources/GGEMSSourceRunSnapshot.hh"
#include "GGEMS/ui/detail/GGEMSDeviceStatus.hh"
#include "GGEMS/ui/detail/GGEMSWorkbenchState.hh"

struct GLFWwindow;

namespace ggems::core {
class GGEMSRun;
}

namespace ggems::core::observer {
class GGEMSTransportObserver;
}

/*! \brief Presents GGEMS diagnostics through Dear ImGui and Vulkan. */
namespace ggems::ui {

class GGEMSImGuiLayer;
class GGEMSVulkanSceneRenderer;

namespace detail {
class GGEMSImGuiIntegration;
class GGEMSVulkanDevice;
class GGEMSVulkanPresenter;
} // namespace detail

/*!
 * \brief Owns the window, renderer, and diagnostic presentation state.
 *
 * Rendering runs in the GLFW event loop. Submitted source snapshots and trace
 *   segments replace pending display data and are consumed by that loop.
 */
class GGEMSGuiApplication {
public:
  /*!
   * \brief Stores the requested window configuration.
   *
   * \param[in] title Window title.
   * \param[in] width Initial window width in screen coordinates.
   * \param[in] height Initial window height in screen coordinates.
   */
  explicit GGEMSGuiApplication(std::string title = "GGEMS GuiMode",
                               std::int32_t width = 1600,
                               std::int32_t height = 900);

  /*! \brief Waits for rendering and releases the GUI resources. */
  ~GGEMSGuiApplication() noexcept;

  /*! \brief Disallows copying the owned UI state. */
  GGEMSGuiApplication(GGEMSGuiApplication const &) = delete;

  /*! \brief Disallows moving the owned UI state. */
  GGEMSGuiApplication(GGEMSGuiApplication &&) = delete;

  /*! \brief Disallows copy assignment of the owned UI state. */
  auto operator=(GGEMSGuiApplication const &) -> GGEMSGuiApplication & = delete;

  /*! \brief Disallows move assignment of the owned UI state. */
  auto operator=(GGEMSGuiApplication &&) -> GGEMSGuiApplication & = delete;

  /*!
   * \brief Creates the window and initializes Vulkan and Dear ImGui.
   *
   * Returns immediately when the window already exists; failed setup releases
   *   owned resources.
   *
   * \throws core::GGEMSRecoverable If dimensions or platform initialization
   *   fail.
   */
  void Initialize();

  /*!
   * \brief Processes events and renders until the window closes.
   *
   * A frame exception ends the loop and prevents rendering from resuming.
   *
   * \throws core::GGEMSInternal If initialization has not completed.
   * \throws core::GGEMSRecoverable If rendering fails or a previous frame
   *   failed.
   */
  void Run();

  /*!
   * \brief Queues the last successful CountDriven source snapshot.
   *
   * The newest submission replaces any pending snapshot. ActivityDriven
   *   presentation is not implemented.
   *
   * \param[in] run Run whose completed source snapshot is copied for
   *   presentation.
   * \throws core::GGEMSRecoverable If the GUI is uninitialized, no completed
   *   snapshot exists, or the snapshot contains an ActivityDriven source.
   */
  auto SubmitLastRunSourceSnapshot(ggems::core::GGEMSRun const &run) -> void;

  /*!
   * \brief Queues replacement diagnostic traces for the render loop.
   *
   * \param[in] segments Owned segments replacing the pending trace set.
   * \throws core::GGEMSRecoverable If the GUI has not been initialized.
   */
  void SubmitParticleTraceSegments(
    std::vector<ggems::render::GGEMSParticleTraceSegment> segments);

  /*!
   * \brief Queues diagnostic lines derived from captured observer records.
   *
   * \param[in] observer Observer whose current records are converted to trace
   *   segments.
   * \throws core::GGEMSRecoverable If the GUI has not been initialized.
   */
  void SubmitParticleTracesFromObserver(
    ggems::core::observer::GGEMSTransportObserver const &observer);

  /*!
   * \brief Queues removal of traces and cancels a pending replacement.
   *
   * \throws core::GGEMSRecoverable If the GUI has not been initialized.
   */
  void ClearParticleTraces();

  /*!
   * \brief Reports whether the window and UI layer exist.
   *
   * \return True when GUI initialization has established both objects.
   */
  [[nodiscard]] auto IsInitialized() const noexcept -> bool;

  /*!
   * \brief Selects automatic matching or a Vulkan device name.
   *
   * Must be configured before initialization; replaces an index selection.
   *
   * \param[in] selection Case-insensitive auto keyword or nonempty device-name
   *   query.
   * \throws core::GGEMSRecoverable If a window exists or the selection is
   *   empty.
   */
  void SetVulkanDevice(std::string selection);

  /*!
   * \brief Selects a Vulkan physical device by enumeration index.
   *
   * \param[in] enumeration_index Index in the Vulkan physical-device
   *   enumeration.
   * \throws core::GGEMSRecoverable If a window already exists.
   */
  void SetVulkanDevice(std::uint32_t enumeration_index);

private:
  /*! \brief Releases GUI resources after waiting for pending device work. */
  void Shutdown() noexcept;

  /*!
   * \brief Marks the window framebuffer for swapchain recreation.
   *
   * \param[in] window GLFW window whose user pointer identifies this
   *   application.
   * \param[in] width New framebuffer width; the callback does not use it.
   * \param[in] height New framebuffer height; the callback does not use it.
   */
  static void FramebufferResizeCallback(GLFWwindow *window, int width,
                                        int height) noexcept;

  /*!
   * \brief Records and presents one frame, handling swapchain invalidation.
   *
   * \param[in] framebuffer_resized Whether a GLFW resize requires swapchain
   *   recreation.
   */
  auto RenderFrame(bool framebuffer_resized) -> void;

  /*! \brief Waits for a usable framebuffer and refreshes presentation. */
  auto RecreateSwapchain() -> void;

  /*! \brief Applies queued data and builds the current presentation frame. */
  auto BuildImGuiFrame() -> void;

  /*! \brief Replaces idle scene targets and their ImGui texture binding. */
  auto RecreateSceneRenderTargets() -> void;

  /*! \brief Publishes the latest queued source snapshot to the panels. */
  auto ApplyPendingSourceRunSnapshot() -> void;

  /*! \brief Applies queued trace replacement or removal while idle. */
  auto ApplyPendingParticleTraceSegments() -> void;

  /*! \brief Requested GLFW window title. */
  std::string title_;

  /*! \brief Requested initial window width in screen coordinates. */
  std::int32_t width_{0};

  /*! \brief Requested initial window height in screen coordinates. */
  std::int32_t height_{0};

  /*! \brief Automatic-selection keyword or device-name query. */
  std::string vulkan_device_name_selector_{"auto"};

  /*! \brief Explicit device index taking precedence over the name query. */
  std::optional<std::uint32_t> vulkan_device_index_selector_;

  /*! \brief Owned GLFW window, destroyed during shutdown. */
  GLFWwindow *window_{nullptr};

  /*! \brief Vulkan instance, surface, device, and queue owner. */
  std::unique_ptr<detail::GGEMSVulkanDevice> device_;

  /*! \brief Swapchain and frame synchronization owner. */
  std::unique_ptr<detail::GGEMSVulkanPresenter> presenter_;

  /*! \brief Dear ImGui context and backend owner. */
  std::unique_ptr<detail::GGEMSImGuiIntegration> imgui_integration_;

  /*! \brief Offscreen axes and diagnostic trace renderer. */
  std::unique_ptr<GGEMSVulkanSceneRenderer> scene_renderer_;

  /*! \brief Panel composition and UI interaction state. */
  std::unique_ptr<GGEMSImGuiLayer> imgui_layer_;

  /*! \brief Device descriptions captured during GUI initialization. */
  detail::GGEMSDeviceStatusSnapshot device_status_{};

  /*! \brief Current camera, selection, and presentation controls. */
  detail::GGEMSWorkbenchState workbench_{};

  /*! \brief Logical width represented by the current scene target. */
  std::uint32_t scene_target_logical_width_{0U};

  /*! \brief Logical height represented by the current scene target. */
  std::uint32_t scene_target_logical_height_{0U};

  /*! \brief Owned completed snapshot currently shown by the panels. */
  std::optional<core::sources::GGEMSSourceRunSnapshot>
    displayed_source_run_snapshot_;

  /*! \brief Protects replacement of the pending source snapshot. */
  std::mutex pending_source_run_snapshot_mutex_;

  /*! \brief Newest submitted snapshot awaiting the UI loop. */
  std::optional<core::sources::GGEMSSourceRunSnapshot>
    pending_source_run_snapshot_;

  /*! \brief Protects queued trace data and clear requests. */
  std::mutex pending_particle_trace_mutex_;

  /*! \brief Owned trace segments awaiting upload by the UI loop. */
  std::vector<ggems::render::GGEMSParticleTraceSegment>
    pending_particle_trace_segments_;

  /*! \brief Distinguishes a submitted empty trace set from no submission. */
  bool has_pending_particle_trace_segments_{false};

  /*! \brief Requests removal of the displayed trace set. */
  bool pending_particle_trace_clear_{false};

  /*! \brief Whether shutdown must terminate the GLFW runtime. */
  bool glfw_initialized_{false};

  /*! \brief Resize notification awaiting the next render iteration. */
  bool framebuffer_resized_{false};

  /*! \brief Latches a terminal frame failure against later loop entry. */
  bool rendering_failed_{false};

  /*! \brief Prevents repeated missing-observer warnings. */
  bool missing_observer_warning_emitted_{false};
};

} // namespace ggems::ui
