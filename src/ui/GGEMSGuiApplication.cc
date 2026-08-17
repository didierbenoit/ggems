#include <algorithm>
#include <format>
#include <string>
#include <string_view>
#include <utility>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

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
#include "GGEMS/ui/GGEMSGuiApplication.hh"
#include "GGEMS/ui/GGEMSDeviceStatus.hh"
#include "GGEMS/ui/GGEMSVulkanContext.hh"
#include "GGEMS/ui/GGEMSVulkanDeviceSelection.hh"
#include "GGEMS/ui/GGEMSWindowIconData.hh"

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
            .platform = platform->GetName()});
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
  vk_context_.reset();

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
  return window_ != nullptr && vk_context_ != nullptr &&
         vk_context_->IsInitialized();
}

// -----------------------------------------------------------------------------

auto GGEMSGuiApplication::SetVulkanDevice(std::string selection) -> void {
  if (!(window_ == nullptr)) {
    throw ggems::core::GGEMSRecoverable("Vulkan device selection must be configured before "
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
    throw ggems::core::GGEMSRecoverable("Vulkan device selection must be configured before "
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
    throw ggems::core::GGEMSRecoverable(GetGLFWErrorMessage("Unable to Initialize GGEMS GuiMode."));
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

  GLFWimage icon{.width = detail::k_ggems_window_icon_width,
                 .height = detail::k_ggems_window_icon_height,
                 .pixels = detail::k_ggems_window_icon_pixels.data()};
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

    vk_context_ = std::make_unique<GGEMSVulkanContext>();
    vk_context_->Initialize(window_, device_selector,
                            std::move(compute_status));
  } catch (...) {
    Shutdown();
    throw;
  }

  GGEMS_INFO("Gui", "GGEMS GuiMode window and Vulkan bootstrap initialized.");
}

// -----------------------------------------------------------------------------

void GGEMSGuiApplication::Run() {
  if (!(window_ != nullptr)) {
    throw ggems::core::GGEMSInternal(
        "GGEMS GuiMode must be initialized before entering its event loop.");
  }

  GGEMS_INFOEX("Gui", 1, "GGEMS GuiMode event loop started.");

  if (!(vk_context_ != nullptr && vk_context_->IsInitialized())) {
    throw ggems::core::GGEMSInternal("GGEMS GuiMode required an initialized Vulkan context "
                       "before entering the event loop.");
  }

  while (glfwWindowShouldClose(window_) == GLFW_FALSE) {
    glfwPollEvents();

    bool framebuffer_resized = framebuffer_resized_;
    framebuffer_resized_ = false;

    vk_context_->RenderFrame(window_, framebuffer_resized);
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

auto GGEMSGuiApplication::SubmitLastRunSourceSnapshot(
    ggems::core::GGEMSRun const &run) -> void {
  if (!(vk_context_ != nullptr && vk_context_->IsInitialized())) {
    throw ggems::core::GGEMSRecoverable(
        "GGEMS GuiMode must be initialized before submitting a source "
      "snapshot.");
  }

  auto snapshot = run.GetLastSourceRunSnapshot();

  if (!(snapshot.has_value())) {
    throw ggems::core::GGEMSRecoverable(
        "GGEMSRun has no successfully completed source snapshot to submit.");
  }

  if (snapshot->HasActivityDrivenSource()) {
    throw ggems::core::GGEMSRecoverable("ActivityDriven GUI presentation is not implemented.");
  }

  if (!run.HasObserver() && !missing_observer_warning_emitted_) {
    missing_observer_warning_emitted_ = true;

    GGEMS_WARN("Gui",
               "No GGEMSTransportObserver is attached to this GGEMSRun. "
               "GuiMode can display the scene, but no particle trajectories "
               "will be available.");
  }

  vk_context_->SubmitSourceRunSnapshot(std::move(*snapshot));
}

// -----------------------------------------------------------------------------

void GGEMSGuiApplication::SubmitParticleTraceSegments(
    std::vector<ggems::render::GGEMSParticleTraceSegment> segments) {
  if (!(vk_context_ != nullptr && vk_context_->IsInitialized())) {
    throw ggems::core::GGEMSRecoverable(
        "GGEMS GuiMode must be initialized before submitting particle traces.");
  }
  vk_context_->SubmitParticleTraceSegments(std::move(segments));
}

// -----------------------------------------------------------------------------

void GGEMSGuiApplication::SubmitParticleTracesFromObserver(
    ggems::core::observer::GGEMSTransportObserver const &observer) {
  SubmitParticleTraceSegments(
      ggems::render::BuildParticleTraceSegments(observer.GetRecords()));
}

// -----------------------------------------------------------------------------

void GGEMSGuiApplication::ClearParticleTraces() {
  if (!(vk_context_ != nullptr && vk_context_->IsInitialized())) {
    throw ggems::core::GGEMSRecoverable(
        "GGEMS GuiMode must be initialized before clearing particle traces.");
  }

  vk_context_->ClearParticleTraces();
}
} // namespace ggems::ui
