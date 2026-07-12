#include <format>
#include <string>
#include <string_view>
#include <utility>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/observer/GGEMSTransportObserver.hh"
#include "GGEMS/render/GGEMSParticleTrace.hh"
#include "GGEMS/ui/GGEMSGuiApplication.hh"
#include "GGEMSVulkanContext.hh"
#include "GGEMSVulkanDeviceSelection.hh"

namespace {
[[nodiscard]] std::string GetGLFWErrorMessage(std::string_view context) {
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
    : title_(title), width_(width), height_(height) {}

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

bool GGEMSGuiApplication::IsInitialised() const noexcept {
  return window_ != nullptr && vk_context_ != nullptr &&
         vk_context_->IsInitialised();
}

// -----------------------------------------------------------------------------

void GGEMSGuiApplication::SetVulkanDevice(std::string selection) {
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

void GGEMSGuiApplication::SetVulkanDevice(std::uint32_t enumeration_index) {
  GGEMS_CHECK_RECOVERABLE(window_ == nullptr,
                          "Vulkan device selection must be configured before "
                          "GGEMS GuiMode initialisation.");

  vulkan_device_index_selector_ = enumeration_index;
}

// -----------------------------------------------------------------------------

void GGEMSGuiApplication::Initialise() {
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

  glfwSetWindowUserPointer(window_, this);
  glfwSetFramebufferSizeCallback(
      window_, &GGEMSGuiApplication::FramebufferResizeCallback);

  try {
    detail::GGEMSVulkanDeviceSelector device_selector =
        vulkan_device_index_selector_.has_value()
            ? detail::GGEMSVulkanDeviceSelector::FromIndex(
                  vulkan_device_index_selector_.value())
            : detail::GGEMSVulkanDeviceSelector::FromString(
                  vulkan_device_name_selector_);

    vk_context_ = std::make_unique<GGEMSVulkanContext>();
    vk_context_->Initialise(window_, device_selector);
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
