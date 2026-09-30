#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <mutex>
#include <optional>
#include <format>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "GGEMS/GGEMSRun.hh"
#include "GGEMS/GGEMSException.hh"
#include "GGEMS/logging/GGEMSLogMacros.hh"
#include "GGEMS/observer/GGEMSTransportObserver.hh"
#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/opencl/GGEMSOpenCLContext.hh"
#include "GGEMS/opencl/GGEMSOpenCLDevice.hh"
#include "GGEMS/opencl/GGEMSOpenCLPlatform.hh"
#include "GGEMS/opencl/GGEMSOpenCLStrings.hh"
#include "GGEMS/render/GGEMSParticleTrace.hh"
#include "GGEMS/sources/GGEMSSourceRunSnapshot.hh"
#include "GGEMS/ui/GGEMSGuiApplication.hh"
#include "GGEMS/ui/detail/GGEMSDeviceStatus.hh"
#include "GGEMS/ui/detail/GGEMSImGuiIntegration.hh"
#include "GGEMS/ui/detail/GGEMSImGuiLayer.hh"
#include "GGEMS/ui/detail/GGEMSVulkanDevice.hh"
#include "GGEMS/ui/detail/GGEMSVulkanDeviceSelection.hh"
#include "GGEMS/ui/detail/GGEMSVulkanPresenter.hh"
#include "GGEMS/ui/detail/GGEMSVulkanSceneRenderer.hh"
#include "GGEMS/ui/detail/GGEMSWindowIconData.hh"

namespace {
// =============================================================================
// =============================================================================

[[nodiscard]] auto BuildComputeStatus(
  std::vector<ggems::ocl::GGEMSOpenCLContext> const &contexts,
  std::vector<ggems::ocl::GGEMSOpenCLPlatform> const &platforms)
  -> ggems::ui::detail::GGEMSComputeStatus {
  if (contexts.empty()) {
    return {};
  }

  ggems::ui::detail::GGEMSComputeStatus compute_status{.initialized = true};

  compute_status.devices.reserve(contexts.size());

  for (std::size_t context_index = 0U; context_index < contexts.size();
       ++context_index) {
    ggems::ocl::GGEMSOpenCLDevice const &device =
      contexts[context_index].GetDevice();

    std::size_t platform_index = device.GetPlatformIndex();

    auto platform = std::ranges::find_if(
      platforms,
      [platform_index](ggems::ocl::GGEMSOpenCLPlatform const &candidate)
        -> bool { return candidate.GetPlatformIndex() == platform_index; });

    if (!(platform != platforms.end())) {
      throw ggems::core::GGEMSInternal(std::format(
        "Active OpenCL context {} refers to unavailable platform index {}.",
        context_index, platform_index));
    }

    compute_status.devices.push_back(
      ggems::ui::detail::GGEMSComputeDeviceStatus{
        .context_index = context_index,
        .name = device.GetName(),
        .type = ggems::ocl::DeviceTypeToString(device.GetType()),
        .platform = platform->GetName(),
      });
  }

  for (ggems::ui::detail::GGEMSComputeDeviceStatus &device :
       compute_status.devices) {
    auto matching_name_count = std::ranges::count_if(
      compute_status.devices,
      [&device](ggems::ui::detail::GGEMSComputeDeviceStatus const &candidate)
        -> bool { return candidate.name == device.name; });

    device.show_platform = matching_name_count > 1;
  }

  return compute_status;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto BuildImGuiBackendEpoch(
  ggems::ui::detail::GGEMSVulkanPresenter const &presenter) noexcept
  -> ggems::ui::detail::GGEMSImGuiIntegration::VulkanBackendEpoch {
  std::uint32_t const image_count = presenter.GetImageCount();

  return ggems::ui::detail::GGEMSImGuiIntegration::VulkanBackendEpoch{
    .image_count = image_count,
    .min_image_count = image_count,
    .color_format = presenter.GetImageFormat(),
    .sample_count = vk::SampleCountFlagBits::e1,
    .view_mask = 0U,
    .depth_format = vk::Format::eUndefined,
    .stencil_format = vk::Format::eUndefined,
  };
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto GetGLFWErrorMessage(std::string_view context)
  -> std::string {
  char const *description{nullptr};
  int error_code = glfwGetError(&description);

  return std::format("{} GLFW error {}: {}.", context, error_code,
                     description != nullptr ? description
                                            : "No diagnostic available");
}
} // namespace

namespace ggems::ui {

// =============================================================================
// =============================================================================

GGEMSGuiApplication::GGEMSGuiApplication(std::string title, std::int32_t width,
                                         std::int32_t height)
    : title_{std::move(title)}, width_{width}, height_{height} {}

// -----------------------------------------------------------------------------

GGEMSGuiApplication::~GGEMSGuiApplication() noexcept { Shutdown(); }

// -----------------------------------------------------------------------------

void GGEMSGuiApplication::Shutdown() noexcept {
  if (device_ != nullptr) {
    try {
      device_->GetDevice().waitIdle();
    } catch (...) {
      std::fputs(
        "[GGEMS Vulkan] Failed to wait for device idle during shutdown.\n",
        stderr);
    }
  }

  if (imgui_integration_ != nullptr) {
    imgui_integration_->UnregisterSceneTexture();
  }

  if (scene_renderer_ != nullptr) {
    scene_renderer_->Shutdown();
  }

  imgui_integration_.reset();
  scene_renderer_.reset();
  imgui_layer_.reset();
  presenter_.reset();
  device_.reset();

  if (window_ != nullptr) {
    glfwDestroyWindow(window_);
    window_ = nullptr;
  }

  if (glfw_initialized_) {
    glfwTerminate();
    glfw_initialized_ = false;
  }
}

// -----------------------------------------------------------------------------

auto GGEMSGuiApplication::IsInitialized() const noexcept -> bool {
  return window_ != nullptr && imgui_layer_ != nullptr;
}

// -----------------------------------------------------------------------------

auto GGEMSGuiApplication::SetVulkanDevice(std::string selection) -> void {
  if (!(window_ == nullptr)) {
    throw ggems::core::GGEMSRecoverable(
      "Vulkan device selection must be configured before "
      "GGEMS GuiMode initialization.");
  }

  if (selection.empty()) {
    throw ggems::core::GGEMSRecoverable(
      "Vulkan device selection expects 'auto' or a non-empty device name.");
  }

  vulkan_device_name_selector_ = std::move(selection);
  vulkan_device_index_selector_.reset();
}

// -----------------------------------------------------------------------------

auto GGEMSGuiApplication::SetVulkanDevice(std::uint32_t enumeration_index)
  -> void {
  if (!(window_ == nullptr)) {
    throw ggems::core::GGEMSRecoverable(
      "Vulkan device selection must be configured before "
      "GGEMS GuiMode initialization.");
  }

  vulkan_device_index_selector_ = enumeration_index;
}

// -----------------------------------------------------------------------------

auto GGEMSGuiApplication::Initialize() -> void {
  if (window_ != nullptr) {
    return;
  }

  if (!(width_ > 0 && height_ > 0)) {
    throw ggems::core::GGEMSRecoverable(
      "GGEMS GuiMode window dimensions must be strictly positive.");
  }

  if (glfwInit() != GLFW_TRUE) {
    throw ggems::core::GGEMSRecoverable(
      GetGLFWErrorMessage("Unable to Initialize GGEMS GuiMode."));
  }

  glfw_initialized_ = true;

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
  glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

  window_ =
    glfwCreateWindow(static_cast<int>(width_), static_cast<int>(height_),
                     title_.c_str(), nullptr, nullptr);

  if (window_ == nullptr) {
    std::string error =
      GetGLFWErrorMessage("Unable to create the GGEMS GuiMode window.");
    Shutdown();
    throw ggems::core::GGEMSRecoverable(error);
  }

  GLFWimage icon{
    .width = detail::k_ggems_window_icon_width,
    .height = detail::k_ggems_window_icon_height,
    .pixels = detail::k_ggems_window_icon_pixels.data(),
  };

  glfwSetWindowIcon(window_, 1, &icon);

  glfwSetWindowUserPointer(window_, this);
  glfwSetFramebufferSizeCallback(
    window_, &GGEMSGuiApplication::FramebufferResizeCallback);

  try {
    auto &opencl = ocl::GGEMSOpenCL::GetInstance();
    detail::GGEMSComputeStatus compute_status =
      BuildComputeStatus(opencl.GetContext(), opencl.GetPlatforms());

    detail::GGEMSVulkanDeviceSelector device_selector =
      vulkan_device_index_selector_.has_value()
        ? detail::GGEMSVulkanDeviceSelector::FromIndex(
            vulkan_device_index_selector_.value())
        : detail::GGEMSVulkanDeviceSelector::FromString(
            vulkan_device_name_selector_);

    try {
      device_ =
        std::make_unique<detail::GGEMSVulkanDevice>(window_, device_selector);

      presenter_ =
        std::make_unique<detail::GGEMSVulkanPresenter>(*device_, window_);

      imgui_integration_ = std::make_unique<detail::GGEMSImGuiIntegration>();
      imgui_integration_->Initialize(
        window_,
        detail::GGEMSImGuiIntegration::VulkanHandles{
          .instance = *device_->GetInstance(),
          .physical_device = *device_->GetPhysicalDevice(),
          .device = *device_->GetDevice(),
          .graphics_queue = *device_->GetGraphicsQueue(),
          .graphics_queue_family = device_->GetGraphicsQueueFamily(),
        },
        BuildImGuiBackendEpoch(*presenter_));

      scene_renderer_ = std::make_unique<GGEMSVulkanSceneRenderer>();
      scene_renderer_->Initialize(device_->GetPhysicalDevice(),
                                  device_->GetDevice(),
                                  vk::Format::eR8G8B8A8Unorm);

      imgui_layer_ = std::make_unique<GGEMSImGuiLayer>();
    } catch (vk::SystemError const &error) {
      throw ggems::core::GGEMSRecoverable(
        std::format("Unable to initialize Vulkan GuiMode: {}.", error.what()));
    }

    device_status_ = detail::GGEMSDeviceStatusSnapshot{
      .renderer = device_->GetStatus(),
      .compute = std::move(compute_status),
    };
  } catch (...) {
    Shutdown();
    throw;
  }

  GGEMS_INFO("Gui", "GGEMS GuiMode window and Vulkan bootstrap initialized.");
}

// -----------------------------------------------------------------------------

void GGEMSGuiApplication::Run() {
  if (!IsInitialized()) {
    throw ggems::core::GGEMSInternal(
      "GGEMS GuiMode must be initialized before entering its event loop.");
  }

  GGEMS_INFOEX("Gui", 1, "GGEMS GuiMode event loop started.");

  while (glfwWindowShouldClose(window_) == GLFW_FALSE) {
    glfwPollEvents();

    bool framebuffer_resized = framebuffer_resized_;
    framebuffer_resized_ = false;

    RenderFrame(framebuffer_resized);
  }

  GGEMS_INFOEX("Gui", 1, "GGEMS GuiMode event loop stopped.");
}

// -----------------------------------------------------------------------------

void GGEMSGuiApplication::FramebufferResizeCallback(GLFWwindow *window, int,
                                                    int) noexcept {
  auto *application =
    static_cast<GGEMSGuiApplication *>(glfwGetWindowUserPointer(window));

  if (application != nullptr) {
    application->framebuffer_resized_ = true;
  }
}

// -----------------------------------------------------------------------------

auto GGEMSGuiApplication::RenderFrame(bool framebuffer_resized) -> void {
  imgui_integration_->ThrowIfBackendFailed();

  try {
    if (framebuffer_resized) {
      RecreateSwapchain();
    }

    if (glfwWindowShouldClose(window_) == GLFW_TRUE) {
      return;
    }

    presenter_->AcquireImage();

    BuildImGuiFrame();

    vk::raii::CommandBuffer const &command_buffer =
      presenter_->BeginRecording();

    scene_renderer_->RecordSceneCommands(
      command_buffer, imgui_layer_->GetParticleTraceVisibility());

    presenter_->BeginMainPass();
    imgui_integration_->RenderDrawData(*command_buffer);

    if (presenter_->EndFrame()) {
      RecreateSwapchain();
    }
  } catch (vk::OutOfDateKHRError const &) {
    RecreateSwapchain();
  } catch (vk::SystemError const &error) {
    throw ggems::core::GGEMSRecoverable(std::format(
      "Unable to render a Vulkan GuiMode frame: {}.", error.what()));
  }
}

// -----------------------------------------------------------------------------

auto GGEMSGuiApplication::RecreateSwapchain() -> void {
  presenter_->RecreateSwapchain(window_);
  imgui_integration_->UpdateVulkanBackend(BuildImGuiBackendEpoch(*presenter_));
}

// -----------------------------------------------------------------------------

auto GGEMSGuiApplication::RecreateSceneRenderTargets() -> void {
  device_->GetDevice().waitIdle();

  imgui_integration_->UnregisterSceneTexture();
  scene_renderer_->RecreateRenderTargets();
  imgui_integration_->RegisterSceneTexture(
    scene_renderer_->GetColorImageView());
}

// -----------------------------------------------------------------------------

auto GGEMSGuiApplication::ApplyPendingSourceRunSnapshot() -> void {
  std::optional<core::sources::GGEMSSourceRunSnapshot> snapshot{};

  {
    std::scoped_lock lock{pending_source_run_snapshot_mutex_};

    if (!pending_source_run_snapshot_.has_value()) {
      return;
    }

    snapshot = std::move(pending_source_run_snapshot_);
    pending_source_run_snapshot_.reset();
  }

  imgui_layer_->SetSourceRunSnapshot(std::move(*snapshot));
}

// -----------------------------------------------------------------------------

auto GGEMSGuiApplication::ApplyPendingParticleTraceSegments() -> void {
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

  device_->GetDevice().waitIdle();

  if (clear_segments) {
    scene_renderer_->ClearParticleTraces();
  }

  if (apply_segments) {
    scene_renderer_->SetParticleTraceSegments(segments);
  }
}

// -----------------------------------------------------------------------------

auto GGEMSGuiApplication::BuildImGuiFrame() -> void {
  ApplyPendingSourceRunSnapshot();
  ApplyPendingParticleTraceSegments();

  imgui_integration_->BeginFrame();

  GGEMSImGuiLayer::ViewportState viewport_state =
    imgui_layer_->GetViewportState();

  if (viewport_state.visible) {
    scene_renderer_->SetViewportExtent(viewport_state.extent);

    if (scene_renderer_->RequiresResize()) {
      RecreateSceneRenderTargets();
    }
  }

  imgui_layer_->BuildFrame(
    presenter_->GetExtent(), imgui_integration_->GetSceneTextureID(),
    scene_renderer_->GetViewportExtent(), device_status_);

  GGEMSImGuiLayer::ViewportState updated_viewport_state =
    imgui_layer_->GetViewportState();

  scene_renderer_->OrbitCamera(updated_viewport_state.orbit_delta_x_pixels,
                               updated_viewport_state.orbit_delta_y_pixels);

  scene_renderer_->PanCamera(updated_viewport_state.pan_delta_x_pixels,
                             updated_viewport_state.pan_delta_y_pixels);

  scene_renderer_->ZoomCamera(updated_viewport_state.zoom_delta);

  if (imgui_layer_->ShouldResetCamera()) {
    scene_renderer_->ResetCamera();
  }

  scene_renderer_->SetShowAxes(imgui_layer_->ShouldShowAxes());
}

// -----------------------------------------------------------------------------

auto GGEMSGuiApplication::SubmitLastRunSourceSnapshot(
  ggems::core::GGEMSRun const &run) -> void {
  if (!IsInitialized()) {
    throw ggems::core::GGEMSRecoverable(
      "GGEMS GuiMode must be initialized before submitting a source "
      "snapshot.");
  }

  auto snapshot = run.GetLastSourceRunSnapshot();

  if (!snapshot.has_value()) {
    throw ggems::core::GGEMSRecoverable(
      "GGEMSRun has no successfully completed source snapshot to submit.");
  }

  if (snapshot->HasActivityDrivenSource()) {
    throw ggems::core::GGEMSRecoverable(
      "ActivityDriven GUI presentation is not implemented.");
  }

  if (!run.HasObserver() && !missing_observer_warning_emitted_) {
    missing_observer_warning_emitted_ = true;

    GGEMS_WARN("Gui",
               "No GGEMSTransportObserver is attached to this GGEMSRun. "
               "GuiMode can display the scene, but no particle trajectories "
               "will be available.");
  }

  std::scoped_lock lock{pending_source_run_snapshot_mutex_};
  pending_source_run_snapshot_ = std::move(*snapshot);
}

// -----------------------------------------------------------------------------

void GGEMSGuiApplication::SubmitParticleTraceSegments(
  std::vector<ggems::render::GGEMSParticleTraceSegment> segments) {
  if (!IsInitialized()) {
    throw ggems::core::GGEMSRecoverable(
      "GGEMS GuiMode must be initialized before submitting particle traces.");
  }

  std::scoped_lock lock{pending_particle_trace_mutex_};

  pending_particle_trace_segments_ = std::move(segments);
  has_pending_particle_trace_segments_ = true;
  pending_particle_trace_clear_ = false;
}

// -----------------------------------------------------------------------------

void GGEMSGuiApplication::SubmitParticleTracesFromObserver(
  ggems::core::observer::GGEMSTransportObserver const &observer) {
  SubmitParticleTraceSegments(
    ggems::render::BuildParticleTraceSegments(observer.GetRecords()));
}

// -----------------------------------------------------------------------------

void GGEMSGuiApplication::ClearParticleTraces() {
  if (!IsInitialized()) {
    throw ggems::core::GGEMSRecoverable(
      "GGEMS GuiMode must be initialized before clearing particle traces.");
  }

  std::scoped_lock lock{pending_particle_trace_mutex_};

  pending_particle_trace_segments_.clear();
  has_pending_particle_trace_segments_ = false;
  pending_particle_trace_clear_ = true;
}
} // namespace ggems::ui
