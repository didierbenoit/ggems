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

#include "GGEMS/core/GGEMSRun.hh"
#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSMacros.hh"
#include "GGEMS/core/observer/GGEMSTransportObserver.hh"
#include "GGEMS/frameworks/GGEMSOpenCL.hh"
#include "GGEMS/frameworks/GGEMSOpenCLContext.hh"
#include "GGEMS/frameworks/GGEMSOpenCLDevice.hh"
#include "GGEMS/frameworks/GGEMSOpenCLPlatform.hh"
#include "GGEMS/frameworks/GGEMSOpenCLStrings.hh"
#include "GGEMS/render/GGEMSParticleTrace.hh"
#include "GGEMS/ui/GGEMSGuiApplication.hh"

#include "GGEMSDeviceStatus.hh"
#include "GGEMSVulkanContext.hh"
#include "GGEMSVulkanDeviceSelection.hh"
#include "GGEMSWindowIconData.hh"

namespace {
// =============================================================================
// =============================================================================

[[nodiscard]] auto BuildComputeStatus(ggems::ocl::GGEMSOpenCL &opencl)
    -> ggems::ui::detail::GGEMSComputeStatus {
  auto const &contexts = opencl.GetContext();

  if (contexts.empty()) {
    return {};
  }

  auto const &platforms = opencl.GetPlatforms();

  ggems::ui::detail::GGEMSComputeStatus compute_status{.initialised = true};

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

    GGEMS_CHECK_INTERNAL(
        platform != platforms.end(),
        std::format(
            "Active OpenCL context {} refers to unavailable platform index {}.",
            context_index, platform_index));

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

  if (glfw_initialised_) {
    glfwTerminate();
    glfw_initialised_ = false;
  }
}

// -----------------------------------------------------------------------------

auto GGEMSGuiApplication::IsInitialised() const noexcept -> bool {
  return window_ != nullptr && vk_context_ != nullptr &&
         vk_context_->IsInitialised();
}

// -----------------------------------------------------------------------------

auto GGEMSGuiApplication::SetVulkanDevice(std::string selection) -> void {
  GGEMS_CHECK_RECOVERABLE(window_ == nullptr,
                          "Vulkan device selection must be configured before "
                          "GGEMS GuiMode initialisation.");

  GGEMS_CHECK_RECOVERABLE(
      !selection.empty(),
      "Vulkan device selection expects 'auto' or a non-empty device name.");

  vulkan_device_name_selector_ = std::move(selection);
  vulkan_device_index_selector_.reset();
}

// -----------------------------------------------------------------------------

auto GGEMSGuiApplication::SetVulkanDevice(std::uint32_t enumeration_index)
    -> void {
  GGEMS_CHECK_RECOVERABLE(window_ == nullptr,
                          "Vulkan device selection must be configured before "
                          "GGEMS GuiMode initialisation.");

  vulkan_device_index_selector_ = enumeration_index;
}

// -----------------------------------------------------------------------------

auto GGEMSGuiApplication::Initialise() -> void {
  if (window_ != nullptr) {
    return;
  }

  GGEMS_CHECK_RECOVERABLE(
      width_ > 0 && height_ > 0,
      "GGEMS GuiMode window dimensions must be strictly positive.");

  if (glfwInit() != GLFW_TRUE) {
    GGEMS_RECOVERABLE(
        GetGLFWErrorMessage("Unable to Initialise GGEMS GuiMode."));
  }

  glfw_initialised_ = true;

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
    GGEMS_RECOVERABLE(error);
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
    detail::GGEMSComputeStatus compute_status = BuildComputeStatus(opencl);

    detail::GGEMSVulkanDeviceSelector device_selector =
        vulkan_device_index_selector_.has_value()
            ? detail::GGEMSVulkanDeviceSelector::FromIndex(
                  vulkan_device_index_selector_.value())
            : detail::GGEMSVulkanDeviceSelector::FromString(
                  vulkan_device_name_selector_);

    vk_context_ = std::make_unique<GGEMSVulkanContext>();
    vk_context_->Initialise(window_, device_selector,
                            std::move(compute_status));
  } catch (...) {
    Shutdown();
    throw;
  }

  GGEMS_INFO("Gui", "GGEMS GuiMode window and Vulkan bootstrap initialised.");
}

// -----------------------------------------------------------------------------

void GGEMSGuiApplication::Run() {
  GGEMS_CHECK_INTERNAL(
      window_ != nullptr,
      "GGEMS GuiMode must be initialised before entering its event loop.");

  GGEMS_INFOEX("Gui", 1, "GGEMS GuiMode event loop started.");

  GGEMS_CHECK_INTERNAL(vk_context_ != nullptr && vk_context_->IsInitialised(),
                       "GGEMS GuiMode required an initialised Vulkan context "
                       "before entering the event loop.");

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
  GGEMS_CHECK_RECOVERABLE(
      vk_context_ != nullptr && vk_context_->IsInitialised(),
      "GGEMS GuiMode must be initialised before submitting a source "
      "snapshot.");

  auto snapshot = run.GetLastSourceRunSnapshot();

  GGEMS_CHECK_RECOVERABLE(
      snapshot.has_value(),
      "GGEMSRun has no successfully completed source snapshot to submit.");

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
  GGEMS_CHECK_RECOVERABLE(
      vk_context_ != nullptr && vk_context_->IsInitialised(),
      "GGEMS GuiMode must be initialised before submitting particle traces.");
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
  GGEMS_CHECK_RECOVERABLE(
      vk_context_ != nullptr && vk_context_->IsInitialised(),
      "GGEMS GuiMode must be initialised before clearing particle traces.");

  vk_context_->ClearParticleTraces();
}
} // namespace ggems::ui
